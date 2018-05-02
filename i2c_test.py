from machine import I2C

addr = 0x18

i2c = I2C(0)
i2c = I2C(0, baudrate=1000000)
i2c = I2C(0, baudrate=100000)
i2c = I2C(0, baudrate=400000)

if addr not in i2c.scan():
    raise Exception("required BMA222 not found at address {}".format(addr))

i2c.writeto(addr, b'\x00')
cfg = i2c.readfrom(addr, 1)

if cfg[0] != 0xf8:
    raise Exception("BMA222 cfg id not found at address 0: {}".format(cfg[0]))

i2c.writeto(addr, b'\x00')
cfg = bytearray(1)
i2c.readfrom_into(addr, cfg)

if cfg[0] != 0xf8:
    raise Exception("BMA222 cfg id not found at address 0: {}".format(cfg[0]))

user = bytearray(5)
i2c.readfrom_mem_into(addr, 0x38, user)

print("{} {} {} {} {}".format(user[0], user[1], user[2], user[3], user[4]))

for i in range(len(user)):
    user[i] = i * 2

print(user)
i2c.writeto_mem(addr, 0x38, user)

i2c.writeto(addr, b'\x38\x01')
i2c.writeto(addr, b'\x39\x02')

check = i2c.readfrom_mem(addr, 0x38, 1)
print(check)
check = i2c.readfrom_mem(addr, 0x39, 1)
print(check)
