#pragma once
#include <vector>
#include <Windows.h>
#include <D3D9.h>

struct SiPonit {
	SiPonit(float v_x, float v_y, float v_z, DWORD c) : x(v_x), y(v_y), z(v_z), color(c) {
		rhw = 1.0f;
	}
	float x, y, z, rhw;
	DWORD color;
};

class StarsGameEngine
{
public:
	StarsGameEngine();
	~StarsGameEngine();
	bool Initialize();
	bool Finialize();
	void Run();
	void DrawLine(int iBeginX, int iBeginY, int iEndX, int iEndY);
	void DrawLineAnt(float fBeginX, float fBeginY, float fEndX, float fEndY);
	bool ClipLine(float& fBeginX, float& fBeginY, float& fEndX, float& fEndY);
	void DrawPoint(int iPosX, int iPosY, unsigned int iColor);
	void DrawPointBrightness(int iPosX, int iPosY, float fBrightness);

private:
	int ComputeLineCode(float x, float y);
	void FillVertexBuffer(std::vector<SiPonit>& m_akPointVec);
	void Render(IDirect3DVertexBuffer9* pkVertexBuffer);
private:
	std::vector<SiPonit> m_akPointVec;
	IDirect3DVertexBuffer9* m_VertexBuffer;
	int						m_iVertexBufferCount;
	int						m_iViewLeft;
	int						m_iViewRight;
	int						m_iViewBottom;
	int						m_iViewTop;
};