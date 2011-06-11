//-----------------------------------------------------------------------------
// Effect File Variables
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
	float2 TexCoord0 : TEXCOORD0;
	float2 TexCoord1 : TEXCOORD1;
	float2 TexCoord2 : TEXCOORD2;
	float2 TexCoord3 : TEXCOORD3;
	float2 TexCoord4 : TEXCOORD4;
	float2 TexCoord5 : TEXCOORD5;
	float2 TexCoord6 : TEXCOORD6;
	float2 TexCoord7 : TEXCOORD7;
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
	float2 TexCoord0 : TEXCOORD0;
	float2 TexCoord1 : TEXCOORD1;
	float2 TexCoord2 : TEXCOORD2;
	float2 TexCoord3 : TEXCOORD3;
	float2 TexCoord4 : TEXCOORD4;
	float2 TexCoord5 : TEXCOORD5;
	float2 TexCoord6 : TEXCOORD6;
	float2 TexCoord7 : TEXCOORD7;
};

//-----------------------------------------------------------------------------
// Simple Vertex Shader
//-----------------------------------------------------------------------------

uniform float TargetWidth;
uniform float TargetHeight;

uniform float WidthRatio;
uniform float HeightRatio;

uniform float2 Defocus = float2(0.0f, 0.0f);
uniform float FocusEnable;

float2 Coord0Offset = float2( 0.0f,  0.0f);
float2 Coord1Offset = float2(-0.2f, -0.6f);
float2 Coord2Offset = float2( 0.4f, -0.4f);
float2 Coord3Offset = float2( 0.6f,  0.2f);
float2 Coord4Offset = float2( 0.2f,  0.6f);
float2 Coord5Offset = float2(-0.4f,  0.6f);
float2 Coord6Offset = float2(-0.6f,  0.2f);
float2 Coord7Offset = float2(-0.6f, -0.4f);

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
	
	float2 InvTexSize = float2(1.0f / TargetWidth, 1.0f / TargetHeight);
	float2 TexCoord = Input.TexCoord;
	TexCoord = TexCoord + 0.5f * InvTexSize;
	Output.TexCoord0 = TexCoord + Coord0Offset * InvTexSize * Defocus;
	Output.TexCoord1 = TexCoord + Coord1Offset * InvTexSize * Defocus;
	Output.TexCoord2 = TexCoord + Coord2Offset * InvTexSize * Defocus;
	Output.TexCoord3 = TexCoord + Coord3Offset * InvTexSize * Defocus;
	Output.TexCoord4 = TexCoord + Coord4Offset * InvTexSize * Defocus;
	Output.TexCoord5 = TexCoord + Coord5Offset * InvTexSize * Defocus;
	Output.TexCoord6 = TexCoord + Coord6Offset * InvTexSize * Defocus;
	Output.TexCoord7 = TexCoord + Coord7Offset * InvTexSize * Defocus;

	return Output;
}

//-----------------------------------------------------------------------------
// Simple Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 d0 = tex2D(DiffuseSampler, Input.TexCoord0);
	float3 d1 = tex2D(DiffuseSampler, Input.TexCoord1).rgb;
	float3 d2 = tex2D(DiffuseSampler, Input.TexCoord2).rgb;
	float3 d3 = tex2D(DiffuseSampler, Input.TexCoord3).rgb;
	float3 d4 = tex2D(DiffuseSampler, Input.TexCoord4).rgb;
	float3 d5 = tex2D(DiffuseSampler, Input.TexCoord5).rgb;
	float3 d6 = tex2D(DiffuseSampler, Input.TexCoord6).rgb;
	float3 d7 = tex2D(DiffuseSampler, Input.TexCoord7).rgb;

	float3 blurred = (d0.rgb + d1 + d2 + d3 + d4 + d5 + d6 + d7) / 8.0f;

	blurred = lerp(d0.rgb, blurred, 1.0f);
	return float4(blurred, d0.a);
}

//-----------------------------------------------------------------------------
// Simple Effect
//-----------------------------------------------------------------------------

technique TestTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		Sampler[0] = <DiffuseSampler>;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
