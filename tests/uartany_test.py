#
# Hook up a FTDI USB-serial adapter to TX, RX, GND on board corresponding to
# desired UART. Open a terminal emulator on the FTDI and follow prompts from
# code below.
#

from machine import UART
from time import sleep

u = UART(1, 9600)

# clear any pending input
u.read()

print(u.any())

print("enter 5 characters (newline counts as a character)")
while True:
    if u.any() == 5:
        break
    sleep(0.1)

print(u.read(5))
print(u.any())
