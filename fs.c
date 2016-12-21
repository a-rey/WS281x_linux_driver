/*
 * fs.c
 *
 * Character device driver interface for the kernel module
 *
 * Aaron Reyes
 */

#include <linux/fs.h>     /* for file_operations */
#include <linux/err.h>    /* for error checking functions like IS_ERR() */
#include <linux/device.h> /* for device_create/destroy */
#include <linux/kernel.h> /* for printk */
#include <linux/module.h> /* for get/put_module */

#include "neopixel.h"     /* for module info */
#include "BCM2835.h"      /* for pwm functions */

#define CLASS_NAME "adafruit"

static int major_num;             /* the major number for this driver */
static struct class* class_ptr;   /* device driver class struct pointer */
static struct device* device_ptr; /* device driver device struct pointer */

/* file system function signatures */
static int fs_open(struct inode *inode, struct file *file);
static int fs_release(struct inode *inode, struct file *file);
static ssize_t fs_read(struct file *filp, char *buff, size_t len, loff_t * offset);
static ssize_t fs_write(struct file *filp, const char *buff, size_t len, loff_t * offset);

/* file system function hooks */
static struct file_operations fops = {
  .read = fs_read,
  .write = fs_write,
  .open = fs_open,
  .release = fs_release
};


int init_fs(void) {
  // dynamically get a major number
  major_num = register_chrdev(0, DRIVER_NAME, &fops);
  if (major_num < 0) {
    printk(KERN_ALERT "[%s] error in register_chrdev: %d\n", DRIVER_NAME, major_num);
    return major_num;
  }
  printk(KERN_INFO "[%s] register_chrdev successful with major number %d\n", DRIVER_NAME, major_num);

  // register the device class
  class_ptr = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(class_ptr)) {
    unregister_chrdev(major_num, DRIVER_NAME);
    printk(KERN_ALERT "[%s] error in class_create: %ld\n", DRIVER_NAME, PTR_ERR(class_ptr));
    return PTR_ERR(class_ptr);
  }
  printk(KERN_INFO "[%s] class_create successful\n", DRIVER_NAME);

  // register the device driver
  device_ptr = device_create(class_ptr, NULL, MKDEV(major_num, 0), NULL, DRIVER_NAME);
  if (IS_ERR(device_ptr)) {
    class_destroy(class_ptr);
    unregister_chrdev(major_num, DRIVER_NAME);
    printk(KERN_ALERT "[%s] error in device_create: %ld\n", DRIVER_NAME, PTR_ERR(device_ptr));
    return PTR_ERR(device_ptr);
  }
  printk(KERN_INFO "[%s] device_create successful\n", DRIVER_NAME);
  return 0;
}


void cleanup_fs(void) {
  device_destroy(class_ptr, MKDEV(major_num, 0));
  class_unregister(class_ptr);
  class_destroy(class_ptr);
  unregister_chrdev(major_num, DRIVER_NAME);
}


/*
 * Called when a process tries to open the device file
 */
static int fs_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  start_pwm();
  printk(KERN_INFO "[%s] device opened\n", DRIVER_NAME);
  return 0;
}


/*
 * Called when a process closes the device file
 */
static int fs_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  stop_pwm();
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

