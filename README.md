# WS281x kernel module
Linux kernel module to control WS281x LEDs

### Usage
Bare metal usage is the following:

```
> insmod ws281x.ko num_leds=<0> pin_num=<1> pin_fun=<2>
```

Parameter descriptions are the following:

```
num_leds: Number of WS281x LEDs to control (int)
pin_num: GPIO pin to set as the PWM output (int)
pin_fun: GPIO pin alternate function (int)
```

Example usage for Raspberry Pi 1 model A/B/A+ using BCM2835 SoC chip hardware with a strand of 30 WS281x LEDs:

```
> insmod ws281x.ko num_leds=30 pin_num=18 pin_fun=5
```

## Limitations

#### **BCM2835 (Raspberry Pi 1 model [A](https://www.raspberrypi.org/products/model-a/)/[B](https://www.raspberrypi.org/products/model-b/)/[A+](https://www.raspberrypi.org/products/model-a-plus/))**

- Blacklisting Broadcom audio driver

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
  # blacklists sound card driver to allow for WS281x driver to use the PWM module without being bothered

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

- Disable DMA interference for RAM refresh

  Additionally, the GPU will halt the DMA module every 500ms for 16us in order to adjust RAM refresh rate based on its temperature. There is a slight change that this may interfere with timing for intense routines. To prevent this, add `disable_pvt=1` to `/boot/cmdline.txt` and reboot the pi.

## Recommended Pin Mappings
- **Raspberry Pi 1 Model A/B**: There are 2 PWM channels and only one (PWM0 through GPIO 18 alt function 5) is available on the pi header.

  ![Raspberry Pi 1 Model A/B Header](http://elinux.org/images/8/80/Pi-GPIO-header-26-sm.png)
