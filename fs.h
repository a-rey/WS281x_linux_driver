/*
 * fs.h
 *
 * Character device driver interface for the kernel module
 *
 * Aaron Reyes
 */

#ifndef _NEOPIXEL_FS_H_
#define _NEOPIXEL_FS_H_

/*
 * initializes the character device driver interface
 */
int init_fs(void);

/*
 * uninitializes the character device driver interface
 */
void cleanup_fs(void);

#endif /* _NEOPIXEL_FS_H_ */