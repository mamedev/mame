$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_cell_segments.sc - per-cell spline segment lists
//
//  Port of GatherCellSegments.frag. The rasterizer has to know
//  which spline pieces run through a cell to decide which of the
//  four surrounding pixels may contribute to an output pixel; this
//  pass resolves them once per cell.
//
//  The splines around a knot come in two shapes, called elements:
//    END     - a spline starting at the knot:   points (pn, p1, p2)
//              pieces (pn, pn, p1), (pn, p1, p2)
//    THROUGH - a spline passing the knot:       points (pm1, pn, p1, pm2, p2)
//              pieces (pm1, pn, p1), (pn, pm1, pm2), (pn, p1, p2)
//  A cell holds at most 4 elements / 12 points: a knot of valence
//  1 or 2 gives one element, valence 3 a THROUGH plus the END of
//  the T-junction base, valence 4 four ENDs; the second knot of a
//  diagonal cell adds its own element.
//
//  Output: 7 records per cell in the 3x3 texel block at
//  (3*cellX, 3*cellY), record r at (3*cellX + r mod 3, 3*cellY + r div 3):
//    rec0 = (element count, type bits, 0, 0), bit i set for a THROUGH element
//    rec1..rec6 = the points of all elements back to back, two per record
//  Every fragment resolves its whole cell and writes the record its
//  position selects; the control point indices come precomputed
//  from fs_knot_cps.sc so the repetition stays cheap.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_positions, 0);
SAMPLER2D(s_flags, 1);
SAMPLER2D(s_cps, 2);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

#define MAX_POINTS 12

GLOBAL float knotsPerRow;
GLOBAL vec2 pts[MAX_POINTS];
GLOBAL float elementCount;
GLOBAL float elementTypes;

vec2 knotUV(float knot)
{
	return texelCenter(knotTexel(knot, knotsPerRow), u_tex_size0.xy);
}
vec2 position(float knot)
{
	if (knot < 0.0) return vec2(0.0, 0.0);
	return texture2DLod(s_positions, knotUV(knot), 0.0).xy;
}
float fetchFlags(float knot)
{
	if (knot < 0.0) return 0.0;
	return texture2DLod(s_flags, knotUV(knot), 0.0).x;
}
vec4 fetchControlPoints(float knot)
{
	return texture2DLod(s_cps, knotUV(knot), 0.0);
}

float computeValence(float flags)
{
	float valence = 0.0;
	if (hasFlag(flags, HAS_NORTHERN_NEIGHBOR)) valence += 1.0;
	if (hasFlag(flags, HAS_EASTERN_NEIGHBOR))  valence += 1.0;
	if (hasFlag(flags, HAS_SOUTHERN_NEIGHBOR)) valence += 1.0;
	if (hasFlag(flags, HAS_WESTERN_NEIGHBOR))  valence += 1.0;
	return valence;
}

// the type bit of the element about to be added
float elementBit()
{
	if (elementCount == 0.0) return 1.0;
	if (elementCount == 1.0) return 2.0;
	if (elementCount == 2.0) return 4.0;
	return 8.0;
}

//----------------------------------------------------------------
// a spline that starts at knot 'node' and leaves through control points cp:
// the end piece (node, node, cp.x) plus the continuation (node, cp.x, cp.y).
// 'slot' must be a compile-time constant at every call site.
//----------------------------------------------------------------
#define addSplineEnd(slot, node, cp) \
	pts[slot] = position(node); \
	pts[slot + 1] = position((cp).x); \
	pts[slot + 2] = ((cp).y > -1.0) ? position((cp).y) : pts[slot + 1]; \
	elementCount += 1.0

//----------------------------------------------------------------
// a spline that passes through knot 'node' with control points cp.xy on one side
// and cp.zw on the other. 'slot' must be a compile-time constant at every call site.
//----------------------------------------------------------------
#define addSplineThrough(slot, node, cp) \
	pts[slot] = position((cp).x); \
	pts[slot + 1] = position(node); \
	pts[slot + 2] = position((cp).z); \
	pts[slot + 3] = ((cp).y > -1.0) ? position((cp).y) : pts[slot]; \
	pts[slot + 4] = ((cp).w > -1.0) ? position((cp).w) : pts[slot + 2]; \
	elementTypes += elementBit(); \
	elementCount += 1.0

void main()
{
	knotsPerRow = 2.0 * (u_source_dims.x - 1.0);
	vec2 texel = fragmentTexel(v_texcoord0, u_target_dims.xy);
	vec2 cell = floor(texel / 3.0);
	vec2 inBlock = texel - 3.0 * cell;
	float record = inBlock.y * 3.0 + inBlock.x;
	if (record >= 7.0 || cell.x >= u_source_dims.x - 1.0 || cell.y >= u_source_dims.y - 1.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	// each cell owns two knots
	float node0 = 2.0 * (cell.y * (u_source_dims.x - 1.0) + cell.x);
	float node1 = node0 + 1.0;

	elementCount = 0.0;
	elementTypes = 0.0;
	for (int i = 0; i < MAX_POINTS; i++) pts[i] = vec2(0.0, 0.0);

	float node0flags = fetchFlags(node0);
	bool hasCorrectedPosition = false;
	float node0points = 0.0; /**< points used by the first knot's elements */

	if (node0flags > 0.0) {
		vec4 cp0 = fetchControlPoints(node0);
		float node0valence = computeValence(node0flags);
		if (node0valence == 1.0) {
			addSplineEnd(0, node0, cp0.xy);
			node0points = 3.0;
		} else if (node0valence == 2.0) {
			addSplineThrough(0, node0, cp0);
			node0points = 5.0;
		} else if (node0valence == 3.0) {
			hasCorrectedPosition = true;
			// two of the three neighbors form a spline through the knot, the third is
			// the base of a T-junction that starts at the knot's corrected position (node1)
			addSplineThrough(0, node0, cp0);
			//T-Base
			vec2 tcp = fetchControlPoints(node1).xy;
			addSplineEnd(5, node1, tcp);
			node0points = 8.0;
		} else { // valence 4
			vec4 cp1 = fetchControlPoints(node1);
			addSplineEnd(0, node0, cp0.xy);
			addSplineEnd(3, node0, cp0.zw);
			addSplineEnd(6, node0, cp1.xy);
			addSplineEnd(9, node0, cp1.zw);
			node0points = 12.0;
		}
	}
	if (!hasCorrectedPosition && node0points <= 5.0) {
		float node1flags = fetchFlags(node1);
		if (node1flags > 0.0) {
			vec4 cp1 = fetchControlPoints(node1);
			float node1valence = computeValence(node1flags);
			// the second knot's points follow the first knot's; node0points is 0, 3 or 5 here,
			// spelled out so every point slot stays a compile-time constant
			if (node1valence == 1.0) {
				vec2 cp = cp1.xy;
				if (node0points == 0.0) { addSplineEnd(0, node1, cp); }
				else if (node0points == 3.0) { addSplineEnd(3, node1, cp); }
				else { addSplineEnd(5, node1, cp); }
			} else if (node1valence == 2.0) {
				if (node0points == 0.0) { addSplineThrough(0, node1, cp1); }
				else if (node0points == 3.0) { addSplineThrough(3, node1, cp1); }
				else { addSplineThrough(5, node1, cp1); }
			}
		}
	}

	if (record == 0.0) {
		gl_FragColor = vec4(elementCount, elementTypes, 0.0, 0.0);
	} else {
		// record r holds points 2r-2 and 2r-1; spelled out so the array indices stay constant
		if (record == 1.0) gl_FragColor = vec4(pts[0], pts[1]);
		else if (record == 2.0) gl_FragColor = vec4(pts[2], pts[3]);
		else if (record == 3.0) gl_FragColor = vec4(pts[4], pts[5]);
		else if (record == 4.0) gl_FragColor = vec4(pts[6], pts[7]);
		else if (record == 5.0) gl_FragColor = vec4(pts[8], pts[9]);
		else gl_FragColor = vec4(pts[10], pts[11]);
	}
}
