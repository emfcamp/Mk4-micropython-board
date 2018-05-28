from machine import RTC
from time import localtime

print(localtime())

rtc = RTC()

rtc.init((2018, 5, 26, 11, 43))
print(localtime())
print(rtc.now())

rtc.deinit()
print(localtime())
print(rtc.now())

try:
    rtc.init((2018, 6))
    print("error: bogus arguments worked")
except:
    print("pass")
