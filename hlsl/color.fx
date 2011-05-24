//-----------------------------------------------------------------------------
// Color-Convolution Effect
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
};

//-----------------------------------------------------------------------------
// Post-Processing Vertex Shader
//-----------------------------------------------------------------------------

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

uniform float WidthRatio;
uniform float HeightRatio;

uniform float YIQEnable;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.x /= TargetWidth;
	Output.Position.y /= TargetHeight;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.x -= 0.5f;
	Output.Position.y -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.TexCoord = Input.TexCoord;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float RedFromRed = 1.0f;
uniform float RedFromGrn = 0.0f;
uniform float RedFromBlu = 0.0f;
uniform float GrnFromRed = 0.0f;
uniform float GrnFromGrn = 1.0f;
uniform float GrnFromBlu = 0.0f;
uniform float BluFromRed = 0.0f;
uniform float BluFromGrn = 0.0f;
uniform float BluFromBlu = 1.0f;

uniform float RedOffset = 0.0f;
uniform float GrnOffset = 0.0f;
uniform float BluOffset = 0.0f;

uniform float RedScale = 1.0f;
uniform float GrnScale = 1.0f;
uniform float BluScale = 1.0f;

uniform float RedFloor = 0.0f;
uniform float GrnFloor = 0.0f;
uniform float BluFloor = 0.0f;

uniform float Saturation = 1.0f;

uniform float RedPower = 2.2f;
uniform float GrnPower = 2.2f;
uniform float BluPower = 2.2f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord + 0.5f / float2(-RawWidth, -RawHeight));
	
	float3 OutRGB = BaseTexel.rgb;

	// -- RGB Tint & Shift --
	float ShiftedRed = dot(OutRGB, float3(RedFromRed, RedFromGrn, RedFromBlu));
	float ShiftedGrn = dot(OutRGB, float3(GrnFromRed, GrnFromGrn, GrnFromBlu));
	float ShiftedBlu = dot(OutRGB, float3(BluFromRed, BluFromGrn, BluFromBlu));
	
	// -- RGB Offset & Scale --
	float3 RGBScale = float3(RedScale, GrnScale, BluScale);
	float3 RGBShift = float3(RedOffset, GrnOffset, BluOffset);
	float3 OutTexel = float3(ShiftedRed, ShiftedGrn, ShiftedBlu) * RGBScale + RGBShift;
	
	// -- Saturation --
	float3 Gray = float3(0.3f, 0.59f, 0.11f);
	float OutLuma = dot(OutTexel, Gray);
	float3 OutChroma = OutTexel - OutLuma;
	float3 Saturated = OutLuma + OutChroma * Saturation;
	
	OutRGB.r = pow(Saturated.r, RedPower);
	OutRGB.g = pow(Saturated.g, GrnPower);
	OutRGB.b = pow(Saturated.b, BluPower);

	// -- Color Compression (increasing the floor of the signal without affecting the ceiling) --
	OutRGB = float3(RedFloor + (1.0f - RedFloor) * OutRGB.r, GrnFloor + (1.0f - GrnFloor) * OutRGB.g, BluFloor + (1.0f - BluFloor) * OutRGB.b);

	return float4(OutRGB, BaseTexel.a);
}

//-----------------------------------------------------------------------------
// Color-Convolution Technique
//-----------------------------------------------------------------------------

technique ColorTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
