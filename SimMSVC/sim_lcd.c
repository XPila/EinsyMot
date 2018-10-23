#include "sim_lcd.h"
#include <stdio.h>
#include <avr/io.h>
#include <windows.h>
#include <io.h>
#include "../EinsyMot/io_atmega2560.h"

#define UM_REDRAW_CHAR             (WM_USER + 0x100)

#define LCD_CGRAM_SIZE             64
#define LCD_DDRAM_SIZE             128
#define LCD_SIZE_08x01             0x0801
#define LCD_SIZE_08x02             0x0802
#define LCD_SIZE_08x04             0x0804
#define LCD_SIZE_16x01             0x1001
#define LCD_SIZE_16x02             0x1002
#define LCD_SIZE_16x04             0x1004
#define LCD_SIZE_20x01             0x1401
#define LCD_SIZE_20x02             0x1402
#define LCD_SIZE_20x04             0x1404
#define LCD_CMD_CLEARDISPLAY       0x01
#define LCD_CMD_RETURNHOME         0x02
#define LCD_CMD_ENTRYMODESET       0x04
#define LCD_CMD_DISPLAYCONTROL     0x08
#define LCD_CMD_CURSORSHIFT        0x10
#define LCD_CMD_FUNCTIONSET        0x20
#define LCD_CMD_SETCGRAMADDR       0x40
#define LCD_CMD_SETDDRAMADDR       0x80
#define LCD_FLG_FUNC8BIT           0x10
#define LCD_FLG_FUNC2LINE          0x08
#define LCD_FLG_FUNC10DOTS         0x04
#define LCD_FLG_SHIFTDISPLAY       0x08
#define LCD_FLG_SHIFTRIGHT         0x04
#define LCD_FLG_DISPLAYON          0x04
#define LCD_FLG_CURSORON           0x02
#define LCD_FLG_BLINKON            0x01
#define LCD_FLG_ENTRYLEFT          0x01
#define LCD_FLG_ENTRYSHIFTINC      0x02

extern FILE _lcdio;
extern FILE* lcdin;
extern FILE* lcdout;

HWND sim_hWND = 0;

HANDLE sim_lcd_outH[2] = {0, 0};
int sim_lcd_outN[2] = {0, 0};
FILE* sim_lcd_outF[2] = {0, 0};

HANDLE sim_lcd_out_thread = 0;
DWORD sim_lcd_out_threadId = 0;

HANDLE sim_lcd_inH[2] = {0, 0};
int sim_lcd_inN[2] = {0, 0};
FILE* sim_lcd_inF[2] = {0, 0};

HANDLE sim_lcd_in_thread = 0;
DWORD sim_lcd_in_threadId = 0;


void sim_lcdio_init(void);
void sim_lcdio_done(void);
void sim_lcd_write(int val);
void sim_window_init(void);
void sim_window_done(void);

DWORD WINAPI sim_lcd_out_threadProc(LPVOID lpParameter);
DWORD WINAPI sim_lcd_in_threadProc(LPVOID lpParameter);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

extern const unsigned char sim_lcd_rom_A[256][10];
extern const unsigned char sim_lcd_rom_B[256][10];
extern const unsigned char sim_lcd_rom_E[256][10];

int sim_lcd_cols = 20;
int sim_lcd_rows = 4;
int sim_lcd_padding_left = 20;
int sim_lcd_padding_top = 24;
unsigned char sim_lcd_row_addr[4] = {0x00, 0x40, 0x14, 0x54};
int sim_lcd_shift = 0;
int sim_lcd_ddram_addr = 0;
int sim_lcd_cgram_addr = -1;
int sim_lcd_func8bit = 1;
int sim_lcd_func2line = 1;
int sim_lcd_func10dots = 1;
int sim_lcd_displayOn = 1;
int sim_lcd_cursorOn = 1;
int sim_lcd_blinkOn = 1;
int sim_lcd_entryLeft = 0;
int sim_lcd_entryShiftInc = 0;

unsigned char* sim_lcd_rom = (unsigned char*)sim_lcd_rom_E[0];
HBITMAP hbitmap_rom = 0;
HDC hdc_rom = 0;
HBITMAP hbitmap_rom_inv = 0;
HDC hdc_rom_inv = 0;
HBITMAP hbitmap_char = 0;
HDC hdc_char = 0;
int dots_x = 5;
int dots_y = 10;
int dot_w = 3;
int dot_h = 3;
int dot_xs = 1;
int dot_ys = 1;
int char_w = 0;
int char_h = 0;
int char_xs = 4;
int char_ys = 4;
COLORREF clr_dot = 0x00a0a0a0;
HBRUSH hbr_dot = 0;
COLORREF clr_bck = 0x00c02020;
HBRUSH hbr_bck = 0;
unsigned char sim_lcd_ddram[LCD_DDRAM_SIZE];
unsigned char sim_lcd_cgram[LCD_CGRAM_SIZE];

void sim_lcd_cmd_setDDRamAddr(int addr)
{
	sim_lcd_cgram_addr = -1; //stop CGRam write
	sim_lcd_ddram_addr = addr; //set address
}

void sim_lcd_cmd_setCGRamAddr(int addr)
{
	sim_lcd_cgram_addr = addr; //set address
}

void sim_lcd_cmd_functionSet(int func8bit, int func2line, int func10dots)
{
	sim_lcd_func8bit = func8bit;
	sim_lcd_func2line = func2line;
	sim_lcd_func10dots = func10dots;
}

void sim_lcd_cmd_cursorShift(int shiftDisplay, int shiftRight)
{
	sim_lcd_cgram_addr = -1;
	if (shiftDisplay)
	{
		sim_lcd_shift += shiftRight?1:-1;
		sim_lcd_shift &= 0x3f;
	}
	else
	{
		sim_lcd_ddram_addr += shiftRight?1:-1;
		sim_lcd_ddram_addr &= 0x7f;
	}
}

void sim_lcd_cmd_displayControl(int displayOn, int cursorOn, int blinkOn)
{
	sim_lcd_displayOn = displayOn;
	sim_lcd_cursorOn = cursorOn;
	sim_lcd_blinkOn = blinkOn; 
}

void sim_lcd_cmd_entryModeSet(int entryLeft, int entryShiftInc)
{
	sim_lcd_entryLeft = entryLeft;
	sim_lcd_entryShiftInc = entryShiftInc;
}

void sim_lcd_cmd_returnHome(void)
{
	sim_lcd_cgram_addr = -1;
	sim_lcd_ddram_addr = 0;
	sim_lcd_shift = 0;
}

void sim_lcd_cmd_clearDisplay(void)
{
	int i;
	//for (i = 0; i < LCD_CGRAM_SIZE; i++)
	//	sim_lcd_cgram[i] = 0x00;
	for (i = 0; i < LCD_DDRAM_SIZE; i++)
		sim_lcd_ddram[i] = 0x00;
	sim_lcd_cgram_addr = -1;
	sim_lcd_ddram_addr = 0;
	sim_lcd_shift = 0;
}

void sim_lcd_cmd(int cmd)
{
	if ((cmd & LCD_CMD_SETDDRAMADDR) != 0)
		sim_lcd_cmd_setDDRamAddr((byte)(cmd & 0x7f));
	else if ((cmd & LCD_CMD_SETCGRAMADDR) != 0)
		sim_lcd_cmd_setCGRamAddr((byte)(cmd & 0x3f));
	else if ((cmd & LCD_CMD_FUNCTIONSET) != 0)
		sim_lcd_cmd_functionSet((cmd & LCD_FLG_FUNC8BIT) != 0, (cmd & LCD_FLG_FUNC2LINE) != 0, (cmd & LCD_FLG_FUNC10DOTS) != 0);
	else if ((cmd & LCD_CMD_CURSORSHIFT) != 0)
		sim_lcd_cmd_cursorShift((cmd & LCD_FLG_SHIFTDISPLAY) != 0, (cmd & LCD_FLG_SHIFTRIGHT) != 0);
	else if ((cmd & LCD_CMD_DISPLAYCONTROL) != 0)
		sim_lcd_cmd_displayControl((cmd & LCD_FLG_DISPLAYON) != 0, (cmd & LCD_FLG_DISPLAYON) != 0, (cmd & LCD_FLG_BLINKON) != 0);
	else if ((cmd & LCD_CMD_ENTRYMODESET) != 0)
		sim_lcd_cmd_entryModeSet((cmd & LCD_FLG_ENTRYLEFT) != 0, (cmd & LCD_FLG_ENTRYSHIFTINC) != 0);
	else if ((cmd & LCD_CMD_RETURNHOME) != 0)
		sim_lcd_cmd_returnHome();
	else if ((cmd & LCD_CMD_CLEARDISPLAY) != 0)
		sim_lcd_cmd_clearDisplay();
}

void sim_lcd_dat(int data)
{
	if (sim_lcd_cgram_addr >= 0)
	{
		sim_lcd_cgram[sim_lcd_cgram_addr++] = data;
	}
	else
	{
		sim_lcd_ddram[sim_lcd_ddram_addr] = data;
//		sim_lcd_render_cell(sim_lcd_ddram_addr);
		PostMessage(sim_hWND, UM_REDRAW_CHAR, sim_lcd_ddram_addr, 0);
		sim_lcd_ddram_addr += sim_lcd_entryLeft?-1:1;
		sim_lcd_ddram_addr &= 0x7f;
	}
}


HBITMAP create_bitmap(int w, int h)
{
	HWND hwnd_desktop = GetDesktopWindow();
	HDC hdc_desktop = GetDC(hwnd_desktop);
	HBITMAP hbitmap = CreateCompatibleBitmap(hdc_desktop, w, h);
	ReleaseDC(hwnd_desktop, hdc_desktop);
	return hbitmap;
}

HDC create_bitmap_dc(HBITMAP hbitmap)
{
	HWND hwnd_desktop = GetDesktopWindow();
	HDC hdc_desktop = GetDC(hwnd_desktop);
	HDC hdc = CreateCompatibleDC(hdc_desktop);
	ReleaseDC(hwnd_desktop, hdc_desktop);
	SelectObject(hdc, hbitmap);
	return hdc;
}

void sim_lcd_render_byte(int x, int y, unsigned char data)
{
	RECT rc_dot = {x, y, x + dot_w, y + dot_h};
	HBRUSH hbr = GetStockObject(WHITE_BRUSH);
	int i; for (i = 0; i < dots_x; i++)
	{
		if ((data & (0x10 >> i)) != 0)
			FillRect(hdc_rom, &rc_dot, hbr);
		OffsetRect(&rc_dot, dot_w + dot_xs, 0);
	}
}

void sim_lcd_render_char(int asci)
{
	int c = (asci >> 4) & 0xf;
	int r = asci & 0xf;
	int x0 = c * char_w;
	int y0 = r * char_h;
	unsigned char* pdata = sim_lcd_rom + 10 * (asci & 0xff);
	int j; for (j = 0; j < dots_y; j++)
		sim_lcd_render_byte(x0, y0 + j * (dot_h + dot_xs), pdata[j]);		
}

void sim_lcd_render_charset(void)
{
	RECT rc = {0, 0, 16*char_w, 16*char_h};
	HBRUSH hbr = GetStockObject(BLACK_BRUSH);
	int asci;
	FillRect(hdc_rom, &rc, hbr);
	for (asci = 0; asci <= 255; asci++)
		sim_lcd_render_char(asci);
	BitBlt(hdc_rom_inv, 0, 0, 16*char_w, 16*char_h, hdc_rom, 0, 0, NOTSRCCOPY);
}

void sim_lcd_init(void)
{
	char_w = dots_x * (dot_w + dot_xs) - dot_xs;
	char_h = dots_y * (dot_h + dot_ys) - dot_ys;
	hbitmap_rom = create_bitmap(16 * char_w, 16 * char_h);
	hbitmap_rom_inv = create_bitmap(16 * char_w, 16 * char_h);
	hbitmap_char = create_bitmap(char_w, char_h);
	hdc_rom = create_bitmap_dc(hbitmap_rom);
	hdc_rom_inv = create_bitmap_dc(hbitmap_rom_inv);
	hdc_char = create_bitmap_dc(hbitmap_char);
	hbr_dot = CreateSolidBrush(clr_dot);
	hbr_bck = CreateSolidBrush(clr_bck);
	sim_lcd_render_charset();
	sim_lcd_cmd_clearDisplay();
	sim_window_init();
}

void sim_lcd_done(void)
{
	sim_window_done();
}

void sim_window_init(void)
{
	HINSTANCE hInstance = GetModuleHandle(0);
	WNDCLASS sWndClass;
	sWndClass.style = CS_HREDRAW | CS_VREDRAW;
	sWndClass.lpfnWndProc = WndProc;
	sWndClass.cbClsExtra = 0;
	sWndClass.cbWndExtra = 0;
	sWndClass.hInstance = hInstance;
	sWndClass.hIcon = LoadIcon( hInstance, IDI_APPLICATION);
	sWndClass.hCursor = LoadCursor( NULL, IDC_ARROW);
	sWndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
//	sWndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	sWndClass.lpszMenuName = NULL;
	sWndClass.lpszClassName = "EinsyMotSim";
	RegisterClass(&sWndClass);
	sim_hWND = CreateWindowEx(0, "EinsyMotSim", "EinsyMotSim", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 512, 256, NULL, NULL, hInstance, NULL);
	ShowWindow(sim_hWND, SW_SHOW);
	UpdateWindow(sim_hWND);
}

void sim_window_done(void)
{
}

void sim_lcdio_init(void)
{
	int ch = 0;
/*	int ret = 0;
	char ch[100];
	FILE* client = 0;
	int nPipe = 0;
	int nFile = 0;
	HANDLE hPipe = CreateNamedPipe("\\\\.\\Pipe\\AVRsim_lcd",
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_NOWAIT,
		2, 512, 512, 100, NULL);
	HANDLE hFile = CreateFile("\\\\.\\Pipe\\AVRsim_lcd",
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		GetLastError();
	}
	if (hFile == INVALID_HANDLE_VALUE)
	{
		GetLastError();
	}
	nFile = _open_osfhandle(hPipe, 0);
	nPipe = _open_osfhandle(hFile, 0);
//	nPipe = _open_osfhandle(hPipe, 0);
//	nFile = _open_osfhandle(hFile, 0);
	if (nPipe < 0)
	{
		GetLastError();
	}
	if (nFile < 0)
	{
		GetLastError();
	}
	lcdio = fdopen(nPipe, "w");
	client = fdopen(nFile, "w");
//	client = fopen("\\\\.\\Pipe\\AVRsim_lcd", "w+");
//	ret = fwrite("nazdar", 1, 6, lcdio);
	fprintf(lcdio, "Einsy motion\ntest\n"); //startup message
	ret = fflush(lcdio);
//	ret = fflush(client);
	ret = fread(ch, 1, 10, client);
	ret = fread(ch, 1, 10, client);
	ret = fread(ch, 1, 10, client);
	ret = fread(ch, 1, 10, client);
	ret = fread(ch, 1, 10, client);
	ret = fread(ch, 1, 10, client);
	fprintf(client, "Einsy motion\ntest\n"); //startup message
//	ret = fflush(lcdio);
	ret = fflush(client);
	ret = fread(ch, 1, 10, lcdio);
*/
	if (!CreatePipe(sim_lcd_outH+0, sim_lcd_outH+1, NULL, 64)) return;
	sim_lcd_outN[0] = _open_osfhandle(sim_lcd_outH[0], 0);
	sim_lcd_outN[1] = _open_osfhandle(sim_lcd_outH[1], 0);
	sim_lcd_outF[0] = fdopen(sim_lcd_outN[0], "r");
	sim_lcd_outF[1] = fdopen(sim_lcd_outN[1], "w");
	lcdout = sim_lcd_outF[1];
	sim_lcd_out_thread = CreateThread(0, 0, sim_lcd_out_threadProc, 0, 0, &sim_lcd_out_threadId);

	if (!CreatePipe(sim_lcd_inH+0, sim_lcd_inH+1, NULL, 1)) return;
	sim_lcd_inN[0] = _open_osfhandle(sim_lcd_inH[0], 0);
	sim_lcd_inN[1] = _open_osfhandle(sim_lcd_inH[1], 0);
	sim_lcd_inF[0] = fdopen(sim_lcd_inN[0], "r");
	sim_lcd_inF[1] = fdopen(sim_lcd_inN[1], "w");

//	fputc('\n', sim_lcd_inF[1]);
//	fflush(sim_lcd_inF[1]);
//	ch = fgetc(sim_lcd_inF[0]);

	lcdin = sim_lcd_inF[0];
	sim_lcd_in_thread = CreateThread(0, 0, sim_lcd_in_threadProc, 0, 0, &sim_lcd_in_threadId);


//	ret = fwrite("nazdar", 1, 6, lcdout);

//	fprintf(lcdio, "Einsy motion\ntest");
//	fflush(lcdio);
/*	ret = read(nFdR, ch, 4);
	ret = read(nFdR, ch+4, 6);
	if (lcdout == 0)
	{
		CloseHandle(hRd);
		CloseHandle(hWr);
		return;
	}*/
}

void sim_lcdio_done(void)
{
}

void sim_lcd_write(int val)
{
	if (val & 0x100)
		sim_lcd_cmd(val & 0xff);
	else
		sim_lcd_dat(val & 0xff);
}

DWORD WINAPI sim_lcd_out_threadProc(LPVOID lpParameter)
{
	int c;
	while ((c = fgetc(sim_lcd_outF[0])) >= 0)
	{
		lcd_put(c);
	}
	return 0;
}

DWORD WINAPI sim_lcd_in_threadProc(LPVOID lpParameter)
{
	int c;
	while ((c = fputc(0, sim_lcd_inF[1])) >= 0)
	{
		fflush(sim_lcd_inF[1]);
	}
	return 0;
}

void sim_lcd_render_cell(HDC hdc, int col, int row)
{
	int x = sim_lcd_padding_left + col * (char_w + char_xs);
	int y = sim_lcd_padding_top + row * (char_h + char_ys);
	int addr_tmp = sim_lcd_row_addr[row] + col;
	int addr = (addr_tmp & 0x40) | (((addr_tmp & 0x3f) + sim_lcd_shift) & 0x3f);
	int asci = sim_lcd_ddram[addr];
	int src_x = char_w * (asci >> 4);
	int src_y = char_h * (asci & 0xf);
	RECT rcChr = {x, y, x + char_w, y + char_h};
	SelectObject(hdc_char, hbr_dot);
	BitBlt(hdc_char, 0, 0, char_w, char_h, hdc_rom, src_x, src_y, MERGECOPY);
	SelectObject(hdc, hbr_bck);
	BitBlt(hdc, x, y, char_w, char_h, hdc_rom_inv, src_x, src_y, MERGECOPY);
	BitBlt(hdc, x, y, char_w, char_h, hdc_char, 0, 0, SRCPAINT);
}



void sim_lcd_render_display(HDC hdc, int x, int y)
{
	int row;
	int col;
	for (row = 0; row < sim_lcd_rows; row++)
		for (col = 0; col < sim_lcd_cols; col++)
			sim_lcd_render_cell(hdc, col, row); 
}

LRESULT sim_wnd_ERASEBKGND(HDC hdc)
{
	RECT rc;
//	int asc = 'A';
//	int r = asc & 0xf;
//	int c = asc >> 4;
	GetClientRect(sim_hWND, &rc);
	FillRect(hdc, &rc, hbr_bck);
	sim_lcd_render_display(hdc, 10, 10);
//	BitBlt(hdc, 0, 0, char_w, char_h, hdc_rom, c*char_w, r*char_h, SRCCOPY);
	return 1;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		return sim_wnd_ERASEBKGND((HDC)wParam);
	case UM_REDRAW_CHAR:
		InvalidateRect(hWnd, 0, 1);
		return 1;
/*	case WM_ACTIVATEAPP:
		bActive = wParam;
		break;
	case WM_PALETTECHANGED:
		if ((HWND)wParam == hWnd)
			break;
	case WM_QUERYNEWPALETTE:
		if (lpDDPal) lpDDSPrimary->SetPalette(lpDDPal);
		DDReLoadBitmap(lpDDSPicture, szBitmap);
		break;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
		case VK_F12:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_COMMAND: 
//		switch(LOWORD(wParam))
//		{
//		}
		break;

//	case WM_TIMER: 
	case WM_LBUTTONDOWN: 
		{
			HRESULT hr;

			RECT rcDstRect;
			GetClientRect(hWnd, &rcDstRect);
			POINT pt = {0, 0};
			ClientToScreen(hWnd, &pt);
			OffsetRect(&rcDstRect, pt.x, pt.y);

		    DDSURFACEDESC sDDSDPicture;
		    sDDSDPicture.dwSize = sizeof(DDSURFACEDESC);
			sDDSDPicture.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
			hr = lpDDSPicture->GetSurfaceDesc(&sDDSDPicture);
		    if(hr != S_OK) break;
			RECT rcSrcRect = {0, 0, sDDSDPicture.dwWidth, sDDSDPicture.dwHeight};

//			while(true)
			{
				DDBLTFX sDDBLTFX;
				sDDBLTFX.dwSize = sizeof(DDBLTFX);
				sDDBLTFX.dwDDFX = 0;
				sDDBLTFX.dwRotationAngle = 0;


				
				hr = lpDDSPrimary->Blt(&rcDstRect, lpDDSPicture, &rcSrcRect, DDBLT_ROTATIONANGLE, &sDDBLTFX);
/*				if (hr == S_OK) break;
				if (hr == DDERR_SURFACELOST)
				{
					if (lpDDSPrimary->Restore() != DD_OK) break;
					if (lpDDSPicture->Restore() != DD_OK) break;
					if (DDReLoadBitmap(lpDDSPicture, szBitmap) != DD_OK) break;
				}
				if (hr != DDERR_WASSTILLDRAWING) break;*/
/*			}

		}
		break;*/
	}
    return DefWindowProc(hWnd, message, wParam, lParam);
};

