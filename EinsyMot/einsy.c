
#include "einsy.h"
#include <avr/io.h>


void einsy_io_setup_pins(void)
{
	//fan output signals
	DDRH |= 0x28; //PH3,5 out
	PORTH &= ~0x28; //PH3,5 low
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
