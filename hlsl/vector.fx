// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
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
	float2 SizeInfo : TEXCOORD1;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 SizeInfo : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Color : COLOR0;
	float2 TexCoord : TEXCOORD0;
	float2 SizeInfo : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(float2 p, float2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0f)) - r;
}

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
	Output.SizeInfo = Input.SizeInfo;

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
uniform float BeamSmooth;

float GetRoundCornerFactor(float2 coord, float2 bounds, float radiusAmount, float smoothAmount)
{
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(bounds.x, bounds.y);
	float amountMinimum = 1.0f / range;
	float radius = range * max(radiusAmount, amountMinimum);
	float smooth = 1.0f / (range * max(smoothAmount, amountMinimum * 2.0f));

	// compute box
	float box = roundBox(bounds * (coord * 2.0f), bounds, radius);

	// apply smooth
	box *= smooth;
	box += 1.0f - pow(smooth * 0.5f, 0.5f);

	float border = smoothstep(1.0f, 0.0f, box);

	return saturate(border);
}

float4 ps_main(PS_INPUT Input) : COLOR
{
	float2 lineSize = Input.SizeInfo / max(QuadDims.x, QuadDims.y); // normalize

	float lineLength = lineSize.x;
	float lineLengthRatio = LengthRatio;
	float lineLengthScale = LengthScale;

	float timeModulate = lerp(1.0f, TimeRatio, TimeScale);
	float lengthModulate = 1.0f - clamp(lineLength / lineLengthRatio, 0.0f, 1.0f);
	float timeLengthModulate = lerp(1.0f, timeModulate * lengthModulate, LengthScale);

	float4 outColor = float4(timeLengthModulate, timeLengthModulate, timeLengthModulate, 1.0f);
	outColor *= Input.Color;

	float RoundCornerFactor = GetRoundCornerFactor(Input.TexCoord - 0.5f, Input.SizeInfo, 1.0f, BeamSmooth);
	outColor.rgb *= RoundCornerFactor;

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
