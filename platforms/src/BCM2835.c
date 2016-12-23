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
#include <asm/io.h>       /* for read/write memory barrier operations */

#include <hal.h>          /* for interface definition */
#include <BCM2835.h>      /* for hardware specific addresses */
#include <neopixel.h>     /* for WS2812_RATE */

/* structure pointers for MMIO operations */
static volatile uint32_t *CM;
static volatile uint32_t *PWM;
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


void map_io(void) {
  CM = (volatile uint32_t *)ioremap(CM_BASE, CM_SIZE);
  PWM = (volatile uint32_t *)ioremap(PWM_BASE, PWM_SIZE);
  GPIO = (volatile uint32_t *)ioremap(GPIO_BASE, GPIO_SIZE);
}


void unmap_io(void) {
  iounmap(CM);
  iounmap(PWM);
  iounmap(GPIO);
}


int start_pwm(void) {
  printk(KERN_INFO "%s: (pwm_start) 1 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // stop the clock if it is in use
  stop_pwm();
  printk(KERN_INFO "%s: (pwm_start) 2 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // setup the PWM clock manager to 3 x WS2812_RATE
  iowrite32(CM_PWM_DIV_PASSWD | CM_PWM_DIV_DIVI(OSC_FREQ / (3 * WS2812_RATE)), CM + CM_PWM_DIV);
  printk(KERN_INFO "%s: (pwm_start) 3 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // source the PWM clock from the oscillator with no MASH filtering BEFORE enabling it
  iowrite32(CM_PWM_CTL_PASSWD | CM_PWM_CTL_MASH(0) | CM_PWM_CTL_SRC_OSC, CM + CM_PWM_CTL);
  printk(KERN_INFO "%s: (pwm_start) 4 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // enable the PWM clock generator with the same config as above
  iowrite32(CM_PWM_CTL_PASSWD | ioread32(CM + CM_PWM_CTL) | CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
  printk(KERN_INFO "%s: (pwm_start) 5 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // wait until the generator is running
  udelay(10);
  while (!(ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY));
  printk(KERN_INFO "%s: (pwm_start) 6 CM_PWM_CTL = 0x%x\n", DRIVER_NAME, ioread32(CM + CM_PWM_CTL));
  // configure 32 bit period transfers
  iowrite32(32, PWM + PWM_RNG1);
  // write some data to send
  iowrite32(0x36db6db6, PWM + PWM_DAT1);
  // configure PWM channel to send data serially out of the data register
  iowrite32(PWM_CTL_MODE1 | PWM_CTL_PWEN1, PWM + PWM_CTL);
  // configure GPIO pin to the correct function for PWM output
  gpio_config(PWM0_PIN_NUM, PWM0_ALT_FUN);
  return 0;
}


void stop_pwm(void) {
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

