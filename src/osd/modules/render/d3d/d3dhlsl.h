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
#include "../frontend/mame/ui/menuitem.h"
#include "../frontend/mame/ui/slider.h"
#include "modules/lib/osdlib.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

// Typedefs for dynamically loaded functions
typedef HRESULT (WINAPI *d3dx_create_effect_from_file_fn)(LPDIRECT3DDEVICE9, LPCTSTR, const D3DXMACRO *, LPD3DXINCLUDE, DWORD, LPD3DXEFFECTPOOL, LPD3DXEFFECT *, LPD3DXBUFFER *);

struct slider_state;

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
		CU_SCREEN_COUNT,
		CU_SOURCE_DIMS,
		CU_TARGET_DIMS,
		CU_TARGET_SCALE,
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
		CU_CHROMA_MODE,
		CU_CHROMA_A,
		CU_CHROMA_B,
		CU_CHROMA_C,
		CU_CHROMA_CONVERSION_GAIN,
		CU_CHROMA_Y_GAIN,
		CU_LUT_ENABLE,
		CU_UI_LUT_ENABLE,

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
class movie_recorder;

/* hlsl_options is the information about runtime-mutable Direct3D HLSL options */
/* in the future this will be moved into an OSD/emu shared buffer */
struct hlsl_options
{
	bool                    params_init = false;
	bool                    params_dirty = false;
	int                     shadow_mask_tile_mode = 0;
	float                   shadow_mask_alpha = 0.0;
	char                    shadow_mask_texture[1024]{ 0 };
	int                     shadow_mask_count_x = 0;
	int                     shadow_mask_count_y = 0;
	float                   shadow_mask_u_size = 0.0;
	float                   shadow_mask_v_size = 0.0;
	float                   shadow_mask_u_offset = 0.0;
	float                   shadow_mask_v_offset = 0.0;
	float                   distortion = 0.0;
	float                   cubic_distortion = 0.0;
	float                   distort_corner = 0.0;
	float                   round_corner = 0.0;
	float                   smooth_border = 0.0;
	float                   reflection = 0.0;
	float                   vignetting = 0.0;
	float                   scanline_alpha = 0.0;
	float                   scanline_scale = 0.0;
	float                   scanline_height = 0.0;
	float                   scanline_variation = 0.0;
	float                   scanline_bright_scale = 0.0;
	float                   scanline_bright_offset = 0.0;
	float                   scanline_jitter = 0.0;
	float                   hum_bar_alpha = 0.0;
	float                   defocus[2]{ 0.0 };
	float                   converge_x[3]{ 0.0 };
	float                   converge_y[3]{ 0.0 };
	float                   radial_converge_x[3]{ 0.0 };
	float                   radial_converge_y[3]{ 0.0 };
	float                   red_ratio[3]{ 0.0 };
	float                   grn_ratio[3]{ 0.0 };
	float                   blu_ratio[3]{ 0.0 };
	float                   offset[3]{ 0.0 };
	float                   scale[3]{ 0.0 };
	float                   power[3]{ 0.0 };
	float                   floor[3]{ 0.0 };
	float                   phosphor[3]{ 0.0 };
	float                   saturation = 0.0;
	int                     chroma_mode = 0;
	float                   chroma_a[2]{ 0.0 };
	float                   chroma_b[2]{ 0.0 };
	float                   chroma_c[2]{ 0.0 };
	float                   chroma_conversion_gain[3]{ 0.0 };
	float                   chroma_y_gain[3]{ 0.0 };

	// NTSC
	int                     yiq_enable = 0;
	float                   yiq_jitter = 0.0;
	float                   yiq_cc = 0.0;
	float                   yiq_a = 0.0;
	float                   yiq_b = 0.0;
	float                   yiq_o = 0.0;
	float                   yiq_p = 0.0;
	float                   yiq_n = 0.0;
	float                   yiq_y = 0.0;
	float                   yiq_i = 0.0;
	float                   yiq_q = 0.0;
	float                   yiq_scan_time = 0.0;
	int                     yiq_phase_count = 0;

	// Vectors
	float                   vector_beam_smooth = 0.0;
	float                   vector_length_scale = 0.0;
	float                   vector_length_ratio = 0.0;

	// Bloom
	int                     bloom_blend_mode = 0;
	float                   bloom_scale = 0.0;
	float                   bloom_overdrive[3]{ 0.0 };
	float                   bloom_level0_weight = 0.0;
	float                   bloom_level1_weight = 0.0;
	float                   bloom_level2_weight = 0.0;
	float                   bloom_level3_weight = 0.0;
	float                   bloom_level4_weight = 0.0;
	float                   bloom_level5_weight = 0.0;
	float                   bloom_level6_weight = 0.0;
	float                   bloom_level7_weight = 0.0;
	float                   bloom_level8_weight = 0.0;

	// Final
	char lut_texture[1024]{ 0 };
	int lut_enable = 0;
	char ui_lut_texture[1024]{ 0 };
	int ui_lut_enable = 0;
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

	int32_t update(std::string *str, int32_t newval);

private:
	slider_desc *   m_desc;
	void *          m_value;
};

class shaders
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

	void begin_draw();
	void end_draw();

	void render_quad(poly_info *poly, int vertnum);

	bool create_vector_target(render_primitive *prim, int screen);
	d3d_render_target* get_vector_target(render_primitive *prim, int screen);
	bool create_texture_target(render_primitive *prim, int width, int height, int screen);
	d3d_render_target* get_texture_target(render_primitive *prim, int width, int height, int screen);
	bool add_render_target(renderer_d3d9* d3d, render_primitive *prim, int source_width, int source_height, int source_screen, int target_width, int target_height);

	void save_snapshot();
	void record_movie();
	void record_audio(const int16_t *buffer, int samples_this_frame);

	void init_fsfx_quad();

	void set_texture(texture_info *info);
	void remove_render_target(int source_width, int source_height, uint32_t screen_index);
	void remove_render_target(d3d_render_target *rt);

	int create_resources();
	void delete_resources();

	// slider-related functions
	std::unique_ptr<slider_state> slider_alloc(std::string &&title, int32_t minval, int32_t defval, int32_t maxval, int32_t incval, slider *arg);
	void init_slider_list();
	std::vector<ui::menu_item> get_slider_list() { return m_sliders; }
	void *get_slider_option(int id, int index = 0);

private:
	void                    blit(IDirect3DSurface9 *dst, bool clear_dst, D3DPRIMITIVETYPE prim_type, uint32_t prim_index, uint32_t prim_count);
	void                    enumerate_screens();

	void                    render_snapshot(IDirect3DSurface9 *surface);
	// Time since last call, only updates once per render of all screens
	double                  delta_time() { return delta_t; }
	d3d_render_target*      find_render_target(int source_width, int source_height, uint32_t screen_index);

	rgb_t                   apply_color_convolution(rgb_t color);

	// Shader passes
	int                     ntsc_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     color_convolution_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     prescale_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     deconverge_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     scanline_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     defocus_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     phosphor_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     post_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum, bool prepare_bloom);
	int                     downsample_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     bloom_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
	int                     chroma_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum);
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
	double                  acc_t;                      // accumulated machine time
	double                  delta_t;                    // data for delta_time
	bitmap_argb32           shadow_bitmap;              // shadow mask bitmap for post-processing shader
	texture_info *          shadow_texture;             // shadow mask texture for post-processing shader
	bitmap_argb32           lut_bitmap;
	texture_info *          lut_texture;
	bitmap_argb32           ui_lut_bitmap;
	texture_info *          ui_lut_texture;
	hlsl_options *          options;                    // current options

	IDirect3DSurface9 *     black_surface;              // black dummy surface
	IDirect3DTexture9 *     black_texture;              // black dummy texture

	bool                    recording_movie;            // ongoing movie recording
	std::unique_ptr<movie_recorder> recorder;           // HLSL post-render movie recorder

	bool                    render_snap;                // whether or not to take HLSL post-render snapshot
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
	effect *                scanline_effect;
	effect *                focus_effect;               // pointer to the focus-effect object
	effect *                phosphor_effect;            // pointer to the phosphor-effect object
	effect *                deconverge_effect;          // pointer to the deconvergence-effect object
	effect *                color_effect;               // pointer to the color-effect object
	effect *                ntsc_effect;                // pointer to the NTSC effect object
	effect *                bloom_effect;               // pointer to the bloom composite effect
	effect *                downsample_effect;          // pointer to the bloom downsample effect
	effect *                vector_effect;              // pointer to the vector-effect object
	effect *                chroma_effect;

	texture_info *          curr_texture;
	d3d_render_target *     curr_render_target;
	poly_info *             curr_poly;

	std::vector<std::unique_ptr<d3d_render_target>> m_render_target_list;

	std::vector<slider*>    internal_sliders;
	std::vector<ui::menu_item> m_sliders;
	std::vector<std::unique_ptr<slider_state>> m_core_sliders;

	static slider_desc      s_sliders[];
	static hlsl_options     last_options;               // last used options
	static char             last_system_name[16];       // last used system

	osd::dynamic_module::ptr d3dx9_dll;
	d3dx_create_effect_from_file_fn d3dx_create_effect_from_file_ptr;
};

#endif
