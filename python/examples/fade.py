# fade.py
#
# fades from alternating colors back and forth
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

# define 2 colors to fade to and from
blue_high = ws281x.Color(R=0x00, G=0x00, B=0xFF)
white_low = ws281x.Color(R=0x01, G=0x01, B=0x01)

# load the module and open a file descriptor to it
ws281x_leds = ws281x.LEDs(num_leds=30, pin_num=18, pin_fun=5, module_path="../../ws281x.ko")

# will fade from color1 to color2 to color1 again
def fade(delay, color1, color2):
  # fade up then back down again
  for x in range(2):
    # for every brightness level
    for y in xrange(254):
      # for each LED
      for led in xrange(ws281x_leds.getNumLEDs()):
        if led % 2 == 0:
          # even is color1
          ws281x_leds.setColor(led, color1)
        else:
          # odd is color2
          ws281x_leds.setColor(led, color2)
      ws281x_leds.render()
      sleep(delay)
      if x == 0:
        color1.setRed(0x00 if (color1.getRed() == 0x00) else (color1.getRed() - 1))
        color1.setGreen(0x00 if (color1.getGreen() == 0x00) else (color1.getGreen() - 1))
        color1.setBlue(0x00 if (color1.getBlue() == 0x00) else (color1.getBlue() - 1))
        color2.setRed((color2.getRed() + 1) if (color2.getRed() > 0x00) else 0x00)
        color2.setGreen((color2.getGreen() + 1) if (color2.getGreen() > 0x00) else 0x00)
        color2.setBlue((color2.getBlue() + 1) if (color2.getBlue() > 0x00) else 0x00)
      else:
        color1.setRed((color1.getRed() + 1) if (color1.getRed() > 0x00) else 0x00)
        color1.setGreen((color1.getGreen() + 1) if (color1.getGreen() > 0x00) else 0x00)
        color1.setBlue((color1.getBlue() + 1) if (color1.getBlue() > 0x00) else 0x00)
        color2.setRed(0x00 if (color2.getRed() == 0x00) else (color2.getRed() - 1))
        color2.setGreen(0x00 if (color2.getGreen() == 0x00) else (color2.getGreen() - 1))
        color2.setBlue(0x00 if (color2.getBlue() == 0x00) else (color2.getBlue() - 1))

# call the routine and fade back and forth a few times
fade(0.01, blue_high, white_low) # 10ms delay
fade(0.01, blue_high, white_low) # 10ms delay
fade(0.01, blue_high, white_low) # 10ms delay

# unload the kernel module and cleanup
del ws281x_leds
