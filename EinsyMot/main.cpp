//main.cpp
/*        
st4_msk = 0x00; // vypne vsechny pohyby
*/

//#include "main.h"
#include "swdelay.h"
#include <stdio.h>
#include <string.h>
#include <Arduino.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/delay.h>
#include <avr/boot.h>
#include <math.h>
#include "uart.h"
//#include "adc.h"
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

#define LCD_BTN_PIN PH6
#define LCD_BTN_DDR DDRH
#define LCD_BTN_PORT PINH

void lcd_blank ( void );
void setup_osc(void);

int loop_cnt = 0;
typedef void (*menu_func_t)(void);
menu_func_t menu = 0;
menu_func_t actual_menu = 0;
int item_cur = 0;
int item_top = 0;

void einsy_tmc_set_ena_axis(uint8_t axis,uint8_t en);
void einsy_tmc_set_dir_axis(uint8_t axis,uint8_t dir);

void menu_main ( void );

void ports_init ( void );

// Timer 0 is shared with millies
ISR(TIMER0_COMPB_vect)
{
  lcd_cycle();
}

//INT6 - pri pruchodu sroubu kolem PINDY
ISR(INT6_vect)
{
}

//initialization after reset
void setup(void)
{
//-------------nastaveni portu
  ports_init ();
//-----------------------

  wdt_disable();
  setup_osc(); //nastaveni oscilatoru CPU

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

  fflush(uart_com);

  einsy_io_setup_pins();
  einsy_tmc_setup_pins();

  spi_init();
  lcd_init();

//-------------TMC block start
  einsy_io_setup_pins();
  einsy_tmc_setup_pins();
  tmc2130_init();
  st4_setup_timer();
  OCR0B = 128;
  TIMSK0 |= (1 << OCIE0B);

  lcd_blank();
//-------------nacteni hodnot z EEPROM
/*
  cam_mode = eeprom_read_word ( ( uint16_t* ) MODE_ADDR );
  if ( cam_mode == MODE_BOTH ) cam_mode = MODE_BOTH; 
  else cam_mode = MODE_ONE ;
  eeprom_write_word ( ( uint16_t* ) MODE_ADDR, cam_mode );
*/

  einsy_tmc_set_ena ( 0x00 ); // disable all
  lcd_blank();
  fprintf_P(lcdout, PSTR(ESC_H(0,0)"Startujem!"));
  lcd_blank();
  menu = menu_main;
}

void menu_main(void)
{
  int item_max = 3; // pocet polozek - 1
  int key = lcd_get();
  if (loop_cnt == 0) fprintf_P(lcdout, PSTR(ESC_H(0,0)));
  if (item_top > item_cur) item_top = item_cur;
  if (item_top < (item_cur - 3)) item_top = (item_cur - 3); //scroll (+1)
  int item; for (item = item_top; item < (item_top + 4); item++) // pocet zobrazenych polozek
  {
    switch (item)
    {
    case 0:
      if (loop_cnt == 0) fprintf_P(lcdout, PSTR("%c0\n"), (item == item_cur)?'>':' ');
      else if ((item == item_cur) && (key == '\n')) 
      {
        item_cur = 0;
        item_top = 0;
        lcd_blank();
        menu = menu_main;
      }
      break;
    case 1:
      if (loop_cnt == 0) fprintf_P(lcdout, PSTR("%c1\n"), (item == item_cur)?'>':' ');
      else if ((item == item_cur) && (key == '\n'))
      {
        item_cur = 0;
        item_top = 0;
        lcd_blank();
        menu = menu_main;
        return;
      }
      break;
    case 2:
      if (loop_cnt == 0) fprintf_P(lcdout, PSTR("%c2\n"), (item == item_cur)?'>':' ');
      else if ((item == item_cur) && (key == '\n'))
      {
        item_cur = 0;
        item_top = 0;
        lcd_blank();
        menu = menu_main;
        return;
      }
      break;
    case 3:
      if (loop_cnt == 0) fprintf_P(lcdout, PSTR("%c3\n"), (item == item_cur)?'>':' ');
      else if ((item == item_cur) && (key == '\n'))
      {
        item_cur = 0;
        item_top = 0;
        lcd_blank();
        menu = menu_main;
        return;
      }
      break;
    }
  }
  if (key == '+')
  {
    item_cur++;
    loop_cnt = -1;
  }
  else if (key == '-')
  {
    item_cur--;
    loop_cnt = -1;
  }
  if (item_cur < 0) item_cur = 0;
  if (item_cur > item_max) item_cur = item_max;
} 

//main loop
void loop(void)
{
	cmd_process();
	loop_cnt++;
	if (loop_cnt > 10000) loop_cnt = 0;
	{
		(*menu)();
	}

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

void lcd_blank ( void )
{
  fprintf_P(lcdout, PSTR(ESC_H(0,0)"                    "));
  fprintf_P(lcdout, PSTR(ESC_H(0,1)"                    "));
  fprintf_P(lcdout, PSTR(ESC_H(0,2)"                    "));
  fprintf_P(lcdout, PSTR(ESC_H(0,3)"                    "));
}

void ports_init ( void )
{
}

void einsy_tmc_set_ena_axis ( uint8_t axis, uint8_t en )
{
  if (en)
    switch (axis)
    {
    case 3: PORTA &= ~0x10; break;
    case 2: PORTA &= ~0x20; break;
    case 1: PORTA &= ~0x40; break;
    case 0: PORTA &= ~0x80; break;
    }
  else
    switch (axis)
    {
    case 3: PORTA |= 0x10; break;
    case 2: PORTA |= 0x20; break;
    case 1: PORTA |= 0x40; break;
    case 0: PORTA |= 0x80; break;
    }
}

void einsy_tmc_set_dir_axis (uint8_t axis, uint8_t dir )
{
  if (dir)
    switch (axis)
    {
    case 0: PORTL |= 0x01; break;
    case 1: PORTL |= 0x02; break;
    case 2: PORTL |= 0x04; break;
    case 3: PORTL |= 0x40; break;
    }
  else
    switch (axis)
    {
    case 0: PORTL &= ~0x01; break;
    case 1: PORTL &= ~0x02; break;
    case 2: PORTL &= ~0x04; break;
    case 3: PORTL &= ~0x40; break;
    }
}

