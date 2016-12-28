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

#ifndef _WS281x_BCM2835_H_
#define _WS281x_BCM2835_H_

/* converts an address to a bus address */
#define BUS_ADDRESS(x) (((x) & 0x00FFFFFF) | 0x7E000000)

/* rate of the oscillator crystal is 19.2MHz */
#define OSC_FREQ 19200000 // hz

/* number of bytes that is used to represent a WS281x LED internally */
#define BYTES_PER_WS281x 4

/* constants used to define a 1/0 as seen by the WS281x LED in the PWM buffer */
#define WS281x_1 ((char)0xC) // 1100
#define WS281x_0 ((char)0x8) // 1000

/* hardware timing delay */
#define HW_DELAY_US 10 // microseconds

/* memory mapping specification for GPIO registers */
#define GPIO_BASE 0x20200000 // physical address
#define GPIO_SIZE (40 * sizeof(uint32_t))

/* MMIO offsets for GPIO registers */
#define GPIO_GPFSEL0   0
#define GPIO_GPFSEL1   1
#define GPIO_GPFSEL2   2
#define GPIO_GPFSEL3   3
#define GPIO_GPFSEL4   4
#define GPIO_GPFSEL5   5
#define GPIO_GPSET0    7
#define GPIO_GPSET1    8
#define GPIO_GPCLR0    10
#define GPIO_GPCLR1    11
#define GPIO_GPLEV0    13
#define GPIO_GPLEV1    14
#define GPIO_GPEDS0    16
#define GPIO_GPEDS1    17
#define GPIO_GPREN0    19
#define GPIO_GPREN1    20
#define GPIO_GPFEN0    22
#define GPIO_GPFEN1    23
#define GPIO_GPHEN0    25
#define GPIO_GPHEN1    26
#define GPIO_GPLEN0    28
#define GPIO_GPLEN1    29
#define GPIO_GPAREN0   31
#define GPIO_GPAREN1   32
#define GPIO_GPAFEN0   34
#define GPIO_GPAFEN1   35
#define GPIO_GPPUD     37
#define GPIO_GPPUDCLK0 38
#define GPIO_GPPUDCLK1 39

/* memory mapping specification for clock manager registers */
#define CM_BASE 0x201010A0 // physical address
#define CM_SIZE (2 * sizeof(uint32_t))

/* MMIO offsets for clock manager registers */
#define CM_PWM_CTL 0
#define CM_PWM_DIV 1

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

/* memory mapping specification for PWM registers */
#define PWM_BASE 0x2020C000 // physical address
#define PWM_SIZE (10 * sizeof(uint32_t))

/* PWM FIFO size */
#define PWM_FIFO_SIZE 16

/* MMIO offsets for PWM registers */
#define PWM_CTL  0
#define PWM_STA  1
#define PWM_DMAC 2
#define PWM_RNG1 4
#define PWM_DAT1 5
#define PWM_FIF1 6
#define PWM_RNG2 8
#define PWM_DAT2 9

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

/* memory mapping specification for DMA5 registers (PWM is mapped to DMA channel 5) */
#define DMA5_BASE 0x20007500 // physical address
#define DMA5_SIZE (9 * sizeof(uint32_t))

/* MMIO offsets for DMA registers */
#define DMA5_CS        0
#define DMA5_CONBLK_AD 1
#define DMA5_TI        2
#define DMA5_SOURCE_AD 3
#define DMA5_DEST_AD   4
#define DMA5_TXFR_LEN  5
#define DMA5_STRIDE    6
#define DMA5_NEXTCONBK 7
#define DMA5_DEBUG     8

/* DMA register masks */
#define DMA5_CS_RESET                      (1 << 31)
#define DMA5_CS_ABORT                      (1 << 30)
#define DMA5_CS_DISDEBUG                   (1 << 29)
#define DMA5_CS_WAIT_OUTSTANDING_WRITES    (1 << 28)
#define DMA5_CS_PANIC_PRIORITY(x)          ((x & 0xF) << 20)
#define DMA5_CS_PRIORITY(x)                ((x & 0xF) << 16)
#define DMA5_CS_ERROR                      (1 << 8)
#define DMA5_CS_WAITING_OUTSTANDING_WRITES (1 << 6)
#define DMA5_CS_DREQ_STOPS_DMA             (1 << 5)
#define DMA5_CS_PAUSED                     (1 << 4)
#define DMA5_CS_DREQ                       (1 << 3)
#define DMA5_CS_INT                        (1 << 2)
#define DMA5_CS_END                        (1 << 1)
#define DMA5_CS_ACTIVE                     (1 << 0)
#define DMA5_TI_NO_WIDE_BURSTS             (1 << 26)
#define DMA5_TI_WAITS(x)                   ((x & 0x1F) << 21)
#define DMA5_TI_PERMAP(x)                  ((x & 0x1F) << 16)
#define DMA5_TI_BURST_LENGTH(x)            ((x & 0xF) << 12)
#define DMA5_TI_SRC_IGNORE                 (1 << 11)
#define DMA5_TI_SRC_DREQ                   (1 << 10)
#define DMA5_TI_SRC_WIDTH                  (1 << 9)
#define DMA5_TI_SRC_INC                    (1 << 8)
#define DMA5_TI_DEST_IGNORE                (1 << 7)
#define DMA5_TI_DEST_DREQ                  (1 << 6)
#define DMA5_TI_DEST_WIDTH                 (1 << 5)
#define DMA5_TI_DEST_INC                   (1 << 4)
#define DMA5_TI_WAIT_RESP                  (1 << 3)
#define DMA5_TI_TDMODE                     (1 << 1)
#define DMA5_TI_INTEN                      (1 << 0)
#define DMA5_TXFR_LEN_YLENGTH(x)           ((x & 0xFFFF) << 16)
#define DMA5_TXFR_LEN_XLENGTH(x)           ((x & 0xFFFF) << 0)
#define DMA5_STRIDE_D_STRIDE(x)            ((x & 0xFFFF) << 16)
#define DMA5_STRIDE_S_STRIDE(x)            ((x & 0xFFFF) << 0)
#define DMA5_DEBUG_READ_ERROR              (1 << 2)
#define DMA5_DEBUG_FIFO_ERROR              (1 << 1)
#define DMA5_DEBUG_READ_LAST_NOT_SET_ERROR (1 << 0)

#endif /* _WS281x_BCM2835_H_ */