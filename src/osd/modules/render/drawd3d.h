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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#undef interface

#include "d3d/d3dcomm.h"
#include "sliderdirtynotifier.h"
#include "modules/lib/osdlib.h"

//============================================================
//  CONSTANTS
//============================================================

#define VERTEX_BASE_FORMAT  (D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEX2)
#define VERTEX_BUFFER_SIZE  (40960*4+4)

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct d3d_base
{
	// internal objects
	IDirect3D9 *d3dobj;
	bool        post_fx_available;

	osd::dynamic_module::ptr d3d9_dll;
};

class shaders;
struct hlsl_options;

/* renderer is the information about Direct3D for the current screen */
class renderer_d3d9 : public osd_renderer, public slider_dirty_notifier
{
public:
	renderer_d3d9(std::shared_ptr<osd_window> window);
	virtual ~renderer_d3d9();

	static bool init(running_machine &machine);
	static void exit();

	virtual int create() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override;
	virtual void record() override;
	virtual void toggle_fsfx() override;
	virtual std::vector<ui::menu_item> get_slider_list() override;
	virtual void set_sliders_dirty() override;

	int                     initialize();

	int                     device_create(HWND device_HWND);
	int                     device_create_resources();
	void                    device_delete();
	void                    device_delete_resources();
	void                    update_presentation_parameters();
	void                    update_gamma_ramp();

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
	void                    batch_vector(const render_primitive &prim);
	void                    batch_vectors(int vector_count);

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
	int                     get_adapter() const { return m_adapter; }
	int                     get_width() const { return m_width; }
	vec2f                   get_dims() const { return vec2f(m_width, m_height); }
	int                     get_height() const { return m_height; }
	int                     get_refresh() const { return m_refresh; }

	IDirect3DDevice9 *      get_device() const { return m_device; }
	D3DPRESENT_PARAMETERS * get_presentation() { return &m_presentation; }

	IDirect3DVertexBuffer9 *get_vertex_buffer() const { return m_vertexbuf; }

	void                    set_toggle(bool toggle) { m_toggle = toggle; }

	D3DFORMAT               get_screen_format() const { return m_screen_format; }
	D3DFORMAT               get_pixel_format() const { return m_pixformat; }
	D3DDISPLAYMODE          get_origmode() const { return m_origmode; }

	UINT32                  get_last_texture_flags() const { return m_last_texture_flags; }

	d3d_texture_manager *   get_texture_manager() const { return m_texture_manager; }
	texture_info *          get_default_texture();

	shaders *               get_shaders() const { return m_shaders; }

private:
	int                     m_adapter;                  // ordinal adapter number
	int                     m_width;                    // current width
	int                     m_height;                   // current height
	int                     m_refresh;                  // current refresh rate
	int                     m_create_error_count;       // number of consecutive create errors

	IDirect3DDevice9 *      m_device;                   // pointer to the Direct3DDevice object
	int                     m_gamma_supported;          // is full screen gamma supported?
	D3DPRESENT_PARAMETERS   m_presentation;             // set of presentation parameters
	D3DDISPLAYMODE          m_origmode;                 // original display mode for the adapter
	D3DFORMAT               m_pixformat;                // pixel format we are using

	IDirect3DVertexBuffer9 *m_vertexbuf;                // pointer to the vertex buffer object
	vertex *                m_lockedbuf;                // pointer to the locked vertex buffer
	int                     m_numverts;                 // number of accumulated vertices

	vertex *                m_vectorbatch;              // pointer to the vector batch buffer
	int                     m_batchindex;               // current index into the vector batch

	poly_info               m_poly[VERTEX_BUFFER_SIZE/3];// array to hold polygons as they are created
	int                     m_numpolys;                 // number of accumulated polygons

	bool                    m_toggle;                   // if we're toggle fsfx

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

	shaders *               m_shaders;                  // HLSL interface

	d3d_texture_manager *   m_texture_manager;          // texture manager
};

#endif // OSD_WINDOWS

#endif // __WIN_DRAWD3D__
