from network import WLAN
from time import sleep

SSID = "foossid"
PASSWORD = "barpassword"
TIMEOUT = 3

def wait_for(cond, dur):
    while not cond() and dur > 0.0:
        sleep(0.25)
        dur = dur - 0.25

    # print("ret: {} {}".format(cond(), dur))
    return cond()


wlan = WLAN()

wlan.active(True)

if wlan.isconnected():
    wlan.disconnect()

print(wlan.scan())

wlan.connect(SSID, PASSWORD, autoconnect=False)

if wait_for(lambda: wlan.ifconfig() is not None, TIMEOUT):
    print(wlan.ifconfig())
    wlan.disconnect()
    if not wait_for(lambda: wlan.ifconfig() is None, TIMEOUT):
        print("failed to disconnect")
else:
    print("failed to connect")


wlan.connect(SSID, PASSWORD, autoconnect=True)
if wait_for(wlan.isconnected, TIMEOUT):
    print(wlan.ifconfig())

wlan.disconnect()

wlan.connect(SSID, PASSWORD, autoconnect=False)
if wait_for(wlan.isconnected, TIMEOUT):
    print(wlan.ifconfig())

wlan.disconnect()
