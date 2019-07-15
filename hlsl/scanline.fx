// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;

sampler DiffuseSampler = sampler_state
{
	Texture = <Diffuse>;
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
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const float PI = 3.1415927f;
static const float HalfPI = PI * 0.5f;

//-----------------------------------------------------------------------------
// Scanline Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 SourceDims;
uniform float2 TargetDims;
uniform float2 QuadDims;

uniform bool SwapXY = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // fix half texel offset (DX9)

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Scanline Pixel Shader
//-----------------------------------------------------------------------------


uniform float2 ScreenScale = float2(1.0f, 1.0f);
uniform float2 ScreenOffset = float2(0.0f, 0.0f);

uniform float ScanlineAlpha = 0.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineHeight = 1.0f;
uniform float ScanlineVariation = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;

float2 GetAdjustedCoords(float2 coord)
{
	// center coordinates
	coord -= 0.5f;

	// apply screen scale
	coord *= ScreenScale;

	// un-center coordinates
	coord += 0.5f;

	// apply screen offset
	coord += ScreenOffset;

	return coord;
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 BaseCoord = GetAdjustedCoords(Input.TexCoord);

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	// clip border
	if (BaseCoord.x < 0.0f || BaseCoord.y < 0.0f ||
		BaseCoord.x > 1.0f || BaseCoord.y > 1.0f)
	{
		// we don't use the clip function, because we don't clear the render target before
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	float BrightnessOffset = (ScanlineBrightOffset * ScanlineAlpha);
	float BrightnessScale = (ScanlineBrightScale * ScanlineAlpha) + (1.0f - ScanlineAlpha);

	float ColorBrightness = 0.299f * BaseColor.r + 0.587f * BaseColor.g + 0.114 * BaseColor.b;

	float ScanlineCoord = BaseCoord.y;
	ScanlineCoord += SwapXY
		? QuadDims.x <= SourceDims.x * 2.0f
			? 0.5f / QuadDims.x // uncenter scanlines if the quad is less than twice the size of the source
			: 0.0f
		: QuadDims.y <= SourceDims.y * 2.0f
			? 0.5f / QuadDims.y // uncenter scanlines if the quad is less than twice the size of the source
			: 0.0f;
	ScanlineCoord *= SourceDims.y * ScanlineScale * PI;

	float ScanlineCoordJitter = ScanlineOffset * HalfPI;
	float ScanlineSine = sin(ScanlineCoord + ScanlineCoordJitter);
	float ScanlineWide = ScanlineHeight + ScanlineVariation * max(1.0f, ScanlineHeight) * (1.0f - ColorBrightness);
	float ScanlineAmount = pow(ScanlineSine * ScanlineSine, ScanlineWide);
	float ScanlineBrightness = ScanlineAmount * BrightnessScale + BrightnessOffset * BrightnessScale;

	BaseColor.rgb *= lerp(1.0f, ScanlineBrightness, ScanlineAlpha);

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
