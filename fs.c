/*
 * fs.c
 *
 * Character device driver interface for the kernel module
 *
 * Aaron Reyes
 */

#include <linux/fs.h>     /* needed for file_operations */
#include <linux/kernel.h> /* needed for printk */

#include "neopixel.h"     /* needed for module info */

static int major_num;     /* the major number for this driver */

/* file system function hooks */
static struct file_operations fops = {
  .read = fs_read,
  .write = fs_write,
  .open = fs_open,
  .release = fs_release
};


int init_fs(void) {
  major_num = register_chrdev(0, DRIVER_NAME, &fops);
  if (major_num < 0) {
    printk(KERN_ALERT "[%s] error in register_chrdev: %d\n", DRIVER_NAME, major_num);
    return major_num;
  }
  return 0;
}


void cleanup_fs(void) {
  int ret = unregister_chrdev(major_num, DRIVER_NAME);
  if (ret < 0) {
    printk(KERN_ALERT "[%s] error in unregister_chrdev: %d\n", DRIVER_NAME, ret);
  }
}


/*
 * Called when a process tries to open the device file
 */
static int fs_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  printk(KERN_INFO "[%s] device opened\n", DRIVER_NAME);
  return 0;
}


/*
 * Called when a process closes the device file
 */
static int fs_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  printk(KERN_INFO "[%s] device closed\n", DRIVER_NAME);
  return 0;
}


/*
 * Called when a process reads from the device file
 *
 * filp - file pointer from include/linux/fs.h
 * buffer - buffer to fill with data for user
 * length - length of that buffer
 * offset - current offset into the file
 *
 * returns the number of bytes read (0 means EOF)
 */
static ssize_t fs_read(struct file *filp, char *buff, size_t len, loff_t * offset) {
  printk(KERN_INFO "[%s] device read %d bytes\n", DRIVER_NAME, len);
  return 0;
}


/*
 * Called when a process writes to the device file
 *
 * filp - file pointer from include/linux/fs.h
 * buffer - buffer with data from the user
 * length - length of that buffer
 * offset - current offset into the file
 *
 * returns the number of bytes written (0 means EOF)
 */
static ssize_t fs_write(struct file *filp, const char *buff, size_t len, loff_t * offset) {
  printk(KERN_INFO "[%s] device write %d bytes\n", DRIVER_NAME, len);
  return len;
}

