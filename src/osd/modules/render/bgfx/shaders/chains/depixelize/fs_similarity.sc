$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_similarity.sc - initial similarity graph
//
//  Port of dissimilar.frag. One fragment per graph texel: the
//  graph is a 2W x 2H grid where texel (2x+1, 2y+1) is the node of
//  pixel (x, y) and the texels between nodes hold edge indicators:
//    E E E E
//    E X E X      X ... node (this pass stores the pixel color)
//    E E E E      E ... edge: EDGE_HORVERT between orthogonal
//    E X E X            neighbors, EDGE_DIAGONAL_* / EDGE_CROSSING
//                       at the center of a 2x2 pixel block
//  Two pixels are connected when their YUV distance is small.
//  Row and column 0 are a border and stay 0.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_source, 0);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

vec4 fetchPixel(vec2 pixel)
{
	return texture2DLod(s_source, texelCenter(pixel, u_tex_size0.xy), 0.0);
}

// source pixel of a graph texel
vec2 pixelOfGraph(vec2 graph)
{
	return floor((graph - 1.0) / 2.0);
}

bool isSimilar(vec4 pixelA, vec4 pixelB)
{
	//Y = 0.299*R + 0.587*G + 0.114*B
	//U = (B-Y)*0.493
	//V = (R-Y)*0.877
	float yA = 0.299 * pixelA.r + 0.587 * pixelA.g + 0.114 * pixelA.b;
	float uA = 0.493 * (pixelA.b - yA);
	float vA = 0.877 * (pixelA.r - yA);
	float yB = 0.299 * pixelB.r + 0.587 * pixelB.g + 0.114 * pixelB.b;
	float uB = 0.493 * (pixelB.b - yB);
	float vB = 0.877 * (pixelB.r - yB);

	bool similar = false;
	if (abs(yA - yB) <= 48.0 / 255.0)
	{
		if (abs(uA - uB) <= 7.0 / 255.0)
		{
			if (abs(vA - vB) <= 6.0 / 255.0)
			{
				similar = true;
			}
		}
	}
	return similar;
}

void main()
{
	vec2 g = fragmentTexel(v_texcoord0, u_target_dims.xy);
	vec2 dims = u_source_dims.xy;

	if (g.x < 1.0 || g.x >= 2.0 * dims.x || g.y < 1.0 || g.y >= 2.0 * dims.y)
	{
		//border hit
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	//evalPos evaluation window position
	// UL UR
	// LL LR
	vec2 evalPos = mod(g, 2.0);
	vec4 value = vec4(0.0, 0.0, 0.0, 0.0);
	if (evalPos.x == 1.0 && evalPos.y == 1.0)
	{
		//node: store the pixel color, the valence pass overwrites this
		value = fetchPixel(pixelOfGraph(g));
	}
	else if (evalPos.x == 0.0 && evalPos.y == 0.0)
	{
		//UL ... diagonal
		float diagonal = 0.0;
		//check UL-LR connection
		vec4 pA = fetchPixel(pixelOfGraph(g + vec2(-1.0,  1.0)));
		vec4 pB = fetchPixel(pixelOfGraph(g + vec2( 1.0, -1.0)));
		if (isSimilar(pA, pB))
		{
			diagonal = EDGE_DIAGONAL_ULLR;
		}
		//check LL-UR connection
		pA = fetchPixel(pixelOfGraph(g + vec2(-1.0, -1.0)));
		pB = fetchPixel(pixelOfGraph(g + vec2( 1.0,  1.0)));
		if (isSimilar(pA, pB))
		{
			diagonal += EDGE_DIAGONAL_LLUR;
		}
		value = vec4(diagonal / 255.0, 0.0, 0.0, 0.0);
	}
	else if (evalPos.x == 0.0)
	{
		//LL ... horizontal edge between the pixels left and right of it
		vec4 pA = fetchPixel(pixelOfGraph(g + vec2(-1.0, 0.0)));
		vec4 pB = fetchPixel(pixelOfGraph(g + vec2( 1.0, 0.0)));
		if (isSimilar(pA, pB))
		{
			value = vec4(EDGE_HORVERT / 255.0, 0.0, 0.0, 0.0);
		}
	}
	else
	{
		//UR ... vertical edge between the pixels below and above it
		vec4 pA = fetchPixel(pixelOfGraph(g + vec2(0.0, -1.0)));
		vec4 pB = fetchPixel(pixelOfGraph(g + vec2(0.0,  1.0)));
		if (isSimilar(pA, pB))
		{
			value = vec4(EDGE_HORVERT / 255.0, 0.0, 0.0, 0.0);
		}
	}
	gl_FragColor = value;
}
