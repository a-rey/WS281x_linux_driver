# simple script for auto testing of the kernel module

import subprocess

print "return code: ", subprocess.call(["insmod", "neopixel.ko"])

f = open('/dev/neopixel', 'r+')

for x in xrange(1,1000000):
  pass

f.close()

subprocess.call(["rmmod", "neopixel.ko"])

print "dmesg: \n", subprocess.Popen("dmesg | tail -20", shell=True, stdout=subprocess.PIPE).stdout.read()
