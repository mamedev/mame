$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_rasterize.sc - Gaussian rasterization of the depixelized image
//
//  Port of GaussRasterizer.frag. Every output pixel maps to a
//  point in source pixel space; the four source pixels around it
//  contribute with Gaussian weights unless the ray from the point
//  to a pixel center crosses one of the spline pieces of the cell
//  (fs_cell_segments.sc), in which case that pixel lies on the
//  other side of a contour and is left out.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_segments, 0);
SAMPLER2D(s_source, 1);
SAMPLER2D(s_graph, 2);

uniform vec4 u_tex_size0;
uniform vec4 u_tex_size1;
uniform vec4 u_tex_size2;
uniform vec4 u_source_dims;

//Constants
#define SEGMENT_SAMPLES 5      /**< linear pieces each spline piece is sampled into */
#define CULL_EPSILON 0.001     /**< margin for the piece-vs-cell bounding box test (cell units) */
#define GAUSS_MULTIPLIER 2.5

GLOBAL vec4 influencingPixels; /**< UL UR LL LR, 1.0 while the pixel may contribute */
GLOBAL vec2 cellSpaceCoords;
// the unit cell square containing the fragment, grown by CULL_EPSILON
GLOBAL vec2 cellMin;
GLOBAL vec2 cellMax;
// rays from the fragment to the four surrounding pixel centers (constant per fragment):
// UL = (rayX.x, rayY.y), UR = (rayX.y, rayY.y), LL = (rayX.x, rayY.x), LR = (rayX.y, rayY.x)
GLOBAL vec2 rayX; /**< x offsets to the left (.x) and right (.y) pixel column */
GLOBAL vec2 rayY; /**< y offsets to the lower (.x) and upper (.y) pixel row */
// gaussian accumulation
GLOBAL vec4 colorSum;
GLOBAL float weightSum;

// record r of a cell, see fs_cell_segments.sc
vec4 fetchCellRecord(vec2 cell, float r)
{
	float row = floor((r + 0.5) / 3.0);
	vec2 texel = 3.0 * cell + vec2(r - 3.0 * row, row);
	return texture2DLod(s_segments, texelCenter(texel, u_tex_size0.xy), 0.0);
}

//----------------------------------------------------------------
// calulates the spline point at parametric position t
//----------------------------------------------------------------
vec2 calcSplinePoint(vec2 p0, vec2 p1, vec2 p2, float t)
{
	float t2 = 0.5 * t * t;
	float a = t2 - t + 0.5;
	float b = -2.0 * t2 + t + 0.5;
	return a * p0 + b * p1 + t2 * p2;
}

//----------------------------------------------------------------
// test the ray (fragment -> pixel center, direction r) against the
// segment (pointA -> pointB, direction s) for intersection.
// The classic form solves t = (ba x s) / (r x s) and u = (ba x r) / (r x s)
// and requires both in [0,1]. Multiplying through by sign(r x s) gives the
// same test on the numerators without the two divisions:
//   0 <= (ba x s) * sign(r x s) <= |r x s|   and   0 <= (ba x r) * sign(r x s) <= |r x s|
//----------------------------------------------------------------
bool rayHitsSegment(float rXs, float baXs, float baXr)
{
	if (rXs == 0.0) {
		return false;
	}
	float sgn = sign(rXs);
	float d = abs(rXs);
	float t = baXs * sgn;
	if ((t < 0.0) || (t > d)) {
		return false;
	}
	float u = baXr * sgn;
	if ((u < 0.0) || (u > d)) {
		return false;
	}
	return true;
}

void findSegmentIntersections(vec2 p0, vec2 p1, vec2 p2)
{
	vec2 pointA = calcSplinePoint(p0, p1, p2, 0.0);
	// the spline segment is sampled at t = STEP, 2*STEP, ... 1.0 (SEGMENT_SAMPLES pieces)
	const float STEP = 1.0 / float(SEGMENT_SAMPLES);
	for (int i = 1; i <= SEGMENT_SAMPLES; i++) {
		vec2 pointB = calcSplinePoint(p0, p1, p2, float(i) * STEP);
		// All four rays lie inside the fragment's unit cell square, so a piece whose
		// bounding box misses that square cannot intersect any of them.
		vec2 pmax = max(pointA, pointB);
		vec2 pmin = min(pointA, pointB);
		if (pmax.x < cellMin.x || pmax.y < cellMin.y || pmin.x > cellMax.x || pmin.y > cellMax.y) {
			pointA = pointB;
			continue;
		}
		vec2 s = pointB - pointA;
		vec2 ba = pointA - cellSpaceCoords;
		float baXs = ba.x * s.y - ba.y * s.x;
		// r x s = r.x*s.y - r.y*s.x and ba x r = ba.x*r.y - ba.y*r.x for the four rays,
		// whose components are all combinations of rayX and rayY
		vec2 rxSy = rayX * s.y;   // r.x*s.y for the left/right column
		vec2 rySx = rayY * s.x;   // r.y*s.x for the lower/upper row
		vec2 baxRy = ba.x * rayY; // ba.x*r.y for the lower/upper row
		vec2 bayRx = ba.y * rayX; // ba.y*r.x for the left/right column
		//evaluate interections (a corner that is already cut off needs no further tests)
		if (influencingPixels.x > 0.0 && rayHitsSegment(rxSy.x - rySx.y, baXs, baxRy.y - bayRx.x)) {
			influencingPixels.x = 0.0;
		}
		if (influencingPixels.y > 0.0 && rayHitsSegment(rxSy.y - rySx.y, baXs, baxRy.y - bayRx.y)) {
			influencingPixels.y = 0.0;
		}
		if (influencingPixels.z > 0.0 && rayHitsSegment(rxSy.x - rySx.x, baXs, baxRy.x - bayRx.x)) {
			influencingPixels.z = 0.0;
		}
		if (influencingPixels.w > 0.0 && rayHitsSegment(rxSy.y - rySx.x, baXs, baxRy.x - bayRx.y)) {
			influencingPixels.w = 0.0;
		}
		pointA = pointB;
	}
}

//----------------------------------------------------------------
// adds the gaussian-weighted color of the given source pixel to the accumulators
//----------------------------------------------------------------
void addPixel(vec2 pixelCoords)
{
	vec4 col = texture2DLod(s_source, texelCenter(pixelCoords, u_tex_size1.xy), 0.0);
	vec2 d = cellSpaceCoords - pixelCoords;
	float weight = exp(-dot(d, d) * GAUSS_MULTIPLIER);
	colorSum += col * weight;
	weightSum += weight;
}

void main()
{
	vec2 dims = u_source_dims.xy;

	// create a boolean vector containing information on the influence of surrounding pixels on this fragment
	// R G B A ... UL UR LL LR
	influencingPixels = vec4(1.0, 1.0, 1.0, 1.0);
	colorSum = vec4(0.0, 0.0, 0.0, 0.0);
	weightSum = 0.0;

	// source pixel space: pixel centers at integer coordinates 0 .. W-1
	cellSpaceCoords = clamp(v_texcoord0 * dims - 0.5, vec2(0.0, 0.0), dims - 1.0);
	// surrounding pixel Coordinates
	vec2 LLCoords = floor(cellSpaceCoords);
	vec2 URCoords = ceil(cellSpaceCoords);
	vec2 ULCoords = vec2(LLCoords.x, URCoords.y);
	vec2 LRCoords = vec2(URCoords.x, LLCoords.y);
	rayX = vec2(LLCoords.x, URCoords.x) - cellSpaceCoords.x;
	rayY = vec2(LLCoords.y, URCoords.y) - cellSpaceCoords.y;
	cellMin = LLCoords - CULL_EPSILON;
	cellMax = URCoords + CULL_EPSILON;

	// The spline pieces running through this cell were resolved once per cell by
	// fs_cell_segments.sc; cut off every surrounding pixel whose connecting ray
	// crosses one of them.
	vec2 cell = min(LLCoords, dims - 2.0);
	vec4 header = fetchCellRecord(cell, 0.0);
	float elementCount = header.x;
	float elementTypes = header.y;
	float q = 0.0; // index of the current element's first point (two points per record)
	float typeBit = 1.0;
	for (int e = 0; e < 4; e++) {
		if (float(e) >= elementCount) break;
		float t = 1.0 + floor(q / 2.0);
		vec4 A = fetchCellRecord(cell, t);
		vec4 B = fetchCellRecord(cell, t + 1.0);
		// C is only needed for THROUGH elements, which never start beyond q = 5; an END at
		// q = 9 would address record 7, so clamp to stay in range
		vec4 C = fetchCellRecord(cell, min(t + 2.0, 6.0));
		// the element's points as consecutive vec2s, independent of the texel alignment
		vec2 P0, P1, P2, P3, P4;
		if (mod(q, 2.0) == 0.0) { P0 = A.xy; P1 = A.zw; P2 = B.xy; P3 = B.zw; P4 = C.xy; }
		else                    { P0 = A.zw; P1 = B.xy; P2 = B.zw; P3 = C.xy; P4 = C.zw; }
		if (hasFlag(elementTypes, typeBit)) {
			// THROUGH: points (pm1, pn, p1, pm2, p2)
			findSegmentIntersections(P0, P1, P2);
			findSegmentIntersections(P1, P0, P3);
			findSegmentIntersections(P1, P2, P4);
			q += 5.0;
		} else {
			// END: points (pn, p1, p2)
			findSegmentIntersections(P0, P0, P1);
			findSegmentIntersections(P0, P1, P2);
			q += 3.0;
		}
		typeBit *= 2.0;
	}

	//influencingPixels order: UL UR LL LR
	// NOTE: the reference implementation reads the connectivity bits of the UL pixel's
	// node from the 8-bit normalized graph as int(g) which is 1 only when all eight bits
	// are set, so the north-western neighbor is added only for fully connected nodes.
	// Decoding the bits properly gives contours a staircase look, so the behaviour is
	// kept deliberately (see GaussRasterizer.frag).
	if (influencingPixels.x > 0.0) {
		//calculate influence of the Pixel
		addPixel(ULCoords);
		//checkout this pixels connected neigbors
		vec2 node = 2.0 * ULCoords + 1.0;
		float edges = floor(texture2DLod(s_graph, texelCenter(node, u_tex_size2.xy), 0.0).y);
		if (edges >= 1.0) {
			addPixel(vec2(ULCoords.x - 1.0, ULCoords.y + 1.0));
		}
	}
	if (influencingPixels.y > 0.0) {
		addPixel(URCoords);
	}
	if (influencingPixels.z > 0.0) {
		addPixel(LLCoords);
	}
	if (influencingPixels.w > 0.0) {
		addPixel(LRCoords);
	}
	gl_FragColor = vec4((colorSum / weightSum).rgb, 1.0) * v_color0;
}
