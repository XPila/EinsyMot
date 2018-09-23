// cmd.h
#ifndef _CMD_H
#define _CMD_H
#include "config.h"
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern FILE* cmd_in;
extern FILE* cmd_out;
extern FILE* cmd_err;

extern void cmd_process(void);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_CMD_H
