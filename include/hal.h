/*
 * hal.h
 *
 * Hardware abstraction layer (or HAL) used by the driver
 *
 * Aaron Reyes
 */

#ifndef _WS281x_HAL_H_
#define _WS281x_HAL_H_

/* rounds num up to the nearest multiple of div */
#define ROUND_UP(num, div) (num + ((div - (num % div)) % div))

/*
 * initializes the hardware interface. returns -1 on failure or 0 on success.
 */
int hal_init(void);

/*
 * renders the user buffer to the WS281x LEDs using the hardware interface
 *
 * buf - the user supplied buffer copied into kernel memory
 * len - length of buf
 */
void hal_render(const char *buf, size_t len);

/*
 * un-initializes the hardware interface
 */
void hal_cleanup(void);

#endif /* _WS281x_HAL_H_ */