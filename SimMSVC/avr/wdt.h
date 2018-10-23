// wdt.h
#ifndef _AVR_WDT_H_
#define _AVR_WDT_H_

#define wdt_enable(del)
#define wdt_disable()
#define wdt_reset()

#define WDTO_15MS   0
#define WDTO_30MS   1
#define WDTO_60MS   2
#define WDTO_120MS  3
#define WDTO_250MS  4
#define WDTO_500MS  5
#define WDTO_1S     6
#define WDTO_2S     7
 

#endif //_AVR_WDT_H_
