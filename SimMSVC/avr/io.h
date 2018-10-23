// io.h
#ifndef _IO_H
#define _IO_H

#include <inttypes.h>


#define OCIE0B 0
#define OCIE1A 0
#define OCIE3A 0
#define WGM13 0
#define WGM12 0
#define WGM11 0
#define WGM10 0
#define COM1A0 0
#define COM1B0 0
#define COM3A0 0
#define COM3B0 0
#define CS10 0
#define CS10 0
#define ADPS2 0
#define ADPS1 0
#define ADPS0 0
#define REFS0 0
#define ADEN 0
#define MUX5 0
#define ADSC 0

#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern unsigned char OCR0B;
extern unsigned char TIMSK0;

extern unsigned short OCR1A;
extern unsigned short OCR1B;
extern unsigned short OCR1C;
extern unsigned short TCNT1;
extern unsigned char TCCR1A;
extern unsigned char TCCR1B;
extern unsigned char TIMSK1;

extern unsigned short OCR3A;
extern unsigned short OCR3B;
extern unsigned short OCR3C;
extern unsigned short TCNT3;
extern unsigned char TCCR3A;
extern unsigned char TCCR3B;
extern unsigned char TIMSK3;

extern unsigned char DDRA;
extern unsigned char DDRB;
extern unsigned char DDRC;
extern unsigned char DDRD;
extern unsigned char DDRE;
extern unsigned char DDRF;
extern unsigned char DDRG;
extern unsigned char DDRH;
extern unsigned char DDRI;
extern unsigned char DDRJ;
extern unsigned char DDRK;
extern unsigned char DDRL;

extern unsigned char PORTA;
extern unsigned char PORTB;
extern unsigned char PORTC;
extern unsigned char PORTD;
extern unsigned char PORTE;
extern unsigned char PORTF;
extern unsigned char PORTG;
extern unsigned char PORTH;
extern unsigned char PORTI;
extern unsigned char PORTJ;
extern unsigned char PORTK;
extern unsigned char PORTL;

extern unsigned char PINA;
extern unsigned char PINB;
extern unsigned char PINC;
extern unsigned char PIND;
extern unsigned char PINE;
extern unsigned char PINF;
extern unsigned char PING;
extern unsigned char PINH;
extern unsigned char PINI;
extern unsigned char PINJ;
extern unsigned char PINK;
extern unsigned char PINL;

extern unsigned char SPCR; //0x2c
#define SPIE   7
#define SPE    6
#define DORD   5
#define MSTR   4
#define CPOL   3
#define CPHA   2
#define SPR1   1
#define SPR0   0

extern unsigned char SPSR; //0x2d
#define SPIF   7
#define WCOL   6
#define SPI2X  0

extern unsigned char SPDR;

extern unsigned char ADCSRA;
extern unsigned char ADCSRB;
extern unsigned char ADMUX;
extern unsigned char DIDR0;
extern unsigned char DIDR2;
extern unsigned short ADC;


extern unsigned char MCUSR;

extern unsigned char WDTCSR;
#define WDRF 0
#define WDCE 0
#define WDE 0

extern unsigned char CLKSEL0;
extern unsigned char CLKSEL1;
extern unsigned char CLKPR;
extern unsigned char UHWCON;
extern unsigned char USBCON;

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_IO_H
