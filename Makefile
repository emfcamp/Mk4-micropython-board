include ../../rules.mk
include mpconfigboard.mk

BOARD := $(notdir $(CURDIR))

all: mpex.out

mpex.out: main_tirtos.o mpex.o ndk.o MSP_EXP432E401Y.o ../../build-$(BOARD)/libmicropython.a
	@echo linking ...
	@$(LD) -o $@ $^ $(LFLAGS)

../../build-$(BOARD)/libmicropython.a:
	make -C ../.. BOARD=$(BOARD)

clean:
	@echo cleaning ...
	@rm -f *.o *.out *.map
