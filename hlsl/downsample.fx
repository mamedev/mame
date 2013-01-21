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
	float4 TexCoord01 : TEXCOORD0;
	float4 TexCoord23 : TEXCOORD1;
	float4 TexCoord45 : TEXCOORD2;
	float4 TexCoord67 : TEXCOORD3;
	float4 TexCoord89 : TEXCOORD4;
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
	float4 TexCoord01 : TEXCOORD0;
	float4 TexCoord23 : TEXCOORD1;
	float4 TexCoord45 : TEXCOORD2;
	float4 TexCoord67 : TEXCOORD3;
	float4 TexCoord89 : TEXCOORD4;
};

//-----------------------------------------------------------------------------
// Downsample Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 TargetSize;
uniform float2 SourceSize;

uniform float2 Defocus = float2(0.0f, 0.0f);

uniform float Brighten;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= TargetSize;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= float2(0.5f, 0.5f);
	Output.Position.xy *= float2(2.0f, 2.0f);
	Output.Color = Input.Color;
	float2 inversePixel = 1.0f / TargetSize;
	Output.TexCoord01.xy = Input.Position.xy * inversePixel + float2(0.75f - 0.5f, 0.32f - 0.5f) * inversePixel;
	Output.TexCoord01.zw = Input.Position.xy * inversePixel + float2(0.35f - 0.5f, 0.65f - 0.5f) * inversePixel;
	Output.TexCoord23.xy = Input.Position.xy * inversePixel + float2(0.31f - 0.5f, 1.25f - 0.5f) * inversePixel;
	Output.TexCoord23.zw = Input.Position.xy * inversePixel + float2(0.58f - 0.5f, 1.61f - 0.5f) * inversePixel;
	Output.TexCoord45.xy = Input.Position.xy * inversePixel + float2(1.00f - 0.5f, 1.75f - 0.5f) * inversePixel;
	Output.TexCoord45.zw = Input.Position.xy * inversePixel + float2(1.35f - 0.5f, 1.64f - 0.5f) * inversePixel;
	Output.TexCoord67.xy = Input.Position.xy * inversePixel + float2(1.65f - 0.5f, 1.32f - 0.5f) * inversePixel;
	Output.TexCoord67.zw = Input.Position.xy * inversePixel + float2(1.72f - 0.5f, 0.93f - 0.5f) * inversePixel;
	Output.TexCoord89.xy = Input.Position.xy * inversePixel + float2(1.57f - 0.5f, 0.57f - 0.5f) * inversePixel;
	Output.TexCoord89.zw = Input.Position.xy * inversePixel + float2(1.25f - 0.5f, 0.33f - 0.5f) * inversePixel;

	return Output;
}

//-----------------------------------------------------------------------------
// Downsample Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float4 texel0 = tex2D(DiffuseSampler, Input.TexCoord01.xy);
	float4 texel1 = tex2D(DiffuseSampler, Input.TexCoord01.zw);
	float4 texel2 = tex2D(DiffuseSampler, Input.TexCoord23.xy);
	float4 texel3 = tex2D(DiffuseSampler, Input.TexCoord23.zw);
	float4 texel4 = tex2D(DiffuseSampler, Input.TexCoord45.xy);
	float4 texel5 = tex2D(DiffuseSampler, Input.TexCoord45.zw);
	float4 texel6 = tex2D(DiffuseSampler, Input.TexCoord67.xy);
	float4 texel7 = tex2D(DiffuseSampler, Input.TexCoord67.zw);
	float4 texel8 = tex2D(DiffuseSampler, Input.TexCoord89.xy);
	float4 texel9 = tex2D(DiffuseSampler, Input.TexCoord89.zw);
	float4 outTexel = (texel0 + texel1 + texel2 + texel3 + texel4 + texel5 + texel6 + texel7 + texel8 + texel9) * 0.1f;
	return float4(outTexel.rgb, 1.0f);
}

//-----------------------------------------------------------------------------
// Downsample Effect
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
