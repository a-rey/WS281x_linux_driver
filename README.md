# Rapsberry Pi Adafruit neopixel kernel module
Kernel module to control Adafruit neopixels

### Timing
The kernel module uses the pi's PWM module in order to generate the signal to control the neopixels. The pixels need to be clocked at 1.2us or 800Kbps. Also, in each 1.2us long pulse, there are 2 different signal representations that the pixels recognize as a 0 or 1. They are as follows:
```
+-------+                   +
|       |                   |
|       |                   |  = 0
|       |___________________|
  0.35us        0.8us

+------------------+        +
|                  |        |
|                  |        |  = 1
|                  |________|
       0.7us         0.6us
```
So in order to generate these signals, the kernel module clocks the PWM signal at 3 x 800Kbps (= 2.4Mhz) so that a given sequence of `100` is a 0 and a sequence of `110` is a 1 to the pixels.

At a lower level, the PWM clock is set by taking the 19.2MHz oscillator clock frequency and dividing it by a certain divisor called DIVI in the BCM2835 datasheet. The kernel module does not use the MASH filter to reduce jitter so DIVF is ignored and the full equation is as follows: `desired frequency = (oscillator frequency) / DIVI`. Solving this equation, we get that DIVI should be `8`.
