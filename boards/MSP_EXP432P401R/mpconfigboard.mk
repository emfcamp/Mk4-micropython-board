CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LD = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
AR = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-ar"

INCLUDES = -I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source/ti/posix/gcc \
    -I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source \
    "-I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    "-I$(XDC_INSTALL_DIR)/packages" \
    -I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include \
    "-I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

CFLAGS += -D__MSP432P401R__ -DMSP432WARE \
    $(INCLUDES) \
    -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -ffunction-sections -fdata-sections -g -gstrict-dwarf \
    -Wall -std=c99

LFLAGS = -Wl,-T,MSP_EXP432P401R_TIRTOS.lds \
    -Wl,-Map,$@.map -Wl,-T,$(KERNEL_BUILD)/linker.cmd -nostartfiles \
    -L$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source \
    -l:ti/display/lib/display.am4fg \
    -l:ti/drivers/lib/drivers_msp432p4xx.am4fg \
    -L$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages/ti/dpl/lib \
    -l:dpl_msp432p4xx.am4fg \
    -L$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source/ti/devices/msp432p4xx/driverlib/gcc \
    -l:msp432p4xx_driverlib.a \
    -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -static -Wl,--gc-sections "-L$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard" -lgcc -lc -lm -lnosys --specs=nano.specs
