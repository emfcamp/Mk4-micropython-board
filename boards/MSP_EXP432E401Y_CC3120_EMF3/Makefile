include ../../$(BUILD)/defs.mk
include mpconfigboard.mk

CFLAGS += "-I../../$(BUILD)"

BOARD := $(notdir $(CURDIR))

KERNEL_BUILD := ./kernel/build

all: mpex.out mpex.bin boot.bin mpex_with_boot.bin

mpex.out: main_tirtos.o mpex.o SoCMSP432E4.o netwifi.o cc3120_fw.o ndkstub.o \
inetaddr.o epiram.o cdcmsc.o CDCD.o SCMSC.o usbdmsc.o \
MSP_EXP432E401Y.o ../../build-$(BOARD)/libmicropython.a
	@ echo building kernel ...
	@ make -C kernel
	@ echo linking ...
	@ $(LD) -o $@ $^ $(LFLAGS)
	@ $(SIZE) $@

mpex.bin: mpex.out
	@ echo creating downloadable binary
	@ $(GCC_ARMCOMPILER)/arm-none-eabi/bin/objcopy -O binary $^ $@

../../build-$(BOARD)/libmicropython.a:
	@ echo building micropython library
	@ make -C ../.. BOARD=$(BOARD)

boot.bin:
	@ echo building bootloader
	@ echo TODO debug why cant place application at page boundary 0x4000 which fails during ccs loading
	@ make -C boot_serial_usb_flash/gcc BUILD=$(BUILD)
	@ $(GCC_ARMCOMPILER)/arm-none-eabi/bin/objcopy -O binary --gap-fill 0xff --pad-to 0x10000 boot_serial_usb_flash/gcc/boot_serial_usb_flash.out boot.bin

mpex_with_boot.bin: boot.bin mpex.bin
	@ echo making unified image containing bootloader and application
	@ cat boot.bin mpex.bin > mpex_with_boot.bin

clean:
	@ echo cleaning $(BOARD) ...
	@ rm -f *.o *.out *.map *.bin
	@ make -C kernel clean
	@ make -C boot_serial_usb_flash/gcc BUILD=$(BUILD) clean
