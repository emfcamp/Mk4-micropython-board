#BOARD ?= CC3220SF_LAUNCHXL
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
include boards/$(BOARD)/mpconfigboard.mk

SRC_C = mpmain.c \
	modmachine.c \
	moduos.c \
	modutime.c \
	machine_i2c.c \
	machine_pin.c \
	machine_sd.c \
	machine_spi.c \
	machine_uart.c \
	fatfs_port.c \
	lib/utils/printf.c \
	lib/utils/stdout_helpers.c \
	lib/utils/pyexec.c \
	lib/libc/string0.c \
	lib/oofatfs/ff.c lib/oofatfs/option/unicode.c \
	lib/timeutils/timeutils.c \
	lib/mp-readline/readline.c

SRC_EXTRA =

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

CFLAGS += $(INC) $(CFLAGS_MOD) $(CFLAGS_EXTRA)

SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB) $(EXTMOD_SRC_C)

OBJ += $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_EXTRA:.c=.o))

all: $(BUILD)/libmicropython.a

$(BUILD)/libmicropython.a: $(OBJ)
	$(AR) r $@ $^

#$(BUILD)/_frozen_mpy.c: frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
#	$(ECHO) "MISC freezing bytecode"
#	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

include $(TOP)/py/mkrules.mk
