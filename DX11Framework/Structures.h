#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 Texcoord;

	inline bool operator<(const SimpleVertex other) const 
	{
		return memcmp((void*)this, (void*)&other, sizeof(SimpleVertex)) > 0;
	}
};