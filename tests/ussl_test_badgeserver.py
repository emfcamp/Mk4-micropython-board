#ssl test

import socket
import ussl

print("creating socket")
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("getting address")
addr = socket.getaddrinfo("badgeserver.emfcamp.org", 443)
print("connecting")
sock.connect(addr[0][-1])
#sockssl = ussl.wrap_socket(sock)
sockssl = ussl.wrap_socket(sock, cert_reqs=ussl.CERT_REQUIRED, ca_certs="DST Root CA X3", servercertfile="badgeserver.emfcamp.org")
#Test failure by passing wrong server cert
#sockssl = ussl.wrap_socket(sock, cert_reqs=ussl.CERT_REQUIRED, ca_certs="DST Root CA X3", servercertfile="iot.eclipse.org")
print("writing")
sockssl.write("hello")
print("reading")
print(sockssl.read(5))
print("done")

