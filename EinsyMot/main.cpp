//main.cpp

//#include "main.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>
#include <avr/boot.h>
#include "uart.h"
#include "cmd.h"
#include "lcd.h"
#include "spi.h"
#include "tmc2130.h"
#include "st4.h"
#include "einsy.h"


#if (UART_COM == 0)
FILE* uart_com = uart0io;
#elif (UART_COM == 1)
FILE* uart_com = uart1io;
#endif //(UART_COM == 0)


void setup_osc(void);


//initialization after reset
void setup(void)
{
	uint16_t cnt;
	int8_t ret;

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

	cmd_in = uart_com;
	cmd_out = uart_com;
	cmd_err = uart_com;

	fprintf_P(uart_com, PSTR("start\n")); //startup message
	fflush(uart_com);

	lcd_init();

	einsy_io_setup_pins();

	einsy_tmc_setup_pins();

	spi_init();

	tmc2130_init();
	tmc2130_set_cur(0, 20);
	tmc2130_set_cur(1, 30);
	tmc2130_set_cur(2, 30);
	tmc2130_set_cur(3, 30);

	st4_setup_axis(0, 100, 10,  200, 650, 650); //res=100ustep/mm, sr0=5mm/s, srm=70mm/s, acc=200mm/s^2, dec=400mm/s^2
	st4_setup_axis(1, 100, 10,  200, 650, 650); //res=100ustep/mm, sr0=5mm/s, srm=70mm/s, acc=200mm/s^2, dec=400mm/s^2
	st4_setup_axis(2, 400, 2,  40, 100, 100);  //res=400ustep/mm, sr0=1mm/s, srm=15mm/s, acc=50mm/s^2,  dec=100mm/s^2
	st4_setup_axis(3, 280, 1,  10, 10, 10);   //res=280ustep/mm, sr0=1mm/s, srm=10mm/s, acc=10mm/s^2,  dec=10mm/s^2

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

	fprintf_P(lcdio, PSTR(ESC_H(0,0)"Einsy motion\ntest")); //startup message

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

// Timer 0 is shared with millies
ISR(TIMER0_COMPB_vect)
{
	lcd_100us(); //slower lcd (full screen ~64ms)
}
