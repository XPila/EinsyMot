//uart.h
#ifndef _UART_H
#define _UART_H

#include <inttypes.h>
#include <stdio.h>
#include "config.h"


extern "C" {
extern FILE* uart0io;
extern FILE* uart1io;
}


extern void uart0_init(void);

extern void uart1_init(void);


#endif //_UART_H
