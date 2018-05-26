from machine import NVSBdev
nvs = NVSBdev(0)

from os import mount, VfsFat
VfsFat.mkfs(nvs)

vfs = VfsFat(nvs)
mount(vfs, "/")

from os import listdir
print(listdir("/"))

from os import statvfs
print(statvfs("/"))

w = open("foo.txt", "w")
w.write("hello, roger\n")
w.close()

print(listdir("/"))

r = open("foo.txt", "r")
print(r.readline())
r.close()
