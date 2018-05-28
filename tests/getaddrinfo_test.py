import socket

print("AF_INET: {} -- AF_INET6: {}".format(socket.AF_INET, socket.AF_INET6))

addrs = socket.getaddrinfo("google.com", 123)
for a in addrs:
    print(a[0], a[3], socket.inet_ntop(socket.AF_INET, socket.sockaddr(a[4])[1]))
    print(a[0], a[3], socket.sockaddr(a[4]))
