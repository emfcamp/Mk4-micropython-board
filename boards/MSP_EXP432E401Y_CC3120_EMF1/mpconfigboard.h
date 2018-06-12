#ifndef MPCONFIBOARD_H_INC
#define MPCONFIBOARD_H_INC

#define MICROPY_HW_BOARD_NAME        "MSP_EXP432E401Y_CC3120"
#define MICROPY_HW_MCU_NAME          "TI MSP432E4"

#define MICROPY_PY_SOCKET            (1)
#define MICROPY_PY_NETWORK           (1)
#define MICROPY_PY_NETWORK_NDK       (0)   /* TI NDK Ethernet */
#define MICROPY_PY_NETWORK_WIFI      (1)   /* TI WiFi */
#define MICROPY_MACHINE_NVSBDEV      (0)
#define MICROPY_MACHINE_SD           (0)
#define MICROPY_HW_HAS_UGFX          (MICROPY_PY_UGFX)  // set in mpconfigbpard.mk



// None of these are correct...
#define MICROPY_HW_UGFX_INTERFACE  UGFX_DRIVER_SPI

#define MICROPY_HW_UGFX_SPI         2

#define MICROPY_HW_UGFX_PORT_CS     GPIOA
#define MICROPY_HW_UGFX_PORT_RST    GPIOA
#define MICROPY_HW_UGFX_PORT_A0     GPIOA
#define MICROPY_HW_UGFX_PIN_CS      (1<<1)
#define MICROPY_HW_UGFX_PIN_RST     (1<<2)
#define MICROPY_HW_UGFX_PIN_A0      (1<<3)

#define MSP_EXP432E401Y_LCD_CS_pin  15
#define MSP_EXP432E401Y_LCD_RST_pin 16
#define MSP_EXP432E401Y_LCD_A0_pin 17


#endif
