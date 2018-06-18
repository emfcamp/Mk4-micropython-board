include ../../$(BUILD)/defs.mk
include mpconfigboard.mk

BOARD := $(notdir $(CURDIR))

KERNEL_BUILD := ./kernel/build

all: mpex.out mpex.bin

mpex.out: main_tirtos.o mpex.o netwifi.o ndkstub.o inetaddr.o epiram.o usb.o usbd_cdc.o MSP_EXP432E401Y.o ../../build-$(BOARD)/libmicropython.a
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

clean:
	@ echo cleaning $(BOARD) ...
	@ rm -f *.o *.out *.map *.bin
	@ make -C kernel clean
