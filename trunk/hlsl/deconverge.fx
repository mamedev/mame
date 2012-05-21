//-----------------------------------------------------------------------------
// Deconvergence Effect
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
	float3 CoordX : TEXCOORD0;
	float3 CoordY : TEXCOORD1;
	float2 TexCoord : TEXCOORD2;
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
	float3 CoordX : TEXCOORD0;
	float3 CoordY : TEXCOORD1;
	float2 TexCoord : TEXCOORD2;
};

//-----------------------------------------------------------------------------
// Deconvergence Vertex Shader
//-----------------------------------------------------------------------------

uniform float3 ConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 ConvergeY = float3(0.0f, 0.0f, 0.0f);

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

uniform float WidthRatio;
uniform float HeightRatio;

uniform float3 RadialConvergeX = float3(0.0f, 0.0f, 0.0f);
uniform float3 RadialConvergeY = float3(0.0f, 0.0f, 0.0f);

uniform float Prescale;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;
	
	float2 TargetRawRatio = float2(TargetWidth / RawWidth, TargetWidth / RawWidth);
	float2 invDims = float2(1.0f / RawWidth, 1.0f / RawHeight);
	float2 Ratios = float2(1.0f / WidthRatio, 1.0f / HeightRatio);
	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.x /= TargetWidth;
	Output.Position.y /= TargetHeight;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.x -= 0.5f;
	Output.Position.y -= 0.5f;
	Output.Position *= float4(2.0f, 2.0f, 1.0f, 1.0f);
	Output.Color = Input.Color;
	float2 TexCoord = Input.TexCoord;

	Output.CoordX = ((((TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + RadialConvergeX / RawWidth) + 0.5f) * Ratios.x + ConvergeX * invDims.x;
	Output.CoordY = ((((TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + RadialConvergeY / RawHeight) + 0.5f) * Ratios.y + ConvergeY * invDims.y;
	Output.TexCoord = TexCoord;	

	return Output;
}

//-----------------------------------------------------------------------------
// Deconvergence Pixel Shader
//-----------------------------------------------------------------------------

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 MagnetOffset = float2(32.0f / RawWidth, 32.0f / RawHeight);
	float2 MagnetCenter = float2(0.9f / WidthRatio, 0.9f / HeightRatio);
	float MagnetDistance = length((MagnetCenter - Input.TexCoord) * float2(WidthRatio, HeightRatio));
	float Deconverge = 1.0f - MagnetDistance / MagnetCenter;
	Deconverge = 1.0f;//clamp(Deconverge, 0.0f, 1.0f);
	float Alpha = tex2D(DiffuseSampler, Input.TexCoord).a;
	
	float2 TargetDims = float2(RawWidth, RawHeight);
	float2 DimOffset = 0.0f / TargetDims;
	float2 TexCoord = Input.TexCoord;
	float3 CoordX = Input.CoordX;
	float3 CoordY = Input.CoordY;
	
	CoordX = lerp(TexCoord.x, CoordX, Deconverge);
	CoordY = lerp(TexCoord.y, CoordY, Deconverge);

	float RedTexel = tex2D(DiffuseSampler, float2(CoordX.x, CoordY.x) - DimOffset).r;
	float GrnTexel = tex2D(DiffuseSampler, float2(CoordX.y, CoordY.y) - DimOffset).g;
	float BluTexel = tex2D(DiffuseSampler, float2(CoordX.z, CoordY.z) - DimOffset).b;
	
	//RedTexel *= Input.RedCoord.x < (WidthRatio / RawWidth) ? 0.0f : 1.0f;
	//RedTexel *= Input.RedCoord.y < (HeightRatio / RawHeight) ? 0.0f : 1.0f;
	//RedTexel *= Input.RedCoord.x > (1.0f / WidthRatio + 1.0f / RawWidth) ? 0.0f : 1.0f;
	//RedTexel *= Input.RedCoord.y > (1.0f / HeightRatio + 1.0f / RawHeight) ? 0.0f : 1.0f;
	//GrnTexel *= Input.GrnCoord.x < (WidthRatio / RawWidth) ? 0.0f : 1.0f;
	//GrnTexel *= Input.GrnCoord.y < (HeightRatio / RawHeight) ? 0.0f : 1.0f;
	//GrnTexel *= Input.GrnCoord.x > (1.0f / WidthRatio + 1.0f / RawWidth) ? 0.0f : 1.0f;
	//GrnTexel *= Input.GrnCoord.y > (1.0f / HeightRatio + 1.0f / RawHeight) ? 0.0f : 1.0f;
	//BluTexel *= Input.BluCoord.x < (WidthRatio / RawWidth) ? 0.0f : 1.0f;
	//BluTexel *= Input.BluCoord.y < (HeightRatio / RawHeight) ? 0.0f : 1.0f;
	//BluTexel *= Input.BluCoord.x > (1.0f / WidthRatio + 1.0f / RawWidth) ? 0.0f : 1.0f;
	//BluTexel *= Input.BluCoord.y > (1.0f / HeightRatio + 1.0f / RawHeight) ? 0.0f : 1.0f;

	return float4(RedTexel, GrnTexel, BluTexel, Alpha);
}

//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

technique DeconvergeTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_3_0 vs_main();
		PixelShader  = compile ps_3_0 ps_main();
	}
}
