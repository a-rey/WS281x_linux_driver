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

#include <linux/io.h>    /* for read/write memory barrier operations */
#include <linux/delay.h> /* for udelay() */

#include "BCM2835.h"     /* for hardware specific addresses */
#include "neopixel.h"    /* for WS2812_RATE */

/* GPIO pin numbers for PWM output */
#define PWM0_PIN_NUM 12
#define PWM1_PIN_NUM 13

/* base of the GPIO register array */
#define GPIO_BASE  (volatile uint32_t*)(0x7E200000)

/* values for fun argument to gpio_config() */
#define GPIO_FUN_ALT0 4 /* GPIO Function Select 0 */
#define GPIO_FUN_ALT1 5 /* GPIO Function Select 1 */
#define GPIO_FUN_ALT2 6 /* GPIO Function Select 2 */
#define GPIO_FUN_ALT3 7 /* GPIO Function Select 3 */
#define GPIO_FUN_ALT4 3 /* GPIO Function Select 4 */
#define GPIO_FUN_ALT5 2 /* GPIO Function Select 5 */

/*
 * Called to configure a GPIO pin to a certain function
 *
 * pin - the GPIO pin number to set
 * fun - the function for the GPIO pin
 */
static void gpio_config(uint32_t pin, uint32_t fun) {
  // get the offset into memory mapped GPIO
  uint32_t reg = pin / 10;
  // get contents of correct GPIO_REG_GPFSEL register
  uint32_t config = readl(GPIO_BASE + (4 * reg));
  // get the bit offset into the GPIO_REG_GPFSEL register
  uint32_t offset = (pin % 10) * 3;
  config &= ~(0x7 << offset);
  config |= (fun << offset);
  // write the new config back
  writel(config, GPIO_BASE + (4 * reg));
}


int start_pwm(void) {
  // stop the clock if it is in use
  stop_pwm();
  // setup the PWM clock manager to 3 x WS2812_RATE
  writel(CM_PWM_DIV_PASSWD | CM_PWM_DIV_DIVI(OSC_FREQ / (3 * WS2812_RATE)), CM_PWM_DIV);
  // source the PWM clock from the oscillator and enable it
  writel(CM_PWM_CTL_PASSWD | CM_PWM_CTL_SRC_OSC | CM_PWM_CTL_ENAB, CM_PWM_CTL);
  // wait for PWM clock manager to settle
  while (!(readl(CM_PWM_CTL) & CM_PWM_CTL_BUSY));
  // configure 32 bit period transfers for both channels
  writel(32, PWM_RNG1);
  writel(32, PWM_RNG2);
  // write some data to send
  writel(0x7F, PWM_DAT1); // blue
  writel(0x7F, PWM_DAT2); // blue
  // configure both PWM channels to send data serially out of the data register
  writel(PWM_CTL_MODE2 | PWM_CTL_PWEN2 | PWM_CTL_MODE1 | PWM_CTL_PWEN1, PWM_CTL);
  // configure GPIO pins
  gpio_config(PWM0_PIN_NUM, GPIO_FUN_ALT0);
  gpio_config(PWM1_PIN_NUM, GPIO_FUN_ALT0);
  return 0;
}


void stop_pwm(void) {
  // turn off PWM
  writel(0, PWM_CTL);
  // turn off the clock
  writel(CM_PWM_CTL_PASSWD, CM_PWM_CTL);
  // wait for reset to complete
  while (!(readl(CM_PWM_CTL) & CM_PWM_CTL_BUSY));
}

