from machine import SPI

spi_id = 1

spi = SPI(spi_id)
print(spi)
spi = SPI(spi_id, 2000000)
print(spi)
spi = SPI(spi_id, 2000000, polarity=0)
print(spi)
spi = SPI(spi_id, 2000000, polarity=0, phase=1)
print(spi)
spi = SPI(spi_id, bits=16, firstbit=SPI.MSB)
print(spi)

spi.init(4000000, polarity=1, phase=0, bits=8, firstbit=SPI.LSB)
print(spi)

buf = bytearray(32)
spi.readinto(buf)
spi.readinto(buf, 0xff)

data = b'01234567'
spi.write(data)

buf = bytearray(8)
spi.write_readinto(data, buf)
