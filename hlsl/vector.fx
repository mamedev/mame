// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Vector Effect
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Vertex Definitions
//-----------------------------------------------------------------------------

struct VS_OUTPUT
{
	float4 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 LineInfo : TEXCOORD1;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 LineInfo : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 LineInfo : TEXCOORD1; // x is the line length, y is unused
};

//-----------------------------------------------------------------------------
// Vector Vertex Shader
//-----------------------------------------------------------------------------

uniform float2 ScreenDims;
uniform float2 QuadDims;

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	Output.Position = float4(Input.Position.xyz, 1.0f);
	Output.Position.xy /= ScreenDims;
	Output.Position.y = 1.0f - Output.Position.y;
	Output.Position.xy -= 0.5f; // center
	Output.Position.xy *= 2.0f; // zoom

	Output.TexCoord = Input.TexCoord;
	Output.LineInfo = Input.LineInfo;

	Output.Color = Input.Color;

	return Output;
}

//-----------------------------------------------------------------------------
// Vector Pixel Shader
//-----------------------------------------------------------------------------

uniform float TimeRatio; // Frame time of the vector (not set)
uniform float TimeScale; // How much frame time affects the vector's fade (not set)
uniform float LengthRatio; // Size at which fade is maximum
uniform float LengthScale; // How much length affects the vector's fade

float4 ps_main(PS_INPUT Input) : COLOR
{
	float lineLength = Input.LineInfo.x / max(QuadDims.x, QuadDims.y); // normalize
	float lineLengthRatio = LengthRatio;
	float lineLengthScale = LengthScale;

	float timeModulate = lerp(1.0f, TimeRatio, TimeScale);
	float lengthModulate = 1.0f - clamp(lineLength / lineLengthRatio, 0.0f, 1.0f);
	float timeLengthModulate = lerp(1.0f, timeModulate * lengthModulate, LengthScale);

	float4 outColor = float4(timeLengthModulate, timeLengthModulate, timeLengthModulate, 1.0f);
	outColor *= Input.Color;

	return outColor;
}

//-----------------------------------------------------------------------------
// Vector Technique
//-----------------------------------------------------------------------------

technique DefaultTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader = compile ps_2_0 ps_main();
	}
}
