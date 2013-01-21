//-----------------------------------------------------------------------------
// Effect File Variables
//-----------------------------------------------------------------------------

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
// Simple Vertex Shader
//-----------------------------------------------------------------------------

uniform float TargetWidth;
uniform float TargetHeight;
uniform float2 TimeParams;
uniform float3 LengthParams;

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
	Output.TexCoord = Input.Position.xy / float2(TargetWidth, TargetHeight);

	return Output;
}

//-----------------------------------------------------------------------------
// Simple Pixel Shader
//-----------------------------------------------------------------------------

// TimeParams.x: Frame time of the vector
// TimeParams.y: How much frame time affects the vector's fade
// LengthParams.x: Length of the vector
// LengthParams.y: How much length affects the vector's fade
// LengthParams.z: Size at which fade is maximum
float4 ps_main(PS_INPUT Input) : COLOR
{
	float timeModulate = lerp(1.0f, TimeParams.x, TimeParams.y) * 2.0;

	float lengthModulate = clamp(1.0f - LengthParams.x / LengthParams.z, 0.0f, 1.0f);
	lengthModulate = lerp(1.0f, timeModulate * lengthModulate, LengthParams.y) * 2.0;

	float4 outColor = Input.Color * float4(lengthModulate, lengthModulate, lengthModulate, 1.0f) * 2.5;
	return outColor;
}

//-----------------------------------------------------------------------------
// Simple Effect
//-----------------------------------------------------------------------------

technique TestTechnique
{
	pass Pass0
	{
		Lighting = FALSE;

		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
	}
}
