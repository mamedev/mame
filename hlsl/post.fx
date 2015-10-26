// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline & Shadowmask Effect
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

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

bool xor(bool a, bool b)
{
	return (a || b) && !(a && b);
}

//-----------------------------------------------------------------------------
// Scanline & Shadowmask Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims; // size of the window or fullscreen
uniform float2 SourceDims; // size of the texture in power-of-two size
uniform float2 TargetDims; // size of the target surface

uniform float2 ShadowDims = float2(32.0f, 32.0f); // size of the shadow texture (extended to power-of-two size)
uniform float2 ShadowUVOffset = float2(0.0f, 0.0f);

uniform bool OrientationSwapXY = false; // false landscape, true portrait for default screen orientation
uniform bool RotationSwapXY = false; // swapped default screen orientation due to screen rotation

uniform bool PrepareBloom = false; // disables some effects for rendering bloom textures
uniform bool PrepareVector = false;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	float2 shadowUVOffset = ShadowUVOffset;
	shadowUVOffset = xor(OrientationSwapXY, RotationSwapXY)
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
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float ScanlineAlpha = 1.0f;
uniform float ScanlineScale = 1.0f;
uniform float ScanlineBrightScale = 1.0f;
uniform float ScanlineBrightOffset = 1.0f;
uniform float ScanlineOffset = 1.0f;
uniform float ScanlineHeight = 1.0f;

uniform float ShadowAlpha = 0.0f;
uniform float2 ShadowCount = float2(6.0f, 6.0f);
uniform float2 ShadowUV = float2(0.25f, 0.25f);

uniform float3 Power = float3(1.0f, 1.0f, 1.0f);
uniform float3 Floor = float3(0.0f, 0.0f, 0.0f);

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 ScreenTexelDims = 1.0f / ScreenDims;

	float2 ScreenCoord = Input.ScreenCoord / ScreenDims;
	float2 BaseCoord = Input.TexCoord;

	// Color
	float4 BaseColor = tex2D(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0f;

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom)
	{
		float2 shadowDims = ShadowDims;
		shadowDims = xor(OrientationSwapXY, RotationSwapXY)
			? shadowDims.yx
			: shadowDims.xy;

		float2 shadowUV = ShadowUV;
		// shadowUV = xor(OrientationSwapXY, RotationSwapXY)
			// ? shadowUV.yx
			// : shadowUV.xy;

		float2 screenCoord = ScreenCoord;
		screenCoord = xor(OrientationSwapXY, RotationSwapXY)
			? screenCoord.yx
			: screenCoord.xy;

		float2 shadowCount = ShadowCount;
		shadowCount = xor(OrientationSwapXY, RotationSwapXY)
			? shadowCount.yx
			: shadowCount.xy;

		float2 shadowTile = (ScreenTexelDims * shadowCount);
		shadowTile = xor(OrientationSwapXY, RotationSwapXY)
			? shadowTile.yx
			: shadowTile.xy;

		float2 ShadowFrac = frac(screenCoord / shadowTile);
		float2 ShadowCoord = (ShadowFrac * shadowUV);
		ShadowCoord += 0.5f / shadowDims; // half texel offset
		// ShadowCoord = xor(OrientationSwapXY, RotationSwapXY)
			// ? ShadowCoord.yx
			// : ShadowCoord.xy;

		float3 ShadowColor = tex2D(ShadowSampler, ShadowCoord).rgb;
		ShadowColor = lerp(1.0f, ShadowColor, ShadowAlpha);

		BaseColor.rgb *= ShadowColor;
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
		// Scanline Simulation (disabled for vector)
		if (!PrepareVector)
		{
			float InnerSine = BaseCoord.y * ScanlineScale * SourceDims.y;
			float ScanJitter = ScanlineOffset * SourceDims.y;
			float ScanBrightMod = sin(InnerSine * PI + ScanJitter);
			float3 ScanColor = lerp(1.0f, (pow(ScanBrightMod * ScanBrightMod, ScanlineHeight) * ScanlineBrightScale + 1.0f + ScanlineBrightOffset) * 0.5f, ScanlineAlpha);

			BaseColor.rgb *= ScanColor;
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
// Scanline & Shadowmask Effect
//-----------------------------------------------------------------------------

technique ScanMaskTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = <DiffuseSampler>;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader = compile ps_3_0 ps_main();
	}
}