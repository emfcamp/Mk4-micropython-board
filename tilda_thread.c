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
#include <ti/sail/opt3001/opt3001.h>

/* Example/Board Header files */
#include "MSP_EXP432E401Y.h"

#include "py/nlr.h"
#include "py/runtime.h"

#include "tilda_thread.h"

Event_Struct evtStruct;
I2C_Handle      i2cHandle;

uint16_t buttonState;
uint16_t lastButtonState;

typedef struct tilda_tca_callback_modes_t {
    bool on_press;
    bool on_release;
} tilda_tca_callback_modes_t;

static tilda_tca_callback_modes_t tildaButtonCallbackModes[Buttons_MAX];

void tilda_init0()
{
    for (int i = 0; i < Buttons_MAX; i++) {
        MP_STATE_PORT(tilda_button_callback)[i] = mp_const_none;
        tildaButtonCallbackModes[i].on_press = false;
        tildaButtonCallbackModes[i].on_release = false;
   }
}

static void tildaGpioCallback(uint8_t index) {
    uint8_t button = index + Buttons_JOY_Center;
    mp_obj_t *tilda_button_callback = &MP_STATE_PORT(tilda_button_callback)[button];
    if (*tilda_button_callback != mp_const_none) {
        mp_sched_schedule(*tilda_button_callback, MP_OBJ_NEW_SMALL_INT(button));
    }
    extern Semaphore_Handle machine_sleep_sem;
    Semaphore_post(machine_sleep_sem);
}

void tcaInterruptHandler(uint8_t index)
{
    // set TCA event flag
    Event_post(tildaEvtHandle, Event_TCA_INT);
}

void hdcInterruptHandler(uint8_t index)
{
    // set hcd data ready event flag
    Event_post(tildaEvtHandle, Event_HDC_INT);
}

void bqInterruptHandler(uint8_t index)
{
    // set bq data ready event flag
    Event_post(tildaEvtHandle, Event_BQ_INT);
}

void readTCAButtons() 
{
    //  read button states 
    uint8_t readBuffer[2];
    uint8_t writeBuffer[1];
    writeBuffer[0] = 0x00;
    I2C_Transaction i2cTransaction;
    i2cTransaction.slaveAddress = 0x20;
    i2cTransaction.writeBuf = writeBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf = readBuffer;
    i2cTransaction.readCount = 2;
    bool status = I2C_transfer(i2cHandle, &i2cTransaction);
    if (status == false) {
        // Unsuccessful I2C transfer
        return;
    }
    lastButtonState = buttonState;
    //  update shared state 
    buttonState = readBuffer[1];
    buttonState = (buttonState << 8) | readBuffer[0];
}

void *tildaThread(void *arg)
{
    I2C_Params      i2cParams;

    tildaSharedStates.sampleRate = 500; // default to 0.5 Sec sample rate

    // setup Event
    Event_construct(&evtStruct, NULL);
    tildaEvtHandle = Event_handle(&evtStruct);

    // Init Internal I2C bus
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2cHandle = I2C_open(MSP_EXP432E401Y_I2C4, &i2cParams);
    // if (i2cHandle == NULL) {
    //     // Display_printf(display, 0, 0, "Error Initializing I2C\n");
    //     while (1);
    // }

    // register BQ25601 Int handler
    GPIO_disableInt(MSP_EXP432E401Y_GPIO_BQ_INT);
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_BQ_INT, bqInterruptHandler);
    GPIO_enableInt(MSP_EXP432E401Y_GPIO_BQ_INT);

    // register TCA9555 Int handler
    GPIO_disableInt(MSP_EXP432E401Y_GPIO_TCA_INT);
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_TCA_INT, tcaInterruptHandler);
    GPIO_enableInt(MSP_EXP432E401Y_GPIO_TCA_INT);

    // register HDC2080 Int handler
    GPIO_disableInt(MSP_EXP432E401Y_GPIO_HDC_INT);
    GPIO_setCallback(MSP_EXP432E401Y_GPIO_HDC_INT, hdcInterruptHandler);
    GPIO_enableInt(MSP_EXP432E401Y_GPIO_HDC_INT);

    // setup charger?
    
    // setup sensors
    
    // do an inital button read
    readTCAButtons();
    lastButtonState = buttonState;

    OPT3001_Handle opt3001Handle = NULL;
    OPT3001_Params opt3001Params;
    OPT3001_Params_init(&opt3001Params);
    opt3001Handle = OPT3001_open(MSP_EXP432E401Y_OPT3001_0, i2cHandle,
            &opt3001Params);

    uint32_t posted;
    bool scheduled;

    // loop
    for (;;) {
        // wait for TCA or HDC evnt or time out (default 500ms, might be settable)
        posted = Event_pend(tildaEvtHandle, 
            Event_Id_NONE,                                  /* andMask */
            Event_BQ_INT + Event_TCA_INT + Event_HDC_INT,   /* orMack */
            tildaSharedStates.sampleRate);

        // if TCA event 
        if (posted & Event_TCA_INT) {
            readTCAButtons();
            scheduled = false;
            //  fire any callbacks if needed
            //  comparing new and last buttons states
            for (int button = 0; button < 16; ++button)
            {
                mp_obj_t *tca_callback_irq = &MP_STATE_PORT(tilda_button_callback)[button];
                if (*tca_callback_irq != mp_const_none) {
                    if (tildaButtonCallbackModes[button].on_press && !((buttonState >> button) & 0x1) && ((lastButtonState >> button) & 0x1)) {
                        mp_sched_schedule(*tca_callback_irq, MP_OBJ_NEW_SMALL_INT(button));
                        scheduled = true;
                    }
                    if (tildaButtonCallbackModes[button].on_release && ((buttonState >> button) & 0x1) && !((lastButtonState >> button) & 0x1)) {
                        mp_sched_schedule(*tca_callback_irq, MP_OBJ_NEW_SMALL_INT(button));
                        scheduled = true;
                    }
                }
            }
            // wake the mp?
            if (scheduled) {
                extern Semaphore_Handle machine_sleep_sem;
                Semaphore_post(machine_sleep_sem);
            }
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
            OPT3001_getLux(opt3001Handle, &tildaSharedStates.optLux);
            // grab battery updates?
            
            // kick off Humidity conversion
            
        }
    }
    
    return NULL;
}

bool getButtonState(TILDA_BUTTONS_Names button)
{
    if (button < Buttons_JOY_Center) {
        // TCA button
        // shift and mask buttonState
        // 0 == button pressed, and shouold return true
        // 1 == button not pressed and should return false
        return !((buttonState >> button) & 0x1);
    } else if (button < Buttons_BTN_Menu) {
        // joystick 
        // 1 == button pressed, and shouold return true
        return GPIO_read(button - Buttons_JOY_Center);
    } else if (button == Buttons_BTN_Menu) {
        // 0 == button pressed, and shouold return true
        return !GPIO_read(button - Buttons_JOY_Center);
    }
    return false;
}

void registerButtonCallback(uint8_t button, mp_obj_t tca_callback_irq,  bool on_press, bool on_release)
{
    mp_obj_t *cb = &MP_STATE_PORT(tilda_button_callback)[button];
    *cb = tca_callback_irq;
    tildaButtonCallbackModes[button].on_press = on_press;
    tildaButtonCallbackModes[button].on_release = on_release;
    if (button >= Buttons_JOY_Center) {
        uint8_t gpioIndex = button - Buttons_JOY_Center;
        // This is a GPIO attached button need to do some extra setup
        
        // call GPIO_setConfig to setup the Interupt direction
        GPIO_PinConfig cfg;
        GPIO_getConfig(gpioIndex, &cfg);

        cfg = (cfg & (~GPIO_CFG_INT_MASK)); // clear INT
        if (button == Buttons_BTN_Menu) { 
            if (on_press)
                cfg |= GPIO_CFG_IN_INT_FALLING;
            if (on_release)
                cfg |= GPIO_CFG_IN_INT_RISING;
        } else {
            if (on_press)
                cfg |= GPIO_CFG_IN_INT_RISING;
            if (on_release)
                cfg |= GPIO_CFG_IN_INT_FALLING;
        }
        GPIO_setConfig(gpioIndex, cfg);

        // register our generic callback handler
        GPIO_disableInt(gpioIndex);
        GPIO_setCallback(gpioIndex, tildaGpioCallback);
        GPIO_enableInt(gpioIndex);
    }
}


void unregisterButtonCallback(uint8_t button)
{
    mp_obj_t *cb = &MP_STATE_PORT(tilda_button_callback)[button];
    *cb = mp_const_none;
    tildaButtonCallbackModes[button].on_press = false;
    tildaButtonCallbackModes[button].on_release = false;

    if (button >= Buttons_JOY_Center) {
        uint8_t gpioIndex = button - Buttons_JOY_Center;
        // This is a GPIO attached button need to do some extra cleanup
        GPIO_disableInt(gpioIndex);
    }
}
