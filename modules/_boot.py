from machine import SD
from uos import mount, VfsFat, remove, listdir
from utime import sleep_ms, time
import sys

def startup():
    try:
        bdev = SD(0)
        vfs = VfsFat(bdev)
        mount(vfs, '/')
        sys.path.append('/modules')
    except:
        print('error: could not mount FAT filesystem')

    try:
        import network

        wifi = network.WLAN(0)
        wifi.active(True)
        start = time()
        while not wifi.ifconfig() and time() < start + 2:
            sleep_ms(200)

        ip = wifi.ifconfig()
        print('IP: ' + ip[0])
    except:
        print('network not available')

    return

def rm(filename):
    remove(filename)

def ls(path='/'):
    return listdir(path)

def cat(filename):
    with open(filename, 'r') as f:
        for l in f:
            print(l, end='')


startup()
