/*
 * main.c
 *
 * Kernel module info, start and stop routines
 *
 * Aaron Reyes
 */

#include <linux/module.h>      /* for module_init/exit */
#include <linux/moduleparam.h> /* for module_param */
#include <linux/kernel.h>      /* for printk KERN_INFO */
#include <linux/init.h>        /* for __init/exit */

#include <fs.h>                /* for fs interface */
#include <WS281x.h>            /* for MODULE_* macros and num_leds */

/*
 * module parameter registration
 */
int num_leds;
module_param(num_leds, int, 0);
MODULE_PARM_DESC(num_leds, " Number of WS281x LEDs to control");
int pin_num;
module_param(pin_num, int, 0);
MODULE_PARM_DESC(pin_num, " GPIO pin to set as the PWM output");
int pin_fun;
module_param(pin_fun, int, 0);
MODULE_PARM_DESC(pin_fun, " GPIO pin alternate function");

/*
 * module initialization routine
 */
static int __init init(void) {
  printk(KERN_INFO "%s: (init) initializing with %d WS281x LEDs on GPIO %d\n", DRIVER_NAME, num_leds, pin_num);
  // check the value of num_leds
  if (num_leds <= 0) {
    printk(KERN_ALERT "%s: (init) invalid number of WS281x LEDs %d\n", DRIVER_NAME, num_leds);
    return -1;
  }
  // todo: check the GPIO pin/function to the hardware
  return init_fs();
}


/*
 * module uninitialization routine
 */
static void __exit cleanup(void) {
  printk(KERN_INFO "%s: (cleanup) uninitializing...\n", DRIVER_NAME);
  cleanup_fs();
}


/*
 * module registration
 */
module_init(init);
module_exit(cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_DESCRIPTION(DRIVER_DESC);
