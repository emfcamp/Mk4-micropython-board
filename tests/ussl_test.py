#ssl test

import socket
import ussl

print("creating socket")
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("getting address")
addr = socket.getaddrinfo("192.168.3.8", 4433)
print("connecting")
sock.connect(addr[0][-1])
#sockssl = ussl.wrap_socket(sock)
sockssl = ussl.wrap_socket(sock, ca_certs="dummy-root-ca-cert", cert_reqs=ussl.CERT_REQUIRED)
print("writing")
sockssl.write("hello")
print("reading")
print(sockssl.read(5))
print("done")

