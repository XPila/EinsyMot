#ifndef __SIM_LCD_H
#define __SIM_LCD_H

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern FILE* sim_lcd_outF[2];

extern void sim_lcd_init(void);
extern void sim_lcd_done(void);
extern void sim_lcdio_init(void);
extern void sim_lcdio_done(void);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //__SIM_LCD_H
