# simple script for auto testing of the kernel module

import subprocess

num_pixels = 1
pin_num = 18
pin_fun = 5

subprocess.call(["insmod", "neopixel.ko",
                           "num_pixels={0}".format(num_pixels),
                           "pin_num={0}".format(pin_num),
                           "pin_fun={0}".format(pin_fun)])

f = open('/dev/neopixel', 'r+')
f.write("\xFF\x00\x00" * num_pixels) # GRB
f.close()

subprocess.call(["rmmod", "neopixel"])

print "dmesg: \n", subprocess.Popen("dmesg | tail -40", shell=True, stdout=subprocess.PIPE).stdout.read()
