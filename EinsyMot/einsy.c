
#include "einsy.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "adc.h"
#include "lcd.h"


uint16_t einsy_adc_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void einsy_io_setup_pins(void)
{
	//fan output signals
	DDRH |= 0x28; //PH3,5 out
	PORTH &= ~0x28; //PH3,5 low
}

uint8_t einsy_get_fans(void)
{
	uint8_t mask = PORTH & 0x28;
	if (mask & 0x08) mask &= ~0x01;
	if (mask & 0x20) mask &= ~0x02;
	mask &= 0x0f;
	return mask;
}

void einsy_set_fans(uint8_t mask)
{
	mask &= 0x03;
	if (mask & 0x01) mask |= 0x08; //bit3 high
	if (mask & 0x02) mask |= 0x20; //bit5 high
	mask &= 0x28;
	PORTH = ((PORTH & ~0x28) | mask);
}

//100k ParCan thermistor (104GT-2), top rating 300C
//32 pairs  raw-C
const uint16_t PROGMEM table_nozzle[] =
{
    1,  713,
   17,  300,
   20,  290,
   23,  280,
   27,  270,
   31,  260,
   37,  250,
   43,  240,
   51,  230,
   61,  220,
   73,  210,
   87,  200,
  106,  190,
  128,  180,
  155,  170,
  189,  160,
  230,  150,
  278,  140,
  336,  130,
  402,  120,
  476,  110,
  554,  100,
  635,   90,
  713,   80,
  784,   70,
  846,   60,
  897,   50,
  937,   40,
  966,   30,
  986,   20,
 1000,   10,
 1010,    0
};

int16_t einsy_calc_temp_nozzle(uint16_t raw)
{
	uint8_t c = sizeof(table_nozzle) >> 1;
	uint8_t n = c >> 1;
	uint8_t i;
	uint16_t ri;
	uint16_t ti;
	uint16_t ri1;
	uint16_t ti1;
	while (1)
	{
		i = n << 1;
		ri = table_nozzle[i + 0];
		ti = table_nozzle[i + 1];
		if ((raw == ri) || (n == c) || (n == 0))
			return ti;
		else if (raw > ri)
		{
			ri1 = table_nozzle[i + 2];
			ti1 = table_nozzle[i + 3];
			if (raw == ri1)
				return ti1;
			else if (raw < ri1)
			{
				return ti + (ti1 - ti) * (raw - ri) / (ri1 - ri);
			}
			else //(raw > ri1)
				n += (c - n) / 2;
		}
		else //(raw < ri)
		{
			ri1 = table_nozzle[i - 2];
			ti1 = table_nozzle[i - 1];
			if (raw == ri1)
				return ti1;
			else if (raw < ri1)
			{
			}
			else //(raw > ri1)
				n >>= 1;
		}
	}
}

//100k bed thermistor
//51 pairs  raw-C
const uint16_t PROGMEM table_bed[] =
{
  23, 300,
  25, 295,
  27, 290,
  28, 285,
  31, 280,
  33, 275,
  35, 270,
  38, 265,
  41, 260,
  44, 255,
  48, 250,
  52, 245,
  56, 240,
  61, 235,
  66, 230,
  71, 225,
  78, 220,
  84, 215,
  92, 210,
 100, 205,
 109, 200,
 120, 195,
 131, 190,
 143, 185,
 156, 180,
 171, 175,
 187, 170,
 205, 165,
 224, 160,
 245, 155,
 268, 150,
 293, 145,
 320, 140,
 348, 135,
 379, 130,
 411, 125,
 445, 120,
 480, 115,
 516, 110,
 553, 105,
 591, 100,
 628,  95,
 665,  90,
 702,  85,
 737,  80,
 770,  75,
 801,  70,
 830,  65,
 857,  60,
 881,  55,
 903,  50,
 922,  45,
 939,  40,
 954,  35,
 966,  30,
 977,  25,
 985,  20,
 993,  15,
 999,  10,
1004,   5,
1008,   0
};

int16_t einsy_calc_temp_bed(uint16_t raw)
{
	return 0;
}

int16_t einsy_calc_temp_ambient(uint16_t raw)
{
	return 0;
}

int16_t einsy_calc_temp_pinda(uint16_t raw)
{
	return 0;
}

void einsy_tmc_setup_pins(void)
{
	//chipselect signals
	DDRG |= 0x05; //PG0,2 out
	DDRK |= 0x30; //PK5,4 out
	PORTG |= 0x05; //PG0,2 high
	PORTK |= 0x30; //PK5,4 high
	//enable signals
	DDRA |= 0xf0; //PA4..7 out
	PORTA |= ~0xf0; //PA4..7 high
	//direction signals
	DDRL |= 0x47; //PL0..2,6 out
	PORTL &= ~0x47; //PL0..2,6 low
	//diag signals
	DDRK &= ~0xcc; //PK2,7,6,3 in
	//step signals
	DDRC |= 0x0f; //PC0..3 out
	PORTC &= ~0x0f; //PC0..3 low
}

//tmc chipselect control
//X -PIN41, PG0 0x01
//Y -PIN39, PG2 0x04
//Z -PIN67, PK5 0x20
//E -PIN66, PK4 0x10

void einsy_tmc_cs_low(uint8_t axis)
{
	switch (axis)
	{
	case 0:
		PORTG &= ~0x01;
		break;
	case 1:
		PORTG &= ~0x04;
		break;
	case 2:
		PORTK &= ~0x20;
		break;
	case 3:
		PORTK &= ~0x10;
		break;
	}
}

void einsy_tmc_cs_high(uint8_t axis)
{
	switch (axis)
	{
	case 0:
		PORTG |= 0x01;
		break;
	case 1:
		PORTG |= 0x04;
		break;
	case 2:
		PORTK |= 0x20;
		break;
	case 3:
		PORTK |= 0x10;
		break;
	}
}

//tmc enable control
//X -PIN29, PA7
//Y -PIN28, PA6
//Z -PIN27, PA5
//E -PIN26, PA4

uint8_t einsy_tmc_get_ena(void)
{
	uint8_t mask = PORTA | 0x0f;
	if (mask & 0x80) mask &= ~0x01;
	if (mask & 0x40) mask &= ~0x02;
	if (mask & 0x20) mask &= ~0x04;
	if (mask & 0x10) mask &= ~0x08;
	mask &= 0x0f;
	return mask;
}

void einsy_tmc_set_ena(uint8_t mask)
{
	mask |= 0xf0;
	if (mask & 0x08) mask &= ~0x10;
	if (mask & 0x04) mask &= ~0x20;
	if (mask & 0x02) mask &= ~0x40;
	if (mask & 0x01) mask &= ~0x80;
	mask &= 0xf0;
	PORTA = ((PORTA & ~0xf0) | mask);
}

//tmc direction control
//X -PIN49, PL0 0x01
//Y -PIN48, PL1 0x02
//Z -PIN47, PL2 0x04
//E -PIN43, PL6 0x40

uint8_t einsy_tmc_get_dir(void)
{
	uint8_t mask = (PORTL & 0x47);
	if (mask & 0x40) mask |= 0x08;
	mask &= 0x0f;
	return mask;
}

void einsy_tmc_set_dir(uint8_t mask)
{
	if (mask & 0x08) mask |= 0x40;
	mask &= 0x47;
	PORTL = ((PORTL & ~0x47) | mask);
}

//tmc diag signal
//X -PIN64, PK2 0x04 // !!! changed from 40 (EINY03)
//Y -PIN69, PK7 0x80
//Z -PIN68, PK6 0x40
//E -PIN65, PK3 0x08

uint8_t einsy_tmc_get_diag(void)
{
	uint8_t mask = (PINK & 0xcc);
	if (mask & 0x04) mask |= 0x01;
	if (mask & 0x80) mask |= 0x02;
	if (mask & 0x40) mask |= 0x04;
	mask &= 0x0f;
	return mask;
}

//tmc step control
//X -PIN37, PC0 0x01
//Y -PIN36, PC1 0x02
//Z -PIN35, PC2 0x04
//E -PIN34, PC3 0x08

void einsy_tmc_do_step(uint8_t mask)
{
	PORTC |= mask;
	asm("nop");
	PORTC &= ~0x0f;
	asm("nop");
}

// Timer 0 is shared with millies
ISR(TIMER0_COMPB_vect)
{
	adc_cycle(); //
	lcd_cycle(); //slower lcd (full screen ~64ms)
}

void adc_ready(void)
{
	uint8_t i;
	for (i = 0; i < 7; i++)
		einsy_adc_val[i] = adc_val[i] >> 4;
}

