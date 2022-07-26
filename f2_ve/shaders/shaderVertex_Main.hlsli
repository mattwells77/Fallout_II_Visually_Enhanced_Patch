#pragma pack_matrix( row_major )
cbuffer ModelViewProjectionConstantBuffer  : register(b0)
{
	float4x4 WorldMatrix;
	float4x4 WorldViewProjectionMatrix;
}



struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoords : TEXCOORD0;
	float3 Normal : NORMAL0;
};


struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TexCoords : TEXCOORD0;
	float3 Normal : NORMAL0;
	float3 WorldPos : POSITION0;
};




VS_OUTPUT VS_Basic(VS_INPUT IN)
{
	VS_OUTPUT OUT;
	//float4 pos = float4(IN.Position.xyz, 1.0f); 
	IN.Position.w = 1.0f;
	//IN.Position.z = 0.0f;
	OUT.Position = mul(IN.Position, WorldViewProjectionMatrix);
	OUT.TexCoords = IN.TexCoords;
	OUT.Normal = mul(IN.Normal, (float3x3)WorldMatrix);
	OUT.WorldPos = mul(IN.Position, WorldMatrix).xyz;

	//OUT.Position = mul(IN.Position, WorldMatrix);
	//OUT.Position = mul(OUT.Position, viewMatrix);
	//OUT.Position = mul(OUT.Position, WorldViewProjectionMatrix);
	//OUT.Position = IN.Position;
	return OUT;
}

