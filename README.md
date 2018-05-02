# TI SimpleLink TI-RTOS MicroPython Port

## Build

* Linux only

* Boards
  * MSP_EXP432E401Y
  * LAUNCHXL_CC3220SF
  * YOUR_BOARD below refers to one of the above

* Download and install the SimpleLink SDK for the desired device family, 2.10+
  * MSP432E4: http://www.ti.com/tool/download/SIMPLELINK-MSP432E4-SDK
  * CC3220: http://www.ti.com/tool/SIMPLELINK-CC3220-SDK

* Download and install GCC toolchain: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

* After installing the SDK and GCC tools:
  * edit $SDK/imports.mak
    * set the paths for XDC_INSTALL_DIR and GCC_ARMCOMPILER
  * cd $SDK/kernel/tirtos/builds/YOUR_BOARD/debug/gcc
  * make

* MicroPython
  * git clone https://github.com/micropython/micropython.git
  * cd micropython/ports
  * tar xf mp_ti.tar.gz
  * cd ti
  * edit boards/YOUR_BOARD/mpconfigboard.mk
    * set the SIMPLELINK_*_INSTALL_DIR to directory with SDK
  * make BOARD=YOUR_BOARD
    * or edit Makefile to change default BOARD

The result should be a library in build-YOUR_BOARD/libmicropython.a that can be used to build an application program.

## Example Application

An example application shows how to use the MP library - eventually, this should probably go in the ports/ti/boards/YOUR_BOARD directories.

* tar xf mpex.tar.gz
* cd mpex/tirtos/gcc
* edit makefile
  * SIMPLELINK_*_INSTALL_DIR - point to same directory as used to build MP
  * MICROPYTHON - point to directory of micropython repo
* make
* flash mpex.out to board
* use REPL via UART terminal

## Status

* SD Card support with FAT filesystem
  * will execute /main.py if present when booted
  * Python module search path includes /modules directory
* UART REPL over XDS110 USB connection
* Hardware Modules - see machine modules

## machine Modules

* Follows ["machine" interface](https://docs.micropython.org/en/latest/pyboard/library/machine.html)
* Pin (GPIO) - read/write pins
  * configuring pins at runtime not supported yet
* I2C - uses I2C driver configured in board.c
  * fixed to max 3 currently
* SPI - uses SPI driver configured in board.c
  * fixed to max 3 currently
* UART - not implemented, stubs-only functions TBD

* deinit(): does nothing since TI-RTOS drivers have power management

## Questions

* init()/deinit() - lifecycle model
* release a resource/object - how/when call I2C_close()?
* multiple instances of same underlying resource are shared - appears to
be the MP model. Multiple creates (opens) of the same id/index as in

``` python
from machine import I2C

i2c1 = I2C(0, baudrate=100000)
print(i2c1)
<machine_i2c.20000448> id=1 baudrate=100000
i2c2 = I2C(0, baudrate=400000)
print(i2c1)
<machine_i2c.20000448> id=1 baudrate=400000
```

results in the last one setting the operating values for all the others.

* static tables are setup for N instances of each driver - these are currently hard-coded to the magic number "3" (see NUM_SPI and NUM_I2C)

### SD

A custom machine_sd.c is provided since there are HW SD drivers in TI-RTOS. This is used for both SPI and HW implementations.
