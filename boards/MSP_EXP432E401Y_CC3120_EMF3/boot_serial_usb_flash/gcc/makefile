
include ../../../../$(BUILD)/defs.mk

include $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/imports.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

OBJECTS = bl_autobaud.obj bl_can.obj bl_check.obj bl_crc32.obj bl_decrypt.obj bl_emac.obj bl_flash.obj bl_i2c.obj bl_main.obj bl_packet.obj bl_ssi.obj bl_uart.obj bl_usb.obj bl_usbfuncs.obj bl_startup_gcc.obj

NAME = boot_serial_usb_flash

CFLAGS = -I.. "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" "-I$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/third_party/CMSIS/Include" -DDeviceFamily_MSP432E4 -D__MSP432E401Y__ -mcpu=cortex-m4 -march=armv7e-m -mthumb -std=c99 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -g -gstrict-dwarf -Wall "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = --entry ResetISR -nostartfiles -Wl,-T,../gcc/bl_link.lds "-Wl,-Map,$(NAME).map" "-L$(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source" -l:ti/display/lib/display.am4fg -l:ti/grlib/lib/gcc/m4f/grlib.a -l:third_party/spiffs/lib/gcc/m4f/spiffs.a -l:ti/drivers/lib/drivers_msp432e4.am4fg -l:third_party/fatfs/lib/gcc/m4f/fatfs.a -l:ti/devices/msp432e4/driverlib/lib/gcc/m4f/msp432e4_driverlib.a -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -static -Wl,--gc-sections "-L$(GCC_ARMCOMPILER)/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard" -lgcc -lc -lm -lnosys --specs=nano.specs

all: $(NAME).out

bl_autobaud.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_autobaud.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_can.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_can.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_check.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_check.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_crc32.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_crc32.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_decrypt.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_decrypt.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_emac.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_emac.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_flash.obj: ../bl_flash.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_i2c.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_i2c.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_main.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_main.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_packet.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_packet.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_ssi.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_ssi.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_uart.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_uart.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_usb.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usb.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_usbfuncs.obj: $(SIMPLELINK_MSP432E4_SDK_INSTALL_DIR)/source/ti/devices/msp432e4/boot_loader/bl_usbfuncs.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

bl_startup_gcc.obj: ../gcc/bl_startup_gcc.S
	@ echo Building $@
	@ $(CC) $(CFLAGS) -x assembler-with-cpp $< -c -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking...
	@ $(LNK)  $(OBJECTS) $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
