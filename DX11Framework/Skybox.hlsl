TextureCube skybox : register(t0);
SamplerState bilinearSampler : register(s0);
cbuffer ConstantBuffer : register(b0)
{
	float4x4 Projection;
	float4x4 View;
	float4x4 World;
	float4 DiffuseLight;
	float4 DiffuseMaterial;
	float4 AmbientLight;
	float4 AmbientMaterial;
    
	float3 LightDir;
	float Count;
    
	float4 SpecularLight;
	float4 SpecularMaterial;
    
	float3 CameraPosition;
	float SpecularPower;
    
	uint EffectToggle;
	float3 __Padding;
    
}

struct VS_Out
{
	float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
	float3 texCoords : TEXCOORD;
};

//Vertex
VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoords : TEXCOORD)
{
	VS_Out output = (VS_Out)0;

	float4 Pos4 = float4(Position.xyz, 1.0f);
	output.position = mul(Pos4, World);
    output.worldPosition = output.position;
	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);
	
	output.position = output.position.xyww; //Fancy Skybox swizzle
	
	output.texCoords = Position;
	
	
	return output;
}

//Same function as simple shader fog function for similarity
float4 applyFog(float4 color, float dist)
{
    float fogAmount = 1.0f - exp(-dist * 0.01f);
    float4 fogColor = float4(0.4f, 0.5f, 0.6f, 1.0f);
    return lerp(color, fogColor, fogAmount);

}

//Pixel
float4 PS_main(VS_Out input) : SV_TARGET
{
	//Set skybox texture
	float4 Texture = skybox.Sample(bilinearSampler, input.texCoords);
	float4 TextOut = Texture;
    switch (EffectToggle)
    {
		case 1:									//Viewmat swizzle					
            return applyFog(TextOut, abs(length(View._41_42_43 - input.worldPosition)));
            break;
		default:
            return TextOut;
            break;
    }
    
}