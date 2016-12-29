# wheel.py
#
# example that cycles through a color wheel
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

# load the module and open a file descriptor to it
ws281x_leds = ws281x.LEDs(num_leds=30, pin_num=18, pin_fun=5, module_path="../../ws281x.ko")

# algorithm interpreted from here:
# https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
def cycle(curr_pos):
  next_pos = 255 - (curr_pos & 0xFF)
  if next_pos < 85:
    return ws281x.Color(R=(255 - (next_pos * 3)),
                        G=0,
                        B=(next_pos * 3))
  elif next_pos < 170:
    next_pos -= 85
    return ws281x.Color(R=0,
                        G=(next_pos * 3),
                        B=(255 - (next_pos * 3)))
  else:
    next_pos -= 170
    return ws281x.Color(R=(next_pos * 3),
                        G=(255 - (next_pos * 3)),
                        B=0)

def wheel(delay):
  for x in xrange(255):
    for led in xrange(ws281x_leds.getNumLEDs()):
      ws281x_leds.setColor(led, cycle(x + led))
    ws281x_leds.render()
    sleep(delay)

# call the routine a few times
wheel(0.03) # 30ms delay
wheel(0.03) # 30ms delay
wheel(0.03) # 30ms delay

# unload the kernel module and cleanup
del ws281x_leds
