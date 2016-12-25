# simple script for auto testing of the kernel module

import binascii
import subprocess

num_pixels = 1

subprocess.call(["insmod", "neopixel.ko", "num_pixels={0}".format(num_pixels)])

f = open('/dev/neopixel', 'r+')
f.write("\xFF\x00\x00" * num_pixels)
f.close()

subprocess.call(["rmmod", "neopixel"])

print "dmesg: \n", subprocess.Popen("dmesg | tail -40", shell=True, stdout=subprocess.PIPE).stdout.read()
