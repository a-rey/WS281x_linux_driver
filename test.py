# simple script for auto testing of the kernel module

import subprocess

num_leds = 30
pin_num = 18
pin_fun = 5

print "loading..."
subprocess.call(["insmod", "ws281x.ko",
                           "num_leds={0}".format(num_leds),
                           "pin_num={0}".format(pin_num),
                           "pin_fun={0}".format(pin_fun)])
print "opening..."
f = open('/dev/ws281x', 'r+')
print "writing..."
f.write("\x00\x00\xFF\xFF\xFF\xFF" * (num_leds / 2)) # GRB
print "closing..."
f.close()
print "unloading..."
subprocess.call(["rmmod", "ws281x"])

print "dmesg: \n", subprocess.Popen('dmesg | tail -50 | grep "ws281x"', shell=True, stdout=subprocess.PIPE).stdout.read()
