# TI SimpleLink TI-RTOS MicroPython Port


See https://badge.emfcamp.org/wiki/Main_Page#EMF2018_-_TiLDA_Mk.CE.B4

```
git clone --recurse-submodules https://github.com/micropython/micropython.git
cd micropython
git submodule add git@github.com:emfcamp/Mk4-micropython-board.git ports/ti
cd ports/ti
./inst_tools MSP_EXP432E401Y_CC3120

make flash-dfu BOARD=MSP_EXP432E401Y_CC3120_EMF3
```
