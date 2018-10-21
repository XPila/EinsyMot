#include "sim_lcd.h"
#include <stdio.h>
#include <avr/io.h>
#include <windows.h>
#include <io.h>

extern FILE _lcdio;
extern FILE* lcdin;
extern FILE* lcdout;

HWND sim_hWND = 0;

HANDLE sim_lcd_outH[2] = {0, 0};
int sim_lcd_outN[2] = {0, 0};
FILE* sim_lcd_outF[2] = {0, 0};
HANDLE sim_lcd_thread = 0;
DWORD sim_lcd_threadId = 0;

void sim_lcdio_init(void);
void sim_lcdio_done(void);
void sim_lcd_write(uint8_t val);
void sim_window_init(void);
void sim_window_done(void);

DWORD WINAPI sim_lcd_threadProc(LPVOID lpParameter);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


void sim_lcd_init(void)
{
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
	sim_hWND = CreateWindowEx(0, "EinsyMotSim", "EinsyMotSim", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 200, NULL, NULL, hInstance, NULL);
	ShowWindow(sim_hWND, SW_SHOW);
	UpdateWindow(sim_hWND);
}

void sim_window_done(void)
{
}

void sim_lcdio_init(void)
{
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
	sim_lcd_thread = CreateThread(0, 0, sim_lcd_threadProc, 0, 0, &sim_lcd_threadId);
	lcdout = sim_lcd_outF[1];
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

void sim_lcd_write(uint8_t val)
{
}

DWORD WINAPI sim_lcd_threadProc(LPVOID lpParameter)
{
	int c;
	while ((c = fgetc(sim_lcd_outF[0])) >= 0)
	{
		lcd_put(c);
	}
	return 0;
}


void sim_lcd_draw_char(HDC hdc, int x, int y, char ch)
{
	RECT rcChr = {x, y, x+100, y+100};
//	RECT rcPix = {};
	FillRect(hdc, &rcChr, GetStockObject(WHITE_BRUSH));
//	for (int r = 0; r < 8; r++)
//		for (int c = 0; c < 10; c++)
}

LRESULT sim_wnd_ERASEBKGND(HDC hdc)
{
	RECT rc;
	GetClientRect(sim_hWND, &rc);
	FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH));
	sim_lcd_draw_char(hdc, 10, 10, 'a');
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
