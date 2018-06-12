CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LD = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
AR = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-ar"

# do we want uGFX?
MICROPY_PY_UGFX ?= 1

NETNDK_INCLUDES =

NETNDK_LIBS = \
    -l:ti/ndk/slnetif/lib/slnetifndk.am4fg \
    -l:ti/ndk/hal/timer_bios/lib/hal_timer.am4fg \
    -l:ti/ndk/hal/eth_stub/lib/hal_eth_stub.am4fg \
    -l:ti/ndk/tools/cgi/lib/cgi.am4fg \
    -l:ti/ndk/tools/hdlc/lib/hdlc.am4fg \
    -l:ti/ndk/tools/console/lib/console_min_ipv4.am4fg \
    -l:ti/ndk/netctrl/lib/netctrl_min_ipv4.am4fg \
    -l:ti/ndk/nettools/lib/nettool_ipv4.am4fg \
    -l:ti/ndk/hal/ser_stub/lib/hal_ser_stub.am4fg \
    -l:ti/ndk/tools/servers/lib/servers_min_ipv4.am4fg \
    -l:ti/ndk/hal/userled_stub/lib/hal_userled_stub.am4fg \
    -l:ti/ndk/stack/lib/stk.am4fg \
    -l:ti/ndk/os/lib/os.am4fg

NETSL_INCLUDES = \
    "-I$(SIMPLELINK_SDK_WIFI_PLUGIN_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_SDK_WIFI_PLUGIN_INSTALL_DIR)/source/ti/drivers/net/wifi"

NETSL_LIBS = \
    "-L$(SIMPLELINK_SDK_WIFI_PLUGIN_INSTALL_DIR)/source" \
    -l:ti/drivers/net/wifi/slnetif/gcc/Release/slnetifwifi.a \
    -l:ti/drivers/net/wifi/gcc/rtos/msp432e4/simplelink.a

INCLUDES = \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/net/bsd" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" \
    "-I$(XDC_INSTALL_DIR)/packages" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include/newlib-nano" \
    "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/include" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

CFLAGS += \
    $(INCLUDES) \
    $(NETSL_INCLUDES) \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    -std=c99

LFLAGS = \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" \
    -l:ti/net/lib/gcc/m4f/slnetsock_release.a \
    -l:ti/net/sntp/lib/gcc/m4f/sntp_release.a \
    $(NETSL_LIBS) \
    -l:ti/display/lib/display.am4fg \
    -l:ti/drivers/lib/drivers_msp432e4.am4fg \
    -l:ti/devices/msp432e4/driverlib/lib/gcc/m4f/msp432e4_driverlib.a \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages" \
    -l:ti/dpl/lib/dpl_msp432e4.am4fg \
    -Wl,-T,MSP_EXP432E401Y_TIRTOS.lds \
    "-Wl,-T,$(KERNEL_BUILD)/linker.cmd" \
    "-Wl,-Map,$@.map" \
    -nostartfiles \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    -Wl,--gc-sections \
    "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/kernel/tirtos/packages/gnu/targets/arm/libs/install-native/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs
