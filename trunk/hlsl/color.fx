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
	
	float2 invDims = float2(1.0f / RawWidth, 1.0f / RawHeight);
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.x /= TargetWidth;
	Output.Position.y /= TargetHeight;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.x -= 0.5f;
	Output.Position.y -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	Output.TexCoord = Input.TexCoord + float2(0.5f, 0.5f) * invDims;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float3 RedRatios = float3(1.0f, 0.0f, 0.0f);
uniform float3 GrnRatios = float3(0.0f, 1.0f, 0.0f);
uniform float3 BluRatios = float3(0.0f, 0.0f, 1.0f);
uniform float3 Offset = float3(0.0f, 0.0f, 0.0f);
uniform float3 Scale = float3(1.0f, 1.0f, 1.0f);
uniform float Saturation = 1.0f;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);
	
	float3 OutRGB = BaseTexel.rgb;

	// -- RGB Tint & Shift --
	float ShiftedRed = dot(OutRGB, RedRatios);
	float ShiftedGrn = dot(OutRGB, GrnRatios);
	float ShiftedBlu = dot(OutRGB, BluRatios);
	
	// -- RGB Offset & Scale --
	float3 OutTexel = float3(ShiftedRed, ShiftedGrn, ShiftedBlu) * Scale + Offset;
	
	// -- Saturation --
	float3 Gray = float3(0.3f, 0.59f, 0.11f);
	float OutLuma = dot(OutTexel, Gray);
	float3 OutChroma = OutTexel - OutLuma;
	float3 Saturated = OutLuma + OutChroma * Saturation;
	
	return float4(Saturated, BaseTexel.a);
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
