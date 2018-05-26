from machine import UART

class SIM800:
    def __init__(self, uart_id, rst_id):
        self.uart = UART(uart_id, 38400, 8, mode=UART.BINARY, timeout=100)
        self.last = None
        self._do_cmd("AT")
        # no echo; sms text format
        self._do_cmd("ATE0;+CMGF=1")

    def _do_cmd(self, cmd):
        self.uart.write(cmd + "\r\n")
        resp = ""
        while not ("OK\r\n" in resp or "ERROR\r\n" in resp):
            buf = self.uart.read()
            if buf is not None:
                # print("buf" + str(buf, "utf-8"))
                resp = resp + str(buf, "utf-8")

        self.last = cmd + ": " + resp
        return resp.strip("OK\r\n") if  "OK\r\n" in resp else None

    def get_info(self):
        carrier = self._do_cmd("AT+COPS?").split('"')[1]
        rssi = self._do_cmd("AT+CSQ").split(" ")[1].split(",")[0]
        batt = self._do_cmd("AT+CBC").split(" ")[1].split(",")[1]

        return {"carrier": carrier, "rssi": rssi, "battery": batt}

    def send_sms(self, number, msg):
        # don't know why this does not work?
        # return self._do_cmd('AT+CMGS="' + number + '"\r\n' + msg + '\x1a')

        self.uart.write('AT+CMGS="' + number + '"\r\n')
        self.uart.read()
        self.uart.write(msg)
        self.uart.write(b'\x1a')

        buf = None
        while buf is None:
            buf = self.uart.read()

        return "" if b"OK\r\n" in buf else None

    def get_time(self):
        resp = self._do_cmd("AT+CCLK?")
        # +CCLK: "18/05/06,13:01:26-28"
        date, time = resp.split('"')[1].split(",")
        year, month, day = [int(x) for x in date.split("/")]
        hour, minute, second = [int(x[:2]) for x in time.split(":")]

        return (year + 2000, month, day, hour, minute, second)

    def request_time(self, save=False):
        # de-register from network
        self._do_cmd("AT+COPS=2")

        # request time from network at next registration
        self._do_cmd("AT+CLTS=1")
        if save:
            self._do_cmd("AT&W")

        # register with network
        self._do_cmd("AT+COPS=0")

    def debug(self):
        return self.last


def main():
    gsm = SIM800(1, 5)

    print(gsm.get_info())
    year, mon, day, hour, minute, _ = gsm.get_time()

    number = "+447973362384"
    # number = "+18056891691"

    print(gsm.send_sms(number, "hello there from MP SIM800: {}:{}".format(hour, minute)))

if __name__ == "__main__":
    main()
