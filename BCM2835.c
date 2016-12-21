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

#include <linux/kernel.h>  /* for printk */
#include <linux/delay.h>   /* for udelay() */
#include <asm/io.h>        /* for read/write memory barrier operations */

#include "BCM2835.h"      /* for hardware specific addresses */
#include "neopixel.h"     /* for WS2812_RATE */

/* GPIO pin numbers for PWM output */
#define PWM0_PIN_NUM 18
#define PWM0_PIN_FUN 5

/* structure pointers for MMIO operations */
static volatile uint32_t *CM;
static volatile uint32_t *PWM;
static volatile uint32_t *GPIO;


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
  // configure GPIO pin to the correct function for PWM output
  uint32_t mapping[] = {4, 5, 6, 7, 3, 2};
  uint32_t reg = PWM0_PIN_NUM / 10;
  uint32_t config = ioread32(GPIO + reg);
  config &= ~(0x7 << ((PWM0_PIN_NUM % 10) * 3));
  config |= (mapping[PWM0_PIN_FUN] << ((PWM0_PIN_NUM % 10) * 3));
  iowrite32(config, GPIO + reg);

  // stop the clock if it is in use
  stop_pwm();
  // setup the PWM clock manager to 3 x WS2812_RATE
  iowrite32(CM_PWM_DIV_PASSWD | CM_PWM_DIV_DIVI(OSC_FREQ / (3 * WS2812_RATE)), CM + CM_PWM_DIV);
  // source the PWM clock from the oscillator and enable it
  iowrite32(CM_PWM_CTL_PASSWD | CM_PWM_CTL_SRC_OSC | CM_PWM_CTL_ENAB, CM + CM_PWM_CTL);
  // wait for PWM clock manager to settle
  while (!(ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY));

  // configure 32 bit period transfers for both channels
  iowrite32(32, PWM + PWM_RNG1);
  udelay(10);
  iowrite32(32, PWM + PWM_RNG2);
  udelay(10);

  // write some data to send
  iowrite32(0xFFFF << 8, PWM + PWM_DAT1); // purple
  udelay(10);
  iowrite32(0xFFFF << 8, PWM + PWM_DAT2); // purple
  udelay(10);

  // configure both PWM channels to send data serially out of the data register
  iowrite32(PWM_CTL_MODE2 | PWM_CTL_PWEN2 | PWM_CTL_MODE1 | PWM_CTL_PWEN1, PWM + PWM_CTL);
  udelay(10);
  return 0;
}


void stop_pwm(void) {
  // turn off PWM
  iowrite32(0, PWM + PWM_CTL);
  // turn off the clock
  iowrite32(CM_PWM_CTL_PASSWD, CM + CM_PWM_CTL);
  // wait for reset to complete
  while (!(ioread32(CM + CM_PWM_CTL) & CM_PWM_CTL_BUSY));
}

