//tmc2130.h - Trinamic stepper driver
#ifndef _TMC2130_H
#define _TMC2130_H

#include <inttypes.h>
#include "config.h"


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)


extern int8_t tmc2130_init(void);


extern uint16_t tmc2130_read_sg(uint8_t axis);


extern uint8_t tmc2130_get_cur(uint8_t axis);

extern void tmc2130_set_cur(uint8_t axis, uint8_t cur);

extern int8_t tmc2130_get_sgt(uint8_t axis);

extern void tmc2130_set_sgt(uint8_t axis, int8_t sgt);


#if defined(__cplusplus)
}
#endif //defined(__cplusplus)
#endif //_TMC2130_H
