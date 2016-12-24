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
#include <asm/io.h>       /* for read/write memory barrier operations */

#include <hal.h>          /* for interface definition */
#include <BCM2835.h>      /* for hardware specific addresses */
#include <neopixel.h>     /* for PIXEL_RATE, num_pixels, PIXEL_DATA_LEN */

/* number of bytes needed to send out for a 55us reset period */
#define PIXEL_RESET_BYTE_PADDING (((55 * 3 * PIXEL_RATE) / 1000000) >> 3)

/* rounds num up to the nearest multiple of div */
#define ROUND_UP(num, div) (num + ((div - (num % div)) % div))

/* constants used to define a 1/0 as seen by the pixel in the PWM buffer */
#define PIXEL_1 110
#define PIXEL_0 100

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
  printk(KERN_INFO "%s: (pwm_stop) before: CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
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
  printk(KERN_INFO "%s: (pwm_stop) after: CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
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
  // enable DMA with PWM
  iowrite32(PWM_DMAC_ENAB | PWM_DMAC_PANIC(7) | PWM_DMAC_DREQ(3), PWM + PWM_DMAC);
  // configure PWM channel to send data serially out of the FIFO
  iowrite32(PWM_CTL_MODE1 | PWM_CTL_USEF1 | PWM_CTL_PWEN1, PWM + PWM_CTL);
  // configure GPIO pin to the correct function for PWM output
  gpio_config(PWM0_PIN_NUM, PWM0_ALT_FUN);
}


/*
 * waits for any current DMA operation to complete then resets the DMA module
 */
static void dma_stop(void) {
  // wait until any current DMA operation completes
  while ((ioread32(DMA5 + DMA5_CS) & DMA5_CS_ACTIVE) && !(ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR)) {
    udelay(10);
  }
  // check for errors
  if (ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR) {
    printk(KERN_ALERT "%s: (dma_stop) DMA ERROR 0x%x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_DEBUG) & 0x7);
  }
  // reset the DMA module
  iowrite32(DMA5_CS_RESET, DMA5 + DMA5_CS);
  udelay(10);
  // clear any old status flags
  iowrite32(DMA5_CS_END | DMA5_CS_INT, DMA5 + DMA5_CS);
  udelay(10);
  // clear old error flags
  iowrite32(DMA5_DEBUG_READ_ERROR | DMA5_DEBUG_FIFO_ERROR | DMA5_DEBUG_READ_LAST_NOT_SET_ERROR, DMA5 + DMA5_DEBUG);
}


/*
 * takes a complete control block and issues it to the DMA module
 */
static void dma_start(void) {
  // set the new DMA control block address
  iowrite32((uint32_t)(&dma_cb), DMA5 + DMA5_CONBLK_AD);
  // begin the DMA transfer with max AXI priority
  iowrite32(DMA5_CS_WAIT_OUTSTANDING_WRITES | DMA5_CS_PANIC_PRIORITY(15) | DMA5_CS_PRIORITY(15) | DMA5_CS_ACTIVE, DMA5 + DMA5_CS);
}


int hal_init(void) {
  char *kbuf;
  uint32_t len;
  // map external IO
  CM = (volatile uint32_t *)ioremap(CM_BASE, CM_SIZE);
  PWM = (volatile uint32_t *)ioremap(PWM_BASE, PWM_SIZE);
  DMA5 = (volatile uint32_t *)ioremap(DMA5_BASE, DMA5_SIZE);
  GPIO = (volatile uint32_t *)ioremap(GPIO_BASE, GPIO_SIZE);
  // start the PWM generation (with no FIFO data)
  pwm_start();
  // find out how many bytes are needed in the buffer
  len = ROUND_UP((num_pixels * PIXEL_DATA_LEN * 3) + PIXEL_RESET_BYTE_PADDING, sizeof(uint32_t));
  // allocate the buffer needed for streaming user data to the PWM module
  kbuf = (char *)kmalloc(len, GFP_KERNEL);
  if (IS_ERR(kbuf)) {
    printk(KERN_ALERT "%s: (hal_init) kmalloc error 0x%p\n", DRIVER_NAME, kbuf);
    return -ENOMEM;
  }
  // store the empty PWM buffer into the DMA control block
  dma_cb.source_ad = (uint32_t)kbuf;
  // set the destination address to be the PWM FIFO
  dma_cb.dest_ad = (uint32_t)(PWM + PWM_FIF1);
  // set the total number of bytes to transfer
  dma_cb.txfr_len = len;
  // configure DMA control block transfer info for:
  // - 32 bit transfers to peripheral 5 (PWM)
  // - increment source address after each transfer
  // - wait for response before next transfer (use destination DREQ)
  dma_cb.ti = DMA5_TI_NO_WIDE_BURSTS | DMA5_TI_PERMAP(5) | DMA5_TI_SRC_INC | DMA5_TI_DEST_DREQ | DMA5_TI_WAIT_RESP;
  // no 2D stride and make sure there is no other chained control block
  dma_cb.stride = 0;
  dma_cb.nextconbk = 0;
  return 0;
}


void hal_render(char *buf, size_t len) {
  int i, j;
  char *kbuf = (char *)dma_cb.source_ad;
  // copy the buffer translation into the control block source buffer
  for (i = 0, j = 0; i < len; i++, j+=3) {
    kbuf[j] = buf[i] ? PIXEL_1 : PIXEL_0;
  }
  // set the rest of the source buffer to 0
  memset(kbuf + (j - 3), 0, dma_cb.txfr_len - (j - 3));
  // send the control block to DMA for transfer
  dma_stop();
  dma_start();
}


void hal_cleanup(void) {
  pwm_stop();
  dma_stop();
  kfree((char *)(dma_cb.source_ad));
  iounmap(CM);
  iounmap(PWM);
  iounmap(DMA5);
  iounmap(GPIO);
}
