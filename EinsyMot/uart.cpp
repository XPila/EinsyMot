//uart.cpp

#include "uart.h"
#include "Arduino.h"

extern "C" {

FILE _uart0io = {0};
FILE _uart1io = {0};
FILE* uart0io = &_uart0io;
FILE* uart1io = &_uart1io;

}

int uart0_putchar(char c, FILE *stream)
{
	Serial.write(c);
	return 0;
}
int uart0_getchar(FILE *stream)
{
	return Serial.read();
}

int uart1_putchar(char c, FILE *stream)
{
	Serial1.write(c);
	return 0;
}
int uart1_getchar(FILE *stream)
{
	return Serial1.read();
}


void uart0_init(void)
{
	Serial.begin(UART0_BDR, SERIAL_8N2); //serial0 - USB
	fdev_setup_stream(uart0io, uart0_putchar, uart0_getchar, _FDEV_SETUP_WRITE | _FDEV_SETUP_READ); //setup uart in/out stream
}

void uart1_init(void)
{
	Serial1.begin(UART1_BDR, SERIAL_8N2); //serial1
	fdev_setup_stream(uart1io, uart1_putchar, uart1_getchar, _FDEV_SETUP_WRITE | _FDEV_SETUP_READ); //setup uart in/out stream
}
