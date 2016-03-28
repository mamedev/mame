// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Bloom Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture DiffuseTexture;

texture BloomTextureA;
texture BloomTextureB;
texture BloomTextureC;
texture BloomTextureD;
texture BloomTextureE;
texture BloomTextureF;
texture BloomTextureG;
texture BloomTextureH;

// vector screen uses twice -1 as many bloom levels
texture BloomTextureI;
texture BloomTextureJ;
texture BloomTextureK;
texture BloomTextureL;
texture BloomTextureM;
texture BloomTextureN;
texture BloomTextureO;

sampler DiffuseSampler = sampler_state
{
	Texture = <DiffuseTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerA = sampler_state
{
	Texture = <BloomTextureA>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerB = sampler_state
{
	Texture = <BloomTextureB>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerC = sampler_state
{
	Texture = <BloomTextureC>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerD = sampler_state
{
	Texture = <BloomTextureD>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerE = sampler_state
{
	Texture = <BloomTextureE>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerF = sampler_state
{
	Texture = <BloomTextureF>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerG = sampler_state
{
	Texture = <BloomTextureG>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerH = sampler_state
{
	Texture = <BloomTextureH>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerI = sampler_state
{
	Texture = <BloomTextureI>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerJ = sampler_state
{
	Texture = <BloomTextureJ>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerK = sampler_state
{
	Texture = <BloomTextureK>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerL = sampler_state
{
	Texture = <BloomTextureL>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerM = sampler_state
{
	Texture = <BloomTextureM>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerN = sampler_state
{
	Texture = <BloomTextureN>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

sampler BloomSamplerO = sampler_state
{
	Texture = <BloomTextureO>;
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
	float2 BloomCoord : TEXCOORD1;
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
	float2 TexCoord : TEXCOORD0;
	float2 BloomCoord : TEXCOORD1;
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
uniform float2 SourceDims;

uniform bool VectorScreen = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.Color = Input.Color;

	float2 TexCoord = Input.TexCoord;
	TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.TexCoord = TexCoord.xy;

	TexCoord += VectorScreen
		? 0.5f / TargetDims.xy
		: 0.5f / SourceDims.xy;

	Output.BloomCoord = TexCoord.xy;

	return Output;
}

//-----------------------------------------------------------------------------
// Bloom Pixel Shader
//-----------------------------------------------------------------------------

uniform float Level0Weight;
uniform float Level1Weight;
uniform float Level2Weight;
uniform float Level3Weight;
uniform float Level4Weight;
uniform float Level5Weight;
uniform float Level6Weight;
uniform float Level7Weight;
uniform float Level8Weight;

uniform int BloomBlendMode = 0; // 0 brighten, 1 darken
uniform float BloomScale;
uniform float3 BloomOverdrive;

float3 GetNoiseFactor(float3 n, float random)
{
	// smaller n become more noisy
	return 1.0f + random * max(0.0f, 0.25f * pow(E, -8 * n));
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float3 texel = tex2D(DiffuseSampler, Input.TexCoord).rgb;

	float3 texelA = tex2D(BloomSamplerA, Input.BloomCoord.xy).rgb;
	float3 texelB = tex2D(BloomSamplerB, Input.BloomCoord.xy).rgb;
	float3 texelC = tex2D(BloomSamplerC, Input.BloomCoord.xy).rgb;
	float3 texelD = tex2D(BloomSamplerD, Input.BloomCoord.xy).rgb;
	float3 texelE = tex2D(BloomSamplerE, Input.BloomCoord.xy).rgb;
	float3 texelF = tex2D(BloomSamplerF, Input.BloomCoord.xy).rgb;
	float3 texelG = tex2D(BloomSamplerG, Input.BloomCoord.xy).rgb;
	float3 texelH = tex2D(BloomSamplerH, Input.BloomCoord.xy).rgb;

	float3 texelI = float3(0.0f, 0.0f, 0.0f);
	float3 texelJ = float3(0.0f, 0.0f, 0.0f);
	float3 texelK = float3(0.0f, 0.0f, 0.0f);
	float3 texelL = float3(0.0f, 0.0f, 0.0f);
	float3 texelM = float3(0.0f, 0.0f, 0.0f);
	float3 texelN = float3(0.0f, 0.0f, 0.0f);
	float3 texelO = float3(0.0f, 0.0f, 0.0f);

	// vector screen uses twice -1 as many bloom levels
	if (VectorScreen)
	{
		texelI = tex2D(BloomSamplerI, Input.BloomCoord.xy).rgb;
		texelJ = tex2D(BloomSamplerJ, Input.BloomCoord.xy).rgb;
		texelK = tex2D(BloomSamplerK, Input.BloomCoord.xy).rgb;
		texelL = tex2D(BloomSamplerL, Input.BloomCoord.xy).rgb;
		texelM = tex2D(BloomSamplerM, Input.BloomCoord.xy).rgb;
		texelN = tex2D(BloomSamplerN, Input.BloomCoord.xy).rgb;
		texelO = tex2D(BloomSamplerO, Input.BloomCoord.xy).rgb;
	}

	float3 blend;

	// brighten
	if (BloomBlendMode == 0)
	{
		float3 bloom = float3(0.0f, 0.0f, 0.0f);

		texel *= Level0Weight;

		if (!VectorScreen)
		{
			bloom += texelA * Level1Weight;
			bloom += texelB * Level2Weight;
			bloom += texelC * Level3Weight;
			bloom += texelD * Level4Weight;
			bloom += texelE * Level5Weight;
			bloom += texelF * Level6Weight;
			bloom += texelG * Level7Weight;
			bloom += texelH * Level8Weight;
		}
		// vector screen uses twice -1 as many bloom levels
		else
		{
			bloom += texelA * (Level1Weight);
			bloom += texelB * (Level1Weight + Level2Weight) * 0.5f;
			bloom += texelC * (Level2Weight);
			bloom += texelD * (Level2Weight + Level3Weight) * 0.5f;
			bloom += texelE * (Level3Weight);
			bloom += texelF * (Level3Weight + Level4Weight) * 0.5f;
			bloom += texelG * (Level4Weight);
			bloom += texelH * (Level4Weight + Level5Weight) * 0.5f;
			bloom += texelI * (Level5Weight);
			bloom += texelJ * (Level5Weight + Level6Weight) * 0.5f;
			bloom += texelK * (Level6Weight);
			bloom += texelL * (Level6Weight + Level7Weight) * 0.5f;
			bloom += texelM * (Level7Weight);
			bloom += texelN * (Level7Weight + Level8Weight) * 0.5f;
			bloom += texelO * (Level8Weight);
		}

		bloom *= BloomScale;

		float3 bloomOverdrive = max(0.0f, texel + bloom - 1.0f) * BloomOverdrive;

		bloom.r += bloomOverdrive.g * 0.5f;
		bloom.r += bloomOverdrive.b * 0.5f;
		bloom.g += bloomOverdrive.r * 0.5f;
		bloom.g += bloomOverdrive.b * 0.5f;
		bloom.b += bloomOverdrive.r * 0.5f;
		bloom.b += bloomOverdrive.g * 0.5f;

		float2 NoiseCoord = Input.TexCoord;
		float3 NoiseFactor = GetNoiseFactor(bloom, random(NoiseCoord));

		blend = texel + bloom * NoiseFactor;
	}

	// darken
	else
	{
		texelA = min(texel, texelA);
		texelB = min(texel, texelB);
		texelC = min(texel, texelC);
		texelD = min(texel, texelD);
		texelE = min(texel, texelE);
		texelF = min(texel, texelF);
		texelG = min(texel, texelG);
		texelH = min(texel, texelH);

		blend = texel * Level0Weight;
		blend = lerp(blend, texelA, Level1Weight * BloomScale);
		blend = lerp(blend, texelB, Level2Weight * BloomScale);
		blend = lerp(blend, texelC, Level3Weight * BloomScale);
		blend = lerp(blend, texelD, Level4Weight * BloomScale);
		blend = lerp(blend, texelE, Level5Weight * BloomScale);
		blend = lerp(blend, texelF, Level6Weight * BloomScale);
		blend = lerp(blend, texelG, Level7Weight * BloomScale);
		blend = lerp(blend, texelH, Level8Weight * BloomScale);
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
