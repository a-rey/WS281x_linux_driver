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

#include <linux/kernel.h> /* for printk KERN_INFO */
#include <linux/delay.h>  /* for udelay() */
#include <linux/string.h> /* for memset() */
#include <linux/err.h>    /* for error checking functions like IS_ERR() */
#include <linux/gfp.h>    /* for __get_free_pages() */
#include <asm/io.h>       /* for read/write IO operations and virtual/physical translation */
#include <asm/page.h>     /* for PAGE_SIZE */

#include <hal.h>          /* for interface definition and ROUND_UP */
#include <WS281x.h>       /* for WS281x macros and user parameters */
#include <BCM2835.h>      /* for platform specific addresses */

/* DMA control block definition */
struct dma_cb_t {
  uint32_t ti;
  uint32_t source_ad;
  uint32_t dest_ad;
  uint32_t txfr_len;
  uint32_t stride;
  uint32_t nextconbk;
  uint32_t reserved[2];
} __attribute__((packed)); // no padding allowed

/* the DMA control block must be 256 bit (or 32 byte) aligned */
static struct dma_cb_t *dma_cb;

/* internal buffer and its length */
static char *kbuf;
static uint32_t kbuf_len;

/* structure pointers for MMIO operations */
static volatile uint32_t *CM;
static volatile uint32_t *PWM;
static volatile uint32_t *DMA5;
static volatile uint32_t *GPIO;


/*
 * Configures a GPIO pin number to the selected function
 */
static void gpio_config(uint32_t pin, uint32_t fun) {
  // register constant mapping for changing a GPIO's function
  uint32_t gpio_fun[] = {4, 5, 6, 7, 3, 2};
  // get offset into MM GPIO
  uint32_t reg = pin / 10;
  // get contents of correct GPIO_REG_GPFSEL register
  uint32_t config = ioread32(GPIO + reg);
  // get bit offset into GPIO_REG_GPFSEL register and override old config
  uint32_t offset = (pin % 10) * 3;
  config &= ~(0x7 << offset);
  config |= (gpio_fun[fun] << offset);
  iowrite32(config, GPIO + reg);
  udelay(HW_DELAY_US);
  printk(KERN_INFO "%s: (gpio_config) GPIO %d set to alternate function %d\n", DRIVER_NAME, pin, fun);
}


/*
 * stops the PWM clock generator and turns off the PWM module
 */
static void pwm_stop(void) {
  // check if the PWM clock is currently running
  if (ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_ENAB) {
    // turn off PWM
    iowrite32(0, PWM + PWM_CTL);
    udelay(HW_DELAY_US);
    // turn off the clock
    iowrite32((CM_PWM_CTL_PASSWD | ioread32(CM + CM_PWM_CTL)) & ~CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
    udelay(HW_DELAY_US);
    // wait until the module settles
    while (ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY);
  }
}


/*
 * initializes the PWM module/clock manager and configures GPIO for PWM output
 */
static void pwm_start(void) {
  // setup the PWM clock manager to 3 x PIXEL_RATE
  iowrite32(CM_PWM_DIV_PASSWD | CM_PWM_DIV_DIVI(OSC_FREQ / (BYTES_PER_WS281x * WS281x_RATE)), CM + CM_PWM_DIV);
  udelay(HW_DELAY_US);
  // source the PWM clock from the oscillator with no MASH filtering
  iowrite32(CM_PWM_CTL_PASSWD | CM_PWM_CTL_MASH(0) | CM_PWM_CTL_SRC_OSC, CM + CM_PWM_CTL);
  udelay(HW_DELAY_US);
  // enable the PWM clock generator with the same config as above
  iowrite32(CM_PWM_CTL_PASSWD | ioread32(CM + CM_PWM_CTL) | CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
  udelay(HW_DELAY_US);
  // wait until the generator is running
  while (!(ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY));
  // configure 32 bit period transfers
  iowrite32(32, PWM + PWM_RNG1);
  udelay(HW_DELAY_US);
  // clear the FIFO
  iowrite32(PWM_CTL_CLRF1, PWM + PWM_CTL);
  udelay(HW_DELAY_US);
  // enable DMA for PWM with alerts at (PWM_FIFO_SIZE / 2)
  iowrite32(PWM_DMAC_ENAB | PWM_DMAC_PANIC(PWM_FIFO_SIZE / 2) | PWM_DMAC_DREQ(PWM_FIFO_SIZE / 2), PWM + PWM_DMAC);
  udelay(HW_DELAY_US);
  // configure PWM channel to send data serially out of the FIFO
  iowrite32(PWM_CTL_MODE1 | PWM_CTL_USEF1, PWM + PWM_CTL);
  udelay(HW_DELAY_US);
  // enable the PWM module
  iowrite32(ioread32(PWM + PWM_CTL) | PWM_CTL_PWEN1, PWM + PWM_CTL);
  udelay(HW_DELAY_US);
}


/*
 * waits for any current DMA operation to complete then resets the DMA module
 */
static void dma_stop(void) {
  // wait until any current DMA operation completes
  while ((ioread32(DMA5 + DMA5_CS) & DMA5_CS_ACTIVE) && !(ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR)) {
    udelay(HW_DELAY_US);
  }
  // check for errors
  if (ioread32(DMA5 + DMA5_CS) & DMA5_CS_ERROR) {
    printk(KERN_ALERT "%s: (dma_stop) DMA ERROR 0x%x\n", DRIVER_NAME, ioread32(DMA5 + DMA5_DEBUG) & 0x7);
  }
  // reset the DMA module
  iowrite32(DMA5_CS_RESET, DMA5 + DMA5_CS);
  udelay(HW_DELAY_US);
  // clear old status flags
  iowrite32(DMA5_CS_END | DMA5_CS_INT, DMA5 + DMA5_CS);
  udelay(HW_DELAY_US);
  // clear old error flags
  iowrite32(DMA5_DEBUG_READ_ERROR | DMA5_DEBUG_FIFO_ERROR | DMA5_DEBUG_READ_LAST_NOT_SET_ERROR, DMA5 + DMA5_DEBUG);
  udelay(HW_DELAY_US);
}


/*
 * takes a complete control block and issues it to the DMA module
 */
static void dma_start(void) {
  // set the new DMA control block physical address
  iowrite32((uint32_t)virt_to_phys(dma_cb), DMA5 + DMA5_CONBLK_AD);
  udelay(HW_DELAY_US);
  // begin the DMA transfer with max AXI priority (15)
  iowrite32(DMA5_CS_WAIT_OUTSTANDING_WRITES | DMA5_CS_PANIC_PRIORITY(15) | DMA5_CS_PRIORITY(15) | DMA5_CS_ACTIVE, DMA5 + DMA5_CS);
  udelay(HW_DELAY_US);
}


int hal_init(void) {
  // map external IO
  CM = (volatile uint32_t *)ioremap(CM_BASE, CM_SIZE);
  PWM = (volatile uint32_t *)ioremap(PWM_BASE, PWM_SIZE);
  DMA5 = (volatile uint32_t *)ioremap(DMA5_BASE, DMA5_SIZE);
  GPIO = (volatile uint32_t *)ioremap(GPIO_BASE, GPIO_SIZE);
  // allocate space for the control block
  dma_cb = (struct dma_cb_t *)__get_free_pages(GFP_KERNEL, sizeof(struct dma_cb_t) / PAGE_SIZE);
  if (IS_ERR(dma_cb)) {
    printk(KERN_ALERT "%s: (hal_init) __get_free_pages error 0x%p\n", DRIVER_NAME, dma_cb);
    return -ENOMEM;
  }
  // zero out the control block
  memset(dma_cb, 0, sizeof(struct dma_cb_t));
  // find out how many bytes are needed in the buffer
  kbuf_len = ROUND_UP((num_leds * WS281x_DATA_LEN * BYTES_PER_WS281x) + WS281x_RESET_PADDING(BYTES_PER_WS281x), sizeof(uint32_t));
  // allocate the buffer needed for streaming user data to the PWM module
  kbuf = (char *)__get_free_pages(GFP_KERNEL, kbuf_len / PAGE_SIZE);
  if (IS_ERR(kbuf)) {
    printk(KERN_INFO "%s: (hal_init) __get_free_pages error 0x%p\n", DRIVER_NAME, kbuf);
    return -ENOMEM;
  }
  // store the physical address of the empty PWM buffer into the DMA control block
  dma_cb->source_ad = (uint32_t)virt_to_phys(kbuf);
  // set the destination address to be the hardware buss address of the PWM FIFO
  dma_cb->dest_ad = BUS_ADDRESS(PWM_BASE + (PWM_FIF1 * sizeof(uint32_t)));
  // set the total number of bytes to transfer
  dma_cb->txfr_len = kbuf_len;
  // configure DMA control block transfer info for:
  // - 32 bit transfers to peripheral 5 (PWM)
  // - increment source address after each transfer
  // - wait for response before next transfer (use destination DREQ)
  dma_cb->ti = DMA5_TI_NO_WIDE_BURSTS | DMA5_TI_PERMAP(5) | DMA5_TI_SRC_INC | DMA5_TI_DEST_DREQ | DMA5_TI_WAIT_RESP;
  // no 2D stride and make sure there is no other chained control block
  dma_cb->stride = 0;
  dma_cb->nextconbk = 0;
  // stop the clock if it is in use
  pwm_stop();
  // start the PWM generation
  pwm_start();
  // configure GPIO pin to the correct function for PWM output
  gpio_config(pin_num, pin_fun);
  return 0;
}


void hal_render(const char *buf, size_t len) {
  int i, j;
  // wait for any DMA transfer in progress to finish
  dma_stop();
  // convert the user buffer into the PWM buffer
  j = 0;
  for (i = 0, j = 0; i < len; i++, j+=BYTES_PER_WS281x) {
    kbuf[j + 3]  = (buf[i] & (1 << 7)) ? (WS281x_1 << 4) : (WS281x_0 << 4);
    kbuf[j + 3] |= (buf[i] & (1 << 6)) ? WS281x_1 : WS281x_0;
    kbuf[j + 2]  = (buf[i] & (1 << 5)) ? (WS281x_1 << 4) : (WS281x_0 << 4);
    kbuf[j + 2] |= (buf[i] & (1 << 4)) ? WS281x_1 : WS281x_0;
    kbuf[j + 1]  = (buf[i] & (1 << 3)) ? (WS281x_1 << 4) : (WS281x_0 << 4);
    kbuf[j + 1] |= (buf[i] & (1 << 2)) ? WS281x_1 : WS281x_0;
    kbuf[j + 0]  = (buf[i] & (1 << 1)) ? (WS281x_1 << 4) : (WS281x_0 << 4);
    kbuf[j + 0] |= (buf[i] & (1 << 0)) ? WS281x_1 : WS281x_0;
  }
  // zero out remaining space for the WS281x RESET signal
  memset(kbuf + j, 0, (dma_cb->txfr_len) - j);
  // send the control block to DMA for transfer
  dma_start();
}


void hal_cleanup(void) {
  dma_stop();
  pwm_stop();
  free_pages((uint32_t)kbuf, kbuf_len / PAGE_SIZE);
  free_pages((uint32_t)dma_cb, sizeof(struct dma_cb_t) / PAGE_SIZE);
  iounmap(CM);
  iounmap(PWM);
  iounmap(DMA5);
  iounmap(GPIO);
}
