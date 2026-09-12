$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_valence.sc - similarity graph node valences
//
//  Port of valenceUpdate.frag. Replaces every node texel of the
//  graph by (valence, connectivity bits) / 255 where the bits are
//  the NODE_* directions of the connected neighbors; edge texels
//  are copied. Runs after fs_similarity.sc and again after
//  fs_crossings.sc.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_graph, 0);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

// graph texel, zero outside the logical graph (the border)
vec4 fetchGraph(vec2 g)
{
	if (g.x < 0.0 || g.y < 0.0 || g.x >= 2.0 * u_source_dims.x || g.y >= 2.0 * u_source_dims.y)
		return vec4(0.0, 0.0, 0.0, 0.0);
	return texture2DLod(s_graph, texelCenter(g, u_tex_size0.xy), 0.0);
}

float edgeValue(vec2 g)
{
	return decode255(fetchGraph(g).x);
}

void main()
{
	vec2 g = fragmentTexel(v_texcoord0, u_target_dims.xy);

	if (g.x < 1.0 || g.x >= 2.0 * u_source_dims.x || g.y < 1.0 || g.y >= 2.0 * u_source_dims.y)
	{
		//border hit
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	vec2 evalPos = mod(g, 2.0);
	if (evalPos.x == 1.0 && evalPos.y == 1.0)
	{
		//calculate node valence
		float valence = 0.0;
		float edges = 0.0;
		//browse neighborhood
		//NW
		if (hasFlag(edgeValue(g + vec2(-1.0, 1.0)), EDGE_DIAGONAL_ULLR)) { valence += 1.0; edges += NODE_NORTHWEST; }
		//N
		if (edgeValue(g + vec2(0.0, 1.0)) > 0.0) { valence += 1.0; edges += NODE_NORTH; }
		//NE
		if (hasFlag(edgeValue(g + vec2(1.0, 1.0)), EDGE_DIAGONAL_LLUR)) { valence += 1.0; edges += NODE_NORTHEAST; }
		//E
		if (edgeValue(g + vec2(1.0, 0.0)) > 0.0) { valence += 1.0; edges += NODE_EAST; }
		//SE
		if (hasFlag(edgeValue(g + vec2(1.0, -1.0)), EDGE_DIAGONAL_ULLR)) { valence += 1.0; edges += NODE_SOUTHEAST; }
		//S
		if (edgeValue(g + vec2(0.0, -1.0)) > 0.0) { valence += 1.0; edges += NODE_SOUTH; }
		//SW
		if (hasFlag(edgeValue(g + vec2(-1.0, -1.0)), EDGE_DIAGONAL_LLUR)) { valence += 1.0; edges += NODE_SOUTHWEST; }
		//W
		if (edgeValue(g + vec2(-1.0, 0.0)) > 0.0) { valence += 1.0; edges += NODE_WEST; }

		gl_FragColor = vec4(valence / 255.0, edges / 255.0, 0.0, 0.0);
	}
	else
	{
		//copy value
		gl_FragColor = fetchGraph(g);
	}
}
