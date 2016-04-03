// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawd3d.h - Win32 Direct3D header
//
//============================================================

#pragma once

#ifndef __WIN_DRAWD3D__
#define __WIN_DRAWD3D__

#ifdef OSD_WINDOWS

#include "d3d/d3dintf.h"
#include "d3d/d3dcomm.h"

//============================================================
//  CONSTANTS
//============================================================

#define VERTEX_BASE_FORMAT  (D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2)
#define VERTEX_BUFFER_SIZE  (40960*4+4)

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct vertex;
class texture_info;
class texture_manager;
struct device;
struct vertex_buffer;
class shaders;
struct hlsl_options;
class poly_info;

/* renderer is the information about Direct3D for the current screen */
class renderer_d3d9 : public osd_renderer
{
public:
	renderer_d3d9(osd_window *window);
	virtual ~renderer_d3d9();

	static bool init(running_machine &machine);
	static void exit();

	virtual int create() override;
	virtual slider_state* get_slider_list() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override;
	virtual void record() override;
	virtual void toggle_fsfx() override;

	int                     initialize();

	int                     device_create(HWND device_HWND);
	int                     device_create_resources();
	void                    device_delete();
	void                    device_delete_resources();

	int                     device_verify_caps();
	int                     device_test_cooperative();

	int                     config_adapter_mode();
	void                    pick_best_mode();
	int                     get_adapter_for_monitor();

	int                     update_window_size();

	int                     pre_window_draw_check();
	void                    begin_frame();
	void                    end_frame();

	void                    draw_line(const render_primitive &prim);
	void                    draw_quad(const render_primitive &prim);
	void                    batch_vector(const render_primitive &prim, float line_time);
	void                    batch_vectors();

	vertex *                mesh_alloc(int numverts);

	void                    process_primitives();
	void                    primitive_flush_pending();

	void                    set_texture(texture_info *texture);
	void                    set_filter(int filter);
	void                    set_wrap(unsigned int wrap);
	void                    set_modmode(int modmode);
	void                    set_blendmode(int blendmode);
	void                    reset_render_states();

	// Setters / getters
	int                     get_adapter() { return m_adapter; }
	int                     get_width() { return m_width; }
	vec2f                   get_dims() { return vec2f(m_width, m_height); }
	int                     get_height() { return m_height; }
	int                     get_refresh() { return m_refresh; }

	device *                get_device() { return m_device; }
	present_parameters *    get_presentation() { return &m_presentation; }

	vertex_buffer *         get_vertex_buffer() { return m_vertexbuf; }
	vertex *                get_locked_buffer() { return m_lockedbuf; }
	VOID **                 get_locked_buffer_ptr() { return (VOID **)&m_lockedbuf; }
	void                    set_locked_buffer(vertex *lockedbuf) { m_lockedbuf = lockedbuf; }

	void                    set_restarting(bool restarting) { m_restarting = restarting; }
	bool                    is_mod2x_supported() { return (bool)m_mod2x_supported; }
	bool                    is_mod4x_supported() { return (bool)m_mod4x_supported; }

	D3DFORMAT               get_screen_format() { return m_screen_format; }
	D3DFORMAT               get_pixel_format() { return m_pixformat; }
	D3DDISPLAYMODE          get_origmode() { return m_origmode; }

	UINT32                  get_last_texture_flags() { return m_last_texture_flags; }

	d3d_texture_manager *   get_texture_manager() { return m_texture_manager; }
	texture_info *          get_default_texture();
	texture_info *          get_vector_texture();

	shaders *               get_shaders() { return m_shaders; }
	hlsl_options *          get_shaders_options() { return m_shaders_options; }

private:
	int                     m_adapter;                  // ordinal adapter number
	int                     m_width;                    // current width
	int                     m_height;                   // current height
	int                     m_refresh;                  // current refresh rate
	int                     m_create_error_count;       // number of consecutive create errors

	device *                m_device;                   // pointer to the Direct3DDevice object
	int                     m_gamma_supported;          // is full screen gamma supported?
	present_parameters      m_presentation;             // set of presentation parameters
	D3DDISPLAYMODE          m_origmode;                 // original display mode for the adapter
	D3DFORMAT               m_pixformat;                // pixel format we are using

	vertex_buffer *         m_vertexbuf;                // pointer to the vertex buffer object
	vertex *                m_lockedbuf;                // pointer to the locked vertex buffer
	int                     m_numverts;                 // number of accumulated vertices

	vertex *                m_vectorbatch;              // pointer to the vector batch buffer
	int                     m_batchindex;               // current index into the vector batch

	poly_info               m_poly[VERTEX_BUFFER_SIZE/3];// array to hold polygons as they are created
	int                     m_numpolys;                 // number of accumulated polygons

	bool                    m_restarting;               // if we're restarting

	int                     m_mod2x_supported;          // is D3DTOP_MODULATE2X supported?
	int                     m_mod4x_supported;          // is D3DTOP_MODULATE4X supported?
	D3DFORMAT               m_screen_format;            // format to use for screen textures

	texture_info *          m_last_texture;             // previous texture
	UINT32                  m_last_texture_flags;       // previous texture flags
	int                     m_last_blendenable;         // previous blendmode
	int                     m_last_blendop;             // previous blendmode
	int                     m_last_blendsrc;            // previous blendmode
	int                     m_last_blenddst;            // previous blendmode
	int                     m_last_filter;              // previous texture filter
	UINT32                  m_last_wrap;                // previous wrap state
	int                     m_last_modmode;             // previous texture modulation

	void *                  m_hlsl_buf;                 // HLSL vertex data
	shaders *               m_shaders;                  // HLSL interface
	hlsl_options *          m_shaders_options;          // HLSL options

	d3d_texture_manager *  m_texture_manager;          // texture manager

	int                     m_line_count;
};

#endif // OSD_WINDOWS

#endif // __WIN_DRAWD3D__
