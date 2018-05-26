include ../../$(BUILD)/defs.mk
include mpconfigboard.mk

BOARD := $(notdir $(CURDIR))

KERNEL_BUILD := ./kernel/build

CFLAGS += -I../..

all: mpex.out

mpex.out: main_tirtos.o mpex.o ndk.o SoCMSP432E4.o MSP_EXP432E401Y.o ../../build-$(BOARD)/libmicropython.a
	@ echo building kernel ...
	@ make -C kernel
	@ echo linking ...
	@ $(LD) -o $@ $^ $(LFLAGS)

../../build-$(BOARD)/libmicropython.a:
	@ echo building micropython library
	@ make -C ../.. BOARD=$(BOARD)

clean:
	@ echo cleaning $(BOARD) ...
	@ rm -f *.o *.out *.map
	@ make -C kernel clean
