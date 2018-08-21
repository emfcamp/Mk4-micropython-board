import network

w = network.WLAN()
w.active(True)

testdata = bytearray(5)
testdata[0] = 0x1
testdata[1] = 0x2
testdata[2] = 0x3
testdata[3] = 0x4
testdata[4] = 0x5

message = "Hello TestFile, here's some data"

filehandle, token = w.fsopen("testfile.txt", w.FS_WRITE)
print("File Handle : %d, access token %d" % (filehandle, token))

#w.fswrite(fh, 0, testdata)
len = w.fswrite(filehandle, 0, message)

print("Written %d bytes" % len)

#w.fsread(filehandle, 0, 10);

w.fsclose(filehandle)
