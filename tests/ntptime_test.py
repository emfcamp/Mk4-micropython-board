import socket
import struct
from machine import RTC
from time import localtime

NTP_DELTA = 3155673600

host = "pool.ntp.org"
port = 123

def get_ntp_time():
    NTP_QUERY = bytearray(48)
    NTP_QUERY[0] = 0x1b

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    msg = None
    try:
        addr = socket.getaddrinfo("0.0.0.0", 0)[0][-1]
        s.bind(addr)

        addr = socket.getaddrinfo(host, port)[0][-1]
        res = s.sendto(NTP_QUERY, addr)

        msg = s.recv(48)
    except:
        pass

    s.close()

    if msg:
        val = struct.unpack("!I", msg[40:44])[0]
        return val - NTP_DELTA
    else:
        return 0


rtc = RTC()
rtc.init(localtime(get_ntp_time()))
print(localtime())
