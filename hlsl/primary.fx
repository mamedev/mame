// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, W. M. Martinez
//-----------------------------------------------------------------------------
// Primary Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define LUT_TEXTURE_WIDTH 4096.0f
#define LUT_SIZE 64.0f
#define LUT_SCALE float2(1.0f / LUT_TEXTURE_WIDTH, 1.0f / LUT_SIZE)

//-----------------------------------------------------------------------------
// Sampler Definitions
//-----------------------------------------------------------------------------

texture Diffuse;
texture LutTexture;

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

sampler2D LutSampler = sampler_state
{
	Texture = <LutTexture>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
	AddressW = CLAMP;
};

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

float3 apply_lut(float3 color)
{
	// NOTE: Do not change the order of parameters here.
	float3 lutcoord = float3((color.rg * (LUT_SIZE - 1.0f) + 0.5f) *
		LUT_SCALE, color.b * (LUT_SIZE - 1.0f));
	float shift = floor(lutcoord.z);

	lutcoord.x += shift * LUT_SCALE.y;
	color.rgb = lerp(tex2D(LutSampler, lutcoord.xy).rgb, tex2D(LutSampler,
		float2(lutcoord.x + LUT_SCALE.y, lutcoord.y)).rgb,
		lutcoord.z - shift);
	return color;
}

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
	float3 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Primary Vertex Shaders
//-----------------------------------------------------------------------------

//static const float Epsilon = 1.0e-7f;

uniform float2 ScreenDims;
uniform float2 TargetDims;

VS_OUTPUT vs_screen_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	// Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

VS_OUTPUT vs_vector_buffer_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

VS_OUTPUT vs_ui_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y; // flip y
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	// Output.TexCoord += 0.5f / TargetDims; // half texel offset correction (DX9)

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Primary Pixel Shaders
//-----------------------------------------------------------------------------

uniform bool LutEnable;
uniform bool UiLutEnable;

float4 ps_screen_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);

	if (LutEnable)
		BaseTexel.rgb = apply_lut(BaseTexel.rgb);
	return BaseTexel;
}

float4 ps_vector_buffer_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);

	if (LutEnable)
		BaseTexel.rgb = apply_lut(BaseTexel.rgb);
	return BaseTexel;
}

float4 ps_ui_main(PS_INPUT Input) : COLOR
{
	float4 BaseTexel = tex2D(DiffuseSampler, Input.TexCoord);
	BaseTexel *= Input.Color;

	if (UiLutEnable)
		BaseTexel.rgb = apply_lut(BaseTexel.rgb);
	return BaseTexel;
}

//-----------------------------------------------------------------------------
// Primary Techniques
//-----------------------------------------------------------------------------

technique ScreenTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_screen_main();
		PixelShader  = compile ps_2_0 ps_screen_main();
	}
}

technique VectorBufferTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_vector_buffer_main();
		PixelShader  = compile ps_2_0 ps_vector_buffer_main();
	}
}

technique UiTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_ui_main();
		PixelShader  = compile ps_2_0 ps_ui_main();
	}
}
