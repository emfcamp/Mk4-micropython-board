include ../../$(BUILD)/defs.mk
include mpconfigboard.mk

BOARD := $(notdir $(CURDIR))

CFLAGS += -D__SF_DEBUG__=1 -I../..

KERNEL_BUILD := ./kernel/build

all: mpex.out

mpex.out: main_tirtos.o mpex.o inetaddr.o NVSCC32XX.o CC3220SF_LAUNCHXL.o netwifi.o SoCCC32XX.o ../../build-$(BOARD)/libmicropython.a
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
