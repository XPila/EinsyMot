//st4.h - four axis stepper motor control
#ifndef _ST4_H
#define _ST4_H

#include "config.h"
#include <inttypes.h>
#include <stdio.h>

//axis index and mask definitions
#define ST4_X             0    // x-axis index
#define ST4_Y             1    // y-axis index
#define ST4_Z             2    // z-axis index
#define ST4_E             3    // e-axis index
#define ST4_MSK_X         1    // x-axis mask
#define ST4_MSK_Y         2    // y-axis mask
#define ST4_MSK_Z         4    // z-axis mask
#define ST4_MSK_E         8    // e-axis mask
#define ST4_MSK_XY        3    // xy-axis mask
#define ST4_MSK_XYZ       7    // xyz-axis mask
#define ST4_MSK_XYZE     15    // xyze-axis mask

// flag definitions
#define ST4_FLG_MOT         1  // motion flag
#define ST4_FLG_ACC         2  // accelerating flag
#define ST4_FLG_DEC         4  // decelerating flag

//steprate thresholds [steps/s] for optimized delay calculation
#define ST4_THR_SR0      32
#define ST4_THR_SR1    7840    // 7808
#define ST4_THR_SR2    9888    // 9856
#define ST4_THR_SR3   13984    // 13952
#define ST4_THR_SR4   22176    // 22144

#define ST4_MIN_SR       32    // minimum steprate [steps/s]
#define ST4_MAX_SR    22000    // maximum steprate [steps/s]

//union for uint32/uint16
typedef union
{
	uint32_t ui32;             //32bit uint, dword
	struct
	{
		uint16_t l;            //16bit uint, low-word
		uint16_t h;            //16bit uint, high-word
	} ui16;
} u32u16_t;

//structure for one axis move transaction
typedef struct
{
	uint16_t nac;              //number of accelerating steps (sr0->srm)
	uint32_t nrm;              //number of running steps (srm)
	uint16_t ndc;              //number of decelerating steps (srm->sr0)
} st4_mov1_t;

//structure for axis parameters
typedef struct
{
	char chr;                  //char ('X', 'Y', ...)
	uint16_t res;              //resolution [steps/mm]
	uint16_t sr0;              //starting steprate [steps/s]
	uint16_t srm;              //maximum steprate [steps/s]
	uint16_t acc;              //acceleration [256*steps/s^2]
	uint16_t dec;              //deceleration [256*steps/s^2]
	uint16_t nac;              //number of accelerating steps (sr0->srm)
	uint16_t ndc;              //number of decelerating steps (srm->sr0)
	u32u16_t srx;              //current steprate [steps/65536*s]
	int32_t  pos;              //current position [steps]
	uint8_t  flg;              //flags (bit 0 - motion, bit1 - accel, bit2 - decel, bit4 - min endstop, bit5 - max endstop)
	uint16_t cnt;              //counter
	st4_mov1_t mov;            //move transaction
} st4_axis_t;



#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)


extern uint8_t  st4_msk;                 // motion and direction mask (bit 0..3 - motion, bit 4..7 - dir)
extern uint8_t  st4_end;                 // endstop enabled mask (bit 0..3 - mask)
extern uint16_t st4_d2;                  // timer delay [500ns]
extern st4_axis_t st4_axis[ST4_NUMAXES]; // axis parameters
//extern st4_move_t st4_move*;           // axis parameters

extern uint8_t st4_max_sr_axis(void);

extern void st4_calc_acdc(uint8_t axis);

extern void st4_calc_move(uint8_t axis, uint32_t n);

extern int8_t st4_mor(uint8_t axis, int32_t val);

extern int8_t st4_moa(uint8_t axis, int32_t val);

extern void st4_setup_axis(uint8_t axis, uint16_t res, float sr0_mms, float srm_mms, float acc_mms2, float dec_mms2);

extern float st4_get_srx_mms(uint8_t axis);
extern void st4_set_srx_mms(uint8_t axis, float srx_mms);

extern float st4_get_sr0_mms(uint8_t axis);
extern void st4_set_sr0_mms(uint8_t axis, float sr0_mms);

extern float st4_get_srm_mms(uint8_t axis);
extern void st4_set_srm_mms(uint8_t axis, float srm_mms);

extern float st4_get_acc_mms2(uint8_t axis);
extern void st4_set_acc_mms2(uint8_t axis, float acc_mms2);

extern float st4_get_dec_mms2(uint8_t axis);
extern void st4_set_dec_mms2(uint8_t axis, float dec_mms2);

extern float st4_get_pos_mm(uint8_t axis);
extern void st4_set_pos_mm(uint8_t axis, float pos_mm);

extern void st4_setup_timer(void);

extern uint16_t st4_sr2d2(uint16_t sr);

extern void st4_gen_tab(void);

extern void st4_fprint_sr_d2(FILE* out, uint16_t sr0, uint16_t sr1);

extern void st4_fprint_sr2d2_tab(FILE* out);

extern void st4_fprint_axis(FILE* out, uint8_t axis);


#if defined(__cplusplus)
}
#endif //defined(__cplusplus)

#endif //_ST4_H
