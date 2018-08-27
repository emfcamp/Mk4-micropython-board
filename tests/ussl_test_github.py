#ssl test

import socket
import ussl

#www.github.com --> cert github.com
#raw.github.com --> cert www.github.com

print("creating socket")
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("getting address")
addr = socket.getaddrinfo("www.github.com", 443)
print("connecting")
sock.connect(addr[0][-1])
#sockssl = ussl.wrap_socket(sock)
sockssl = ussl.wrap_socket(sock, cert_reqs=ussl.CERT_REQUIRED, ca_certs="DigiCert High Assurance EV Root CA", servercertfile="github.com")
print("writing")
sockssl.write("/GET")
print("reading")
print(sockssl.read(5))
print("done")

