import network
from time import localtime
print("Starting test")

print(localtime())

w = network.WLAN()
w.active(True)

print(w.rtc_now())

w.rtc_set((2018, 5, 26, 11, 43))
print(w.rtc_now())

try:
    w.rtc_set((2018, 6))
    print("error: bogus arguments worked")
except:
    print("pass")
