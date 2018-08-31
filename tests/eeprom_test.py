from machine import EEPROM

ee = EEPROM()
data = bytearray(16)
for i in range(len(data)):
    data[i] = i

ee.write(data, 64)
rd = ee.read(64, 16)
print(rd)

try:
    ee.write(data, 13)
    print("fail write bad address")
except:
    pass

try:
    ee.read(0, 9)
    print("fail read bad address")
except:
    pass

try:
    ee.read(7, 16)
    print("read size fail")
except:
    pass
