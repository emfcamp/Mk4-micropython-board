#/bin/bash

if test -d tools/simplelink_msp432e4_sdk_*
then
    ln -s `pwd`/tools/simplelink_msp432e4_sdk_*
fi

if test -d tools/simplelink_msp432p4_sdk_*
then
    ln -s `pwd`/tools/simplelink_msp432p4_sdk_*
fi

if test -d tools/simplelink_cc32xx_sdk_*
then
    ln -s `pwd`/tools/simplelink_cc32xx_sdk_*
fi

if test -d tools/simplelink_sdk_wifi_plugin*
then
    ln -s `pwd`/tools/simplelink_sdk_wifi_plugin*
fi

if test -d tools/gcc-arm-none-eabi*
then
    ln -s `pwd`/tools/gcc-arm-none-eabi-*
fi

if test -d tools/xdctools_*
then
    ln -s `pwd`/tools/xdctools_*
fi
