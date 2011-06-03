//============================================================
//
//  drawd3d.c - Win32 Direct3D HLSL-specific header
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

#ifndef __WIN_D3DHLSL__
#define __WIN_D3DHLSL__


#include "aviio.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

/* hlsl_options is the information about runtime-mutable Direct3D HLSL options */
/* in the future this will be moved into an OSD/emu shared buffer */
typedef struct _hlsl_options hlsl_options;
struct _hlsl_options
{
	float					shadow_mask_alpha;
	int						shadow_mask_count_x;
	int						shadow_mask_count_y;
	float					shadow_mask_u_size;
	float					shadow_mask_v_size;
	float					curvature;
	float					pincushion;
	float					scanline_alpha;
	float					scanline_scale;
	float					scanline_height;
	float					scanline_bright_scale;
	float					scanline_bright_offset;
	float					scanline_offset;
	float					defocus_x;
	float					defocus_y;
	float					red_converge_x;
	float					red_converge_y;
	float					green_converge_x;
	float					green_converge_y;
	float					blue_converge_x;
	float					blue_converge_y;
	float					red_radial_converge_x;
	float					red_radial_converge_y;
	float					green_radial_converge_x;
	float					green_radial_converge_y;
	float					blue_radial_converge_x;
	float					blue_radial_converge_y;
	float					red_from_red;
	float					red_from_green;
	float					red_from_blue;
	float					green_from_red;
	float					green_from_green;
	float					green_from_blue;
	float					blue_from_red;
	float					blue_from_green;
	float					blue_from_blue;
	float					red_offset;
	float					green_offset;
	float					blue_offset;
	float					red_scale;
	float					green_scale;
	float					blue_scale;
	float					red_power;
	float					green_power;
	float					blue_power;
	float					red_floor;
	float					green_floor;
	float					blue_floor;
	float					red_phosphor_life;
	float					green_phosphor_life;
	float					blue_phosphor_life;
	float					saturation;
};

class hlsl_info
{
public:
	// construction/destruction
	hlsl_info();
	~hlsl_info();

	void init(d3d *d3dintf, win_window_info *window);
	void init_fsfx_quad(void *vertbuf);

	bool enabled() { return master_enable; }

	void begin();
	void init_effect_info(d3d_poly_info *poly);
	void render_quad(d3d_poly_info *poly, int vertnum);
	void end();

	int register_texture(d3d_texture_info *texture);
	int register_prescaled_texture(d3d_texture_info *texture, int scwidth, int scheight);

	void window_save();
	void window_record();
	bool recording() { return avi_output_file != NULL; }

	void avi_update_snap(d3d_surface *surface);
	void render_snapshot(d3d_surface *surface);
	void record_texture();

	void frame_complete();

	void set_texture(d3d_texture_info *texture);

	int create_resources();
	void delete_resources();

	// slider-related functions
	slider_state *init_slider_list();

private:
	void					end_avi_recording();
	void					begin_avi_recording(const char *name);

	bool 					screen_encountered[9];		// whether a given screen was encountered this frame

	d3d *                   d3dintf;					// D3D interface
	win_window_info *       window;						// D3D window info

	bool					master_enable;				// overall enable flag
	bool					yiq_enable;					// YIQ-convolution flag
	int						prescale_size_x;			// prescale size x
	int						prescale_size_y;			// prescale size y
	int						preset;						// preset, if relevant
	bitmap_t *				shadow_bitmap;				// shadow mask bitmap for post-processing shader
	d3d_texture_info *		shadow_texture;				// shadow mask texture for post-processing shader
	int						registered_targets;			// number of registered HLSL targets (i.e., screens)
	hlsl_options *			options;					// current uniform state
	avi_file *				avi_output_file;			// AVI file
	bitmap_t *				avi_snap;					// AVI snapshot
	int						avi_frame;					// AVI frame
	attotime				avi_frame_period;			// AVI frame period
	attotime				avi_next_frame_time;		// AVI next frame time
	d3d_surface *           avi_copy_surface;			// AVI destination surface in system memory
	d3d_texture *           avi_copy_texture;			// AVI destination texture in system memory
	d3d_surface *           avi_final_target;			// AVI upscaled surface
	d3d_texture *           avi_final_texture;			// AVI upscaled texture
	bool					render_snap;				// whether or not to take HLSL post-render snapshot
	bool					snap_rendered;				// whether we just rendered our HLSL post-render shot or not
	d3d_surface *           snap_copy_target;			// snapshot destination surface in system memory
	d3d_texture *           snap_copy_texture;			// snapshot destination surface in system memory
	d3d_surface *           snap_target;				// snapshot upscaled surface
	d3d_texture *           snap_texture;				// snapshot upscaled texture
	int						snap_width;					// snapshot width
	int						snap_height;				// snapshot height

	// HLSL effects
	d3d_surface *   		backbuffer;					// pointer to our device's backbuffer
	d3d_effect *			curr_effect;				// pointer to the currently active effect object
	d3d_effect *			effect;						// pointer to the current primary-effect object
	d3d_effect *			prescale_effect;			// pointer to the current prescale-effect object
	d3d_effect *			post_effect;				// pointer to the current post-effect object
	d3d_effect *			pincushion_effect;			// pointer to the current pincushion-effect object
	d3d_effect *			focus_effect;				// pointer to the current focus-effect object
	d3d_effect *			phosphor_effect;			// pointer to the current phosphor-effect object
	d3d_effect *			deconverge_effect;			// pointer to the current deconvergence-effect object
	d3d_effect *			color_effect;				// pointer to the current color-effect object
	d3d_effect *			yiq_encode_effect;			// pointer to the current YIQ encoder effect object
	d3d_effect *			yiq_decode_effect;			// pointer to the current YIQ decoder effect object
	d3d_vertex *			fsfx_vertices;				// pointer to our full-screen-quad object

	// render targets
	int						target_use_count[9];		// Whether or not a target has been used yet
	d3d_texture_info *		target_in_use[9];			// Target texture that is currently in use
	d3d_surface *			last_target[9];				// Render target surface pointer for each screen's previous frame
	d3d_texture *			last_texture[9];			// Render target texture pointer for each screen's previous frame
	d3d_surface *			prescaletarget0[9];			// Render target surface pointer (prescale, if necessary)
	d3d_surface *			target0[9];					// Render target surface pointer (pass 0, if necessary)
	d3d_surface *			target1[9];					// Render target surface pointer (pass 1, if necessary)
	d3d_surface *			target2[9];					// Render target surface pointer (pass 2, if necessary)
	d3d_surface *			target3[9];					// Render target surface pointer (pass 3, if necessary)
	d3d_surface *			target4[9];					// Render target surface pointer (pass 4, if necessary)
	d3d_surface *			smalltarget0[9];			// Render target surface pointer (small pass 0, if necessary)
	d3d_texture *			prescaletexture0[9];		// Render target surface pointer (prescale, if necessary)
	d3d_texture *			texture0[9];				// Render target texture pointer (pass 0, if necessary)
	d3d_texture *			texture1[9];				// Render target texture pointer (pass 1, if necessary)
	d3d_texture *			texture2[9];				// Render target texture pointer (pass 2, if necessary)
	d3d_texture *			texture3[9];				// Render target texture pointer (pass 3, if necessary)
	d3d_texture *			texture4[9];				// Render target texture pointer (pass 4, if necessary)
	d3d_texture *			smalltexture0[9];			// Render target texture pointer (small pass 0, if necessary)
};

#endif
