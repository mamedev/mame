// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline & Shadowmask Effect
//-----------------------------------------------------------------------------

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
	float2 SourceCoord : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float2 ScreenCoord : TEXCOORD2;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 SourceCoord : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float2 ScreenCoord : TEXCOORD2;
};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float PI = 3.1415927f;
static const float PHI = 1.618034f;

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 TargetDims;
uniform float2 QuadDims;

uniform float2 ShadowDims = float2(32.0f, 32.0f); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0f, 0.0f);

uniform bool SwapXY = false;

uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures
uniform bool VectorScreen = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += PrepareBloom
		? 0.0f / TargetDims  // use half texel offset (DX9) to do the blur for first bloom layer
		: 0.5f / TargetDims; // fix half texel offset correction (DX9)

	Output.ScreenCoord = Input.Position.xy / ScreenDims;

	Output.SourceCoord = Input.TexCoord;
	Output.SourceCoord += 0.5f / TargetDims;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

uniform float HumBarDesync = 60.0f / 59.94f - 1.0f; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
uniform float HumBarAlpha = 0.0f;

uniform float TimeMilliseconds = 0.0f;

uniform float2 ScreenScale = float2(1.0f, 1.0f);
uniform float2 ScreenOffset = float2(0.0f, 0.0f);

uniform float ScanlineAlpha = 0.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineHeight = 1.0f;
uniform float ScanlineVariation = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;

uniform float3 BackColor = float3(0.0f, 0.0f, 0.0f);

uniform int ShadowTileMode = 0; // 0 based on screen (quad) dimension, 1 based on source dimension
uniform float ShadowAlpha = 0.0f;
uniform float2 ShadowCount = float2(6.0f, 6.0f);
uniform float2 ShadowUV = float2(0.25f, 0.25f);

uniform float3 Power = float3(1.0f, 1.0f, 1.0f);
uniform float3 Floor = float3(0.0f, 0.0f, 0.0f);

float2 GetAdjustedCoords(float2 coord, float2 centerOffset)
{
	// center coordinates
	coord -= centerOffset;

	// apply screen scale
	coord /= ScreenScale;

	// un-center coordinates
	coord += centerOffset;

	// apply screen offset
	coord += (centerOffset * 2.0) * ScreenOffset;

	return coord;
}

// vector screen has the same quad texture coordinates for every screen orientation, raster screen differs
float2 GetShadowCoord(float2 QuadCoord, float2 SourceCoord)
{
	float2 QuadTexel = 1.0f / QuadDims;
	float2 SourceTexel = 1.0f / SourceDims;

	float2 canvasCoord = ShadowTileMode == 0
		? QuadCoord + ShadowUVOffset / QuadDims
		: SourceCoord + ShadowUVOffset / SourceDims;
	float2 canvasTexelDims = ShadowTileMode == 0
		? QuadTexel
		: SourceTexel;

	float2 shadowDims = ShadowDims;
	float2 shadowUV = ShadowUV;
	float2 shadowCount = ShadowCount;

	// swap x/y vector and raster in screen mode (not source mode)
	canvasCoord = ShadowTileMode == 0 && SwapXY
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y vector and raster in screen mode (not source mode)
	shadowCount = ShadowTileMode == 0 && SwapXY
		? shadowCount.yx
		: shadowCount.xy;

	float2 shadowTile = canvasTexelDims * shadowCount;

	// swap x/y vector in screen mode (not raster and not source mode)
	shadowTile = VectorScreen && ShadowTileMode == 0 && SwapXY
		? shadowTile.yx
		: shadowTile.xy;

	float2 shadowFrac = frac(canvasCoord / shadowTile);

	// swap x/y raster in screen mode (not vector and not source mode)
	shadowFrac = !VectorScreen && ShadowTileMode == 0 && SwapXY
		? shadowFrac.yx
		: shadowFrac.xy;

	float2 shadowCoord = (shadowFrac * shadowUV);
	shadowCoord += 0.5f / shadowDims; // half texel offset

	return shadowCoord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenCoord = Input.ScreenCoord;
	float2 TexCoord = GetAdjustedCoords(Input.TexCoord, 0.5f);
	float2 SourceCoord = GetAdjustedCoords(Input.SourceCoord, 0.5f);

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, TexCoord);
	BaseColor.a = 1.0f;

	// keep border
	if (!PrepareBloom)
	{
		// clip border
		clip(TexCoord < 0.0f || TexCoord > 1.0f ? -1 : 1);
	}

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom && ShadowAlpha > 0.0f)
	{
		float2 ShadowCoord = GetShadowCoord(ScreenCoord, SourceCoord);

		float4 ShadowColor = tex2D(ShadowSampler, ShadowCoord);
		float3 ShadowMaskColor = lerp(1.0f, ShadowColor.rgb, ShadowAlpha);
		float ShadowMaskClear = (1.0f - ShadowColor.a) * ShadowAlpha;

		// apply shadow mask color
		BaseColor.rgb *= ShadowMaskColor;
		// clear shadow mask by background color
		BaseColor.rgb = lerp(BaseColor.rgb, BackColor, ShadowMaskClear);
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

	// Scanline Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		// Scanline Simulation (may not affect vector screen)
		if (!VectorScreen && ScanlineAlpha > 0.0f)
		{
			float BrightnessOffset = (ScanlineBrightOffset * ScanlineAlpha);
			float BrightnessScale = (ScanlineBrightScale * ScanlineAlpha) + (1.0f - ScanlineAlpha);

			float ColorBrightness = 0.299f * BaseColor.r + 0.587f * BaseColor.g + 0.114 * BaseColor.b;

			float ScanlineCoord = SourceCoord.y * SourceDims.y * ScanlineScale * PI;
			float ScanlineCoordJitter = ScanlineOffset * PHI;
			float ScanlineSine = sin(ScanlineCoord + ScanlineCoordJitter);
			float ScanlineWide = ScanlineHeight + ScanlineVariation * max(1.0f, ScanlineHeight) * (1.0f - ColorBrightness);
			float ScanlineAmount = pow(ScanlineSine * ScanlineSine, ScanlineWide);
			float ScanlineBrightness = ScanlineAmount * BrightnessScale + BrightnessOffset * BrightnessScale;

			BaseColor.rgb *= lerp(1.0f, ScanlineBrightness, ScanlineAlpha);
		}

		// Hum Bar Simulation (may not affect vector screen)
		if (!VectorScreen && HumBarAlpha > 0.0f)
		{
			float HumBarStep = frac(TimeMilliseconds * HumBarDesync);
			float HumBarBrightness = 1.0 - frac(SourceCoord.y + HumBarStep) * HumBarAlpha;
			BaseColor.rgb *= HumBarBrightness;
		}
	}

	return BaseColor;
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Technique
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