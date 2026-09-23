$input v_color0, v_texcoord0

// license:MIT
// copyright-holders:Felix Kreuzer
//============================================================
//
//  fs_crossings.sc - crossing diagonal elimination
//
//  Port of eliminateCrossings.frag. Where the similarity graph
//  connects both diagonals of a 2x2 pixel block (EDGE_CROSSING)
//  only one may survive. Fully connected blocks lose both; else
//  the three heuristics of the paper vote: curves (the longer
//  chain of valence-2 nodes wins), sparse pixels (the smaller
//  connected component in a window wins) and islands (a diagonal
//  that keeps a valence-1 node connected wins). Diagonals of fully
//  connected blocks are removed as well; everything else is copied.
//
//  Reads the graph with node valences (fs_valence.sc) and writes
//  the graph with the crossing resolved; fs_valence.sc runs again
//  afterwards.
//
//============================================================

#include "depixelize.sh"

SAMPLER2D(s_graph, 0);

uniform vec4 u_tex_size0;
uniform vec4 u_source_dims;
uniform vec4 u_target_dims;

GLOBAL vec2 g; /**< this fragment's graph texel */

// graph texel, zero outside the logical graph (the border)
vec4 fetchGraph(vec2 gc)
{
	if (gc.x < 0.0 || gc.y < 0.0 || gc.x >= 2.0 * u_source_dims.x || gc.y >= 2.0 * u_source_dims.y)
		return vec4(0.0, 0.0, 0.0, 0.0);
	return texture2DLod(s_graph, texelCenter(gc, u_tex_size0.xy), 0.0);
}

// .x of a graph texel: edge value, or the valence of a node
float edgeValue(vec2 gc)
{
	return decode255(fetchGraph(gc).x);
}

// .y of a node texel: its NODE_* connectivity bits
float nodeEdges(vec2 gc)
{
	return decode255(fetchGraph(gc).y);
}

// index into the 6x6 label window; row and column are the original 8x8 window coordinates 1..6
#define LABEL(row, col) (6 * ((row) - 1) + ((col) - 1))

GLOBAL int voteA;
GLOBAL int voteB;
GLOBAL int componentSizeA;
GLOBAL int componentSizeB;
GLOBAL int lArray[36]; /**< 6x6 connected component labels of voteSparsePixels */


void voteIslands() {
	if( edgeValue(g + vec2(-1.0, 1.0)) == 1.0) {
		voteA = voteA + 5;
		return;
	}
	if( edgeValue(g + vec2(1.0, -1.0)) == 1.0) {
		voteA = voteA + 5;
		return;
	}
	if( edgeValue(g + vec2(-1.0, -1.0)) == 1.0) {
		voteB = voteB + 5;
		return;
	}
	if( edgeValue(g + vec2(1.0, 1.0)) == 1.0) {
		voteB = voteB + 5;
		return;
	}
}

void countForComponent(int c) {
	if (c == 1) componentSizeA++;
	else if (c == 2) componentSizeB++;
}

//----------------------------------------------------------------
// labels the ring of nodes at the given level (0: the four nodes of the
// crossing's 2x2 block, 1: the ring around them) in the 6x6 label window
// and counts the sizes of components A (1) and B (2). Called with constant
// levels so every array index is a compile-time constant after inlining.
//----------------------------------------------------------------
void labelLevel(int level) {
	// neigborhood indices
	int nNW=0;
	int nW=0;
	int nSW=0;
	int nS=0;
	int nSE=0;
	int nE=0;
	int nNE=0;
	int nN=0;

	int xOFFSET = -(1+2*level);
	int yOFFSET = 1+(2*level);
	//NW corner-node
	//nhood ... stores this nodes neighboring information taken from the similarity graph
	float nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
	//float nhood = similarityGraph[FragCoordX-(1+2*level) + (FragCoordY+1+(2*level))*17];
	int currentComponentIndex = LABEL(3-level, 3-level);
	//current value in the label-array
	int currentComponent = lArray[currentComponentIndex];
	nS  = LABEL(4-level, 3-level);//TODO: OPtimieren
	nSW = LABEL(4-level, 2-level);
	nW  = LABEL(3-level, 2-level);
	nNW = LABEL(2-level, 2-level);
	nN  = LABEL(2-level, 3-level);
	nNE = LABEL(2-level, 4-level);
	nE  = LABEL(3-level, 4-level);

	if (currentComponent == 0) {
		//this block scans neighborhood for connected components, in case this node has not yet been labeled
		if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0) )		{currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTHWEST) && (lArray[nSW] !=0) )		{currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0) )				{currentComponent = lArray[nW];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTHWEST) && (lArray[nNW] !=0) )	{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0) )			{currentComponent = lArray[nN];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTHEAST) && (lArray[nNE] !=0) )	{currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0) )	{currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
	}
	if (currentComponent !=0) {
		//check SW W NW N NE
		if (hasFlag(nhood, NODE_SOUTHWEST)) {
			//SW
			//check if node is already labeled
			if( lArray[nSW] == 0) {
				lArray[nSW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_WEST)) {
			//W
			if(lArray[nW] == 0) {
				lArray[nW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTHWEST)) {
			if( lArray[nNW] == 0) {
				lArray[nNW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTH)) {
			//N
			if(lArray[nN] == 0) {
				lArray[nN] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTHEAST)) {
			//NE
			if(lArray[nNE] == 0) {
				lArray[nNE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
	}
	//N nodes
	if(level>0) {
		UNROLL
		for(int i = 0 ; i < level*2; i++) {
			xOFFSET = -(2*level-1)+2*i;
			yOFFSET = +1+2*level;
			nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
			//nhood = similarityGraph[FragCoordX-(2*level-1)+2*i + (FragCoordY+1+2*level)*17];
			currentComponentIndex = LABEL(3-level, i+4-level);
			currentComponent = lArray[currentComponentIndex];
			nW  = LABEL(3-level, i+3-level);
			nNW = LABEL(2-level, i+3-level);
			nN  = LABEL(2-level, i+4-level);
			nNE = LABEL(2-level, i+5-level);
			nE  = LABEL(3-level, i+5-level);
			if (currentComponent == 0) {
				//this block scans neighborhood for connected components, in case this node has not yet been labeled
				if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0) )		{currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTHWEST) && (lArray[nNW] !=0) )		{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0) )			{currentComponent = lArray[nN];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTHEAST) && (lArray[nNE] !=0) )	{currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0) )	{currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			}
			//check NW,N,NE neighbors
			if(currentComponent != 0) {
				if (hasFlag(nhood, NODE_NORTHWEST)) {
					if(lArray[nNW] == 0) {
						// label the NW neighbor in the label-array
						lArray[nNW] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_NORTH)) {
					if(lArray[nN] == 0) {
						// label the N neighbor in the label-array
						lArray[nN] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_NORTHEAST)) {
					if(lArray[nNE] == 0) {
						// label the NE neighbor in the label-array
						lArray[nNE] = currentComponent;
						countForComponent(currentComponent);
					}
				}
			}
		}
	}
	
	//NE corner-node
	xOFFSET = (1+2*level);
	yOFFSET = 1+(2*level);
	nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
	
	//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
	//current value in the label-array
	currentComponentIndex = LABEL(3-level, 4+level);
	currentComponent = lArray[currentComponentIndex];
	nW  = LABEL(3-level, 3+level);
	nNW	= LABEL(2-level, 3+level);
	nN  = LABEL(2-level, 4+level);
	nNE = LABEL(2-level, 5+level);
	nE  = LABEL(3-level, 5+level);
	nSE = LABEL(4-level, 5+level);
	nS  = LABEL(4-level, 4+level);
	if (currentComponent == 0) {
		//this block scans neighborhood for connected components, in case this node has not yet been labeled
		if(hasFlag(nhood, NODE_WEST) && (lArray[nNW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_NORTHWEST) && (lArray[nNW] !=0)){currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_NORTH)		  && (lArray[nN] !=0 )){currentComponent = lArray[nN];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_NORTHEAST) && (lArray[nNE] !=0)){currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_EAST)		      && (lArray[nE] !=0 )){currentComponent = lArray[nE];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if(hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0)){currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
	}
	if (currentComponent != 0) {
		//check NW N NE E SE
		if (hasFlag(nhood, NODE_NORTHWEST)) {
			if (lArray[nNW] == 0) {
				lArray[nNW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTH)) {
			if (lArray[nN] == 0) {
				lArray[nN] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTHEAST)) {
			if (lArray[nNE] == 0) {
				lArray[nNE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_EAST)) {
			if (lArray[nE] == 0) {
				lArray[nE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTHEAST)) {
			if (lArray[nSE] == 0) {
				lArray[nSE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
	}
	//E nodes
	if(level>0) {
		UNROLL
		for(int i = 0 ; i < level*2; i++) {
			xOFFSET = 1+2*level;
			yOFFSET = 2*level-1-2*i;
			nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
			//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
			currentComponentIndex = LABEL(i+4-level, 4+level);
			currentComponent = lArray[currentComponentIndex];
			nN = LABEL(i+3-level, 4+level);
			nNE= LABEL(i+3-level, 5+level);
			nE = LABEL(i+4-level, 5+level);
			nSE= LABEL(i+5-level, 5+level);
			nS = LABEL(i+5-level, 4+level);
			if (currentComponent == 0) {
				//this block scans neighborhood for connected components, in case this node has not yet been labeled
				if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0)){currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTHEAST) && (lArray[nNE] !=0)){currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0))			{currentComponent = lArray[nE];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0)){currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			}
			//check NE,E,SE neighbors
			if(currentComponent != 0) {
				if (hasFlag(nhood, NODE_NORTHEAST)) {
					if( lArray[nNE] == 0) {
						// label the NW neighbor in the label-array
						lArray[nNE] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_EAST)) {
					if( lArray[nE] == 0) {
						// label the N neighbor in the label-array
						lArray[nE] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_SOUTHEAST)) {
					if( lArray[nSE] == 0) {
						// label the NE neighbor in the label-array
						lArray[nSE] = currentComponent;
						countForComponent(currentComponent);
					}
				}
			}
		}
	}
	
	//SE corner-node
	xOFFSET = (1+2*level);
	yOFFSET = -(1+2*level);
	nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
	//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
	currentComponentIndex = LABEL(4+level, 4+level);
	currentComponent = lArray[currentComponentIndex];
	nN =LABEL(3+level, 4+level);
	nNE=LABEL(3+level, 5+level);
	nE= LABEL(4+level, 5+level);
	nSE=LABEL(5+level, 5+level);
	nS= LABEL(5+level, 4+level);
	nSW=LABEL(5+level, 3+level);
	nW =LABEL(4+level, 3+level);
	if (currentComponent == 0) {
		//this block scans neighborhood for connected components, in case this node has not yet been labeled
		if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0))	 {currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTHEAST) && (lArray[nNE] !=0))	 {currentComponent = lArray[nNE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0))			 {currentComponent = lArray[nE];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0))		 {currentComponent = lArray[nS];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
	}
	if (currentComponent !=0) {
		//check NE E SE S SW
		if (hasFlag(nhood, NODE_NORTHEAST)) {
			if (lArray[nNE] == 0){
				lArray[nNE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_EAST)) {
			if (lArray[nE] == 0){
				lArray[nE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTHEAST)) {
			if (lArray[nSE] == 0){
				lArray[nSE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTH)) {
			if (lArray[nS] == 0){
				lArray[nS] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTHWEST)) {
			if (lArray[nSW] == 0){
				lArray[nSW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
	}
	
	//S nodes
	if(level>0) {
		UNROLL
		for(int i = 0 ; i < level*2; i++) {
			xOFFSET = -(2*level-1)+2*i;
			yOFFSET = -(1+2*level);
			nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
			//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
			currentComponentIndex = LABEL(4+level, i+4-level);
			currentComponent = lArray[currentComponentIndex];
			nE = LABEL(4+level, i+5-level);
			nSE= LABEL(5+level, i+5-level);
			nS = LABEL(5+level, i+4-level);
			nSW= LABEL(5+level, i+3-level);
			nW = LABEL(4+level, i+3-level);
			if (currentComponent == 0) {
				//this block scans neighborhood for connected components, in case this node has not yet been labeled
				if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0)){currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0))			{currentComponent = lArray[nS];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0)){currentComponent = lArray[nW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			}
			//check SW,S,SE neighbors
			if(currentComponent != 0) {
				if (hasFlag(nhood, NODE_SOUTHEAST)) {
					if(lArray[nSE] == 0) {
						lArray[nSE] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_SOUTH)) {
					if(lArray[nS] == 0) {
						lArray[nS] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_SOUTHWEST)) {
					if(lArray[nSW] == 0) {
						lArray[nSW] = currentComponent;
						countForComponent(currentComponent);
					}
				}
			}
		}
	}
	
	//SW corner-node
	xOFFSET = -(1+2*level);
	yOFFSET = -(1+2*level);
	nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
	//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
	currentComponentIndex = LABEL(4+level, 3-level);
	currentComponent = lArray[currentComponentIndex];
	nE =LABEL(4+level, 4-level);
	nSE=LABEL(5+level, 4-level);
	nS= LABEL(5+level, 3-level);
	nSW=LABEL(5+level, 2-level);
	nW= LABEL(4+level, 2-level);
	nNW=LABEL(3+level, 2-level);
	nN =LABEL(3+level, 3-level);
	if (currentComponent == 0) {
		//this block scans neighborhood for connected components, in case this node has not yet been labeled
		if( hasFlag(nhood, NODE_EAST) && (lArray[nE] !=0)){currentComponent = lArray[nE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTHEAST) && (lArray[nSE] !=0)){currentComponent = lArray[nSE]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0))			{currentComponent = lArray[nS];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_SOUTHWEST) && (lArray[nSW] !=0)){currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0))			{currentComponent = lArray[nW];  lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTHWEST) && (lArray[nNW] !=0)){currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
		else if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0)){currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
	}
	if (currentComponent !=0) {
		//check SE S SW W NW
		if (hasFlag(nhood, NODE_SOUTHEAST)) {
			if (lArray[nSE] == 0){
				lArray[nSE] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTH)) {
			if (lArray[nS] == 0){
				lArray[nS] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_SOUTHWEST)) {
			if (lArray[nSW] == 0){
				lArray[nSW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_WEST)) {
			if (lArray[nW] == 0){
				lArray[nW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
		if (hasFlag(nhood, NODE_NORTHWEST)) {
			if (lArray[nNW] == 0){
				lArray[nNW] = currentComponent;
				countForComponent(currentComponent);
			}
		}
	}
	
	//W nodes
	if(level>0) {
		UNROLL
		for(int i = 0 ; i < level*2; i++) {
			xOFFSET = -(1+2*level);
			yOFFSET = -(2*level-1)+2*i;
			nhood = nodeEdges(g + vec2(float(xOFFSET), float(yOFFSET)));
			//nhood = similarityGraph[FragCoordX + xOFFSET + (FragCoordY + yOFFSET)*17];
			currentComponentIndex = LABEL(3+level-i, 3-level);
			currentComponent = lArray[currentComponentIndex];
			nN= LABEL(2+level-i, 3-level);
			nNW=LABEL(2+level-i, 2-level);
			nW =LABEL(3+level-i, 2-level);
			nSW=LABEL(4+level-i, 2-level);
			nS =LABEL(4+level-i, 3-level);
			if (currentComponent == 0) {
				//this block scans neighborhood for connected components, in case this node has not yet been labeled
				if( hasFlag(nhood, NODE_SOUTH) && (lArray[nS] !=0))	{currentComponent = lArray[nS]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_SOUTHWEST) && (lArray[nSW] !=0))	{currentComponent = lArray[nSW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_WEST) && (lArray[nW] !=0))			{currentComponent = lArray[nW];	 lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTHWEST) && (lArray[nNW] !=0))	{currentComponent = lArray[nNW]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
				else if( hasFlag(nhood, NODE_NORTH) && (lArray[nN] !=0))	{currentComponent = lArray[nN]; lArray[currentComponentIndex]=currentComponent;countForComponent(currentComponent);}
			}
			//check SW W NW neighbors
			if(currentComponent != 0) {
				if (hasFlag(nhood, NODE_SOUTHWEST)) {
					if( lArray[nSW] == 0) {
						lArray[nSW] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_WEST)) {
					if( lArray[nW] == 0) {
						lArray[nW] = currentComponent;
						countForComponent(currentComponent);
					}
				}
				if (hasFlag(nhood, NODE_NORTHWEST)) {
					if( lArray[nNW] == 0) {
						lArray[nNW] = currentComponent;
						countForComponent(currentComponent);
					}
				}
			}
		}
	}
}

void voteSparsePixels() {
	//INFO on border treatment
	// border treatment currently relies on the similiaritygraph texture wrap setting beeing GL_CLAMP_TO_BORDER
	// addidtionally GL_TEXTURE_BORDER_COLOR needs to be set to black(0,0,0,0), which is the default value btw.
	
	//label-array 8x8
	//let component A be 1 and B be 2
	for (int n = 0; n < 36; n++) lArray[n] = 0;
	lArray[LABEL(3, 3)] = 1;
	lArray[LABEL(3, 4)] = 2;
	lArray[LABEL(4, 3)] = 2;
	lArray[LABEL(4, 4)] = 1;
	
	labelLevel(0);
	labelLevel(1);
	//now that the connected component sizes are computed we vote for the smaller component
	if(componentSizeA < componentSizeB) {
		//vote for A ... weight is difference between the sizes of the components
		voteA = voteA + (componentSizeB - componentSizeA);
	} else if(componentSizeA > componentSizeB) {
		//vote for B ... weight is difference between the sizes of the components
		voteB = voteB + (componentSizeA - componentSizeB);
	}
}


int traceNodes(vec2 nodeCoords, float predecessorNodeDirection) {
	//codes the edge directions
			// N  ... 128
			// NE ... 64
			// E  ... 32
			// SE ... 16
			// S  ... 8
			// SW ... 4
			// W  ... 2
			// NW ... 1
	int totalLength = 0; // initial total length

	vec2 currentNodeCoords = nodeCoords;
	//check node valence
	vec4 currentNodeValue = fetchGraph(currentNodeCoords);
	float valence = decode255(currentNodeValue.x);
	float edges = decode255(currentNodeValue.y);

	vec2 nextNodeCoords = vec2(0.0, 0.0);
	float directionToCurrentNode = 0.0;

	// follow the chain of valence-2 nodes; bounded so a broken graph cannot hang the GPU
	for (int step = 0; step < 4096; step++) {
		if (valence != 2.0) break;
		//get next neighbor: the node's other edge (edges XOR predecessor)
		float nextNodeDirection = hasFlag(edges, predecessorNodeDirection) ? edges - predecessorNodeDirection : edges + predecessorNodeDirection;

		if (nextNodeDirection == NODE_NORTHWEST) {
			nextNodeCoords = currentNodeCoords + vec2(-1.0, 1.0);
			directionToCurrentNode = NODE_SOUTHEAST;
		} else if (nextNodeDirection == NODE_WEST) {
			nextNodeCoords = currentNodeCoords + vec2(-1.0, 0.0);
			directionToCurrentNode = NODE_EAST;
		} else if (nextNodeDirection == NODE_SOUTHWEST) {
			nextNodeCoords = currentNodeCoords + vec2(-1.0, -1.0);
			directionToCurrentNode = NODE_NORTHEAST;
		} else if (nextNodeDirection == NODE_SOUTH) {
			nextNodeCoords = currentNodeCoords + vec2(0.0, -1.0);
			directionToCurrentNode = NODE_NORTH;
		} else if (nextNodeDirection == NODE_SOUTHEAST) {
			nextNodeCoords = currentNodeCoords + vec2(1.0, -1.0);
			directionToCurrentNode = NODE_NORTHWEST;
		} else if (nextNodeDirection == NODE_EAST) {
			nextNodeCoords = currentNodeCoords + vec2(1.0, 0.0);
			directionToCurrentNode = NODE_WEST;
		} else if (nextNodeDirection == NODE_NORTHEAST) {
			nextNodeCoords = currentNodeCoords + vec2(1.0, 1.0);
			directionToCurrentNode = NODE_SOUTHWEST;
		} else if (nextNodeDirection == NODE_NORTH) {
			nextNodeCoords = currentNodeCoords + vec2(0.0, 1.0);
			directionToCurrentNode = NODE_SOUTH;
		} else {
			//this should not be the case, but just in case ;)
			return 0;
		}
		//get next node
		currentNodeCoords = nextNodeCoords;
		predecessorNodeDirection = directionToCurrentNode;
		currentNodeValue = fetchGraph(currentNodeCoords);
		valence = decode255(currentNodeValue.x);
		edges = decode255(currentNodeValue.y);
		totalLength++;
	}
	return totalLength;
}


void voteCurves() {
	int lengthA = 1;
	int lengthB = 1;
	//coordinates for nodes A1,A2,B1,B2
	//A1 B2
	//B1 A2
	vec2 A1 = g + vec2(-1.0,  1.0);
	vec2 A2 = g + vec2( 1.0, -1.0);
	vec2 B1 = g + vec2(-1.0, -1.0);
	vec2 B2 = g + vec2( 1.0,  1.0);
	lengthA = lengthA + traceNodes(A1, NODE_SOUTHEAST);
	lengthA = lengthA + traceNodes(A2, NODE_NORTHWEST);
	lengthB = lengthB + traceNodes(B1, NODE_NORTHEAST);
	lengthB = lengthB + traceNodes(B2, NODE_SOUTHWEST);
	//evaluate lengths and vote
	if (lengthA==lengthB) {
		//no one wins
		return;
	} else if(lengthA > lengthB) {
		//A wins
		voteA = voteA + lengthA - lengthB;
	} else {
		//B wins
		voteB = voteB + lengthB - lengthA;
	}
}

/*
returns true if the 2x2 block of nodes around the crossing diagonals is fully connected
*/
bool isFullyConnectedCD() {
	//we only have to look at one neighboring edge to determine whether a 2x2 block is fully connected or not
	if (edgeValue(g + vec2(0.0, 1.0)) == 0.0 ) {
		return false;
	} else return true;
	
}

bool isFullyConnectedD() {
	//examine upper edge
	if ( edgeValue(g + vec2(0.0, 1.0)) == EDGE_HORVERT) {
		//examine right edge
		if ( edgeValue(g + vec2(1.0, 0.0)) == EDGE_HORVERT) {
			//examine lower edge
			if ( edgeValue(g + vec2(0.0, -1.0)) == EDGE_HORVERT) {
				//we could skip the last edgecheck -> this one is fully connected for shure
				if ( edgeValue(g + vec2(-1.0, 0.0)) == EDGE_HORVERT) {
					return true;
				} else return false;
			} else return false;
		} else return false;
	} else return false;
}

void main() {
	g = fragmentTexel(v_texcoord0, u_target_dims.xy);
	vec4 fragmentXColor = fetchGraph(g);
	float fragmentValue = decode255(fragmentXColor.x);
	//check if fragment hit is a crossing diagonal
	if (fragmentValue == EDGE_CROSSING) {
		// we are looking at a crossing diagonal
		// 1. check if 2x2 block is fully connected
		if (isFullyConnectedCD()) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
			return;
		}
		voteA = 0;
		voteB = 0;
		componentSizeA = 2;
		componentSizeB = 2;
		voteCurves();
		voteIslands();
		voteSparsePixels();

		//eliminate loser
		if (voteA == voteB) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		} else if (voteA > voteB) {
			gl_FragColor = vec4(EDGE_DIAGONAL_ULLR / 255.0, 0.0, 0.0, 0.0);
		} else {
			gl_FragColor = vec4(EDGE_DIAGONAL_LLUR / 255.0, 0.0, 0.0, 0.0);
		}
	} else if (fragmentValue == EDGE_DIAGONAL_ULLR || fragmentValue == EDGE_DIAGONAL_LLUR) {
		//we just hit a Diagonal ... it might be fully connected anyways
		if (isFullyConnectedD()) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
		} else {
			gl_FragColor = fragmentXColor;
		}
	} else {
		//copy texel
		gl_FragColor = fragmentXColor;
	}
}
