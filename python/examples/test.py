import subprocess
from time import sleep

num_leds = 30
pin_num = 18
pin_fun = 5

subprocess.call(["insmod", "../../ws281x.ko",
                           "num_leds={0}".format(num_leds),
                           "pin_num={0}".format(pin_num),
                           "pin_fun={0}".format(pin_fun)])

for x in xrange(10):
  f = open('/dev/ws281x', 'r+')
  f.write("\xFF\x00\x00" * num_leds)
  f.close()

  sleep(0.5)

  f = open('/dev/ws281x', 'r+')
  f.write("\x00\xFF\x00\xFF\xFF\xFF\x00\x00\xFF" * (num_leds / 3)) # red, white, and blue repeating
  f.close()

  sleep(0.5)

subprocess.call(["rmmod", "ws281x"])