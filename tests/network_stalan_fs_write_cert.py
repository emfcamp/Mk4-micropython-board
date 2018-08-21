import network

w = network.WLAN()
w.active(True)

# TI - dummy-root-ca-cert.pem
certdata_dummy = "\
-----BEGIN CERTIFICATE-----\n\
MIIDyzCCArOgAwIBAgIJAJOqV2LiCvmIMA0GCSqGSIb3DQEBCwUAMHwxCzAJBgNV\n\
BAYTAklMMQ8wDQYDVQQIDAZTaGFyb24xEDAOBgNVBAcMB1JhYW5hbmExHzAdBgNV\n\
BAoMFlRleGFzIEluc3RydW1lbnRzIEluYy4xDDAKBgNVBAsMA1JORDEbMBkGA1UE\n\
AwwSZHVtbXktcm9vdC1jYS1jZXJ0MB4XDTE2MDkxMTEzMTgxMFoXDTI2MDkwOTEz\n\
MTgxMFowfDELMAkGA1UEBhMCSUwxDzANBgNVBAgMBlNoYXJvbjEQMA4GA1UEBwwH\n\
UmFhbmFuYTEfMB0GA1UECgwWVGV4YXMgSW5zdHJ1bWVudHMgSW5jLjEMMAoGA1UE\n\
CwwDUk5EMRswGQYDVQQDDBJkdW1teS1yb290LWNhLWNlcnQwggEiMA0GCSqGSIb3\n\
DQEBAQUAA4IBDwAwggEKAoIBAQC+F2fNjjVABanE6OWnG1wIlc4ODDVul/5W1YCC\n\
i76uEzUOOMBxYTOFkL6b0rJN7Y22orJC22WuGd3admsG5o4MpYXqJKPOnSa8gZyH\n\
1UGDC+Las5jCg1oHSCI/uZ6D767e+6XVO61JCd6xcAv9AWpfVqA1Rte1utRo6U3r\n\
Duo8/lnCn7DSVgTVVyW1+PAvgRQJDh8+iaTO3KcPpJF6ePZy1ko2LxJtkFhJYM5Y\n\
mEhnqYA7HaS7vCSLk037JUju8scyzJJB9TYoP+2Qz83QIxdNGdSkiTCZYjYCOjxu\n\
MZZEtMPiHEkOgBkCYZvvo9RvPCkCRBSTNRuq9PI68jJ7gBjdAgMBAAGjUDBOMB0G\n\
A1UdDgQWBBT9y29z8eQmTlanPieiimw8XsaxMzAfBgNVHSMEGDAWgBT9y29z8eQm\n\
TlanPieiimw8XsaxMzAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA+\n\
xBZzTey9Z17lCaxbOt8DLdhGm37NIXxU8qjLSWsW6CfPQ47xju8aHuy0CFLZmVTl\n\
EsWhqIAZR/JBiofD0fNMGC6w1EsT0HgtPQsoBU40bquQ//oMNxy2gLKhGf631yDc\n\
Hb+w7MWd1pYXAUHZP1upy+9MB4fFQEZrD8fB1mPhZBnMPzgKL/Q2EU4aUWL53Gii\n\
3ZmxWzVcO2FNkdSoXunnzmmu94ZRliUevHRoy9iWmjJrYd7WfNdxyIBuYK8QFZ9l\n\
VV6SfQDzPHoC39ux/AvyJ2AiUnJmGIhb7V+wtzRB328Rn5/rA2u4OJ0ZucN864Ox\n\
zfDcM/moAoyTj0uE8EJe\n\
-----END CERTIFICATE-----\n\
"

# DST Root CA X3.pem
certdata_dst = "\
-----BEGIN CERTIFICATE-----\n\
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n\
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n\
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n\
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n\
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n\
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n\
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n\
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n\
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n\
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n\
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n\
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n\
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n\
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n\
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n\
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n\
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n\
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n\
-----END CERTIFICATE-----\n\
"


# This file can be der or pem, and needs to contain the chain back to the root ca
# doesn't need to include the server cert itself

print("Writing Cert Data - TI Dummy")
print(certdata_dummy)
filehandle, token = w.fsopen("/cert/dummy-root-ca-cert", (w.FS_CREATE | w.FS_OVERWRITE), maxsize=8192 )
print("File Handle : %d, access token %d" % (filehandle, token))
len = w.fswrite(filehandle, 0, certdata_dummy)
print("Written %d bytes" % len)
w.fsclose(filehandle)

print("Writing Cert Data - DST")
print(certdata_dst)
filehandle, token = w.fsopen("/cert/DST Root CA X3", (w.FS_CREATE | w.FS_OVERWRITE), maxsize=8192 )
print("File Handle : %d, access token %d" % (filehandle, token))
len = w.fswrite(filehandle, 0, certdata_dst)
print("Written %d bytes" % len)
w.fsclose(filehandle)


