# colorwipe.py
#
# example that fills the LEDs one after the other with a color
#
# Aaron Reyes

# change our python path to allow for the interface ws281x.py
# to be imported from the parent directory
import sys
sys.path.insert(0, '../')

# this will be the delay function for routines
from time import sleep

# import the library ws281x.py
import ws281x

# define some basic colors
red   = ws281x.Color(R=0xFF, G=0x00, B=0x00)
green = ws281x.Color(R=0x00, G=0xFF, B=0x00)
blue  = ws281x.Color(R=0x00, G=0x00, B=0xFF)

# load the module and open a file descriptor to it
ws281x_leds = ws281x.LEDs(num_leds=30, pin_num=18, pin_fun=5, module_path="../../ws281x.ko")

# the color routine
def colorWipe(color, delay):
  for led in xrange(ws281x_leds.getNumLEDs()):
    ws281x_leds.setColor(led, color)
    ws281x_leds.render()
    sleep(delay)

# # call the routine with some colors
# colorWipe(red, 0.05)   # 50ms delay
# colorWipe(green, 0.05) # 50ms delay
# colorWipe(blue, 0.05)  # 50ms delay

# unload the kernel module and cleanup
del ws281x_leds
