// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Shadowmask Effect
//-----------------------------------------------------------------------------

#define MONOCHROME 1
#define DICHROME 2
#define TRICHROME 3

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture DiffuseTexture;

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

texture ShadowTexture;

sampler ShadowSampler = sampler_state
{
	Texture = <ShadowTexture>;
	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_INPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 ScreenCoord : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 ScreenCoord : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float PI = 3.1415927;
static const float HalfPI = PI * 0.5;

//-----------------------------------------------------------------------------
// Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 TargetDims;
uniform float2 TargetScale;
uniform float2 QuadDims;

uniform float2 ShadowDims = float2(32.0, 32.0); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0, 0.0);

uniform bool SwapXY = false;

uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures
uniform bool VectorScreen = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0 - Output.Position.y; // flip y
	Output.Position.xy -= 0.5; // center
	Output.Position.xy *= 2.0; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += PrepareBloom
		? 0.0               // use half texel offset (DX9) to do the blur for first bloom layer
		: 0.5 / TargetDims; // fix half texel offset (DX9)

	Output.ScreenCoord = Input.Position.xy / ScreenDims;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

uniform float HumBarDesync = 60.0 / 59.94 - 1.0; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
uniform float HumBarAlpha = 0.0;

uniform float TimeMilliseconds = 0.0;

uniform float2 ScreenScale = float2(1.0, 1.0);
uniform float2 ScreenOffset = float2(0.0, 0.0);

uniform float3 BackColor = float3(0.0, 0.0, 0.0);

uniform int ShadowTileMode = 0; // 0 based on screen (quad) dimension, 1 based on source dimension
uniform float ShadowAlpha = 0.0;
uniform float2 ShadowCount = float2(6.0, 6.0);
uniform float2 ShadowUV = float2(0.25, 0.25);

uniform float3 Power = float3(1.0, 1.0, 1.0);
uniform float3 Floor = float3(0.0, 0.0, 0.0);

uniform int ChromaMode = 3;
uniform float3 ConversionGain = float3(0.0, 0.0, 0.0);

float2 GetAdjustedCoords(float2 coord)
{
	// center coordinates
	coord -= 0.5;

	// apply screen scale
	coord *= ScreenScale;

	// un-center coordinates
	coord += 0.5;

	// apply screen offset
	coord += ScreenOffset;

	return coord;
}

float2 GetShadowCoord(float2 TargetCoord, float2 SourceCoord)
{
	// base-target dimensions (without oversampling)
	float2 BaseTargetDims = TargetDims / TargetScale;
	BaseTargetDims = SwapXY
		? BaseTargetDims.yx
		: BaseTargetDims.xy;

	float2 canvasCoord = ShadowTileMode == 0
		? TargetCoord + ShadowUVOffset / BaseTargetDims
		: SourceCoord + ShadowUVOffset / SourceDims;
	float2 canvasTexelDims = ShadowTileMode == 0
		? 1.0 / BaseTargetDims
		: 1.0 / SourceDims;

	float2 shadowDims = ShadowDims;
	float2 shadowUV = ShadowUV;
	float2 shadowCount = ShadowCount;

	// swap x/y in screen mode (not source mode)
	canvasCoord = ShadowTileMode == 0 && SwapXY
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y in screen mode (not source mode)
	shadowCount = ShadowTileMode == 0 && SwapXY
		? shadowCount.yx
		: shadowCount.xy;

	float2 shadowTile = canvasTexelDims * shadowCount;

	float2 shadowFrac = frac(canvasCoord / shadowTile);

	// swap x/y in screen mode (not source mode)
	shadowFrac = ShadowTileMode == 0 && SwapXY
		? shadowFrac.yx
		: shadowFrac.xy;

	float2 shadowCoord = (shadowFrac * shadowUV);
	shadowCoord += ShadowTileMode == 0
		? 0.5 / shadowDims // fix half texel offset (DX9)
		: 0.0;

	return shadowCoord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenCoord = Input.ScreenCoord;
	float2 BaseCoord = GetAdjustedCoords(Input.TexCoord);

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0;

	// clip border
	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0 ||
		BaseCoord.x > 1.0 || BaseCoord.y > 1.0)
	{
		// we don't use the clip function, because we don't clear the render target before
		return float4(0.0, 0.0, 0.0, 1.0);
	}

	// Color Compression (may not affect bloom)
	if (!PrepareBloom)
	{
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = Floor + (1.0f - Floor) * BaseColor.rgb;
	}

	// Color Power (may affect bloom)
	BaseColor.r = pow(BaseColor.r, Power.r);
	BaseColor.g = pow(BaseColor.g, Power.g);
	BaseColor.b = pow(BaseColor.b, Power.b);

	// Hum Bar Simulation (may not affect vector screen)
	if (!PrepareBloom && !VectorScreen && HumBarAlpha > 0.0)
	{
		float HumBarStep = frac(TimeMilliseconds * HumBarDesync);
		float HumBarBrightness = 1.0 - frac(BaseCoord.y + HumBarStep) * HumBarAlpha;
		BaseColor.rgb *= HumBarBrightness;
	}

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom && ShadowAlpha > 0.0)
	{
		float2 ShadowCoord = GetShadowCoord(ScreenCoord, BaseCoord);

		float4 ShadowColor = tex2D(ShadowSampler, ShadowCoord);
		float3 ShadowMaskColor = lerp(1.0, ShadowColor.rgb, ShadowAlpha);
		float ShadowMaskClear = (1.0 - ShadowColor.a) * ShadowAlpha;

		// apply shadow mask color
		BaseColor.rgb *= ShadowMaskColor;
		// clear shadow mask by background color
		BaseColor.rgb = lerp(BaseColor.rgb, BackColor, ShadowMaskClear);
	}

	// Preparation for phosphor color conversion
	if (ChromaMode == MONOCHROME) {
		BaseColor.r = dot(ConversionGain, BaseColor.rgb);
		BaseColor.gb = float2(BaseColor.r, BaseColor.r);
	} else if (ChromaMode == DICHROME) {
		BaseColor.r = dot(ConversionGain.rg, BaseColor.rg);
		BaseColor.g = BaseColor.r;
	}

	return BaseColor;
}

//-----------------------------------------------------------------------------
// Shadowmask Technique
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
