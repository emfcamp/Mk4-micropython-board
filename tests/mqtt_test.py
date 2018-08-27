from time import sleep

import ussl
import network
w=network.WLAN()
print(w.ifconfig())

#import sys
#sys.path.append("/flash/modules")
from umqtt.simple import MQTTClient

#c = MQTTClient("umqtt_client", "192.168.1.199", ssl=True)
#c = MQTTClient("umqtt_client", "iot.eclipse.org")
#c = MQTTClient("umqtt_client", "iot.eclipse.org", ssl=True)
#c = MQTTClient("umqtt_client", "iot.eclipse.org", ssl=True, ssl_params={"cert_reqs":ussl.CERT_REQUIRED, "ca_certs":"DST Root CA X3"})

# Note servercertfile needs to be in .der format
c = MQTTClient("umqtt_client", "iot.eclipse.org", ssl=True, ssl_params={"cert_reqs":ussl.CERT_REQUIRED, "ca_certs":"DST Root CA X3", "servercertfile":"iot.eclipse.org"})

#, user="user", password="password")
print("Connecting")
c.connect()
print("connected")

i = 0
 
while 1:
   print("Sending") 
   i = i+1
   msg = str(i)
   print(msg)
   c.publish("count", msg)
   sleep(1)
