#ifndef MPCONFIBOARD_H_INC
#define MPCONFIBOARD_H_INC

#define MICROPY_HW_BOARD_NAME        "TiLDA Mk4 Prototype"
#define MICROPY_HW_MCU_NAME          "TI MSP432E4"

#define MICROPY_PY_SOCKET            (1)
#define MICROPY_PY_NETWORK           (1)
#define MICROPY_PY_NETWORK_NDK       (0)   /* TI NDK Ethernet */
#define MICROPY_PY_NETWORK_WIFI      (1)   /* TI WiFi */
#define MICROPY_MACHINE_NVSBDEV      (0)
#define MICROPY_MACHINE_SD           (0)

#endif
