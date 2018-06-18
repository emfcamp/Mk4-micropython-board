#ifndef MPCONFIBOARD_H_INC
#define MPCONFIBOARD_H_INC

#include "MSP_EXP432E401Y.h"

#define MICROPY_HW_BOARD_NAME        "TiLDA Mk4 Prototype"
#define MICROPY_HW_MCU_NAME          "TI MSP432E4"

#define MICROPY_PY_SOCKET            (1)
#define MICROPY_PY_NETWORK           (1)
#define MICROPY_PY_NETWORK_NDK       (0)   /* TI NDK Ethernet */
#define MICROPY_PY_NETWORK_WIFI      (1)   /* TI WiFi */
#define MICROPY_MACHINE_NVSBDEV      (1)
#define MICROPY_MACHINE_SD           (0)
#define MICROPY_HW_USB_REPL          (1)   /* Enable the USB and REPL */
#define MICROPY_HW_USB_MSC           (1)   /* Only enable this is USB_REPL is also enabled */
#define MICROPY_HW_UART_REPL         MSP_EXP432E401Y_UART4
#define MICROPY_HW_UART_REPL_BAUD    115200
#define MICROPY_HW_HAS_UGFX          (MICROPY_PY_UGFX)  // set in mpconfigbpard.mk

#define MICROPY_HW_ENABLE_STORAGE    (1)
#define MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE (1)

#if MICROPY_HW_ENABLE_INTERNAL_FLASH_STORAGE
// Provide block device macros if internal flash storage is enabled
#define MICROPY_HW_BDEV_IOCTL flash_bdev_ioctl
#define MICROPY_HW_BDEV_READBLOCK flash_bdev_readblock
#define MICROPY_HW_BDEV_WRITEBLOCK flash_bdev_writeblock
#endif

#if MICROPY_HW_HAS_UGFX
#define MICROPY_HW_UGFX_SPI         MSP_EXP432E401Y_SPI0
#define MICROPY_HW_UGFX_PIN_CS      MSP_EXP432E401Y_LCD_CS
#define MICROPY_HW_UGFX_PIN_RST     MSP_EXP432E401Y_GPIO_LCD_RST
#define MICROPY_HW_UGFX_PIN_A0      MSP_EXP432E401Y_GPIO_LCD_DCX
#endif

#endif
