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

//Simple sin wave generation function
float generateWave(float xPos, float speed, float length, float height)
{
    float freq = 2.0f / length;
    float phase = Count * speed;
    
    return sin((xPos + phase) * freq) * height;
    
}

//Vertex
VS_Out VS_main(float3 Position : POSITION, float3 Normal : NORMAL, float2 TexCoords : TEXCOORD)
{
    VS_Out output = (VS_Out) 0;
    
    float4 Pos4 = float4(Position, 1.0f);
    
    //To World
    output.position = mul(Pos4, World);
    output.worldNormal = mul(normalize(Normal), (float3x3) World);
    
    //Y Position is offset by the wave which are combined using the sum of sines
    output.position.y = (generateWave(output.position.x, 4.0f, 10.0f, 2.0f) +
                         generateWave(output.position.x, 8.0f, 5.0f, 0.5f) +
                         generateWave(output.position.x, 15.0f, 15.0f, 0.25f) +
                         generateWave(output.position.x, 1.0f, 15.0f, 0.5f) + 
                         generateWave(output.position.z, 4.0f, 10.0f, 2.0f));
    
    output.worldPosition = output.position.xyz;
    //To View
    output.position = mul(output.position, View);
    //To Proj
    output.position = mul(output.position, Projection);
    
  
    
    
    output.texCoords = TexCoords;
   
    return output;
}

float3 calculateNormal(float3 posA, float3 posB, float3 posC)
{
    //Simple Normal Calculation
    //cross(v0 - v1, v0 - v2)
    float3 e1 = posA - posB;
    float3 e2 = posA - posC;
    return cross(e1, e2);
}

//Geometry Shaders for normal calculation (No Calculus Required)
[maxvertexcount(3)]//Grab triangle primitives
void GS_main(triangle VS_Out input[3] : SV_POSITION, inout TriangleStream<VS_Out> OutputStream)
{
    
    
    float3 norm = calculateNormal(input[0].worldPosition, input[1].worldPosition, input[2].worldPosition);
    
    //Construct output struct
    VS_Out output[3] = { (VS_Out) 0, (VS_Out) 0, (VS_Out) 0 };
    output[0] = input[0];
    output[1] = input[1];
    output[2] = input[2];
    
    //Throw Normals in output array
    output[0].worldNormal = norm;
    output[1].worldNormal = norm;
    output[2].worldNormal = norm;
    
    for (int j = 0; j < 3; j++)
    {   
        //Unsure whether to prenormalize or not
        output[j].worldNormal = normalize(output[j].worldNormal);
        //output[j].position += (sin(output[j].worldPosition.x) * 10) * output[j].worldNormal.xyzz;
        OutputStream.Append(output[j]); 
    }
}