#ifndef _AVR_INTERRUPT_H_
#define _AVR_INTERRUPT_H_

#define ISR(vect) void vect(void)

#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern void cli(void);
extern void sei(void);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_AVR_INTERRUPT_H_
