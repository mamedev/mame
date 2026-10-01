$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_correct.sc - corrected T-junction positions
//
//  Port of UpdateCorrectedPositions.vert. After the optimization
//  moved the knots, the corrected position of every T-junction
//  (the second knot of its cell, flagged -1) is recomputed from
//  the optimized position of its parent knot (the first knot of
//  the cell) and the parent's two spline neighbors; all other
//  knots are copied.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_positions, 0);
SAMPLER2D(s_flags, 1);
SAMPLER2D(s_neighbors, 2);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

GLOBAL float knotsPerRow;

vec2 knotUV(float knot)
{
	return texelCenter(knotTexel(knot, knotsPerRow), u_tex_size0.xy);
}
vec2 fetchPosition(float knot)
{
	if (knot < 0.0) return vec2(0.0, 0.0);
	return texture2DLod(s_positions, knotUV(knot), 0.0).xy;
}
float fetchFlags(float knot)
{
	if (knot < 0.0) return 0.0;
	return texture2DLod(s_flags, knotUV(knot), 0.0).x;
}
vec4 fetchNeighbors(float knot)
{
	if (knot < 0.0) return vec4(-1.0, -1.0, -1.0, -1.0);
	return texture2DLod(s_neighbors, knotUV(knot), 0.0);
}

vec2 calcAdjustedPoint(vec2 p0, vec2 p1, vec2 p2)
{
	return 0.125 * p0 + 0.75 * p1 + 0.125 * p2;
}

void main()
{
	knotsPerRow = 2.0 * (u_source_dims.x - 1.0);
	vec2 texel = fragmentTexel(v_texcoord0, u_target_dims.xy);
	float knot = texel.y * knotsPerRow + texel.x;
	float flags = fetchFlags(knot);
	vec2 optimizedPos;

	if (flags == -1.0) {
		//get position, flags and neighborhood indices from parent vertex
		float id = knot - 1.0;
		vec2 parentPosition = fetchPosition(id);
		float parentFlags = fetchFlags(id);
		vec4 parentNeighborIndices = fetchNeighbors(id);
		vec2 p0 = vec2(0.0, 0.0);
		vec2 p1 = vec2(0.0, 0.0);

		float count = 0.0;
		if (hasFlag(parentFlags, HAS_NORTHERN_SPLINE)) {
			p0 = fetchPosition(parentNeighborIndices.x);
			count += 1.0;
		}
		if (hasFlag(parentFlags, HAS_EASTERN_SPLINE)) {
			if (count == 0.0) p0 = fetchPosition(parentNeighborIndices.y); else p1 = fetchPosition(parentNeighborIndices.y);
			count += 1.0;
		}
		if (hasFlag(parentFlags, HAS_SOUTHERN_SPLINE)) {
			if (count == 0.0) p0 = fetchPosition(parentNeighborIndices.z); else if (count == 1.0) p1 = fetchPosition(parentNeighborIndices.z);
			count += 1.0;
		}
		if (hasFlag(parentFlags, HAS_WESTERN_SPLINE)) {
			if (count == 0.0) p0 = fetchPosition(parentNeighborIndices.w); else if (count == 1.0) p1 = fetchPosition(parentNeighborIndices.w);
			count += 1.0;
		}
		if (count == 2.0) {
			optimizedPos = calcAdjustedPoint(p0, parentPosition, p1);
		} else {
			optimizedPos = parentPosition;
		}
	} else {
		optimizedPos = fetchPosition(knot);
	}
	gl_FragColor = vec4(optimizedPos, 0.0, 0.0);
}
