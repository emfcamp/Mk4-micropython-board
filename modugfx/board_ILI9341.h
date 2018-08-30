/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "py/mphal.h"

#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPIMSP432E4DMA.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>

#include <ti/devices/msp432e4/driverlib/ssi.h>

#define SPI_DIRECT 1

#if SPI_DIRECT
static uint32_t spiBase = 0x0;
#endif

static SPI_Handle spi_h;
#ifdef MICROPY_HW_UGFX_BL_PWM
static PWM_Handle pwm_h;
#endif

static SPI_Transaction trans;

#define DMA_BUFF_LEN 80
static uint8_t dma_buffer[DMA_BUFF_LEN*2];
static uint16_t dma_buffer_i;

extern orientation_t blit_rotation;

#if SPI_DIRECT
static inline void Direct_write(uint32_t base, uint32_t value)
{
    SSIEnable(spiBase);
    SSIDataPut(spiBase, value);
    while (SSIBusy(spiBase));
    SSIDisable(spiBase);
}
#endif

static GFXINLINE void change_spi_speed(uint32_t clk) {
    SPI_close(spi_h);
    SPI_Params params;
    SPI_Params_init(&params);
    params.bitRate = clk;
    params.dataSize = 8;
    params.frameFormat = 0;
    spi_h = SPI_open(MICROPY_HW_UGFX_SPI, &params);
}


static GFXINLINE void init_board(GDisplay *g) {
    // As we are not using multiple displays we set g->board to NULL as we don't use it.
    //g->board = NULL;

    SPI_Params params;
    SPI_Params_init(&params);
    params.bitRate = 30000000;
    params.dataSize = 8;
    params.frameFormat = 0;

    spi_h = SPI_open(MICROPY_HW_UGFX_SPI, &params);

#if SPI_DIRECT
    spiBase = ((SPIMSP432E4DMA_HWAttrs *)(spi_h->hwAttrs))->baseAddr;
#endif

#ifdef MICROPY_HW_UGFX_BL_PWM
    PWM_Params pwmParams;
    PWM_Params_init(&pwmParams);
    pwmParams.idleLevel = PWM_IDLE_LOW;      // Output low when PWM is not running
    pwmParams.periodUnits = PWM_PERIOD_HZ;   // Period is in Hz
    pwmParams.periodValue = 10000;           // 10kHz
    pwmParams.dutyUnits = PWM_DUTY_FRACTION; // Duty is in fractional percentage
    pwmParams.dutyValue = PWM_DUTY_FRACTION_MAX/2; // 50% initial duty cycle

    pwm_h = PWM_open(MICROPY_HW_UGFX_BL_PWM, &pwmParams);

    PWM_start(pwm_h);
#else
#ifdef MICROPY_HW_UGFX_PIN_BL
    GPIO_write(MICROPY_HW_UGFX_PIN_BL, 1);
#endif
#endif

    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
    GPIO_write(MICROPY_HW_UGFX_PIN_RST, 1);
    GPIO_write(MICROPY_HW_UGFX_PIN_A0, 1);
}

static GFXINLINE void post_init_board(GDisplay *g) {
	(void) g;
}

static inline void set_blit_rotation(orientation_t o){
    blit_rotation = o;
}

static GFXINLINE void setpin_reset(GDisplay *g, bool_t state) {
    (void) g;
    if (state) {
        GPIO_write(MICROPY_HW_UGFX_PIN_RST, 0);
    }
    else {
        GPIO_write(MICROPY_HW_UGFX_PIN_RST, 1);
    }
}

static GFXINLINE void set_backlight(GDisplay *g, uint8_t percent) {
    (void) g;
#ifdef MICROPY_HW_UGFX_BL_PWM
   PWM_setDuty(pwm_h,(PWM_DUTY_FRACTION_MAX / 100)*percent);
#endif
}

static GFXINLINE void acquire_bus(GDisplay *g) {
    (void) g;
}

static GFXINLINE void release_bus(GDisplay *g) {
    (void) g;
    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}

static GFXINLINE void write_index(GDisplay *g, uint16_t index) {
    (void) g;

    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);
    GPIO_write(MICROPY_HW_UGFX_PIN_A0, 0);

    //System_printf("%x ", index);
#if  SPI_DIRECT
    Direct_write(spiBase, index);
#else
    trans.txBuf = &index;
    trans.rxBuf = NULL;
    trans.count = 1;

    (void)SPI_transfer(spi_h, &trans);
#endif

    GPIO_write(MICROPY_HW_UGFX_PIN_A0, 1);
    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}

static GFXINLINE void write_data(GDisplay *g, uint16_t data) {
    (void) g;

    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);

#if SPI_DIRECT
    Direct_write(spiBase, data);
#else
    trans.txBuf = &data;
    trans.rxBuf = NULL;
    trans.count = 1;
    (void)SPI_transfer(spi_h, &trans);
#endif
}

static GFXINLINE void write_data16_block_flush(GDisplay *g) {
   (void) g;

   GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);

   trans.txBuf = &dma_buffer;
   trans.rxBuf = NULL;
   trans.count = dma_buffer_i;
   (void)SPI_transfer(spi_h, &trans);

   dma_buffer_i = 0;

   //GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}

static GFXINLINE void write_data16_block(GDisplay *g, uint16_t data) {

   dma_buffer[dma_buffer_i++] = data>>8;
   dma_buffer[dma_buffer_i++] = data&0xFF;

   if (dma_buffer_i >= DMA_BUFF_LEN*2)
      write_data16_block_flush(g);
}

static GFXINLINE void write_data16_repeated(GDisplay *g, uint16_t data, uint32_t cnt) {
    (void) g;

   GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);

   uint16_t be_data = ((data & 0xff00) >> 8) | ((data & 0xff) << 8);
   uint16_t * buf = (uint16_t *)dma_buffer;
   for (uint32_t i = 0; i < DMA_BUFF_LEN; i++) {
       *buf++ = be_data;
   }

   trans.txBuf = &dma_buffer;
   trans.rxBuf = NULL;
   trans.count = DMA_BUFF_LEN * 2;

   while(cnt >= DMA_BUFF_LEN){
      (void)SPI_transfer(spi_h, &trans);
      cnt -= DMA_BUFF_LEN;
   }
   trans.count = cnt * 2;
   (void)SPI_transfer(spi_h, &trans);

   //GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}

static GFXINLINE uint16_t read_data(GDisplay *g) {
    (void) g;
    uint8_t d;

    SPI_Transaction trans;
    trans.txBuf = NULL;
    trans.rxBuf = &d;
    trans.count = 1;
    (void)SPI_transfer(spi_h, &trans);

    return d;
}

static GFXINLINE void setreadmode(GDisplay *g) {
    (void) g;
}

static GFXINLINE void setwritemode(GDisplay *g) {
    (void) g;
}

#endif /* _GDISP_LLD_BOARD_H */
