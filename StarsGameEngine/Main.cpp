#include <Windows.h>
#include "StarsGameEngine.h"

HWND				g_hGameWnd = NULL;
StarsGameEngine*	g_StarsGameEngine = NULL;
int					g_iWidth = 800;
int					g_iHeight = 600;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}
}

BOOL CreateWnd(HINSTANCE hInstance)
{
	WCHAR acWindowClass[256] = L"StarsGameEngine";

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = acWindowClass;
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	DEVMODE devmode;
	memset(&devmode, 0, sizeof(devmode));
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	int iMaxGraphWidth = devmode.dmPelsWidth;
	int iMaxGraphHeight = devmode.dmPelsHeight;
	int iWindowX = iMaxGraphWidth / 2 - g_iWidth / 2;
	int iWindowY = iMaxGraphHeight / 2 - g_iHeight / 2 - 32;

	g_hGameWnd = CreateWindow(acWindowClass, acWindowClass, (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME, iWindowX, iWindowY, g_iWidth, g_iHeight, NULL, NULL, hInstance, NULL);
	if(!g_hGameWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hGameWnd, SW_SHOW);
	UpdateWindow(g_hGameWnd);
	BringWindowToTop(g_hGameWnd);
	return TRUE;
}


BOOL InitGame()
{
	g_StarsGameEngine = new StarsGameEngine();
	if(!g_StarsGameEngine)
	{
		return FALSE;
	}

	if(!g_StarsGameEngine->Initialize())
	{
		return FALSE;
	}
	return TRUE;
}

void GameLoop()
{
	static int iLastTime = timeGetTime();
	static int iPosX = 0;
	static int iPosY = 0;
	static int iDir = 0;
	static int iPosXEx = 0;
	static int iPosYEx = 0;
	if(timeGetTime() - iLastTime > 200)
	{
		iLastTime = timeGetTime();
		switch(iDir)
		{
		case 0:
			iPosX += 2;
			if(iPosX * 10 > g_iWidth)
			{
				iPosX = g_iWidth / 10;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 1:
			iPosY += 2;
			if (iPosY * 10 > g_iHeight)
			{
				iPosY = g_iHeight / 10;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 2:
			iPosX -= 2;
			if(iPosX < 0)
			{
				iPosX = 0;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 3:
			iPosY -= 2;
			if(iPosY < 0)
			{
				iPosY = 0;
				iDir = (iDir + 1) % 4;
			}
			break;
		}
		iPosXEx = rand() % (g_iWidth / 10);
		iPosYEx = rand() % (g_iHeight / 10);
	}

	//g_StarsGameEngine->DrawLineAnt(40, 30, iPosX, iPosY);
	g_StarsGameEngine->DrawTriangle(40, 30, iPosX, iPosY, iPosXEx, iPosYEx);
	g_StarsGameEngine->Run();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if(!CreateWnd(hInstance))
	{
		return 0;
	}

	if(!InitGame())
	{
		return 0;
	}

	MSG msg;
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}
			if(!TranslateAccelerator(msg.hwnd, NULL, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			continue;
		}

		GameLoop();

		static int s_iFrameTime = timeGetTime();

		int iDeltaTime = timeGetTime() - s_iFrameTime;
		iDeltaTime = 16 - iDeltaTime;

		if(iDeltaTime > 0)
		{
			Sleep(iDeltaTime);
		}

		s_iFrameTime = timeGetTime();
	}
	return true;
}
