#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
//#include <wrl.h>

#include "DDSTextureLoader.h"
#include "Camera.h"
#include "OBJLoader.h"
#include "Structures.h"
class GameObject;

typedef float FLOAT32;

using namespace DirectX;
//using Microsoft::WRL::ComPtr;


struct ConstantBuffer
{
	XMMATRIX Projection;
	XMMATRIX View;
	XMMATRIX World;
	XMFLOAT4 DiffuseLight;
	XMFLOAT4 DiffuseMaterial;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;
	XMFLOAT3 LightDir;
	FLOAT32 Count;
	XMFLOAT4 SpecularLight;
	XMFLOAT4 SpecularMaterial;
	XMFLOAT3 CameraPosition;
	FLOAT32 SpecularPower;
	uint32_t EffectToggle;
	XMFLOAT3 _PADDING;
};

class DX11Framework
{
	int _WindowWidth = 1280;
	int _WindowHeight = 768;

	ID3D11DeviceContext* _immediateContext = nullptr;
	ID3D11Device* _device;
	IDXGIDevice* _dxgiDevice = nullptr;
	IDXGIFactory2* _dxgiFactory = nullptr;
	ID3D11RenderTargetView* _frameBufferView = nullptr;
	IDXGISwapChain1* _swapChain;
	D3D11_VIEWPORT _viewport;

	ID3D11RasterizerState* _fillState;
	ID3D11RasterizerState* _wireframeState;

	ID3D11VertexShader* _vertexShader;
	ID3D11VertexShader* _skyBoxVertexShader;
	ID3D11VertexShader* _waterVertexShader;
	ID3D11InputLayout* _inputLayout;

	ID3D11GeometryShader* _waterGeometryShader;

	ID3D11PixelShader* _pixelShader;
	ID3D11PixelShader* _skyBoxPixelShader;
	ID3D11Buffer* _constantBuffer;
	ID3D11Buffer* _cubeVertexBuffer;
	ID3D11Buffer* _cubeIndexBuffer;
	ID3D11Buffer* _pyramidVertexBuffer;
	ID3D11Buffer* _pyramidIndexBuffer;

	MeshData _mesh;
	GameObject* _gameObject;
	GameObject* _water;
	GameObject* _skyBox;
	GameObject* _teapot;

	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11DepthStencilState* _depthStencilSkybox;
	ID3D11SamplerState* _bilinearSamplerState;
	ID3D11BlendState* _transparencyBlendState;
	ID3D11ShaderResourceView* _waterTexture;
	ID3D11ShaderResourceView* _sandTexture;
	ID3D11ShaderResourceView* _skyboxTexture;
	ID3D11ShaderResourceView* _teapotTexture;


	HWND _windowHandle;

	XMFLOAT4X4 _Planet;
	XMFLOAT4X4 _Moon;
	XMFLOAT4X4 _View;
	XMFLOAT4X4 _Projection;

	Camera* _camera;
	Camera* _staticCamera[3];

	ConstantBuffer _cbData;

	XMFLOAT4 _diffuseLight;
	XMFLOAT4 _diffuseMaterial;
	XMFLOAT4 _ambientLight;
	XMFLOAT4 _ambientMaterial;
	XMFLOAT3 _lightDir;
	XMFLOAT4 _specularLight;
	XMFLOAT4 _specularMaterial;
	XMFLOAT3 _cameraPosition;
	FLOAT32 _specularPower;
	INT32 _effectToggle;

public:
	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateD3DDevice();
	HRESULT CreateSwapChainAndFrameBuffer();
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexIndexBuffers();
	HRESULT InitPipelineVariables();
	HRESULT InitRunTimeData();
	~DX11Framework();
	void Update();
	void Draw();
};