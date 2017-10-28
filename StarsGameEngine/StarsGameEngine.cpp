#include "StarsGameEngine.h"
#include <math.h>
#pragma comment(lib, "D3D9.lib")

IDirect3D9 *d3d9 = nullptr;
IDirect3DDevice9 *device9 = nullptr;
D3DPRESENT_PARAMETERS params;

#define D3D_FVF_VECTOR (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
extern int					g_iWidth;
extern int					g_iHeight;
extern HWND					g_hGameWnd;

// 工具函数.//////////////////////////////

template<typename T>
void Swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

float fPart(float fValue)
{
	return fValue - (int)fValue;
}

float rfPart(float fValue)
{
	return 1.f - fPart(fValue);
}

void SortByY(int& iPointX1, int& iPointY1, int& iPointX2, int& iPointY2, int& iPointX3, int& iPointY3)
{
	if(iPointY1 > iPointY2)
	{
		int iTempX = iPointX1;
		int iTempY = iPointY1;
		iPointX1 = iPointX2;
		iPointY1 = iPointY2;
		iPointX2 = iTempX;
		iPointY2 = iTempY;
	}

	if(iPointY2 > iPointY3)
	{
		int iTempX = iPointX2;
		int iTempY = iPointY2;
		iPointX2 = iPointX3;
		iPointY2 = iPointY3;
		iPointX3 = iTempX;
		iPointY3 = iTempY;
	}

	if (iPointY1 > iPointY2)
	{
		int iTempX = iPointX1;
		int iTempY = iPointY1;
		iPointX1 = iPointX2;
		iPointY1 = iPointY2;
		iPointX2 = iTempX;
		iPointY2 = iTempY;
	}
}

/////////////////////////////////////////


StarsGameEngine::StarsGameEngine()
{
	m_VertexBuffer = nullptr;
	m_akPointVec.clear();
	m_iViewBottom = 10;
	m_iViewTop = 50;
	m_iViewLeft = 10;
	m_iViewRight = 70;
}

StarsGameEngine::~StarsGameEngine()
{

}


bool InitD3D(HWND hwnd)
{
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (nullptr == d3d9)
	{
		return false;
	}

	params.BackBufferWidth = g_iWidth;
	params.BackBufferHeight = g_iHeight;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;
	params.BackBufferCount = 1;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = 0;
	params.SwapEffect = D3DSWAPEFFECT_COPY;
	params.hDeviceWindow = hwnd;
	params.Windowed = TRUE;
	params.EnableAutoDepthStencil = FALSE;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = 0;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &device9);
	if (FAILED(hr)) {
		return false;
	}

	device9->SetRenderState(D3DRS_LIGHTING, false);

	return true;
}

bool StarsGameEngine::Initialize()
{
	InitD3D(g_hGameWnd);

	return true;
}

bool StarsGameEngine::Finialize()
{
	return true;
}

void StarsGameEngine::Run()
{
	if (m_akPointVec.size() > 0)
	{
		FillVertexBuffer(m_akPointVec);
		m_akPointVec.clear();
		Render(m_VertexBuffer);
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
		m_iVertexBufferCount = 0;
	}
}

void StarsGameEngine::DrawLine(int iBeginX, int iBeginY, int iEndX, int iEndY)
{
	float fTempX0 = iBeginX, fTempY0 = iBeginY, fTempX1 = iEndX, fTempY1 = iEndY;
	bool bAccept = ClipLine(fTempX0, fTempY0, fTempX1, fTempY1);
	if(!bAccept)
	{
		return;
	}
	iBeginX = fTempX0; iBeginY = fTempY0; iEndX = fTempX1; iEndY = fTempY1;

	bool bSweep = false;
	if(abs(iEndX - iBeginX) < abs(iEndY - iBeginY))
	{
		Swap(iBeginX, iBeginY);
		Swap(iEndX, iEndY);
		bSweep = true;
	}
	if(iBeginX > iEndX)
	{
		Swap(iBeginX, iEndX);
		Swap(iBeginY, iEndY);
	}

	int iDeltaX = iEndX - iBeginX;
	int iDeltaY = abs(iEndY - iBeginY);
	float fDelat = iDeltaY * 1.0f / iDeltaX;
	int iDeltaDivY = (iEndY - iBeginY) > 0 ? 1 : -1;
	int y = iBeginY;
	float fOffY = 0.f;

	//创建Rectangle  
	int iTempX, iTempY;
	for (int x = iBeginX; x < iEndX; ++x)
	{
		if (bSweep) { iTempX = y, iTempY = x; } else { iTempX = x, iTempY = y; }
		DrawPoint(iTempX, iTempY, 0xff000000);

		fOffY += fDelat;
		if(fOffY >= 0.5f)
		{
			y += iDeltaDivY;
			fOffY -= 1.0f;
		}
	}
}

// 抗锯齿
void StarsGameEngine::DrawLineAnt(float fBeginX, float fBeginY, float fEndX, float fEndY)
{
	bool bAccept = ClipLine(fBeginX, fBeginY, fEndX, fEndY);
	if (!bAccept)
	{
		return;
	}

	bool bSweep = false;
	if (abs(fEndX - fBeginX) < abs(fEndY - fBeginY))
	{
		Swap(fBeginX, fBeginY);
		Swap(fEndX, fEndY);
		bSweep = true;
	}
	if (fBeginX > fEndX)
	{
		Swap(fBeginX, fEndX);
		Swap(fBeginY, fEndY);
	}

	float fDeltaX = fEndX - fBeginX;
	float fDeltaY = fEndY - fBeginY;
	float fDelat = fDeltaY  / fDeltaX;

	int xEnd = int(fBeginX + 0.5f);
	int yEnd = fBeginY + fDelat * (xEnd - fBeginX);
	float xGap = rfPart(fBeginX + 0.5f);
	int xpxl1 = xEnd;
	int ypxl1 = int(yEnd);
	if(!bSweep)
	{
		DrawPointBrightness(xpxl1, ypxl1, rfPart(yEnd) * xGap);
		DrawPointBrightness(xpxl1, ypxl1 + 1, fPart(yEnd) * xGap);
	}
	else
	{
		DrawPointBrightness(ypxl1, xpxl1, rfPart(yEnd) * xGap);
		DrawPointBrightness(ypxl1 + 1, xpxl1, fPart(yEnd) * xGap);
	}
	float intery = yEnd + fDelat;

	xEnd = int(fEndX + 0.5f);
	yEnd = fEndY + fDelat * (xEnd - fEndX);
	xGap = fPart(fEndX + 0.5f);
	int xpxl2 = xEnd;
	int ypxl2 = int(yEnd);
	if (!bSweep)
	{
		DrawPointBrightness(xpxl2, ypxl2, rfPart(yEnd) * xGap);
		DrawPointBrightness(xpxl2, ypxl2 + 1, fPart(yEnd) * xGap);
	}
	else
	{
		DrawPointBrightness(ypxl2, xpxl2, rfPart(yEnd) * xGap);
		DrawPointBrightness(ypxl2 + 1, xpxl2, fPart(yEnd) * xGap);
	}

	for (int x = xpxl1 + 1; x <= xpxl2; ++x)
	{
		if (!bSweep)
		{
			DrawPointBrightness(x, (int)intery, rfPart(intery));
			DrawPointBrightness(x, (int)intery + 1, fPart(intery));
		}
		else
		{
			DrawPointBrightness((int)intery, x, rfPart(intery));
			DrawPointBrightness((int)intery + 1, x, fPart(intery));
		}
		intery += fDelat;
	}
}

void StarsGameEngine::DrawTriangle(int iPointX1, int iPointY1, int iPointX2, int iPointY2, int iPointX3, int iPointY3)
{
	SortByY(iPointX1, iPointY1, iPointX2, iPointY2, iPointX3, iPointY3);

	// 特殊情况
	if (iPointY1 == iPointY3)
	{
		DrawLine(iPointX1, iPointY1, iPointX2, iPointY1);
		DrawLine(iPointX1, iPointY1, iPointX3, iPointY1);
		return;
	}


	for (int y = iPointY1; y <= iPointY3; ++y)
	{
		if (y < iPointY2 || (y == iPointY2 && iPointY2 == iPointY3))
		{
			float fXS = iPointX1 == iPointX3 ? iPointX1 : iPointX1 - (iPointY1 - y) * 1.0f / (iPointY1 - iPointY3) * (iPointX1 - iPointX3);
			float fXE = iPointX1 == iPointX2 ? iPointX1 : iPointX1 - (iPointY1 - y) * 1.0f / (iPointY1 - iPointY2) * (iPointX1 - iPointX2);
			DrawLine(fXS, y, fXE, y);
		}
		else
		{
			float fXS = iPointX1 == iPointX3 ? iPointX1 : iPointX1 - (iPointY1 - y) * 1.0f / (iPointY1 - iPointY3) * (iPointX1 - iPointX3);
			float fXE = iPointX2 == iPointX3 ? iPointX2 : iPointX2 - (iPointY2 - y) * 1.0f / (iPointY2 - iPointY3) * (iPointX2 - iPointX3);
			DrawLine(fXS, y, fXE, y);
		}
	}
}

#define INSIDE 0
#define OUT_LEFT 1
#define OUT_RIGHT 2
#define OUT_BOTTOM 4
#define OUT_TOP 8

int StarsGameEngine::ComputeLineCode(float x, float y)
{
	int iOutCode = INSIDE;
	if(x < m_iViewLeft)
	{
		iOutCode |= OUT_LEFT;
	}
	else if(x > m_iViewRight)
	{
		iOutCode |= OUT_RIGHT;
	}
	if(y < m_iViewBottom)
	{
		iOutCode |= OUT_BOTTOM;
	}
	else if(y > m_iViewTop)
	{
		iOutCode |= OUT_TOP;
	}
	return iOutCode;
}

bool StarsGameEngine::ClipLine(float& fBeginX, float& fBeginY, float& fEndX, float& fEndY)
{
	float fX0 = fBeginX, fY0 = fBeginY, fX1 = fEndX, fY1 = fEndY;
	int iCode0 = ComputeLineCode(fX0, fY0);
	int iCode1 = ComputeLineCode(fX1, fY1);
	float fScope = (fEndY - fBeginY) / (fEndX - fBeginX);

	bool bAccept = false;

	while(true)
	{
		if(!(iCode0 | iCode1))	// 直线的两个点都在矩形内
		{
			bAccept = true;
			break;
		}
		else if(iCode0 & iCode1)	// 直线的两个点都在矩形的同一边，和矩形没有交点，直接返回
		{
			break;
		}
		else
		{
			float fTempX, fTempY;
			int iTempCode = iCode0 ? iCode0 : iCode1;
			if (iTempCode & OUT_LEFT)
			{
				fTempX = m_iViewLeft;
				fTempY = fY1 - fScope * (fX1 - fTempX);
			}
			else if(iTempCode & OUT_RIGHT)
			{
				fTempX = m_iViewRight;
				fTempY = fY1 - fScope * (fX1 - fTempX);
			}
			else if(iTempCode & OUT_TOP)
			{
				fTempY = m_iViewTop;
				fTempX = fX1 - (fY1 - fTempY) / fScope;
			}
			else if(iTempCode & OUT_BOTTOM)
			{
				fTempY = m_iViewBottom;
				fTempX = fX1 - (fY1 - fTempY) / fScope;
			}
			if(iTempCode == iCode0)
			{
				fX0 = fTempX;
				fY0 = fTempY;
				iCode0 = ComputeLineCode(fX0, fY0);
			}
			else
			{
				fX1 = fTempX;
				fY1 = fTempY;
				iCode1 = ComputeLineCode(fX1, fY1);
			}
		}
	}

	fBeginX = fX0;
	fBeginY = fY0;
	fEndX = fX1;
	fEndY = fY1;
	return bAccept;
}

void StarsGameEngine::DrawPoint(int iPosX, int iPosY, unsigned int iColor)
{
	m_akPointVec.push_back(SiPonit(iPosX,iPosY,0,iColor));
}

void StarsGameEngine::DrawPointBrightness(int iPosX, int iPosY, float fBrightness)
{
	DrawPoint(iPosX, iPosY, (int(fBrightness * 0x000000ff) << 24) + 0x00000000);
}

void StarsGameEngine::FillVertexBuffer(std::vector<SiPonit>& akPointVec)
{
	int iPointSize = akPointVec.size();

	HRESULT hr = device9->CreateVertexBuffer(iPointSize* sizeof(SiPonit)* 6, 0, D3D_FVF_VECTOR, D3DPOOL_SYSTEMMEM, &m_VertexBuffer, nullptr);
	if (FAILED(hr)) {
		return;
	}

	SiPonit *vectors;
	m_VertexBuffer->Lock(0, 0, (void**)&vectors, 0);

	for (int i = 0; i < iPointSize; ++i)
	{
		SiPonit& kPoint = akPointVec[i];
		int iBeginIndex = i * 6;
		vectors[iBeginIndex] = SiPonit(kPoint.x * 10, kPoint.y * 10 + 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 1] = SiPonit(kPoint.x * 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 2] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 3] = SiPonit(kPoint.x * 10, kPoint.y * 10 + 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 4] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 5] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10 + 10, kPoint.z, kPoint.color);
	}
	m_iVertexBufferCount = iPointSize * 6;
}

void StarsGameEngine::Render(IDirect3DVertexBuffer9* pkVertexBuffer)
{
	if (nullptr == pkVertexBuffer)
	{
		return;
	}
	if (nullptr == device9) 
	{
		return ;
	}
	device9->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 255, 255, 255), 0.0f, 0);
	device9->SetStreamSource(0, pkVertexBuffer, 0, sizeof(SiPonit));
	device9->SetFVF(D3D_FVF_VECTOR);
	device9->BeginScene();
	device9->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	device9->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device9->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_iVertexBufferCount / 3);
	device9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	device9->EndScene();
	device9->Present(0, 0, 0, 0);
}
