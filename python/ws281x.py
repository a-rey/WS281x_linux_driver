# ws281x.py
#
# A wrapper Python class for the ws281x kernel module
#
# Aaron Reyes

import os.path
import subprocess

MODULE_NAME = "ws281x"

# todo: nice error handling to check if module is loaded before trying to load it again

def check_color(x):
  if (type(x) is not int) or (x > 0xFF) or (x < 0 ):
      raise ValueError("Invalid color value: {0}".format(x))


class Color(object):
  """
  Helper class to represent an LED and its colors
  """

  def __init__(self, R, G, B):
    super(Color, self).__init__()
    # check values
    check_color(R)
    check_color(G)
    check_color(B)
    self.R = R
    self.G = G
    self.B = B

  def __str__(self):
    return "R={0} G={1} B={2}".format(self.R, self.G, self.B)

  def serialize(self):
    return "".join([chr(self.G), chr(self.R), chr(self.B)])

  def getGreen(self):
    return self.G

  def setGreen(self, G):
    check_color(G)
    self.G = G

  def getRed(self):
    return self.G

  def setRed(self, R):
    check_color(R)
    self.R = R

  def getBlue(self):
    return self.B

  def setBlue(self, B):
    check_color(B)
    self.B = B



class LEDs(object):
  """
  Simple Python class to abstract the WS281x kernel module into an easy to use package
  """

  def __init__(self, num_leds, pin_num, pin_fun, module_path):
    super(LEDs, self).__init__()
    # check values
    if num_leds <= 0:
      raise ValueError("Invalid number of LEDs: {0}".format(num_leds))
    if not os.path.isfile(module_path):
      raise ValueError("Invalid module path: {0}".format(module_path))
    self.num_leds = num_leds
    self.pin_num = pin_num
    self.pin_fun = pin_fun
    # create internal LED array
    self.leds = [Color(R=0x00, G=0x00, B=0x00) for led in xrange(num_leds)]
    # Load the kernel module and open it
    ret = subprocess.call(["insmod", module_path,
                           "num_leds={0}".format(num_leds),
                           "pin_num={0}".format(pin_num),
                           "pin_fun={0}".format(pin_fun)])
    # check command status
    if ret != 0:
      raise ValueError("Failed to load module")
    self.module = open("/dev/{0}".format(MODULE_NAME), 'r+')

  def __del__(self):
    # close the file and remove the kernel module
    self.module.close()
    subprocess.call(["rmmod", MODULE_NAME])

  def setColor(self, led, color):
    # check values
    if led >= len(self.leds):
      raise ValueError("Invalid led index: {0}".format(led))
    check_color(color.getRed())
    check_color(color.getGreen())
    check_color(color.getBlue())
    # set the correct LED
    self.leds[led] = color

  def getColor(self, led):
    if led >= len(self.leds):
      raise ValueError("Invalid led index: {0}".format(led))
    return self.leds[led]

  def getNumLEDs(self):
    return self.num_leds

  def render(self):
    buf = [led.serialize() for led in self.leds]
    print buf
    self.module.write("".join(buf))

