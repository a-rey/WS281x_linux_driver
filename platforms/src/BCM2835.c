/*
 * BCM2835.c
 *
 * Hardware library for controlling the PWM with DMA on the BCM2835 SoC
 *
 * References:
 * http://elinux.org/BCM2835_datasheet_errata
 * https://www.scribd.com/doc/127599939/BCM2835-Audio-clocks
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
 *
 * Aaron Reyes
 */

#include <linux/kernel.h> /* for printk */
#include <linux/delay.h>  /* for udelay() */
#include <linux/slab.h>   /* for kmalloc and kfree */
#include <linux/string.h> /* for memset() */
#include <linux/err.h>    /* for error checking functions like IS_ERR() */
#include <asm/io.h>       /* for read/write IO operations and virtual/physical translation */

#include <hal.h>          /* for interface definition */
#include <BCM2835.h>      /* for hardware specific addresses */
#include <neopixel.h>     /* for PIXEL_RATE, num_pixels, PIXEL_DATA_LEN */

/* number of bytes needed to send out for a 55us reset period */
#define PIXEL_RESET_BYTE_PADDING (((55 * 3 * PIXEL_RATE) / 1000000) >> 3)

/* rounds num up to the nearest multiple of div */
#define ROUND_UP(num, div) (num + ((div - (num % div)) % div))

/* constants used to define a 1/0 as seen by the pixel in the PWM buffer */
#define PIXEL_1 ((char)0x6) // 110
#define PIXEL_0 ((char)0x4) // 100

/* DMA control block definition */
struct dma_cb_t {
  uint32_t ti;
  uint32_t source_ad;
  uint32_t dest_ad;
  uint32_t txfr_len;
  uint32_t stride;
  uint32_t nextconbk;
  uint32_t reserved[2];
};

/* the DMA control block must be 256 bit (or 32 byte) aligned */
struct dma_cb_t dma_cb __attribute__((aligned(32)));

/* structure pointers for MMIO operations */
static volatile uint32_t *CM;
static volatile uint32_t *PWM;
static volatile uint32_t *DMA5;
static volatile uint32_t *GPIO;


/*
 * Configures a GPIO pin number to the selected function
 */
static void gpio_config(uint32_t pin, uint32_t fun) {
  // get offset into MM GPIO
  uint32_t reg = pin / 10;
  // get contents of correct GPIO_REG_GPFSEL register
  uint32_t config = ioread32(GPIO + reg);
  // get bit offset into GPIO_REG_GPFSEL register and override old config
  uint32_t offset = (pin % 10) * 3;
  config &= ~(0x7 << offset);
  config |= (fun << offset);
  iowrite32(config, GPIO + reg);
}


/*
 * stops the PWM clock generator and turns off the PWM module
 */
static void pwm_stop(void) {
  // check if the PWM clock is currently running
  if (ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_ENAB) {
    // turn off PWM
    iowrite32(0, PWM + PWM_CTL);
    // turn off the clock
    iowrite32((CM_PWM_CTL_PASSWD | ioread32(CM + CM_PWM_CTL)) & ~CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
    // wait for clock to settle
    udelay(10);
    while (ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY);
  }
}


/*
 * initializes the PWM module/clock manager and configures GPIO for PWM output
 */
static void pwm_start(void) {
  // stop the clock if it is in use
  pwm_stop();
  // setup the PWM clock manager to 3 x PIXEL_RATE
  iowrite32(CM_PWM_DIV_PASSWD | CM_PWM_DIV_DIVI(OSC_FREQ / (3 * PIXEL_RATE)), CM + CM_PWM_DIV);
  // source the PWM clock from the oscillator with no MASH filtering
  iowrite32(CM_PWM_CTL_PASSWD | CM_PWM_CTL_MASH(0) | CM_PWM_CTL_SRC_OSC, CM + CM_PWM_CTL);
  // enable the PWM clock generator with the same config as above
  iowrite32(CM_PWM_CTL_PASSWD | ioread32(CM + CM_PWM_CTL) | CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
  // wait until the generator is running
  udelay(10);
  while (!(ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY));
  // configure 32 bit period transfers
  iowrite32(32, PWM + PWM_RNG1);
  // clear the FIFO
  iowrite32(PWM_CTL_CLRF1, PWM + PWM_CTL);
  // enable DMA with PWM (activate DREQ/PANIC when queue < PWM_FIFO_SIZE)
  // iowrite32(PWM_DMAC_ENAB | PWM_DMAC_PANIC(PWM_FIFO_SIZE) | PWM_DMAC_DREQ(PWM_FIFO_SIZE), PWM + PWM_DMAC);

  iowrite32(0xdb6db692, PWM + PWM_FIF1);
  iowrite32(0x49249249, PWM + PWM_FIF1);
  iowrite32(0x24000000, PWM + PWM_FIF1);
  iowrite32(0x00000000, PWM + PWM_FIF1);
  iowrite32(0x00000000, PWM + PWM_FIF1);
  iowrite32(0x00000000, PWM + PWM_FIF1);
  iowrite32(0x00000000, PWM + PWM_FIF1);

  // configure PWM channel to send data serially out of the FIFO
  iowrite32(PWM_CTL_MODE1 | PWM_CTL_USEF1 | PWM_CTL_PWEN1, PWM + PWM_CTL);
  // configure GPIO pin to the correct function for PWM output
  gpio_config(PWM0_PIN_NUM, PWM0_ALT_FUN);
}


/*
 * waits for any current DMA operation to complete then resets the DMA module
 */
static void dma_stop(void) {
  printk(KERN_INFO "%s: (dma_stop) (begin) DMA5_CS = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CS));
  // wait until any current DMA operation completes
  while ((ioread32(DMA5 + DMA5_CS) & DMA5_CS_ACTIVE) && !(ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR)) {
    printk(KERN_INFO "%s: (dma_stop) DMA operation in progress DMA5_CS = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CS));
    udelay(10);
  }
  // check for errors
  if (ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR) {
    printk(KERN_ALERT "%s: (dma_stop) DMA ERROR 0x%x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_DEBUG) & 0x7);
  }
  // reset the DMA module
  iowrite32(DMA5_CS_RESET, DMA5 + DMA5_CS);
  udelay(10);
  // clear old status flags
  iowrite32(DMA5_CS_END | DMA5_CS_INT, DMA5 + DMA5_CS);
  udelay(10);
  printk(KERN_INFO "%s: (dma_stop) (end) DMA5_CS = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CS));
  // clear old error flags
  iowrite32(DMA5_DEBUG_READ_ERROR | DMA5_DEBUG_FIFO_ERROR | DMA5_DEBUG_READ_LAST_NOT_SET_ERROR, DMA5 + DMA5_DEBUG);
}


/*
 * takes a complete control block and issues it to the DMA module
 */
static void dma_start(void) {
  printk(KERN_INFO "%s: (dma_start) (begin) DMA5_CONBLK_AD = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CONBLK_AD));
  printk(KERN_INFO "%s: (dma_start) (begin) DMA5_CS = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CS));
  // set the new DMA control block physical address
  iowrite32(virt_to_phys(&dma_cb), DMA5 + DMA5_CONBLK_AD);
  // begin the DMA transfer with max AXI priority (15)
  iowrite32(DMA5_CS_WAIT_OUTSTANDING_WRITES | DMA5_CS_PANIC_PRIORITY(15) | DMA5_CS_PRIORITY(15), DMA5 + DMA5_CS);
  printk(KERN_INFO "%s: (dma_start) (end) DMA5_CONBLK_AD = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CONBLK_AD));
  printk(KERN_INFO "%s: (dma_start) (end) DMA5_CS = 0x%08x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_CS));
  iowrite32(ioread32(DMA5 + DMA5_CS) |  DMA5_CS_ACTIVE, DMA5 + DMA5_CS);
}


int hal_init(void) {
  // char *kbuf;
  // uint32_t len;
  // map external IO
  CM = (volatile uint32_t *)ioremap(CM_BASE, CM_SIZE);
  PWM = (volatile uint32_t *)ioremap(PWM_BASE, PWM_SIZE);
  DMA5 = (volatile uint32_t *)ioremap(DMA5_BASE, DMA5_SIZE);
  GPIO = (volatile uint32_t *)ioremap(GPIO_BASE, GPIO_SIZE);
  // start the PWM generation (with no FIFO data)
  pwm_start();
  // printk(KERN_INFO "%s: (hal_init) control block P address = 0x%08x\n", DRIVER_NAME, virt_to_phys(&dma_cb));
  // printk(KERN_INFO "%s: (hal_init) control block V address = 0x%08x\n", DRIVER_NAME, (uint32_t)(&dma_cb));
  // // find out how many bytes are needed in the buffer
  // len = ROUND_UP((num_pixels * PIXEL_DATA_LEN * 3) + PIXEL_RESET_BYTE_PADDING, sizeof(uint32_t));
  // printk(KERN_INFO "%s: (hal_init) internal buffer length = %d bytes\n", DRIVER_NAME, len);
  // // allocate the buffer needed for streaming user data to the PWM module
  // kbuf = (char *)kmalloc(len, GFP_KERNEL);
  // if (IS_ERR(kbuf)) {
  //   printk(KERN_ALERT "%s: (hal_init) kmalloc error 0x%p\n", DRIVER_NAME, kbuf);
  //   return -ENOMEM;
  // }
  // // store the physical address of the empty PWM buffer into the DMA control block
  // dma_cb.source_ad = virt_to_phys(kbuf);
  // printk(KERN_INFO "%s: (hal_init) source P address = 0x%08x\n", DRIVER_NAME, dma_cb.source_ad);
  // printk(KERN_INFO "%s: (hal_init) source V address = 0x%08x\n", DRIVER_NAME, (uint32_t)(kbuf));
  // // set the destination address to be the PWM FIFO
  // dma_cb.dest_ad = 0x7E20C000 + (PWM_FIF1 * sizeof(uint32_t));
  // printk(KERN_INFO "%s: (hal_init) destination P address = 0x%08x\n", DRIVER_NAME, dma_cb.dest_ad);
  // printk(KERN_INFO "%s: (hal_init) destination V address = 0x%08x\n", DRIVER_NAME, (uint32_t)(PWM + PWM_FIF1));
  // // set the total number of bytes to transfer
  // dma_cb.txfr_len = len;
  // // configure DMA control block transfer info for:
  // // - 32 bit transfers to peripheral 5 (PWM)
  // // - increment source address after each transfer
  // // - wait for response before next transfer (use destination DREQ)
  // dma_cb.ti = DMA5_TI_NO_WIDE_BURSTS | DMA5_TI_PERMAP(5) | DMA5_TI_SRC_INC | DMA5_TI_DEST_DREQ | DMA5_TI_WAIT_RESP;
  // printk(KERN_INFO "%s: (hal_init) transfer info = 0x%08x\n", DRIVER_NAME, dma_cb.ti);
  // // no 2D stride and make sure there is no other chained control block
  // dma_cb.stride = 0;
  // dma_cb.nextconbk = 0;
  return 0;
}


void hal_render(char *buf, size_t len) {
  int i, j;
  char *kbuf = (char *)phys_to_virt(dma_cb.source_ad);
  // convert the user buffer into the PWM buffer
  j = 0;
  for (i = 0, j = 0; i < len; i++, j+=3) {
    kbuf[j] = (buf[i] & (1 << 7)) ? (PIXEL_1 << 5) : (PIXEL_0 << 5);
    kbuf[j] |= (buf[i] & (1 << 6)) ? (PIXEL_1 << 2) : (PIXEL_0 << 2);
    kbuf[j] |= (buf[i] & (1 << 5)) ? (PIXEL_1 >> 1) : (PIXEL_0 >> 1);
    kbuf[j + 1] = (buf[i] & (1 << 5)) ? ((PIXEL_1 & 0x1) << 7) : ((PIXEL_0 & 0x1) << 7);
    kbuf[j + 1] |= (buf[i] & (1 << 4)) ? (PIXEL_1 << 4) : (PIXEL_0 << 4);
    kbuf[j + 1] |= (buf[i] & (1 << 3)) ? (PIXEL_1 << 1) : (PIXEL_0 << 1);
    kbuf[j + 1] |= (buf[i] & (1 << 2)) ? (PIXEL_1 >> 2) : (PIXEL_0 >> 2);
    kbuf[j + 2] = (buf[i] & (1 << 2)) ? ((PIXEL_1 & 0x3) << 6) : ((PIXEL_0 & 0x3) << 6);
    kbuf[j + 2] |= (buf[i] & (1 << 1)) ? (PIXEL_1 << 3) : (PIXEL_0 << 3);
    kbuf[j + 2] |= (buf[i] & (1 << 0)) ? PIXEL_1 : PIXEL_0;
    printk(KERN_INFO "%s: (hal_render) buf[%d] = 0x%02x\n", DRIVER_NAME, i, buf[i]);
    printk(KERN_INFO "%s: (hal_render)   kbuf[%d] = 0x%02x\n", DRIVER_NAME, j, kbuf[j]);
    printk(KERN_INFO "%s: (hal_render)   kbuf[%d] = 0x%02x\n", DRIVER_NAME, j + 1, kbuf[j + 1]);
    printk(KERN_INFO "%s: (hal_render)   kbuf[%d] = 0x%02x\n", DRIVER_NAME, j + 2, kbuf[j + 2]);
  }
  // zero out remaining space for the pixel RESET signal
  memset(kbuf + j, 0, dma_cb.txfr_len - j);
  printk(KERN_INFO "%s: (hal_render) kbuf[%d:%d] = 0x0\n", DRIVER_NAME, j, dma_cb.txfr_len);
  // send the control block to DMA for transfer
  dma_stop();
  dma_start();
}


void hal_cleanup(void) {
  pwm_stop();
  // dma_stop();
  // kfree(phys_to_virt(dma_cb.source_ad));
  iounmap(CM);
  iounmap(PWM);
  iounmap(DMA5);
  iounmap(GPIO);
}
