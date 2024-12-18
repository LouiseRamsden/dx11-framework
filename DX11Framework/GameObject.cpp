#include "GameObject.h"


GameObject::GameObject(ID3D11ShaderResourceView* tex, MeshData mesh, XMFLOAT3 pos, XMFLOAT3 scale, XMFLOAT3 rot, void (*updateFunc)(float, GameObject*))
{
	//Set private vars
	(_updateFunction) = updateFunc;
	_texture = tex;
	_meshData = mesh;
	//Create World Mat
	XMStoreFloat4x4(&this->_world, XMMatrixIdentity() *
		XMMatrixScaling(scale.x, scale.y, scale.z) *
		XMMatrixRotationX(rot.x) * XMMatrixRotationY(rot.y) * XMMatrixRotationZ(rot.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z));
}

void GameObject::Update(float dt) 
{	
	//run update function if it exists in memory
	if (_updateFunction != nullptr) 
	{
		_updateFunction(dt,this);
		return;
	}
}

void GameObject::SetWorldMat(XMFLOAT3 pos, XMFLOAT3 scale, XMFLOAT3 rot)
{
	XMStoreFloat4x4(&this->_world, XMMatrixIdentity() * 
		XMMatrixScaling(scale.x, scale.y, scale.z) * 
		XMMatrixRotationX(rot.x) * XMMatrixRotationY(rot.y) * XMMatrixRotationZ(rot.z) *
		XMMatrixTranslation(pos.x, pos.y, pos.z));
};

void GameObject::Draw(ID3D11DeviceContext* immediateContext, ConstantBuffer* constBufferData, ID3D11Buffer* constBuffer, ID3D11VertexShader* vs, ID3D11PixelShader* ps ,ID3D11SamplerState* sampler, ID3D11GeometryShader* gs) 
{

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	constBufferData->World = XMMatrixTranspose(XMLoadFloat4x4(this->GetWorldMat()));

	immediateContext->Map(constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, constBufferData, sizeof(ConstantBuffer));
	immediateContext->Unmap(constBuffer, 0);

	UINT stride = { sizeof(SimpleVertex) };
	UINT offset = 0;
	immediateContext->IASetVertexBuffers(0, 1, &this->GetMeshData()->VertexBuffer, &this->GetMeshData()->VBStride, &this->GetMeshData()->VBOffset);
	immediateContext->IASetIndexBuffer(this->GetMeshData()->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	immediateContext->VSSetShader(vs, nullptr, 0);
	immediateContext->GSSetShader(gs, nullptr, 0);//With Geometry Shader, it should unbind it for everything that doesnt use it
	immediateContext->PSSetShader(ps, nullptr, 0);
	immediateContext->PSSetSamplers(0, 1, &sampler);
	immediateContext->PSSetShaderResources(0, 1, &this->_texture);

	immediateContext->DrawIndexed(this->GetMeshData()->IndexCount, 0, 0);//Draw :)
}