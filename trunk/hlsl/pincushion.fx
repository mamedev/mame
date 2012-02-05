//-----------------------------------------------------------------------------
// Pincushion Post-Processing Effect
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
	float2 RedCoord : TEXCOORD2;
	float2 GreenCoord : TEXCOORD3;
	float2 BlueCoord : TEXCOORD4;
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
	float2 TexCoord : TEXCOORD0;
	float2 RedCoord : TEXCOORD2;
	float2 GreenCoord : TEXCOORD3;
	float2 BlueCoord : TEXCOORD4;
};

//-----------------------------------------------------------------------------
// Post-Processing Vertex Shader
//-----------------------------------------------------------------------------

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

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
	Output.TexCoord = Input.TexCoord;
	
	Output.TexCoord.x -= 0.25f;
	Output.TexCoord.y -= 0.25f;
	Output.TexCoord.x -= 0.5f;
	Output.TexCoord.y -= 0.5f;
	Output.TexCoord.x /= 18.0f * (TargetWidth / RawWidth);
	Output.TexCoord.y /= 18.0f * (TargetHeight / RawHeight);
	Output.TexCoord.x += 0.5f;
	Output.TexCoord.y += 0.5f;

	return Output;
}

//-----------------------------------------------------------------------------
// Post-Processing Pixel Shader
//-----------------------------------------------------------------------------

uniform float PI = 3.14159265f;

uniform float PincushionAmountX = 0.1f;
uniform float PincushionAmountY = 0.1f;

uniform float WidthRatio;
uniform float HeightRatio;

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 Ratios = float2(WidthRatio, HeightRatio);

	// -- Screen Pincushion Calculation --
	float2 UnitCoord = Input.TexCoord * Ratios * 2.0f - 1.0f;

	float PincushionR2 = pow(length(UnitCoord),2.0f) / pow(length(Ratios), 2.0f);
	float2 PincushionCurve = UnitCoord * PincushionAmountX * PincushionR2;
	float2 BaseCoord = Input.TexCoord + PincushionCurve;

	return tex2D(DiffuseSampler, BaseCoord);
}

//-----------------------------------------------------------------------------
// Post-Processing Effect
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
