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
#define WS2812_RATE    800000 // Hz

#endif /* _NEOPIXEL_H_ */