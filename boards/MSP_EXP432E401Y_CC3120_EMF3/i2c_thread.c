/*
 * This file is part of the Micro Python project, http://micropython.org/
 * And the Electromagnetic Field: TiLDA Mk4 Badge
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Electromagnetic Field Badge Team Authors
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

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Example/Board Header files */
#include "MSP_EXP432E401Y.h"

#include "i2c_thread.h"

Event_Struct evtStruct;


void tcaInterruptHandler(uint8_t index)
{
    // set TCA event flag
    Event_post(i2cEvtHandle, Event_TCA_INT);
}

void hdcInterruptHandler(uint8_t index)
{
    // set hcd data ready event flag
    Event_post(i2cEvtHandle, Event_HDC_INT);
}

void bqInterruptHandler(uint8_t index)
{
    // set bq data ready event flag
    Event_post(i2cEvtHandle, Event_BQ_INT);
}

void *i2cThread(void *arg)
{
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;

    i2cSharedStates.sampleRate = 500; // default to 0.5 Sec sample rate

    // setup Event
    Event_construct(&evtStruct, NULL);
    i2cEvtHandle = Event_handle(&evtStruct);

    // setup Mutex
    mutexSharedStates = MutexP_create(NULL);
    // if (mutexSharedStates == NULL) {
    //     // Display_printf(display, 0, 0, "Could not create USB Wait mutex.\n");
    //     while(1);
    // }

    // Init Internal I2C bus
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(MSP_EXP432E401Y_I2C4, &i2cParams);
    // if (i2c == NULL) {
    //     // Display_printf(display, 0, 0, "Error Initializing I2C\n");
    //     while (1);
    // }

    // register BQ25601 Int handler
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_BQ_INT, bqInterruptHandler);
    
    // register TCA9555 Int handler
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_TCA_INT, tcaInterruptHandler);

    // register HDC2080 Int handler
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_HDC_INT, hdcInterruptHandler);

    // setup charger?
    
    // setup sensors
    
    // do initial i2c read to populate shared states?

    uint32_t posted;

    // loop
    for (;;) {
        // wait for TCA or HDC evnt or time out (default 500ms, might be settable)
        posted = Event_pend(i2cEvtHandle, 
            Event_Id_NONE,                                  /* andMask */
            Event_BQ_INT + Event_TCA_INT + Event_HDC_INT,   /* orMack */
            i2cSharedStates.sampleRate);

        // if TCA event 
        if (posted & Event_TCA_INT) {
            //  read button states update shared state 
             
            //  fire any callbacks if needed
             
            // wake the mp?
            // extern Semaphore_Handle machine_sleep_sem;
            // Semaphore_post(machine_sleep_sem);
        }

        // else if bq event
        if (posted & Event_BQ_INT) {
            // check what changed
        
        }

        // else if hcd data ready
        if (posted & Event_HDC_INT){
            // grab temp and hum readings 
        
        }

        // else if time out 
        if (posted == 0) {
            // grab TMP temp readings
            
            // grab lux readings
            
            // grab battery updates?
            
            // kick off Humidity conversion
            
        }
    }
    
    return NULL;
}