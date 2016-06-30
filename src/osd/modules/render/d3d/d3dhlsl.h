// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawd3d.c - Win32 Direct3D HLSL-specific header
//
//============================================================

#ifndef __WIN_D3DHLSL__
#define __WIN_D3DHLSL__

#include <vector>
#include "aviio.h"
#include "../frontend/mame/ui/menuitem.h"
#include "../frontend/mame/ui/slider.h"
#include "modules/lib/osdlib.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

// Typedefs for dynamically loaded functions
typedef HRESULT (WINAPI *d3dx_create_effect_from_file_fn)(LPDIRECT3DDEVICE9, LPCTSTR, const D3DXMACRO *, LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT *, LPD3DXBUFFER *);

class effect;
class shaders;

class uniform
{
public:
	typedef enum
	{
		UT_VEC4,
		UT_VEC3,
		UT_VEC2,
		UT_FLOAT,
		UT_INT,
		UT_BOOL,
		UT_MATRIX,
		UT_SAMPLER
	} uniform_type;

	enum
	{
		CU_SCREEN_DIMS = 0,
		CU_SOURCE_DIMS,
		CU_TARGET_DIMS,
		CU_QUAD_DIMS,

		CU_SWAP_XY,
		CU_VECTOR_SCREEN,

		CU_NTSC_CCFREQ,
		CU_NTSC_A,
		CU_NTSC_B,
		CU_NTSC_O,
		CU_NTSC_P,
		CU_NTSC_NOTCH,
		CU_NTSC_YFREQ,
		CU_NTSC_IFREQ,
		CU_NTSC_QFREQ,
		CU_NTSC_HTIME,
		CU_NTSC_ENABLE,

		CU_COLOR_RED_RATIOS,
		CU_COLOR_GRN_RATIOS,
		CU_COLOR_BLU_RATIOS,
		CU_COLOR_OFFSET,
		CU_COLOR_SCALE,
		CU_COLOR_SATURATION,

		CU_CONVERGE_LINEAR_X,
		CU_CONVERGE_LINEAR_Y,
		CU_CONVERGE_RADIAL_X,
		CU_CONVERGE_RADIAL_Y,

		CU_FOCUS_SIZE,

		CU_PHOSPHOR_LIFE,

		CU_POST_VIGNETTING,
		CU_POST_DISTORTION,
		CU_POST_CUBIC_DISTORTION,
		CU_POST_DISTORT_CORNER,
		CU_POST_ROUND_CORNER,
		CU_POST_SMOOTH_BORDER,
		CU_POST_REFLECTION,
		CU_POST_SHADOW_ALPHA,
		CU_POST_SHADOW_COUNT,
		CU_POST_SHADOW_UV,
		CU_POST_SHADOW_UV_OFFSET,
		CU_POST_SHADOW_DIMS,
		CU_POST_SCANLINE_ALPHA,
		CU_POST_SCANLINE_SCALE,
		CU_POST_SCANLINE_HEIGHT,
		CU_POST_SCANLINE_VARIATION,
		CU_POST_SCANLINE_BRIGHT_SCALE,
		CU_POST_SCANLINE_BRIGHT_OFFSET,
		CU_POST_POWER,
		CU_POST_FLOOR,

		CU_COUNT
	};

	uniform(effect *shader, const char *name, uniform_type type, int id);

	void        update();

protected:
	uniform_type m_type;
	int         m_id;

	effect      *m_shader;
	D3DXHANDLE  m_handle;
};

class effect
{
	friend class uniform;

public:
	effect(shaders *shadersys, IDirect3DDevice9 *dev, const char *name, const char *path);
	~effect();

	void        begin(UINT *passes, DWORD flags);
	void        begin_pass(UINT pass);

	void        end();
	void        end_pass();

	void        set_technique(const char *name);

	void        set_vector(D3DXHANDLE param, int count, float *vector);
	void        set_float(D3DXHANDLE param, float value);
	void        set_int(D3DXHANDLE param, int value);
	void        set_bool(D3DXHANDLE param, bool value);
	void        set_matrix(D3DXHANDLE param, D3DXMATRIX *matrix);
	void        set_texture(D3DXHANDLE param, IDirect3DTexture9 *tex);

	void        add_uniform(const char *name, uniform::uniform_type type, int id);
	void        update_uniforms();

	D3DXHANDLE  get_parameter(D3DXHANDLE param, const char *name);

	shaders*    get_shaders() { return m_shaders; }

	bool        is_valid() { return m_valid; }

private:
	std::vector<std::unique_ptr<uniform>> m_uniform_list;

	ID3DXEffect *m_effect;
	shaders     *m_shaders;

	bool        m_valid;
};

class d3d_render_target;
class cache_target;
class renderer_d3d9;

/* hlsl_options is the information about runtime-mutable Direct3D HLSL options */
/* in the future this will be moved into an OSD/emu shared buffer */
struct hlsl_options
{
	bool                    params_init;
	bool                    params_dirty;
	int                     shadow_mask_tile_mode;
	float                   shadow_mask_alpha;
	char                    shadow_mask_texture[1024];
	int                     shadow_mask_count_x;
	int                     shadow_mask_count_y;
	float                   shadow_mask_u_size;
	float                   shadow_mask_v_size;
	float                   shadow_mask_u_offset;
	float                   shadow_mask_v_offset;
	float                   distortion;
	float                   cubic_distortion;
	float                   distort_corner;
	float                   round_corner;
	float                   smooth_border;
	float                   reflection;
	float                   vignetting;
	float                   scanline_alpha;
	float                   scanline_scale;
	float                   scanline_height;
	float                   scanline_variation;
	float                   scanline_bright_scale;
	float                   scanline_bright_offset;
	float                   scanline_jitter;
	float                   hum_bar_alpha;
	float                   defocus[2];
	float                   converge_x[3];
	float                   converge_y[3];
	float                   radial_converge_x[3];
	float                   radial_converge_y[3];
	float                   red_ratio[3];
	float                   grn_ratio[3];
	float                   blu_ratio[3];
	float                   offset[3];
	float                   scale[3];
	float                   power[3];
	float                   floor[3];
	float                   phosphor[3];
	float                   saturation;

	// NTSC
	int                     yiq_enable;
	float                   yiq_jitter;
	float                   yiq_cc;
	float                   yiq_a;
	float                   yiq_b;
	float                   yiq_o;
	float                   yiq_p;
	float                   yiq_n;
	float                   yiq_y;
	float                   yiq_i;
	float                   yiq_q;
	float                   yiq_scan_time;
	int                     yiq_phase_count;

	// Vectors
	float                   vector_beam_smooth;
	float                   vector_length_scale;
	float                   vector_length_ratio;

	// Bloom
	int                     bloom_blend_mode;
	float                   bloom_scale;
	float                   bloom_overdrive[3];
	float                   bloom_level0_weight;
	float                   bloom_level1_weight;
	float                   bloom_level2_weight;
	float                   bloom_level3_weight;
	float                   bloom_level4_weight;
	float                   bloom_level5_weight;
	float                   bloom_level6_weight;
	float                   bloom_level7_weight;
	float                   bloom_level8_weight;
};

struct slider_desc
{
	const char *                name;
	int                         minval;
	int                         defval;
	int                         maxval;
	int                         step;
	int                         slider_type;
	int                         screen_type;
	int                         id;
	float                       scale;
	const char *                format;
	std::vector<const char *>   strings;
};

class slider
{
public:
	slider(slider_desc *desc, void *value, bool *dirty) : m_desc(desc), m_value(value) { }

	INT32 update(std::string *str, INT32 newval);

private:
	slider_desc *   m_desc;
	void *          m_value;
};

class shaders : public slider_changed_notifier
{
	friend class effect;
	friend class uniform;

public:
	// construction/destruction
	shaders();
	~shaders();

	bool init(d3d_base *d3dintf, running_machine *machine, renderer_d3d9 *renderer);

	bool enabled() { return post_fx_enable && d3dintf->post_fx_available; }
	void toggle() { post_fx_enable = initialized && !post_fx_enable; }

	d3d_render_target* get_vector_target(render_primitive *prim);
	bool create_vector_target(render_primitive *prim);

	void begin_frame();
	void end_frame();

	void begin_draw();
	void end_draw();

	void render_quad(poly_info *poly, int vertnum);

	bool register_texture(render_primitive *prim, texture_info *texture);
	d3d_render_target* get_texture_target(render_primitive *prim, texture_info *texture);
	bool add_render_target(renderer_d3d9* d3d, render_primitive *prim, texture_info* texture, int source_width, int source_height, int target_width, int target_height);
	bool add_cache_target(renderer_d3d9* d3d, texture_info* texture, int source_width, int source_height, int target_width, int target_height, int screen_index);

	void window_save();
	void window_record();
	bool recording() const { return avi_output_file != nullptr; }

	void avi_update_snap(IDirect3DSurface9 *surface);
	void render_snapshot(IDirect3DSurface9 *surface);
	void record_texture();
	void init_fsfx_quad();

	void                    set_texture(texture_info *info);
	d3d_render_target *     find_render_target(texture_info *texture);
	void                    remove_render_target(texture_info *texture);
	void                    remove_render_target(int source_width, int source_height, UINT32 screen_index, UINT32 page_index);
	void                    remove_render_target(d3d_render_target *rt);

	int create_resources();
	void delete_resources();

	// slider-related functions
	virtual INT32 slider_changed(running_machine &machine, void *arg, int /*id*/, std::string *str, INT32 newval) override;
	slider_state* slider_alloc(running_machine &machine, int id, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, void *arg);
	void init_slider_list();
	std::vector<ui::menu_item> get_slider_list() { return m_sliders; }
	void *get_slider_option(int id, int index = 0);

private:
	void                    blit(IDirect3DSurface9 *dst, bool clear_dst, D3DPRIMITIVETYPE prim_type, UINT32 prim_index, UINT32 prim_count);
	void                    enumerate_screens();

	void                    end_avi_recording();
	void                    begin_avi_recording(const char *name);

	d3d_render_target*      find_render_target(int source_width, int source_height, UINT32 screen_index, UINT32 page_index);
	cache_target *          find_cache_target(UINT32 screen_index, int width, int height);
	void                    remove_cache_target(cache_target *cache);

	rgb_t                   apply_color_convolution(rgb_t color);

	// Shader passes
	int                     ntsc_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     color_convolution_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     prescale_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     deconverge_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     defocus_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     phosphor_pass(d3d_render_target *rt, cache_target *ct, int source_index, poly_info *poly, int vertnum);
	int                     post_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum, bool prepare_bloom);
	int                     downsample_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     bloom_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     distortion_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     vector_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     vector_buffer_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     screen_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	void                    ui_pass(poly_info *poly, int vertnum);

	d3d_base *              d3dintf;                    // D3D interface

	running_machine *       machine;
	renderer_d3d9 *         d3d;                        // D3D renderer

	bool                    post_fx_enable;             // overall enable flag
	bool                    oversampling_enable;        // oversampling enable flag
	int                     num_screens;                // number of emulated physical screens
	int                     curr_screen;                // current screen for render target operations
	bitmap_argb32           shadow_bitmap;              // shadow mask bitmap for post-processing shader
	texture_info *          shadow_texture;             // shadow mask texture for post-processing shader
	hlsl_options *          options;                    // current options

	avi_file::ptr           avi_output_file;            // AVI file
	bitmap_rgb32            avi_snap;                   // AVI snapshot
	int                     avi_frame;                  // AVI frame
	attotime                avi_frame_period;           // AVI frame period
	attotime                avi_next_frame_time;        // AVI next frame time
	IDirect3DSurface9 *     avi_copy_surface;           // AVI destination surface in system memory
	IDirect3DTexture9 *     avi_copy_texture;           // AVI destination texture in system memory
	IDirect3DSurface9 *     avi_final_target;           // AVI upscaled surface
	IDirect3DTexture9 *     avi_final_texture;          // AVI upscaled texture

	IDirect3DSurface9 *     black_surface;              // black dummy surface
	IDirect3DTexture9 *     black_texture;              // black dummy texture

	bool                    render_snap;                // whether or not to take HLSL post-render snapshot
	bool                    snap_rendered;              // whether we just rendered our HLSL post-render shot or not
	IDirect3DSurface9 *     snap_copy_target;           // snapshot destination surface in system memory
	IDirect3DTexture9 *     snap_copy_texture;          // snapshot destination surface in system memory
	IDirect3DSurface9 *     snap_target;                // snapshot upscaled surface
	IDirect3DTexture9 *     snap_texture;               // snapshot upscaled texture
	int                     snap_width;                 // snapshot width
	int                     snap_height;                // snapshot height

	bool                    initialized;                // whether or not we're initialized

	// HLSL effects
	IDirect3DSurface9 *     backbuffer;                 // pointer to our device's backbuffer
	effect *                curr_effect;                // pointer to the currently active effect object
	effect *                default_effect;             // pointer to the primary-effect object
	effect *                prescale_effect;            // pointer to the prescale-effect object
	effect *                post_effect;                // pointer to the post-effect object
	effect *                distortion_effect;          // pointer to the distortion-effect object
	effect *                focus_effect;               // pointer to the focus-effect object
	effect *                phosphor_effect;            // pointer to the phosphor-effect object
	effect *                deconverge_effect;          // pointer to the deconvergence-effect object
	effect *                color_effect;               // pointer to the color-effect object
	effect *                ntsc_effect;                // pointer to the NTSC effect object
	effect *                bloom_effect;               // pointer to the bloom composite effect
	effect *                downsample_effect;          // pointer to the bloom downsample effect
	effect *                vector_effect;              // pointer to the vector-effect object

	texture_info *          curr_texture;
	d3d_render_target *     curr_render_target;
	poly_info *             curr_poly;

	std::vector<std::unique_ptr<d3d_render_target>> m_render_target_list;
	std::vector<std::unique_ptr<cache_target>> m_cache_target_list;

	std::vector<slider*>    internal_sliders;
	std::vector<ui::menu_item> m_sliders;

	static slider_desc      s_sliders[];
	static hlsl_options     last_options;               // last used options
	static char             last_system_name[16];       // last used system

	osd::dynamic_module::ptr d3dx9_dll;
	d3dx_create_effect_from_file_fn d3dx_create_effect_from_file_ptr;
};

#endif
