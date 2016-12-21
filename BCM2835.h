/*
 * BCM2835.h
 *
 * Constants specific to the Raspberry Pi 1 BCM2835 SoC chip
 *
 * References:
 * http://elinux.org/BCM2835_datasheet_errata
 * https://www.scribd.com/doc/127599939/BCM2835-Audio-clocks
 * https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf
 *
 * Aaron Reyes
 */

#ifndef _NEOPIXEL_BCM2835_H_
#define _NEOPIXEL_BCM2835_H_

/* rate of the oscillator crystal is 19.2MHz */
#define OSC_FREQ 19200000 // hz

/* PWM clock manager MMIO registers virtual addresses */
#define CM_PWM_CTL (volatile uint32_t*)(0x7E1010A0)
#define CM_PWM_DIV (volatile uint32_t*)(0x7E1010A4)

/* PWM clock manager register masks */
#define CM_PWM_CTL_PASSWD      (0x5A << 24)
#define CM_PWM_CTL_MASH(x)     ((x & 0x3) << 9)
#define CM_PWM_CTL_FLIP        (1 << 8)
#define CM_PWM_CTL_BUSY        (1 << 7)
#define CM_PWM_CTL_KILL        (1 << 5)
#define CM_PWM_CTL_ENAB        (1 << 4)
#define CM_PWM_CTL_SRC_GND     (0 << 0)
#define CM_PWM_CTL_SRC_OSC     (1 << 0)
#define CM_PWM_CTL_SRC_TSTDBG0 (2 << 0)
#define CM_PWM_CTL_SRC_TSTDBG1 (3 << 0)
#define CM_PWM_CTL_SRC_PLLA    (4 << 0)
#define CM_PWM_CTL_SRC_PLLC    (5 << 0)
#define CM_PWM_CTL_SRC_PLLD    (6 << 0)
#define CM_PWM_CTL_SRC_HDMIAUX (7 << 0)
#define CM_PWM_DIV_PASSWD      (0x5A << 24)
#define CM_PWM_DIV_DIVI(x)     ((x & 0xFFF) << 12)
#define CM_PWM_DIV_DIVF(x)     ((x & 0xFFF) << 0)

/* PWM MMIO registers virtual addresses */
#define PWM_CTL  (volatile uint32_t*)(0x7E20C000)
#define PWM_STA  (volatile uint32_t*)(0x7E20C004)
#define PWM_DMAC (volatile uint32_t*)(0x7E20C008)
#define PWM_RNG1 (volatile uint32_t*)(0x7E20C010)
#define PWM_DAT1 (volatile uint32_t*)(0x7E20C014)
#define PWM_FIF1 (volatile uint32_t*)(0x7E20C018)
#define PWM_RNG2 (volatile uint32_t*)(0x7E20C020)
#define PWM_DAT2 (volatile uint32_t*)(0x7E20C024)

/* PWM register masks */
#define PWM_CTL_MSEN2     (1 << 15)
#define PWM_CTL_USEF2     (1 << 13)
#define PWM_CTL_POLA2     (1 << 12)
#define PWM_CTL_SBIT2     (1 << 11)
#define PWM_CTL_RPTL2     (1 << 10)
#define PWM_CTL_MODE2     (1 << 9)
#define PWM_CTL_PWEN2     (1 << 8)
#define PWM_CTL_MSEN1     (1 << 7)
#define PWM_CTL_CLRF1     (1 << 6)
#define PWM_CTL_USEF1     (1 << 5)
#define PWM_CTL_POLA1     (1 << 4)
#define PWM_CTL_SBIT1     (1 << 3)
#define PWM_CTL_RPTL1     (1 << 2)
#define PWM_CTL_MODE1     (1 << 1)
#define PWM_CTL_PWEN1     (1 << 0)
#define PWM_STA_STA4      (1 << 12)
#define PWM_STA_STA3      (1 << 11)
#define PWM_STA_STA2      (1 << 10)
#define PWM_STA_STA1      (1 << 9)
#define PWM_STA_BERR      (1 << 8)
#define PWM_STA_GAP04     (1 << 7)
#define PWM_STA_GAP03     (1 << 6)
#define PWM_STA_GAP02     (1 << 5)
#define PWM_STA_GAP01     (1 << 4)
#define PWM_STA_RERR1     (1 << 3)
#define PWM_STA_WERR1     (1 << 2)
#define PWM_STA_EMPT1     (1 << 1)
#define PWM_STA_FULL1     (1 << 0)
#define PWM_DMAC_ENAB     (1 << 31)
#define PWM_DMAC_PANIC(x) ((x & 0xFF) << 8)
#define PWM_DMAC_DREQ(x)  ((x & 0xFF) << 0)

/*
 * starts the PWM signal needed to drive the neopixels
 */
int start_pwm(void);

/*
 * stops the PWM signal needed to drive the neopixels
 */
void stop_pwm(void);

#endif /* _NEOPIXEL_BCM2835_H_ */