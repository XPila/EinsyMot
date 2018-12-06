//cmd_einsy.c
#include "cmd_einsy.h"
#include <string.h>
#include <avr/pgmspace.h>
#include "st4.h"
#include "einsy.h"

#define MOD_MSK_0      0x0000
#define MOD_MSK_X      0x0001
#define MOD_MSK_Y      0x0002
#define MOD_MSK_Z      0x0004
#define MOD_MSK_E      0x0008
#define MOD_MSK_I      0x0010
#define MOD_MSK_O      0x0020
#define MOD_MSK_T      0x0040
#define MOD_MSK_XYZE   0x000f

#define MOD_ID_0       0xff
#define MOD_ID_X       0x00
#define MOD_ID_Y       0x01
#define MOD_ID_Z       0x02
#define MOD_ID_E       0x03
#define MOD_ID_I       0x04
#define MOD_ID_O       0x05
#define MOD_ID_T       0x06


#define CMD_ID_RST     0x01 //reset
#define CMD_ID_VER     0x02 //version
#define CMD_ID_MOA     0x03 //move absolute
#define CMD_ID_MOR     0x04 //move relative
#define CMD_ID_RES     0x05 //resolution
#define CMD_ID_SR0     0x06 //starting steprate
#define CMD_ID_SRM     0x07 //maximum steprate
#define CMD_ID_ACC     0x08 //acceleration
#define CMD_ID_DEC     0x09 //deceleration
#define CMD_ID_POS     0x0a //position
#define CMD_ID_ENA     0x0b //enabled
#define CMD_ID_LED     0x0c //leds
#define CMD_ID_SRX     0x0d //current steprate
#define CMD_ID_DIR     0x0e //direction
#define CMD_ID_MOT     0x0f //motion

#define CMD_ID_ERR     0x41 //error
#define CMD_ID_FAN     0x42 //fan control

char cmd_mod_char(uint8_t mod_id)
{
	switch (mod_id)
	{
	case MOD_ID_X: return 'X';
	case MOD_ID_Y: return 'Y';
	case MOD_ID_Z: return 'Z';
	case MOD_ID_E: return 'E';
	case MOD_ID_I: return 'I';
	case MOD_ID_O: return 'O';
	case MOD_ID_T: return 'T';
	}
	return '?';
}

int8_t cmd_parse_mod_msk(char* pstr, uint16_t* pmod_msk)
{
	int8_t ret = 0;
	char c;
	do
	{
		c = *pstr++;
		switch (c)
		{
		case 'X':
			*pmod_msk |= MOD_MSK_X;
			break;
		case 'Y':
			*pmod_msk |= MOD_MSK_Y;
			break;
		case 'Z':
			*pmod_msk |= MOD_MSK_Z;
			break;
		case 'E':
			*pmod_msk |= MOD_MSK_E;
			break;
		case 'I':
			*pmod_msk |= MOD_MSK_I;
			break;
		case 'O':
			*pmod_msk |= MOD_MSK_O;
			break;
		case 'T':
			*pmod_msk |= MOD_MSK_T;
			break;
		case '*':
			if (ret == 0) *pmod_msk = MOD_MSK_XYZE;
			break;
		case '!':
		case '?':
			if (ret == 0) *pmod_msk = MOD_MSK_0;
			return ret;
		default:
			return CMD_ER_SYN;
		}
		ret++;
	} while (ret <= 3);
	return CMD_ER_SYN;
}

int8_t cmd_parse_cmd_id(char* pstr, uint8_t* pcmd_id)
{
	//generic commands (3char)
	if ((pstr[3] == 0) || (pstr[3] == ' '))
	{
		*pcmd_id = 0;
		if (strncmp_P(pstr, PSTR("rst"), 3) == 0) *pcmd_id = CMD_ID_RST;
		else if (strncmp_P(pstr, PSTR("ver"), 3) == 0) *pcmd_id = CMD_ID_VER;
		else if (strncmp_P(pstr, PSTR("moa"), 3) == 0) *pcmd_id = CMD_ID_MOA;
		else if (strncmp_P(pstr, PSTR("mor"), 3) == 0) *pcmd_id = CMD_ID_MOR;
		else if (strncmp_P(pstr, PSTR("res"), 3) == 0) *pcmd_id = CMD_ID_RES;
		else if (strncmp_P(pstr, PSTR("sr0"), 3) == 0) *pcmd_id = CMD_ID_SR0;
		else if (strncmp_P(pstr, PSTR("srm"), 3) == 0) *pcmd_id = CMD_ID_SRM;
		else if (strncmp_P(pstr, PSTR("acc"), 3) == 0) *pcmd_id = CMD_ID_ACC;
		else if (strncmp_P(pstr, PSTR("dec"), 3) == 0) *pcmd_id = CMD_ID_DEC;
		else if (strncmp_P(pstr, PSTR("pos"), 3) == 0) *pcmd_id = CMD_ID_POS;
		else if (strncmp_P(pstr, PSTR("ena"), 3) == 0) *pcmd_id = CMD_ID_ENA;
		else if (strncmp_P(pstr, PSTR("led"), 3) == 0) *pcmd_id = CMD_ID_LED;
		else if (strncmp_P(pstr, PSTR("srx"), 3) == 0) *pcmd_id = CMD_ID_SRX;
		else if (strncmp_P(pstr, PSTR("dir"), 3) == 0) *pcmd_id = CMD_ID_DIR;
		else if (strncmp_P(pstr, PSTR("mot"), 3) == 0) *pcmd_id = CMD_ID_MOT;
		else if (strncmp_P(pstr, PSTR("err"), 3) == 0) *pcmd_id = CMD_ID_ERR;
		else if (strncmp_P(pstr, PSTR("fan"), 3) == 0) *pcmd_id = CMD_ID_FAN;
		
		if (*pcmd_id != 0)
			return 3;
	}
	return CMD_ER_SYN;
}

int8_t cmd_do_mod_wout_args(uint8_t mod_id, char pref, uint8_t cmd_id)
{
	if (mod_id == MOD_ID_0)
	{
		if (pref == '!')
		{
			switch (cmd_id)
			{
			case CMD_ID_RST:
				if (cmd_err) fprintf_P(cmd_err, PSTR("RESET\n"));
//				exit(0);
				return CMD_OK;
			}
		}
		else if (pref == '?')
		{
			switch (cmd_id)
			{
			case CMD_ID_VER:
				fprintf_P(cmd_out, PSTR("EinsyMotion %d.%d.%d-%d "), FW_VERSION/100, FW_VERSION%100/10, FW_VERSION%10, FW_BUILDNR);
				return CMD_OK;
			case CMD_ID_ENA:
				cmd_print_ui8(einsy_tmc_get_ena());
				return CMD_OK;
			case CMD_ID_LED:
//				cmd_print_ui16(sla_get_led());
				return CMD_OK;
			case CMD_ID_DIR:
				cmd_print_ui8(einsy_tmc_get_dir());
				return CMD_OK;
			case CMD_ID_MOT:
				cmd_print_ui8(st4_msk & 0x0f);
				return CMD_OK;
			case CMD_ID_FAN:
				cmd_print_ui8(einsy_get_fans());
				return CMD_OK;
			}
		}
	}
	else if ((mod_id >= MOD_ID_X) && (mod_id <= MOD_ID_E))
	{
		if (pref == '?')
		{
			switch (cmd_id)
			{
			case CMD_ID_RES:
				cmd_print_ui16(st4_axis[mod_id].res);
				return CMD_OK;
			case CMD_ID_SR0:
				cmd_print_ui16(st4_axis[mod_id].sr0);
				return CMD_OK;
			case CMD_ID_SRM:
				cmd_print_ui16(st4_axis[mod_id].srm);
				return CMD_OK;
			case CMD_ID_ACC:
				cmd_print_ui16(st4_axis[mod_id].acc);
				return CMD_OK;
			case CMD_ID_DEC:
				cmd_print_ui16(st4_axis[mod_id].dec);
				return CMD_OK;
			case CMD_ID_POS:
				cmd_print_i32(st4_axis[mod_id].pos);
				return CMD_OK;
			case CMD_ID_SRX:
				cmd_print_ui16(st4_axis[mod_id].srx.ui16.h);
				return CMD_OK;
			}
		}
	}
	return CMD_ER_SYN;
}

int8_t cmd_do_wout_args(uint16_t mod_msk, char pref, uint8_t cmd_id)
{
	int8_t ret = CMD_ER_SYN;
	uint8_t mod;
	uint16_t msk;
	if (mod_msk == MOD_MSK_0)
		return cmd_do_mod_wout_args(MOD_ID_0, pref, cmd_id);
	msk = MOD_MSK_X;
	for (mod = MOD_ID_X; mod <= MOD_ID_E; mod++)
	{
		if (msk & mod_msk)
		{
			ret = cmd_do_mod_wout_args(mod, pref, cmd_id);
			if (ret < 0) break;
		}
		msk <<= 1;
	}
	return ret;
}

int8_t cmd_do_mod_with_args(uint8_t mod_id, char pref, uint8_t cmd_id, char* pstr)
{
	int8_t ret;
	var_num_t val0;
	var_num_t val1;

	uint16_t val_ui16;
	uint16_t val1_ui16;
	uint16_t val2_ui16;
	int16_t val_i16;
	int32_t val_i32;
	if (mod_id == MOD_ID_0)
	{
		if (pref == '!')
			switch (cmd_id)
			{
			case CMD_ID_ENA:
				if ((ret = cmd_scan_ui8_min_max(pstr, &val0.ui8, 0, 15)) < 0) return ret;
				einsy_tmc_set_ena(val0.ui8);
				return CMD_OK;
			case CMD_ID_LED:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, 0, 65535)) < 0) return ret;
//				sla_set_led(val_ui16);
				return CMD_OK;
			case CMD_ID_DIR:
				if ((ret = cmd_scan_ui8_min_max(pstr, &val0.ui8, 0, 15)) < 0) return ret;
				st4_msk = (st4_msk & 0x0f) | (val0.ui8 << 4);
				einsy_tmc_set_dir(val0.ui8);
				return CMD_OK;
			case CMD_ID_MOT:
				if ((ret = cmd_scan_ui8_min_max(pstr, &val0.ui8, 0, 15)) < 0) return ret;
				st4_msk = (st4_msk & 0xf0) | (val0.ui8);
				return CMD_OK;
			case CMD_ID_ERR:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val0.ui16, 0, 8)) < 0) return ret;
//				sla_error(-((int8_t)val_ui16));
				return CMD_OK;
			case CMD_ID_FAN:
				if ((ret = cmd_scan_ui8_min_max(pstr, &val0.ui8, 0, 3)) < 0) return ret;
				einsy_set_fans(val0.ui8);
				return CMD_OK;
			}
	}
	else if ((mod_id >= MOD_ID_X) && (mod_id <= MOD_ID_E))
	{
		if (pref == '!')
			switch (cmd_id)
			{
			case CMD_ID_MOA:
				if ((ret = cmd_scan_i32(pstr, &val_i32)) < 0) return ret;
				if (st4_moa(mod_id, val_i32) < 0) return CMD_ER_BSY;
				return ret;
			case CMD_ID_MOR:
				if ((ret = cmd_scan_i32(pstr, &val_i32)) < 0) return ret;
				if (st4_mor(mod_id, val_i32) < 0) return CMD_ER_BSY;
				return ret;
			case CMD_ID_RES:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, 1, 1000)) < 0) return ret;
				st4_axis[mod_id].res = val_ui16;
				return ret;
			case CMD_ID_SR0:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, ST4_MIN_SR, ST4_MAX_SR)) < 0) return ret;
				st4_axis[mod_id].sr0 = val_ui16;
				return ret;
			case CMD_ID_SRM:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, ST4_MIN_SR, ST4_MAX_SR)) < 0) return ret;
				st4_axis[mod_id].srm = val_ui16;
				return ret;
			case CMD_ID_ACC:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, 100, 10000)) < 0) return ret;
				st4_axis[mod_id].acc = val_ui16;
				return ret;
			case CMD_ID_DEC:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, 100, 10000)) < 0) return ret;
				st4_axis[mod_id].dec = val_ui16;
				return ret;
			case CMD_ID_POS:
				if ((ret = cmd_scan_i32(pstr, &val0.i32)) < 0) return ret;
				st4_axis[mod_id].pos = val0.i32;
				return ret;
			case CMD_ID_SRX:
				if ((ret = cmd_scan_ui16_min_max(pstr, &val_ui16, 0, ST4_MAX_SR)) < 0) return ret;
				if (val_ui16 && (val_ui16 < ST4_MIN_SR)) val_ui16 = ST4_MIN_SR;
				st4_axis[mod_id].srx.ui16.l = 0;
				st4_axis[mod_id].srx.ui16.h = val_ui16;
				if (cmd_err) fprintf_P(cmd_err, PSTR("SRX=%08lx (h=%04x l=%04x)\n"), st4_axis[mod_id].srx.ui32, st4_axis[mod_id].srx.ui16.h, st4_axis[mod_id].srx.ui16.l);
				return ret;
			}
	}
	return CMD_ER_SYN;
}

int8_t cmd_do_with_args(uint16_t mod_msk, char pref, uint8_t cmd_id, char* pstr)
{
	int8_t ret = CMD_ER_SYN;
	uint8_t mod;
	uint16_t msk;
	if (mod_msk == MOD_MSK_0)
		return cmd_do_mod_with_args(MOD_ID_0, pref, cmd_id, pstr);
	msk = MOD_MSK_X;
	for (mod = MOD_ID_X; mod <= MOD_ID_E; mod++)
	{
		if (msk & mod_msk)
		{
			if (*pstr == 0) return CMD_ER_SYN;
			ret = cmd_do_mod_with_args(mod, pref, cmd_id, pstr);
			if (ret < 0) return ret;
			pstr += ret;
		}
		msk <<= 1;
	}
	return CMD_OK;
}
