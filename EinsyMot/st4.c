//st4.c
#include "st4.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//get/set direction control signals (defined in config.h)
extern uint8_t ST4_GET_DIR(void);
extern void ST4_SET_DIR(uint8_t mask);

//do step function (defined in config.h)
extern void ST4_DO_STEP(uint8_t mask);

//global variables
uint8_t  st4_msk = 0;             // motion and direction mask (bit 0..3 - motion, bit 4..7 - dir)
uint8_t  st4_end = 0;             // endstop enabled mask (bit 0..3 - mask)
uint16_t st4_d2 = 0;              // timer delay [500ns]
st4_axis_t st4_axis[ST4_NUMAXES]; // axis parameters

//macro shortcuts for more readable code
#define _res(a) st4_axis[a].res
#define _srx(a) st4_axis[a].srx.ui32
#define _srxl(a) st4_axis[a].srx.ui16.l
#define _srxh(a) st4_axis[a].srx.ui16.h
#define _sr0(a) st4_axis[a].sr0
#define _srm(a) st4_axis[a].srm
#define _acc(a) st4_axis[a].acc
#define _dec(a) st4_axis[a].dec
#define _nac(a) st4_axis[a].nac
#define _ndc(a) st4_axis[a].ndc
#define _pos(a) st4_axis[a].pos
#define _cnt(a) st4_axis[a].cnt

char st4_axis_chr(uint8_t axis)
{
	switch (axis)
	{
	case 0: return 'X';
	case 1: return 'Y';
	case 2: return 'Z';
	case 3: return 'E';
	}
	return '?';
}

uint8_t st4_axis_idx(char chr)
{
	switch (chr)
	{
	case 'X': return 0;
	case 'Y': return 1;
	case 'Z': return 2;
	case 'E': return 3;
	}
	return (uint8_t)-1;
}

//find axis with maximum sr
uint8_t st4_max_sr_axis(void)
{
	if (_srxh(0) >= _srxh(1))
	{
		if (_srxh(0) >= _srxh(2))
		{
			if (_srxh(0) >= _srxh(3))
				return 0;
			else
				return 3;
		}
		else
		{
			if (_srxh(2) >= _srxh(3))
				return 2;
			else
				return 3;
		}
	}
	else
	{
		if (_srxh(1) >= _srxh(2))
		{
			if (_srxh(1) >= _srxh(3))
				return 1;
			else
				return 3;
		}
		else
		{
			if (_srxh(2) >= _srxh(3))
				return 2;
			else
				return 3;
		}
	}
}

void st4_calc_acdc(uint8_t axis)
{
	uint16_t srm_sub_sr0 = _srm(axis) - _sr0(axis);
	uint16_t srm_add_sr0 = _srm(axis) + _sr0(axis);
	// acceleration time [us]
	uint32_t t_ac = (4096 * (uint32_t)srm_sub_sr0) / _acc(axis);
	// deceleration time [us]
	uint32_t t_dc = (4096 * (uint32_t)srm_sub_sr0) / _dec(axis);
	// acceleration steps [1]
	_nac(axis) = (uint16_t)(((t_ac >> 4) * (uint32_t)srm_add_sr0 + 62500) / 125000);
	// deceleration steps [1]
	_ndc(axis) = (uint16_t)(((t_dc >> 4) * (uint32_t)srm_add_sr0 + 62500) / 125000);
}

void st4_calc_move(uint8_t axis, uint32_t n)
{
	uint16_t nacdc = _nac(axis) + _ndc(axis);
	if (n >= nacdc)
	{
		st4_axis[axis].mov.nac = _nac(axis);
		st4_axis[axis].mov.nrm = n - (uint32_t)nacdc;
		st4_axis[axis].mov.ndc = _ndc(axis);
	}
	else
	{
		st4_axis[axis].mov.nac = (uint32_t)_nac(axis) * n / nacdc;
		st4_axis[axis].mov.nrm = 0;
		st4_axis[axis].mov.ndc = n - st4_axis[axis].mov.nac;
	}
//	printf_P(PSTR("st4_calc_move %d   %d %ld %d\n"), axis, st4_axis[axis].mov.nac, st4_axis[axis].mov.nrm, st4_axis[axis].mov.ndc);
}

int8_t st4_mor(uint8_t axis, int32_t val)
{
	uint8_t msk = 1 << axis;
	if (st4_msk & msk) return -1; //busy
	if (val == 0) return 0; //no motion
	if (val > 0)
	{
		ST4_SET_DIR(ST4_GET_DIR() & ~msk);
		st4_msk &= ~(msk << 4);
		st4_calc_move(axis, (uint32_t)val);
	}
	else
	{
		ST4_SET_DIR(ST4_GET_DIR() | msk);
		st4_msk |= msk << 4;
		st4_calc_move(axis, (uint32_t)(-val));
	}
	_srxl(axis) = 0;
	_srxh(axis) = _sr0(axis);
	st4_msk |= msk;
}

int8_t st4_moa(uint8_t axis, int32_t val)
{
	return st4_mor(axis, val - st4_axis[axis].pos);
}

void st4_setup_axis(uint8_t axis, uint16_t res, float sr0_mms, float srm_mms, float acc_mms2, float dec_mms2)
{
	st4_axis[axis].chr = st4_axis_chr(axis);
	st4_axis[axis].res = res;
	st4_set_sr0_mms(axis, sr0_mms);
	st4_set_srm_mms(axis, srm_mms);
	st4_set_acc_mms2(axis, acc_mms2);
	st4_set_dec_mms2(axis, dec_mms2);
	st4_calc_acdc(axis);
}

float st4_get_srx_mms(uint8_t axis)
{
	return (float)_srxh(axis) / _res(axis);
}

void st4_set_srx_mms(uint8_t axis, float sr0_mms)
{
	_srx(axis) = (uint32_t)(sr0_mms * _res(axis)) << 16;
}

float st4_get_sr0_mms(uint8_t axis)
{
	return (float)_sr0(axis) / _res(axis);
}

void st4_set_sr0_mms(uint8_t axis, float sr0_mms)
{
	_sr0(axis) = (uint16_t)(sr0_mms * _res(axis));
}

float st4_get_srm_mms(uint8_t axis)
{
	return (float)_srm(axis) / _res(axis);
}

void st4_set_srm_mms(uint8_t axis, float srm_mms)
{
	_srm(axis) = (uint16_t)(srm_mms * _res(axis));
}

float st4_get_acc_mms2(uint8_t axis)
{
	return (float)_acc(axis) * 256 / _res(axis);
}

void st4_set_acc_mms2(uint8_t axis, float acc_mms2)
{
	_acc(axis) = (uint16_t)((uint32_t)(acc_mms2 * _res(axis)) / 256);
}

float st4_get_dec_mms2(uint8_t axis)
{
	return (float)_dec(axis) * 256 / _res(axis);
}

void st4_set_dec_mms2(uint8_t axis, float dec_mms2)
{
	_dec(axis) = (uint16_t)((uint32_t)(dec_mms2 * _res(axis)) / 256);
}

float st4_get_pos_mm(uint8_t axis)
{
	return (float)_pos(axis) / _res(axis);
}

void st4_set_pos_mm(uint8_t axis, float pos_mm)
{
	_pos(axis) = (int16_t)(pos_mm * _res(axis));
}

void st4_cycle_axis0(void)
{
	if (st4_axis[0].mov.nac)
	{
		_srx(0) += ((uint32_t)_acc(0) * st4_d2) << 3;// / 8192;
		st4_axis[0].mov.nac--;
	}
	else if (st4_axis[0].mov.nrm)
	{
		st4_axis[0].mov.nrm--;
	}
	else if (st4_axis[0].mov.ndc)
	{
		_srx(0) -= ((uint32_t)_dec(0) * st4_d2) << 3;// / 8192;
		st4_axis[0].mov.ndc--;
		if (st4_axis[0].mov.ndc == 0)
		{
			st4_msk &= ~1;
			_srx(0) = 0;
		}
	}
}

void st4_cycle_axis1(void)
{
	if (st4_axis[1].mov.nac)
	{
		_srx(1) += ((uint32_t)_acc(1) * st4_d2) << 3;// / 8192;
		st4_axis[1].mov.nac--;
	}
	else if (st4_axis[1].mov.nrm)
	{
		st4_axis[1].mov.nrm--;
	}
	else if (st4_axis[1].mov.ndc)
	{
		_srx(1) -= ((uint32_t)_dec(1) * st4_d2) << 3;// / 8192;
		st4_axis[1].mov.ndc--;
		if (st4_axis[1].mov.ndc == 0)
		{
			st4_msk &= ~2;
			_srx(1) = 0;
		}
	}
}

#if (ST4_NUMAXES > 2)
void st4_cycle_axis2(void)
{
	if (st4_axis[2].mov.nac)
	{
		_srx(2) += ((uint32_t)_acc(2) * st4_d2) << 3;// / 8192;
		st4_axis[2].mov.nac--;
	}
	else if (st4_axis[2].mov.nrm)
	{
		st4_axis[2].mov.nrm--;
	}
	else if (st4_axis[2].mov.ndc)
	{
		_srx(2) -= ((uint32_t)_dec(2) * st4_d2) << 3;// / 8192;
		st4_axis[2].mov.ndc--;
		if (st4_axis[2].mov.ndc == 0)
		{
			st4_msk &= ~4;
			_srx(2) = 0;
		}
	}
}
#endif //(ST4_NUMAXES > 2)


void st4_setup_timer(void)
{
	// waveform generation = 0100 = CTC
	TCCR1B &= ~(1<<WGM13);
	TCCR1B |=  (1<<WGM12);
	TCCR1A &= ~(1<<WGM11);
	TCCR1A &= ~(1<<WGM10);
	// output mode = 00 (disconnected)
	TCCR1A &= ~(3<<COM1A0);
	TCCR1A &= ~(3<<COM1B0);
  // Set the timer pre-scaler
	TCCR1B = (TCCR1B & ~(0x07<<CS10)) | (2<<CS10);
	// Plan the first interrupt after 8ms from now.
	OCR1A = 0x200;
	TCNT1 = 0;
	TIMSK1 |= (1<<OCIE1A);
}

inline void st4_cycle(void)
{
	uint8_t axis;
	uint8_t max_sr_axis = st4_max_sr_axis();
	uint16_t max_sr = _srxh(max_sr_axis);
	uint8_t sm = 0;
	uint8_t em = 0;
	if (max_sr)
	{
		em = ST4_GET_END() & st4_end;
		if (em & 0x01) st4_msk &= ~0x01;
		if (em & 0x02) st4_msk &= ~0x02;
		st4_d2 = st4_sr2d2(max_sr);
		if (st4_msk & 1)
		{
			if (_cnt(0) <= _srxh(0))
			{
				_cnt(0) += max_sr;
				sm |= 1;
				if (st4_msk & 0x10) _pos(0)--;
				else _pos(0)++;
			}
			_cnt(0) -= _srxh(0);
			if (sm & 1) st4_cycle_axis0();
		}
		if (st4_msk & 2)
		{
			if (_cnt(1) <= _srxh(1))
			{
				_cnt(1) += max_sr;
				sm |= 2;
				if (st4_msk & 0x20) _pos(1)--;
				else _pos(1)++;
			}
			_cnt(1) -= _srxh(1);
			if (sm & 2) st4_cycle_axis1();
		}
#if (ST4_NUMAXES > 2)
		if (st4_msk & 4)
		{
			if (_cnt(2) <= _srxh(2))
			{
				_cnt(2) += max_sr;
				sm |= 4;
				if (st4_msk & 0x40) _pos(2)--;
				else _pos(2)++;
			}
			_cnt(2) -= _srxh(2);
			if (sm & 4) st4_cycle_axis2();
		}
#endif //(ST4_NUMAXES > 2)
#if (ST4_NUMAXES > 3)
		if (st4_msk & 8)
		{
			if (_cnt(3) <= _srxh(3))
			{
				_cnt(3) += max_sr;
				sm |= 8;
				if (st4_msk & 0x80) _pos(3)--;
				else _pos(3)++;
			}
			_cnt(3) -= _srxh(3);
			//if (sm & 8) st4_cycle_axis3();
		}
#endif //(ST4_NUMAXES > 3)
		ST4_DO_STEP(sm);
	}
	else
		st4_d2 = 2000;
	OCR1A = st4_d2;
}


//look up table for calculating delay (3 segments, 128 values)
uint8_t st4_tab_sr2d2[128+128+128];

uint16_t st4_sr2d2(uint16_t sr)
{
	if (sr == 0) return 0; // zero steprate - returns 0
	if (sr >= ST4_THR_SR3)
		return st4_tab_sr2d2[256 + (sr >> 6) - 218];
	if (sr >= ST4_THR_SR2)
		return st4_tab_sr2d2[128 + (sr >> 5) - 308];
	if (sr >= ST4_THR_SR1)
		return st4_tab_sr2d2[(sr >> 4) - 488];
	if (sr >= ST4_THR_SR0)
		return (uint16_t)(((uint32_t)2000000 + (sr >> 1)) / sr);
//	if ()
//		return ((uint32_t)2000000) / sr;
//		st4_tab_B[(sr >> 8) - 31]
}

// generate LUT segment (128 bytes)
//  sr0 - starting steprate [steps/s]
//  sh  - steprate shift [bits]
void st4_gen_seg(uint16_t sr0, uint8_t sh, uint8_t* pseg)
{
	uint8_t c = 128;           //sub-segment count
	uint8_t s = 1 << sh;       //sub-segment len
	uint16_t srx;              //shifted steprate [s*steps/s]
	uint16_t srx0 = sr0 >> sh; //shifted starting steprate [l*steps/s]
	uint16_t sr;               //steprate [steps/s]
	float    df;               //delay as float [us]
	uint16_t d2;               //delay*2 as 16bit uint [0.5us]
	float    de;               //delay error for d2 [%]
	float    df_sum;           //sum of df values in any sub-segment
	uint8_t i;
	uint8_t j;
	int n = 0;
	for (i = 0; i < c; i++)
	{
		df_sum = 0;
		for (j = 0; j < s; j++)
		{
			sr = sr0 + n;
			df = (float)1000000 / sr;
			df_sum += df;
			d2 = (int)(2 * df + 0.5);
			de = 100 * (((float)d2 / 2 - df) / df);
//			printf("%5d %5.2f %5.2f %5.2f%%\n", sr, df, (float)d2/2, de);
			n++;
		}
		d2 = (int)(2 * df_sum / s + 0.5);
		pseg[i] = d2;
//		printf("%5d %5.2f%\n", sr - s + 1, (float)d2/2);
	}
}

void st4_gen_tab(void)
{
	st4_gen_seg(ST4_THR_SR1, 4, st4_tab_sr2d2 + 0);
	st4_gen_seg(ST4_THR_SR2, 5, st4_tab_sr2d2 + 128);
	st4_gen_seg(ST4_THR_SR3, 6, st4_tab_sr2d2 + 256);
}

void st4_fprint_sr_d2(FILE* out, uint16_t sr0, uint16_t sr1)
{
	uint16_t sr; //steprate [steps/s]
	float    df; //delay as float [us]
	uint16_t d2; //delay*2 as 16bit int, calculated [0.5us resolution]
	uint16_t d2A; //delay*2 as 16bit int, from table [0.5us resolution]
	float    de; //delay error for d2 [%]
	float    deA; //delay error for d2A [%]
	for (sr = sr0; sr < sr1; sr++)
	{
		df = (float)1000000 / sr;
		d2 = (int)(2 * df + 0.5);
		d2A = st4_sr2d2(sr);
		de = 100 * (((float)d2/2 - df) / df);
		deA = 100 * (((float)d2A/2 - df) / df);
		fprintf_P(out, PSTR("%5d %5.2f %5.2f %5.2f%% %5.2f %5.2f%% \n"), sr, df, (float)d2/2, de, (float)d2A/2, deA);
	}
}

void st4_fprint_sr2d2_seg(FILE* out, uint8_t* pseg)
{
	uint8_t i;
	uint8_t j;
	fprintf_P(out, PSTR("=("));
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 32; j++)
		{
			fprintf_P(out, PSTR("%u"), pgm_read_byte(pseg + 32*i + j));
			if ((i < 3) || (j < 31))
				fprintf_P(out, PSTR(", "));
		}
		if (i < 3)
			fprintf_P(out, PSTR("\n"));
	}
	fprintf_P(out, PSTR(");\n"));
}

void st4_fprint_sr2d2_tab(FILE* out)
{
	st4_fprint_sr2d2_seg(stdout, st4_tab_sr2d2 + 0);
	st4_fprint_sr2d2_seg(stdout, st4_tab_sr2d2 + 128);
	st4_fprint_sr2d2_seg(stdout, st4_tab_sr2d2 + 256);
}

void st4_fprint_axis(FILE* out, uint8_t axis)
{
	fprintf_P(out, PSTR("axis %c\n"), st4_axis[axis].chr);
	fprintf_P(out, PSTR(" res=%8d [steps/mm]\n"), st4_axis[axis].res);
	fprintf_P(out, PSTR(" sr0=%8.0f [mm/s]\n"), st4_get_sr0_mms(axis));
	fprintf_P(out, PSTR(" srm=%8.0f [mm/s]\n"), st4_get_srm_mms(axis));
	fprintf_P(out, PSTR(" acc=%8.0f [mm/s^2]\n"), st4_get_acc_mms2(axis));
	fprintf_P(out, PSTR(" dec=%8.0f [mm/s^2]\n"), st4_get_dec_mms2(axis));
}

ISR(TIMER1_COMPA_vect)
{
	st4_cycle();
}
