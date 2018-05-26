#
# Check the time module functions
#

from machine import freq
from os import uname
from time import sleep, sleep_ms, sleep_us, ticks_ms, ticks_us, ticks_cpu

print("machine: {}, freq: {}".format(uname()[4], freq()))

for sleep_fxn, sleep_dur in [(sleep, 1), (sleep_ms, 1000), (sleep_us, 1000000)]:
    for ticks_fxn in [ticks_ms, ticks_us, ticks_cpu]:
        beg_time = ticks_fxn()
        sleep_fxn(sleep_dur)
        end_time = ticks_fxn()
        print("dur: {}, ticks: {}".format(sleep_dur, (end_time - beg_time)))
