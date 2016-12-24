/*
 * hal.h
 *
 * Hardware abstraction layer (or HAL) used by the driver
 *
 * Aaron Reyes
 */

#ifndef _NEOPIXEL_HAL_H_
#define _NEOPIXEL_HAL_H_

int hal_init(void);

void hal_render(char *buf, size_t len);

void hal_cleanup(void);

#endif /* _NEOPIXEL_HAL_H_ */