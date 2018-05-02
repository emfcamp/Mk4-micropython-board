def startup():
    try:
        from machine import SD
        from uos import mount, VfsFat
        import sys

        bdev = SD(0)
        vfs = VfsFat(bdev)
        mount(vfs, '/')
        sys.path.append('/modules')
    except:
        print('error: could not mount FAT filesystem')

    try:
        import network
        from utime import sleep_ms, time

        lan = None
        if hasattr(network, "WLAN"):
            lan = network.WLAN(0)
        elif hasattr(network, "LAN"):
            lan = network.LAN(0)

        if not lan:
            raise OSError("no network module configured")

        lan.active(True)
        start = time()
        while not lan.ifconfig() and time() < start + 5:
            print('.')
            sleep_ms(200)

        print('getting ip') 
        ip = lan.ifconfig()
        print('IP: ' + ip[0])
    except:
        print('error: network not available')

    return

startup()
