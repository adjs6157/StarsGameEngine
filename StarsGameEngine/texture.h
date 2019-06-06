#pragma once
#include <Windows.h>
#include "math.h"
#include <string>
#include "assert.h"

class Texture {
public:
    Texture(): _data(nullptr), _w(0), _h(0), _channel(0)
    { }
    
    ~Texture();

    DWORD GetColor(float u, float v)
    {
        assert(_data != nullptr);

        int i = ConverToIndex(u, v);

		return 0xFF000000 + (_data[i] << 16) + (_data[i + 1] << 8) + (_data[i + 2]);
    }

	unsigned int GetW() const { return _w; }
	unsigned int GetH() const { return _h; }
    bool InitFromFile(const std::string & fname);

private:
    int ConverToIndex(float u, float v)
    {
        u = Math::Clamp(u, 1.0f, 0.0f);
        v = Math::Clamp(v, 1.0f, 0.0f);
        int x = (u * (_w - 1));
		int y = (v * (_h - 1));
        return static_cast<int>(y * _w * _channel + x * _channel);
    }

private:
    unsigned int _channel;
	unsigned int _w;
	unsigned int _h;
	unsigned char * _data;
};
