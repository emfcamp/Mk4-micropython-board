/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

//#include STM32_HAL_H
#include "py/mphal.h"
#include <ti/drivers/SPI.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>


SPI_Handle spi_h;
PWM_Handle pwm_h;

SPI_Transaction trans;
#define DMA_BUFF_LEN 100
uint8_t dma_buffer[DMA_BUFF_LEN*2];
uint16_t dma_buffer_i;

extern orientation_t blit_rotation;



static GFXINLINE void change_spi_speed(uint32_t clk) {
    SPI_close(spi_h);
    SPI_Params params;
    SPI_Params_init(&params);
    params.bitRate = clk;
    params.dataSize = 8;
    params.frameFormat = 0;
    //params.transferMode = SPI_MODE_CALLBACK;
    spi_h = SPI_open(MICROPY_HW_UGFX_SPI, &params);   
}


static GFXINLINE void init_board(GDisplay *g) {
	// As we are not using multiple displays we set g->board to NULL as we don't use it.


    SPI_Params params;
    SPI_Params_init(&params);
    params.bitRate = 10000000;
    params.dataSize = 8;
    params.frameFormat = 0;
    //params.transferMode = SPI_MODE_CALLBACK;


    spi_h = SPI_open(MICROPY_HW_UGFX_SPI, &params);
    
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
	if(state) {
		// reset lcd
		//GPIO_clear_pin(MICROPY_HW_UGFX_PORT_RST, MICROPY_HW_UGFX_PIN_RST);
        //GPIOPinWrite(MICROPY_HW_UGFX_PORT_RST, MICROPY_HW_UGFX_PIN_RST, 0);
        GPIO_write(MICROPY_HW_UGFX_PIN_RST, 0);
	} else {
		//GPIO_set_pin(MICROPY_HW_UGFX_PORT_RST, MICROPY_HW_UGFX_PIN_RST);
        //GPIOPinWrite(MICROPY_HW_UGFX_PORT_RST, MICROPY_HW_UGFX_PIN_RST, MICROPY_HW_UGFX_PIN_RST);
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
	//GPIOPinWrite(MICROPY_HW_UGFX_PORT_CS, MICROPY_HW_UGFX_PIN_CS, MICROPY_HW_UGFX_PIN_CS);  //CS high
    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}
static GFXINLINE void write_index(GDisplay *g, uint16_t index) {
	(void) g;

	//GPIOPinWrite(MICROPY_HW_UGFX_PORT_CS, MICROPY_HW_UGFX_PIN_CS, 0);  //CS low
	//GPIOPinWrite(MICROPY_HW_UGFX_PORT_A0, MICROPY_HW_UGFX_PIN_A0, 0);  //CMD low
    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);
    GPIO_write(MICROPY_HW_UGFX_PIN_A0, 0);

	//HAL_SPI_Transmit(&ili_spi, &index, 1, 1000);


    trans.txBuf = &index;
    trans.rxBuf = NULL;
    trans.count = 1;

    (void)SPI_transfer(spi_h, &trans);
	//GPIOPinWrite(MICROPY_HW_UGFX_PORT_A0, MICROPY_HW_UGFX_PIN_A0, MICROPY_HW_UGFX_PIN_A0);  //CMD high
    GPIO_write(MICROPY_HW_UGFX_PIN_A0, 1);
    GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
}
static GFXINLINE void write_data(GDisplay *g, uint16_t data) {
	(void) g;
	//HAL_SPI_Transmit(&ili_spi, &data, 1, 1000);
GPIO_write(MICROPY_HW_UGFX_PIN_CS, 0);
    trans.txBuf = &data;
    trans.rxBuf = NULL;
    trans.count = 1;
    (void)SPI_transfer(spi_h, &trans);
    //GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
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
   
   uint8_t c_low = data&0xff;
   uint8_t c_high = data>>8;
   for (int i = 0; i < DMA_BUFF_LEN*2;){
      dma_buffer[i] = c_high;
      i++;
      dma_buffer[i] = c_low;
      i++;
   }
   
   trans.txBuf = &dma_buffer;
   trans.rxBuf = NULL;
   
   while(cnt >= DMA_BUFF_LEN){         
      trans.count = DMA_BUFF_LEN*2;
      (void)SPI_transfer(spi_h, &trans);
      cnt -= DMA_BUFF_LEN;
   }
   trans.count = cnt*2;
   (void)SPI_transfer(spi_h, &trans);

   
   //GPIO_write(MICROPY_HW_UGFX_PIN_CS, 1);
   
   
}

static GFXINLINE uint16_t read_data(GDisplay *g) {
	(void) g;
	uint8_t d;
	//HAL_SPI_Receive(&ili_spi, &d, 1, 1000);
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
