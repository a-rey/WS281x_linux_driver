/*
 * neopixel.h
 *
 * Constants specific to Adafruit's neopixel and this kernel module
 *
 * Aaron Reyes
 */

#ifndef _NEOPIXEL_H_
#define _NEOPIXEL_H_

#define DRIVER_NAME    "neopixel"
#define DRIVER_VERSION "0.1"
#define DRIVER_AUTHOR  "Aaron Reyes"
#define DRIVER_DESC    "A driver for Adafruit neopixels"
#define DRIVER_LICENSE "Dual MIT/GPL"

/* signal rate of the pixels */
#define PIXEL_RATE 800000 // Hz

/* number of bytes required to program a given pixel */
#define PIXEL_DATA_LEN 3 // bytes (for GRB)

/* number of bytes for the RESET signal */
// takes the number of bytes used per pixel by the driver internals
#define PIXEL_RESET_PADDING(x) (((55 * (x) * PIXEL_RATE) / 1000000) >> 3) // bytes

/* number of pixels currently under control */
extern int num_pixels;

/* GPIO pin to use as output */
extern int pin_num;

/* GPIO pin function to use */
extern int pin_fun;

#endif /* _NEOPIXEL_H_ */