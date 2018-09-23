//swdelay.h - Software delay functions
#ifndef _SWDELAY_H
#define _SWDELAY_H


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)


#if (F_CPU == 16000000)

//delay functions
extern void _swdelay_625ns(void);
extern void _swdelay_10us(void);
extern void _swdelay_40us(void);
extern void _swdelay_n10us(void);
extern void _swdelay_n40us(void);

//delay macros
#define swdelay_125ns() asm("nop\nnop")              //2cycles = 125ns@16MHz
#define swdelay_625ns() asm("call _swdelay_625ns")   //10cycles = 625ns@16MHz
#define swdelay_10us() asm("call _swdelay_10us")     //5+1+38*4-1+5 cycles = 10.062us
#define swdelay_40us() asm("call _swdelay_40us")     //5+1+158*4-1+5 cycles = 40.125us
#define swdelay_n10us(n) asm("ldi r19,%0\ncall _swdelay_n10us" :: "i" (n))
#define swdelay_n40us(n) asm("ldi r19,%0\ncall _swdelay_n40us" :: "i" (n))

#endif //(F_CPU == 16000000)


#if defined(__cplusplus)
}
#endif //defined(__cplusplus)
#endif //_DELAY_H