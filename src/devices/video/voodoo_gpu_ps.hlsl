// license:BSD-3-Clause
// copyright-holders: Ted Green

Texture2D dither4x4Texture : register (t0);
Texture2D dither2x2Texture : register (t1);
SamplerState ditherState : register (s0);

Texture2D renderTexture : register (t2);
SamplerState renderState : register (s2);

Texture2D texTexture0 : register (t3);
SamplerState texState0 : register (s3);

Texture2D texTexture1 : register (t4);
SamplerState texState1 : register (s4);

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
	int c_mselect;
	int c_reverse_blend;
	int c_add_aclocal;
	int c_invert_output;
	int a_zero_aother;
	int a_sub_alocal;
	int a_mselect;
	int a_reverse_blend;
	int a_add_aclocal;
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

cbuffer cbFogTable : register(b3)
{
	float4 fog_table[64]; // 8.0 fog alpha, 6.0 fog delta alpha, 1.0 fog delta alpha invert (2nd lsb of 6.2 fog delta alpha), pad
};

cbuffer cbFogCtrl : register(b4)
{
	float3 fogColor;
	int fog_enable;
	int fogadd;
	int fogmult;
	int fogza;
	int fogconstant;
	int fogdither;
	int fogzones;
	float pad30, pad31;
};

//--------------------------------------------------------------------------------------
// Function declarations
//--------------------------------------------------------------------------------------
float4 SelectOther(float4 iterColor, float4 texColor);
float4 SelectLocal(float4 iterColor, float4 texColor, float4 iterPos);
float4 Dither(float4 Pos, float4 color);
float4 ColorCombine(float4 c_other, float4 c_local, Combine_Struct ctrl, float msel4, float3 msel5);
float CalcDepth(float oow);
void AlphaTest(float srcAlpha, float refAlpha, int alphaOp);
float4 FogUnit(float4 srcColor, float fogDepth, float4 inputPos);

//--------------------------------------------------------------------------------------
// Stage 0 Pixel Shader
//--------------------------------------------------------------------------------------
struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	noperspective float Oow : FOG;
	float4 Col : COLOR;
	float4 TexPos0 : TEXCOORD0;
	float4 TexPos1 : TEXCOORD1;
};

struct PS_OUTPUT
{
	float4 Col : SV_TARGET;
	float4 ColBeforeFog : SV_TARGET1;
	float Depth : SV_DEPTH;
};

PS_OUTPUT PS(PS_INPUT input)
{
	PS_OUTPUT output;
	// Reduce the input to 8 bit color
	float4 inColor = floor(input.Col * 255.0f) / 255.0f;
	
	float4 texColor = 0.0f;
	float lod = 0;
	// Tex1
	if (texCtrl[1].enable) {
		if (1 || texCtrl[1].send_config == 0) {
			// Not sure if this should be clamped
			lod = texTexture1.CalculateLevelOfDetailUnclamped(texState1, input.TexPos1.xy);
			if (texCtrl[1].tloddither)
				lod += dither2x2Texture.Sample(ditherState, input.Pos.xy / 4.0f).x / 16.0f;
			texColor.bgra = texTexture1.SampleLevel(texState1, input.TexPos1.xy, lod);
			float msel4, msel5;
			msel4 = (texCtrl[1].detailBias - lod) * texCtrl[1].detailScale;  // detail_factor (voodoo 1 doc)
			msel4 = clamp(msel4, 0.0f, texCtrl[1].detailMax);
			msel4 /= 255.0f;
			msel5 = frac(lod); // lod fraction
			texColor = ColorCombine(0.0f, texColor, texCombine[1], msel4, msel5);
			// Floor texture color to 8 bits
			texColor = floor(texColor * 255.0f) / 255.0f;
		} else {
			texColor.r = ((texCtrl[1].send_config >> 16)& 0xff) / 255.0f;
			texColor.g = ((texCtrl[1].send_config >> 8) & 0xff) / 255.0f;
			texColor.b = (texCtrl[1].send_config & 0xff) / 255.0f;
			texColor.a = 1.0f;
		}
	}
	// Tex0
	if (texCtrl[0].enable) {
		if (texCtrl[0].send_config == 0) {
			float4 texColor0;
			// Not sure if this should be clamped
			lod = texTexture0.CalculateLevelOfDetailUnclamped(texState0, input.TexPos0.xy);
			if (texCtrl[0].tloddither)
				lod += dither2x2Texture.Sample(ditherState, input.Pos.xy / 4.0f).x / 16.0f;
			texColor0.bgra = texTexture0.SampleLevel(texState0, input.TexPos0.xy, lod);
			float msel4, msel5;
			msel4 = (texCtrl[0].detailBias - lod) * texCtrl[0].detailScale;  // detail_factor (voodoo 1 doc)
			msel4 = clamp(msel4, 0.0f, texCtrl[0].detailMax);
			msel4 /= 255.0f;
			msel5 = frac(lod); // lod fraction
			texColor = ColorCombine(texColor, texColor0, texCombine[0], msel4, msel5);
			// Floor texture color to 8 bits
			texColor = floor(texColor * 255.0f) / 255.0f;
		} else {
			texColor.r = ((texCtrl[0].send_config >> 16)& 0xff) / 255.0f;
			texColor.g = ((texCtrl[0].send_config >> 8) & 0xff) / 255.0f;
			texColor.b = (texCtrl[0].send_config & 0xff) / 255.0f;
			texColor.a = 1.0f;
		}
	}

	float4 srcColor;
	if (pixel_mode == 0) {
		// Select c_other
		float4 c_other = SelectOther(input.Col, texColor);

		// Select c_local
		float4 c_local = SelectLocal(input.Col, texColor, input.Pos);

		// Color combining
		srcColor = ColorCombine(c_other, c_local, colCtrl, texColor.a, texColor.rgb);
	}
	else {
		// Pixel pipeline LFB write
		srcColor = input.Col;
	}

	// Alpha Testing
	if (alpha_test_enable) {
		AlphaTest(srcColor.a, alpha_ref, alpha_op);
	}

	// Depth Calculation
	float depth = 1.0f;
	float fogDepth;
	if (pixel_mode) {
		// Use w directly with no biasing
		fogDepth = input.Oow;
		depth = fogDepth;
	} else {
		fogDepth = CalcDepth(input.Oow);
		if (depth_enable) {
			if (depth_src_select) {
				depth = zSrc;
			} else {
				if (wfloat_select) {
					if (depthfloat_select == 0)
						depth = fogDepth;
					else
						depth = CalcDepth(input.Pos.z / 256.0f);  // 12.20 -> 4.28  TODO Check
				} else {
					depth = input.Pos.z / 32.0f;  // TODO Check
					if (zbias_enable)
						depth += zBias;
					saturate(depth);
				}
			}
		}
	}

	// Some blending modes use color before fog
	output.ColBeforeFog = srcColor;

	// Fog
	srcColor = FogUnit(srcColor, fogDepth, input.Pos);

	output.Col = srcColor;
	output.Depth = depth;
	return output;
}
//--------------------------------------------------------------------------------------
// COMP Pixel Shader
//--------------------------------------------------------------------------------------
struct COMP_PS_INPUT
{
	float4 Pos : SV_POSITION;
	noperspective float Oow : FOG;
	float2 TexPos : TEXCOORD;
};

struct COMP_PS_OUTPUT
{
	float4 Col : SV_TARGET;
};

COMP_PS_OUTPUT COMP_PS(COMP_PS_INPUT input)
{
	COMP_PS_OUTPUT output;

	float4 srcColor = renderTexture.Sample(renderState, input.TexPos.xy);

	// Dithering
	srcColor = Dither(input.Pos, srcColor * 255.0f);

	// Reduce bits for RGB565 format compression
	srcColor.rb = floor(srcColor.rb / 8.0f) * 8.0f;
	srcColor.g = floor(srcColor.g / 4.0f) * 4.0f;
	srcColor /= 255.0;

	output.Col = srcColor;

	return output;
}

//--------------------------------------------------------------------------------------
// FAST Fill Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT FASTFILL_PS(PS_INPUT input)
{
	PS_OUTPUT output;

	output.Col = input.Col;
	output.Depth = input.Oow;

	return output;
}
//--------------------------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------------------------
float4 SelectOther(float4 iterColor, float4 texColor)
{
	float4 c_other;
	// Select c_other rgb
	switch (rgbselect) {
	case 0:
		c_other.rgb = iterColor.rgb;  // Is saturation needed???
		break;
	case 1:
		c_other.rgb = texColor.rgb;
		break;
	case 2:
		c_other.rgb = color1.rgb;
		break;
	case 3:
	default:
		c_other.rgb = 0.0f; // Linear Frame Buffer voodoo > 3
		break;
	}
	// Select c_other a
	switch (aselect) {
	case 0:
		c_other.a = iterColor.a; // Is saturation needed???
		break;
	case 1:
		c_other.a = texColor.a;
		break;
	case 2:
		c_other.a = color1.a;
		break;
	case 3:
	default:
		c_other.a = 0.0f; // Linear Frame Buffer voodoo > 3
		break;
	}
	return c_other;
}

float4 SelectLocal(float4 iterColor, float4 texColor, float4 iterPos)
{
	float4 c_local;
	// Select c_local rgb

	// TODO: Voodoo 1 doc says bit 7 of texture alpha, other docs say bit 0
	int select = c_localselect_override ? (texColor.a >= 128.0f / 255.0f ? 1 : 0) : c_localselect;
	if (select == 0)
		c_local.rgb = iterColor.rgb; // Need to check saturation???
	else
		c_local.rgb = color0.rgb;

	// Select c_local alpha
	switch (a_localselect) {
	case 0:
		c_local.a = iterColor.a; // Is saturation needed???
		break;
	case 1:
		c_local.a = color0.a;
		break;
	case 2:
		c_local.a = saturate(iterPos.z); // interated Z[27:20], clamped  TODO
		break;
	case 3:
	default:
		c_local.a = saturate(iterPos.w); // iterated W[29:32], clamped TODO
		break;
	}
	return c_local;
}

float4 Dither(float4 Pos, float4 color)
{
	float4 outColor;
	float dither;
	if (dither_sel == 0) {
		return color;
	} else if (dither_sel == 1) {
		// 4x4 dithering
		dither = dither4x4Texture.Sample(ditherState, Pos.xy / 4.0f).x * 255.0f;
	} else {
		// 2x2 dithering
		dither = dither2x2Texture.Sample(ditherState, Pos.xy / 4.0f).x * 255.0f;
	}

	outColor.rb = floor((color.rb * 2.0f - floor(color.rb / 16.0f) + floor(color.rb / 128.0f) + dither) / 2.0f);
	outColor.g = floor((color.g * 4.0f - floor(color.g / 16.0f) + floor(color.g / 64.0f) + dither) / 4.0f);
	outColor.a = color.a;

	return outColor;
}

float4 ColorCombine(float4 c_other, float4 c_local, Combine_Struct ctrl, float msel4, float3 msel5)
{
	float4 dst = c_other;

	if (ctrl.c_zero_cother)
		dst.rgb = 0.0f;

	if (ctrl.a_zero_aother)
		dst.a = 0.0f;

	if (ctrl.c_sub_clocal)
		dst.rgb -= c_local.rgb;

	if (ctrl.a_sub_alocal)
		dst.a -= c_local.a;

	float4 blendFactor;
	switch (ctrl.c_mselect) {
	case 0:
		blendFactor.rgb = 0.0f;
		break;
	case 1:
		blendFactor.rgb = c_local.rgb;
		break;
	case 2:
		blendFactor.rgb = c_other.a;
		break;
	case 3:
		blendFactor.rgb = c_local.a;
		break;
	case 4:
		blendFactor.rgb = msel4;
		break;
	case 5:
	default:
		blendFactor.rgb = msel5;
		break;
	}

	switch (ctrl.a_mselect) {
	case 0:
		blendFactor.a = 0.0f;
		break;
	case 1:
		blendFactor.a = c_local.a;
		break;
	case 2:
		blendFactor.a = c_other.a;
		break;
	case 3:
		blendFactor.a = c_local.a;
		break;
	case 4:
	default:
		blendFactor.a = msel4;
		break;
	}

	if (ctrl.c_reverse_blend==0)
		blendFactor.rgb = 1.0f - blendFactor.rgb;

	if (ctrl.a_reverse_blend==0)
		blendFactor.a = 1.0f - blendFactor.a;

	// Perform the blend
	dst *= blendFactor;

	// truncate  TODO: check this
	dst = floor(dst * 255.0f) / 255.0f;

	switch (ctrl.c_add_aclocal) {
	case 0:
		break;
	case 1:
		dst.rgb += c_local.rgb;
		break;
	case 2:
	default:
		dst.rgb += c_local.a;
		break;
	}

	switch (ctrl.a_add_aclocal) {
	case 0:
		break;
	case 1:
	case 2:
	default:
		dst.a += c_local.a;
		break;
	}

	dst = saturate(dst);

	if (ctrl.c_invert_output)
		dst.rgb = 1.0f - dst.rgb;

	if (ctrl.a_invert_output)
		dst.a = 1.0f - dst.a;

	return dst;
}

float CalcDepth(float oow)
{
	float depth;
	// Top 16 of 16.32 set
	if (oow >= 1.0f) {
		depth = -1.0f;
	}
	// Middle 16 of 16.32 not set
	else if (oow < 1.0f / 65536.0f) {
		depth = 1.0f;
	} else {
		uint temp = oow * 65536.0f * 65536.0f;
		uint exp = firstbithigh(temp) ^ 31; // 31 - firstbithigh
		temp >>= 20 - (exp + 1);
		temp &= 0xfff;
		// Invert
		temp ^= 0xfff;
		temp |= (exp << 12);
		// Add one
		temp++;
		depth = float(temp);
		// Normalize to 16 bits (0xffff = 1.0)
		depth /= 65535.0f;
	}
	// Add bias
	if (zbias_enable)
		depth += zBias;

	// Need to clamp after bias adjust
	saturate(depth);

	return depth;
}

void AlphaTest(float srcAlpha, float refAlpha, int alphaOp)
{
	switch (alphaOp) {
	case 0: // never
		discard;
		break;
	case 1: // less than
		if (srcAlpha >= refAlpha)
			discard;
		break;
	case 2: // equal
		if (srcAlpha != refAlpha)
			discard;
		break;
	case 3: // less than or equal
		if (srcAlpha > refAlpha)
			discard;
		break;
	case 4: // greater than
		if (srcAlpha <= refAlpha)
			discard;
		break;
	case 5: // not equal
		if (srcAlpha == refAlpha)
			discard;
		break;
	case 6: // greater than or equal
		if (srcAlpha < refAlpha)
			discard;
		break;
	case 7: // always
	default:
		break;
	}
}

float4 FogUnit(float4 srcColor, float fogDepth, float4 inputPos)
{
	if (fog_enable == 0) {
		return srcColor;
	} else {
		float4 foggedColor = srcColor;
		
		if (fogmult)
			foggedColor.rgb = 0.0f;
		
		if (fogconstant) {
			foggedColor.rgb += fogColor.rgb;
		} else {
			float3 fogMult = -foggedColor.rgb;  // TODO: Check 2's complement
			if (fogadd == 0)
				fogMult += fogColor;
			// Fog Alpha
			float fogAlpha;
			switch (fogza) {
			case 0:
				fogDepth *= 65535.0f; // Un-normalize to 16 bits
				float fogExp = floor(fogDepth / 1024.0f); // 4 bits exp, 2 bit mant [11:10]
				float fogMant = floor(fogDepth / 4.0f) - fogExp * 256.0f; // 8 bit mant [9:2]
				fogAlpha = fog_table[fogExp][0];
				float fogDelta = fog_table[fogExp][1]; // 6.0
				float fogSel = fog_table[fogExp][2]; // 1 bit from 2nd lsb of fog delta alpha
				fogDelta *= fogMant;
				// Reduce 14 to 10 bits
				fogDelta = floor(fogDelta / 16.0f);
				if (fogzones == 1 && fogSel == 1.0f)
					fogDelta = -fogDelta;
				// Convert to 7.4
				fogDelta /= 16.0f;
				// Add dither
				if (fogdither)
					fogDelta += dither4x4Texture.Sample(ditherState, inputPos.xy / 4.0f).x * 255.0f / 16.0f;
				fogAlpha += fogDelta;
				fogAlpha = floor(fogAlpha);
				fogAlpha /= 255.0f; // Normalize
				break;
			case 1:
				fogAlpha = srcColor.a;
				break;
			case 2:
				float iterZClamp = inputPos.z;
				fogAlpha = iterZClamp; // iterated Z (27:20), clamped   TODO: fix
				break;
			case 3:
			default:
				float iterWClamp = inputPos.w;
				fogAlpha = iterWClamp; // iterated W (39:32), clamped  TODO: fix
				break;
			}
			foggedColor.rgb += fogMult * fogAlpha;
		}
		saturate(foggedColor);
		return foggedColor;
	}
}
