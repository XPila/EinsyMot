// einsy.h
#ifndef _EINSY_H
#define _EINSY_H
#include "config.h"
#include <stdio.h>


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)


extern void einsy_io_setup_pins(void);

extern void einsy_tmc_setup_pins(void);
//tmc chipselect control
extern void einsy_tmc_cs_low(uint8_t axis);
extern void einsy_tmc_cs_high(uint8_t axis);
//tmc enable control
extern uint8_t einsy_tmc_get_ena(void);
extern void einsy_tmc_set_ena(uint8_t mask);
//tmc direction control
extern uint8_t einsy_tmc_get_dir(void);
extern void einsy_tmc_set_dir(uint8_t mask);
//tmc diag signal
extern uint8_t einsy_tmc_get_diag(void);
//tmc step control
extern void einsy_tmc_do_step(uint8_t mask);


#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_EINSY_H
