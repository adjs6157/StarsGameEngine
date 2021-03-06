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

void ProcessInput()
{
	static int iLastUpdateTime = timeGetTime();
	if (timeGetTime() - iLastUpdateTime > 10)
	{
		iLastUpdateTime = timeGetTime();

		Vector3 kCameraPos = g_StarsGameEngine->GetCameraPosition();

		if (GetKeyState('W') & 0x800)
		{
			kCameraPos.fX += 30;
		}
		if (GetKeyState('S') & 0x800)
		{
			kCameraPos.fX -= 30;
		}
		if (GetKeyState('A') & 0x800)
		{
			kCameraPos.fY += 30;
		}
		if (GetKeyState('D') & 0x800)
		{
			kCameraPos.fY -= 30;
		}
		if (GetKeyState('Q') & 0x800)
		{
			kCameraPos.fZ += 30;
		}
		if (GetKeyState('E') & 0x800)
		{
			kCameraPos.fZ -= 30;
		}
		g_StarsGameEngine->SetCameraPosition(kCameraPos);

		Vector3 kCameraRotate = g_StarsGameEngine->GetCameraRotate();
		if (GetKeyState('I') & 0x800)
		{
			kCameraRotate.fX += 0.01;
		}
		if (GetKeyState('K') & 0x800)
		{
			kCameraRotate.fX -= 0.01;
		}
		if (GetKeyState('J') & 0x800)
		{
			kCameraRotate.fY += 0.01;
		}
		if (GetKeyState('L') & 0x800)
		{
			kCameraRotate.fY -= 0.01;
		}
		if (GetKeyState('U') & 0x800)
		{
			kCameraRotate.fZ += 0.01;
		}
		if (GetKeyState('O') & 0x800)
		{
			kCameraRotate.fZ -= 0.01;
		}
		g_StarsGameEngine->SetCameraRotate(kCameraRotate);
	}
}

void GameLoop()
{
	ProcessInput();

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
			iPosX += 10;
			if(iPosX > g_iWidth)
			{
				iPosX = g_iWidth;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 1:
			iPosY += 10;
			if (iPosY > g_iHeight)
			{
				iPosY = g_iHeight;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 2:
			iPosX -= 10;
			if(iPosX < 0)
			{
				iPosX = 0;
				iDir = (iDir + 1) % 4;
			}
			break;
		case 3:
			iPosY -= 10;
			if(iPosY < 0)
			{
				iPosY = 0;
				iDir = (iDir + 1) % 4;
			}
			break;
		}
		iPosXEx = rand() % (g_iWidth);
		iPosYEx = rand() % (g_iHeight);
	}
	//////画线//////////////////////////////////////////////////////////////////////////
	//g_StarsGameEngine->DrawLine(400, 300, iPosX, iPosY);
	////////////////////////////////////////////////////////////////////////////////////

	//////画线(带抗锯齿)/////////////////////////////////////////////////////////////////
	//g_StarsGameEngine->DrawLineAnt(400, 300, iPosX, iPosY);
	////////////////////////////////////////////////////////////////////////////////////

	//////2D三角形//////////////////////////////////////////////////////////////////////////
	//g_StarsGameEngine->DrawTriangle(400, 300, iPosX, iPosY, iPosXEx, iPosYEx);
	////////////////////////////////////////////////////////////////////////////////////

	///////3D三角形///////////////////////////////////////////////////////////////////////////
	//g_StarsGameEngine->DrawTriAngle3D(100, 150, 150, 100, 150, -50, 100, -50, -50, 0xffff0000);
	//g_StarsGameEngine->DrawTriAngle3D(100, 150, 150, 100, -50, 150, 100, -50, -50, 0xffff0000);
	//g_StarsGameEngine->DrawTriAngle3D(120, 100, 100, 120, 100, -100, 120, -100, -100, 0xff000000);
	//g_StarsGameEngine->DrawTriAngle3D(120, 100, 100, 120, -100, 100, 120, -100, -100, 0xff000000);
	/////////////////////////////////////////////////////////////////////////////////////////

	////////正方形////////////////////////////////////////////////////////////////////////////
	//// 左
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 100, 100, 0, 100, 100, 100, 0, 0xff000000);
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 0, 100, 0, 100, 100, 0, 0, 0xff000000);
	//// 右
	//g_StarsGameEngine->DrawTriAngle3D(200, 100, 100, 200, 0, 100, 200, 100, 0, 0xffff0000);
	//g_StarsGameEngine->DrawTriAngle3D(200, 100, 0, 200, 0, 100, 200, 0, 0, 0xffff0000);
	//// 上
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 100, 200, 100, 100, 200, 100, 0, 0xff00ff00);
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 100, 200, 100, 0, 100, 100, 0, 0xff00ff00);
	//// 下
	//g_StarsGameEngine->DrawTriAngle3D(100, 0, 100, 200, 0, 100, 200, 0, 0, 0xff0000ff);
	//g_StarsGameEngine->DrawTriAngle3D(100, 0, 100, 200, 0, 0, 100, 0, 0, 0xff0000ff);
	//// 前
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 0, 200, 100, 0, 100, 0, 0, 0xffffff00);
	//g_StarsGameEngine->DrawTriAngle3D(200, 100, 0, 200, 0, 0, 100, 0, 0, 0xffffff00);
	//// 后
	//g_StarsGameEngine->DrawTriAngle3D(100, 100, 100, 200, 100, 100, 100, 0, 100, 0xff00ffff);
	//g_StarsGameEngine->DrawTriAngle3D(200, 100, 100, 200, 0, 100, 100, 0, 100, 0xff00ffff);
	/////////////////////////////////////////////////////////////////////////////////////////

	/////////UV 贴图///////////////////////////////////////////////////////////////////////////
	g_StarsGameEngine->SetEngineFlag(StarsEngineFlag_Draw_UV);
	SiPonit kPoint1(100, 150, 150, 0xFFFFFFFF); kPoint1.u = 0; kPoint1.v = 0;
	SiPonit kPoint2(100, 150, -50, 0xFFFFFFFF); kPoint2.u = 1; kPoint2.v = 0;
	SiPonit kPoint3(100, -50, -50, 0xFFFFFFFF); kPoint3.u = 1; kPoint3.v = 1;
	SiPonit kPoint4(100, -50, 150, 0xFFFFFFFF); kPoint4.u = 0; kPoint4.v = 1;
	g_StarsGameEngine->DrawTriAngle3D(kPoint1, kPoint2, kPoint3);
	g_StarsGameEngine->DrawTriAngle3D(kPoint1, kPoint4, kPoint3);
	////////////////////////////////////////////////////////////////////////////////////////////

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
