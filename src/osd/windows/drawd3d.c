//============================================================
//
//  drawd3d.c - Win32 Direct3D implementation
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

// Useful info:
//  Windows XP/2003 shipped with DirectX 8.1
//  Windows 2000 shipped with DirectX 7a
//  Windows 98SE shipped with DirectX 6.1a
//  Windows 98 shipped with DirectX 5
//  Windows NT shipped with DirectX 3.0a
//  Windows 95 shipped with DirectX 2

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#undef interface

// MAME headers
#include "emu.h"
#include "render.h"
#include "ui.h"
#include "rendutil.h"
#include "options.h"
#include "emuopts.h"
#include "aviio.h"
#include "png.h"

// MAMEOS headers
#include "d3dintf.h"
#include "winmain.h"
#include "window.h"
#include "config.h"
#include "strconv.h"
#include "d3dcomm.h"
#include "drawd3d.h"



//============================================================
//  DEBUGGING
//============================================================

extern void mtlog_add(const char *event);



//============================================================
//  CONSTANTS
//============================================================

#define ENABLE_BORDER_PIX	(1)

enum
{
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_DYNAMIC,
	TEXTURE_TYPE_SURFACE
};



//============================================================
//  MACROS
//============================================================

#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)



//============================================================
//  GLOBALS
//============================================================

static d3d_base *				d3dintf; // FIX ME

static const line_aa_step line_aa_1step[] =
{
	{  0.00f,  0.00f,  1.00f  },
	{ 0 }
};

static const line_aa_step line_aa_4step[] =
{
	{ -0.25f,  0.00f,  0.25f  },
	{  0.25f,  0.00f,  0.25f  },
	{  0.00f, -0.25f,  0.25f  },
	{  0.00f,  0.25f,  0.25f  },
	{ 0 }
};

//============================================================
//  INLINES
//============================================================

INLINE BOOL GetClientRectExceptMenu(HWND hWnd, PRECT pRect, BOOL fullscreen)
{
	static HMENU last_menu;
	static RECT last_rect;
	static RECT cached_rect;
	HMENU menu = GetMenu(hWnd);
	BOOL result = GetClientRect(hWnd, pRect);

	if (!fullscreen || !menu)
		return result;

	// to avoid flicker use cache if we can use
	if (last_menu != menu || memcmp(&last_rect, pRect, sizeof *pRect) != 0)
	{
		last_menu = menu;
		last_rect = *pRect;

		SetMenu(hWnd, NULL);
		result = GetClientRect(hWnd, &cached_rect);
		SetMenu(hWnd, menu);
	}

	*pRect = cached_rect;
	return result;
}


INLINE UINT32 ycc_to_rgb(UINT8 y, UINT8 cb, UINT8 cr)
{
	/* original equations:

        C = Y - 16
        D = Cb - 128
        E = Cr - 128

        R = clip(( 298 * C           + 409 * E + 128) >> 8)
        G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
        B = clip(( 298 * C + 516 * D           + 128) >> 8)

        R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
        G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
        B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)

        R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
        G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
        B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

        R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
        G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
        B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)
    */
	int r, g, b, common;

	common = 298 * y - 298 * 16;
	r = (common +                        409 * cr - 409 * 128 + 128) >> 8;
	g = (common - 100 * cb + 100 * 128 - 208 * cr + 208 * 128 + 128) >> 8;
	b = (common + 516 * cb - 516 * 128                        + 128) >> 8;

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	return MAKE_ARGB(0xff, r, g, b);
}


INLINE UINT32 texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	return (FPTR)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}


INLINE void set_texture(d3d_info *d3d, d3d_texture_info *texture)
{
	HRESULT result;
	if (texture != d3d->last_texture)
	{
		d3d->last_texture = texture;
		d3d->last_texture_flags = (texture == NULL ? 0 : texture->flags);
		result = (*d3dintf->device.set_texture)(d3d->device, 0, (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
		if (d3d->hlsl != NULL)
			d3d->hlsl->set_texture(texture);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture call\n", (int)result);
	}
}


INLINE void set_filter(d3d_info *d3d, int filter)
{
	HRESULT result;
	if (filter != d3d->last_filter)
	{
		d3d->last_filter = filter;
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MINFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MAGFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MINFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MAGFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}


INLINE void set_wrap(d3d_info *d3d, int wrap)
{
	HRESULT result;
	if (wrap != d3d->last_wrap)
	{
		d3d->last_wrap = wrap;
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSU, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSV, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSU, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSV, wrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}


INLINE void set_modmode(d3d_info *d3d, DWORD modmode)
{
	HRESULT result;
	if (modmode != d3d->last_modmode)
	{
		d3d->last_modmode = modmode;
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, D3DTSS_COLOROP, modmode);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, D3DTSS_COLOROP, modmode);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}


INLINE void set_blendmode(d3d_info *d3d, int blendmode)
{
	HRESULT result;
	int blendenable;
	int blendop;
	int blendsrc;
	int blenddst;

	// choose the parameters
	switch (blendmode)
	{
		default:
		case BLENDMODE_NONE:			blendenable = FALSE;	blendop = D3DBLENDOP_ADD;	blendsrc = D3DBLEND_SRCALPHA;	blenddst = D3DBLEND_INVSRCALPHA;	break;
		case BLENDMODE_ALPHA:			blendenable = TRUE;		blendop = D3DBLENDOP_ADD;	blendsrc = D3DBLEND_SRCALPHA;	blenddst = D3DBLEND_INVSRCALPHA;	break;
		case BLENDMODE_RGB_MULTIPLY:	blendenable = TRUE;		blendop = D3DBLENDOP_ADD;	blendsrc = D3DBLEND_DESTCOLOR;	blenddst = D3DBLEND_ZERO;			break;
		case BLENDMODE_ADD:				blendenable = TRUE;		blendop = D3DBLENDOP_ADD;	blendsrc = D3DBLEND_SRCALPHA;	blenddst = D3DBLEND_ONE;			break;
	}

	// adjust the bits that changed
	if (blendenable != d3d->last_blendenable)
	{
		d3d->last_blendenable = blendenable;
		result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ALPHABLENDENABLE, blendenable);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blendop != d3d->last_blendop)
	{
		d3d->last_blendop = blendop;
		result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_BLENDOP, blendop);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blendsrc != d3d->last_blendsrc)
	{
		d3d->last_blendsrc = blendsrc;
		result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_SRCBLEND, blendsrc);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blenddst != d3d->last_blenddst)
	{
		d3d->last_blenddst = blenddst;
		result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_DESTBLEND, blenddst);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}
}


INLINE void reset_render_states(d3d_info *d3d)
{
	// this ensures subsequent calls to the above setters will force-update the data
	d3d->last_texture = (d3d_texture_info *)~0;
	d3d->last_filter = -1;
	d3d->last_blendenable = -1;
	d3d->last_blendop = -1;
	d3d->last_blendsrc = -1;
	d3d->last_blenddst = -1;
	d3d->last_wrap = -1;
}



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawd3d_exit(void);
static int drawd3d_window_init(win_window_info *window);
static void drawd3d_window_destroy(win_window_info *window);
static render_primitive_list *drawd3d_window_get_primitives(win_window_info *window);
static void drawd3d_window_save(win_window_info *window);
static void drawd3d_window_record(win_window_info *window);
static int drawd3d_window_draw(win_window_info *window, HDC dc, int update);

// devices
static int device_create(win_window_info *window);
static int device_create_resources(d3d_info *d3d);
static void device_delete(d3d_info *d3d);
static void device_delete_resources(d3d_info *d3d);
static int device_verify_caps(d3d_info *d3d, win_window_info *window);
static int device_test_cooperative(d3d_info *d3d);

// video modes
static int config_adapter_mode(win_window_info *window);
static int get_adapter_for_monitor(d3d_info *d3d, win_monitor_info *monitor);
static void pick_best_mode(win_window_info *window);
static int update_window_size(win_window_info *window);

// drawing
static void draw_line(d3d_info *d3d, const render_primitive *prim);
static void draw_quad(d3d_info *d3d, const render_primitive *prim);

// primitives
static d3d_vertex *primitive_alloc(d3d_info *d3d, int numverts);
static void primitive_flush_pending(d3d_info *d3d);

// textures
static void texture_compute_size(d3d_info *d3d, int texwidth, int texheight, d3d_texture_info *texture);
static void texture_set_data(d3d_info *d3d, d3d_texture_info *texture, const render_texinfo *texsource, UINT32 flags);
static void texture_prescale(d3d_info *d3d, d3d_texture_info *texture);
static d3d_texture_info *texture_find(d3d_info *d3d, const render_primitive *prim);
static void texture_update(d3d_info *d3d, const render_primitive *prim);

//============================================================
//  drawd3d_init
//============================================================

int drawd3d_init(running_machine &machine, win_draw_callbacks *callbacks)
{
	int version = downcast<windows_options &>(machine.options()).d3d_version();
	d3dintf = NULL;

	// try Direct3D 9 if requested
	if (version >= 9)
		d3dintf = drawd3d9_init();

#if DIRECT3D_VERSION < 0x0900
	// if that didn't work, try Direct3D 8
	if (d3dintf == NULL && version >= 8)
		d3dintf = drawd3d8_init();
#endif

	// if we failed, note the error
	if (d3dintf == NULL)
	{
		mame_printf_error("Unable to initialize Direct3D.\n");
		return 1;
	}

	// fill in the callbacks
	callbacks->exit = drawd3d_exit;
	callbacks->window_init = drawd3d_window_init;
	callbacks->window_get_primitives = drawd3d_window_get_primitives;
	callbacks->window_draw = drawd3d_window_draw;
	callbacks->window_save = drawd3d_window_save;
	callbacks->window_record = drawd3d_window_record;
	callbacks->window_destroy = drawd3d_window_destroy;
	return 0;
}



//============================================================
//  drawd3d_exit
//============================================================

static void drawd3d_exit(void)
{
	if (d3dintf != NULL)
		(*d3dintf->d3d.release)(d3dintf);
}



//============================================================
//  drawd3d_window_init
//============================================================

static int drawd3d_window_init(win_window_info *window)
{
	d3d_info *d3d;

	// allocate memory for our structures
	d3d = global_alloc_clear(d3d_info);
	window->drawdata = d3d;
	d3d->window = window;
	d3d->hlsl = NULL;

	// experimental: load a PNG to use for vector rendering; it is treated
	// as a brightness map
	emu_file file(window->machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(d3d->vector_bitmap, file, NULL, "vector.png");
	if (d3d->vector_bitmap.valid())
	{
		d3d->vector_bitmap.fill(MAKE_ARGB(0xff,0xff,0xff,0xff));
		render_load_png(d3d->vector_bitmap, file, NULL, "vector.png", true);
	}

	d3d->default_bitmap.allocate(8, 8);
	d3d->default_bitmap.fill(MAKE_ARGB(0xff,0xff,0xff,0xff));

	// configure the adapter for the mode we want
	if (config_adapter_mode(window))
		goto error;

	// create the device immediately for the full screen case (defer for window mode)
	if (window->fullscreen && device_create(window))
		goto error;

	return 0;

error:
	drawd3d_window_destroy(window);
	mame_printf_error("Unable to initialize Direct3D.\n");
	return 1;
}



static void drawd3d_window_record(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;

	if (d3d->hlsl != NULL)
		d3d->hlsl->window_record();
}

static void drawd3d_window_save(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;

	if (d3d->hlsl != NULL)
		d3d->hlsl->window_save();
}


//============================================================
//  drawd3d_window_destroy
//============================================================

static void drawd3d_window_destroy(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;

	// skip if nothing
	if (d3d == NULL)
		return;

	if (d3d->hlsl->recording())
		d3d->hlsl->window_record();

	// delete the device
	device_delete(d3d);

	// free the memory in the window
	global_free(d3d);
	window->drawdata = NULL;
}



//============================================================
//  drawd3d_window_get_primitives
//============================================================

static render_primitive_list *drawd3d_window_get_primitives(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;
	RECT client;

	GetClientRectExceptMenu(window->hwnd, &client, window->fullscreen);
	if (rect_width(&client) > 0 && rect_height(&client) > 0)
	{
		window->target->set_bounds(rect_width(&client), rect_height(&client), winvideo_monitor_get_aspect(window->monitor));
		window->target->set_max_update_rate((d3d->refresh == 0) ? d3d->origmode.RefreshRate : d3d->refresh);
	}
	return &window->target->get_primitives();
}



//============================================================
//  drawd3d_window_draw
//============================================================

static int drawd3d_window_draw(win_window_info *window, HDC dc, int update)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;
	render_primitive *prim;
	HRESULT result;

	// if we're in the middle of resizing, leave things alone
	if (window->resize_state == RESIZE_STATE_RESIZING)
		return 0;

	// if we haven't been created, just punt
	if (d3d == NULL)
		return 1;

	// if we have a device, check the cooperative level
	if (d3d->device != NULL)
	{
		int error = device_test_cooperative(d3d);
		if (error)
			return 1;
	}

	// in window mode, we need to track the window size
	if (!window->fullscreen || d3d->device == NULL)
	{
		// if the size changes, skip this update since the render target will be out of date
		if (update_window_size(window))
			return 0;

		// if we have no device, after updating the size, return an error so GDI can try
		if (d3d->device == NULL)
			return 1;
	}

mtlog_add("drawd3d_window_draw: begin");
	result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	d3d->hlsl->record_texture();

	// first update any textures
	window->primlist->acquire_lock();
	for (prim = window->primlist->first(); prim != NULL; prim = prim->next())
	{
		if (prim->texture.base != NULL)
		{
			texture_update(d3d, prim);
		}
		else if(d3d->hlsl->vector_enabled() && PRIMFLAG_GET_VECTORBUF(prim->flags))
		{
			if (!d3d->hlsl->get_vector_target(d3d))
			{
				d3d->hlsl->create_vector_target(d3d, prim);
			}
		}
	}

	// begin the scene
mtlog_add("drawd3d_window_draw: begin_scene");
	result = (*d3dintf->device.begin_scene)(d3d->device);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device begin_scene call\n", (int)result);

	d3d->lockedbuf = NULL;

	// loop over primitives
	if(d3d->hlsl->enabled())
	{
		d3d->hlsl_buf = (void*)primitive_alloc(d3d, 6);
		d3d->hlsl->init_fsfx_quad(d3d->hlsl_buf);
	}

mtlog_add("drawd3d_window_draw: primitive loop begin");
	for (prim = window->primlist->first(); prim != NULL; prim = prim->next())
		switch (prim->type)
		{
			case render_primitive::LINE:
				draw_line(d3d, prim);
				break;

			case render_primitive::QUAD:
				draw_quad(d3d, prim);
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
mtlog_add("drawd3d_window_draw: primitive loop end");
	window->primlist->release_lock();

	// flush any pending polygons
mtlog_add("drawd3d_window_draw: flush_pending begin");
	primitive_flush_pending(d3d);
mtlog_add("drawd3d_window_draw: flush_pending end");

	// finish the scene
mtlog_add("drawd3d_window_draw: end_scene begin");
	result = (*d3dintf->device.end_scene)(d3d->device);
mtlog_add("drawd3d_window_draw: end_scene end");
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device end_scene call\n", (int)result);

	// present the current buffers
mtlog_add("drawd3d_window_draw: present begin");
	result = (*d3dintf->device.present)(d3d->device, NULL, NULL, NULL, NULL, 0);
mtlog_add("drawd3d_window_draw: present end");
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device present call\n", (int)result);

	d3d->hlsl->frame_complete();

	return 0;
}



//============================================================
//  device_create
//============================================================

static int device_create(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;
	HRESULT result;
	int verify;

	// if a device exists, free it
	if (d3d->device != NULL)
		device_delete(d3d);

	// verify the caps
	verify = device_verify_caps(d3d, window);
	if (verify == 2)
	{
		mame_printf_error("Error: Device does not meet minimum requirements for Direct3D rendering\n");
		return 1;
	}
	if (verify == 1)
		mame_printf_warning("Warning: Device may not perform well for Direct3D rendering\n");

	// verify texture formats
	result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8);
	if (result != D3D_OK)
	{
		mame_printf_error("Error: A8R8G8B8 format textures not supported\n");
		return 1;
	}

	// pick a YUV texture format
	d3d->yuv_format = D3DFMT_UYVY;
	result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, 0, D3DRTYPE_TEXTURE, D3DFMT_UYVY);
	if (result != D3D_OK)
	{
		d3d->yuv_format = D3DFMT_YUY2;
		result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, 0, D3DRTYPE_TEXTURE, D3DFMT_YUY2);
		if (result != D3D_OK)
			d3d->yuv_format = D3DFMT_A8R8G8B8;
	}
	mame_printf_verbose("Direct3D: YUV format = %s\n", (d3d->yuv_format == D3DFMT_YUY2) ? "YUY2" : (d3d->yuv_format == D3DFMT_UYVY) ? "UYVY" : "RGB");

try_again:
	// try for XRGB first
	d3d->screen_format = D3DFMT_X8R8G8B8;
	result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, d3d->dynamic_supported ? D3DUSAGE_DYNAMIC : 0, D3DRTYPE_TEXTURE, d3d->screen_format);
	if (result != D3D_OK)
	{
		// if not, try for ARGB
		d3d->screen_format = D3DFMT_A8R8G8B8;
		result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, d3d->dynamic_supported ? D3DUSAGE_DYNAMIC : 0, D3DRTYPE_TEXTURE, d3d->screen_format);
		if (result != D3D_OK && d3d->dynamic_supported)
		{
			d3d->dynamic_supported = FALSE;
			goto try_again;
		}
		if (result != D3D_OK)
		{
			mame_printf_error("Error: unable to configure a screen texture format\n");
			return 1;
		}
	}

	// initialize the D3D presentation parameters
	memset(&d3d->presentation, 0, sizeof(d3d->presentation));
	d3d->presentation.BackBufferWidth				= d3d->width;
	d3d->presentation.BackBufferHeight				= d3d->height;
	d3d->presentation.BackBufferFormat				= d3d->pixformat;
	d3d->presentation.BackBufferCount				= video_config.triplebuf ? 2 : 1;
	d3d->presentation.MultiSampleType				= D3DMULTISAMPLE_NONE;
	d3d->presentation.SwapEffect					= D3DSWAPEFFECT_DISCARD;
	d3d->presentation.hDeviceWindow					= window->hwnd;
	d3d->presentation.Windowed						= !window->fullscreen || win_has_menu(window);
	d3d->presentation.EnableAutoDepthStencil		= FALSE;
	d3d->presentation.AutoDepthStencilFormat		= D3DFMT_D16;
	d3d->presentation.Flags							= 0;
	d3d->presentation.FullScreen_RefreshRateInHz	= d3d->refresh;
	d3d->presentation.PresentationInterval			= ((video_config.triplebuf && window->fullscreen) || video_config.waitvsync || video_config.syncrefresh) ?
														D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

	// create the D3D device
	result = (*d3dintf->d3d.create_device)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, win_window_list->hwnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &d3d->presentation, &d3d->device);
	if (result != D3D_OK)
	{
		// if we got a "DEVICELOST" error, it may be transitory; count it and only fail if
		// we exceed a threshold
		if (result == D3DERR_DEVICELOST)
		{
			d3d->create_error_count++;
			if (d3d->create_error_count < 10)
				return 0;
		}

		//  fatal error if we just can't do it
		mame_printf_error("Unable to create the Direct3D device (%08X)\n", (UINT32)result);
		return 1;
	}
	d3d->create_error_count = 0;
	mame_printf_verbose("Direct3D: Device created at %dx%d\n", d3d->width, d3d->height);

	// set the max texture size
	window->target->set_max_texture_size(d3d->texture_max_width, d3d->texture_max_height);
	mame_printf_verbose("Direct3D: Max texture size = %dx%d\n", (int)d3d->texture_max_width, (int)d3d->texture_max_height);

	// set the gamma if we need to
	if (window->fullscreen)
	{
		// only set the gamma if it's not 1.0f
		windows_options &options = downcast<windows_options &>(window->machine().options());
		float brightness = options.full_screen_brightness();
		float contrast = options.full_screen_contrast();
		float gamma = options.full_screen_gamma();
		if (brightness != 1.0f || contrast != 1.0f || gamma != 1.0f)
		{
			// warn if we can't do it
			if (!d3d->gamma_supported)
				mame_printf_warning("Direct3D: Warning - device does not support full screen gamma correction.\n");
			else
			{
				D3DGAMMARAMP ramp;
				int i;

				// create a standard ramp and set it
				for (i = 0; i < 256; i++)
					ramp.red[i] = ramp.green[i] = ramp.blue[i] = apply_brightness_contrast_gamma(i, brightness, contrast, gamma) << 8;
				(*d3dintf->device.set_gamma_ramp)(d3d->device, 0, &ramp);
			}
		}
	}

	int ret = d3d->hlsl->create_resources(false);
	if (ret != 0)
	    return ret;

	return device_create_resources(d3d);
}



//============================================================
//  device_create_resources
//============================================================

static int device_create_resources(d3d_info *d3d)
{
	HRESULT result;

	// allocate a vertex buffer to use
	result = (*d3dintf->device.create_vertex_buffer)(d3d->device,
				sizeof(d3d_vertex) * VERTEX_BUFFER_SIZE,
				D3DUSAGE_DYNAMIC | D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY,
				VERTEX_BASE_FORMAT | ((d3d->hlsl->enabled() && d3dintf->post_fx_available) ? D3DFVF_XYZW : D3DFVF_XYZRHW), D3DPOOL_DEFAULT, &d3d->vertexbuf);
	if (result != D3D_OK)
	{
		mame_printf_error("Error creating vertex buffer (%08X)", (UINT32)result);
		return 1;
	}

	// set the vertex format
	result = (*d3dintf->device.set_vertex_format)(d3d->device, (D3DFORMAT)(VERTEX_BASE_FORMAT | ((d3d->hlsl->enabled() && d3dintf->post_fx_available) ? D3DFVF_XYZW : D3DFVF_XYZRHW)));
	if (result != D3D_OK)
	{
		mame_printf_error("Error setting vertex format (%08X)", (UINT32)result);
		return 1;
	}

	// set the fixed render state
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ZENABLE, D3DZB_FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_FILLMODE, D3DFILL_SOLID);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_SHADEMODE, D3DSHADE_FLAT);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ZWRITEENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ALPHATESTENABLE, TRUE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_LASTPIXEL, TRUE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_CULLMODE, D3DCULL_NONE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ZFUNC, D3DCMP_LESS);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ALPHAREF, 0);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_DITHERENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_FOGENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_SPECULARENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_STENCILENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_WRAP0, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_CLIPPING, TRUE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_LIGHTING, FALSE);
	result = (*d3dintf->device.set_render_state)(d3d->device, D3DRS_COLORVERTEX, TRUE);

	result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(d3d->device, 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	// reset the local states to force updates
	reset_render_states(d3d);

	// clear the buffer
	result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	result = (*d3dintf->device.present)(d3d->device, NULL, NULL, NULL, NULL, 0);

	if (d3d->default_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = d3d->default_bitmap.raw_pixptr(0);
		texture.rowpixels = d3d->default_bitmap.rowpixels();
		texture.width = d3d->default_bitmap.width();
		texture.height = d3d->default_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		d3d->default_texture = texture_create(d3d, &texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32));
	}

	// experimental: if we have a vector bitmap, create a texture for it
	if (d3d->vector_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = &d3d->vector_bitmap.pix32(0);
		texture.rowpixels = d3d->vector_bitmap.rowpixels();
		texture.width = d3d->vector_bitmap.width();
		texture.height = d3d->vector_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		d3d->vector_texture = texture_create(d3d, &texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32));
	}

	return 0;
}



//============================================================
//  device_delete
//============================================================

static void device_delete(d3d_info *d3d)
{
	// free our effects
	d3d->hlsl->delete_resources(false);

	// delete the HLSL interface
	global_free(d3d->hlsl);

	// free our base resources
	device_delete_resources(d3d);

	// free the device itself
	if (d3d->device != NULL)
		(*d3dintf->device.release)(d3d->device);
	d3d->device = NULL;
}



//============================================================
//  device_delete_resources
//============================================================

static void device_delete_resources(d3d_info *d3d)
{
	// free all textures
	while (d3d->texlist != NULL)
	{
		d3d_texture_info *tex = d3d->texlist;
		d3d->texlist = tex->next;
		if (tex->d3dfinaltex != NULL)
		{
			if (tex->d3dtex == tex->d3dfinaltex) tex->d3dtex = NULL;
			(*d3dintf->texture.release)(tex->d3dfinaltex);
			tex->d3dfinaltex = NULL;
		}
		if (tex->d3dtex != NULL)
		{
			(*d3dintf->texture.release)(tex->d3dtex);
			tex->d3dtex = NULL;
		}
		if (tex->d3dsurface != NULL)
		{
			(*d3dintf->surface.release)(tex->d3dsurface);
			tex->d3dsurface = NULL;
		}
		global_free(tex);
	}

	// free the vertex buffer
	if (d3d->vertexbuf != NULL)
		(*d3dintf->vertexbuf.release)(d3d->vertexbuf);
	d3d->vertexbuf = NULL;

	global_free(d3d->default_texture);
	d3d->default_texture = NULL;
}



//============================================================
//  device_verify_caps
//============================================================

static int device_verify_caps(d3d_info *d3d, win_window_info *window)
{
	int retval = 0;
	HRESULT result;
	DWORD tempcaps;

	// fetch a few core caps
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_TEXTURE_CAPS, &d3d->texture_caps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_ASPECT, &d3d->texture_max_aspect);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_WIDTH, &d3d->texture_max_width);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_HEIGHT, &d3d->texture_max_height);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);

	d3d->hlsl = global_alloc_clear(hlsl_info);
	d3d->hlsl->init(d3dintf, window);

	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_MAX_PS30_INSN_SLOTS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D Error %08X during get_caps_dword call\n", (int)result);
	if(tempcaps < 512)
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support Pixel Shader 3.0, falling back to non-PS rendering\n");
		d3dintf->post_fx_available = false;
	}

	// verify presentation capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_PRESENTATION_INTERVALS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPRESENT_INTERVAL_IMMEDIATE))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support immediate presentations\n");
		retval = 2;
	}
	if (!(tempcaps & D3DPRESENT_INTERVAL_ONE))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support per-refresh presentations\n");
		retval = 2;
	}

	// verify device capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_DEV_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DDEVCAPS_CANRENDERAFTERFLIP))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support queued rendering after a page flip\n");
		retval = 1;
	}
	if (!(tempcaps & D3DDEVCAPS_HWRASTERIZATION))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support hardware rasterization\n");
		retval = 1;
	}

	// verify source blend capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_SRCBLEND_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPBLENDCAPS_SRCALPHA))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support source alpha blending with source alpha\n");
		retval = 2;
	}
	if (!(tempcaps & D3DPBLENDCAPS_DESTCOLOR))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support source alpha blending with destination color\n");
		retval = 2;
	}

	// verify destination blend capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_DSTBLEND_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPBLENDCAPS_ZERO))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support dest alpha blending with zero\n");
		retval = 2;
	}
	if (!(tempcaps & D3DPBLENDCAPS_ONE))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support dest alpha blending with one\n");
		retval = 2;
	}
	if (!(tempcaps & D3DPBLENDCAPS_INVSRCALPHA))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support dest alpha blending with inverted source alpha\n");
		retval = 2;
	}

	// verify texture capabilities
	if (!(d3d->texture_caps & D3DPTEXTURECAPS_ALPHA))
	{
		mame_printf_verbose("Direct3D: Error - Device does not support texture alpha\n");
		retval = 2;
	}

	// verify texture filter capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_TEXTURE_FILTER_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPTFILTERCAPS_MAGFPOINT))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support point-sample texture filtering for magnification\n");
		retval = 1;
	}
	if (!(tempcaps & D3DPTFILTERCAPS_MAGFLINEAR))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support bilinear texture filtering for magnification\n");
		retval = 1;
	}
	if (!(tempcaps & D3DPTFILTERCAPS_MINFPOINT))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support point-sample texture filtering for minification\n");
		retval = 1;
	}
	if (!(tempcaps & D3DPTFILTERCAPS_MINFLINEAR))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support bilinear texture filtering for minification\n");
		retval = 1;
	}

	// verify texture addressing capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_TEXTURE_ADDRESS_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPTADDRESSCAPS_CLAMP))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support texture clamping\n");
		retval = 1;
	}
	if (!(tempcaps & D3DPTADDRESSCAPS_WRAP))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support texture wrapping\n");
		retval = 1;
	}

	// verify texture operation capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_TEXTURE_OP_CAPS, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DTEXOPCAPS_MODULATE))
	{
		mame_printf_verbose("Direct3D: Warning - Device does not support texture modulation\n");
		retval = 1;
	}

	// set a simpler flag to indicate mod2x and mod4x texture modes
	d3d->mod2x_supported = ((tempcaps & D3DTEXOPCAPS_MODULATE2X) != 0);
	d3d->mod4x_supported = ((tempcaps & D3DTEXOPCAPS_MODULATE4X) != 0);

	// set a simpler flag to indicate we can use a gamma ramp
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_CAPS2, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	d3d->dynamic_supported = ((tempcaps & D3DCAPS2_DYNAMICTEXTURES) != 0);
	d3d->gamma_supported = ((tempcaps & D3DCAPS2_FULLSCREENGAMMA) != 0);
	if (d3d->dynamic_supported) mame_printf_verbose("Direct3D: Using dynamic textures\n");

	// set a simpler flag to indicate we can use StretchRect
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, CAPS_STRETCH_RECT_FILTER, &tempcaps);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	d3d->stretch_supported = ((tempcaps & D3DPTFILTERCAPS_MAGFPOINT) != 0);
	if (d3d->stretch_supported && video_config.prescale > 1) mame_printf_verbose("Direct3D: Using StretchRect for prescaling\n");

	return retval;
}



//============================================================
//  device_test_cooperative
//============================================================

static int device_test_cooperative(d3d_info *d3d)
{
	HRESULT result;

	// check our current status; if we lost the device, punt to GDI
	result = (*d3dintf->device.test_cooperative_level)(d3d->device);
	if (result == D3DERR_DEVICELOST)
		return 1;

	// if we're able to reset ourselves, try it
	if (result == D3DERR_DEVICENOTRESET)
	{
		mame_printf_verbose("Direct3D: resetting device\n");

		// free all existing resources and call reset on the device
		device_delete_resources(d3d);
		d3d->hlsl->delete_resources(true);
		result = (*d3dintf->device.reset)(d3d->device, &d3d->presentation);

		// if it didn't work, punt to GDI
		if (result != D3D_OK)
		{
			printf("Unable to reset, result %08x\n", (UINT32)result);
			return 1;
		}

		// try to create the resources again; if that didn't work, delete the whole thing
		if (device_create_resources(d3d))
		{
			mame_printf_verbose("Direct3D: failed to recreate resources for device; failing permanently\n");
			device_delete(d3d);
			return 1;
		}

		if (d3d->hlsl->create_resources(true))
		{
			mame_printf_verbose("Direct3D: failed to recreate HLSL resources for device; failing permanently\n");
			device_delete(d3d);
			return 1;
		}
	}
	return 0;
}



//============================================================
//  config_adapter_mode
//============================================================

static int config_adapter_mode(win_window_info *window)
{
	d3d_adapter_identifier identifier;
	d3d_info *d3d = (d3d_info *)window->drawdata;
	HRESULT result;

	// choose the monitor number
	d3d->adapter = get_adapter_for_monitor(d3d, window->monitor);

	// get the identifier
	result = (*d3dintf->d3d.get_adapter_identifier)(d3dintf, d3d->adapter, 0, &identifier);
	if (result != D3D_OK)
	{
		mame_printf_error("Error getting identifier for adapter #%d\n", d3d->adapter);
		return 1;
	}
	mame_printf_verbose("Direct3D: Configuring adapter #%d = %s\n", d3d->adapter, identifier.Description);

	// get the current display mode
	result = (*d3dintf->d3d.get_adapter_display_mode)(d3dintf, d3d->adapter, &d3d->origmode);
	if (result != D3D_OK)
	{
		mame_printf_error("Error getting mode for adapter #%d\n", d3d->adapter);
		return 1;
	}

	// choose a resolution: window mode case
	if (!window->fullscreen || !video_config.switchres || win_has_menu(window))
	{
		RECT client;

		// bounds are from the window client rect
		GetClientRectExceptMenu(window->hwnd, &client, window->fullscreen);
		d3d->width = client.right - client.left;
		d3d->height = client.bottom - client.top;

		// pix format is from the current mode
		d3d->pixformat = d3d->origmode.Format;
		d3d->refresh = 0;

		// make sure it's a pixel format we can get behind
		if (d3d->pixformat != D3DFMT_X1R5G5B5 && d3d->pixformat != D3DFMT_R5G6B5 && d3d->pixformat != D3DFMT_X8R8G8B8)
		{
			char *utf8_device = utf8_from_tstring(window->monitor->info.szDevice);
			if (utf8_device != NULL)
			{
				mame_printf_error("Device %s currently in an unsupported mode\n", utf8_device);
				osd_free(utf8_device);
			}
			return 1;
		}
	}

	// choose a resolution: full screen mode case
	else
	{
		// default to the current mode exactly
		d3d->width = d3d->origmode.Width;
		d3d->height = d3d->origmode.Height;
		d3d->pixformat = d3d->origmode.Format;
		d3d->refresh = d3d->origmode.RefreshRate;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			pick_best_mode(window);
	}

	// see if we can handle the device type
	result = (*d3dintf->d3d.check_device_type)(d3dintf, d3d->adapter, D3DDEVTYPE_HAL, d3d->pixformat, d3d->pixformat, !window->fullscreen);
	if (result != D3D_OK)
	{
		char *utf8_device = utf8_from_tstring(window->monitor->info.szDevice);
		if (utf8_device != NULL)
		{
			mame_printf_error("Proposed video mode not supported on device %s\n", utf8_device);
			osd_free(utf8_device);
		}
		return 1;
	}
	return 0;
}



//============================================================
//  get_adapter_for_monitor
//============================================================

static int get_adapter_for_monitor(d3d_info *d3d, win_monitor_info *monitor)
{
	int maxadapter = (*d3dintf->d3d.get_adapter_count)(d3dintf);
	int adapternum;

	// iterate over adapters until we error or find a match
	for (adapternum = 0; adapternum < maxadapter; adapternum++)
	{
		HMONITOR curmonitor;

		// get the monitor for this adapter
		curmonitor = (*d3dintf->d3d.get_adapter_monitor)(d3dintf, adapternum);

		// if we match the proposed monitor, this is it
		if (curmonitor == monitor->handle)
			return adapternum;
	}

	// default to the default
	return D3DADAPTER_DEFAULT;
}



//============================================================
//  pick_best_mode
//============================================================

static void pick_best_mode(win_window_info *window)
{
	double target_refresh = 60.0;
	INT32 target_width, target_height;
	d3d_info *d3d = (d3d_info *)window->drawdata;
	INT32 minwidth, minheight;
	float best_score = 0.0f;
	int maxmodes;
	int modenum;

	// determine the refresh rate of the primary screen
	const screen_device *primary_screen = window->machine().config().first_screen();
	if (primary_screen != NULL)
		target_refresh = ATTOSECONDS_TO_HZ(primary_screen->refresh_attoseconds());

	// determine the minimum width/height for the selected target
	// note: technically we should not be calling this from an alternate window
	// thread; however, it is only done during init time, and the init code on
	// the main thread is waiting for us to finish, so it is safe to do so here
	window->target->compute_minimum_size(minwidth, minheight);

	// use those as the target for now
	target_width = minwidth;
	target_height = minheight;

	// determine the maximum number of modes
	maxmodes = (*d3dintf->d3d.get_adapter_mode_count)(d3dintf, d3d->adapter, D3DFMT_X8R8G8B8);

	// enumerate all the video modes and find the best match
	mame_printf_verbose("Direct3D: Selecting video mode...\n");
	for (modenum = 0; modenum < maxmodes; modenum++)
	{
		float size_score, refresh_score, final_score;
		D3DDISPLAYMODE mode;
		HRESULT result;

		// check this mode
		result = (*d3dintf->d3d.enum_adapter_modes)(d3dintf, d3d->adapter, D3DFMT_X8R8G8B8, modenum, &mode);
		if (result != D3D_OK)
			break;

		// skip non-32 bit modes
		if (mode.Format != D3DFMT_X8R8G8B8)
			continue;

		// compute initial score based on difference between target and current
		size_score = 1.0f / (1.0f + fabs((float)(mode.Width - target_width)) + fabs((float)(mode.Height - target_height)));

		// if the mode is too small, give a big penalty
		if (mode.Width < minwidth || mode.Height < minheight)
			size_score *= 0.01f;

		// if mode is smaller than we'd like, it only scores up to 0.1
		if (mode.Width < target_width || mode.Height < target_height)
			size_score *= 0.1f;

		// if we're looking for a particular mode, that's a winner
		if (mode.Width == window->maxwidth && mode.Height == window->maxheight)
			size_score = 2.0f;

		// compute refresh score
		refresh_score = 1.0f / (1.0f + fabs((double)mode.RefreshRate - target_refresh));

		// if refresh is smaller than we'd like, it only scores up to 0.1
		if ((double)mode.RefreshRate < target_refresh)
			refresh_score *= 0.1f;

		// if we're looking for a particular refresh, make sure it matches
		if (mode.RefreshRate == window->refresh)
			refresh_score = 2.0f;

		// weight size and refresh equally
		final_score = size_score + refresh_score;

		// best so far?
		mame_printf_verbose("  %4dx%4d@%3dHz -> %f\n", mode.Width, mode.Height, mode.RefreshRate, final_score * 1000.0f);
		if (final_score > best_score)
		{
			best_score = final_score;
			d3d->width = mode.Width;
			d3d->height = mode.Height;
			d3d->pixformat = mode.Format;
			d3d->refresh = mode.RefreshRate;
		}
	}
	mame_printf_verbose("Direct3D: Mode selected = %4dx%4d@%3dHz\n", d3d->width, d3d->height, d3d->refresh);
}



//============================================================
//  update_window_size
//============================================================

static int update_window_size(win_window_info *window)
{
	d3d_info *d3d = (d3d_info *)window->drawdata;
	RECT client;

	// get the current window bounds
	GetClientRectExceptMenu(window->hwnd, &client, window->fullscreen);

	// if we have a device and matching width/height, nothing to do
	if (d3d->device != NULL && rect_width(&client) == d3d->width && rect_height(&client) == d3d->height)
	{
		// clear out any pending resizing if the area didn't change
		if (window->resize_state == RESIZE_STATE_PENDING)
			window->resize_state = RESIZE_STATE_NORMAL;
		return FALSE;
	}

	// if we're in the middle of resizing, leave it alone as well
	if (window->resize_state == RESIZE_STATE_RESIZING)
		return FALSE;

	// set the new bounds and create the device again
	d3d->width = rect_width(&client);
	d3d->height = rect_height(&client);
	if (device_create(window))
		return FALSE;

	// reset the resize state to normal, and indicate we made a change
	window->resize_state = RESIZE_STATE_NORMAL;
	return TRUE;
}



//============================================================
//  draw_line
//============================================================

static void draw_line(d3d_info *d3d, const render_primitive *prim)
{
	const line_aa_step *step = line_aa_4step;
	render_bounds b0, b1;
	d3d_vertex *vertex;
	INT32 r, g, b, a;
	d3d_poly_info *poly;
	float effwidth;
	DWORD color;
	int i;

	// compute the effective width based on the direction of the line
	effwidth = prim->width;
	if (effwidth < 0.5f)
		effwidth = 0.5f;

	// determine the bounds of a quad to draw this line
	render_line_to_quad(&prim->bounds, effwidth, &b0, &b1);

	// iterate over AA steps
	for (step = PRIMFLAG_GET_ANTIALIAS(prim->flags) ? line_aa_4step : line_aa_1step; step->weight != 0; step++)
	{
		// get a pointer to the vertex buffer
		vertex = primitive_alloc(d3d, 4);
		if (vertex == NULL)
			return;

		// rotate the unit vector by 135 degrees and add to point 0
		vertex[0].x = b0.x0 + step->xoffs;
		vertex[0].y = b0.y0 + step->yoffs;

		// rotate the unit vector by -135 degrees and add to point 0
		vertex[1].x = b0.x1 + step->xoffs;
		vertex[1].y = b0.y1 + step->yoffs;

		// rotate the unit vector by 45 degrees and add to point 1
		vertex[2].x = b1.x0 + step->xoffs;
		vertex[2].y = b1.y0 + step->yoffs;

		// rotate the unit vector by -45 degrees and add to point 1
		vertex[3].x = b1.x1 + step->xoffs;
		vertex[3].y = b1.y1 + step->yoffs;

		// determine the color of the line
		r = (INT32)(prim->color.r * step->weight * 255.0f);
		g = (INT32)(prim->color.g * step->weight * 255.0f);
		b = (INT32)(prim->color.b * step->weight * 255.0f);
		a = (INT32)(prim->color.a * 255.0f);
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		if (a > 255) a = 255;
		color = D3DCOLOR_ARGB(a, r, g, b);

		// if we have a texture to use for the vectors, use it here
		if (d3d->vector_texture != NULL)
		{
			vertex[0].u0 = d3d->vector_texture->ustart;
			vertex[0].v0 = d3d->vector_texture->vstart;

			vertex[2].u0 = d3d->vector_texture->ustop;
			vertex[2].v0 = d3d->vector_texture->vstart;

			vertex[1].u0 = d3d->vector_texture->ustart;
			vertex[1].v0 = d3d->vector_texture->vstop;

			vertex[3].u0 = d3d->vector_texture->ustop;
			vertex[3].v0 = d3d->vector_texture->vstop;
		}
		else if(d3d->default_texture != NULL)
		{
			vertex[0].u0 = d3d->default_texture->ustart;
			vertex[0].v0 = d3d->default_texture->vstart;

			vertex[2].u0 = d3d->default_texture->ustart;
			vertex[2].v0 = d3d->default_texture->vstart;

			vertex[1].u0 = d3d->default_texture->ustart;
			vertex[1].v0 = d3d->default_texture->vstart;

			vertex[3].u0 = d3d->default_texture->ustart;
			vertex[3].v0 = d3d->default_texture->vstart;
		}

		// set the color, Z parameters to standard values
		for (i = 0; i < 4; i++)
		{
			vertex[i].z = 0.0f;
			vertex[i].rhw = 1.0f;
			vertex[i].color = color;
		}

		// now add a polygon entry
		poly = &d3d->poly[d3d->numpolys++];
		poly->type = D3DPT_TRIANGLESTRIP;
		poly->count = 2;
		poly->numverts = 4;
		poly->flags = prim->flags;
		poly->modmode = D3DTOP_MODULATE;
		poly->texture = d3d->vector_texture;
	}
}



//============================================================
//  draw_quad
//============================================================

static void draw_quad(d3d_info *d3d, const render_primitive *prim)
{
	d3d_texture_info *texture = texture_find(d3d, prim);
	DWORD color, modmode;
	d3d_vertex *vertex;
	INT32 r, g, b, a;
	d3d_poly_info *poly;
	int i;

	texture = texture != NULL ? texture : d3d->default_texture;

	// get a pointer to the vertex buffer
	vertex = primitive_alloc(d3d, 4);
	if (vertex == NULL)
		return;

	// fill in the vertexes clockwise
	vertex[0].x = prim->bounds.x0 - 0.5f;
	vertex[0].y = prim->bounds.y0 - 0.5f;
	vertex[1].x = prim->bounds.x1 - 0.5f;
	vertex[1].y = prim->bounds.y0 - 0.5f;
	vertex[2].x = prim->bounds.x0 - 0.5f;
	vertex[2].y = prim->bounds.y1 - 0.5f;
	vertex[3].x = prim->bounds.x1 - 0.5f;
	vertex[3].y = prim->bounds.y1 - 0.5f;

	// set the texture coordinates
	if(texture != NULL)
	{
		float du = texture->ustop - texture->ustart;
		float dv = texture->vstop - texture->vstart;
		vertex[0].u0 = texture->ustart + du * prim->texcoords.tl.u;
		vertex[0].v0 = texture->vstart + dv * prim->texcoords.tl.v;
		vertex[1].u0 = texture->ustart + du * prim->texcoords.tr.u;
		vertex[1].v0 = texture->vstart + dv * prim->texcoords.tr.v;
		vertex[2].u0 = texture->ustart + du * prim->texcoords.bl.u;
		vertex[2].v0 = texture->vstart + dv * prim->texcoords.bl.v;
		vertex[3].u0 = texture->ustart + du * prim->texcoords.br.u;
		vertex[3].v0 = texture->vstart + dv * prim->texcoords.br.v;
	}

	// determine the color, allowing for over modulation
	r = (INT32)(prim->color.r * 255.0f);
	g = (INT32)(prim->color.g * 255.0f);
	b = (INT32)(prim->color.b * 255.0f);
	a = (INT32)(prim->color.a * 255.0f);
	modmode = D3DTOP_MODULATE;
	if (texture != NULL)
	{
		if (d3d->mod2x_supported && (r > 255 || g > 255 || b > 255))
		{
			if (d3d->mod4x_supported && (r > 2*255 || g > 2*255 || b > 2*255))
			{
				r >>= 2; g >>= 2; b >>= 2;
				modmode = D3DTOP_MODULATE4X;
			}
			else
			{
				r >>= 1; g >>= 1; b >>= 1;
				modmode = D3DTOP_MODULATE2X;
			}
		}
	}
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (a > 255) a = 255;
	color = D3DCOLOR_ARGB(a, r, g, b);

	// set the color, Z parameters to standard values
	for (i = 0; i < 4; i++)
	{
		vertex[i].z = 0.0f;
		vertex[i].rhw = 1.0f;
		vertex[i].color = color;
	}

	// now add a polygon entry
	poly = &d3d->poly[d3d->numpolys++];
	poly->type = D3DPT_TRIANGLESTRIP;
	poly->count = 2;
	poly->numverts = 4;
	poly->flags = prim->flags;
	poly->modmode = modmode;
	poly->texture = texture;
}


//============================================================
//  primitive_alloc
//============================================================

static d3d_vertex *primitive_alloc(d3d_info *d3d, int numverts)
{
	HRESULT result;

	// if we're going to overflow, flush
	if (d3d->lockedbuf != NULL && d3d->numverts + numverts >= VERTEX_BUFFER_SIZE)
		primitive_flush_pending(d3d);

	// if we don't have a lock, grab it now
	if (d3d->lockedbuf == NULL)
	{
		result = (*d3dintf->vertexbuf.lock)(d3d->vertexbuf, 0, 0, (VOID **)&d3d->lockedbuf, D3DLOCK_DISCARD);
		if (result != D3D_OK)
			return NULL;
	}

	// if we already have the lock and enough room, just return a pointer
	if (d3d->lockedbuf != NULL && d3d->numverts + numverts < VERTEX_BUFFER_SIZE)
	{
		int oldverts = d3d->numverts;
		d3d->numverts += numverts;
		return &d3d->lockedbuf[oldverts];
	}
	return NULL;
}



//============================================================
//  primitive_flush_pending
//============================================================

static void primitive_flush_pending(d3d_info *d3d)
{
	HRESULT result;
	int polynum;
	int vertnum;

	// ignore if we're not locked
	if (d3d->lockedbuf == NULL)
		return;

	// unlock the buffer
	result = (*d3dintf->vertexbuf.unlock)(d3d->vertexbuf);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during vertex buffer unlock call\n", (int)result);
	d3d->lockedbuf = NULL;

	// set the stream
	result = (*d3dintf->device.set_stream_source)(d3d->device, 0, d3d->vertexbuf, sizeof(d3d_vertex));
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_stream_source call\n", (int)result);

	d3d->hlsl->begin();

	// first remember the original render target in case we need to set a new one
	if(d3d->hlsl->enabled() && d3dintf->post_fx_available)
	{
		vertnum = 6;
	}
	else
	{
		vertnum = 0;
	}

	// now do the polys
	for (polynum = 0; polynum < d3d->numpolys; polynum++)
	{
		d3d_poly_info *poly = &d3d->poly[polynum];
		int newfilter;

		// set the texture if different
		set_texture(d3d, poly->texture);

		// set filtering if different
		if (poly->texture != NULL)
		{
			newfilter = FALSE;
			if (PRIMFLAG_GET_SCREENTEX(poly->flags))
				newfilter = video_config.filter;
			set_filter(d3d, newfilter);
			set_wrap(d3d, PRIMFLAG_GET_TEXWRAP(poly->flags));
			set_modmode(d3d, poly->modmode);

			d3d->hlsl->init_effect_info(poly);
		}

		// set the blendmode if different
		set_blendmode(d3d, PRIMFLAG_GET_BLENDMODE(poly->flags));

		if(d3d->hlsl->enabled() && d3dintf->post_fx_available)
		{
			assert(vertnum + poly->numverts <= d3d->numverts);

			d3d->hlsl->render_quad(poly, vertnum);
		}
		else
		{
			assert(vertnum + poly->numverts <= d3d->numverts);

			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		}

		vertnum += poly->numverts;
	}

	d3d->hlsl->end();

	// reset the vertex count
	d3d->numverts = 0;
	d3d->numpolys = 0;
}


void texture_destroy(d3d_info *d3d, d3d_texture_info *info)
{
}

//============================================================
//  texture_create
//============================================================

d3d_texture_info *texture_create(d3d_info *d3d, const render_texinfo *texsource, UINT32 flags)
{
	d3d_texture_info *texture;
	HRESULT result;

	// allocate a new texture
	texture = global_alloc_clear(d3d_texture_info);

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->xprescale = video_config.prescale;
	texture->yprescale = video_config.prescale;

	// compute the size
	texture_compute_size(d3d, texsource->width, texsource->height, texture);

	// non-screen textures are easy
	if (!PRIMFLAG_GET_SCREENTEX(flags))
	{
		assert(PRIMFLAG_TEXFORMAT(flags) != TEXFORMAT_YUY16);
		result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture->d3dtex);
		if (result != D3D_OK)
			goto error;
		texture->d3dfinaltex = texture->d3dtex;
		texture->type = TEXTURE_TYPE_PLAIN;
	}

	// screen textures are allocated differently
	else
	{
		D3DFORMAT format;
		DWORD usage = d3d->dynamic_supported ? D3DUSAGE_DYNAMIC : 0;
		D3DPOOL pool = d3d->dynamic_supported ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
		int maxdim = MAX(d3d->presentation.BackBufferWidth, d3d->presentation.BackBufferHeight);
		int attempt;

		// pick the format
		if (PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_YUY16)
			format = d3d->yuv_format;
		else if (PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_ARGB32 || PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_PALETTEA16)
			format = D3DFMT_A8R8G8B8;
		else
			format = d3d->screen_format;

		// don't prescale above screen size
		while (texture->xprescale > 1 && texture->rawwidth * texture->xprescale >= 2 * maxdim)
			texture->xprescale--;
		while (texture->xprescale > 1 && texture->rawwidth * texture->xprescale > d3d->texture_max_width)
			texture->xprescale--;
		while (texture->yprescale > 1 && texture->rawheight * texture->yprescale >= 2 * maxdim)
			texture->yprescale--;
		while (texture->yprescale > 1 && texture->rawheight * texture->yprescale > d3d->texture_max_height)
			texture->yprescale--;
		if (texture->xprescale != video_config.prescale || texture->yprescale != video_config.prescale)
			mame_printf_verbose("Direct3D: adjusting prescale from %dx%d to %dx%d\n", video_config.prescale, video_config.prescale, texture->xprescale, texture->yprescale);

		// loop until we allocate something or error
		for (attempt = 0; attempt < 2; attempt++)
		{
			// second attempt is always 1:1
			if (attempt == 1)
				texture->xprescale = texture->yprescale = 1;

			// screen textures with no prescaling are pretty easy
			if (texture->xprescale == 1 && texture->yprescale == 1)
			{
				result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, usage, format, pool, &texture->d3dtex);
				if (result == D3D_OK)
				{
					texture->d3dfinaltex = texture->d3dtex;
					texture->type = d3d->dynamic_supported ? TEXTURE_TYPE_DYNAMIC : TEXTURE_TYPE_PLAIN;

					if (d3d->hlsl->enabled() && !d3d->hlsl->register_texture(texture))
					{
						goto error;
					}

					break;
				}
			}

			// screen textures with prescaling require two allocations
			else
			{
				int scwidth, scheight;
				D3DFORMAT finalfmt;

				// use an offscreen plain surface for stretching if supported
				// (won't work for YUY textures)
				if (d3d->stretch_supported && PRIMFLAG_GET_TEXFORMAT(flags) != TEXFORMAT_YUY16)
				{
					result = (*d3dintf->device.create_offscreen_plain_surface)(d3d->device, texture->rawwidth, texture->rawheight, format, D3DPOOL_DEFAULT, &texture->d3dsurface);
					if (result != D3D_OK)
						continue;
					texture->type = TEXTURE_TYPE_SURFACE;
				}

				// otherwise, we allocate a dynamic texture for the source
				else
				{
					result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, usage, format, pool, &texture->d3dtex);
					if (result != D3D_OK)
						continue;
					texture->type = d3d->dynamic_supported ? TEXTURE_TYPE_DYNAMIC : TEXTURE_TYPE_PLAIN;
				}

				// for the target surface, we allocate a render target texture
				scwidth = texture->rawwidth * texture->xprescale;
				scheight = texture->rawheight * texture->yprescale;

				// target surfaces typically cannot be YCbCr, so we always pick RGB in that case
				finalfmt = (format != d3d->yuv_format) ? format : D3DFMT_A8R8G8B8;
				result = (*d3dintf->device.create_texture)(d3d->device, scwidth, scheight, 1, D3DUSAGE_RENDERTARGET, finalfmt, D3DPOOL_DEFAULT, &texture->d3dfinaltex);
				if (result == D3D_OK)
				{
					if (d3d->hlsl->enabled() && !d3d->hlsl->register_prescaled_texture(texture))
					{
						goto error;
					}
					break;
				}
				(*d3dintf->texture.release)(texture->d3dtex);
				texture->d3dtex = NULL;
			}
		}
	}

	// copy the data to the texture
	texture_set_data(d3d, texture, texsource, flags);

	// add us to the texture list
	if(d3d->texlist != NULL)
		d3d->texlist->prev = texture;
	texture->prev = NULL;
	texture->next = d3d->texlist;
	d3d->texlist = texture;
	return texture;

error:
	d3dintf->post_fx_available = false;
	printf("Direct3D: Critical warning: A texture failed to allocate. Expect things to get bad quickly.\n");
	if (texture->d3dsurface != NULL)
		(*d3dintf->surface.release)(texture->d3dsurface);
	if (texture->d3dtex != NULL)
		(*d3dintf->texture.release)(texture->d3dtex);
	global_free(texture);
	return NULL;
}



//============================================================
//  texture_compute_size
//============================================================

static void texture_compute_size(d3d_info *d3d, int texwidth, int texheight, d3d_texture_info *texture)
{
	int finalheight = texheight;
	int finalwidth = texwidth;

	// if we're not wrapping, add a 1-2 pixel border on all sides
	texture->xborderpix = 0;
	texture->yborderpix = 0;
	if (ENABLE_BORDER_PIX && !(texture->flags & PRIMFLAG_TEXWRAP_MASK))
	{
		// note we need 2 pixels in X for YUY textures
		texture->xborderpix = (PRIMFLAG_GET_TEXFORMAT(texture->flags) == TEXFORMAT_YUY16) ? 2 : 1;
		texture->yborderpix = 1;
	}

	// compute final texture size
	finalwidth += 2 * texture->xborderpix;
	finalheight += 2 * texture->yborderpix;

	// round width/height up to nearest power of 2 if we need to
	if (!(d3d->texture_caps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
	{
		// first the width
		if (finalwidth & (finalwidth - 1))
		{
			finalwidth |= finalwidth >> 1;
			finalwidth |= finalwidth >> 2;
			finalwidth |= finalwidth >> 4;
			finalwidth |= finalwidth >> 8;
			finalwidth++;
		}

		// then the height
		if (finalheight & (finalheight - 1))
		{
			finalheight |= finalheight >> 1;
			finalheight |= finalheight >> 2;
			finalheight |= finalheight >> 4;
			finalheight |= finalheight >> 8;
			finalheight++;
		}
	}

	// round up to square if we need to
	if (d3d->texture_caps & D3DPTEXTURECAPS_SQUAREONLY)
	{
		if (finalwidth < finalheight)
			finalwidth = finalheight;
		else
			finalheight = finalwidth;
	}

	// adjust the aspect ratio if we need to
	while (finalwidth < finalheight && finalheight / finalwidth > d3d->texture_max_aspect)
		finalwidth *= 2;
	while (finalheight < finalwidth && finalwidth / finalheight > d3d->texture_max_aspect)
		finalheight *= 2;

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if ((finalwidth > d3d->texture_max_width && finalwidth - 2 * texture->xborderpix <= d3d->texture_max_width) ||
		(finalheight > d3d->texture_max_height && finalheight - 2 * texture->yborderpix <= d3d->texture_max_height))
	{
		finalwidth -= 2 * texture->xborderpix;
		finalheight -= 2 * texture->yborderpix;
		texture->xborderpix = 0;
		texture->yborderpix = 0;
	}

	// if we're above the max width/height, do what?
	if (finalwidth > d3d->texture_max_width || finalheight > d3d->texture_max_height)
	{
		static int printed = FALSE;
		if (!printed) mame_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth, finalheight, (int)d3d->texture_max_width, (int)d3d->texture_max_height);
		printed = TRUE;
	}

	// compute the U/V scale factors
	texture->ustart = (float)texture->xborderpix / (float)finalwidth;
	texture->ustop = (float)(texwidth + texture->xborderpix) / (float)finalwidth;
	texture->vstart = (float)texture->yborderpix / (float)finalheight;
	texture->vstop = (float)(texheight + texture->yborderpix) / (float)finalheight;

	// set the final values
	texture->rawwidth = finalwidth;
	texture->rawheight = finalheight;
}



//============================================================
//  copyline_palette16
//============================================================

INLINE void copyline_palette16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*src];
	for (x = 0; x < width; x++)
		*dst++ = 0xff000000 | palette[*src++];
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*--src];
}



//============================================================
//  copyline_palettea16
//============================================================

INLINE void copyline_palettea16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = palette[*src];
	for (x = 0; x < width; x++)
		*dst++ = palette[*src++];
	if (xborderpix)
		*dst++ = palette[*--src];
}



//============================================================
//  copyline_rgb32
//============================================================

INLINE void copyline_rgb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT32 srcpix = *src;
			*dst++ = 0xff000000 | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
		for (x = 0; x < width; x++)
		{
			UINT32 srcpix = *src++;
			*dst++ = 0xff000000 | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
		if (xborderpix)
		{
			UINT32 srcpix = *--src;
			*dst++ = 0xff000000 | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = 0xff000000 | *src;
		for (x = 0; x < width; x++)
			*dst++ = 0xff000000 | *src++;
		if (xborderpix)
			*dst++ = 0xff000000 | *--src;
	}
}



//============================================================
//  copyline_argb32
//============================================================

INLINE void copyline_argb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT32 srcpix = *src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
		for (x = 0; x < width; x++)
		{
			UINT32 srcpix = *src++;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
		if (xborderpix)
		{
			UINT32 srcpix = *--src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + RGB_RED(srcpix)] | palette[0x100 + RGB_GREEN(srcpix)] | palette[RGB_BLUE(srcpix)];
		}
	}

	// direct case
	else
	{
		if (xborderpix)
			*dst++ = *src;
		for (x = 0; x < width; x++)
			*dst++ = *src++;
		if (xborderpix)
			*dst++ = *--src;
	}
}



//============================================================
//  copyline_yuy16_to_yuy2
//============================================================

INLINE void copyline_yuy16_to_yuy2(UINT16 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 2);
	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src--;
			*dst++ = palette[0x000 + (srcpix0 >> 8)] | (srcpix0 << 8);
			*dst++ = palette[0x000 + (srcpix0 >> 8)] | (srcpix1 << 8);
		}
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			*dst++ = palette[0x000 + (srcpix0 >> 8)] | (srcpix0 << 8);
			*dst++ = palette[0x000 + (srcpix1 >> 8)] | (srcpix1 << 8);
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			*dst++ = palette[0x000 + (srcpix1 >> 8)] | (srcpix0 << 8);
			*dst++ = palette[0x000 + (srcpix1 >> 8)] | (srcpix1 << 8);
		}
	}

	// direct case
	else
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src--;
			*dst++ = (srcpix0 >> 8) | (srcpix0 << 8);
			*dst++ = (srcpix0 >> 8) | (srcpix1 << 8);
		}
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			*dst++ = (srcpix0 >> 8) | (srcpix0 << 8);
			*dst++ = (srcpix1 >> 8) | (srcpix1 << 8);
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			*dst++ = (srcpix1 >> 8) | (srcpix0 << 8);
			*dst++ = (srcpix1 >> 8) | (srcpix1 << 8);
		}
	}
}



//============================================================
//  copyline_yuy16_to_uyvy
//============================================================

INLINE void copyline_yuy16_to_uyvy(UINT16 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 2);
	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src--;
			*dst++ = palette[0x100 + (srcpix0 >> 8)] | (srcpix0 & 0xff);
			*dst++ = palette[0x100 + (srcpix0 >> 8)] | (srcpix1 & 0xff);
		}
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			*dst++ = palette[0x100 + (srcpix0 >> 8)] | (srcpix0 & 0xff);
			*dst++ = palette[0x100 + (srcpix1 >> 8)] | (srcpix1 & 0xff);
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			*dst++ = palette[0x100 + (srcpix1 >> 8)] | (srcpix0 & 0xff);
			*dst++ = palette[0x100 + (srcpix1 >> 8)] | (srcpix1 & 0xff);
		}
	}

	// direct case
	else
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = src[0];
			UINT16 srcpix1 = src[1];
			*dst++ = srcpix0;
			*dst++ = (srcpix0 & 0xff00) | (srcpix1 & 0x00ff);
		}
		for (x = 0; x < width; x += 2)
		{
			*dst++ = *src++;
			*dst++ = *src++;
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			*dst++ = (srcpix1 & 0xff00) | (srcpix0 & 0x00ff);
			*dst++ = srcpix1;
		}
	}
}



//============================================================
//  copyline_yuy16_to_argb
//============================================================

INLINE void copyline_yuy16_to_argb(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 2);
	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != NULL)
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = src[0];
			UINT16 srcpix1 = src[1];
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
		}
		for (x = 0; x < width / 2; x++)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
			*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
		}
	}

	// direct case
	else
	{
		if (xborderpix)
		{
			UINT16 srcpix0 = src[0];
			UINT16 srcpix1 = src[1];
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
		}
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
		}
		if (xborderpix)
		{
			UINT16 srcpix1 = *--src;
			UINT16 srcpix0 = *--src;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
			*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
		}
	}
}



//============================================================
//  texture_set_data
//============================================================

static void texture_set_data(d3d_info *d3d, d3d_texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
	D3DLOCKED_RECT rect;
	HRESULT result;
	int miny, maxy;
	int dsty;

	// lock the texture
	switch (texture->type)
	{
		default:
		case TEXTURE_TYPE_PLAIN:	result = (*d3dintf->texture.lock_rect)(texture->d3dtex, 0, &rect, NULL, 0);					break;
		case TEXTURE_TYPE_DYNAMIC:	result = (*d3dintf->texture.lock_rect)(texture->d3dtex, 0, &rect, NULL, D3DLOCK_DISCARD);	break;
		case TEXTURE_TYPE_SURFACE:	result = (*d3dintf->surface.lock_rect)(texture->d3dsurface, &rect, NULL, D3DLOCK_DISCARD);	break;
	}
	if (result != D3D_OK)
		return;

	// loop over Y
	miny = 0 - texture->yborderpix;
	maxy = texsource->height + texture->yborderpix;
	for (dsty = miny; dsty < maxy; dsty++)
	{
		int srcy = (dsty < 0) ? 0 : (dsty >= texsource->height) ? texsource->height - 1 : dsty;
		void *dst = (BYTE *)rect.pBits + (dsty + texture->yborderpix) * rect.Pitch;

		// switch off of the format and
		switch (PRIMFLAG_GET_TEXFORMAT(flags))
		{
			case TEXFORMAT_PALETTE16:
				copyline_palette16((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				break;

			case TEXFORMAT_PALETTEA16:
				copyline_palettea16((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				break;

			case TEXFORMAT_RGB32:
				copyline_rgb32((UINT32 *)dst, (UINT32 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				break;

			case TEXFORMAT_ARGB32:
				copyline_argb32((UINT32 *)dst, (UINT32 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				break;

			case TEXFORMAT_YUY16:
				if (d3d->yuv_format == D3DFMT_YUY2)
					copyline_yuy16_to_yuy2((UINT16 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				else if (d3d->yuv_format == D3DFMT_UYVY)
					copyline_yuy16_to_uyvy((UINT16 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				else
					copyline_yuy16_to_argb((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, texture->xborderpix);
				break;

			default:
				mame_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
				break;
		}
	}

	// unlock
	switch (texture->type)
	{
		default:
		case TEXTURE_TYPE_PLAIN:	result = (*d3dintf->texture.unlock_rect)(texture->d3dtex, 0);	break;
		case TEXTURE_TYPE_DYNAMIC:	result = (*d3dintf->texture.unlock_rect)(texture->d3dtex, 0);	break;
		case TEXTURE_TYPE_SURFACE:	result = (*d3dintf->surface.unlock_rect)(texture->d3dsurface);	break;
	}
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);

	// prescale
	texture_prescale(d3d, texture);
}



//============================================================
//  texture_prescale
//============================================================

static void texture_prescale(d3d_info *d3d, d3d_texture_info *texture)
{
	d3d_surface *surface;
	HRESULT result;
	int i;

	// if we don't need to, just skip it
	if (texture->d3dtex == texture->d3dfinaltex)
		return;

	// for all cases, we need to get the surface of the render target
	result = (*d3dintf->texture.get_surface_level)(texture->d3dfinaltex, 0, &surface);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture get_surface_level call\n", (int)result);

	// if we have an offscreen plain surface, we can just StretchRect to it
	if (texture->type == TEXTURE_TYPE_SURFACE)
	{
		RECT source, dest;

		assert(texture->d3dsurface != NULL);

		// set the source bounds
		source.left = source.top = 0;
		source.right = texture->texinfo.width + 2 * texture->xborderpix;
		source.bottom = texture->texinfo.height + 2 * texture->yborderpix;

		// set the target bounds
		dest = source;
		dest.right *= texture->xprescale;
		dest.bottom *= texture->yprescale;

		// do the stretchrect
		result = (*d3dintf->device.stretch_rect)(d3d->device, texture->d3dsurface, &source, surface, &dest, D3DTEXF_POINT);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device stretct_rect call\n", (int)result);
	}

	// if we are using a texture render target, we need to do more preparations
	else
	{
		d3d_surface *backbuffer;

		assert(texture->d3dtex != NULL);

		// first remember the original render target and set the new one
		result = (*d3dintf->device.get_render_target)(d3d->device, 0, &backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device get_render_target call\n", (int)result);
		result = (*d3dintf->device.set_render_target)(d3d->device, 0, surface);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 1\n", (int)result);
		reset_render_states(d3d);

		// start the scene
		result = (*d3dintf->device.begin_scene)(d3d->device);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device begin_scene call\n", (int)result);

		// configure the rendering pipeline
		set_filter(d3d, FALSE);
		set_blendmode(d3d, BLENDMODE_NONE);
		result = (*d3dintf->device.set_texture)(d3d->device, 0, texture->d3dtex);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_texture call\n", (int)result);

		// lock the vertex buffer
		result = (*d3dintf->vertexbuf.lock)(d3d->vertexbuf, 0, 0, (VOID **)&d3d->lockedbuf, D3DLOCK_DISCARD);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during vertex buffer lock call\n", (int)result);

		// configure the X/Y coordinates on the target surface
		d3d->lockedbuf[0].x = -0.5f;
		d3d->lockedbuf[0].y = -0.5f;
		d3d->lockedbuf[1].x = (float)((texture->texinfo.width + 2 * texture->xborderpix) * texture->xprescale) - 0.5f;
		d3d->lockedbuf[1].y = -0.5f;
		d3d->lockedbuf[2].x = -0.5f;
		d3d->lockedbuf[2].y = (float)((texture->texinfo.height + 2 * texture->yborderpix) * texture->yprescale) - 0.5f;
		d3d->lockedbuf[3].x = (float)((texture->texinfo.width + 2 * texture->xborderpix) * texture->xprescale) - 0.5f;
		d3d->lockedbuf[3].y = (float)((texture->texinfo.height + 2 * texture->yborderpix) * texture->yprescale) - 0.5f;

		// configure the U/V coordintes on the source texture
		d3d->lockedbuf[0].u0 = 0.0f;
		d3d->lockedbuf[0].v0 = 0.0f;
		d3d->lockedbuf[1].u0 = (float)(texture->texinfo.width + 2 * texture->xborderpix) / (float)texture->rawwidth;
		d3d->lockedbuf[1].v0 = 0.0f;
		d3d->lockedbuf[2].u0 = 0.0f;
		d3d->lockedbuf[2].v0 = (float)(texture->texinfo.height + 2 * texture->yborderpix) / (float)texture->rawheight;
		d3d->lockedbuf[3].u0 = (float)(texture->texinfo.width + 2 * texture->xborderpix) / (float)texture->rawwidth;
		d3d->lockedbuf[3].v0 = (float)(texture->texinfo.height + 2 * texture->yborderpix) / (float)texture->rawheight;

		// reset the remaining vertex parameters
		for (i = 0; i < 4; i++)
		{
			d3d->lockedbuf[i].z = 0.0f;
			d3d->lockedbuf[i].rhw = 1.0f;
			d3d->lockedbuf[i].color = D3DCOLOR_ARGB(0xff,0xff,0xff,0xff);
		}

		// unlock the vertex buffer
		result = (*d3dintf->vertexbuf.unlock)(d3d->vertexbuf);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during vertex buffer unlock call\n", (int)result);
		d3d->lockedbuf = NULL;

		// set the stream and draw the triangle strip
		result = (*d3dintf->device.set_stream_source)(d3d->device, 0, d3d->vertexbuf, sizeof(d3d_vertex));
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_stream_source call\n", (int)result);
		result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLESTRIP, 0, 2);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);

		// end the scene
		result = (*d3dintf->device.end_scene)(d3d->device);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device end_scene call\n", (int)result);

		// reset the render target and release our reference to the backbuffer
		result = (*d3dintf->device.set_render_target)(d3d->device, 0, backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 2\n", (int)result);
		(*d3dintf->surface.release)(backbuffer);
		reset_render_states(d3d);
	}

	// release our reference to the target surface
	(*d3dintf->surface.release)(surface);
}



//============================================================
//  texture_find
//============================================================

static d3d_texture_info *texture_find(d3d_info *d3d, const render_primitive *prim)
{
	UINT32 texhash = texture_compute_hash(&prim->texture, prim->flags);
	d3d_texture_info *texture;

	// find a match
	for (texture = d3d->texlist; texture != NULL; texture = texture->next)
	{
		UINT32 test_screen = (UINT32)texture->texinfo.osddata >> 1;
		UINT32 test_page = (UINT32)texture->texinfo.osddata & 1;
		UINT32 prim_screen = (UINT32)prim->texture.osddata >> 1;
		UINT32 prim_page = (UINT32)prim->texture.osddata & 1;
		if (test_screen != prim_screen || test_page != prim_page)
		{
			continue;
		}

		if (texture->hash == texhash &&
			texture->texinfo.base == prim->texture.base &&
			texture->texinfo.width == prim->texture.width &&
			texture->texinfo.height == prim->texture.height &&
			((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		{
			// Reject a texture if it belongs to an out-of-date render target, so as to cause the HLSL system to re-cache
			if (d3d->hlsl->enabled() && prim->texture.width != 0 && prim->texture.height != 0 && (prim->flags & PRIMFLAG_SCREENTEX_MASK) != 0)
			{
				if (d3d->hlsl->find_render_target(texture) != NULL)
				{
					return texture;
				}
			}
			else
			{
				return texture;
			}
		}
	}

	// nothing found, check if we need to unregister something with hlsl
	if (d3d->hlsl->enabled())
	{
		if (prim->texture.width == 0 || prim->texture.height == 0)
		{
			return NULL;
		}

		UINT32 prim_screen = (UINT32)prim->texture.osddata >> 1;
		UINT32 prim_page = (UINT32)prim->texture.osddata & 1;

		for (texture = d3d->texlist; texture != NULL; texture = texture->next)
		{
			UINT32 test_screen = (UINT32)texture->texinfo.osddata >> 1;
			UINT32 test_page = (UINT32)texture->texinfo.osddata & 1;
			if (test_screen != prim_screen || test_page != prim_page)
			{
				continue;
			}

			// Clear our old texture reference
			if (texture->hash == texhash &&
			    texture->texinfo.base == prim->texture.base &&
			    ((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0 &&
			    (texture->texinfo.width != prim->texture.width ||
			     texture->texinfo.height != prim->texture.height))
			{
				d3d->hlsl->remove_render_target(texture);
				break;
			}
		}
	}
	return NULL;
}


//============================================================
//  texture_update
//============================================================

static void texture_update(d3d_info *d3d, const render_primitive *prim)
{
	d3d_texture_info *texture = texture_find(d3d, prim);

	// if we didn't find one, create a new texture
	if (texture == NULL)
	{
		texture = texture_create(d3d, &prim->texture, prim->flags);
	}

	// if we found it, but with a different seqid, copy the data
	if (texture->texinfo.seqid != prim->texture.seqid)
	{
		texture_set_data(d3d, texture, &prim->texture, prim->flags);
		texture->texinfo.seqid = prim->texture.seqid;
	}
}


//============================================================
//  d3d_cache_target::~d3d_cache_target
//============================================================

d3d_cache_target::~d3d_cache_target()
{
	if (last_texture != NULL)
	{
		(*d3dintf->texture.release)(last_texture);
		last_texture = NULL;
	}
	if (last_target != NULL)
	{
		(*d3dintf->surface.release)(last_target);
		last_target = NULL;
	}
}


//============================================================
//  d3d_cache_target::init - initializes a target cache
//============================================================

bool d3d_cache_target::init(d3d_info *d3d, d3d_base *d3dintf, int width, int height, int prescale_x, int prescale_y)
{
	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &last_texture);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(last_texture, 0, &last_target);

	return true;
}

//============================================================
//  d3d_render_target::~d3d_render_target
//============================================================

d3d_render_target::~d3d_render_target()
{
	for (int index = 0; index < 5; index++)
	{
		if (texture[index] != NULL)
		{
			(*d3dintf->texture.release)(texture[index]);
			texture[index] = NULL;
		}
		if (target[index] != NULL)
		{
			(*d3dintf->surface.release)(target[index]);
			target[index] = NULL;
		}
	}

	if (prescaletexture != NULL)
	{
		(*d3dintf->texture.release)(prescaletexture);
		prescaletexture = NULL;
	}
	if (prescaletarget != NULL)
 	{
		(*d3dintf->surface.release)(prescaletarget);
		prescaletarget = NULL;
	}

	if (smalltexture != NULL)
	{
		(*d3dintf->texture.release)(smalltexture);
		smalltexture = NULL;
	}
	if (smalltarget != NULL)
	{
		(*d3dintf->surface.release)(smalltarget);
		smalltarget = NULL;
	}
}


//============================================================
//  d3d_render_target::init - initializes a render target
//============================================================

bool d3d_render_target::init(d3d_info *d3d, d3d_base *d3dintf, int width, int height, int prescale_x, int prescale_y)
{
	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture[0]);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(texture[0], 0, &target[0]);

	result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture[1]);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(texture[1], 0, &target[1]);

	result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture[2]);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(texture[2], 0, &target[2]);

	result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture[3]);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(texture[3], 0, &target[3]);

	result = (*d3dintf->device.create_texture)(d3d->device, width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture[4]);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(texture[4], 0, &target[4]);

	result = (*d3dintf->device.create_texture)(d3d->device, width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &smalltexture);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(smalltexture, 0, &smalltarget);

	result = (*d3dintf->device.create_texture)(d3d->device, width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &prescaletexture);
	if (result != D3D_OK)
		return false;
	(*d3dintf->texture.get_surface_level)(prescaletexture, 0, &prescaletarget);

	target_width = width * prescale_x;
	target_height = height * prescale_y;

	return true;
}
