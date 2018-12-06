//config.h - main configuration file
#ifndef _CONFIG_H
#define _CONFIG_H


//--------------------------------------
//version and buildnumber
#define FW_VERSION 90 //it means version 0.9.0
#define FW_BUILDNR 70 //number of commits in 'master'

#define BOARD_EINSY


//--------------------------------------
//UART configuration
#define UART0_BDR 115200 //UART0
#define UART1_BDR 115200 //UART1
//stdin & stdout uart0/1
#define UART_STD 0
//communication uart0/1
#define UART_COM 0


//--------------------------------------
//ADC configuration
// chanels:
// 0 - ADC0 - PF0 - temp0 (nozzle)
// 1 - ADC1 - PF1 - temp1 (aux)
// 2 - ADC2 - PF2 - temp2 (bed)
// 3 - ADC3 - PF3 - temp3 (pinda)
// 4 - ADC4 - PF4 - voltage0 (system power)
// 5 - ADC6 - PF6 - temp4 (ambient)
// 6 - ADC9 - PK1 - voltage1 (bed power)
//#define ADC_CHAN_MSK      0b0000001001011111 //used AD channels bit mask (0,1,2,3,4,6,9)
#define ADC_CHAN_MSK      0x25f     //used AD channels bit mask (0,1,2,3,4,6,9)
#define ADC_CHAN_CNT      7         //number of used channels)
#define ADC_OVRSAMPL      16        //oversampling multiplier
#define ADC_READY         adc_ready //callback function ()


//--------------------------------------
//CMD configuration (file stream command processor)
#define CMD_MAX_LINE 42        //;-)


//--------------------------------------
//LCD configuration
#define LCD_4BIT           // 4bit mode
#define LCD_KNOB           // lcd knob input
#define LCD_IBUF    8      // input buffer size (for knob data input)
#define LCD_OBUF   64      // output buffer size (for lcd data output)
#define LCD_INBL           // input non blocking mode
//#define LCD_ONBL         // output non blocking mode
#define LCD_FILE           // lcd file stream lcdio
#define LCD_ESCC           // lcd escape codes
//lcd pinout
#define LCD_PIN_EN 61      // enable signal
#define LCD_PIN_RS 82      // register select signal
#define LCD_PIN_D4 59      // data line 4
#define LCD_PIN_D5 70      // data line 5
#define LCD_PIN_D6 85      // data line 6
#define LCD_PIN_D7 71      // data line 7
//#define LCD_PIN_BL 32  // back light control
//button pinout
#define LCD_PIN_BTN_EN1 72 // phase1
#define LCD_PIN_BTN_EN2 14 // phase2
#define LCD_PIN_BTN_ENC  9 // the click 


//--------------------------------------
//SWI2C configuration
#define SWI2C
#define SWI2C_SDA          2 //SDA on PD1
#define SWI2C_SCL          3 //SCL on PD0
#define SWI2C_A8
#define SWI2C_TMO          2048 //2048 cycles timeout


//--------------------------------------
//TMC2130 - Trinamic stepper driver
#define TMC2130_NUMAXES    4
//chipselect control
#define TMC2130_CS_LOW     einsy_tmc_cs_low
#define TMC2130_CS_HIGH    einsy_tmc_cs_high
//SPI configuration
#define TMC2130_SPI_RATE   0 // fosc/4 = 4MHz
#define TMC2130_SPCR       SPI_SPCR(TMC2130_SPI_RATE, 1, 1, 1, 0)
#define TMC2130_SPSR       SPI_SPSR(TMC2130_SPI_RATE)
//default axis configuration (common for all axes)
#define TMC2130_DEF_CUR    16 // default current
#define TMC2130_DEF_SGT    3  // default stallguard threshold
#define TMC2130_DEF_CST    1500 //default coolstep threshold


//--------------------------------------
//ST4 - stepper motion control
#define ST4_TIMER      1
#define ST4_NUMAXES    4
//direction control
#define ST4_GET_DIR    einsy_tmc_get_dir
#define ST4_SET_DIR    einsy_tmc_set_dir
//endstop sampling function
#define ST4_GET_END    einsy_tmc_get_diag
//step function
#define ST4_DO_STEP    einsy_tmc_do_step


//MSVC or MINGW simulator
#ifdef _SIMULATOR
#define _SIM_LCD
#define _SIM_TIMER1
#undef UART_STD
#undef UART_COM
#define UART_STD -1
#define UART_COM 0
#include <simulator.h>
//#define register
#define asm(code)
#define _INLINE __inline
//#define uart_std stdout
//#define uart_com stdout
#define SIM_LCD_WRITE sim_lcd_write
#else //_SIMULATOR
#define _INLINE inline
#endif //_SIMULATOR

#endif //_CONFIG_H
