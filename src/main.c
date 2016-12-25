/*
 * main.c
 *
 * Kernel module info, start and stop routines
 *
 * Aaron Reyes
 */

#include <linux/module.h>      /* for module_init/exit */
#include <linux/moduleparam.h> /* for module_param */
#include <linux/kernel.h>      /* for printk */
#include <linux/stat.h>        /* for */
#include <linux/init.h>        /* for __init/exit */

#include <fs.h>                /* for fs interface */
#include <neopixel.h>          /* for MODULE_* macros and num_pixels */

/*
 * module parameter registration
 */
int num_pixels;
module_param(num_pixels, int, 0);
MODULE_PARM_DESC(num_pixels, " Number of pixels currently being controlled");

/*
 * module initialization routine
 */
static int __init init(void) {
  printk(KERN_INFO "%s: (*** init ***) initializing with %d pixels...\n", DRIVER_NAME, num_pixels);
  // check the value of num_pixels
  if (num_pixels <= 0) {
    printk(KERN_ALERT "%s: (init) invalid number of pixels %d\n", DRIVER_NAME, num_pixels);
    return -1;
  }
  return init_fs();
}


/*
 * module uninitialization routine
 */
static void __exit cleanup(void) {
  printk(KERN_INFO "%s: (*** cleanup ***) uninitializing...\n", DRIVER_NAME);
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
