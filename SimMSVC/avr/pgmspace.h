// pgmspace.h
#ifndef _PGMSPACE_H
#define _PGMSPACE_H

#define PROGMEM
#define PSTR
#define printf_P printf
#define fprintf_P fprintf
#define fputs_P fputs
#define strncmp_P strncmp
#define sscanf_P sscanf
#define pgm_read_byte(addr) (*((uint8_t*)addr))

#endif //_PGMSPACE_H
