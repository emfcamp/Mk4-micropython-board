from machine import Pin, sleep
from time import sleep_ms

pushes = 0

def count_pushes(index):
    global pushes
    pushes += 1

pin = Pin(0)
pin.irq(count_pushes)

while pushes < 5:
    print(pushes)
    sleep()

while pushes < 10:
    print(pushes)
    sleep_ms(1000)

print("done")
