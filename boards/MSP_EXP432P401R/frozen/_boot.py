def startup():
    try:
        from machine import NVSBdev as Bdev
        from uos import mount, VfsFat
        import sys

        bdev = Bdev(0)
        vfs = VfsFat(bdev)
        mount(vfs, '/')
        sys.path.append('/modules')
    except:
        print('error: could not mount FAT filesystem')

    return

startup()
