SIMPLELINK_CC32XX_SDK_INSTALL_DIR ?= /home/a0324034/tmp/simplelink_cc32xx_sdk_1_60_00_01_eng

include $(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/imports.mak

KERNEL_BUILD := $(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/builds/CC3220SF_LAUNCHXL/debug

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LD = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
AR = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-ar"

CONFIGPKG = $(KERNEL_BUILD)/gcc

CFLAGS = -I../.. \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source/ti/drivers/net/wifi" \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    "-I$(XDC_INSTALL_DIR)/packages" \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=soft \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = "-L$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source" \
    -l:gcc/client/mqtt_client.a \
    -l:ti/net/lib/gcc/m4/slnetsock_debug.a \
    -l:ti/drivers/net/wifi/slnetif/gcc/Release/slnetifwifi.a \
    -Wl,-T,../../tirtos/gcc/CC3220SF_LAUNCHXL_TIRTOS.lds \
    "-Wl,-Map,$(NAME).map" \
    -l:ti/display/lib/display.am4g \
    -l:ti/drivers/lib/drivers_cc32xx.am4g \
    -l:third_party/fatfs/lib/fatfs.am4g \
    "-L$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/source/ti/drivers/net/wifi" \
    -l:gcc/rtos/simplelink.a \
    "-L$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -l:ti/dpl/lib/dpl_cc32xx.am4g \
    "-Wl,-T,$(KERNEL_BUILD)/gcc/linker.cmd" \
    -l:ti/devices/cc32xx/driverlib/gcc/Release/driverlib.a \
    -march=armv7e-m \
    -mthumb \
    -nostartfiles \
    -static \
    -Wl,--gc-sections \
    "-L$(SIMPLELINK_CC32XX_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

# $(CONFIGPKG)/linker.cmd $(CONFIGPKG)/compiler.opt:
# 	@ $(ECHOBLANKLINE)
# 	@ echo $(CONFIGPKG) is not built.
# 	@ echo You can build it by issuing $(MAKE) in $(CONFIGPKG).
# 	@ $(ECHOBLANKLINE)

# %.o: %.c
# 	@ echo compiling $@ ...
# 	@ $(CC) $(CFLAGS) $< -c @$(CONFIGPKG)/compiler.opt -o $@
