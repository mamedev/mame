$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_knot_cps.sc - control point indices of every knot's splines
//
//  Port of GatherKnotControlPoints.frag. The spline pieces around
//  a knot are defined by knot indices that have to be chased
//  through several dependent flag and neighbor fetches (getCPs).
//  This pass does that once per knot so fs_cell_segments.sc only
//  has to turn indices into positions. A knot's texel holds:
//    first knot of a cell (sector 0):
//      valence 1: (cp.x, cp.y, -1, -1)      the END element's control points
//      valence 2: (cp.x, cp.y, cp.z, cp.w)  the THROUGH element's control points
//      valence 3: (cp.x, cp.y, cp.z, cp.w)  the THROUGH element; the T-junction base is in the second knot's texel
//      valence 4: (N.x, N.y, E.x, E.y)      two of the four END elements, the others in the second knot's texel
//    second knot of a cell (sector 1):
//      parent valence 3: (cp.x, cp.y, -1, -1)  the T-junction base, an END element starting at this (corrected) knot
//      parent valence 4: (S.x, S.y, W.x, W.y)
//      otherwise its own valence 1 / 2 as above (the second knot of a diagonal cell)
//  Anything else is (-1, -1, -1, -1).
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_flags, 0);
SAMPLER2D(s_neighbors, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

GLOBAL float knotsPerRow;

vec2 knotUV(float knot)
{
	return texelCenter(knotTexel(knot, knotsPerRow), u_tex_size0.xy);
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

float getNeighborIndex(float sourceIndex, float dir)
{
	vec4 n = fetchNeighbors(sourceIndex);
	if (dir == DIR_NORTH) return n.x;
	if (dir == DIR_EAST)  return n.y;
	if (dir == DIR_SOUTH) return n.z;
	return n.w;
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

//----------------------------------------------------------------
// finds the control points a spline continues with beyond the given neighbor:
// .x is the neighbor itself (or its corrected position), .y the point after it (-1 if none)
//----------------------------------------------------------------
vec2 getCPs(float node0neighborIndex, float dir)
{
	vec2 cpArray = vec2(node0neighborIndex, -1.0);
	float checkFwdX, checkFwdY, checkFwdZ, checkBack, chkdirX, chkdirY;
	if (dir == DIR_NORTH) {
		checkFwdX = HAS_NORTHERN_SPLINE; checkFwdY = HAS_EASTERN_SPLINE;  checkFwdZ = HAS_WESTERN_SPLINE;
		checkBack = HAS_SOUTHERN_SPLINE; chkdirX = DIR_EAST;  chkdirY = DIR_WEST;
	} else if (dir == DIR_EAST) {
		checkFwdX = HAS_EASTERN_SPLINE;  checkFwdY = HAS_SOUTHERN_SPLINE; checkFwdZ = HAS_NORTHERN_SPLINE;
		checkBack = HAS_WESTERN_SPLINE;  chkdirX = DIR_SOUTH; chkdirY = DIR_NORTH;
	} else if (dir == DIR_SOUTH) {
		checkFwdX = HAS_SOUTHERN_SPLINE; checkFwdY = HAS_WESTERN_SPLINE;  checkFwdZ = HAS_EASTERN_SPLINE;
		checkBack = HAS_NORTHERN_SPLINE; chkdirX = DIR_WEST;  chkdirY = DIR_EAST;
	} else {
		checkFwdX = HAS_WESTERN_SPLINE;  checkFwdY = HAS_NORTHERN_SPLINE; checkFwdZ = HAS_SOUTHERN_SPLINE;
		checkBack = HAS_EASTERN_SPLINE;  chkdirX = DIR_NORTH; chkdirY = DIR_SOUTH;
	}

	float node0neighborFlags = fetchFlags(node0neighborIndex);
	//check for t-junktion
	if (hasFlag(node0neighborFlags, checkBack)) {
		//the spline continues through the next control point
		//get next spline control point to compute segment extension
		float nextDir = -1.0;
		if (hasFlag(node0neighborFlags, checkFwdX))      nextDir = dir;
		else if (hasFlag(node0neighborFlags, checkFwdY)) nextDir = chkdirX;
		else if (hasFlag(node0neighborFlags, checkFwdZ)) nextDir = chkdirY;
		if (nextDir >= 0.0) {
			float neighborsNeighborIndex = getNeighborIndex(node0neighborIndex, nextDir);
			float neighborsNeighborflags = fetchFlags(neighborsNeighborIndex);
			if (hasFlag(neighborsNeighborflags, HAS_CORRECTED_POSITION)) {
				cpArray.y = neighborsNeighborIndex + 1.0;
			} else {
				cpArray.y = neighborsNeighborIndex;
			}
		}
	} else {
		if (hasFlag(node0neighborFlags, HAS_CORRECTED_POSITION)) {
			cpArray.x += 1.0;
		}
	}
	return cpArray;
}

//----------------------------------------------------------------
// control points on both sides of a valence-2 knot, in N,E,S,W order
//----------------------------------------------------------------
vec4 gatherValence2(float flags, vec4 neighbors)
{
	vec4 cpArray = vec4(-1.0, -1.0, -1.0, -1.0);
	bool foundFirst = false;
	if (hasFlag(flags, HAS_NORTHERN_NEIGHBOR)) {
		cpArray.xy = getCPs(neighbors.x, DIR_NORTH);
		foundFirst = true;
	}
	if (hasFlag(flags, HAS_EASTERN_NEIGHBOR)) {
		if (foundFirst) {
			cpArray.zw = getCPs(neighbors.y, DIR_EAST);
		} else {
			cpArray.xy = getCPs(neighbors.y, DIR_EAST);
			foundFirst = true;
		}
	}
	if (hasFlag(flags, HAS_SOUTHERN_NEIGHBOR)) {
		if (foundFirst) {
			cpArray.zw = getCPs(neighbors.z, DIR_SOUTH);
		} else {
			cpArray.xy = getCPs(neighbors.z, DIR_SOUTH);
			foundFirst = true;
		}
	}
	if (hasFlag(flags, HAS_WESTERN_NEIGHBOR)) {
		cpArray.zw = getCPs(neighbors.w, DIR_WEST);
	}
	return cpArray;
}

//----------------------------------------------------------------
// the single spline leaving a valence-1 knot
//----------------------------------------------------------------
vec2 gatherValence1(float flags, vec4 neighbors)
{
	if (hasFlag(flags, HAS_NORTHERN_NEIGHBOR)) return getCPs(neighbors.x, DIR_NORTH);
	if (hasFlag(flags, HAS_EASTERN_NEIGHBOR))  return getCPs(neighbors.y, DIR_EAST);
	if (hasFlag(flags, HAS_SOUTHERN_NEIGHBOR)) return getCPs(neighbors.z, DIR_SOUTH);
	if (hasFlag(flags, HAS_WESTERN_NEIGHBOR))  return getCPs(neighbors.w, DIR_WEST);
	return vec2(-1.0, -1.0);
}

void main()
{
	knotsPerRow = 2.0 * (u_source_dims.x - 1.0);
	vec2 texel = fragmentTexel(v_texcoord0, u_target_dims.xy);
	float sector = mod(texel.x, 2.0);
	float node0 = texel.y * knotsPerRow + texel.x - sector;
	float node1 = node0 + 1.0;
	vec4 cpOut = vec4(-1.0, -1.0, -1.0, -1.0);

	float node0flags = fetchFlags(node0);
	float node0valence = (node0flags > 0.0) ? computeValence(node0flags) : 0.0;

	if (sector == 0.0) {
		if (node0flags > 0.0) {
			vec4 node0neighbors = fetchNeighbors(node0);
			if (node0valence == 1.0) {
				cpOut.xy = gatherValence1(node0flags, node0neighbors);
			} else if (node0valence == 2.0) {
				cpOut = gatherValence2(node0flags, node0neighbors);
			} else if (node0valence == 3.0) {
				// two of the three neighbors form a spline through the knot, the third is
				// the base of a T-junction that starts at the knot's corrected position (node1)
				vec4 cpArray = vec4(-1.0, -1.0, -1.0, -1.0);
				bool foundFirst = false;
				if (hasFlag(node0flags, HAS_NORTHERN_NEIGHBOR) && hasFlag(node0flags, HAS_NORTHERN_SPLINE)) {
					cpArray.xy = getCPs(node0neighbors.x, DIR_NORTH);
					foundFirst = true;
				}
				if (hasFlag(node0flags, HAS_EASTERN_NEIGHBOR) && hasFlag(node0flags, HAS_EASTERN_SPLINE)) {
					if (foundFirst) {
						cpArray.zw = getCPs(node0neighbors.y, DIR_EAST);
					} else {
						cpArray.xy = getCPs(node0neighbors.y, DIR_EAST);
						foundFirst = true;
					}
				}
				if (hasFlag(node0flags, HAS_SOUTHERN_NEIGHBOR) && hasFlag(node0flags, HAS_SOUTHERN_SPLINE)) {
					if (foundFirst) {
						cpArray.zw = getCPs(node0neighbors.z, DIR_SOUTH);
					} else {
						cpArray.xy = getCPs(node0neighbors.z, DIR_SOUTH);
						foundFirst = true;
					}
				}
				if (hasFlag(node0flags, HAS_WESTERN_NEIGHBOR) && hasFlag(node0flags, HAS_WESTERN_SPLINE)) {
					cpArray.zw = getCPs(node0neighbors.w, DIR_WEST);
				}
				cpOut = cpArray;
			} else { // valence 4
				cpOut = vec4(getCPs(node0neighbors.x, DIR_NORTH), getCPs(node0neighbors.y, DIR_EAST));
			}
		}
	} else {
		if (node0flags > 0.0 && node0valence == 3.0) {
			// the T-junction base starts at this knot, the corrected position of node0
			vec4 node0neighbors = fetchNeighbors(node0);
			float tBaseDir = 0.0;
			float tBaseNeighborIndex = -1.0;
			if (hasFlag(node0flags, HAS_NORTHERN_NEIGHBOR) && !hasFlag(node0flags, HAS_NORTHERN_SPLINE)) {
				tBaseDir = DIR_NORTH;
				tBaseNeighborIndex = node0neighbors.x;
			}
			if (hasFlag(node0flags, HAS_EASTERN_NEIGHBOR) && !hasFlag(node0flags, HAS_EASTERN_SPLINE)) {
				tBaseDir = DIR_EAST;
				tBaseNeighborIndex = node0neighbors.y;
			}
			if (hasFlag(node0flags, HAS_SOUTHERN_NEIGHBOR) && !hasFlag(node0flags, HAS_SOUTHERN_SPLINE)) {
				tBaseDir = DIR_SOUTH;
				tBaseNeighborIndex = node0neighbors.z;
			}
			if (hasFlag(node0flags, HAS_WESTERN_NEIGHBOR) && !hasFlag(node0flags, HAS_WESTERN_SPLINE)) {
				tBaseDir = DIR_WEST;
				tBaseNeighborIndex = node0neighbors.w;
			}
			cpOut.xy = getCPs(tBaseNeighborIndex, tBaseDir);
		} else if (node0flags > 0.0 && node0valence == 4.0) {
			vec4 node0neighbors = fetchNeighbors(node0);
			cpOut = vec4(getCPs(node0neighbors.z, DIR_SOUTH), getCPs(node0neighbors.w, DIR_WEST));
		} else {
			// node0 has at most one element (valence <= 2), so node1 may carry its own
			float node1flags = fetchFlags(node1);
			if (node1flags > 0.0) {
				vec4 node1neighbors = fetchNeighbors(node1);
				float node1valence = computeValence(node1flags);
				if (node1valence == 1.0) {
					cpOut.xy = gatherValence1(node1flags, node1neighbors);
				} else if (node1valence == 2.0) {
					cpOut = gatherValence2(node1flags, node1neighbors);
				}
			}
		}
	}
	gl_FragColor = cpOut;
}
