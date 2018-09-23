//swdelay.c - Software delay functions
#include "swdelay.h"


//delay functions

#if (F_CPU == 16000000)

void _swdelay_625ns(void)
{ }                          // 5cycles call + 5cycles ret = 625ns@16MHz

void _swdelay_10us(void)
{                            // 5cycles call
	asm("ldi r18, 0");       // 1cycle
	asm("inc r18");          // 1cycle
	asm("cpi r18, 39");      // 1cycle
	asm("brne .-6");         // 1/2cycles
}                            // 5cycles ret

void _swdelay_40us(void)
{                            // 5cycles call
	asm("ldi r18, 0");       // 1cycle
	asm("inc r18");          // 1cycle
	asm("cpi r18, 159");     // 1cycle
	asm("brne .-6");         // 1/2cycles
}                            // 5cycles ret

void _swdelay_n10us(void)
{                            // 5cycles call
	asm("call _swdelay_10us"); // 10us
	asm("dec r19");          // 1cycle
	asm("cpi r19, 0");       // 1cycle
	asm("brne .-10");        // 1/2cycles
}                            // 5cycles ret

void _swdelay_n40us(void)
{                            // 5cycles call
	asm("call _swdelay_40us"); // 40us
	asm("dec r19");          // 1cycle
	asm("cpi r19, 0");       // 1cycle
	asm("brne .-10");        // 1/2cycles
}                            // 5cycles ret

#endif //(F_CPU == 16000000)
