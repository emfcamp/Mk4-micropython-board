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

#ifndef __I2C_THREAD_H
#define __I2C_THREAD_H

/* BIOS module Headers */
#include <ti/sysbios/knl/Event.h>
#include <ti/drivers/dpl/MutexP.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <stdbool.h>

#define Event_TCA_INT   Event_Id_00
#define Event_BQ_INT    Event_Id_01
#define Event_HDC_INT   Event_Id_02

#ifdef __cplusplus
extern "C" {
#endif

// Event handle for use intrenal and from mod tilda
Event_Handle i2cEvtHandle;

// shared states
typedef struct i2c_shared_states_t {
    uint32_t sampleRate;
    uint32_t batteryVoltage;
    uint32_t vbusAttached;
    uint32_t chargeState;
    uint32_t tmpTemperature;
    uint32_t hcdTemperatue;
    uint32_t hcdHumidity;
    float optLux;
    bool button1;
    bool button2;
    bool button3;
    bool button4;
    bool button5;
    bool button6;
    bool button7;
    bool button8;
    bool button9;
    bool buttonStar;
    bool button0;
    bool buttonHash;
    bool buttonA;
    bool buttonB;
    bool buttonCall;
    bool buttonEnd;
} i2c_shared_states_t;

i2c_shared_states_t i2cSharedStates;
static MutexP_Handle mutexSharedStates;

void * i2cThread(void *arg);


#ifdef __cplusplus
}
#endif


#endif // __I2C_THREAD_H