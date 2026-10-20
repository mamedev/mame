$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_optimize.sc - spline energy optimization
//
//  Port of OptimizeEnergy.vert. Every knot that lies on a spline
//  through two neighbors is moved along the gradient of the
//  curvature energy of its segment, balanced against a positional
//  energy that keeps it near its original place, using a bracketed
//  golden section line search. Knots on sharp corners
//  (DONT_OPTIMIZE) and on T-junctions are left alone. The chain
//  runs this pass twice, feeding the first result into the second.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_positions, 0);
SAMPLER2D(s_flags, 1);
SAMPLER2D(s_neighbors, 2);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

//Golden section search
#define R                 0.61803399
#define TOL               0.0001
#define SEARCH_STEPS      20   // cap on the golden section steps per knot, as in the reference implementation
#define BRACKET_SEARCH_A  0.1
#define BRACKET_SEARCH_B  -0.1
#define GOLD              1.618034
#define GLIMIT            10.0
#define TINY              0.000000001 // prevents division by zero

GLOBAL float knotsPerRow;
GLOBAL vec2 pos; /**< this knot's position */

//----------------------------------------------------------------
// knot texture access; a negative index never names a knot and reads as zeros
// (all knot textures have the same size as this pass's output)
//----------------------------------------------------------------
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

float calcPositionalEnergy(vec2 pNew, vec2 pOld)
{
	float dist = 2.5 * distance(pNew, pOld);
	return dist * dist * dist * dist;
}

vec2 calcGradient(vec2 node1, vec2 node2, vec2 node3)
{
	return 8.0 * node2 - 4.0 * node1 - 4.0 * node3;
}

float calcSegmentCurveEnergy(vec2 node1, vec2 node2, vec2 node3)
{
	vec2 tmp = node1 - 2.0 * node2 + node3;
	return tmp.x * tmp.x + tmp.y * tmp.y;
}

// total energy of the segment (n0, pos - gradient * x, n1)
float energyAt(vec2 n0, vec2 n1, vec2 gradient, float x)
{
	vec2 pOpt = pos - gradient * x;
	return calcSegmentCurveEnergy(n0, pOpt, n1) + calcPositionalEnergy(pOpt, pos);
}

vec3 findBracket(vec2 n0, vec2 n1, vec2 gradient)
{
	float ulim, u, r, q, fu, dum, qr;
	float ax = BRACKET_SEARCH_A;
	float bx = BRACKET_SEARCH_B;
	float fa = energyAt(n0, n1, gradient, ax);
	float fb = energyAt(n0, n1, gradient, bx);
	if (fb > fa) {
		//switch roles of a and b so we can go downhill from a to b
		dum = ax;
		ax = bx;
		bx = dum;
		dum = fb;
		fb = fa;
		fa = dum;
	}
	//first guess for c
	float cx = bx + GOLD * (bx - ax);
	float fc = energyAt(n0, n1, gradient, cx);
	//find bracket. The loop is bounded so a degenerate case cannot hang the GPU, and
	//it has no break or early return: shader model 3 handles those poorly, so a
	//finished search just idles through the remaining iterations.
	vec3 result = vec3(0.0, 0.0, 0.0);
	bool done = false;
	for (int step = 0; step < 64; step++) {
		if (!done && fb > fc) {
			r = (bx - ax) * (fb - fc);
			q = (bx - cx) * (fb - fa);
			qr = q - r;
			u = bx - ((bx - cx) * q - (bx - ax) * r) / (2.0 * sign(qr) * max(abs(qr), TINY));
			ulim = bx + GLIMIT * (cx - bx);
			if ((bx - u) * (u - cx) > 0.0) {
				fu = energyAt(n0, n1, gradient, u);
				if (fu < fc) {
					result = vec3(bx, u, cx);
					done = true;
				} else if (fu > fb) {
					result = vec3(ax, bx, u);
					done = true;
				} else {
					u = cx + GOLD * (cx - bx);
					fu = energyAt(n0, n1, gradient, u);
				}
			} else if ((cx - u) * (u - ulim) > 0.0) {
				fu = energyAt(n0, n1, gradient, u);
				if (fu < fc) {
					dum = cx + GOLD * (cx - bx);
					bx = cx;
					cx = u;
					u = dum;
					fb = fc;
					fc = fu;
					fu = energyAt(n0, n1, gradient, u);
				}
			} else if ((u - ulim) * (ulim - cx) >= 0.0) {
				u = ulim;
				fu = energyAt(n0, n1, gradient, u);
			} else {
				u = cx + GOLD * (cx - bx);
				fu = energyAt(n0, n1, gradient, u);
			}
			if (!done) {
				ax = bx;
				bx = cx;
				cx = u;
				fa = fb;
				fb = fc;
				fc = fu;
			}
		}
	}
	if (!done) {
		result = vec3(ax, bx, cx);
	}
	return result;
}

// returns (gradient.xy, offset): the knot moves to pos - gradient * offset
vec3 searchOffset(vec2 n0, vec2 n1)
{
	vec2 gradient = calcGradient(n0, pos, n1);
	if (length(gradient) > 0.0) {
		gradient = normalize(gradient);
	} else return vec3(0.0, 0.0, 0.0);

	vec3 bracket = findBracket(n0, n1, gradient);

	float x0 = bracket.x;
	float x1 = 0.0;
	float x2 = 0.0;
	float x3 = bracket.z;

	// NOTE: the original defines the golden section constant as "#define C 1 - R"
	// without parentheses, so its "C * x" expands to "1 - R * x" and the line search
	// probes are not those of a textbook golden section search. The knot placement
	// (and the positional penalty it was tuned with) depends on that; with the
	// intended arithmetic the optimized splines of neighboring knots cross and leave
	// black speckles at the contours. The expanded expressions are kept verbatim.
	if (abs(bracket.z - bracket.y) > abs(bracket.y - bracket.x)) {
		x1 = bracket.y;
		x2 = bracket.y + 1.0 - R * (bracket.z - bracket.y);
	} else {
		x1 = bracket.y - 1.0 - R * (bracket.y - bracket.x);
		x2 = bracket.y;
	}
	float f1 = energyAt(n0, n1, gradient, x1);
	float f2 = energyAt(n0, n1, gradient, x2);
	float fx;
	for (int counter = 0; counter < SEARCH_STEPS; counter++) {
		// no break here either, see findBracket
		if (abs(x3 - x0) > TOL * (abs(x1) + abs(x2))) {
			if (f2 < f1) {
				x0 = x1;
				x1 = x2;
				x2 = R * x1 + 1.0 - R * x3;
				fx = energyAt(n0, n1, gradient, x2);
				f1 = f2;
				f2 = fx;
			}
			else {
				x3 = x2;
				x2 = x1;
				x1 = R * x2 + 1.0 - R * x0;
				fx = energyAt(n0, n1, gradient, x1);
				f2 = f1;
				f1 = fx;
			}
		}
	}
	float offset = 0.0;
	if (f1 < f2) {
		offset = x1;
	} else {
		offset = x2;
	}
	return vec3(gradient, offset);
}

//----------------------------------------------------------------
// the spline neighbor of this knot in one direction: its position, or its
// corrected position when it is the endpoint of a T-junction. Returns false
// and sets noOpt when the segment must not be optimized.
//----------------------------------------------------------------
bool splineNeighbor(float flags, float neighbor, float ownDontOptimize, float neighborDontOptimize, float neighborBackSpline, inout vec2 position, inout bool noOpt)
{
	float neighborflags = fetchFlags(neighbor);
	if (hasFlag(flags, ownDontOptimize) || hasFlag(neighborflags, neighborDontOptimize)) {
		noOpt = true;
		return false;
	}
	if (hasFlag(neighborflags, HAS_CORRECTED_POSITION) && !hasFlag(neighborflags, neighborBackSpline)) {
		//our neighbour is an endpoint on a t-junction and needs to be adjusted
		position = fetchPosition(neighbor + 1.0);
	} else {
		position = fetchPosition(neighbor);
	}
	return true;
}

void main()
{
	knotsPerRow = 2.0 * (u_source_dims.x - 1.0);
	vec2 texel = fragmentTexel(v_texcoord0, u_target_dims.xy);
	float knot = texel.y * knotsPerRow + texel.x;
	pos = fetchPosition(knot);
	float flags = fetchFlags(knot);
	vec4 neighbors = fetchNeighbors(knot);

	vec2 optimizedPos = pos;
	// only knots on splines (flags > 16) without a sharp corner (flags < 512)
	if (flags > 16.0 && flags < 512.0) {
		vec2 n0 = vec2(0.0, 0.0);
		vec2 n1 = vec2(0.0, 0.0);
		float splineCount = 0.0;
		bool splineNoOpt = false;
		vec2 p = vec2(0.0, 0.0);

		if (hasFlag(flags, HAS_NORTHERN_SPLINE)) {
			if (splineNeighbor(flags, neighbors.x, DONT_OPTIMIZE_N, DONT_OPTIMIZE_S, HAS_SOUTHERN_SPLINE, p, splineNoOpt)) {
				if (splineCount == 0.0) n0 = p; else n1 = p;
				splineCount += 1.0;
			}
		}
		if (hasFlag(flags, HAS_EASTERN_SPLINE) && !splineNoOpt) {
			if (splineNeighbor(flags, neighbors.y, DONT_OPTIMIZE_E, DONT_OPTIMIZE_W, HAS_WESTERN_SPLINE, p, splineNoOpt)) {
				if (splineCount == 0.0) n0 = p; else n1 = p;
				splineCount += 1.0;
			}
		}
		if (hasFlag(flags, HAS_SOUTHERN_SPLINE) && !splineNoOpt) {
			if (splineNeighbor(flags, neighbors.z, DONT_OPTIMIZE_S, DONT_OPTIMIZE_N, HAS_NORTHERN_SPLINE, p, splineNoOpt)) {
				if (splineCount == 0.0) n0 = p; else n1 = p;
				splineCount += 1.0;
			}
		}
		if (hasFlag(flags, HAS_WESTERN_SPLINE) && !splineNoOpt) {
			if (splineNeighbor(flags, neighbors.w, DONT_OPTIMIZE_W, DONT_OPTIMIZE_E, HAS_EASTERN_SPLINE, p, splineNoOpt)) {
				if (splineCount == 0.0) n0 = p; else n1 = p;
				splineCount += 1.0;
			}
		}

		if (splineCount == 2.0 && !splineNoOpt) {
			vec3 shift = searchOffset(n0, n1);
			optimizedPos = pos - shift.xy * shift.z;
		}
	}
	gl_FragColor = vec4(optimizedPos, 0.0, 0.0);
}
