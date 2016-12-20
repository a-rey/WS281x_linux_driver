# simple script for auto testing of the kernel module

print "return code: ", subprocess.call(["insmod", "../neopixel.ko"])

f = open('/dev/neopixel', 'w')
f.read()
f.write("test")
f.close()

subprocess.call(["rmmod", "neopixel.ko"])

print "dmesg: \n", subprocess.Popen("dmesg | tail -20", shell=True, stdout=subprocess.PIPE).stdout.read()
