#ifndef __SIMULATOR_H
#define __SIMULATOR_H

//#include "c:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\include\stdio.h"
//#include "c:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\include\stdio.h"

#define __SRD	0x0001		/* OK to read */
#define __SWR	0x0002		/* OK to write */
#define __SSTR	0x0004		/* this is an sprintf/snprintf string */
#define __SPGM	0x0008		/* fmt string is in progmem */
#define __SERR	0x0010		/* found error */
#define __SEOF	0x0020		/* found EOF */
#define __SUNGET 0x040		/* ungetc() happened */
#define __SMALLOC 0x80		/* handle is malloc()ed */ 

#define _FDEV_SETUP_READ  __SRD
#define _FDEV_SETUP_WRITE __SWR
#define _FDEV_SETUP_RW    (__SRD|__SWR)

#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

#include <stdio.h>
//extern FILE* uart0io;
//extern FILE* uart1io;

extern void fdev_setup_stream(void* stream, void* put, void* get, int rwflag);


#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //__SIMULATOR_H
