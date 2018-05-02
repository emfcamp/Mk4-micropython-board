/*
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include <pthread.h>

#include <ti/sysbios/BIOS.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>

#include "Board.h"

#define STACKSIZE 8192u
#define MPHEAPSIZE 16384u
#define EXITMSG "MicroPython exit\n"

extern int mp_main(void * heap, uint32_t heapsize, uint32_t stacksize,
                   UART_Handle uart);

static uint8_t heap[MPHEAPSIZE];

static void * mpThread(void * arg)
{
   UART_Handle uart;
   UART_Params params;

   UART_Params_init(&params);
   params.baudRate = 57600;
   params.writeDataMode = UART_DATA_BINARY;
   params.readDataMode = UART_DATA_BINARY;

   if ((uart = UART_open(0, &params)) == NULL) {
       while (1);
   }

   mp_main(heap, sizeof(heap), STACKSIZE, uart);

   UART_write(uart, EXITMSG, sizeof(EXITMSG));
   UART_close(uart);

   return NULL;
}

/*
 *  ======== main ========
 */
int main(void)
{
    Board_initGeneral();
    GPIO_init();
    I2C_init();
    SPI_init();
    UART_init();
    PWM_init();

    pthread_t thread;
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, STACKSIZE);
    pthread_create(&thread, &attrs, mpThread, NULL);
    pthread_attr_destroy(&attrs);

    if (!thread) {
        while(1);
    }

    BIOS_start();

    return (0);
}
