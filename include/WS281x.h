/*
 * WS281x.h
 *
 * Constants specific to WS281x LEDs and this kernel module
 *
 * Aaron Reyes
 */

#ifndef _WS281x_H_
#define _WS281x_H_

#define DRIVER_NAME    "ws281x"
#define DRIVER_VERSION "0.1"
#define DRIVER_AUTHOR  "Aaron Reyes"
#define DRIVER_DESC    "A driver for WS281x LEDs"
#define DRIVER_LICENSE "Dual MIT/GPL"

/* signal rate of the pixels */
#define WS281x_RATE 800000 // Hz

/* number of bytes required to program a given pixel */
#define WS281x_DATA_LEN 3 // bytes (for GRB)

/* number of bytes for the RESET signal (55us) */
// takes the number of bytes used per pixel by the driver internals
#define WS281x_RESET_PADDING(x) (((55 * (x) * WS281x_RATE) / 1000000) >> 3) // bytes

/* number of WS281x LEDs currently under control */
extern int num_leds;

/* GPIO pin to use as output */
extern int pin_num;

/* GPIO pin alternate function to use */
extern int pin_fun;

#endif /* _WS281x_H_ */