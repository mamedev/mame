// license:BSD-3-Clause
// copyright-holders:W. M. Martinez
//-----------------------------------------------------------------------------
// Phosphor Chromaticity to sRGB Transform Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture   = <Diffuse>;
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
	float2 PrevCoord : TEXCOORD1;
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
	float2 PrevCoord : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Chroma Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;

uniform bool Passthrough;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0.0;

	Output.Position = float4(Input.Position.xyz, 1.0);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0 - Output.Position.y; // flip y
	Output.Position.xy -= 0.5; // center
	Output.Position.xy *= 2.0; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5 / TargetDims; // half texel offset correction (DX9)

	Output.PrevCoord = Output.TexCoord;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Chroma Pixel Shader
//-----------------------------------------------------------------------------

uniform float3 YGain = float3(0.2126, 0.7152, 0.0722);
uniform float2 ChromaA = float2(0.630, 0.340);
uniform float2 ChromaB = float2(0.310, 0.595);
uniform float2 ChromaC = float2(0.155, 0.070);

static const float3x3 XYZ_TO_sRGB = {
	 3.2406, -1.5372, -0.4986,
        -0.9689,  1.8758,  0.0415,
	 0.0557, -0.2040,  1.0570
};

float4 ps_main(PS_INPUT Input) : COLOR
{
	const float4 cin = tex2D(DiffuseSampler, Input.TexCoord);
	float4 cout = float4(0.0, 0.0, 0.0, cin.a);
	const float3x2 xy = { ChromaA, ChromaB, ChromaC };

	for (int i = 0; i < 3; ++i) {
		const float Y = YGain[i] * cin[i];
		const float X = xy[i].x * (Y / xy[i].y);
		const float Z = (1.0 - xy[i].x - xy[i].y) * (Y / xy[i].y);
		cout.rgb += mul(XYZ_TO_sRGB, float3(X, Y, Z));
	}
	return cout;
}

//-----------------------------------------------------------------------------
// Phosphor Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
