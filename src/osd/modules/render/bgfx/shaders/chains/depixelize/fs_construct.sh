// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_construct.sh - B-spline control point construction
//
//  Port of FullCellGraphConstruction.geom (as the fragment pass of
//  the reference implementation). Every 2x2 pixel block (cell) of
//  the similarity graph yields two knots: the block center, or two
//  points beside its diagonal edge, or for a T-junction the center
//  plus its corrected position. A knot records which of the four
//  neighboring cells it connects to (its NEIGHBOR bits and their
//  knot indices), which of those connections continue a smooth
//  spline (SPLINE bits) and whether a corner must stay sharp
//  (DONT_OPTIMIZE bits).
//
//  One fragment per knot texel computes the whole cell and writes
//  the attribute of its sector selected by the including file:
//    CP_OUTPUT_POSITION   (x, y, 0, 0) in pixel units
//    CP_OUTPUT_FLAGS      (flags, 0, 0, 0), -1 marks a corrected position
//    CP_OUTPUT_NEIGHBORS  (N, E, S, W) knot indices, -1 if none
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_graph, 0);
SAMPLER2D(s_source, 1);

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

//central offsets
#define XOFFSET_CUL -0.25
#define YOFFSET_CUL  0.25
#define XOFFSET_CUR  0.25
#define YOFFSET_CUR  0.25
#define XOFFSET_CLL -0.25
#define YOFFSET_CLL -0.25
#define XOFFSET_CLR  0.25
#define YOFFSET_CLR -0.25

GLOBAL vec2 cellPos; /**< the 2x2 cell this fragment belongs to */

// graph texel, zero outside the logical graph (the border)
float edgeValue(vec2 gc)
{
	if (gc.x < 0.0 || gc.y < 0.0 || gc.x >= 2.0 * u_source_dims.x || gc.y >= 2.0 * u_source_dims.y)
		return 0.0;
	return decode255(texture2DLod(s_graph, texelCenter(gc, u_tex_size0.xy), 0.0).x);
}

vec4 fetchPixel(vec2 pixel)
{
	return texture2DLod(s_source, texelCenter(pixel, u_tex_size1.xy), 0.0);
}

// true if the two pixels differ enough for a contour (used at T-junctions)
bool isContour(vec2 pixelL, vec2 pixelR)
{
	vec4 pL = fetchPixel(pixelL);
	vec4 pR = fetchPixel(pixelR);
	float yA = 0.299 * pL.r + 0.587 * pL.g + 0.114 * pL.b;
	float uA = 0.493 * (pL.b - yA);
	float vA = 0.877 * (pL.r - yA);
	float yB = 0.299 * pR.r + 0.587 * pR.g + 0.114 * pR.b;
	float uB = 0.493 * (pR.b - yB);
	float vB = 0.877 * (pR.r - yB);
	return distance(vec3(yA, uA, vA), vec3(yB, uB, vB)) > 100.0 / 255.0;
}

// knot index of the given sector of the neighboring cell in direction dir
float getNeighborIndex(float dir, float targetSector)
{
	float dy = u_source_dims.x - 1.0;
	vec2 c = cellPos;
	if (dir == DIR_NORTH)      c.y += 1.0;
	else if (dir == DIR_EAST)  c.x += 1.0;
	else if (dir == DIR_SOUTH) c.y -= 1.0;
	else if (dir == DIR_WEST)  c.x -= 1.0;
	return (c.y * dy + c.x) * 2.0 + targetSector;
}

vec2 calcAdjustedPoint(vec2 p0, vec2 p1, vec2 p2)
{
	return 0.125 * p0 + 0.75 * p1 + 0.125 * p2;
}

// decides whether two adjacent segments meet in a corner (kept sharp) or a curve
bool checkForCorner(vec2 spline1, vec2 spline2)
{
	//calculate inner angle
	float dp = dot(normalize(spline1), normalize(spline2));
	//there seems to be a numerical problem when using normalize & dot product
	//angles:
	// -0.7071
	// -0.3162
	// 0
	if (dp > -0.7072 && dp < -0.7070) {
		return true;
	} else if (dp > -0.3163 && dp < -0.3161) {
		return true;
	} else if (dp > -0.0001 && dp < 0.0001) {
		return true;
	}
	return false;
}

void main()
{
	vec2 texel = fragmentTexel(v_texcoord0, u_target_dims.xy);
	float sector = mod(texel.x, 2.0);
	cellPos = vec2(floor(texel.x / 2.0), texel.y);
	vec2 dims = u_source_dims.xy;

	if (cellPos.x >= dims.x - 1.0 || cellPos.y >= dims.y - 1.0)
	{
		// outside the (W-1) x (H-1) cells, never read
		gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		return;
	}

	//the cell corresponds to a "diagonal"-texel in the similarity graph
	//at first we fetch the surrounding edges (N,W,S,E of the pixel) and the diagonal itself from the similaritygraph
	vec2 gd = cellPos * 2.0 + 2.0;
	float eCenter      = edgeValue(gd);
	float eNorth       = edgeValue(gd + vec2( 0.0,  1.0));
	float eNorthCenter = edgeValue(gd + vec2( 0.0,  2.0));
	float eEast        = edgeValue(gd + vec2( 1.0,  0.0));
	float eEastCenter  = edgeValue(gd + vec2( 2.0,  0.0));
	float eSouth       = edgeValue(gd + vec2( 0.0, -1.0));
	float eSouthCenter = edgeValue(gd + vec2( 0.0, -2.0));
	float eWest        = edgeValue(gd + vec2(-1.0,  0.0));
	float eWestCenter  = edgeValue(gd + vec2(-2.0,  0.0));

	//init vertexdata
	vec2 v0_pos = vec2(-1.0, -1.0);
	vec4 v0_neighbors = vec4(-1.0, -1.0, -1.0, -1.0);
	float v0_flags = 0.0;
	vec2 v1_pos = vec2(-1.0, -1.0);
	vec4 v1_neighbors = vec4(-1.0, -1.0, -1.0, -1.0);
	float v1_flags = 0.0;

	//corner detection
	bool ignoreN = cellPos.y > dims.y - 3.0;
	bool ignoreE = cellPos.x > dims.x - 3.0;
	bool ignoreS = cellPos.y < 1.0;
	bool ignoreW = cellPos.x < 1.0;

	bool neighborsFound = false;
	bool nNeighborsFound = false;
	bool wNeighborsFound = false;
	bool sNeighborsFound = false;
	bool eNeighborsFound = false;
	float neighborCount = 0.0;
	float nNeighborIndex = -1.0;
	float wNeighborIndex = -1.0;
	float sNeighborIndex = -1.0;
	float eNeighborIndex = -1.0;

	//we need these for solving the t-junction position adjustment issues
	vec2 nVector = vec2(0.0, 0.0);
	vec2 eVector = vec2(0.0, 0.0);
	vec2 sVector = vec2(0.0, 0.0);
	vec2 wVector = vec2(0.0, 0.0);

	//browse neighborhood
	if (!ignoreN && eNorth == 0.0) {
		nNeighborsFound = true;
		neighborsFound = true;
		neighborCount += 1.0;
		//the northern neighbor depends on the northern diagonal
		if (eNorthCenter == EDGE_DIAGONAL_ULLR) {
			nNeighborIndex = getNeighborIndex(DIR_NORTH, 0.0);
			nVector = vec2(-0.25, 0.75);
		} else if (eNorthCenter == EDGE_DIAGONAL_LLUR) {
			nNeighborIndex = getNeighborIndex(DIR_NORTH, 1.0);
			nVector = vec2(0.25, 0.75);
		} else {
			nNeighborIndex = getNeighborIndex(DIR_NORTH, 0.0);
			nVector = vec2(0.0, 1.0);
		}
	}
	//is there a western edge?
	if (!ignoreW && eWest == 0.0) {
		wNeighborsFound = true;
		neighborsFound = true;
		neighborCount += 1.0;
		//the western neighbor depends on the western diagonal
		if (eWestCenter == EDGE_DIAGONAL_ULLR) {
			wNeighborIndex = getNeighborIndex(DIR_WEST, 1.0);
			wVector = vec2(-0.75, 0.25);
		} else if (eWestCenter == EDGE_DIAGONAL_LLUR) {
			wNeighborIndex = getNeighborIndex(DIR_WEST, 1.0);
			wVector = vec2(-0.75, -0.25);
		} else {
			wNeighborIndex = getNeighborIndex(DIR_WEST, 0.0);
			wVector = vec2(-1.0, 0.0);
		}
	}
	//is there a southern edge?
	if (!ignoreS && eSouth == 0.0) {
		sNeighborsFound = true;
		neighborsFound = true;
		neighborCount += 1.0;
		//the southern neighbor depends on the southern diagonal
		if (eSouthCenter == EDGE_DIAGONAL_ULLR) {
			sNeighborIndex = getNeighborIndex(DIR_SOUTH, 1.0);
			sVector = vec2(0.25, -0.75);
		} else if (eSouthCenter == EDGE_DIAGONAL_LLUR) {
			sNeighborIndex = getNeighborIndex(DIR_SOUTH, 0.0);
			sVector = vec2(-0.25, -0.75);
		} else {
			sNeighborIndex = getNeighborIndex(DIR_SOUTH, 0.0);
			sVector = vec2(0.0, -1.0);
		}
	}
	//is there a eastern edge?
	if (!ignoreE && eEast == 0.0) {
		eNeighborsFound = true;
		neighborsFound = true;
		neighborCount += 1.0;
		//the eastern neighbor depends on the eastern diagonal
		if (eEastCenter == EDGE_DIAGONAL_ULLR) {
			eNeighborIndex = getNeighborIndex(DIR_EAST, 0.0);
			eVector = vec2(0.75, -0.25);
		} else if (eEastCenter == EDGE_DIAGONAL_LLUR) {
			eNeighborIndex = getNeighborIndex(DIR_EAST, 0.0);
			eVector = vec2(0.75, 0.25);
		} else {
			eNeighborIndex = getNeighborIndex(DIR_EAST, 0.0);
			eVector = vec2(1.0, 0.0);
		}
	}

	if (neighborsFound) {
		//the four pixels of the cell
		vec2 LLPixel = cellPos;
		vec2 ULPixel = LLPixel + vec2(0.0, 1.0);
		vec2 LRPixel = LLPixel + vec2(1.0, 0.0);
		vec2 URPixel = LLPixel + vec2(1.0, 1.0);

		//gather vertexdata
		vec2 centerPos = cellPos + 0.5; //The "world space" position of the 2x2 center
		// we now check if the central 2x2 block is "split by a diagonal"
		if (eCenter == EDGE_DIAGONAL_ULLR) {
			bool twoNeighbors = true;
			//create the Vertex in the first sector -v0
			v0_pos = centerPos + vec2(XOFFSET_CLL, YOFFSET_CLL);
			if (sNeighborsFound) { v0_flags = HAS_SOUTHERN_NEIGHBOR + HAS_SOUTHERN_SPLINE; }
			else twoNeighbors = false;
			if (wNeighborsFound) { v0_flags += HAS_WESTERN_NEIGHBOR + HAS_WESTERN_SPLINE; }
			else twoNeighbors = false;
			if (twoNeighbors) {
				if (checkForCorner(sVector - vec2(XOFFSET_CLL, YOFFSET_CLL), wVector - vec2(XOFFSET_CLL, YOFFSET_CLL))) {
					v0_flags += DONT_OPTIMIZE_S + DONT_OPTIMIZE_W;
				}
			}
			v0_neighbors = vec4(-1.0, -1.0, sNeighborIndex, wNeighborIndex);

			twoNeighbors = true;
			//emit the Vertex in the second sector -v1
			v1_pos = centerPos + vec2(XOFFSET_CUR, YOFFSET_CUR);
			if (nNeighborsFound) { v1_flags = HAS_NORTHERN_NEIGHBOR + HAS_NORTHERN_SPLINE; }
			else twoNeighbors = false;
			if (eNeighborsFound) { v1_flags += HAS_EASTERN_NEIGHBOR + HAS_EASTERN_SPLINE; }
			else twoNeighbors = false;
			if (twoNeighbors) {
				if (checkForCorner(nVector - vec2(XOFFSET_CUR, YOFFSET_CUR), eVector - vec2(XOFFSET_CUR, YOFFSET_CUR))) {
					v1_flags += DONT_OPTIMIZE_N + DONT_OPTIMIZE_E;
				}
			}
			v1_neighbors = vec4(nNeighborIndex, eNeighborIndex, -1.0, -1.0);
		}
		else if (eCenter == EDGE_DIAGONAL_LLUR) {
			bool twoNeighbors = true;
			//emit the Vertex in the first sector
			v0_pos = centerPos + vec2(XOFFSET_CUL, YOFFSET_CUL);
			if (nNeighborsFound) { v0_flags = HAS_NORTHERN_NEIGHBOR + HAS_NORTHERN_SPLINE; }
			else twoNeighbors = false;
			if (wNeighborsFound) { v0_flags += HAS_WESTERN_NEIGHBOR + HAS_WESTERN_SPLINE; }
			else twoNeighbors = false;
			if (twoNeighbors) {
				if (checkForCorner(nVector - vec2(XOFFSET_CUL, YOFFSET_CUL), wVector - vec2(XOFFSET_CUL, YOFFSET_CUL))) {
					v0_flags += DONT_OPTIMIZE_N + DONT_OPTIMIZE_W;
				}
			}
			v0_neighbors = vec4(nNeighborIndex, -1.0, -1.0, wNeighborIndex);

			twoNeighbors = true;
			//emit the Vertex in the second sector
			v1_pos = centerPos + vec2(XOFFSET_CLR, YOFFSET_CLR);
			if (sNeighborsFound) { v1_flags = HAS_SOUTHERN_NEIGHBOR + HAS_SOUTHERN_SPLINE; }
			else twoNeighbors = false;
			if (eNeighborsFound) { v1_flags += HAS_EASTERN_NEIGHBOR + HAS_EASTERN_SPLINE; }
			else twoNeighbors = false;
			if (twoNeighbors) {
				if (checkForCorner(sVector - vec2(XOFFSET_CLR, YOFFSET_CLR), eVector - vec2(XOFFSET_CLR, YOFFSET_CLR))) {
					v1_flags += DONT_OPTIMIZE_S + DONT_OPTIMIZE_E;
				}
			}
			v1_neighbors = vec4(-1.0, eNeighborIndex, sNeighborIndex, -1.0);
		}
		else {
			//there is only one Vertex but we have to create a second one- as a dummy, in order to keep our indexing working
			v0_pos = centerPos;
			if (nNeighborsFound) { v0_flags += HAS_NORTHERN_NEIGHBOR; }
			if (eNeighborsFound) { v0_flags += HAS_EASTERN_NEIGHBOR; }
			if (sNeighborsFound) { v0_flags += HAS_SOUTHERN_NEIGHBOR; }
			if (wNeighborsFound) { v0_flags += HAS_WESTERN_NEIGHBOR; }

			if (neighborCount == 2.0) {
				if (nNeighborsFound) { v0_flags += HAS_NORTHERN_SPLINE; }
				if (eNeighborsFound) { v0_flags += HAS_EASTERN_SPLINE; }
				if (sNeighborsFound) { v0_flags += HAS_SOUTHERN_SPLINE; }
				if (wNeighborsFound) { v0_flags += HAS_WESTERN_SPLINE; }
			}
			else if (neighborCount == 3.0) {
				// T-junction: the two contour edges (or, failing that, the straight pair)
				// form the spline through the knot, the corrected position lies on it
				float contours = 0.0;
				float contourCount = 0.0;
				vec2 p0 = vec2(0.0, 0.0);
				vec2 p1 = vec2(0.0, 0.0);
				if (nNeighborsFound && isContour(ULPixel, URPixel)) {
					p0 = nVector;
					contourCount = 1.0;
					contours = HAS_NORTHERN_SPLINE;
				}
				if (eNeighborsFound && isContour(URPixel, LRPixel)) {
					if (contourCount == 0.0) p0 = eVector; else p1 = eVector;
					contourCount += 1.0;
					contours += HAS_EASTERN_SPLINE;
				}
				if (sNeighborsFound && isContour(LRPixel, LLPixel)) {
					if (contourCount == 0.0) p0 = sVector; else if (contourCount == 1.0) p1 = sVector;
					contourCount += 1.0;
					contours += HAS_SOUTHERN_SPLINE;
				}
				if (wNeighborsFound && isContour(LLPixel, ULPixel)) {
					if (contourCount == 0.0) p0 = wVector; else if (contourCount == 1.0) p1 = wVector;
					contourCount += 1.0;
					contours += HAS_WESTERN_SPLINE;
				}
				if (contourCount == 2.0) {
					v0_flags += contours + HAS_CORRECTED_POSITION;
					v1_pos = calcAdjustedPoint(centerPos + p0, centerPos, centerPos + p1);
					//mark vertex as corrected position vertex by setting its flag to -1
					v1_flags = -1.0;
				} else {
					//use angles to solve the problem
					if (nNeighborsFound && sNeighborsFound) {
						v0_flags += HAS_NORTHERN_SPLINE + HAS_SOUTHERN_SPLINE + HAS_CORRECTED_POSITION;
						v1_pos = calcAdjustedPoint(centerPos + nVector, centerPos, centerPos + sVector);
						v1_flags = -1.0;
					}
					else {
						v0_flags += HAS_EASTERN_SPLINE + HAS_WESTERN_SPLINE + HAS_CORRECTED_POSITION;
						v1_pos = calcAdjustedPoint(centerPos + eVector, centerPos, centerPos + wVector);
						v1_flags = -1.0;
					}
				}
			}
			v0_neighbors = vec4(nNeighborIndex, eNeighborIndex, sNeighborIndex, wNeighborIndex);
		}
	}

	//write this fragment's sector
#if defined(CP_OUTPUT_POSITION)
	gl_FragColor = vec4((sector == 0.0) ? v0_pos : v1_pos, 0.0, 0.0);
#elif defined(CP_OUTPUT_FLAGS)
	gl_FragColor = vec4((sector == 0.0) ? v0_flags : v1_flags, 0.0, 0.0, 0.0);
#elif defined(CP_OUTPUT_NEIGHBORS)
	gl_FragColor = (sector == 0.0) ? v0_neighbors : v1_neighbors;
#else
#error define one of CP_OUTPUT_POSITION, CP_OUTPUT_FLAGS, CP_OUTPUT_NEIGHBORS
#endif
}
