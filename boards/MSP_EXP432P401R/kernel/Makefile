include ../../sdk/defs.mk

XDCPATH = $(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/source;$(SIMPLELINK_MSP432P4_SDK_INSTALL_DIR)/kernel/tirtos/packages

CFG_FILE = kernel.cfg

PLATFORM = ti.platforms.msp432:MSP432P401R

#XDCOPTIONS=v
export XDCOPTIONS

XS = $(XDC_INSTALL_DIR)/xs

# configuro will determine if a (re)config is needed so no dependency
all:
	@ $(XS) --xdcpath="$(XDCPATH)" xdc.tools.configuro -o ./build \
            -t gnu.targets.arm.M4F -p $(PLATFORM) -c "$(GCC_ARMCOMPILER)" \
            --compileOptions "$(CFLAGS)" $(CFG_FILE)

clean:
	@ echo cleaning ...
	@ rm -rf ./build
