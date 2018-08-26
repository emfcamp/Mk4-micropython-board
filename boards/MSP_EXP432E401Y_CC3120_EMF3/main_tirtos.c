/*
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== main_tirtos.c ========
 */

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* Example/Board Header files */
#include "MSP_EXP432E401Y.h"

/* External ram setup helper */
#include "epiram.h"

#include "mpconfigboard.h"

//extern void ti_ndk_config_Global_startupFxn();
extern void *mainThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    4096

/*
 * Enable bootloader selection via Pin PB2 HIGH  (JOY Right)
 */
#include "ti/devices/msp432e4/driverlib/driverlib.h"

void checkBOOTCFG()
{
    if (FLASH_CTRL->BOOTCFG & FLASH_BOOTCFG_NW) {
        FLASH_CTRL->FMA = 0x75100000; // BOOTCFG write address
        FLASH_CTRL->FMD = 0x7FFF2AFE; // new BOOTCFG value with PB2 boot pin
        FLASH_CTRL->FMC = FLASH_FMC_WRKEY | FLASH_FMC_COMT;
    }
}

/*
 * Custom reset function called by TI-RTOS before main so we can setup the EPI SRAM
 */
void tildaResetFxn()
{
    ExternalRAM_init();
    return;
}

/*
 * Custom start function called by TI-RTOS before main so we can setup the EPI SRAM
 */
void tildaStartFxn()
{
    ExternalRAM_init();
    checkBOOTCFG();
    return;
}

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t           thread;
    pthread_attr_t      pAttrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Call board init functions */
    MSP_EXP432E401Y_initGeneral();
    MSP_EXP432E401Y_initGPIO();

    #if MICROPY_HW_USB_REPL
    MSP_EXP432E401Y_initUSB(MSP_EXP432E401Y_USBDEVICE);
    #endif

    /* Set priority and stack size attributes */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 1;

    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&pAttrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    pthread_attr_setschedparam(&pAttrs, &priParam);

    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    retc = pthread_create(&thread, &pAttrs, mainThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }

    BIOS_start();

    return (0);
}
