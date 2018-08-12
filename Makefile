BOARD ?= MSP_EXP432E401Y
BUILD ?= build-$(BOARD)

include ../../py/mkenv.mk
-include mpconfigport.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# directory containing scripts to be frozen as bytecode
FROZEN_MPY_DIR ?= modules

# include py core make definitions
include $(TOP)/py/py.mk

# include board-specific make definitions
-include $(BUILD)/defs.mk
include boards/$(BOARD)/mpconfigboard.mk

SRC_C = mpmain.c \
	modmachine.c \
	modsocket.c \
	modnetwork.c \
	network_ndklan.c \
	network_stalan.c \
	moduos.c \
	modutime.c \
	machine_adc.c \
	machine_i2c.c \
	machine_pin.c \
	machine_sd.c \
	machine_spi.c \
	machine_uart.c \
	machine_nvsbdev.c \
	machine_pwm.c \
	machine_rtc.c \
	led.c \
	storage.c \
	fatfs_port.c \
	lib/utils/printf.c \
	lib/utils/stdout_helpers.c \
	lib/utils/pyexec.c \
	lib/libc/string0.c \
	lib/oofatfs/ff.c lib/oofatfs/option/unicode.c \
	lib/timeutils/timeutils.c \
	lib/netutils/netutils.c \
	lib/mp-readline/readline.c \
	lib/utils/sys_stdio_mphal.c

SRC_EXTRA =

# for uGFX driver module
ifeq ($(MICROPY_PY_UGFX),1)
CFLAGS += -DMICROPY_PY_UGFX=1
GFXLIB=./extmod/ugfx
include $(GFXLIB)/gfx.mk
include $(GFXLIB)/drivers/gdisp/ILI9341/driver.mk
INC += $(foreach d, $(GFXINC), -I$d)
INC += -I./modugfx
SRC_C += modugfx.c
SRC_UGFX += $(GFXSRC)
SRC_C += ./modugfx/ugfx_widgets.c
SRC_C += ./modugfx/ugfx_containers.c
SRC_C += ./modugfx/ugfx_styles.c
## Add Toggle driver
SRC_UGFX += ./modugfx/ugfx_ginput_lld_toggle.c
endif

HDR_QSTR = machine_nvsbdev.h machine_sd.h

INC += -I.
INC += -I$(TOP)
INC += -Iboards/$(BOARD)
INC += -I$(BUILD)

CFLAGS += $(INC) $(CFLAGS_MOD) $(CFLAGS_EXTRA)

SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C) $(HDR_QSTR)

OBJ += $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_EXTRA:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_UGFX:.c=.o))

all: $(BUILD)/libmicropython.a
	$(ECHO) building $(BOARD) ...
	$(Q)make BUILD=$(BUILD) -C boards/$(BOARD)

targets:
	$(ECHO) "Usage: make BOARD=<board>"
	$(Q)ls -1 boards

./tools:
	$(error "TI SimpleLink tools not installed - run: ./inst_tools $(BOARD)")

flash-jlink: all
	$(ECHO) ====== Flashing with JLinkExe ========
	$(ECHO) "r\nh\nloadbin boards/$(BOARD)/mpex.bin 0x00\nr\nexit\n" |\
	JLinkExe -if swd -device MSP432E401Y -speed 4000 -autoconnect 1

flash-dfu: all
	cp boards/$(BOARD)/mpex.bin boards/$(BOARD)/mpex.dfu
	dfu-suffix -a boards/$(BOARD)/mpex.dfu -v 0x1cbe -p 0x00ff
	dfu-prefix -s 0x0000 -a boards/$(BOARD)/mpex.dfu
	dfu-util -D boards/$(BOARD)/mpex.dfu


clean: clean-board

clean-board:
	$(ECHO) cleaning $(BOARD) ...
	$(Q)make BUILD=$(BUILD) -C boards/$(BOARD) clean

$(BUILD)/libmicropython.a: $(OBJ)
	$(ECHO) AR $@ ...
	$(Q)$(AR) r $@ $^

$(BUILD)/defs.mk: ./tools
	$(Q)mkdir -p $(BUILD)
	./gen_defs ./tools > $@

include $(TOP)/py/mkrules.mk
