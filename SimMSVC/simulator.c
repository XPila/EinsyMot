#include "simulator.h"
#include <stdio.h>
#include <avr/io.h>
#include <config.h>

#ifdef _SIM_LCD
#include "sim_lcd.h"
#endif //_SIM_LCD

unsigned char OCR0B = 0;
unsigned char TIMSK0 = 0;
unsigned char TIMSK1 = 0;
unsigned short OCR1A = 0;
unsigned short OCR1B = 0;
unsigned short OCR1C = 0;
unsigned short TCNT1 = 0;
unsigned char TCCR1A = 0;
unsigned char TCCR1B = 0;

unsigned char TIMSK3 = 0;
unsigned short TCNT3 = 0;
unsigned short OCR3A = 0;
unsigned char TCCR3A = 0;
unsigned char TCCR3B = 0;


unsigned char DDRA = 0;
unsigned char DDRB = 0;
unsigned char DDRC = 0;
unsigned char DDRD = 0;
unsigned char DDRE = 0;
unsigned char DDRF = 0;
unsigned char DDRG = 0;
unsigned char DDRH = 0;
unsigned char DDRI = 0;
unsigned char DDRJ = 0;
unsigned char DDRK = 0;
unsigned char DDRL = 0;

unsigned char PORTA = 0;
unsigned char PORTB = 0;
unsigned char PORTC = 0;
unsigned char PORTD = 0;
unsigned char PORTE = 0;
unsigned char PORTF = 0;
unsigned char PORTG = 0;
unsigned char PORTH = 0;
unsigned char PORTI = 0;
unsigned char PORTJ = 0;
unsigned char PORTK = 0;
unsigned char PORTL = 0;

unsigned char PINA = 0;
unsigned char PINB = 0;
unsigned char PINC = 0;
unsigned char PIND = 0;
unsigned char PINE = 0;
unsigned char PINF = 0;
unsigned char PING = 0;
unsigned char PINH = 0;
unsigned char PINI = 0;
unsigned char PINJ = 0;
unsigned char PINK = 0;
unsigned char PINL = 0;

unsigned char SPCR = 0;
unsigned char SPSR = 0;
unsigned char SPDR = 0;

unsigned char ADCSRA = 0;
unsigned char ADCSRB = 0;
unsigned char ADMUX = 0;
unsigned char DIDR0 = 0;
unsigned char DIDR2 = 0;
unsigned short ADC = 0;

unsigned char MCUSR = 0;
unsigned char WDTCSR = 0;
unsigned char CLKSEL0 = 0;
unsigned char CLKSEL1 = 0;
unsigned char CLKPR = 0;
unsigned char UHWCON = 0;
unsigned char USBCON = 0;


#include <windows.h>
#include <io.h>

extern FILE _uart0io;
extern FILE _uart1io;
extern FILE _lcdio;
extern FILE* uart0io;
extern FILE* uart1io;
extern FILE* lcd0io;
extern FILE* lcd1io;


HANDLE sim_timer0_thread = 0;
DWORD sim_timer0_threadId = 0;
HANDLE sim_timer1_thread = 0;
DWORD sim_timer1_threadId = 0;

void sim_cycle(void);
void sim_init(void);
void sim_done(void);

void sim_uart0io_init(void);
void sim_uart0io_done(void);
void sim_uart1io_init(void);
void sim_uart1io_done(void);

DWORD WINAPI sim_timer0_threadProc(LPVOID lpParameter);

#ifdef _SIM_TIMER1
DWORD WINAPI sim_timer1_threadProc(LPVOID lpParameter);
#endif //_SIM_TIMER1

extern void TIMER0_COMPB_vect(void);
extern void TIMER1_COMPA_vect(void);

void sim_cycle(void)
{
	MSG sMsg;
	int iResult = 0;
	if (PeekMessage(&sMsg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&sMsg); 
		DispatchMessage(&sMsg);
	}
#ifdef _SIM_LCD
	fflush(sim_lcd_outF[1]);
#endif //_SIM_LCD
	Sleep(10);
}

void sim_init(void)
{
#ifdef _SIM_LCD
	sim_lcd_init();
#endif //_SIM_LCD
	sim_timer0_thread = CreateThread(0, 0, sim_timer0_threadProc, 0, 0, &sim_timer0_threadId);
#ifdef _SIM_TIMER1
	sim_timer1_thread = CreateThread(0, 0, sim_timer1_threadProc, 0, 0, &sim_timer1_threadId);
#endif //_SIM_TIMER1
}

void sim_done(void)
{
#ifdef _SIM_LCD
	sim_window_done();
#endif //_SIM_LCD
}

void sim_uart0io_init(void)
{
	char buff[256];
	int sz;
	HWND wnd;

/*
	int nPipe = 0;
	HANDLE hPipe = CreateNamedPipe("\\\\.\\Pipe\\AVRsim_uart0",
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, MAX_PATH, MAX_PATH, NMPWAIT_USE_DEFAULT_WAIT, NULL);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		GetLastError();
	}
	nPipe = _open_osfhandle(hPipe, 0);
	if (nPipe < 0)
	{
		GetLastError();
	}
	uart0io = fdopen(nPipe, "w+");
*/
/*	if (!CreatePipe(huart0i, huart0i + 1, NULL, 512)) return;
	if (!CreatePipe(huart0o, huart0o + 1, NULL, 512))
	{
		CloseHandle(huart0i[0]);
		CloseHandle(huart0i[1]);
		return;
	}
	int nfd_ir = _open_osfhandle(huart0i[0], 0);
	int nfd_ow = _open_osfhandle(huart0o[1], 0);
	uart0io = fdopen(nPort, "w+");
	uart0io = fdopen(nPort, "w+");
//	HANDLE hPort = 0;
	int nPort = 0;
	char cText[256];
	int i;*/
/*	HANDLE hPort = 0;
	int nPort = 0;
//	if ((hPort = CreateFile( "\\\\.\\COM40", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	if ((hPort = CreateFile( "\\\\.\\COM40", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL)) == INVALID_HANDLE_VALUE)
		return;
	nPort = _open_osfhandle(hPort, 0);
	uart0io = fdopen(nPort, "r+");
//	uart0io = fopen("\\\\.\\COM40", "w+");

	fprintf(uart0io, "Test\n");
	fflush(uart0io);

	sz = 1;
	while (sz)
	{
	sz = fread(buff, 1, 256, uart0io);
	if (sz > 0)
	{
		printf("256 ok\n");
	}
	}

	if (uart0io == 0)
	{
//		CloseHandle(hPort);
		return;
	}
//	i = fgetc(uart0io);
	//fgets(cText, 256, uart0io);

	sim_init_window();
//	uart0io = fopen("\\\\.\\COM40", "rb");*/
}

void sim_uart0io_done(void)
{
}

void sim_uart1io_init(void)
{
}

void sim_uart1io_done(void)
{
}


void _delay_us(double __us)
{
}

void _delay_ms(double __ms)
{
}

uint8_t boot_signature_byte_get(uint8_t addr)
{
}

void cli(void)
{
}

void sei(void)
{
}

uint8_t eeprom_read_byte (const uint8_t *__p)
{
}

uint8_t eeprom_write_byte (const uint8_t *__p, uint8_t value)
{
}

uint8_t eeprom_update_byte (const uint8_t *__p, uint8_t value)
{
}

void fdev_setup_stream(void* stream, void* put, void* get, int rwflag)
{
	if (stream == &_uart0io) sim_uart0io_init();
	else if (stream == &_uart1io) sim_uart1io_init();
#ifdef _SIM_LCD
	else if (stream == &_lcdio) sim_lcdio_init();
#endif //_SIM_LCD

/*#define fdev_setup_stream(stream, p, g, f) \
	do { \
		(stream)->put = p; \
		(stream)->get = g; \
		(stream)->flags = f; \
		(stream)->udata = 0; \
	} while(0) */
}

DWORD WINAPI sim_timer0_threadProc(LPVOID lpParameter)
{
	int i;
	while (1)
	{
		for (i = 0; i < 10; i++)
			TIMER0_COMPB_vect();
		Sleep(10);
	}
	return 0;
}

#ifdef _SIM_TIMER1
DWORD WINAPI sim_timer1_threadProc(LPVOID lpParameter)
{
	int i;
	while (1)
	{
		for (i = 0; i < 1000; i++)
			TIMER1_COMPA_vect();
		Sleep(20);
	}
	return 0;
}
#endif //_SIM_TIMER1
