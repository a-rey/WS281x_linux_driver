/*
 * hal.h
 *
 * Hardware abstraction layer (or HAL) used by the driver
 *
 * Aaron Reyes
 */

#ifndef _NEOPIXEL_HAL_H_
#define _NEOPIXEL_HAL_H_

/*
 * map all required hardware IO addresses into the virtual memory of this module
 */
void map_io(void);

/*
 * unmap all used hardware IO addresses from the virtual memory of this module
 */
void unmap_io(void);

/*
 * starts the PWM signal needed to drive the neopixels
 */
int start_pwm(void);

/*
 * stops the PWM signal needed to drive the neopixels
 */
void stop_pwm(void);

#endif /* _NEOPIXEL_HAL_H_ */