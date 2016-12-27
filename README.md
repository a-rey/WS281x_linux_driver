# Rapsberry Pi Adafruit neopixel kernel module
Kernel module to control Adafruit neopixels

## Limitations:

This kernel module uses the PWM hardware, which is also shared by the default audio driver. These cannot be used together. Run `lsmod` and see if `snd_bcm2835` is listed like so:
```
root@raspberrypi:~/driver/module # lsmod
Module                  Size  Used by
8192cu                608727  0
cfg80211              498258  1 8192cu
rfkill                 21277  2 cfg80211
snd_bcm2835            22966  0
snd_pcm                95089  1 snd_bcm2835
snd_timer              22568  1 snd_pcm
snd                    68010  3 snd_bcm2835,snd_timer,snd_pcm
bcm2835_gpiomem         3823  0
bcm2835_wdt             4133  0
uio_pdrv_genirq         3750  0
uio                    10166  1 uio_pdrv_genirq
ipv6                  363633  38
```
If so, we need to blacklist the Broadcom audio kernel module by creating a file `/etc/modprobe.d/snd-blacklist.conf` with the following info in it:
```
# blacklists sound card driver to allow for neopixel driver to use the PWM module without being bothered

blacklist snd_bcm2835
```
Then reboot the device and run `lsmod` to see if `snd_bcm2835` is loaded. If it is still being loaded after blacklisting, you may also need to comment it out in the `/etc/modules` file like so:
```
# /etc/modules: kernel modules to load at boot time.
#
# This file contains the names of kernel modules that should be loaded
# at boot time, one per line. Lines beginning with "#" are ignored.

# snd_bcm2835
```
If audio is needed for your project, you can use a USB audio device instead.

## HW Timing:
The timing interface is described here depending on the Raspberry Pi hardware.

#### BCM2835 (Raspberry Pi 1 model A/B/A+/B+)
The kernel module uses the pi's PWM module in order to generate the signal to control the neopixels. The pixels need to be clocked at 1.25us per bit or 800Kbps. There are different signal representations that the pixels recognize as a 0, 1 or a RESET from the [WS2812 datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812.pdf). They are as follows:

```
+--------+                   +
|        |                   |
|        |                   | = 0
|        |___________________|
  0.35us         0.8us

+------------------+         +
|                  |         |
|                  |         | = 1
|                  |_________|
       0.7us          0.6us

+                            +
|                            |
|                            | = RESET
|____________________________|
            50us

Error bars on all signals are +/- 150ns
```

The timing here looks complicated, but it can be simplified by looking at maximums and minimums of each signal. With this in mind we get the following table:

| Parameter | Min   | Typical | Max  | Units |
| ---------:|:-----:|:-------:|:----:|:-----:|
| 0 high    | 200   |  350    | 500  | ns    |
| 1 high    | 550   |  700    | 850  | ns    |
| 0 low     | 650   |  800    | 950  | ns    |
| 1 low     | 450   |  600    | 750  | ns    |
| RESET     | 50000 |  N/A    | N/A  | ns    |

Running the PWM hardware at 4 * 800Kbps (or 3.2MHz) will give a pulse length of 312.5ns. This will allow for a sequence of `1000` to be a 0 and a sequence of `1100` to be a 1 to the pixels while also fitting inside the bounds for the hardware.

At a lower level, the PWM clock is set by taking the 19.2MHz oscillator clock frequency and dividing it by a divisor called DIVI in the BCM2835 datasheet. The kernel module does not use the MASH filter to reduce jitter so DIVF is ignored and the full equation is as follows: `desired frequency = (oscillator frequency) / DIVI`. Solving this equation, we get that DIVI should be `6`. The PWM module is programmed to send out data in serial mode from the 16 x 32 bit FIFO. This FIFO is then fed data using the DMA module on the pi in order bypass the CPU and avoid the kernel module task from being suspended in the middle of a data transfer to the PWM FIFO that could mess up the timing due to a FIFO underflow. The DMA module operates using a control block data structure that defines a given DMA operation. This control block is then loaded into DMA MMIO and then executed.

### Pin Mappings
- **Raspberry Pi 1 Model A/B**: There are 2 PWM channels and only one (PWM0 through GPIO 18 alt function 5) is available on the pi header.
![Raspberry Pi 1 Model A/B Header](http://elinux.org/images/8/80/Pi-GPIO-header-26-sm.png)
