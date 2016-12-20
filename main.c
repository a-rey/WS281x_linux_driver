/*
 * main.c
 *
 * Kernel module info, start and stop routines
 *
 * Aaron Reyes
 */

#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for printk */
#include <linux/init.h>   /* Needed for the macros */

#include "fs.h"           /* needed for fs interface */
#include "neopixel.h"     /* needed for module info */


/*
 * module initialization routine
 */
static int __init init(void) {
  printk(KERN_INFO "[%s] initializing...\n", DRIVER_NAME);
  if (init_fs() < 0) {
    return -1;
  }
  return 0;
}


/*
 * module uninitialization routine
 */
static void __exit cleanup(void) {
  printk(KERN_INFO "[%s] uninitializing...\n", DRIVER_NAME);
  cleanup_fs();
}


/*
 * module registration
 */
module_init(init);
module_exit(cleanup);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
