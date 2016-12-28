# simple script for auto testing of the kernel module

import subprocess

num_pixels = 30
pin_num = 18
pin_fun = 5

print "loading..."
subprocess.call(["insmod", "neopixel.ko",
                           "num_pixels={0}".format(num_pixels),
                           "pin_num={0}".format(pin_num),
                           "pin_fun={0}".format(pin_fun)])
print "opening..."
f = open('/dev/neopixel', 'r+')
print "writing..."
f.write("\x00\xff\xff" * num_pixels) # GRB
print "closing..."
f.close()
print "unloading..."
subprocess.call(["rmmod", "neopixel"])

print "dmesg: \n", subprocess.Popen('dmesg | tail -50 | grep "neopixel"', shell=True, stdout=subprocess.PIPE).stdout.read()
