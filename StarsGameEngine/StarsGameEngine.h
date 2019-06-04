#pragma once
#include <vector>
#include <Windows.h>
#include <D3D9.h>
#include "texture.h"

struct SiPonit {
	SiPonit()
	{
		x = y = z = 0.f;
		rhw = 1.0f;
		color = 0;
		u = v = 0.f;
	}
	SiPonit(float v_x, float v_y, float v_z, DWORD c) 
		: x(v_x), y(v_y), z(v_z), color(c) 
	{
		rhw = 1.0f;
	}
	float x, y, z, rhw;
	DWORD color;
	float u, v;
};

struct Vector3
{
	Vector3()
	{
		fX = 0;
		fY = 0;
		fZ = 0;
	}

	Vector3(float _fX, float _fY, float _fZ)
	{
		fX = _fX;
		fY = _fY;
		fZ = _fZ;
	}

	float fX;
	float fY;
	float fZ;
};

struct Triangle
{
	Triangle()
	{
		point1 = SiPonit();
		point2 = SiPonit();
		point3 = SiPonit();
		corePoint = SiPonit();
	}

	Triangle(const SiPonit kPoint1, const SiPonit kPoint2, const SiPonit kPoint3)
	{
		point1 = kPoint1;
		point2 = kPoint2;
		point3 = kPoint3;
	}

	SiPonit point1;
	SiPonit point2;
	SiPonit point3;
	SiPonit corePoint;
};

#define D3DX_PI 3.1415926f
#define D3DX_2PI (D3DX_PI * 2)

enum StarsEngineFlag
{
	StarsEngineFlag_None = 1 << 0,			
	StarsEngineFlag_ZOrder = 1 << 1,		// 开启深度缓冲检测
	StarsEngineFlag_ZWrite = 1 << 2,		// 开启写深度缓冲
	StarsEngineFlag_Draw_Line = 1 << 3,		// 开启线框模式
	StarsEngineFlag_Draw_Fill = 1 << 4,		// 开启填充模式
	StarsEngineFlag_Draw_UV = 1 << 4,		// 开启UV贴图
};

class StarsGameEngine
{
public:
	StarsGameEngine();
	~StarsGameEngine();
	bool Initialize();
	bool Finialize();
	void ClearFrame();
	void Run();
	void DrawLine(SiPonit kPointB, SiPonit kPointE);
	void DrawLineAnt(float fBeginX, float fBeginY, float fEndX, float fEndY);
	void DrawTriangle(SiPonit kPoint1, SiPonit kPoint2, SiPonit kPoint3);
	void DrawTriAngle3D(int iPointX1, int iPointY1, int iPointZ1, int iPointX2, int iPointY2, int iPointZ2, int iPointX3, int iPointY3, int iPointZ3, unsigned int iColor = 0xff000000);
	void DrawTriAngle3D(SiPonit kPoint1, SiPonit kPoint2, SiPonit kPoint3);
	bool ClipLine(float& fBeginX, float& fBeginY, float& fEndX, float& fEndY);
	void DrawPoint(int iPosX, int iPosY, int iPosZ, unsigned int iColor);
	void DrawPointBrightness(int iPosX, int iPosY, float fBrightness);
	void SetCameraPosition(const Vector3& kPos);
	Vector3 GetCameraPosition();
	void SetCameraRotate(const Vector3& kRotate);
	Vector3 GetCameraRotate();
	bool HasEngineFlag(int iFlag);
	void SetEngineFlag(int iFlag);
private:
	int ComputeLineCode(float x, float y);
	void FillVertexBuffer(SiPonit* akFrame, int iWidth, int iHeight);
	void Render(IDirect3DVertexBuffer9* pkVertexBuffer);
	SiPonit VertexTransform(const SiPonit&  kPoint);
	void WroldTransForm(SiPonit& kPoint);
	void ViewTransForm(SiPonit& kPoint);
	void ProjectionTransForm(SiPonit& kPoint);
	void ScreenTransForm(SiPonit& kPoint);
	void ArtistSort(std::vector<Triangle>& akTriangleVec);
	void VertexShader(const SiPonit& kPoint, SiPonit& kPointOut);
	void FragmentShader(const SiPonit& kPoint, DWORD& kColorOut);
	void FragmentShader_Tex(const SiPonit& kPoint, DWORD& kColorOut);
private:
	//std::vector<SiPonit> m_akPointVec;
	SiPonit*				m_akFrame;
	int*					m_aiZOrder;
	IDirect3DVertexBuffer9* m_VertexBuffer;
	int						m_iVertexBufferCount;
	int						m_iViewLeft;
	int						m_iViewRight;
	int						m_iViewBottom;
	int						m_iViewTop;
	D3DMATRIX				m_kProjection;
	Vector3					m_kCameraPos;
	Vector3					m_kCameraRotate;
	D3DMATRIX				m_kView;
	std::vector<Triangle> m_akTriangleVec;
	int						m_iStarsEngineFlag;
	Texture					m_kTexture;
};