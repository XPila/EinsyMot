//tmc2130.c - Trinamic stepper driver

#include "tmc2130.h"
#include <avr/io.h>
#include "spi.h"
#include <stdio.h>
#include <avr/pgmspace.h>

//TMC2130 registers
#define TMC2130_REG_GCONF      0x00 // 17 bits
#define TMC2130_REG_GSTAT      0x01 // 3 bits
#define TMC2130_REG_IOIN       0x04 // 8+8 bits
#define TMC2130_REG_IHOLD_IRUN 0x10 // 5+5+4 bits
#define TMC2130_REG_TPOWERDOWN 0x11 // 8 bits
#define TMC2130_REG_TSTEP      0x12 // 20 bits
#define TMC2130_REG_TPWMTHRS   0x13 // 20 bits
#define TMC2130_REG_TCOOLTHRS  0x14 // 20 bits
#define TMC2130_REG_THIGH      0x15 // 20 bits
#define TMC2130_REG_XDIRECT    0x2d // 32 bits
#define TMC2130_REG_VDCMIN     0x33 // 23 bits
#define TMC2130_REG_MSLUT0     0x60 // 32 bits
#define TMC2130_REG_MSLUT1     0x61 // 32 bits
#define TMC2130_REG_MSLUT2     0x62 // 32 bits
#define TMC2130_REG_MSLUT3     0x63 // 32 bits
#define TMC2130_REG_MSLUT4     0x64 // 32 bits
#define TMC2130_REG_MSLUT5     0x65 // 32 bits
#define TMC2130_REG_MSLUT6     0x66 // 32 bits
#define TMC2130_REG_MSLUT7     0x67 // 32 bits
#define TMC2130_REG_MSLUTSEL   0x68 // 32 bits
#define TMC2130_REG_MSLUTSTART 0x69 // 8+8 bits
#define TMC2130_REG_MSCNT      0x6a // 10 bits
#define TMC2130_REG_MSCURACT   0x6b // 9+9 bits
#define TMC2130_REG_CHOPCONF   0x6c // 32 bits
#define TMC2130_REG_COOLCONF   0x6d // 25 bits
#define TMC2130_REG_DCCTRL     0x6e // 24 bits
#define TMC2130_REG_DRV_STATUS 0x6f // 32 bits
#define TMC2130_REG_PWMCONF    0x70 // 22 bits
#define TMC2130_REG_PWM_SCALE  0x71 // 8 bits
#define TMC2130_REG_ENCM_CTRL  0x72 // 2 bits
#define TMC2130_REG_LOST_STEPS 0x73 // 20 bits

//chipselect control
extern void TMC2130_CS_LOW(uint8_t axis);
extern void TMC2130_CS_HIGH(uint8_t axis);

//read/write register
#define tmc2130_rd(axis, addr, rval) tmc2130_rx(axis, addr, rval)
#define tmc2130_wr(axis, addr, wval) tmc2130_tx(axis, addr | 0x80, wval)

//rx/tx datagram
uint8_t tmc2130_tx(uint8_t axis, uint8_t addr, uint32_t wval);
uint8_t tmc2130_rx(uint8_t axis, uint8_t addr, uint32_t* rval);

//convert ustep resolution to mres
uint8_t tmc2130_usteps2mres(uint16_t usteps);

//register CHOPCONF as 32bit value
uint32_t _CHOPCONF(uint8_t toff, uint8_t hstrt, uint8_t hend, uint8_t fd3, uint8_t disfdcc, uint8_t rndtf, uint8_t chm, uint8_t tbl, uint8_t vsense, uint8_t vhighfs, uint8_t vhighchm, uint8_t sync, uint8_t mres, uint8_t intpol, uint8_t dedge, uint8_t diss2g);

//register PWMCONF as 32bit value
uint32_t _PWMCONF(uint8_t pwm_ampl, uint8_t pwm_grad, uint8_t pwm_freq, uint8_t pwm_auto, uint8_t pwm_symm, uint8_t freewheel);


//structure for tmc2130 axis parameters (size = 9bytes)
typedef struct
{
	uint32_t chopconf;  //chopconf register as 32bit value
	uint8_t  ihold;     //holding current (0..63)
	uint8_t  irun;      //running current (0..63)
	int8_t   sg_thr;    //stallguard threshold (-128..+127)
	uint16_t tcoolthrs; //coolstep threshold (0..65535)
} tmc2130_axis_t;


//axis parameters
tmc2130_axis_t tmc2130_axis[TMC2130_NUMAXES];


//shortcut macros
#define __chopconf(axis)  tmc2130_axis[axis].chopconf
#define __ihold(axis)     tmc2130_axis[axis].ihold
#define __irun(axis)      tmc2130_axis[axis].irun
#define __sg_thr(axis)    tmc2130_axis[axis].sg_thr
#define __tcoolthrs(axis) tmc2130_axis[axis].tcoolthrs


uint8_t tmc2130_usteps2mres(uint16_t usteps)
{
	uint8_t mres = 8; while (mres && (usteps >>= 1)) mres--;
	return mres;
}

int8_t tmc2130_init_axis(uint8_t axis)
{
	uint8_t mres = 4;   // 16 microsteps
	uint8_t intpol = 1; // interpolation
	uint8_t toff = 3;   // toff = 3 (fchop = 27.778kHz)
	uint8_t hstrt = 5;  // initial 4, modified to 5
	uint8_t hend = 1;
	uint8_t fd3 = 0;
	uint8_t rndtf = 0;  // random off time
	uint8_t chm = 0;    // spreadCycle
	uint8_t tbl = 2;    // blanking time
	uint8_t vsense = 1; // high sensitivity (1/2 current)
	uint32_t chopconf = _CHOPCONF(toff, hstrt, hend, fd3, 0, rndtf, chm, tbl, vsense, 0, 0, 0, mres, intpol, 0, 0);
	__chopconf(axis)  = chopconf;
	__ihold(axis)     = TMC2130_DEF_CUR;
	__irun(axis)      = TMC2130_DEF_CUR;
	__sg_thr(axis)    = TMC2130_DEF_SGT;
	__tcoolthrs(axis) = TMC2130_DEF_CST;
	tmc2130_wr(axis, TMC2130_REG_CHOPCONF, __chopconf(axis));
	tmc2130_rd(axis, TMC2130_REG_CHOPCONF, &chopconf);
//	printf_P(PSTR("tmc2130_wr_CHOPCONF out=0x%08lx in=0x%08lx\n"), tmc2130_axis[axis].chopconf, valr);
	if (__chopconf(axis) != chopconf) return -1;
	tmc2130_wr(axis, TMC2130_REG_TPOWERDOWN, 0x00000000);
	tmc2130_wr(axis, TMC2130_REG_COOLCONF, (((uint32_t)__sg_thr(axis)) << 16));
	tmc2130_wr(axis, TMC2130_REG_TCOOLTHRS, __tcoolthrs(axis));
	tmc2130_wr(axis, TMC2130_REG_GCONF, 0x00003180);
	return 0;
}

int8_t tmc2130_init(void)
{
	uint8_t ret = 0;
	if (tmc2130_init_axis(0) < 0) ret |= 0x01;
#if (TMC2130_NUMAXES > 1)
	if (tmc2130_init_axis(1) < 0) ret |= 0x02;
#endif //(TMC2130_NUMAXES > 1)
#if (TMC2130_NUMAXES > 2)
	if (tmc2130_init_axis(2) < 0) ret |= 0x04;
#endif //(TMC2130_NUMAXES > 2)
#if (TMC2130_NUMAXES > 3)
	if (tmc2130_init_axis(3) < 0) ret |= 0x08;
#endif //(TMC2130_NUMAXES > 3)
	if (ret) ret |= 0x80;
	return (int8_t)ret;
}

uint16_t tmc2130_read_sg(uint8_t axis)
{
	uint32_t val32 = 0;
	tmc2130_rd(axis, TMC2130_REG_DRV_STATUS, &val32);
	return (val32 & 0x3ff);
}

uint8_t tmc2130_get_cur(uint8_t axis)
{
	uint8_t vsense = (__chopconf(axis) & ((uint32_t)1 << 17))?1:0;
	return vsense?__irun(axis):(__irun(axis) << 1);
}

void tmc2130_set_cur(uint8_t axis, uint8_t cur)
{
	uint8_t vsense = 1;
	if (cur >= 32)
	{
		vsense = 0;
		cur >>= 1;
	}
	__chopconf(axis) &= ~((uint32_t)1 << 17);
	__chopconf(axis) |= ((uint32_t)(vsense & 1) << 17);
	__ihold(axis) = cur;
	__irun(axis) = cur;
	tmc2130_wr(axis, TMC2130_REG_CHOPCONF, __chopconf(axis));
	tmc2130_wr(axis, TMC2130_REG_IHOLD_IRUN, 0x000f0000 | ((__irun(axis) & 0x1f) << 8) | (__ihold(axis) & 0x1f));
}

int8_t tmc2130_get_sgt(uint8_t axis)
{
	return __sg_thr(axis);
}

void tmc2130_set_sgt(uint8_t axis, int8_t sgt)
{
	__sg_thr(axis) = sgt;
	tmc2130_wr(axis, TMC2130_REG_COOLCONF, (((uint32_t)__sg_thr(axis)) << 16));
}

uint16_t tmc2130_get_cst(uint8_t axis)
{
	return __tcoolthrs(axis);
}

void tmc2130_set_cst(uint8_t axis, uint16_t cst)
{
	__tcoolthrs(axis) = cst;
	tmc2130_wr(axis, TMC2130_REG_TCOOLTHRS, __tcoolthrs(axis));
}


//spi
#define TMC2130_SPI_ENTER()    spi_setup(TMC2130_SPCR, TMC2130_SPSR)
#define TMC2130_SPI_TXRX       spi_txrx
#define TMC2130_SPI_LEAVE()

uint8_t tmc2130_tx(uint8_t axis, uint8_t addr, uint32_t wval)
{
	//datagram1 - request
	TMC2130_SPI_ENTER();
	TMC2130_CS_LOW(axis);
	TMC2130_SPI_TXRX(addr); // address
	TMC2130_SPI_TXRX((wval >> 24) & 0xff); // MSB
	TMC2130_SPI_TXRX((wval >> 16) & 0xff);
	TMC2130_SPI_TXRX((wval >> 8) & 0xff);
	TMC2130_SPI_TXRX(wval & 0xff); // LSB
	TMC2130_CS_HIGH(axis);
	TMC2130_SPI_LEAVE();
}

uint8_t tmc2130_rx(uint8_t axis, uint8_t addr, uint32_t* rval)
{
	//datagram1 - request
	TMC2130_SPI_ENTER();
	TMC2130_CS_LOW(axis);
	TMC2130_SPI_TXRX(addr); // address
	TMC2130_SPI_TXRX(0); // MSB
	TMC2130_SPI_TXRX(0);
	TMC2130_SPI_TXRX(0);
	TMC2130_SPI_TXRX(0); // LSB
	TMC2130_CS_HIGH(axis);
	TMC2130_SPI_LEAVE();
	//datagram2 - response
	TMC2130_SPI_ENTER();
	TMC2130_CS_LOW(axis);
	uint8_t stat = TMC2130_SPI_TXRX(0); // status
	uint32_t val32 = 0;
	val32 = TMC2130_SPI_TXRX(0); // MSB
	val32 = (val32 << 8) | TMC2130_SPI_TXRX(0);
	val32 = (val32 << 8) | TMC2130_SPI_TXRX(0);
	val32 = (val32 << 8) | TMC2130_SPI_TXRX(0); // LSB
	TMC2130_CS_HIGH(axis);
	TMC2130_SPI_LEAVE();
	if (rval != 0) *rval = val32;
	return stat;
}


uint32_t _CHOPCONF(uint8_t toff, uint8_t hstrt, uint8_t hend, uint8_t fd3, uint8_t disfdcc, uint8_t rndtf, uint8_t chm, uint8_t tbl, uint8_t vsense, uint8_t vhighfs, uint8_t vhighchm, uint8_t sync, uint8_t mres, uint8_t intpol, uint8_t dedge, uint8_t diss2g)
{
	uint32_t val = 0;
	val |= (uint32_t)(toff & 15);
	val |= (uint32_t)(hstrt & 7) << 4;
	val |= (uint32_t)(hend & 15) << 7;
	val |= (uint32_t)(fd3 & 1) << 11;
	val |= (uint32_t)(disfdcc & 1) << 12;
	val |= (uint32_t)(rndtf & 1) << 13;
	val |= (uint32_t)(chm & 1) << 14;
	val |= (uint32_t)(tbl & 3) << 15;
	val |= (uint32_t)(vsense & 1) << 17;
	val |= (uint32_t)(vhighfs & 1) << 18;
	val |= (uint32_t)(vhighchm & 1) << 19;
	val |= (uint32_t)(sync & 15) << 20;
	val |= (uint32_t)(mres & 15) << 24;
	val |= (uint32_t)(intpol & 1) << 28;
	val |= (uint32_t)(dedge & 1) << 29;
	val |= (uint32_t)(diss2g & 1) << 30;
	return val;
}

uint32_t _PWMCONF(uint8_t pwm_ampl, uint8_t pwm_grad, uint8_t pwm_freq, uint8_t pwm_auto, uint8_t pwm_symm, uint8_t freewheel)
{
	uint32_t val = 0;
	val |= (uint32_t)(pwm_ampl & 255);
	val |= (uint32_t)(pwm_grad & 255) << 8;
	val |= (uint32_t)(pwm_freq & 3) << 16;
	val |= (uint32_t)(pwm_auto & 1) << 18;
	val |= (uint32_t)(pwm_symm & 1) << 19;
	val |= (uint32_t)(freewheel & 3) << 20;
	return val;
}
