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
	float2 RedCoord : TEXCOORD0;
	float2 GrnCoord : TEXCOORD1;
	float2 BluCoord : TEXCOORD2;
	float2 TexCoord : TEXCOORD3;
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
	float2 RedCoord : TEXCOORD0;
	float2 GrnCoord : TEXCOORD1;
	float2 BluCoord : TEXCOORD2;
	float2 TexCoord : TEXCOORD3;
};

//-----------------------------------------------------------------------------
// Deconvergence Vertex Shader
//-----------------------------------------------------------------------------

uniform float RedConvergeX;
uniform float RedConvergeY;
uniform float GrnConvergeX;
uniform float GrnConvergeY;
uniform float BluConvergeX;
uniform float BluConvergeY;

uniform float TargetWidth;
uniform float TargetHeight;

uniform float RawWidth;
uniform float RawHeight;

uniform float WidthRatio;
uniform float HeightRatio;

uniform float RedRadialConvergeX;
uniform float RedRadialConvergeY;
uniform float GrnRadialConvergeX;
uniform float GrnRadialConvergeY;
uniform float BluRadialConvergeX;
uniform float BluRadialConvergeY;

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

	Output.RedCoord.x = ((((TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + RedRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + RedConvergeX * invDims.x;
	Output.GrnCoord.x = ((((TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + GrnRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + GrnConvergeX * invDims.x;
	Output.BluCoord.x = ((((TexCoord.x / Ratios.x) - 0.5f)) * (1.0f + BluRadialConvergeX / RawWidth) + 0.5f) * Ratios.x + BluConvergeX * invDims.x;
	
	Output.RedCoord.y = ((((TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + RedRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + RedConvergeY * invDims.y;
	Output.GrnCoord.y = ((((TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + GrnRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + GrnConvergeY * invDims.y;
	Output.BluCoord.y = ((((TexCoord.y / Ratios.y) - 0.5f)) * (1.0f + BluRadialConvergeY / RawHeight) + 0.5f) * Ratios.y + BluConvergeY * invDims.y;

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
	
	float2 RawDims = float2(RawWidth, RawHeight);
	float2 TexCoord = Input.TexCoord * RawDims;
	float2 RedCoord = Input.RedCoord * RawDims;
	float2 GrnCoord = Input.GrnCoord * RawDims;
	float2 BluCoord = Input.BluCoord * RawDims;
	TexCoord.y = TexCoord.y - frac(TexCoord.y);
	RedCoord.y = RedCoord.y - frac(RedCoord.y);
	GrnCoord.y = GrnCoord.y - frac(GrnCoord.y);
	BluCoord.y = BluCoord.y - frac(BluCoord.y);
	
	RedCoord = lerp(TexCoord, RedCoord, Deconverge) / RawDims;
	GrnCoord = lerp(TexCoord, GrnCoord, Deconverge) / RawDims;
	BluCoord = lerp(TexCoord, BluCoord, Deconverge) / RawDims;

	float RedTexel = tex2D(DiffuseSampler, RedCoord + float2(0.0f, 0.5f) / RawDims).r;
	float GrnTexel = tex2D(DiffuseSampler, GrnCoord + float2(0.0f, 0.5f) / RawDims).g;
	float BluTexel = tex2D(DiffuseSampler, BluCoord + float2(0.0f, 0.5f) / RawDims).b;
	
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
