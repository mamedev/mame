//============================================================
//
//  drawd3d.h - Win32 Direct3D header
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

#ifndef __WIN_DRAWD3D__
#define __WIN_DRAWD3D__


#include "d3dhlsl.h"


//============================================================
//  CONSTANTS
//============================================================

#define VERTEX_BASE_FORMAT	(D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define VERTEX_BUFFER_SIZE	(2048*4+4)



//============================================================
//  TYPE DEFINITIONS
//============================================================

/* d3d_info is the information about Direct3D for the current screen */
struct d3d_info
{
	int						adapter;					// ordinal adapter number
	int						width, height;				// current width, height
	int						refresh;					// current refresh rate
	int						create_error_count;			// number of consecutive create errors

	win_window_info *		window;						// current window info

	d3d_device *			device;						// pointer to the Direct3DDevice object
	int						gamma_supported;			// is full screen gamma supported?
	d3d_present_parameters	presentation;				// set of presentation parameters
	D3DDISPLAYMODE			origmode;					// original display mode for the adapter
	D3DFORMAT				pixformat;					// pixel format we are using

	d3d_vertex_buffer *		vertexbuf;					// pointer to the vertex buffer object
	d3d_vertex *			lockedbuf;					// pointer to the locked vertex buffer
	int						numverts;					// number of accumulated vertices

	d3d_poly_info			poly[VERTEX_BUFFER_SIZE/3]; // array to hold polygons as they are created
	int						numpolys;					// number of accumulated polygons

	d3d_texture_info *		texlist;					// list of active textures
	int						dynamic_supported;			// are dynamic textures supported?
	int						stretch_supported;			// is StretchRect with point filtering supported?
	int						mod2x_supported;			// is D3DTOP_MODULATE2X supported?
	int						mod4x_supported;			// is D3DTOP_MODULATE4X supported?
	D3DFORMAT				screen_format;				// format to use for screen textures
	D3DFORMAT				yuv_format;					// format to use for YUV textures

	DWORD					texture_caps;				// textureCaps field
	DWORD					texture_max_aspect;			// texture maximum aspect ratio
	DWORD					texture_max_width;			// texture maximum width
	DWORD					texture_max_height;			// texture maximum height

	d3d_texture_info *		last_texture;				// previous texture
	UINT32					last_texture_flags;			// previous texture flags
	int						last_blendenable;			// previous blendmode
	int						last_blendop;				// previous blendmode
	int						last_blendsrc;				// previous blendmode
	int						last_blenddst;				// previous blendmode
	int						last_filter;				// previous texture filter
	int						last_wrap;					// previous wrap state
	DWORD					last_modmode;				// previous texture modulation

	bitmap_argb32			vector_bitmap;				// experimental: bitmap for vectors
	d3d_texture_info *		vector_texture;				// experimental: texture for vectors

	bitmap_rgb32			default_bitmap;				// experimental: default bitmap
	d3d_texture_info *		default_texture;			// experimental: default texture

	void *					hlsl_buf;					// HLSL vertex data
	hlsl_info *				hlsl;						// HLSL interface
};



//============================================================
//  PROTOTYPES
//============================================================

d3d_texture_info *texture_create(d3d_info *d3d, const render_texinfo *texsource, UINT32 flags);

#endif
