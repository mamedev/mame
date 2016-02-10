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

static const float PI = 3.1415927f;
static const float PHI = 1.618034f;

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims; // size of the window or fullscreen
uniform float2 SourceDims; // size of the texture in power-of-two size
uniform float2 SourceRect; // size of the uv rectangle
uniform float2 TargetDims; // size of the target surface

uniform float2 ShadowDims = float2(32.0f, 32.0f); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0f, 0.0f);

uniform bool SwapXY = false;

uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures
uniform bool PrepareVector = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float2 shadowUVOffset = ShadowUVOffset;
	shadowUVOffset = SwapXY
		? shadowUVOffset.yx
		: shadowUVOffset.xy;

	float2 ScreenCoordOffset = 0.0f;
	ScreenCoordOffset += shadowUVOffset;

	Output.ScreenCoord = Input.Position.xy;
	Output.ScreenCoord += ScreenCoordOffset;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = PrepareVector
		? Input.Position.xy / ScreenDims
		: Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

uniform float HumBarHertzRate = 60.0f / 59.94f - 1.0f; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
uniform float HumBarAlpha = 0.0f;

uniform float TimeMilliseconds = 0.0f;

uniform float2 ScreenScale = float2(1.0f, 1.0f);
uniform float2 ScreenOffset = float2(0.0f, 0.0f);

uniform float ScanlineAlpha = 0.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineHeight = 1.0f;

uniform float3 BackColor = float3(0.0f, 0.0f, 0.0f);

uniform int ShadowTileMode = 0; // 0 based on screen dimension, 1 based on source dimension
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

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenTexelDims = 1.0f / ScreenDims;
	float2 SourceTexelDims = 1.0f / SourceDims;
	float2 SourceRes = SourceDims * SourceRect;

	float2 HalfSourceRect = SourceRect * 0.5f;

	float2 ScreenCoord = Input.ScreenCoord / ScreenDims;
	float2 BaseCoord = GetAdjustedCoords(Input.TexCoord, HalfSourceRect);

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	if (BaseCoord.x < 0.0f || BaseCoord.y < 0.0f)
	{
		BaseColor.rgb = 0.0f;
	}

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom && ShadowAlpha > 0.0f)
	{
		float2 shadowDims = ShadowDims;
		shadowDims = SwapXY
			? shadowDims.yx
			: shadowDims.xy;

		float2 shadowUV = ShadowUV;
		// shadowUV = SwapXY
			// ? shadowUV.yx
			// : shadowUV.xy;

		float2 screenCoord = ShadowTileMode == 0 ? ScreenCoord : BaseCoord;
		screenCoord = SwapXY
			? screenCoord.yx
			: screenCoord.xy;

		float2 shadowCount = ShadowCount;
		shadowCount = SwapXY
			? shadowCount.yx
			: shadowCount.xy;

		float2 shadowTile = ((ShadowTileMode == 0 ? ScreenTexelDims : SourceTexelDims) * shadowCount);
		shadowTile = SwapXY
			? shadowTile.yx
			: shadowTile.xy;

		float2 ShadowFrac = frac(screenCoord / shadowTile);
		float2 ShadowCoord = (ShadowFrac * shadowUV);
		ShadowCoord += 0.5f / shadowDims; // half texel offset
		// ShadowCoord = SwapXY
			// ? ShadowCoord.yx
			// : ShadowCoord.xy;

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
		if (!PrepareVector && ScanlineAlpha > 0.0f)
		{
			float ScanCoord = BaseCoord.y * SourceDims.y * ScanlineScale * PI;
			float ScanCoordJitter = ScanlineOffset * PHI;
			float ScanSine = sin(ScanCoord + ScanCoordJitter);
			float ScanSineScaled = pow(ScanSine * ScanSine, ScanlineHeight);
			float ScanBrightness = ScanSineScaled * ScanlineBrightScale + 1.0f + ScanlineBrightOffset;

			BaseColor.rgb *= lerp(1.0f, ScanBrightness * 0.5f, ScanlineAlpha);
		}

		// Hum Bar Simulation (may not affect vector screen)
		if (!PrepareVector && HumBarAlpha > 0.0f)
		{
			float HumTimeStep = frac(TimeMilliseconds * HumBarHertzRate);
			float HumBrightness = 1.0 - frac(BaseCoord.y / SourceRect.y + HumTimeStep) * HumBarAlpha;
			BaseColor.rgb *= HumBrightness;
		}
	}

	// Output
	float4 Output = PrepareVector
		? BaseColor * (Input.Color + float4(1.0f, 1.0f, 1.0f, 0.0f))
		: BaseColor * Input.Color;
	Output.a = 1.0f;

	return Output;
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