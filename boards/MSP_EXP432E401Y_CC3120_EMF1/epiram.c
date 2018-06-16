/*
 * This file is part of the Micro Python project, http://micropython.org/
 * And the Electromagnetic Field: TiLDA Mk4 Badge
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Electromagnetic Field Badge Team Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include "epiram.h"

/*
 *  ======== ExternalRAM_init ========
 *  This is mostly taken direct from the MSP432E4 SDK epi_sdram_basic exmpale.
 */
void ExternalRAM_init(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    MAP_GPIOPinConfigure(GPIO_PH0_EPI0S0);
    MAP_GPIOPinConfigure(GPIO_PH1_EPI0S1);
    MAP_GPIOPinConfigure(GPIO_PH2_EPI0S2);
    MAP_GPIOPinConfigure(GPIO_PH3_EPI0S3);
    MAP_GPIOPinConfigure(GPIO_PC7_EPI0S4);
    MAP_GPIOPinConfigure(GPIO_PC6_EPI0S5);
    MAP_GPIOPinConfigure(GPIO_PC5_EPI0S6);
    MAP_GPIOPinConfigure(GPIO_PC4_EPI0S7);
    MAP_GPIOPinConfigure(GPIO_PA6_EPI0S8);
    MAP_GPIOPinConfigure(GPIO_PA7_EPI0S9);
    MAP_GPIOPinConfigure(GPIO_PG1_EPI0S10);
    MAP_GPIOPinConfigure(GPIO_PG0_EPI0S11);
    MAP_GPIOPinConfigure(GPIO_PM3_EPI0S12);
    MAP_GPIOPinConfigure(GPIO_PM2_EPI0S13);
    MAP_GPIOPinConfigure(GPIO_PM1_EPI0S14);
    MAP_GPIOPinConfigure(GPIO_PM0_EPI0S15);
    MAP_GPIOPinConfigure(GPIO_PL0_EPI0S16);
    MAP_GPIOPinConfigure(GPIO_PL1_EPI0S17);
    MAP_GPIOPinConfigure(GPIO_PL2_EPI0S18);
    MAP_GPIOPinConfigure(GPIO_PL3_EPI0S19);
    MAP_GPIOPinConfigure(GPIO_PB3_EPI0S28);
    MAP_GPIOPinConfigure(GPIO_PP2_EPI0S29);
    MAP_GPIOPinConfigure(GPIO_PP3_EPI0S30);
    MAP_GPIOPinConfigure(GPIO_PK5_EPI0S31);

    MAP_GPIOPinTypeEPI(GPIO_PORTA_BASE, (GPIO_PIN_7 | GPIO_PIN_6));
    MAP_GPIOPinTypeEPI(GPIO_PORTB_BASE, (GPIO_PIN_3));
    MAP_GPIOPinTypeEPI(GPIO_PORTC_BASE, (GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 |
                                         GPIO_PIN_4));
    MAP_GPIOPinTypeEPI(GPIO_PORTG_BASE, (GPIO_PIN_1 | GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTH_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTK_BASE, (GPIO_PIN_5));
    MAP_GPIOPinTypeEPI(GPIO_PORTL_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTM_BASE, (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 |
                                         GPIO_PIN_0));
    MAP_GPIOPinTypeEPI(GPIO_PORTP_BASE, (GPIO_PIN_3 | GPIO_PIN_2));
    /* Enable the clock to the EPI and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPI0);

    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_EPI0)))
    {
    }

    /* Configure the EPI to access the SDRAM memory at 60 MHz Set the EPI
     * clock to half the system clock. */
    MAP_EPIDividerSet(EPI0_BASE, 1);

    /* Sets the usage mode of the EPI module.  For this example we will use
     * the SDRAM mode to talk to the external 64MB SDRAM daughter card. */
    MAP_EPIModeSet(EPI0_BASE, EPI_MODE_SDRAM);

    /* Configure the SDRAM mode.  We configure the SDRAM according to our core
     * clock frequency.  We will use the normal (or full power) operating
     * state which means we will not use the low power self-refresh state.
     * Set the SDRAM size to 64MB with a refresh interval of 468 clock ticks*/
    MAP_EPIConfigSDRAMSet(EPI0_BASE, (EPI_SDRAM_CORE_FREQ_50_100 |
                                      EPI_SDRAM_FULL_POWER |
                                      EPI_SDRAM_SIZE_64MBIT), 468);

    /* Set the address map.  The EPI0 is mapped from 0x60000000 to 0x01FFFFFF.
     * For this example, we will start from a base address of 0x60000000 with
     * a size of 256MB.  Although our SDRAM is only 64MB, there is no 64MB
     * aperture option so we pick the next larger size. */
    MAP_EPIAddressMapSet(EPI0_BASE, EPI_ADDR_RAM_SIZE_256MB | EPI_ADDR_RAM_BASE_6);

    /* Wait for the SDRAM wake-up to complete by polling the SDRAM
     * initialization sequence bit.  This bit is true when the SDRAM interface
     * is going through the initialization and false when the SDRAM interface
     * it is not in a wake-up period. */
    while(EPI0->STAT & EPI_STAT_INITSEQ)
    {
    }
}