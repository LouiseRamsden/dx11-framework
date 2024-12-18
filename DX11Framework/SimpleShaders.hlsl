//Basic Shader for most objects

Texture2D diffuseTex : register(t0);
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
    float4 color : COLOR;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL0;
    float3 viewPosition : POSITION1;
    float2 texCoords : TEXCOORD;
};

//Simple Vertex
VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoords : TEXCOORD)
{   
    VS_Out output = (VS_Out)0;
    
    float4 Pos4 = float4(Position, 1.0f);
    
    //To World
    output.position = mul(Pos4, World);
    output.worldNormal = mul(normalize(Normal), (float3x3) World);
    output.worldPosition = output.position.xyz;
    
    //To View
    output.position = mul(output.position, View);
    //To Proj
    output.position = mul(output.position, Projection);
    //Set TexCoords
    output.texCoords = TexCoords;
   
    return output;
}

//Gooch and Phong Shading Functions
float3 GoochDiffuse(float3 lightDirection, float3 worldNormals, float4 hotColor, float4 coldColor)
{
    //G = (1 + ln) / 2
    //G * h + (1 - G) * c
    float GoochAmount = (1.0f + dot(lightDirection, worldNormals)) / 2.0f;
    return GoochAmount * hotColor + (1 - GoochAmount) * coldColor;
}
float3 PhongDiffuse(float3 lightDirection, float3 worldNormal, float4 diffuseMaterial, float4 diffuseLight)
{
    //D = sat(ln) * m 
    float DiffuseAmount = saturate(dot(lightDirection, normalize(worldNormal)));
    return DiffuseAmount * (diffuseMaterial * diffuseLight);
}
float3 PhongAmbient(float4 ambientLight, float4 ambientMaterial)
{
    //a = m
    return ambientLight * ambientMaterial;
}
float3 PhongSpecular(float3 lightDirection, float3 cameraPosition, float3 worldNormal, float3 worldPosition, float4 specularLight, float4 specularMaterial,float specularPower)
{
    //float3 worldToCamera = cameraPosition - worldPosition;
    //float normalLightDot = dot(normalize(worldNormal), normalize(lightDirection));

	float3 Reflection = normalize(2 * dot(normalize(worldNormal), normalize(lightDirection)) * worldNormal) * (dot(worldNormal, lightDirection) > 0.0f);
    return pow(saturate(dot(Reflection, normalize(-cameraPosition - worldPosition))), specularPower) * (specularLight * specularMaterial);
    
    //Blinn phong implementation from Foundations of Game Engine Development Vol-2 Page 96 (roughly)
    //float3 halfAngle = normalize(-cameraPosition + lightDirection);
    //float highlight = pow(saturate(dot(worldNormal, halfAngle)), specularPower) * (dot(worldNormal, lightDirection) > 0.0);
    //return (specularLight * specularMaterial * highlight);
    
}
// Struggles with transparency
float4 applyFog(float4 color, float dist)
{
    //Basic Fog Implementation
    float fogAmount = 1.0f - exp(-dist * 0.03f);
    float4 fogColor = float4(0.4f, 0.5f, 0.6f, 5.0f);
    return lerp(color, fogColor, fogAmount);

}
//Pixel
float4 PS_main(VS_Out input) : SV_TARGET
{

    // Set Gooch Shader Colour Values
    float4 red = float4(1.0f, 0.0f, 0.0f, 1.0f);
    float4 blue = float4(0.0f, 0.0f, 1.0f, 1.0f);
    
    //Calculate the out values of Gooch and Phong Diffuse, Ambient, Specular and Diffuse Texture
    float3 GoochOut = GoochDiffuse(LightDir, input.worldNormal, red, blue);

    float3 DiffuseOut = PhongDiffuse(LightDir, input.worldNormal, DiffuseMaterial, DiffuseLight);
    
    float3 AmbientOut = PhongAmbient(AmbientLight, AmbientMaterial);
    
    float3 SpecularOut = PhongSpecular(LightDir, CameraPosition, input.worldNormal, input.worldPosition, SpecularLight, SpecularMaterial, SpecularPower);

    float4 Texture = diffuseTex.Sample(bilinearSampler, input.texCoords);
    float4 TextOut = Texture;
    
    
    
    //Switch Out colors based on Effect Toggle in Constbuffer
    switch (EffectToggle)
    {
        case 1:                                                                                     //viewmat swizzle
            input.color = applyFog(TextOut + DiffuseOut.rgbb + float4(SpecularOut, 1), abs(length(View._41_42_43 - input.worldPosition)));
            break;
        case 0:
            input.color = TextOut + DiffuseOut.rgbb + float4(SpecularOut, 1);
            break;
        case 2:
            input.color = float4(GoochOut /*+ SpecularOut*/, 1);
            break;
        case 3:
            input.color = float4(input.worldNormal.rgb, 1);
            break;
        default:
            input.color = float4(AmbientOut + DiffuseOut + SpecularOut, 1);
            break;
    }
    
    return input.color;
}