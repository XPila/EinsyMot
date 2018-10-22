//main.cpp
//test
//#include "main.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>
#include <avr/boot.h>
#include "uart.h"
#include "adc.h"
#include "lcd.h"
#include "spi.h"
#include "tmc2130.h"
#include "st4.h"
#include "einsy.h"
#include "cmd.h"


#if (UART_COM == 0)
FILE* uart_com = uart0io;
//#define uart_com uart0io
#elif (UART_COM == 1)
//FILE* uart_com = uart1io;
#define uart_com uart1io
#endif //(UART_COM == 0)


void setup_osc(void);



//initialization after reset
void setup(void)
{
//	uint16_t cnt;
//	int8_t ret;

	wdt_disable();

	setup_osc();

	uart0_init(); //uart0
	uart1_init(); //uart1

#if (UART_STD == 0)
	stdin = uart0io; // stdin = uart0
	stdout = uart0io; // stdout = uart0
#elif (UART_STD == 1)
	stdin = uart1io; // stdin = uart1
	stdout = uart1io; // stdout = uart1
#endif //(UART_STD == 1)

	uart_com = uart0io;

	cmd_in = uart_com;
	cmd_out = uart_com;
	cmd_err = uart_com;

#ifdef _SIMULATOR
	cmd_in = stdin;
	cmd_out = stdout;
	cmd_err = stderr;
#endif //_SIMULATOR

	fprintf_P(cmd_out, PSTR("start\n")); //startup message
	fflush(cmd_out);

	adc_init();

	lcd_init();
	fprintf_P(lcdout, PSTR(ESC_H(0,0)"Einsy motion\ntest")); //startup message
	fflush(lcdout);

	einsy_io_setup_pins();

	einsy_tmc_setup_pins();

	spi_init();

	tmc2130_init();

	tmc2130_set_cur(0, 20);
	tmc2130_set_cur(1, 30);
	tmc2130_set_cur(2, 30);
	tmc2130_set_cur(3, 30);

	tmc2130_set_sgt(0, 8);
	tmc2130_set_sgt(1, 8);
	tmc2130_set_sgt(2, 8);
	tmc2130_set_sgt(3, 8);

	// res - resolution [steps/unit]
	// sr0 - starting steprate [unit/s]
	// srm - maximum steprate [unit/s]
	// acc - acceleration [unit/s^2]
	// dec - deceleration [unit/s^2]
	//          axis  res  sr0   srm  acc  dec
	st4_setup_axis(0, 100,  10,  200, 650, 650);
	st4_setup_axis(1, 100,  10,  200, 650, 650);
	st4_setup_axis(2, 400,   2,   40, 100, 100);
	st4_setup_axis(3, 280,  20,   50, 400, 400);

#if 0
	st4_setup_axis(4, 100, 10, 210, 650, 650); //res=400ustep/mm, sr0=1mm/s, srm=10mm/s, acc=10mm/s^2, dec=10mm/s^2


	uint16_t dx = 20000;
	uint16_t dy = 20000;
	uint16_t dz = 0;
	uint16_t de = 0;
	uint32_t dd = ((uint32_t)dx * dx) + ((uint32_t)dy * dy) + ((uint32_t)dz * dz) + ((uint32_t)de * de);
	uint16_t d = sqrt(dd);

	st4_axis[0].srx.ui16.h = dx;
	st4_axis[0].cnt = 0;
	st4_axis[1].srx.ui16.h = dy;
	st4_axis[1].cnt = 0;
	st4_axis[2].srx.ui16.h = dz;
	st4_axis[2].cnt = 0;
	st4_axis[3].srx.ui16.h = de;
	st4_axis[3].cnt = 0;

	st4_axis[4].cnt = d;
	st4_axis[4].srx.ui16.l = 0;
	st4_axis[4].srx.ui16.h = st4_axis[4].sr0;
	st4_calc_move(4, d);
/*	st4_axis[4].cac = st4_axis[4].nac;
	st4_axis[4].cdc = st4_axis[4].ndc;
	st4_axis[4].crm = d - (st4_axis[4].cac + st4_axis[4].cdc);
	st4_axis[4].flg = 0x0f;*/

	st4_msk = 0x07;
	einsy_tmc_set_ena(0x07);

#endif

	_delay_ms(50);

	st4_setup_timer();

	OCR0B = 128;
	TIMSK0 |= (1 << OCIE0B);


	//st4_fprint_sr2d2_tab(cmd_err);
	//st4_fprint_sr_d2(cmd_err, ST4_THR_SR0, ST4_THR_SR4);
	//st4_gen_seg(ST4_THR_SR3, 6, 0);
}


//main loop
void loop(void)
{
//	st4_cycle();
	cmd_process();

#if 0
	int key = lcd_get();
	if (key > 0)
	{
		fputc(key, cmd_err);
	}
#endif

#if 0
	fprintf_P(lcdout, PSTR(ESC_H(0,0)"%04d %04d %04d %04d\n%04d %04d %04d %04d"),
		einsy_adc_val[0],
		einsy_adc_val[1],
		einsy_adc_val[2],
		einsy_adc_val[3],
		einsy_adc_val[4],
		einsy_adc_val[5],
		einsy_adc_val[6],
		einsy_adc_val[7]
		);
#endif

#if 0
	if (st4_msk & 1)
	{
		uint16_t sg = tmc2130_read_sg(0);
		uint16_t srxh = st4_axis[0].srx.ui16.h;
		uint16_t diag = einsy_tmc_get_diag();
		uint32_t ms = millis();
		fprintf_P(uart_com, PSTR("sgX=%5d diag=%d millis=%ld srxh=%u\n"), sg, diag, ms, srxh);
	}
#endif

#if 0
	if (einsy_tmc_get_ena())
	if (((st4_msk & 0x0f) == 0) || (millis() > 6000))
	{
		einsy_tmc_set_ena(0x00);
	}
#endif
}

void setup_osc(void)
{
/*
	CLKSEL0 = 0x15;    //Choose Crystal oscillator with BOD
	CLKSEL1 = 0x0f;    //CLKSEL1.EXCKSEL0..3 = 1;
	CLKPR = 0x80;      //Change the clock prescaler, first change bit CLKPCE
	CLKPR = 0x00;      //
*/
}

//extern "C" {
//}
