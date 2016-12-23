/*
 * main.c
 *
 * Kernel module info, start and stop routines
 *
 * Aaron Reyes
 */

#include <linux/module.h> /* for module_init/exit */
#include <linux/kernel.h> /* for printk */
#include <linux/init.h>   /* for __init/exit */

#include <fs.h>           /* for fs interface */
#include <neopixel.h>     /* for MODULE_* macros */


/*
 * module initialization routine
 */
static int __init init(void) {
  printk(KERN_INFO "%s: (*** init ***) initializing...\n", DRIVER_NAME);
  if (init_fs() < 0) {
    return -1;
  }
  return 0;
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
