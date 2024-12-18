#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>


#pragma once

using namespace DirectX;


enum CameraType 
{
	DYNAMIC,
	STATIC
};


class Camera
{	
	XMFLOAT4X4 _viewMat;
	XMFLOAT4X4 _projMat;
	XMFLOAT3 _eye;
	XMFLOAT3 _at;
	XMFLOAT3 _up;
	XMFLOAT3 _forward;
	float _roll = 0.0f;
	float _pitch = 0.0f;
	float _yaw = 0.0f;
	float _fwOffset = 0.0f;
	float _upOffset = 0.0f;
	float _rtOffset = 0.0f;
	CameraType _type;
	

public:
	void Update(float dt);
	inline float GetRoll() { return _roll; }
	inline float GetPitch() { return _pitch; }
	inline float GetYaw() { return _yaw; }
	inline void SetRoll(float newRoll) { _roll = newRoll; }
	inline void SetPitch(float newPitch) { _pitch = newPitch; }
	inline void SetYaw(float newYaw) { _yaw = newYaw; }
	inline void MoveFwd(float amount) { _fwOffset = amount; }
	inline void MoveUp(float amount) { _upOffset = amount; }
	inline void MoveRight(float amount) { _rtOffset = amount; }
	inline void ResetValues() {_roll = 0.0f; _pitch = 0.0f; _yaw = 0.0f; _fwOffset = 0.0f; _upOffset = 0.0f; _rtOffset = 0.0f; }

	inline XMFLOAT4X4* GetViewMatrix() { return &this->_viewMat; }
	inline XMFLOAT4X4* GetProjectionMatrix() { return &this->_projMat; }

	Camera(D3D11_VIEWPORT viewPort,
		XMFLOAT3 eye = XMFLOAT3(0.0f, 10.0f, -20.0f), 
		XMFLOAT3 at = XMFLOAT3(0, 0, 1), 
		XMFLOAT3 up = XMFLOAT3(0, 1, 0), CameraType type = DYNAMIC);
};
