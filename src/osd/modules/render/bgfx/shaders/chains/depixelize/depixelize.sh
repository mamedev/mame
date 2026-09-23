// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  depixelize.sh - shared definitions of the depixelize chain
//
//  Kopf & Lischinski, "Depixelizing Pixel Art" (2011), GPU
//  implementation by Felix Kreuzer (2014/2015):
//  https://github.com/falichs/Depixelizing-Pixel-Art-on-GPUs
//
//  Every pass of the chain is a fragment shader with one output.
//  Texels are addressed by integer index derived from the quad
//  texture coordinate and read back with normalized coordinates,
//  so the passes are independent of the framebuffer origin of the
//  backend. All integer data lives in float render targets.
//
//  Data layout (W x H = emulated screen size, from u_source_dims):
//    source   W x H          BGRA8   copy of the screen
//    graph    2W x 2H        BGRA8   similarity graph, see fs_similarity.sc
//    knots    2W x 2H        RGBA32F one texel per B-spline control point (knot):
//                                    knot k = 2*(cellY*(W-1)+cellX)+sector is texel
//                                    (2*cellX+sector, cellY), i.e. k = y*2(W-1)+x
//    segments 3W x 3H        RGBA32F 7 records per cell in a 3x3 block, see fs_cell_segments.sc
//  Targets may be larger than needed (user prescale); the logical
//  sizes above come from u_source_dims, the physical sizes from the
//  u_tex_sizeN / u_target_dims uniforms.
//
//  Flags and indices are exact integers stored as floats; bit tests
//  use hasFlag() so the shaders also compile for GLSL 1.20 and
//  Direct3D 9, which have no integer bit operations.
//
//============================================================

#include "common.sh"

// file-scope variables shared between functions: HLSL (and the Metal and
// SPIR-V paths, which go through the HLSL front end) treat plain globals as
// uniforms, GLSL has no static
#if BGFX_SHADER_LANGUAGE_GLSL
#define GLOBAL
#else
#define GLOBAL static
#endif

// similarity graph edge values (fs_similarity.sc)
#define EDGE_HORVERT        16.0
#define EDGE_DIAGONAL_ULLR  32.0
#define EDGE_DIAGONAL_LLUR  64.0
#define EDGE_CROSSING       96.0

// similarity graph node connectivity bits (fs_valence.sc)
#define NODE_NORTH      128.0
#define NODE_NORTHEAST   64.0
#define NODE_EAST        32.0
#define NODE_SOUTHEAST   16.0
#define NODE_SOUTH        8.0
#define NODE_SOUTHWEST    4.0
#define NODE_WEST         2.0
#define NODE_NORTHWEST    1.0

// knot flags (fs_construct.sh)
#define HAS_NORTHERN_NEIGHBOR      1.0
#define HAS_EASTERN_NEIGHBOR       2.0
#define HAS_SOUTHERN_NEIGHBOR      4.0
#define HAS_WESTERN_NEIGHBOR       8.0
#define HAS_NORTHERN_SPLINE       16.0
#define HAS_EASTERN_SPLINE        32.0
#define HAS_SOUTHERN_SPLINE       64.0
#define HAS_WESTERN_SPLINE       128.0
#define HAS_CORRECTED_POSITION   256.0
#define DONT_OPTIMIZE_N          512.0
#define DONT_OPTIMIZE_E         1024.0
#define DONT_OPTIMIZE_S         2048.0
#define DONT_OPTIMIZE_W         4096.0

// knot directions
#define DIR_NORTH   1.0
#define DIR_EAST    2.0
#define DIR_SOUTH   4.0
#define DIR_WEST    8.0

// true if the power-of-two bit is set in the non-negative integer flags
bool hasFlag(float flags, float bit)
{
	return mod(floor(flags / bit), 2.0) >= 1.0;
}

// an 8-bit normalized channel back to its integer value
float decode255(float channel)
{
	return floor(channel * 255.0 + 0.5);
}

// integer texel index of this fragment in a target of the given size
vec2 fragmentTexel(vec2 texcoord, vec2 targetDims)
{
	return floor(texcoord * targetDims);
}

// normalized coordinate of the center of an integer texel
vec2 texelCenter(vec2 texel, vec2 texSize)
{
	return (texel + 0.5) / texSize;
}

// the texel of a knot index in the knot textures (see the layout above)
vec2 knotTexel(float knot, float knotsPerRow)
{
	float row = floor((knot + 0.5) / knotsPerRow);
	return vec2(knot - row * knotsPerRow, row);
}
