﻿#include "StarsGameEngine.h"
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

//y方向的视角 纵横比 近剪裁平面到原点的距离 远剪裁平面到原点的距离
D3DMATRIX BuildProjectionMatrix(float fov, float aspect, float znear, float zfar)
{
	D3DMATRIX proj;
	ZeroMemory(&proj, sizeof(D3DMATRIX));

	proj._11 = 1 / tan(fov * 0.5f) / aspect;
	proj._22 = 1 / tan(fov * 0.5f);
	proj._33 = -(znear + zfar) / (zfar - znear);
	proj._34 = -(2 * znear * zfar) / (zfar - znear);
	proj._43 = -1;

	return proj;
}

D3DMATRIX MatrixDotMatrix(const D3DMATRIX& kM1, const D3DMATRIX& kM2);
D3DMATRIX BuildViewMatrix(const Vector3& kCameraPos, const Vector3& kCameraRotate)
{
	D3DMATRIX ProjView;
	ZeroMemory(&ProjView, sizeof(D3DMATRIX));
	D3DMATRIX projRotateX;
	ZeroMemory(&projRotateX, sizeof(D3DMATRIX));
	D3DMATRIX projRotateY;
	ZeroMemory(&projRotateY, sizeof(D3DMATRIX));
	D3DMATRIX projRotateZ;
	ZeroMemory(&projRotateZ, sizeof(D3DMATRIX));
	D3DMATRIX projMove;
	ZeroMemory(&projMove, sizeof(D3DMATRIX));
	D3DMATRIX projNsgats;
	ZeroMemory(&projNsgats, sizeof(D3DMATRIX));

	float fSinX = sin(-kCameraRotate.fX);
	float fCosX = cos(-kCameraRotate.fX);
	float fSinY = sin(-kCameraRotate.fY);
	float fConsY = cos(-kCameraRotate.fY);
	float fSinZ = sin(-kCameraRotate.fZ);
	float fCosZ = cos(-kCameraRotate.fZ);

	projRotateX._11 = 1;
	projRotateX._22 = fCosX;
	projRotateX._34 = -fSinX;
	projRotateX._32 = fSinX;
	projRotateX._33 = fCosX;
	projRotateX._44 = 1;

	projRotateY._11 = fConsY;
	projRotateY._13 = fSinY;
	projRotateY._22 = 1;
	projRotateY._31 = -fSinY;
	projRotateY._33 = fConsY;
	projRotateY._44 = 1;

	projRotateZ._11 = fCosZ;
	projRotateZ._12 = -fSinZ;
	projRotateZ._21 = fSinZ;
	projRotateZ._22 = fCosZ;
	projRotateZ._33 = 1;
	projRotateZ._44 = 1;

	projMove._11 = 1;
	projMove._14 = -kCameraPos.fX;
	projMove._22 = 1;
	projMove._24 = -kCameraPos.fY;
	projMove._33 = 1;
	projMove._34 = -kCameraPos.fZ;
	projMove._44 = 1;

	projNsgats._11 = 1;
	projNsgats._22 = 1;
	projNsgats._33 = -1;
	
	ProjView = MatrixDotMatrix(projRotateZ, projMove);
	ProjView = MatrixDotMatrix(projRotateX, ProjView);
	ProjView = MatrixDotMatrix(projRotateY, ProjView);
	ProjView = MatrixDotMatrix(projNsgats, ProjView);

	return ProjView;
}

SiPonit PointDotMatrix(const SiPonit& kPoint, const D3DMATRIX& kMatrix)
{
	SiPonit kReslut;
	kReslut.rhw = kPoint.rhw;
	kReslut.color = kPoint.color;
	kReslut.x = kPoint.x * kMatrix._11 + kPoint.y * kMatrix._21 + kPoint.z * kMatrix._31 + 1.f * kMatrix._41;
	kReslut.y = kPoint.x * kMatrix._12 + kPoint.y * kMatrix._22 + kPoint.z * kMatrix._32 + 1.f * kMatrix._42;
	kReslut.z = kPoint.x * kMatrix._13 + kPoint.y * kMatrix._23 + kPoint.z * kMatrix._33 + 1.f * kMatrix._43;
	kReslut.rhw = kPoint.x * kMatrix._14 + kPoint.y * kMatrix._24 + kPoint.z * kMatrix._34 + 1.f * kMatrix._43;
	return kReslut;
}

SiPonit MatrixDotPoint(const SiPonit& kPoint, const D3DMATRIX& kMatrix)
{
	SiPonit kReslut;
	kReslut.rhw = kPoint.rhw;
	kReslut.color = kPoint.color;
	kReslut.x = kMatrix._11 * kPoint.x + kMatrix._12 * kPoint.y + kMatrix._13 * kPoint.z + kMatrix._14 * 1;
	kReslut.y = kMatrix._21 * kPoint.x + kMatrix._22 * kPoint.y + kMatrix._23 * kPoint.z + kMatrix._24 * 1;
	kReslut.z = kMatrix._31 * kPoint.x + kMatrix._32 * kPoint.y + kMatrix._33 * kPoint.z + kMatrix._34 * 1;
	kReslut.rhw = kMatrix._41 * kPoint.x + kMatrix._42 * kPoint.y + kMatrix._43 * kPoint.z + kMatrix._44 * 1;
	return kReslut;
}

D3DMATRIX MatrixDotMatrix( const D3DMATRIX& kM1, const D3DMATRIX& kM2)
{
	D3DMATRIX kResult;
	kResult._11 = kM1._11 * kM2._11 + kM1._12 * kM2._21 + kM1._13 * kM2._31 + kM1._14 * kM2._41;
	kResult._12 = kM1._11 * kM2._12 + kM1._12 * kM2._22 + kM1._13 * kM2._32 + kM1._14 * kM2._42;
	kResult._13 = kM1._11 * kM2._13 + kM1._12 * kM2._23 + kM1._13 * kM2._33 + kM1._14 * kM2._43;
	kResult._14 = kM1._11 * kM2._14 + kM1._12 * kM2._24 + kM1._13 * kM2._34 + kM1._14 * kM2._44;
	kResult._21 = kM1._21 * kM2._11 + kM1._22 * kM2._21 + kM1._23 * kM2._31 + kM1._24 * kM2._41;
	kResult._22 = kM1._21 * kM2._12 + kM1._22 * kM2._22 + kM1._23 * kM2._32 + kM1._24 * kM2._42;
	kResult._23 = kM1._21 * kM2._13 + kM1._22 * kM2._23 + kM1._23 * kM2._33 + kM1._24 * kM2._43;
	kResult._24 = kM1._21 * kM2._14 + kM1._22 * kM2._24 + kM1._24 * kM2._34 + kM1._24 * kM2._44;
	kResult._31 = kM1._31 * kM2._11 + kM1._32 * kM2._21 + kM1._33 * kM2._31 + kM1._34 * kM2._41;
	kResult._32 = kM1._31 * kM2._12 + kM1._32 * kM2._22 + kM1._33 * kM2._32 + kM1._34 * kM2._42;
	kResult._33 = kM1._31 * kM2._13 + kM1._32 * kM2._23 + kM1._33 * kM2._33 + kM1._34 * kM2._43;
	kResult._34 = kM1._31 * kM2._14 + kM1._32 * kM2._24 + kM1._33 * kM2._34 + kM1._34 * kM2._44;
	kResult._41 = kM1._41 * kM2._11 + kM1._42 * kM2._21 + kM1._43 * kM2._31 + kM1._44 * kM2._41;
	kResult._42 = kM1._41 * kM2._12 + kM1._42 * kM2._22 + kM1._43 * kM2._32 + kM1._44 * kM2._42;
	kResult._43 = kM1._41 * kM2._13 + kM1._42 * kM2._23 + kM1._43 * kM2._33 + kM1._44 * kM2._43;
	kResult._44 = kM1._41 * kM2._14 + kM1._42 * kM2._24 + kM1._43 * kM2._34 + kM1._44 * kM2._44;
	return kResult;
}

/////////////////////////////////////////


StarsGameEngine::StarsGameEngine()
{
	m_VertexBuffer = nullptr;
	m_akPointVec.clear();
	m_iViewBottom = 100;
	m_iViewTop = 500;
	m_iViewLeft = 100;
	m_iViewRight = 700;
	m_kCameraPos = Vector3();
	m_kCameraRotate = Vector3();
	ZeroMemory(&m_kProjection, sizeof(D3DMATRIX));
	m_kProjection = BuildProjectionMatrix(D3DX_PI / 3, 1.333f, 40, 1000);
	ZeroMemory(&m_kView, sizeof(D3DMATRIX));
	m_kView = BuildViewMatrix(m_kCameraPos, m_kCameraRotate);
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

void StarsGameEngine::DrawTriAngle3D(int iPointX1, int iPointY1, int iPointZ1, int iPointX2, int iPointY2, int iPointZ2, int iPointX3, int iPointY3, int iPointZ3)
{
	SiPonit kPointTemp1(iPointX1, iPointY1, iPointZ1, 0);
	SiPonit kPoint1;
	kPoint1 = VertexTransform(kPointTemp1);

	SiPonit kPointTemp2(iPointX2, iPointY2, iPointZ2, 0);
	SiPonit kPoint2;
	kPoint2 = VertexTransform(kPointTemp2);

	SiPonit kPointTemp3(iPointX3, iPointY3, iPointZ3, 0);
	SiPonit kPoint3;
	kPoint3 = VertexTransform(kPointTemp3);
	DrawTriangle(kPoint1.x, kPoint1.y, kPoint2.x, kPoint2.y, kPoint3.x, kPoint3.y);
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

void StarsGameEngine::SetCameraPosition(const Vector3& kPos)
{
	m_kCameraPos = kPos;
	m_kView = BuildViewMatrix(m_kCameraPos, m_kCameraRotate);
}

Vector3 StarsGameEngine::GetCameraPosition()
{
	return m_kCameraPos;
}

void StarsGameEngine::SetCameraRotate(const Vector3& kRotate)
{
	m_kCameraRotate = kRotate;
	m_kView = BuildViewMatrix(m_kCameraPos, m_kCameraRotate);
}

Vector3 StarsGameEngine::GetCameraRotate()
{
	return m_kCameraRotate;
}

void StarsGameEngine::FillVertexBuffer(std::vector<SiPonit>& akPointVec)
{
	int iPointSize = akPointVec.size();

	HRESULT hr = device9->CreateVertexBuffer(iPointSize* sizeof(SiPonit), 0, D3D_FVF_VECTOR, D3DPOOL_SYSTEMMEM, &m_VertexBuffer, nullptr);
	if (FAILED(hr)) {
		return;
	}

	SiPonit *vectors;
	m_VertexBuffer->Lock(0, 0, (void**)&vectors, 0);

	for (int i = 0; i < iPointSize; ++i)
	{
		SiPonit& kPoint = akPointVec[i];
		int iBeginIndex = i;
		vectors[iBeginIndex] = SiPonit(kPoint.x, kPoint.y, kPoint.z, kPoint.color);
		/*vectors[iBeginIndex + 1] = SiPonit(kPoint.x * 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 2] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 3] = SiPonit(kPoint.x * 10, kPoint.y * 10 + 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 4] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10, kPoint.z, kPoint.color);
		vectors[iBeginIndex + 5] = SiPonit(kPoint.x * 10 + 10, kPoint.y * 10 + 10, kPoint.z, kPoint.color);*/
	}
	m_iVertexBufferCount = iPointSize;
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
	device9->DrawPrimitive(D3DPT_POINTLIST, 0, m_iVertexBufferCount);
	device9->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	device9->EndScene();
	device9->Present(0, 0, 0, 0);
}

SiPonit StarsGameEngine::VertexTransform(const SiPonit& kPoint)
{
	SiPonit kReslult = kPoint;
	WroldTransForm(kReslult);
	ViewTransForm(kReslult);
	ProjectionTransForm(kReslult);
	ScreenTransForm(kReslult);
	return kReslult;
}

void StarsGameEngine::WroldTransForm(SiPonit& kPoint)
{
	return;
}

void StarsGameEngine::ViewTransForm(SiPonit& kPoint)
{
	kPoint = MatrixDotPoint(kPoint, m_kView);
	return;
}

void StarsGameEngine::ProjectionTransForm(SiPonit& kPoint)
{
	kPoint = MatrixDotPoint(kPoint, m_kProjection);
}

void StarsGameEngine::ScreenTransForm(SiPonit& kPoint)
{
	kPoint.x = kPoint.x * g_iWidth / 2 / kPoint.rhw + g_iWidth / 2;
	kPoint.y = kPoint.y * g_iHeight / 2 / kPoint.rhw + g_iHeight / 2;
}
