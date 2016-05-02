// license:BSD-3-Clause
// copyright-holders: Ted Green

cbuffer cbFrameBufferCtrl : register(b0)
{
	float xSize;
	float ySize;
	int flipY;
	int enablePerspective0;
	int enablePerspective1;
	float pad00, pad01, pad02;
};

struct Combine_Struct {
	int c_zero_cother;
	int c_sub_clocal;
	int  c_mselect;
	int c_reverse_blend;
	int  c_add_aclocal;
	int c_invert_output;
	int a_zero_aother;
	int a_sub_alocal;
	int  a_mselect;
	int a_reverse_blend;
	int  a_add_aclocal;
	int a_invert_output;
};

cbuffer cbColorCtrl : register(b1)
{
	float4 color0;
	float4 color1;
	Combine_Struct colCtrl;
	int rgbselect;
	int aselect;
	int c_localselect;
	int a_localselect;
	int c_localselect_override;
	int enable_param_clamp;
	int enable_texture;
	// Depth Ctrl
	int depth_enable;
	int depthfloat_select;
	int wfloat_select;
	int zbias_enable;
	int depth_src_select;
	float zBias;  // From zaColor
	float zSrc;  // From zaColor
	// Alpha Testing
	int alpha_test_enable;
	int alpha_op;
	float alpha_ref;
	// Select pixel mode
	int pixel_mode;
	// Dithering
	int dither_sel;
	float pad10;
};


#define NUM_TEX 2
struct Tex_Ctrl_Struct
{
	float2 texSize;
	int enable;
	int tloddither;
	float detailScale;
	float detailBias;
	float detailMax;
	int send_config;
	//float pad;
};

cbuffer cbTexInterface : register(b2)
{
	Combine_Struct texCombine[NUM_TEX];
	Tex_Ctrl_Struct texCtrl[NUM_TEX];
	//float pad20, pad21;
};
//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------
float2 ConvertScreenPos(float2 posXY);

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
	float4 Col : COLOR;
	float3 TexPos0 : TEXCOORD0;
	float3 TexPos1 : TEXCOORD1;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	noperspective float Oow : FOG;
	float4 Col : COLOR;
	float4 TexPos0 : TEXCOORD0;
	float4 TexPos1 : TEXCOORD1;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.Pos.xy = ConvertScreenPos(input.Pos.xy);

	output.Pos.z = input.Pos.z;
	if (pixel_mode) {
		output.Pos.w = 1.0f;
	}
	else {
		output.Pos.w = rcp(input.Pos.w);
	}
	output.Pos.xy = output.Pos.xy * output.Pos.w;

	output.Oow = input.Pos.w;
	
	output.Col = input.Col / 255.0f;

	if (pixel_mode) {
		output.TexPos0 = 0.0f;
		output.TexPos1 = 0.0f;
	} else {
		// TMU0
		output.TexPos0.z = 0.0f;
		if (enablePerspective0==0)
			output.TexPos0.w = 1.0f;
		else
			output.TexPos0.w = rcp(input.TexPos0.z);
		output.TexPos0.xy = input.TexPos0.xy * output.TexPos0.w / texCtrl[0].texSize.xy;
		// TMU1
		output.TexPos1.z = 0.0f;
		if (enablePerspective1==0)
			output.TexPos1.w = 1.0f;
		else
			output.TexPos1.w = rcp(input.TexPos1.z);
		output.TexPos1.xy = input.TexPos1.xy * output.TexPos1.w / texCtrl[1].texSize.xy;
	}
	return output;
}
//--------------------------------------------------------------------------------------
// Pixel Vertex Shader
//--------------------------------------------------------------------------------------
struct PIXEL_VS_INPUT
{
	float4 intPos : POSITION;
	float4 intCol : COLOR;
};


PS_INPUT PIXEL_VS(PIXEL_VS_INPUT input)
{
	PS_INPUT output;

	output.Pos.xy = ConvertScreenPos(float2(asint(input.intPos.xy)));

	output.Pos.z = float(asint(input.intPos.z)) / 65535.0f;
	output.Pos.w = 1.0f;

	output.Oow = float(asint(input.intPos.w)) / 65535.0f;
	
	output.Col.r = float(asint(input.intCol.r));
	output.Col.g = float(asint(input.intCol.g));
	output.Col.b = float(asint(input.intCol.b));
	output.Col.a = float(asint(input.intCol.a));
	output.Col /= 255.0f;

	output.TexPos0 = 0.0f;
	output.TexPos1 = 0.0f;

	return output;
}

//--------------------------------------------------------------------------------------
// Comp Vertex Shader
//--------------------------------------------------------------------------------------
struct COMP_VS_INPUT
{
	float4 Pos : POSITION;
};

struct COMP_PS_INPUT
{
	float4 Pos : SV_POSITION;
	noperspective float Oow : FOG;
	float2 TexPos : TEXCOORD;
};

COMP_PS_INPUT COMP_VS(COMP_VS_INPUT input)
{
	COMP_PS_INPUT output = (COMP_PS_INPUT)0;

	if (pixel_mode) {
		output.Pos.xy = ConvertScreenPos(float2(asint(input.Pos.xy)));

		output.Pos.z = float(asint(input.Pos.z)) / 65535.0f;
	} else {
		output.Pos.xy = ConvertScreenPos(input.Pos.xy);

		output.Pos.z = input.Pos.z;
	}

	output.Pos.w = 1.0f;

	output.Oow = input.Pos.w;

	// Convert screen xy coordinates to texture uv coordinates
	output.TexPos.x = (output.Pos.x + 1.0f) / 2.0f;
	output.TexPos.y = (1.0f - output.Pos.y) / 2.0f;

	return output;
}

//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------
float2 ConvertScreenPos(float2 posXY)
{
	float2 output;
	output.x = posXY.x / (xSize / 2.0f) - 1.0f + 1 / xSize;
	output.y = 1.0f - posXY.y / (ySize / 2.0f) - 1 / ySize;
	if (flipY)
		output.y = -output.y;
	return output;
}

