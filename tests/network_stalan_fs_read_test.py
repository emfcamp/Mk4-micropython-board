import network

w = network.WLAN()
w.active(True)

filehandle, token = w.fsopen("testfile.txt", w.FS_READ)
print("File Handle : %d, access token %d" % (filehandle, token))

data = w.fsread(filehandle, 0, 1024);

print("Read %d bytes" % len(data))
print(data)

w.fsclose(filehandle)

