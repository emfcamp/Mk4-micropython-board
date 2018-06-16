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
/** ============================================================================
 *  @file       MSP_EXP432E401Y.h
 *
 *  @brief      MSP_EXP432E401Y Board Specific APIs
 *
 *  The MSP_EXP432E401Y header file should be included in an application as
 *  follows:
 *  @code
 *  #include <MSP_EXP432E401Y.h>
 *  @endcode
 *
 *  ============================================================================
 */

#ifndef __MSP_EXP432E401Y_H
#define __MSP_EXP432E401Y_H

#ifdef __cplusplus
extern "C" {
#endif

/* LEDs on MSP_EXP432E401Y are active high. */
#define MSP_EXP432E401Y_GPIO_LED_OFF (0)
#define MSP_EXP432E401Y_GPIO_LED_ON  (1)

/*!
 *  @def    MSP_EXP432E401Y_ADCName
 *  @brief  Enum of ADC channels on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_ADCName {
    MSP_EXP432E401Y_ADC0 = 0,
    MSP_EXP432E401Y_ADC1,

    MSP_EXP432E401Y_ADCCOUNT
} MSP_EXP432E401Y_ADCName;

/*!
 *  @def    MSP_EXP432E401Y_GPIOName
 *  @brief  Enum of LED names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_GPIOName {

// Inputs
    MSP_EXP432E401Y_GPIO_JOYC = 0,
    MSP_EXP432E401Y_GPIO_JOYU,
    MSP_EXP432E401Y_GPIO_JOYD,
    MSP_EXP432E401Y_GPIO_JOYL,
    MSP_EXP432E401Y_GPIO_JOYR,
    // MSP_EXP432E401Y_GPIO_BNT_MENU,
    MSP_EXP432E401Y_GPIO_SIM_STATUS,
    MSP_EXP432E401Y_GPIO_SIM_NETLIGHT,
    // MSP_EXP432E401Y_GPIO_SIM_RI,
    MSP_EXP432E401Y_GPIO_BQ_INT,
    // MSP_EXP432E401Y_GPIO_TCA_INT,
    MSP_EXP432E401Y_GPIO_LCD_TEAR,
    MSP_EXP432E401Y_CC_HOST_IRQ,
    // MSP_EXP432E401Y_GPIO_VBUS_DET,
    // MSP_EXP432E401Y_ADC_A_X,
    // MSP_EXP432E401Y_ADC_A_Y,
    // MSP_EXP432E401Y_ADC_CH3,
    MSP_EXP432E401Y_ADC_SPK,

// Outputs
    MSP_EXP432E401Y_GPIO_ETHLED0,
    MSP_EXP432E401Y_GPIO_ETHLED1,
    MSP_EXP432E401Y_GPIO_LED1,
    MSP_EXP432E401Y_GPIO_LED2,
    MSP_EXP432E401Y_TIM_WS2812,
    MSP_EXP432E401Y_GPIO_SIM_PWR_KEY,
    MSP_EXP432E401Y_PWM_MIC,
    MSP_EXP432E401Y_PWM_LCD_BLIGHT,
    MSP_EXP432E401Y_GPIO_LCD_DCX,
    MSP_EXP432E401Y_GPIO_LCD_RST,
    MSP_EXP432E401Y_LCD_CS,
    MSP_EXP432E401Y_GPIO_MUX_A,
    MSP_EXP432E401Y_GPIO_MUX_B,
    // MSP_EXP432E401Y_GPIO_T_X,
    // MSP_EXP432E401Y_GPIO_T_Y,
    // MSP_EXP432E401Y_GPIO_FET,
    // MSP_EXP432E401Y_GPIO_CH4,
    MSP_EXP432E401Y_FLASH_CS,
    MSP_EXP432E401Y_CC_RST,
    MSP_EXP432E401Y_CC_nHIB_pin,
    MSP_EXP432E401Y_CC_CS_pin,

    MSP_EXP432E401Y_GPIOCOUNT
} MSP_EXP432E401Y_GPIOName;

/*!
 *  @def    MSP_EXP432E401Y_I2CName
 *  @brief  Enum of I2C names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_I2CName {
    MSP_EXP432E401Y_I2C5 = 0,
    MSP_EXP432E401Y_I2C9,

    MSP_EXP432E401Y_I2CCOUNT
} MSP_EXP432E401Y_I2CName;

/*!
 *  @def    MSP_EXP432E401Y_NVSName
 *  @brief  Enum of NVS names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_NVSName {
    MSP_EXP432E401Y_NVSMSP432E40 = 0,

    MSP_EXP432E401Y_NVSCOUNT
} MSP_EXP432E401Y_NVSName;

/*!
 *  @def    MSP_EXP432E401Y_PWMName
 *  @brief  Enum of PWM names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_PWMName {
    MSP_EXP432E401Y_PWM0 = 0,

    MSP_EXP432E401Y_PWMCOUNT
} MSP_EXP432E401Y_PWMName;

/*!
 *  @def    MSP_EXP432E401Y_SPIName
 *  @brief  Enum of SPI names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_SPIName {
    MSP_EXP432E401Y_SPI2 = 0,
    MSP_EXP432E401Y_SPI3,
    MSP_EXP432E401Y_SPI0,

    MSP_EXP432E401Y_SPICOUNT
} MSP_EXP432E401Y_SPIName;

/*!
 *  @def    MSP_EXP432E401Y_UARTName
 *  @brief  Enum of UARTs on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_UARTName {
    MSP_EXP432E401Y_UART3 = 0,
    MSP_EXP432E401Y_UART2,

    MSP_EXP432E401Y_UARTCOUNT
} MSP_EXP432E401Y_UARTName;

/*!
 *  @def    MSP_EXP432E401Y_USBMode
 *  @brief  Enum of USB setup function on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_USBMode {
    MSP_EXP432E401Y_USBULPI,
    MSP_EXP432E401Y_USBDEVICE,
    MSP_EXP432E401Y_USBHOST
} MSP_EXP432E401Y_USBMode;

/*!
 *  @def    MSP_EXP432E401Y_USBMSCHFatFsName
 *  @brief  Enum of USBMSCHFatFs names on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_USBMSCHFatFsName {
    MSP_EXP432E401Y_USBMSCHFATFS0 = 0,

    MSP_EXP432E401Y_USBMSCHFATFSCOUNT
} MSP_EXP432E401Y_USBMSCHFATFSName;

/*
 *  @def    MSP_EXP432E401Y_WatchdogName
 *  @brief  Enum of Watchdogs on the MSP_EXP432E401Y dev board
 */
typedef enum MSP_EXP432E401Y_WatchdogName {
    MSP_EXP432E401Y_WATCHDOG0 = 0,

    MSP_EXP432E401Y_WATCHDOGCOUNT
} MSP_EXP432E401Y_WatchdogName;

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 *  This includes:
 *     - Enable clock sources for peripherals
 */
extern void MSP_EXP432E401Y_initGeneral(void);

/*!
 *  @brief  Initialize board specific GPIO settings
 *
 *  This function initializes the board specific GPIO settings and
 *  then calls the GPIO_init API to initialize the GPIO module.
 *
 *  The GPIOs controlled by the GPIO module are determined by the GPIO_PinConfig
 *  variable.
 */
extern void MSP_EXP432E401Y_initGPIO(void);

/*!
 *  @brief  Initialize board specific USB settings
 *
 *  This function initializes the board specific USB settings and pins based on
 *  the USB mode of operation.
 *
 *  @param      usbMode    USB mode of operation
 */
extern void MSP_EXP432E401Y_initUSB(MSP_EXP432E401Y_USBMode usbMode);


#ifdef __cplusplus
}
#endif

#endif /* __MSP_EXP432E401Y_H */
