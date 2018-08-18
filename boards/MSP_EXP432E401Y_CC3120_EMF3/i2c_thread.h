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

/*
 * first 16 entries of this matchs the pin order of the TCA port expander
 * next match the order form MSP_EXP432E401Y_GPIOName
 */
typedef enum TILDA_BUTTONS_Names 
{
    Buttons_BTN_1 = 0,
    Buttons_BTN_End,
    Buttons_BTN_2,
    Buttons_BTN_3,
    Buttons_BTN_6,
    Buttons_BTN_5,
    Buttons_BTN_4,
    Buttons_BTN_7,
    Buttons_BTN_8,
    Buttons_BTN_9,
    Buttons_BTN_Hash,
    Buttons_BTN_0,
    Buttons_BTN_Star,
    Buttons_BTN_Call,
    Buttons_BTN_A,
    Buttons_BTN_B,
    Buttons_JOY_Center,
    Buttons_JOY_Up,
    Buttons_JOY_Down,
    Buttons_JOY_Left,
    Buttons_JOY_Right,
    Buttons_BTN_Menu,
} TILDA_BUTTONS_Names;

// shared states
typedef struct i2c_shared_states_t {
    uint32_t sampleRate;
    // uint32_t batteryVoltage;
    bool vbusAttached;
    uint8_t chargeState; 
    float tmpTemperature;
    float hcdTemperatue;
    float hcdHumidity;
    float optLux;
} i2c_shared_states_t;

i2c_shared_states_t i2cSharedStates;

uint16_t buttonState;
uint16_t lastButtonState;

typedef struct i2c_tca_callbacks_t {
    void* tca_callback_irq;
    bool on_press;
    bool on_release;
} i2c_tca_callbacks_t;

i2c_tca_callbacks_t i2cTCACallbacks[16];

void * i2cThread(void *arg);

bool getButtonState(TILDA_BUTTONS_Names button);
void registerTCACallback(uint8_t button, void* tca_callback_irq,  bool on_press, bool on_release);
void unregisterTCACallback(uint8_t button);


#ifdef __cplusplus
}
#endif


#endif // __I2C_THREAD_H