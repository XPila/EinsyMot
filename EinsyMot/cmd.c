//cmd.c - file stream command processor
#include "cmd.h"
#include <string.h>
#include <avr/pgmspace.h>


FILE* cmd_in = 0;
FILE* cmd_out = 0;
FILE* cmd_err = 0;

uint8_t cmd_echo = 0;

extern int8_t cmd_parse_mod_msk(char* pstr, uint16_t* pmod_msk);
extern int8_t cmd_parse_cmd_id(char* pstr, uint8_t* pcmd_id);
extern int8_t cmd_do_wout_args(uint16_t mod_msk, char pref, uint8_t cmd_id);
extern int8_t cmd_do_with_args(uint16_t mod_msk, char pref, uint8_t cmd_id, char* pstr);


int8_t cmd_print_ui8(uint8_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%u "), val);
	return (int8_t)((n < 0)?0:n);
}

int8_t cmd_print_ui16(uint16_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%u "), val);
	return (int8_t)((n < 0)?0:n);
}

int8_t cmd_print_ui32(uint32_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%lu "), val);
	return (int8_t)((n < 0)?0:n);
}

int8_t cmd_print_i8(int8_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%i "), val);
	return (int8_t)((n < 0)?0:n);
}

int8_t cmd_print_i16(int16_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%i "), val);
	return (int8_t)((n < 0)?0:n);
}

int8_t cmd_print_i32(int32_t val)
{
	int n = fprintf_P(cmd_out, PSTR("%li "), val);
	return (int8_t)((n < 0)?0:n);
}


int8_t cmd_scan_ui8(char* pstr, uint8_t* pval)
{
#ifdef _SIMULATOR
	uint16_t val;
	int n; if (sscanf_P(pstr, PSTR("%hu%n"), &val, &n) != 1) return CMD_ER_SYN;
	*pval = (uint8_t)val;
#else //_SIMULATOR
	int n; if (sscanf_P(pstr, PSTR("%hhu%n"), pval, &n) != 1) return CMD_ER_SYN;
#endif //_SIMULATOR
	return (int8_t)n;
}

int8_t cmd_scan_ui8_min_max(char* pstr, uint8_t* pval, uint8_t min_val, uint8_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_ui8(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

int8_t cmd_scan_ui16(char* pstr, uint16_t* pval)
{
	int n; if (sscanf_P(pstr, PSTR("%u%n"), pval, &n) != 1) return CMD_ER_SYN;
	return (int8_t)n;
}

int8_t cmd_scan_ui16_min_max(char* pstr, uint16_t* pval, uint16_t min_val, uint16_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_ui16(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

int8_t cmd_scan_ui32(char* pstr, uint32_t* pval)
{
	int n; if (sscanf_P(pstr, PSTR("%lu%n"), pval, &n) != 1) return CMD_ER_SYN;
	return (int8_t)n;
}

int8_t cmd_scan_ui32_min_max(char* pstr, uint32_t* pval, uint32_t min_val, uint32_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_ui32(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

int8_t cmd_scan_i8(char* pstr, int8_t* pval)
{
#ifdef _SIMULATOR
	int16_t val;
	int n; if (sscanf_P(pstr, PSTR("%hi%n"), &val, &n) != 1) return CMD_ER_SYN;
	*pval = (int8_t)val;
#else //_SIMULATOR
	int n; if (sscanf_P(pstr, PSTR("%hhi%n"), pval, &n) != 1) return CMD_ER_SYN;
#endif //_SIMULATOR
	return (int8_t)n;
}

int8_t cmd_scan_i8_min_max(char* pstr, int8_t* pval, int8_t min_val, int8_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_i8(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

int8_t cmd_scan_i16(char* pstr, int16_t* pval)
{
	int n; if (sscanf_P(pstr, PSTR("%i%n"), pval, &n) != 1) return CMD_ER_SYN;
	return (int8_t)n;
}

int8_t cmd_scan_i16_min_max(char* pstr, int16_t* pval, int16_t min_val, int16_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_i16(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

int8_t cmd_scan_i32(char* pstr, int32_t* pval)
{
	int n; if (sscanf_P(pstr, PSTR("%li%n"), pval, &n) != 1) return CMD_ER_SYN;
	return (int8_t)n;
}

int8_t cmd_scan_i32_min_max(char* pstr, int32_t* pval, int32_t min_val, int32_t max_val)
{
	int8_t ret; if ((ret = cmd_scan_i32(pstr, pval)) < 0) return ret;
	if ((*pval < min_val) || (*pval > max_val)) return CMD_ER_OOR; else return ret; 
}

void cmd_putc(char c)
{
	fputc(c, cmd_out);
}

#ifdef _SIMULATOR
#include <conio.h>
#endif //_SIMULATOR

void cmd_process(void)
{
	static char line[CMD_MAX_LINE];
	static int count = 0;
	char c = -1;
	char* pstr = 0;
	uint16_t mod_msk = 0;
	uint8_t cmd_id = 0;
	int8_t ret = -1;
	if (count < CMD_MAX_LINE)
	{
#ifdef _SIMULATOR
		//simulator single char mode
		while (kbhit() && ((c = (char)getch()) >= 0) && (putchar((c==0x0d)?'\n':c)))
		//TODO: line mode
#else //_SIMULATOR
		while ((c = (char)fgetc(cmd_in)) >= 0)
#endif //_SIMULATOR
		{
			if (c == '\r') c = 0;
			if (c == '\n') c = 0;
			line[count] = c;
			if (c) count++;
			if ((c == 0) || (count >= CMD_MAX_LINE)) break;
		}
	}
	if (count >= CMD_MAX_LINE)
	{ //command overflow
		fprintf_P(cmd_out, PSTR("ERROR: command overflow\n"), count);
		count = 0;
	}
	//fprintf_P(cmd_err, PSTR("cmd_process %d\n"), count);
	else if ((count > 0) && (c == 0))
	{ //line received
		if (cmd_echo)
		{
			fputs(line, cmd_out);
			fputc('\n', cmd_out);
		}
		//if (cmd_err) fprintf_P(cmd_err, PSTR("line received: '%s' %d\n"), line, count);
		pstr = line;
		mod_msk = 0;
		ret = cmd_parse_mod_msk(pstr, &mod_msk);
		//if (cmd_err) fprintf_P(cmd_err, PSTR("cmd_parse_mod_msk ret=%d mod_msk=%u\n"), ret, mod_msk);
		if (ret >= 0)
		{
			pstr += ret;
			c = *pstr;
			pstr++;
			ret = cmd_parse_cmd_id(pstr, &cmd_id);
			//if (cmd_err) fprintf_P(cmd_err, PSTR("cmd_parse_cmd_id ret=%d cmd_id=%u\n"), ret, cmd_id);
			if (ret >= 0)
			{
				pstr += ret;
				if (*pstr == 0)
					ret = cmd_do_wout_args(mod_msk, c, cmd_id);
				else if (*pstr == ' ')
					ret = cmd_do_with_args(mod_msk, c, cmd_id, pstr + 1);
				else
					ret = CMD_ER_SYN;
			}
		}
		fprintf_P(cmd_out, (ret == 0)?PSTR("ok\n"):PSTR("e%d\n"), -ret);
		count = 0;
	}
	else
	{ //nothing received
		// TODO: lifetime
	}
}
