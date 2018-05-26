#
# Basic exercise of ADC single-sample interface. Assumes two channels configured
# in board.c
#

from machine import ADC

adc0 = ADC(0)
try:
    adcX = ADC(0)
    print("pass")
except:
    print("error: multiple opens")

try:
    adcX = ADC(99)
    print("error: bogus channel open")
except:
    print("pass")

adc1 = ADC(1)

print(adc0, adc1)

for i in range(10):
    print(adc0.convert(), adc1.convert())
