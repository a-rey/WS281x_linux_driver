# simple script for auto testing of the kernel module

import subprocess

print "return code: ", subprocess.Popen("insmod neopixel.ko num_pixels=30", shell=True, stdout=subprocess.PIPE).stdout.read()

f = open('/dev/neopixel', 'r+')

f.write("acb" * 30)

f.close()

subprocess.call(["rmmod", "neopixel.ko"])

print "dmesg: \n", subprocess.Popen("dmesg | tail -20", shell=True, stdout=subprocess.PIPE).stdout.read()
