BOARD ?= MSP_EXP432E401Y_CC3120_EMF3
BUILD ?= build-$(BOARD)

include ../../py/mkenv.mk
-include mpconfigport.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# directory containing scripts to be frozen as bytecode
FROZEN_MPY_DIR = boards/$(BOARD)/frozen

# include py core make definitions
include $(TOP)/py/py.mk

# include board-specific make definitions
-include $(BUILD)/defs.mk
include boards/$(BOARD)/mpconfigboard.mk

FILE2H = $(TOP)/tools/file2h.py

BOOT_PY_SOURCE = init-files/boot.py
BOOTSTRAP_PY_SOURCE = init-files/bootstrap.py
GEN_BOOT_PY_HEADER = $(HEADER_BUILD)/boot.py.h
GEN_BOOTSTRAP_PY_HEADER = $(HEADER_BUILD)/bootstrap.py.h

SRC_C = \
	modmachine.c \
	modsocket.c \
	modnetwork.c \
	modtilda.c \
	modaudio.c \
	neopix.c \
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
	tilda_buttons.c \
	tilda_sensors.c \
	tilda_thread.c \
	$(BOARD_SRC_C) \
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
	lib/utils/sys_stdio_mphal.c \
	lib/utils/interrupt_char.c

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

HDR_QSTR = machine_nvsbdev.h machine_sd.h $(BUILD)/$(BOARD)_qstr.h

INC += -I.
INC += -I$(TOP)
INC += -Iboards/$(BOARD)
INC += -I$(BUILD)

CFLAGS += $(INC) $(CFLAGS_MOD) $(CFLAGS_EXTRA)

SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C) $(HDR_QSTR)

SRC_QSTR_AUTO_DEPS += $(GEN_BOOT_PY_HEADER)
SRC_QSTR_AUTO_DEPS += $(GEN_BOOTSTRAP_PY_HEADER)

OBJ += $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_EXTRA:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_UGFX:.c=.o))

# main.c can't be even preprocessed without $(GEN_CDCINF_HEADER)
mpmain-emf.c: $(GEN_BOOT_PY_HEADER) $(GEN_BOOTSTRAP_PY_HEADER)

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
	$(ECHO) "r\nh\nloadbin boards/$(BOARD)/mpex_with_boot.bin 0x00\nr\nexit\n" |\
	JLinkExe -if swd -device MSP432E401Y -speed 4000 -autoconnect 1

flash-dfu: all
	cp boards/$(BOARD)/mpex_with_boot.bin boards/$(BOARD)/mpex_with_boot.dfu
	dfu-suffix -a boards/$(BOARD)/mpex_with_boot.dfu -v 0x1cbe -p 0x00ff
	dfu-prefix -s 0x0000 -a boards/$(BOARD)/mpex_with_boot.dfu
	dfu-util -D boards/$(BOARD)/mpex_with_boot.dfu

# Need to decide if we need this... - this doesn't update the bootloader
flash-dfu-mpexonly: all
	cp boards/$(BOARD)/mpex.bin boards/$(BOARD)/mpex.dfu
	dfu-suffix -a boards/$(BOARD)/mpex.dfu -v 0x1cbe -p 0x00ff
	dfu-prefix -s 0x10000 -a boards/$(BOARD)/mpex.dfu
	dfu-util -D boards/$(BOARD)/mpex.dfu

xds110-reset:
	$(ECHO) "Resetting Target via XDS110"
	$(ECHO) "Tools install at http://processors.wiki.ti.com/index.php/XDS_Emulation_Software_Package"
	../../../ti/ccs_base/common/uscif/dbgjtag -f @xds110 -Y reset,system=yes

xds110-unlock-msp432e:
	$(ECHO) "Unlocking MSP432E via JTAG unlock sequence from XDS110"
	$(ECHO) "Tools install at http://processors.wiki.ti.com/index.php/XDS_Emulation_Software_Package"
	../../../ti/ccs_base/common/uscif/dbgjtag -f @xds110 -Y unlock,mode=msp432e4

clean: clean-board

clean-board:
	$(ECHO) cleaning $(BOARD) ...
	$(Q)make BUILD=$(BUILD) -C boards/$(BOARD) clean

$(OBJ): $(BUILD)/$(BOARD)_qstr.h

$(BUILD)/$(BOARD)_qstr.h: boards/$(BOARD)/$(BOARD).csv
	./idgen.py -o $(BUILD)/$(BOARD) -p $(BOARD) $^

$(BUILD)/libmicropython.a: $(OBJ)
	$(ECHO) AR $@ ...
	$(Q)$(AR) r $@ $^

$(BUILD)/defs.mk: ./tools
	$(Q)mkdir -p $(BUILD)
	./gen_defs ./tools > $@

$(GEN_BOOT_PY_HEADER): $(BOOT_PY_SOURCE) $(FILE2H) | $(HEADER_BUILD)
		$(ECHO) "Convert boot.py"
		$(Q)$(PYTHON) $(FILE2H) $< > $@

$(GEN_BOOTSTRAP_PY_HEADER): $(BOOTSTRAP_PY_SOURCE) $(FILE2H) | $(HEADER_BUILD)
		$(ECHO) "Convert bootstrap.py"
		$(Q)$(PYTHON) $(FILE2H) $< > $@

include $(TOP)/py/mkrules.mk
