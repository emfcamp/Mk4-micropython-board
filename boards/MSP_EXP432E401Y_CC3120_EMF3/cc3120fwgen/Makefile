include ../../../build-MSP_EXP432E401Y_CC3120_EMF3/defs.mk

all: cc3120fw_ucf.h

cc3120fw.ucf: buildFw.bash
	@ echo building firmware assets
	./buildFw.bash $(UNIFLASH_INSTALL_DIR) $(SIMPLELINK_SDK_WIFI_PLUGIN_INSTALL_DIR) 

cc3120fw_ucf.h: cc3120fw.ucf
	@ echo creating header file
	xxd -i cc3120fw.ucf > cc3120fw_ucf.h
	sed -i 's/unsigned char/const unsigned char/g' cc3120fw_ucf.h
	sed -i 's/cc3120fw_ucf/cc3120r_prod_sdk210sp_Programming_ucf/g' cc3120fw_ucf.h
	echo "#define CC3120_FW_VERSION_NWP 0x03070001" >> cc3120fw_ucf.h
	cp cc3120fw_ucf.h ../cc3120fw_ucf.h

clean:
	@ echo cleaning ...
	@ rm -rf cc3120fw_ucf.h cc3120fw.sli cc3120fw.ucf projects
