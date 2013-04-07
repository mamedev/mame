//============================================================
//
//  d3dcomm.h - Common Win32 Direct3D structures
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#ifndef __WIN_D3DCOMM__
#define __WIN_D3DCOMM__

struct d3d_info;

//============================================================
//  CONSTANTS
//============================================================

namespace d3d
{

//============================================================
//  FORWARD DECLARATIONS
//============================================================

class texture_info;

//============================================================
//  TYPE DEFINITIONS
//============================================================

/* d3d::texture_info holds information about a texture */
class texture_info
{
public:
	texture_info(d3d_info *d3d, render_texinfo *texsource, UINT32 flags);
	~texture_info();

	render_texinfo *		get_texinfo() { return m_texinfo; }

	int						get_width() { return m_rawwidth; }
	int						get_height() { return m_rawheight; }
	int						get_xscale() { return m_xprescale; }
	int						get_yscale() { return m_yprescale; }

	void					set_data(const render_texinfo *texsource, UINT32 flags);

public:
	void prescale();
	void compute_size(int texwidth, int texheight);

	d3d_info *				m_d3d;						// d3d info pointer
	texture_info *			m_next;						// next texture in the list
	texture_info *			m_prev;						// prev texture in the list

	UINT32					m_hash;						// hash value for the texture
	UINT32					m_flags;					// rendering flags
	render_texinfo *		m_texinfo;					// copy of the texture info
	float					m_ustart, m_ustop;			// beginning/ending U coordinates
	float					m_vstart, m_vstop;			// beginning/ending V coordinates
	int						m_rawwidth, m_rawheight;	// raw width/height of the texture
	int						m_type;						// what type of texture are we?
	int						m_xborderpix, m_yborderpix;	// number of border pixels on X/Y
	int						m_xprescale, m_yprescale;	// X/Y prescale factor
	int						m_cur_frame;				// what is our current frame?
	int						m_prev_frame;				// what was our last frame? (used to determine pause state)
	d3d_texture *			m_d3dtex;					// Direct3D texture pointer
	d3d_surface *			m_d3dsurface;				// Direct3D offscreen plain surface pointer
	d3d_texture *           m_d3dfinaltex;              // Direct3D final (post-scaled) texture
	int                     m_target_index;             // Direct3D target index
};

}; // d3d


/* d3d_poly_info holds information about a single polygon/d3d primitive */
struct d3d_poly_info
{
		D3DPRIMITIVETYPE        type;                       // type of primitive
		UINT32                  count;                      // total number of primitives
		UINT32                  numverts;                   // total number of vertices
		UINT32                  flags;                      // rendering flags
		DWORD                   modmode;                    // texture modulation mode
		d3d::texture_info *     texture;                    // pointer to texture info
		float                   line_time;                  // used by vectors
		float                   line_length;                // used by vectors
};


/* d3d_vertex describes a single vertex */
struct d3d_vertex
{
	float                   x, y, z;                    // X,Y,Z coordinates
	float                   rhw;                        // RHW when no HLSL, padding when HLSL
	D3DCOLOR                color;                      // diffuse color
	float                   u0, v0;                     // texture stage 0 coordinates
};


/* line_aa_step is used for drawing antialiased lines */
struct line_aa_step
{
	float                   xoffs, yoffs;               // X/Y deltas
	float                   weight;                     // weight contribution
};

#endif
