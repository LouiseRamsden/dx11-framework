#include "Camera.h"

Camera::Camera(D3D11_VIEWPORT viewport, XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up, CameraType type) 
{

	float aspect = viewport.Width / viewport.Height;

	_eye = eye;
	_forward = at;
	_up = up;

	_type = type;

	SetPitch(0.0f);
	SetYaw(0.0f);

	//setup view and projection matrix
	XMStoreFloat4x4(&_viewMat, XMMatrixLookToLH(XMLoadFloat3(&_eye), XMLoadFloat3(&_forward), XMLoadFloat3(&_up)));
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), aspect, 0.01f, 100.0f);
	XMStoreFloat4x4(&_projMat, perspective);
}

void Camera::Update(float dt) 
{
	//Skip everything else if camera is static
	if (_type == STATIC) 
	{
		ResetValues();
		return;
	}
	
	//Handle Keyboard inputs for camera
	if (GetAsyncKeyState('W') & 0xF000)
		MoveFwd(10.0f * dt);
	if (GetAsyncKeyState('S') & 0xF000)
		MoveFwd(-10.0f * dt); 
	if (GetAsyncKeyState('Q') & 0xF000)
		SetRoll(80.0f * dt);
	if (GetAsyncKeyState('E') & 0xF000)
		SetRoll(-80.0f * dt);
	if (GetAsyncKeyState('A') & 0xF000)
		MoveRight(-10.0f * dt);
	if (GetAsyncKeyState('D') & 0xF000)
		MoveRight(10.0f * dt);
	if (GetAsyncKeyState(VK_SPACE) & 0xF000)
		MoveUp(10.0f * dt);
	if (GetAsyncKeyState(VK_SHIFT) & 0xF000)
		MoveUp(-10.0f * dt);
	if (GetAsyncKeyState(VK_RIGHT) & 0xF000)
		SetYaw(80.0f * dt);
	if (GetAsyncKeyState(VK_LEFT) & 0xF000)
		SetYaw(-80.0f * dt);
	if (GetAsyncKeyState(VK_UP) & 0xF000)
		SetPitch(-80.0f * dt);
	if (GetAsyncKeyState(VK_DOWN) & 0xF000)
		SetPitch(80.0f * dt);


	//grab rotation matrices for current inputs and invert view matrix
	XMMATRIX cameraTransform = XMMatrixInverse(nullptr, XMLoadFloat4x4(&_viewMat));
	XMVECTOR right = cameraTransform.r[0];
	XMVECTOR up = cameraTransform.r[1];
	XMVECTOR forward = cameraTransform.r[2];
	XMMATRIX rollRot = XMMatrixRotationAxis(forward, XMConvertToRadians(GetRoll()));
	XMMATRIX pitchRot = XMMatrixRotationAxis(right, XMConvertToRadians(GetPitch()));
	XMMATRIX yawRot = XMMatrixRotationAxis(up, XMConvertToRadians(GetYaw()));

	//rotate based on calculated matrices
	cameraTransform.r[0] = XMVector3TransformNormal(cameraTransform.r[0], rollRot);
	cameraTransform.r[1] = XMVector3TransformNormal(cameraTransform.r[1], rollRot);
	cameraTransform.r[1] = XMVector3TransformNormal(cameraTransform.r[1], pitchRot);
	cameraTransform.r[2] = XMVector3TransformNormal(cameraTransform.r[2], pitchRot);
	cameraTransform.r[0] = XMVector3TransformNormal(cameraTransform.r[0], yawRot);
	cameraTransform.r[2] = XMVector3TransformNormal(cameraTransform.r[2], yawRot);
	//move based on inputs
	cameraTransform.r[3] = cameraTransform.r[3] + (cameraTransform.r[2] * _fwOffset);
	cameraTransform.r[3] = cameraTransform.r[3] + (cameraTransform.r[1] * _upOffset);
	cameraTransform.r[3] = cameraTransform.r[3] + (cameraTransform.r[0] * _rtOffset);

	XMStoreFloat4x4(&_viewMat, XMMatrixInverse(nullptr, cameraTransform));//set other cameratransforms as view matrix
	ResetValues();
	return;
}
