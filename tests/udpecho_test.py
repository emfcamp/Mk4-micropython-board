#
# run a local UDP echo server to service this client:
#
# ncat -e /bin/cat -k -u -l 1234
#
# adjust server and port to match
#

from socket import socket, AF_INET, SOCK_DGRAM, getaddrinfo
from time import sleep

server = "192.168.10.150"
port = 1234

try:
    from network import LAN
    lan = LAN(0)
    while lan.ifconfig() is None:
        print("waiting for net")
        sleep(1)
except:
    pass

udp = socket(AF_INET, SOCK_DGRAM)
addr = getaddrinfo("0.0.0.0", 0)[0][-1]
udp.bind(addr)

addr = getaddrinfo(server, port)[0][-1]

count = 0
while True:
    sent = udp.sendto(bytes("hello: {}".format(count), "utf-8"), addr)
    print("sent {} bytes".format(sent))
    msg = udp.recvfrom(20)
    print(str(msg[0], "utf-8"))
    sleep(1)
    count = count + 1
