// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Bloom Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
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
	Texture = <DiffuseA>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler1 = sampler_state
{
	Texture = <DiffuseB>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler2 = sampler_state
{
	Texture = <DiffuseC>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler3 = sampler_state
{
	Texture = <DiffuseD>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler4 = sampler_state
{
	Texture = <DiffuseE>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler5 = sampler_state
{
	Texture = <DiffuseF>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler6 = sampler_state
{
	Texture = <DiffuseG>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler7 = sampler_state
{
	Texture = <DiffuseH>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler8 = sampler_state
{
	Texture = <DiffuseI>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSampler9 = sampler_state
{
	Texture = <DiffuseJ>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler DiffuseSamplerA = sampler_state
{
	Texture = <DiffuseK>;
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
	float2 TexCoord0 : TEXCOORD0;
	float4 TexCoord12 : TEXCOORD1;
	float4 TexCoord34 : TEXCOORD2;
	float4 TexCoord56 : TEXCOORD3;
	float4 TexCoord78 : TEXCOORD4;
	float4 TexCoord9A : TEXCOORD5;
};

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord0 : TEXCOORD0;
	float4 TexCoord12 : TEXCOORD1;
	float4 TexCoord34 : TEXCOORD2;
	float4 TexCoord56 : TEXCOORD3;
	float4 TexCoord78 : TEXCOORD4;
	float4 TexCoord9A : TEXCOORD5;
};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float E = 2.7182817f;
static const float Gelfond = 23.140692f; // e^pi (Gelfond constant)
static const float GelfondSchneider = 2.6651442f; // 2^sqrt(2) (Gelfond-Schneider constant)

//-----------------------------------------------------------------------------
// Funcions
//-----------------------------------------------------------------------------

// www.stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader/
float random(float2 seed)
{
	// irrationals for pseudo randomness
	float2 i = float2(Gelfond, GelfondSchneider);

	return frac(cos(dot(seed, i)) * 123456.0f);
}

//-----------------------------------------------------------------------------
// Bloom Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 TargetDims;
uniform float2 SourceRect;

uniform float2 Level0Size;
uniform float4 Level12Size;
uniform float4 Level34Size;
uniform float4 Level56Size;
uniform float4 Level78Size;
uniform float4 Level9ASize;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.Color = Input.Color;

	float2 TexCoord = Input.Position.xy / ScreenDims;
	TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.TexCoord0 = TexCoord;
	Output.TexCoord12 = TexCoord.xyxy + (0.5f / Level12Size);
	Output.TexCoord34 = TexCoord.xyxy + (0.5f / Level34Size);
	Output.TexCoord56 = TexCoord.xyxy + (0.5f / Level56Size);
	Output.TexCoord78 = TexCoord.xyxy + (0.5f / Level78Size);
	Output.TexCoord9A = TexCoord.xyxy + (0.5f / Level9ASize);

	return Output;
}

//-----------------------------------------------------------------------------
// Bloom Pixel Shader
//-----------------------------------------------------------------------------

uniform float Level0Weight;
uniform float2 Level12Weight;
uniform float2 Level34Weight;
uniform float2 Level56Weight;
uniform float2 Level78Weight;
uniform float2 Level9AWeight;

uniform int BloomBlendMode = 0; // 0 addition, 1 darken
uniform float BloomScale;
uniform float3 BloomOverdrive;

float3 GetNoiseFactor(float3 n, float random)
{
	// smaller n become more noisy
	return 1.0f + random * max(0.0f, 0.25f * pow(E, -8 * n));
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float3 texel0 = tex2D(DiffuseSampler0, Input.TexCoord0).rgb;
	float3 texel1 = tex2D(DiffuseSampler1, Input.TexCoord12.xy).rgb;
	float3 texel2 = tex2D(DiffuseSampler2, Input.TexCoord12.zw).rgb;
	float3 texel3 = tex2D(DiffuseSampler3, Input.TexCoord34.xy).rgb;
	float3 texel4 = tex2D(DiffuseSampler4, Input.TexCoord34.zw).rgb;
	float3 texel5 = tex2D(DiffuseSampler5, Input.TexCoord56.xy).rgb;
	float3 texel6 = tex2D(DiffuseSampler6, Input.TexCoord56.zw).rgb;
	float3 texel7 = tex2D(DiffuseSampler7, Input.TexCoord78.xy).rgb;
	float3 texel8 = tex2D(DiffuseSampler8, Input.TexCoord78.zw).rgb;
	float3 texel9 = tex2D(DiffuseSampler9, Input.TexCoord9A.xy).rgb;
	float3 texelA = tex2D(DiffuseSamplerA, Input.TexCoord9A.zw).rgb;

	float3 blend;

	// addition
	if (BloomBlendMode == 0)
	{
		texel0 *= Level0Weight;
		texel1 *= Level12Weight.x;
		texel2 *= Level12Weight.y;
		texel3 *= Level34Weight.x;
		texel4 *= Level34Weight.y;
		texel5 *= Level56Weight.x;
		texel6 *= Level56Weight.y;
		texel7 *= Level78Weight.x;
		texel8 *= Level78Weight.y;
		texel9 *= Level9AWeight.x;
		texelA *= Level9AWeight.y;

		float3 bloom = float3(
			texel1 +
			texel2 +
			texel3 +
			texel4 +
			texel5 +
			texel6 +
			texel7 +
			texel8 +
			texel9 +
			texelA) * BloomScale;

		float3 bloomOverdrive = max(0.0f, texel0 + bloom - 1.0f) * BloomOverdrive;

		bloom.r += bloomOverdrive.g * 0.5f;
		bloom.r += bloomOverdrive.b * 0.5f;
		bloom.g += bloomOverdrive.r * 0.5f;
		bloom.g += bloomOverdrive.b * 0.5f;
		bloom.b += bloomOverdrive.r * 0.5f;
		bloom.b += bloomOverdrive.g * 0.5f;

		float2 NoiseCoord = Input.TexCoord0;
		float3 NoiseFactor = GetNoiseFactor(bloom, random(NoiseCoord));

		blend = texel0 + bloom * NoiseFactor;
	}

	// darken
	else
	{
		texel1 = min(texel0, texel1);
		texel2 = min(texel0, texel2);
		texel3 = min(texel0, texel3);
		texel4 = min(texel0, texel4);
		texel5 = min(texel0, texel5);
		texel6 = min(texel0, texel6);
		texel7 = min(texel0, texel7);
		texel8 = min(texel0, texel8);
		texel9 = min(texel0, texel9);
		texelA = min(texel0, texelA);

		blend = texel0 * Level0Weight;
		blend = lerp(blend, texel1, Level12Weight.x * BloomScale);
		blend = lerp(blend, texel2, Level12Weight.y * BloomScale);
		blend = lerp(blend, texel3, Level34Weight.x * BloomScale);
		blend = lerp(blend, texel4, Level34Weight.y * BloomScale);
		blend = lerp(blend, texel5, Level56Weight.x * BloomScale);
		blend = lerp(blend, texel6, Level56Weight.y * BloomScale);
		blend = lerp(blend, texel7, Level78Weight.x * BloomScale);
		blend = lerp(blend, texel8, Level78Weight.y * BloomScale);
		blend = lerp(blend, texel9, Level9AWeight.x * BloomScale);
		blend = lerp(blend, texelA, Level9AWeight.y * BloomScale);
	}

	return float4(blend, 1.0f);
}

//-----------------------------------------------------------------------------
// Bloom Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader = compile ps_3_0 ps_main();
	}
}
