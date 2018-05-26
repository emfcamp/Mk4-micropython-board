from machine import UART

# UART 0 is typically used for REPL
uid = 1

uart = UART(uid, 19200)
print(uart)
uart.deinit()

uart = UART(uid, 9600, 7, 1, 2, timeout=500, read_buf_len=32)
print(uart)
uart.deinit()

uart = UART(uid)
uart.init(38400, 8, None, 1, mode=UART.BINARY)
print(uart)
uart.deinit()

uart.init(57600, 8, UART.EVEN, 1, mode=UART.TEXT)
print(uart)
uart.deinit()

try:
    uart.init(19200, flow=UART.RTS | UART.CTS)
except OSError as fail:
    print("No flow control support")

uart.init(9600)
print(uart)
uart.deinit()
