#pragma once
#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Structures.h"
#include "OBJLoader.h"
#include "DX11Framework.h"


using namespace DirectX;
struct ConstantBuffer;

class GameObject
{
private:
	ID3D11ShaderResourceView* _texture = nullptr;
	MeshData _meshData;
	XMFLOAT4X4 _world;
	void (*_updateFunction)(float,GameObject*);

public:

	GameObject(ID3D11ShaderResourceView* tex, MeshData mesh, XMFLOAT3 pos, XMFLOAT3 scale, XMFLOAT3 rot, void (*updateFunc)(float, GameObject*) = nullptr);

	void Update(float dt);

	void SetShaderResource(ID3D11ShaderResourceView* in) { _texture = in; }
	void SetMeshData(MeshData in) { _meshData = in; }

	ID3D11ShaderResourceView** GetShaderResource() { return &_texture; }
	MeshData* GetMeshData() { return &_meshData; }

	void SetWorldMat(XMFLOAT3 pos, XMFLOAT3 scale, XMFLOAT3 rot);
	XMFLOAT4X4* GetWorldMat() { return &_world; }

	XMFLOAT4 GetPosition();
	XMFLOAT4 GetRotation();
	XMFLOAT4 GetScale();

	void Draw(ID3D11DeviceContext* immediateContext, ConstantBuffer* constBufferData, ID3D11Buffer* constBuffer, ID3D11VertexShader* vs, ID3D11PixelShader* ps, ID3D11SamplerState* sampler, ID3D11GeometryShader* gs = nullptr);
};

