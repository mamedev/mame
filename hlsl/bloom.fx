//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

texture DiffuseA;
texture DiffuseB;
texture DiffuseC;
texture DiffuseD;
texture DiffuseE;
texture DiffuseF;
texture DiffuseG;
texture DiffuseH;
texture DiffuseI;
texture DiffuseJ;
texture DiffuseK;

sampler DiffuseSampler0 = sampler_state
{
	Texture   = <DiffuseA>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler1 = sampler_state
{
	Texture   = <DiffuseB>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler2 = sampler_state
{
	Texture   = <DiffuseC>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler3 = sampler_state
{
	Texture   = <DiffuseD>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler4 = sampler_state
{
	Texture   = <DiffuseE>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler5 = sampler_state
{
	Texture   = <DiffuseF>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler6 = sampler_state
{
	Texture   = <DiffuseG>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler7 = sampler_state
{
	Texture   = <DiffuseH>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler8 = sampler_state
{
	Texture   = <DiffuseI>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler9 = sampler_state
{
	Texture   = <DiffuseJ>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler10 = sampler_state
{
	Texture   = <DiffuseK>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Bloom Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 TargetSize;

uniform float DiffuseScaleA;
uniform float DiffuseScaleB;
uniform float DiffuseScaleC;
uniform float DiffuseScaleD;
uniform float DiffuseScaleE;
uniform float DiffuseScaleF;
uniform float DiffuseScaleG;
uniform float DiffuseScaleH;
uniform float DiffuseScaleI;
uniform float DiffuseScaleJ;
uniform float DiffuseScaleK;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= TargetSize;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= float2(0.5f, 0.5f);
	Output.Position.xy *= float2(2.0f, 2.0f);
	Output.Color = Input.Color;
	float2 inversePixel = 1.0f / TargetSize;
	Output.TexCoord = Input.Position.xy * inversePixel;

	return Output;
}

//-----------------------------------------------------------------------------
// Bloom Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float3 texel0 = tex2D(DiffuseSampler0, Input.TexCoord).rgb * DiffuseScaleA * 1.00f;
	float3 texel1 = tex2D(DiffuseSampler1, Input.TexCoord).rgb * DiffuseScaleB * 0.95f;
	float3 texel2 = tex2D(DiffuseSampler2, Input.TexCoord).rgb * DiffuseScaleC * 0.85f;
	float3 texel3 = tex2D(DiffuseSampler3, Input.TexCoord).rgb * DiffuseScaleD * 0.75f;
	float3 texel4 = tex2D(DiffuseSampler4, Input.TexCoord).rgb * DiffuseScaleE * 0.65f;
	float3 texel5 = tex2D(DiffuseSampler5, Input.TexCoord).rgb * DiffuseScaleF * 0.55f;
	float3 texel6 = tex2D(DiffuseSampler6, Input.TexCoord).rgb * DiffuseScaleG * 0.45f;
	float3 texel7 = tex2D(DiffuseSampler7, Input.TexCoord).rgb * DiffuseScaleH * 0.35f;
	float3 texel8 = tex2D(DiffuseSampler8, Input.TexCoord).rgb * DiffuseScaleI * 0.25f;
	float3 texel9 = tex2D(DiffuseSampler9, Input.TexCoord).rgb * DiffuseScaleJ * 0.15f;
	float3 texel10 = tex2D(DiffuseSampler10, Input.TexCoord).rgb * DiffuseScaleK * 0.10f;
	return float4(texel0 + texel1 + texel2 + texel3 + texel4 +
	        texel5 + texel6 + texel7 + texel8 + texel9 + texel10, 1.0f);
}

//-----------------------------------------------------------------------------
// Downsample Effect
//-----------------------------------------------------------------------------

technique TestTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = <DiffuseSampler0>; // 2048x2048
		Sampler[1] = <DiffuseSampler1>; // 1024x1024
		Sampler[2] = <DiffuseSampler2>; // 512x512
		Sampler[3] = <DiffuseSampler3>; // 256x256
		Sampler[4] = <DiffuseSampler4>; // 128x128
		Sampler[5] = <DiffuseSampler5>; // 64x64
		Sampler[6] = <DiffuseSampler6>; // 32x32
		Sampler[7] = <DiffuseSampler7>; // 16x16
		Sampler[8] = <DiffuseSampler8>; // 8x8
		Sampler[9] = <DiffuseSampler9>; // 4x4
		Sampler[10] = <DiffuseSampler10>; // 2x2

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
