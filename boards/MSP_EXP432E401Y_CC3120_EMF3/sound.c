//*****************************************************************************
//
// sound.c - Sound driver
//
// Copyright (c) 2013-2018 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions
//   are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

#include <ti/devices/msp432e4/inc/msp432.h>

#include <ti/devices/msp432e4/driverlib/types.h>
#include <ti/devices/msp432e4/driverlib/pwm.h>
#include <ti/devices/msp432e4/driverlib/inc/hw_timer.h>

#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/rom.h>
#include <ti/devices/msp432e4/driverlib/sysctl.h>
#include <ti/devices/msp432e4/driverlib/timer.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerMSP432E4.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOMSP432E4.h>

#include <ti/devices/msp432e4/inc/msp432.h>
#include <ti/devices/msp432e4/driverlib/interrupt.h>
#include <ti/devices/msp432e4/driverlib/gpio.h>
#include <ti/devices/msp432e4/driverlib/pwm.h>
#include <ti/devices/msp432e4/driverlib/pin_map.h>

#include <ti/drivers/pwm/PWMMSP432E4.h>

#include "sound.h"

//*****************************************************************************
//
//! \addtogroup sound_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// This structure defines the internal state of the sound driver.
//
//*****************************************************************************
typedef struct
{
    //
    // The number of clocks per PWM period.
    //
    uint32_t ui32Period;

    //
    // A set of flags indicating the mode of the sound driver.
    //
    volatile uint32_t ui32Flags;

    //
    // A pointer to the sound buffer being played.
    //
    const int16_t *pi16Buffer;

    //
    // The length of the sound buffer, in bytes.
    //
    uint32_t ui32Length;

    //
    // The current playback offset into the sound buffer.
    //
    uint32_t ui32Offset;

    //
    // The volume to playback the sound stream.  This is a value between 0
    // (for silence) and 256 (for full volume).
    //
    int32_t i32Volume;

    //
    // The previous and current sound samples, used for interpolating from
    // 8 kHz to 64 kHz sound.
    //
    int16_t pi16Samples[2];

    //
    // The sound step, which corresponds to the current interpolation point
    // between the previous and current sound samples.
    //
    int32_t i32Step;

    //
    // The current requested rate adjustment.  This is cleared when the
    // the adjustment is made.
    //
    int32_t i32RateAdjust;

    //
    // The callback function that indicates when half of the sound buffer has
    // bene played and is therefore ready to be refilled.
    //
    void (*pfnCallback)(uint32_t ui32Half);
}
tSoundState;

//*****************************************************************************
//
// The flags that are in tSoundState.ui32Flags.
//
//*****************************************************************************
#define SOUND_FLAG_STARTUP      0
#define SOUND_FLAG_SHUTDOWN     1
#define SOUND_FLAG_PLAY         2
#define SOUND_FLAG_8KHZ         3
#define SOUND_FLAG_16KHZ        4
#define SOUND_FLAG_32KHZ        5
#define SOUND_FLAG_64KHZ        6

//*****************************************************************************
//
// The current state of the sound driver.
//
//*****************************************************************************
static tSoundState g_sSoundState;

typedef struct SPWM_Object {
    uint32_t pwmBaseAddr;
    uint32_t pwmOutput;
    uint32_t prescalar;
    uint32_t periodTicks;
    uint32_t pwmGenerator;
    uint32_t pinConfig;
    uint32_t pwmOutputBit;
    uint32_t pwmOutputNum;
} SPWM_Object;

static SPWM_Object spwm = {
    .pwmBaseAddr = PWM0_BASE,
    .pwmOutput = PWM_OUT_2,
    .pwmOutputBit = PWM_OUT_2_BIT,
    .prescalar = 1,
    .periodTicks = 1000,
    .pwmGenerator = PWM_GEN_1,
    .pinConfig = PWMMSP432E4_PF2_M0PWM2,
};

void SPWM_intHandler(void)
{
    PWMGenIntClear(spwm.pwmBaseAddr, spwm.pwmGenerator, PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD);
    PWMPulseWidthSet(spwm.pwmBaseAddr, spwm.pwmOutput, 2000);
}

void SPWM_setup()
{
    uint8_t port = GPIOMSP432E4_getPortFromPinConfig(spwm.pinConfig);
    Power_setDependency(GPIOMSP432E4_getPowerResourceId(port));
    Power_setDependency(PowerMSP432E4_PERIPH_PWM0);

    GPIOPinConfigure(GPIOMSP432E4_getPinMapFromPinConfig(spwm.pinConfig));
    uint32_t gpioBaseAddress = GPIOMSP432E4_getGpioBaseAddr(
        GPIOMSP432E4_getPortFromPinConfig(spwm.pinConfig));
    GPIOPinTypePWM(gpioBaseAddress,
        GPIOMSP432E4_getPinFromPinConfig(spwm.pinConfig));

    uint32_t periodTicks = 120000000u / 64000u;

    PWMGenConfigure(spwm.pwmBaseAddr, spwm.pwmGenerator, PWM_GEN_MODE_UP_DOWN);
    PWMGenPeriodSet(spwm.pwmBaseAddr, spwm.pwmGenerator, periodTicks);

    PWMOutputInvert(spwm.pwmBaseAddr, spwm.pwmOutputBit, false);
    PWMOutputState(spwm.pwmBaseAddr, spwm.pwmOutputBit, true);
    PWMPulseWidthSet(spwm.pwmBaseAddr, spwm.pwmOutput, 6000);

    PWMGenIntTrigEnable(spwm.pwmBaseAddr, spwm.pwmGenerator, PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD);
    PWMIntEnable(spwm.pwmBaseAddr, PWM_INT_GEN_1);

    PWMGenEnable(spwm.pwmBaseAddr, spwm.pwmGenerator);
}

//*****************************************************************************
//
//! Handles the TIMER5A interrupt.
//!
//! This function responds to the TIMER5A interrupt, updating the duty cycle of
//! the output waveform in order to produce sound.  It is the application's
//! responsibility to ensure that this function is called in response to the
//! TIMER5A interrupt, typically by installing it in the vector table as the
//! handler for the TIMER5A interrupt.
//!
//! \return None.
//
//*****************************************************************************
void
SoundIntHandler(void)
{
    int32_t i32DutyCycle;

    //
    // If there is an adjustment to be made, the apply it and set allow the
    // update to be done on the next load.
    //
    if(g_sSoundState.i32RateAdjust)
    {
        g_sSoundState.ui32Period += g_sSoundState.i32RateAdjust;
        g_sSoundState.i32RateAdjust = 0;
        //TimerLoadSet(TIMER5_BASE, TIMER_A, g_sSoundState.ui32Period);
    }

    //
    // Clear the timer interrupt.
    //
    PWMGenIntClear(spwm.pwmBaseAddr, spwm.pwmGenerator, PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD);
    //ROM_TimerIntClear(TIMER5_BASE, TIMER_CAPA_EVENT);

    //
    // See if the startup ramp is in progress.
    //
    if(HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_STARTUP))
    {
        //
        // Increment the ramp count.
        //
        g_sSoundState.i32Step++;

        //
        // Increase the pulse width of the output by one clock.
        //
        //ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, g_sSoundState.i32Step);

        //
        // See if this was the last step of the ramp.
        //
        if(g_sSoundState.i32Step >= (g_sSoundState.ui32Period / 2))
        {
            //
            // Indicate that the startup ramp has completed.
            //
            HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_STARTUP) = 0;

            //
            // Set the step back to zero for the start of audio playback.
            //
            g_sSoundState.i32Step = 0;
        }

        //
        // There is nothing further to be done.
        //
        return;
    }

    //
    // See if the shutdown ramp is in progress.
    //
    if(HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_SHUTDOWN))
    {
        //
        // See if this was the last step of the ramp.
        //
        if(g_sSoundState.i32Step == 1)
        {
            //
            // Disable the output signals.
            //
            //ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, g_sSoundState.ui32Period);

            //
            // Clear the sound flags.
            //
            g_sSoundState.ui32Flags = 0;

            //
            // Disable the speaker amp.
            //
            //ROM_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_4, 0);
        }
        else
        {
            //
            // Decrement the ramp count.
            //
            g_sSoundState.i32Step--;

            //
            // Decrease the pulse width of the output by one clock.
            //
            //ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, g_sSoundState.i32Step);
        }

        //
        // There is nothing further to be done.
        //
        return;
    }

    //
    // Compute the value of the PCM sample based on the blended average of the
    // previous and current samples.  It should be noted that linear
    // interpolation does not produce the best results with sound (it produces
    // a significant amount of harmonic aliasing) but it is fast.
    //
    i32DutyCycle =
        (((g_sSoundState.pi16Samples[0] * (8 - g_sSoundState.i32Step)) +
          (g_sSoundState.pi16Samples[1] * g_sSoundState.i32Step)) / 8);

    //
    // Adjust the magnitude of the sample based on the current volume.  Since a
    // multiplicative volume control is implemented, the volume value
    // results in nearly linear volume adjustment if it is squared.
    //
    i32DutyCycle = (((i32DutyCycle * g_sSoundState.i32Volume *
                      g_sSoundState.i32Volume) / 65536) + 32768);

    //
    // Set the PWM duty cycle based on this PCM sample.
    //
    i32DutyCycle = (g_sSoundState.ui32Period * i32DutyCycle) / 65536;
    //ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, i32DutyCycle);
    PWMPulseWidthSet(spwm.pwmBaseAddr, spwm.pwmOutput, i32DutyCycle);

    //
    // Increment the sound step based on the sample rate.
    //
    if(HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_8KHZ))
    {
        g_sSoundState.i32Step = (g_sSoundState.i32Step + 1) & 7;
    }
    else if(HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_16KHZ))
    {
        g_sSoundState.i32Step = (g_sSoundState.i32Step + 2) & 7;
    }
    else if(HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_32KHZ))
    {
        g_sSoundState.i32Step = (g_sSoundState.i32Step + 4) & 7;
    }

    //
    // See if the next sample has been reached.
    //
    if(g_sSoundState.i32Step == 0)
    {
        //
        // Copy the current sample to the previous sample.
        //
        g_sSoundState.pi16Samples[0] = g_sSoundState.pi16Samples[1];

        //
        // Get the next sample from the buffer.
        //
        g_sSoundState.pi16Samples[1] =
            g_sSoundState.pi16Buffer[g_sSoundState.ui32Offset];

        //
        // Increment the buffer pointer.
        //
        g_sSoundState.ui32Offset++;
        if(g_sSoundState.ui32Offset == g_sSoundState.ui32Length)
        {
            g_sSoundState.ui32Offset = 0;
        }

        //
        // Call the callback function if one of the half-buffers has been
        // consumed.
        //
        if(g_sSoundState.pfnCallback)
        {
            if(g_sSoundState.ui32Offset == 0)
            {
                g_sSoundState.pfnCallback(1);
            }
            else if(g_sSoundState.ui32Offset == (g_sSoundState.ui32Length / 2))
            {
                g_sSoundState.pfnCallback(0);
            }
        }
    }
}

//*****************************************************************************
//
//! Initializes the sound driver.
//!
//! \param ui32SysClock is the frequency of the system clock.
//!
//! This function initializes the sound driver, preparing it to output sound
//! data to the speaker.
//!
//! The system clock should be as high as possible; lower clock rates reduces
//! the quality of the produced sound.  For the best quality sound, the system
//! should be clocked at 120 MHz.
//!
//! \note In order for the sound driver to function properly, the sound driver
//! interrupt handler (SoundIntHandler()) must be installed into the vector
//! table for the timer 5 subtimer A interrupt.
//!
//! \return None.
//
//*****************************************************************************
void
SoundInit(uint32_t ui32SysClock)
{
    //
    // Enable the peripherals used by the sound driver.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);

    //
    // Compute the PWM period based on the system clock.
    //
    g_sSoundState.ui32Period = ui32SysClock / 64000;

    //
    // Set the default volume.
    //
    g_sSoundState.i32Volume = 255;

#if 1
    SPWM_setup();
#else
    //
    // Configure the timer to run in PWM mode.
    //
    if((HWREG(TIMER5_BASE + TIMER_O_CTL) & TIMER_CTL_TBEN) == 0)
    {
        ROM_TimerConfigure(TIMER5_BASE, (TIMER_CFG_SPLIT_PAIR |
                                         TIMER_CFG_A_PWM |
                                         TIMER_CFG_B_PERIODIC));
    }
    ROM_TimerLoadSet(TIMER5_BASE, TIMER_A, g_sSoundState.ui32Period - 1);
    ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, g_sSoundState.ui32Period);
    ROM_TimerControlLevel(TIMER5_BASE, TIMER_A, true);

    //
    // Update the timer values on timeouts and not immediately.
    //
    TimerUpdateMode(TIMER5_BASE, TIMER_A, TIMER_UP_LOAD_TIMEOUT |
                                          TIMER_UP_MATCH_TIMEOUT);

    //
    // Configure the timer to generate an interrupt at every time-out event.
    //
    ROM_TimerIntEnable(TIMER5_BASE, TIMER_CAPA_EVENT);

    //
    // Enable the timer.  At this point, the timer generates an interrupt
    // every 15.625 us.
    //
    ROM_TimerEnable(TIMER5_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER5A);
#endif

    //
    // Clear the sound flags.
    //
    g_sSoundState.ui32Flags = 0;
}

//*****************************************************************************
//
//! Make adjustments to the sample period of the PWM audio.
//!
//! \param i32RateAdjust is a signed value of the adjustment to make to the
//! current sample period.
//!
//! This function allows the sample period to be adjusted if the application
//! needs to make small adjustments to the playback rate of the audio.  This
//! should only be used to makke smaller adjustments to the sample rate since
//! large changes cause distortion in the output.
//!
//! \return None.
//
//*****************************************************************************
void
SoundPeriodAdjust(int32_t i32RateAdjust)
{
    g_sSoundState.i32RateAdjust += i32RateAdjust;
}

//*****************************************************************************
//
//! Starts playback of a sound stream.
//!
//! \param pi16Buffer is a pointer to the buffer that contains the sound to
//! play.
//! \param ui32Length is the length of the buffer in samples.  This should be
//! a multiple of two.
//! \param ui32Rate is the sound playback rate; valid values are 8000, 16000,
//! 32000, and 64000.
//! \param pfnCallback is the callback function that is called when either half
//! of the sound buffer has been played.
//!
//! This function starts the playback of a sound stream contained in an
//! audio ping-pong buffer.  The buffer is played repeatedly until
//! SoundStop() is called.  Playback of the sound stream begins
//! immediately, so the buffer should be pre-filled with the initial sound
//! data prior to calling this function.
//!
//! \return Returns \b true if playback was started and \b false if it could
//! not be started (because something is already playing).
//
//*****************************************************************************
bool
SoundStart(int16_t *pi16Buffer, uint32_t ui32Length, uint32_t ui32Rate,
           void (*pfnCallback)(uint32_t ui32Half))
{
    //
    // Return without playing the buffer if something is already playing.
    //
    if(g_sSoundState.ui32Flags)
    {
        return(false);
    }

    //
    // Set the sample rate flag.
    //
    if(ui32Rate == 8000)
    {
        HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_8KHZ) = 1;
    }
    else if(ui32Rate == 16000)
    {
        HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_16KHZ) = 1;
    }
    else if(ui32Rate == 32000)
    {
        HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_32KHZ) = 1;
    }
    else if(ui32Rate == 64000)
    {
        HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_64KHZ) = 1;
    }
    else
    {
        return(false);
    }

    //
    // Enable the speaker amp.
    //
    //ROM_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_4, GPIO_PIN_4);

    //
    // Save the pointer to the buffer.
    //
    g_sSoundState.pi16Buffer = pi16Buffer;
    g_sSoundState.ui32Length = ui32Length;

    //
    // Save the pointer to the callback function.
    //
    g_sSoundState.pfnCallback = pfnCallback;

    //
    // Start playback from the beginning of the buffer.
    //
    g_sSoundState.ui32Offset = 0;

    //
    // Initialize the sample buffer with silence.
    //
    g_sSoundState.pi16Samples[0] = 0;
    g_sSoundState.pi16Samples[1] = 0;

    //
    // Start playback of the stream.
    //
    HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_STARTUP) = 1;
    HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_PLAY) = 1;

    //
    // Set the step for the startup ramp.
    //
    g_sSoundState.i32Step = 1;

    //
    // Enable the timer interrupt.
    //
    //ROM_TimerMatchSet(TIMER5_BASE, TIMER_A, 1);
    PWMPulseWidthSet(spwm.pwmBaseAddr, spwm.pwmOutput, 1);

    //
    // Success.
    //
    return(true);
}

//*****************************************************************************
//
//! Stops playback of the current sound stream.
//!
//! This function immediately stops playback of the current sound stream.  As
//! a result, the output is changed directly to the mid-point, possibly
//! resulting in a pop or click.  It is then ramped down to no output,
//! eliminating the current draw through the amplifier and speaker.
//!
//! \return None.
//
//*****************************************************************************
void
SoundStop(void)
{
    //
    // See if playback is in progress.
    //
    if((g_sSoundState.ui32Flags != 0) &&
       (HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_SHUTDOWN) == 0))
    {
        //
        // Temporarily disable the timer interrupt.
        //
        //ROM_IntDisable(INT_TIMER5A);
        PWMIntDisable(spwm.pwmBaseAddr, PWM_INT_GEN_1);

        //
        // Clear the sound flags and set the shutdown flag (to try to avoid a
        // pop, though one may still occur based on the current position of the
        // output waveform).
        //
        g_sSoundState.ui32Flags = 0;
        HWREGBITW(&g_sSoundState.ui32Flags, SOUND_FLAG_SHUTDOWN) = 1;

        //
        // Set the shutdown step to the first.
        //
        g_sSoundState.i32Step = g_sSoundState.ui32Period / 2;

        //
        // Reenable the timer interrupt.
        //
        //ROM_IntEnable(INT_TIMER5A);
        PWMIntEnable(spwm.pwmBaseAddr, PWM_INT_GEN_1);
    }
}

//*****************************************************************************
//
//! Determines if the sound driver is busy.
//!
//! This function determines if the sound driver is busy, either performing the
//! startup or shutdown ramp for the speaker or playing a sound stream.
//!
//! \return Returns \b true if the sound driver is busy and \b false otherwise.
//
//*****************************************************************************
bool
SoundBusy(void)
{
    //
    // The sound driver is busy if the sound flags are not zero.
    //
    return(g_sSoundState.ui32Flags != 0);
}

//*****************************************************************************
//
//! Sets the volume of the sound playback.
//!
//! \param i32Volume is the volume of the sound playback, specified as a value
//! between 0 (for silence) and 255 (for full volume).
//!
//! This function sets the volume of the sound playback.  Setting the volume to
//! 0 mutes the output, while setting the volume to 256 plays the sound
//! stream without any volume adjustment (that is, full volume).
//!
//! \return None.
//
//*****************************************************************************
void
SoundVolumeSet(int32_t i32Volume)
{
    //
    // Set the volume mulitplier to be used.
    //
    g_sSoundState.i32Volume = i32Volume;
}

//*****************************************************************************
//
//! Increases the volume of the sound playback.
//!
//! \param i32Volume is the amount by which to increase the volume of the
//! sound playback, specified as a value between 0 (for no adjustment) and 255
//! maximum adjustment).
//!
//! This function increases the volume of the sound playback relative to the
//! current volume.
//!
//! \return None.
//
//*****************************************************************************
void
SoundVolumeUp(int32_t i32Volume)
{
    //
    // Compute the new volume, limiting to the maximum if required.
    //
    i32Volume = g_sSoundState.i32Volume + i32Volume;
    if(i32Volume > 255)
    {
        i32Volume = 255;
    }

    //
    // Set the new volume.
    //
    g_sSoundState.i32Volume = i32Volume;
}

//*****************************************************************************
//
//! Decreases the volume of the sound playback.
//!
//! \param i32Volume is the amount by which to decrease the volume of the
//! sound playback, specified as a value between 0 (for no adjustment) and 255
//! maximum adjustment).
//!
//! This function decreases the volume of the sound playback relative to the
//! current volume.
//!
//! \return None.
//
//*****************************************************************************
void
SoundVolumeDown(int32_t i32Volume)
{
    //
    // Compute the new volume, limiting to the minimum if required.
    //
    i32Volume = g_sSoundState.i32Volume - i32Volume;
    if(i32Volume < 0)
    {
        i32Volume = 0;
    }

    //
    // Set the new volume.
    //
    g_sSoundState.i32Volume = i32Volume;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
