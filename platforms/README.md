# Platform implementation notes

### **BCM2835 (Raspberry Pi 1 model [A](https://www.raspberrypi.org/products/model-a/)/[B](https://www.raspberrypi.org/products/model-b/)/[A+](https://www.raspberrypi.org/products/model-a-plus/))**

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

At a lower level, the PWM clock is set by taking the 19.2MHz oscillator clock frequency and dividing it by a divisor called DIVI in the BCM2835 datasheet. The kernel module does not use the MASH filter to reduce jitter so DIVF is ignored and the full equation is as follows: `desired frequency = (oscillator frequency) / DIVI`. Solving this equation, we get that DIVI should be `6`. The PWM module is programmed to send out data in serial mode from the 16 x 32 bit FIFO. This FIFO is then fed data using the DMA module on the pi in order bypass the CPU and avoid the kernel module task from being suspended in the middle of a data transfer to the PWM FIFO that could mess up the timing due to a FIFO underflow. The DMA module operates using a control block data structure that defines a given DMA operation using physical addresses of the source and destination buffers. This control block is then loaded into DMA MMIO and then executed.

Notes on kernel programming:
- `ioread32` and `iowrite32` are used to provide memory barriers for accessing IO
- `__get_free_pages` is used to get physically contiguous pages aligned to the system `PAGE_SIZE` for DMA transfers
- `udelay` is used whenever writing to IO since hardware modules tend to e sensitive (especially the PWM clock manager)
