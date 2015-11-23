// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawd3d.c - Win32 Direct3D implementation
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

#include "rendutil.h"
#include "emuopts.h"
#include "aviio.h"

// MAMEOS headers
#include "modules/render/d3d/d3dintf.h"
#include "winmain.h"
#include "window.h"
#include "modules/render/d3d/d3dcomm.h"
#include "drawd3d.h"


//============================================================
//  DEBUGGING
//============================================================

extern void mtlog_add(const char *event);


//============================================================
//  CONSTANTS
//============================================================

#define ENABLE_BORDER_PIX   (1)

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

	return rgb_t(0xff, r, g, b);
}


//============================================================
//  drawd3d_init
//============================================================

static d3d::base *               d3dintf; // FIX ME


//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawd3d_exit(void);


//============================================================
//  drawd3d_window_init
//============================================================

int d3d::renderer::create()
{
	if (!initialize())
	{
		destroy();
		osd_printf_error("Unable to initialize Direct3D.\n");
		return 1;
	}

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

void d3d::renderer::toggle_fsfx()
{
	set_restarting(true);
}

void d3d::renderer::record()
{
	get_shaders()->window_record();
}

void d3d::renderer::save()
{
	get_shaders()->window_save();
}


//============================================================
//  drawd3d_window_destroy
//============================================================

void d3d::renderer::destroy()
{
	if (get_shaders() != NULL && get_shaders()->recording())
		get_shaders()->window_record();

}


//============================================================
//  drawd3d_window_get_primitives
//============================================================

render_primitive_list *d3d::renderer::get_primitives()
{
	RECT client;

	GetClientRectExceptMenu(window().m_hwnd, &client, window().fullscreen());
	if (rect_width(&client) > 0 && rect_height(&client) > 0)
	{
		window().target()->set_bounds(rect_width(&client), rect_height(&client), window().aspect());
		window().target()->set_max_update_rate((get_refresh() == 0) ? get_origmode().RefreshRate : get_refresh());
	}
	if (m_shaders != NULL)
	{
		window().target()->set_transform_primitives(!m_shaders->enabled());
	}
	return &window().target()->get_primitives();
}


//============================================================
//  drawnone_create
//============================================================

static osd_renderer *drawd3d_create(osd_window *window)
{
	return global_alloc(d3d::renderer(window));
}

int drawd3d_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	d3dintf = NULL;

	// Use Direct3D9
	d3dintf = d3d::drawd3d9_init();

	// if we failed, note the error
	if (d3dintf == NULL)
	{
		osd_printf_error("Unable to initialize Direct3D.\n");
		return 1;
	}

	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawd3d_exit;
	callbacks->create = drawd3d_create;
	return 0;
}


//============================================================
//  drawd3d_window_draw
//============================================================

int d3d::renderer::draw(const int update)
{
	int check = pre_window_draw_check();
	if (check >= 0)
		return check;

	begin_frame();
	process_primitives();
	end_frame();

	return 0;
}

namespace d3d
{
void renderer::set_texture(texture_info *texture)
{
	if (texture != m_last_texture)
	{
		m_last_texture = texture;
		m_last_texture_flags = (texture == NULL ? 0 : texture->get_flags());
		HRESULT result = (*d3dintf->device.set_texture)(m_device, 0, (texture == NULL) ? get_default_texture()->get_finaltex() : texture->get_finaltex());
		m_shaders->set_texture(texture);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture call\n", (int)result);
	}
}

void renderer::set_filter(int filter)
{
	if (filter != m_last_filter)
	{
		m_last_filter = filter;
		HRESULT result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MINFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MAGFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MINFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_MAGFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}

void renderer::set_wrap(D3DTEXTUREADDRESS wrap)
{
	if (wrap != m_last_wrap)
	{
		m_last_wrap = wrap;
		HRESULT result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSU, wrap);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSV, wrap);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSU, wrap);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, (D3DTEXTURESTAGESTATETYPE)D3DTSS_ADDRESSV, wrap);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}

void renderer::set_modmode(DWORD modmode)
{
	if (modmode != m_last_modmode)
	{
		m_last_modmode = modmode;
		HRESULT result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, D3DTSS_COLOROP, modmode);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
		result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, D3DTSS_COLOROP, modmode);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture_stage_state call\n", (int)result);
	}
}

void renderer::set_blendmode(int blendmode)
{
	int blendenable;
	int blendop;
	int blendsrc;
	int blenddst;

	// choose the parameters
	switch (blendmode)
	{
		default:
		case BLENDMODE_NONE:            blendenable = FALSE;    blendop = D3DBLENDOP_ADD;   blendsrc = D3DBLEND_SRCALPHA;   blenddst = D3DBLEND_INVSRCALPHA;    break;
		case BLENDMODE_ALPHA:           blendenable = TRUE;     blendop = D3DBLENDOP_ADD;   blendsrc = D3DBLEND_SRCALPHA;   blenddst = D3DBLEND_INVSRCALPHA;    break;
		case BLENDMODE_RGB_MULTIPLY:    blendenable = TRUE;     blendop = D3DBLENDOP_ADD;   blendsrc = D3DBLEND_DESTCOLOR;  blenddst = D3DBLEND_ZERO;           break;
		case BLENDMODE_ADD:             blendenable = TRUE;     blendop = D3DBLENDOP_ADD;   blendsrc = D3DBLEND_SRCALPHA;   blenddst = D3DBLEND_ONE;            break;
	}

	// adjust the bits that changed
	if (blendenable != m_last_blendenable)
	{
		m_last_blendenable = blendenable;
		HRESULT result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ALPHABLENDENABLE, blendenable);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blendop != m_last_blendop)
	{
		m_last_blendop = blendop;
		HRESULT result = (*d3dintf->device.set_render_state)(m_device, D3DRS_BLENDOP, blendop);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blendsrc != m_last_blendsrc)
	{
		m_last_blendsrc = blendsrc;
		HRESULT result = (*d3dintf->device.set_render_state)(m_device, D3DRS_SRCBLEND, blendsrc);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}

	if (blenddst != m_last_blenddst)
	{
		m_last_blenddst = blenddst;
		HRESULT result = (*d3dintf->device.set_render_state)(m_device, D3DRS_DESTBLEND, blenddst);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_state call\n", (int)result);
	}
}

void renderer::reset_render_states()
{
	// this ensures subsequent calls to the above setters will force-update the data
	m_last_texture = (texture_info *)~0;
	m_last_filter = -1;
	m_last_blendenable = -1;
	m_last_blendop = -1;
	m_last_blendsrc = -1;
	m_last_blenddst = -1;
	m_last_wrap = (D3DTEXTUREADDRESS)-1;
}

texture_manager::texture_manager(renderer *d3d)
{
	m_renderer = d3d;

	m_texlist = NULL;
	m_vector_texture = NULL;
	m_default_texture = NULL;

	// check for dynamic texture support
	DWORD tempcaps;
	HRESULT result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_CAPS2, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	m_dynamic_supported = ((tempcaps & D3DCAPS2_DYNAMICTEXTURES) != 0);
	if (m_dynamic_supported) osd_printf_verbose("Direct3D: Using dynamic textures\n");

	// check for stretchrect support
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_STRETCH_RECT_FILTER, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	m_stretch_supported = ((tempcaps & D3DPTFILTERCAPS_MAGFPOINT) != 0);
	if (m_stretch_supported && video_config.prescale > 1) osd_printf_verbose("Direct3D: Using StretchRect for prescaling\n");

	// get texture caps
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_TEXTURE_CAPS, &m_texture_caps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_ASPECT, &m_texture_max_aspect);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_WIDTH, &m_texture_max_width);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, CAPS_MAX_TEXTURE_HEIGHT, &m_texture_max_height);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);

	// pick a YUV texture format
	m_yuv_format = D3DFMT_UYVY;
	result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, d3d->get_pixel_format(), 0, D3DRTYPE_TEXTURE, D3DFMT_UYVY);
	if (result != D3D_OK)
	{
		m_yuv_format = D3DFMT_YUY2;
		result = (*d3dintf->d3d.check_device_format)(d3dintf, d3d->get_adapter(), D3DDEVTYPE_HAL, d3d->get_pixel_format(), 0, D3DRTYPE_TEXTURE, D3DFMT_YUY2);
		if (result != D3D_OK)
			m_yuv_format = D3DFMT_A8R8G8B8;
	}
	osd_printf_verbose("Direct3D: YUV format = %s\n", (m_yuv_format == D3DFMT_YUY2) ? "YUY2" : (m_yuv_format == D3DFMT_UYVY) ? "UYVY" : "RGB");

	// set the max texture size
	d3d->window().target()->set_max_texture_size(m_texture_max_width, m_texture_max_height);
	osd_printf_verbose("Direct3D: Max texture size = %dx%d\n", (int)m_texture_max_width, (int)m_texture_max_height);
}

texture_manager::~texture_manager()
{
}

void texture_manager::create_resources()
{
	// experimental: load a PNG to use for vector rendering; it is treated
	// as a brightness map
	emu_file file(m_renderer->window().machine().options().art_path(), OPEN_FLAG_READ);
	render_load_png(m_vector_bitmap, file, NULL, "vector.png");
	if (m_vector_bitmap.valid())
	{
		m_vector_bitmap.fill(rgb_t(0xff,0xff,0xff,0xff));
		render_load_png(m_vector_bitmap, file, NULL, "vector.png", true);
	}

	m_default_bitmap.allocate(8, 8);
	m_default_bitmap.fill(rgb_t(0xff,0xff,0xff,0xff));

	if (m_default_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = m_default_bitmap.raw_pixptr(0);
		texture.rowpixels = m_default_bitmap.rowpixels();
		texture.width = m_default_bitmap.width();
		texture.height = m_default_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		m_default_texture = global_alloc(texture_info(this, &texture, m_renderer->window().prescale(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32)));
	}

	// experimental: if we have a vector bitmap, create a texture for it
	if (m_vector_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = &m_vector_bitmap.pix32(0);
		texture.rowpixels = m_vector_bitmap.rowpixels();
		texture.width = m_vector_bitmap.width();
		texture.height = m_vector_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		m_vector_texture = global_alloc(texture_info(this, &texture, m_renderer->window().prescale(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32)));
	}
}

void texture_manager::delete_resources()
{
	// is part of m_texlist and will be free'd there
	//global_free(m_default_texture);
	m_default_texture = NULL;

	//global_free(m_vector_texture);
	m_vector_texture = NULL;

	// free all textures
	while (m_texlist != NULL)
	{
		texture_info *tex = m_texlist;
		m_texlist = tex->get_next();
		global_free(tex);
	}
}

UINT32 texture_manager::texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	return (FPTR)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

texture_info *texture_manager::find_texinfo(const render_texinfo *texinfo, UINT32 flags)
{
	UINT32 hash = texture_compute_hash(texinfo, flags);
	texture_info *texture;

	// find a match
	for (texture = m_renderer->get_texture_manager()->get_texlist(); texture != NULL; texture = texture->get_next())
	{
		UINT32 test_screen = (UINT32)texture->get_texinfo().osddata >> 1;
		UINT32 test_page = (UINT32)texture->get_texinfo().osddata & 1;
		UINT32 prim_screen = (UINT32)texinfo->osddata >> 1;
		UINT32 prim_page = (UINT32)texinfo->osddata & 1;
		if (test_screen != prim_screen || test_page != prim_page)
		{
			continue;
		}

		if (texture->get_hash() == hash &&
			texture->get_texinfo().base == texinfo->base &&
			texture->get_texinfo().width == texinfo->width &&
			texture->get_texinfo().height == texinfo->height &&
			((texture->get_flags() ^ flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		{
			// Reject a texture if it belongs to an out-of-date render target, so as to cause the HLSL system to re-cache
			if (m_renderer->get_shaders()->enabled() && texinfo->width != 0 && texinfo->height != 0 && (flags & PRIMFLAG_SCREENTEX_MASK) != 0)
			{
				if (m_renderer->get_shaders()->find_render_target(texture) != NULL)
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

	// Nothing found, check if we need to unregister something with HLSL
	if (m_renderer->get_shaders()->enabled())
	{
		if (texinfo->width == 0 || texinfo->height == 0)
		{
			return NULL;
		}

		UINT32 prim_screen = texinfo->osddata >> 1;
		UINT32 prim_page = texinfo->osddata & 1;

		for (texture = m_renderer->get_texture_manager()->get_texlist(); texture != NULL; texture = texture->get_next())
		{
			UINT32 test_screen = texture->get_texinfo().osddata >> 1;
			UINT32 test_page = texture->get_texinfo().osddata & 1;
			if (test_screen != prim_screen || test_page != prim_page)
			{
				continue;
			}

			// Clear out our old texture reference
			if (texture->get_hash() == hash &&
				texture->get_texinfo().base == texinfo->base &&
				((texture->get_flags() ^ flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0 &&
				(texture->get_texinfo().width != texinfo->width ||
					texture->get_texinfo().height != texinfo->height))
			{
				m_renderer->get_shaders()->remove_render_target(texture);
			}
		}
	}

	return NULL;
}

renderer::renderer(osd_window *window)
	: osd_renderer(window, FLAG_NONE), m_adapter(0), m_width(0), m_height(0), m_refresh(0), m_create_error_count(0), m_device(NULL), m_gamma_supported(0), m_pixformat(), 
	m_vertexbuf(NULL), m_lockedbuf(NULL), m_numverts(0), m_vectorbatch(NULL), m_batchindex(0), m_numpolys(0), m_restarting(false), m_mod2x_supported(0), m_mod4x_supported(0), 
	m_screen_format(), m_last_texture(NULL), m_last_texture_flags(0), m_last_blendenable(0), m_last_blendop(0), m_last_blendsrc(0), m_last_blenddst(0), m_last_filter(0),
	m_last_wrap(), m_last_modmode(0), m_hlsl_buf(NULL), m_shaders(NULL), m_shaders_options(NULL), m_texture_manager(NULL), m_line_count(0)
{
}

int renderer::initialize()
{
	// configure the adapter for the mode we want
	if (config_adapter_mode())
		return false;

	// create the device immediately for the full screen case (defer for window mode)
	if (window().fullscreen() && device_create(window().m_focus_hwnd))
		return false;

	return true;
}

int renderer::pre_window_draw_check()
{
	// if we're in the middle of resizing, leave things alone
	if (window().m_resize_state == RESIZE_STATE_RESIZING)
		return 0;

	// if we're restarting the renderer, leave things alone
	if (m_restarting)
	{
		m_shaders->toggle();

		m_restarting = false;
	}

	// if we have a device, check the cooperative level
	if (m_device != NULL)
	{
		if (device_test_cooperative())
		{
			return 1;
		}
	}

	// in window mode, we need to track the window size
	if (!window().fullscreen() || m_device == NULL)
	{
		// if the size changes, skip this update since the render target will be out of date
		if (update_window_size())
			return 0;

		// if we have no device, after updating the size, return an error so GDI can try
		if (m_device == NULL)
			return 1;
	}

	return -1;
}

void texture_manager::update_textures()
{
	for (render_primitive *prim = m_renderer->window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		if (prim->texture.base != NULL)
		{
			texture_info *texture = find_texinfo(&prim->texture, prim->flags);
			if (texture == NULL)
			{
				// if there isn't one, create a new texture
				global_alloc(texture_info(this, &prim->texture, m_renderer->window().prescale(), prim->flags));
			}
			else
			{
				// if there is one, but with a different seqid, copy the data
				if (texture->get_texinfo().seqid != prim->texture.seqid)
				{
					texture->set_data(&prim->texture, prim->flags);
					texture->get_texinfo().seqid = prim->texture.seqid;
				}
			}
		}
		else if(m_renderer->get_shaders()->vector_enabled() && PRIMFLAG_GET_VECTORBUF(prim->flags))
		{
			if (!m_renderer->get_shaders()->get_vector_target())
			{
				m_renderer->get_shaders()->create_vector_target(prim);
			}
		}
	}
}

void renderer::begin_frame()
{
	HRESULT result = (*d3dintf->device.clear)(m_device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

	m_shaders->begin_frame();

	window().m_primlist->acquire_lock();

	// first update any textures
	m_texture_manager->update_textures();

	// begin the scene
	mtlog_add("drawd3d_window_draw: begin_scene");
	result = (*d3dintf->device.begin_scene)(m_device);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device begin_scene call\n", (int)result);

	m_lockedbuf = NULL;

	if(m_shaders->enabled())
	{
		m_hlsl_buf = (void*)mesh_alloc(6);
		m_shaders->init_fsfx_quad(m_hlsl_buf);
	}

	m_line_count = 0;

	// loop over primitives
	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
		if (prim->type == render_primitive::LINE && PRIMFLAG_GET_VECTOR(prim->flags))
			m_line_count++;
}

void renderer::process_primitives()
{
	// Rotating index for vector time offsets
	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		switch (prim->type)
		{
			case render_primitive::LINE:
				if (PRIMFLAG_GET_VECTOR(prim->flags))
				{
					if (m_line_count > 0)
						batch_vectors();
					else
						continue;
				}
				else
				{
					draw_line(prim);
				}
				break;

			case render_primitive::QUAD:
				draw_quad(prim);
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
	}
}

void renderer::end_frame()
{
	window().m_primlist->release_lock();

	// flush any pending polygons
	primitive_flush_pending();

	m_shaders->end_frame();

	// finish the scene
	HRESULT result = (*d3dintf->device.end_scene)(m_device);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device end_scene call\n", (int)result);

	// present the current buffers
	result = (*d3dintf->device.present)(m_device, NULL, NULL, NULL, NULL, 0);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device present call\n", (int)result);
}

//============================================================
//  device_create
//============================================================

int renderer::device_create(HWND device_hwnd)
{
	// if a device exists, free it
	if (m_device != NULL)
	{
		device_delete();
	}

	// verify the caps
	int verify = device_verify_caps();
	if (verify == 2)
	{
		osd_printf_error("Error: Device does not meet minimum requirements for Direct3D rendering\n");
		return 1;
	}
	if (verify == 1)
	{
		osd_printf_warning("Warning: Device may not perform well for Direct3D rendering\n");
	}

	// verify texture formats
	HRESULT result = (*d3dintf->d3d.check_device_format)(d3dintf, m_adapter, D3DDEVTYPE_HAL, m_pixformat, 0, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8);
	if (result != D3D_OK)
	{
		osd_printf_error("Error: A8R8G8B8 format textures not supported\n");
		return 1;
	}

	m_texture_manager = global_alloc(texture_manager(this));

try_again:
	// try for XRGB first
	m_screen_format = D3DFMT_X8R8G8B8;
	result = (*d3dintf->d3d.check_device_format)(d3dintf, m_adapter, D3DDEVTYPE_HAL, m_pixformat, m_texture_manager->is_dynamic_supported() ? D3DUSAGE_DYNAMIC : 0, D3DRTYPE_TEXTURE, m_screen_format);
	if (result != D3D_OK)
	{
		// if not, try for ARGB
		m_screen_format = D3DFMT_A8R8G8B8;
		result = (*d3dintf->d3d.check_device_format)(d3dintf, m_adapter, D3DDEVTYPE_HAL, m_pixformat, m_texture_manager->is_dynamic_supported() ? D3DUSAGE_DYNAMIC : 0, D3DRTYPE_TEXTURE, m_screen_format);
		if (result != D3D_OK && m_texture_manager->is_dynamic_supported())
		{
			m_texture_manager->set_dynamic_supported(FALSE);
			goto try_again;
		}
		if (result != D3D_OK)
		{
			osd_printf_error("Error: unable to configure a screen texture format\n");
			return 1;
		}
	}

	// initialize the D3D presentation parameters
	memset(&m_presentation, 0, sizeof(m_presentation));
	m_presentation.BackBufferWidth               = m_width;
	m_presentation.BackBufferHeight              = m_height;
	m_presentation.BackBufferFormat              = m_pixformat;
	m_presentation.BackBufferCount               = video_config.triplebuf ? 2 : 1;
	m_presentation.MultiSampleType               = D3DMULTISAMPLE_NONE;
	m_presentation.SwapEffect                    = D3DSWAPEFFECT_DISCARD;
	m_presentation.hDeviceWindow                 = window().m_hwnd;
	m_presentation.Windowed                      = !window().fullscreen() || window().win_has_menu();
	m_presentation.EnableAutoDepthStencil        = FALSE;
	m_presentation.AutoDepthStencilFormat        = D3DFMT_D16;
	m_presentation.Flags                         = 0;
	m_presentation.FullScreen_RefreshRateInHz    = m_refresh;
	m_presentation.PresentationInterval          = ((video_config.triplebuf && window().fullscreen()) ||
													video_config.waitvsync || video_config.syncrefresh) ?
													D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

	// create the D3D device
	result = (*d3dintf->d3d.create_device)(d3dintf, m_adapter, D3DDEVTYPE_HAL, device_hwnd,
					D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE, &m_presentation, &m_device);
	if (result != D3D_OK)
	{
		// if we got a "DEVICELOST" error, it may be transitory; count it and only fail if
		// we exceed a threshold
		if (result == D3DERR_DEVICELOST)
		{
			m_create_error_count++;
			if (m_create_error_count < 10)
			{
				return 0;
			}
		}

		//  fatal error if we just can't do it
		osd_printf_error("Unable to create the Direct3D device (%08X)\n", (UINT32)result);
		return 1;
	}
	m_create_error_count = 0;
	osd_printf_verbose("Direct3D: Device created at %dx%d\n", m_width, m_height);

	// set the gamma if we need to
	if (window().fullscreen())
	{
		// only set the gamma if it's not 1.0f
		windows_options &options = downcast<windows_options &>(window().machine().options());
		float brightness = options.full_screen_brightness();
		float contrast = options.full_screen_contrast();
		float gamma = options.full_screen_gamma();
		if (brightness != 1.0f || contrast != 1.0f || gamma != 1.0f)
		{
			// warn if we can't do it
			if (!m_gamma_supported)
			{
				osd_printf_warning("Direct3D: Warning - device does not support full screen gamma correction.\n");
			}
			else
			{
				// create a standard ramp and set it
				D3DGAMMARAMP ramp;
				for (int i = 0; i < 256; i++)
				{
					ramp.red[i] = ramp.green[i] = ramp.blue[i] = apply_brightness_contrast_gamma(i, brightness, contrast, gamma) << 8;
				}
				(*d3dintf->device.set_gamma_ramp)(m_device, 0, &ramp);
			}
		}
	}

	// create shader options only once
	if (m_shaders_options == NULL)
	{
		m_shaders_options = (hlsl_options*)global_alloc_clear(hlsl_options);
		m_shaders_options->params_init = false;
	}

	m_shaders = (shaders*)global_alloc_clear(shaders);
	m_shaders->init(d3dintf, &window().machine(), this);

	int failed = m_shaders->create_resources(false);
	if (failed)
	{
		return failed;
	}

	return device_create_resources();
}


//============================================================
//  device_create_resources
//============================================================

int renderer::device_create_resources()
{
	// allocate a vertex buffer to use
	HRESULT result = (*d3dintf->device.create_vertex_buffer)(m_device,
				sizeof(vertex) * VERTEX_BUFFER_SIZE,
				D3DUSAGE_DYNAMIC | D3DUSAGE_SOFTWAREPROCESSING | D3DUSAGE_WRITEONLY,
				VERTEX_BASE_FORMAT | ((m_shaders->enabled() && d3dintf->post_fx_available) ? D3DFVF_XYZW : D3DFVF_XYZRHW),
				D3DPOOL_DEFAULT, &m_vertexbuf);
	if (result != D3D_OK)
	{
		osd_printf_error("Error creating vertex buffer (%08X)\n", (UINT32)result);
		return 1;
	}

	// set the vertex format
	result = (*d3dintf->device.set_vertex_format)(m_device, (D3DFORMAT)(VERTEX_BASE_FORMAT | ((m_shaders->enabled() &&
		d3dintf->post_fx_available) ? D3DFVF_XYZW : D3DFVF_XYZRHW)));
	if (result != D3D_OK)
	{
		osd_printf_error("Error setting vertex format (%08X)\n", (UINT32)result);
		return 1;
	}

	// set the fixed render state
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ZENABLE, D3DZB_FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_FILLMODE, D3DFILL_SOLID);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_SHADEMODE, D3DSHADE_FLAT);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ZWRITEENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ALPHATESTENABLE, TRUE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_LASTPIXEL, TRUE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_CULLMODE, D3DCULL_NONE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ZFUNC, D3DCMP_LESS);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ALPHAREF, 0);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_DITHERENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_FOGENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_SPECULARENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_STENCILENABLE, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_WRAP0, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_CLIPPING, TRUE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_LIGHTING, FALSE);
	result = (*d3dintf->device.set_render_state)(m_device, D3DRS_COLORVERTEX, TRUE);

	result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(m_device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	result = (*d3dintf->device.set_texture_stage_state)(m_device, 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	// reset the local states to force updates
	reset_render_states();

	// clear the buffer
	result = (*d3dintf->device.clear)(m_device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	result = (*d3dintf->device.present)(m_device, NULL, NULL, NULL, NULL, 0);

	m_texture_manager->create_resources();

	return 0;
}


//============================================================
//  device_delete
//============================================================

renderer::~renderer()
{
	device_delete();

	if (m_shaders_options != NULL)
	{
		global_free(m_shaders_options);
		m_shaders_options = NULL;
	}
}

void renderer::device_delete()
{
	if (m_shaders != NULL)
	{
		// free our effects
		m_shaders->delete_resources(false);

		// delete the HLSL interface
		global_free(m_shaders);
	}

	// free our base resources
	device_delete_resources();

	if (m_texture_manager != NULL)
	{
		global_free(m_texture_manager);
		m_texture_manager = NULL;
	}

	// free the device itself
	if (m_device != NULL)
	{
		(*d3dintf->device.reset)(m_device, &m_presentation);
		(*d3dintf->device.release)(m_device);
		m_device = NULL;
	}
}


//============================================================
//  device_delete_resources
//============================================================

void renderer::device_delete_resources()
{
	if (m_texture_manager != NULL)
	{
		m_texture_manager->delete_resources();
	}

	// free the vertex buffer
	if (m_vertexbuf != NULL)
	{
		(*d3dintf->vertexbuf.release)(m_vertexbuf);
		m_vertexbuf = NULL;
	}
}


//============================================================
//  device_verify_caps
//============================================================

int renderer::device_verify_caps()
{
	int retval = 0;

	DWORD tempcaps;
	HRESULT result = (*d3dintf->d3d.get_caps_dword)(d3dintf, m_adapter, D3DDEVTYPE_HAL, CAPS_MAX_PS30_INSN_SLOTS, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D Error %08X during get_caps_dword call\n", (int)result);
	if (tempcaps < 512)
	{
		osd_printf_verbose("Direct3D: Warning - Device does not support Pixel Shader 3.0, falling back to non-PS rendering\n");
		d3dintf->post_fx_available = false;
	}

	// verify presentation capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, m_adapter, D3DDEVTYPE_HAL, CAPS_PRESENTATION_INTERVALS, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DPRESENT_INTERVAL_IMMEDIATE))
	{
		osd_printf_verbose("Direct3D: Error - Device does not support immediate presentations\n");
		retval = 2;
	}
	if (!(tempcaps & D3DPRESENT_INTERVAL_ONE))
	{
		osd_printf_verbose("Direct3D: Error - Device does not support per-refresh presentations\n");
		retval = 2;
	}

	// verify device capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, m_adapter, D3DDEVTYPE_HAL, CAPS_DEV_CAPS, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DDEVCAPS_CANRENDERAFTERFLIP))
	{
		osd_printf_verbose("Direct3D: Warning - Device does not support queued rendering after a page flip\n");
		retval = 1;
	}
	if (!(tempcaps & D3DDEVCAPS_HWRASTERIZATION))
	{
		osd_printf_verbose("Direct3D: Warning - Device does not support hardware rasterization\n");
		retval = 1;
	}

	// verify texture operation capabilities
	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, m_adapter, D3DDEVTYPE_HAL, CAPS_TEXTURE_OP_CAPS, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	if (!(tempcaps & D3DTEXOPCAPS_MODULATE))
	{
		osd_printf_verbose("Direct3D: Warning - Device does not support texture modulation\n");
		retval = 1;
	}

	// set a simpler flag to indicate mod2x and mod4x texture modes
	m_mod2x_supported = ((tempcaps & D3DTEXOPCAPS_MODULATE2X) != 0);
	m_mod4x_supported = ((tempcaps & D3DTEXOPCAPS_MODULATE4X) != 0);

	result = (*d3dintf->d3d.get_caps_dword)(d3dintf, m_adapter, D3DDEVTYPE_HAL, CAPS_CAPS2, &tempcaps);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during get_caps_dword call\n", (int)result);
	m_gamma_supported = ((tempcaps & D3DCAPS2_FULLSCREENGAMMA) != 0);

	return retval;
}


//============================================================
//  device_test_cooperative
//============================================================

int renderer::device_test_cooperative()
{
	// check our current status; if we lost the device, punt to GDI
	HRESULT result = (*d3dintf->device.test_cooperative_level)(m_device);
	if (result == D3DERR_DEVICELOST)
		return 1;

	// if we're able to reset ourselves, try it
	if (result == D3DERR_DEVICENOTRESET)
	{
		osd_printf_verbose("Direct3D: resetting device\n");

		// free all existing resources and call reset on the device
		m_shaders->delete_resources(true);
		device_delete_resources();
		result = (*d3dintf->device.reset)(m_device, &m_presentation);

		// if it didn't work, punt to GDI
		if (result != D3D_OK)
		{
			osd_printf_error("Unable to reset, result %08x\n", (UINT32)result);
			return 1;
		}

		// try to create the resources again; if that didn't work, delete the whole thing
		if (device_create_resources())
		{
			osd_printf_verbose("Direct3D: failed to recreate resources for device; failing permanently\n");
			device_delete();
			return 1;
		}

		if (m_shaders->create_resources(true))
		{
			osd_printf_verbose("Direct3D: failed to recreate HLSL resources for device; failing permanently\n");
			device_delete();
			return 1;
		}
	}
	return 0;
}


//============================================================
//  config_adapter_mode
//============================================================

int renderer::config_adapter_mode()
{
	adapter_identifier identifier;

	// choose the monitor number
	m_adapter = get_adapter_for_monitor();

	// get the identifier
	HRESULT result = (*d3dintf->d3d.get_adapter_identifier)(d3dintf, m_adapter, 0, &identifier);
	if (result != D3D_OK)
	{
		osd_printf_error("Error getting identifier for adapter #%d\n", m_adapter);
		return 1;
	}
	osd_printf_verbose("Direct3D: Configuring adapter #%d = %s\n", m_adapter, identifier.Description);

	// get the current display mode
	result = (*d3dintf->d3d.get_adapter_display_mode)(d3dintf, m_adapter, &m_origmode);
	if (result != D3D_OK)
	{
		osd_printf_error("Error getting mode for adapter #%d\n", m_adapter);
		return 1;
	}

	// choose a resolution: window mode case
	if (!window().fullscreen() || !video_config.switchres || window().win_has_menu())
	{
		RECT client;

		// bounds are from the window client rect
		GetClientRectExceptMenu(window().m_hwnd, &client, window().fullscreen());
		m_width = client.right - client.left;
		m_height = client.bottom - client.top;

		// pix format is from the current mode
		m_pixformat = m_origmode.Format;
		m_refresh = 0;

		// make sure it's a pixel format we can get behind
		if (m_pixformat != D3DFMT_X1R5G5B5 && m_pixformat != D3DFMT_R5G6B5 && m_pixformat != D3DFMT_X8R8G8B8)
		{
			osd_printf_error("Device %s currently in an unsupported mode\n", window().monitor()->devicename());
			return 1;
		}
	}

	// choose a resolution: full screen mode case
	else
	{
		// default to the current mode exactly
		m_width = m_origmode.Width;
		m_height = m_origmode.Height;
		m_pixformat = m_origmode.Format;
		m_refresh = m_origmode.RefreshRate;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			pick_best_mode();
	}

	// see if we can handle the device type
	result = (*d3dintf->d3d.check_device_type)(d3dintf, m_adapter, D3DDEVTYPE_HAL, m_pixformat, m_pixformat, !window().fullscreen());
	if (result != D3D_OK)
	{
		osd_printf_error("Proposed video mode not supported on device %s\n", window().monitor()->devicename());
		return 1;
	}
	return 0;
}


//============================================================
//  get_adapter_for_monitor
//============================================================

int renderer::get_adapter_for_monitor()
{
	int maxadapter = (*d3dintf->d3d.get_adapter_count)(d3dintf);

	// iterate over adapters until we error or find a match
	for (int adapternum = 0; adapternum < maxadapter; adapternum++)
	{
		// get the monitor for this adapter
		HMONITOR curmonitor = (*d3dintf->d3d.get_adapter_monitor)(d3dintf, adapternum);

		// if we match the proposed monitor, this is it
		if (curmonitor == *((HMONITOR *)window().monitor()->oshandle()))
		{
			return adapternum;
		}
	}

	// default to the default
	return D3DADAPTER_DEFAULT;
}


//============================================================
//  pick_best_mode
//============================================================

void renderer::pick_best_mode()
{
	double target_refresh = 60.0;
	INT32 minwidth, minheight;
	float best_score = 0.0f;

	// determine the refresh rate of the primary screen
	const screen_device *primary_screen = window().machine().config().first_screen();
	if (primary_screen != NULL)
	{
		target_refresh = ATTOSECONDS_TO_HZ(primary_screen->refresh_attoseconds());
	}

	// determine the minimum width/height for the selected target
	// note: technically we should not be calling this from an alternate window
	// thread; however, it is only done during init time, and the init code on
	// the main thread is waiting for us to finish, so it is safe to do so here
	window().target()->compute_minimum_size(minwidth, minheight);

	// use those as the target for now
	INT32 target_width = minwidth;
	INT32 target_height = minheight;

	// determine the maximum number of modes
	int maxmodes = (*d3dintf->d3d.get_adapter_mode_count)(d3dintf, m_adapter, D3DFMT_X8R8G8B8);

	// enumerate all the video modes and find the best match
	osd_printf_verbose("Direct3D: Selecting video mode...\n");
	for (int modenum = 0; modenum < maxmodes; modenum++)
	{
		// check this mode
		D3DDISPLAYMODE mode;
		HRESULT result = (*d3dintf->d3d.enum_adapter_modes)(d3dintf, m_adapter, D3DFMT_X8R8G8B8, modenum, &mode);
		if (result != D3D_OK)
			break;

		// skip non-32 bit modes
		if (mode.Format != D3DFMT_X8R8G8B8)
			continue;

		// compute initial score based on difference between target and current
		float size_score = 1.0f / (1.0f + fabs((float)(mode.Width - target_width)) + fabs((float)(mode.Height - target_height)));

		// if the mode is too small, give a big penalty
		if (mode.Width < minwidth || mode.Height < minheight)
			size_score *= 0.01f;

		// if mode is smaller than we'd like, it only scores up to 0.1
		if (mode.Width < target_width || mode.Height < target_height)
			size_score *= 0.1f;

		// if we're looking for a particular mode, that's a winner
		if (mode.Width == window().m_win_config.width && mode.Height == window().m_win_config.height)
			size_score = 2.0f;

		// compute refresh score
		float refresh_score = 1.0f / (1.0f + fabs((double)mode.RefreshRate - target_refresh));

		// if refresh is smaller than we'd like, it only scores up to 0.1
		if ((double)mode.RefreshRate < target_refresh)
			refresh_score *= 0.1f;

		// if we're looking for a particular refresh, make sure it matches
		if (mode.RefreshRate == window().m_win_config.refresh)
			refresh_score = 2.0f;

		// weight size and refresh equally
		float final_score = size_score + refresh_score;

		// best so far?
		osd_printf_verbose("  %4dx%4d@%3dHz -> %f\n", mode.Width, mode.Height, mode.RefreshRate, final_score * 1000.0f);
		if (final_score > best_score)
		{
			best_score = final_score;
			m_width = mode.Width;
			m_height = mode.Height;
			m_pixformat = mode.Format;
			m_refresh = mode.RefreshRate;
		}
	}
	osd_printf_verbose("Direct3D: Mode selected = %4dx%4d@%3dHz\n", m_width, m_height, m_refresh);
}


//============================================================
//  update_window_size
//============================================================

int renderer::update_window_size()
{
	// get the current window bounds
	RECT client;
	GetClientRectExceptMenu(window().m_hwnd, &client, window().fullscreen());

	// if we have a device and matching width/height, nothing to do
	if (m_device != NULL && rect_width(&client) == m_width && rect_height(&client) == m_height)
	{
		// clear out any pending resizing if the area didn't change
		if (window().m_resize_state == RESIZE_STATE_PENDING)
			window().m_resize_state = RESIZE_STATE_NORMAL;
		return FALSE;
	}

	// if we're in the middle of resizing, leave it alone as well
	if (window().m_resize_state == RESIZE_STATE_RESIZING)
		return FALSE;

	// set the new bounds and create the device again
	m_width = rect_width(&client);
	m_height = rect_height(&client);
	if (device_create(window().m_focus_hwnd))
		return FALSE;

	// reset the resize state to normal, and indicate we made a change
	window().m_resize_state = RESIZE_STATE_NORMAL;
	return TRUE;
}


//============================================================
//  batch_vectors
//============================================================

void renderer::batch_vectors()
{
	windows_options &options = downcast<windows_options &>(window().machine().options());

	int vector_size = (options.antialias() ? 24 : 6);
	m_vectorbatch = mesh_alloc(m_line_count * vector_size);
	m_batchindex = 0;

	static int start_index = 0;
	int line_index = 0;
	float period = options.screen_vector_time_period();
	UINT32 cached_flags = 0;
	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		switch (prim->type)
		{
			case render_primitive::LINE:
				if (PRIMFLAG_GET_VECTOR(prim->flags))
				{
					if (period == 0.0f || m_line_count == 0)
					{
						batch_vector(prim, 1.0f);
					}
					else
					{
						batch_vector(prim, (float)(start_index + line_index) / ((float)m_line_count * period));
						line_index++;
					}
					cached_flags = prim->flags;
				}
				break;

			default:
				// Skip
				break;
		}
	}

	// now add a polygon entry
	m_poly[m_numpolys].init(D3DPT_TRIANGLELIST, m_line_count * (options.antialias() ? 8 : 2), vector_size * m_line_count, cached_flags,
		m_texture_manager->get_vector_texture(), D3DTOP_MODULATE, 0.0f, 1.0f, 0.0f, 0.0f);
	m_numpolys++;

	start_index += (int)((float)line_index * period);
	if (m_line_count > 0)
	{
		start_index %= m_line_count;
	}

	m_line_count = 0;
}

void renderer::batch_vector(const render_primitive *prim, float line_time)
{
	// compute the effective width based on the direction of the line
	float effwidth = prim->width;
	if (effwidth < 0.5f)
	{
		effwidth = 0.5f;
	}

	// determine the bounds of a quad to draw this line
	render_bounds b0, b1;
	render_line_to_quad(&prim->bounds, effwidth, &b0, &b1);

	// iterate over AA steps
	for (const line_aa_step *step = PRIMFLAG_GET_ANTIALIAS(prim->flags) ? line_aa_4step : line_aa_1step;
		step->weight != 0; step++)
	{
		// get a pointer to the vertex buffer
		if (m_vectorbatch == NULL)
			return;

		m_vectorbatch[m_batchindex + 0].x = b0.x0 + step->xoffs;
		m_vectorbatch[m_batchindex + 0].y = b0.y0 + step->yoffs;
		m_vectorbatch[m_batchindex + 1].x = b0.x1 + step->xoffs;
		m_vectorbatch[m_batchindex + 1].y = b0.y1 + step->yoffs;
		m_vectorbatch[m_batchindex + 2].x = b1.x0 + step->xoffs;
		m_vectorbatch[m_batchindex + 2].y = b1.y0 + step->yoffs;

		m_vectorbatch[m_batchindex + 3].x = b0.x1 + step->xoffs;
		m_vectorbatch[m_batchindex + 3].y = b0.y1 + step->yoffs;
		m_vectorbatch[m_batchindex + 4].x = b1.x0 + step->xoffs;
		m_vectorbatch[m_batchindex + 4].y = b1.y0 + step->yoffs;
		m_vectorbatch[m_batchindex + 5].x = b1.x1 + step->xoffs;
		m_vectorbatch[m_batchindex + 5].y = b1.y1 + step->yoffs;

		float dx = b1.x1 - b0.x1;
		float dy = b1.y1 - b0.y1;
		float line_length = sqrtf(dx * dx + dy * dy);

		// determine the color of the line
		INT32 r = (INT32)(prim->color.r * step->weight * 255.0f);
		INT32 g = (INT32)(prim->color.g * step->weight * 255.0f);
		INT32 b = (INT32)(prim->color.b * step->weight * 255.0f);
		INT32 a = (INT32)(prim->color.a * 255.0f);
		if (r > 255 || g > 255 || b > 255)
		{
			if (r > 2*255 || g > 2*255 || b > 2*255)
			{
				r >>= 2; g >>= 2; b >>= 2;
			}
			else
			{
				r >>= 1; g >>= 1; b >>= 1;
			}
		}
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		if (a > 255) a = 255;
		DWORD color = D3DCOLOR_ARGB(a, r, g, b);

		vec2f& start = (get_vector_texture() ? get_vector_texture()->get_uvstart() : get_default_texture()->get_uvstart());
		vec2f& stop = (get_vector_texture() ? get_vector_texture()->get_uvstop() : get_default_texture()->get_uvstop());

		m_vectorbatch[m_batchindex + 0].u0 = start.c.x;
		m_vectorbatch[m_batchindex + 0].v0 = start.c.y;
		m_vectorbatch[m_batchindex + 1].u0 = start.c.x;
		m_vectorbatch[m_batchindex + 1].v0 = stop.c.y;
		m_vectorbatch[m_batchindex + 2].u0 = stop.c.x;
		m_vectorbatch[m_batchindex + 2].v0 = start.c.y;

		m_vectorbatch[m_batchindex + 3].u0 = start.c.x;
		m_vectorbatch[m_batchindex + 3].v0 = stop.c.y;
		m_vectorbatch[m_batchindex + 4].u0 = stop.c.x;
		m_vectorbatch[m_batchindex + 4].v0 = start.c.y;
		m_vectorbatch[m_batchindex + 5].u0 = stop.c.x;
		m_vectorbatch[m_batchindex + 5].v0 = stop.c.y;

		m_vectorbatch[m_batchindex + 0].u1 = line_length;
		m_vectorbatch[m_batchindex + 1].u1 = line_length;
		m_vectorbatch[m_batchindex + 2].u1 = line_length;
		m_vectorbatch[m_batchindex + 3].u1 = line_length;
		m_vectorbatch[m_batchindex + 4].u1 = line_length;
		m_vectorbatch[m_batchindex + 5].u1 = line_length;

		// set the color, Z parameters to standard values
		for (int i = 0; i < 6; i++)
		{
			m_vectorbatch[m_batchindex + i].z = 0.0f;
			m_vectorbatch[m_batchindex + i].rhw = 1.0f;
			m_vectorbatch[m_batchindex + i].color = color;
		}

		m_batchindex += 6;
	}
}


//============================================================
//  draw_line
//============================================================

void renderer::draw_line(const render_primitive *prim)
{
	// compute the effective width based on the direction of the line
	float effwidth = prim->width;
	if (effwidth < 0.5f)
	{
		effwidth = 0.5f;
	}

	// determine the bounds of a quad to draw this line
	render_bounds b0, b1;
	render_line_to_quad(&prim->bounds, effwidth, &b0, &b1);

	// iterate over AA steps
	for (const line_aa_step *step = PRIMFLAG_GET_ANTIALIAS(prim->flags) ? line_aa_4step : line_aa_1step;
		step->weight != 0; step++)
	{
		// get a pointer to the vertex buffer
		vertex *vertex = mesh_alloc(4);
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
		INT32 r = (INT32)(prim->color.r * step->weight * 255.0f);
		INT32 g = (INT32)(prim->color.g * step->weight * 255.0f);
		INT32 b = (INT32)(prim->color.b * step->weight * 255.0f);
		INT32 a = (INT32)(prim->color.a * 255.0f);
		if (r > 255) r = 255;
		if (g > 255) g = 255;
		if (b > 255) b = 255;
		if (a > 255) a = 255;
		DWORD color = D3DCOLOR_ARGB(a, r, g, b);

		vec2f& start = (get_vector_texture() ? get_vector_texture()->get_uvstart() : get_default_texture()->get_uvstart());
		vec2f& stop = (get_vector_texture() ? get_vector_texture()->get_uvstop() : get_default_texture()->get_uvstop());

		vertex[0].u0 = start.c.x;
		vertex[0].v0 = start.c.y;

		vertex[2].u0 = stop.c.x;
		vertex[2].v0 = start.c.y;

		vertex[1].u0 = start.c.x;
		vertex[1].v0 = stop.c.y;

		vertex[3].u0 = stop.c.x;
		vertex[3].v0 = stop.c.y;

		// set the color, Z parameters to standard values
		for (int i = 0; i < 4; i++)
		{
			vertex[i].z = 0.0f;
			vertex[i].rhw = 1.0f;
			vertex[i].color = color;
		}

		// now add a polygon entry
		m_poly[m_numpolys].init(D3DPT_TRIANGLESTRIP, 2, 4, prim->flags, get_vector_texture(),
								D3DTOP_MODULATE, 0.0f, 1.0f, 0.0f, 0.0f);
		m_numpolys++;
	}
}


//============================================================
//  draw_quad
//============================================================

void renderer::draw_quad(const render_primitive *prim)
{
	texture_info *texture = m_texture_manager->find_texinfo(&prim->texture, prim->flags);

	if (texture == NULL)
	{
		texture = get_default_texture();
	}

	// get a pointer to the vertex buffer
	vertex *vertex = mesh_alloc(4);
	if (vertex == NULL)
		return;

	// fill in the vertexes clockwise
	vertex[0].x = prim->bounds.x0;
	vertex[0].y = prim->bounds.y0;
	vertex[1].x = prim->bounds.x1;
	vertex[1].y = prim->bounds.y0;
	vertex[2].x = prim->bounds.x0;
	vertex[2].y = prim->bounds.y1;
	vertex[3].x = prim->bounds.x1;
	vertex[3].y = prim->bounds.y1;
	float width = prim->bounds.x1 - prim->bounds.x0;
	float height = prim->bounds.y1 - prim->bounds.y0;

	// set the texture coordinates
	if(texture != NULL)
	{
		vec2f& start = texture->get_uvstart();
		vec2f& stop = texture->get_uvstop();
		vec2f delta = stop - start;
		vertex[0].u0 = start.c.x + delta.c.x * prim->texcoords.tl.u;
		vertex[0].v0 = start.c.y + delta.c.y * prim->texcoords.tl.v;
		vertex[1].u0 = start.c.x + delta.c.x * prim->texcoords.tr.u;
		vertex[1].v0 = start.c.y + delta.c.y * prim->texcoords.tr.v;
		vertex[2].u0 = start.c.x + delta.c.x * prim->texcoords.bl.u;
		vertex[2].v0 = start.c.y + delta.c.y * prim->texcoords.bl.v;
		vertex[3].u0 = start.c.x + delta.c.x * prim->texcoords.br.u;
		vertex[3].v0 = start.c.y + delta.c.y * prim->texcoords.br.v;
	}

	// determine the color, allowing for over modulation
	INT32 r = (INT32)(prim->color.r * 255.0f);
	INT32 g = (INT32)(prim->color.g * 255.0f);
	INT32 b = (INT32)(prim->color.b * 255.0f);
	INT32 a = (INT32)(prim->color.a * 255.0f);
	DWORD modmode = D3DTOP_MODULATE;
	if (texture != NULL)
	{
		if (m_mod2x_supported && (r > 255 || g > 255 || b > 255))
		{
			if (m_mod4x_supported && (r > 2*255 || g > 2*255 || b > 2*255))
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
	DWORD color = D3DCOLOR_ARGB(a, r, g, b);

	// adjust half pixel X/Y offset, set the color, Z parameters to standard values
	for (int i = 0; i < 4; i++)
	{
		vertex[i].x -= 0.5f;
		vertex[i].y -= 0.5f;
		vertex[i].z = 0.0f;
		vertex[i].rhw = 1.0f;
		vertex[i].color = color;
	}

	// now add a polygon entry
	m_poly[m_numpolys].init(D3DPT_TRIANGLESTRIP, 2, 4, prim->flags, texture, modmode, width, height);
	m_numpolys++;
}

void poly_info::init(D3DPRIMITIVETYPE type, UINT32 count, UINT32 numverts,
							UINT32 flags, texture_info *texture, UINT32 modmode,
							float line_time, float line_length,
							float prim_width, float prim_height)
{
	init(type, count, numverts, flags, texture, modmode, prim_width, prim_height);
	m_line_time = line_time;
	m_line_length = line_length;
}

void poly_info::init(D3DPRIMITIVETYPE type, UINT32 count, UINT32 numverts,
							UINT32 flags, texture_info *texture, UINT32 modmode,
							float prim_width, float prim_height)
{
	m_type = type;
	m_count = count;
	m_numverts = numverts;
	m_flags = flags;
	m_texture = texture;
	m_modmode = modmode;
	m_prim_width = prim_width;
	m_prim_height = prim_height;
}


//============================================================
//  primitive_alloc
//============================================================

vertex *renderer::mesh_alloc(int numverts)
{
	HRESULT result;

	// if we're going to overflow, flush
	if (m_lockedbuf != NULL && m_numverts + numverts >= VERTEX_BUFFER_SIZE)
	{
		primitive_flush_pending();

		if(m_shaders->enabled())
		{
			m_hlsl_buf = (void*)mesh_alloc(6);
			m_shaders->init_fsfx_quad(m_hlsl_buf);
		}
	}

	// if we don't have a lock, grab it now
	if (m_lockedbuf == NULL)
	{
		result = (*d3dintf->vertexbuf.lock)(m_vertexbuf, 0, 0, (VOID **)&m_lockedbuf, D3DLOCK_DISCARD);
		if (result != D3D_OK)
			return NULL;
	}

	// if we already have the lock and enough room, just return a pointer
	if (m_lockedbuf != NULL && m_numverts + numverts < VERTEX_BUFFER_SIZE)
	{
		int oldverts = m_numverts;
		m_numverts += numverts;
		return &m_lockedbuf[oldverts];
	}
	return NULL;
}


//============================================================
//  primitive_flush_pending
//============================================================

void renderer::primitive_flush_pending()
{
	// ignore if we're not locked
	if (m_lockedbuf == NULL)
	{
		return;
	}

	// unlock the buffer
	HRESULT result = (*d3dintf->vertexbuf.unlock)(m_vertexbuf);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during vertex buffer unlock call\n", (int)result);
	m_lockedbuf = NULL;

	// set the stream
	result = (*d3dintf->device.set_stream_source)(m_device, 0, m_vertexbuf, sizeof(vertex));
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_stream_source call\n", (int)result);

	m_shaders->begin_draw();

	int vertnum = 0;
	if (m_shaders->enabled())
	{
		vertnum = 6;
	}

	// now do the polys
	for (int polynum = 0; polynum < m_numpolys; polynum++)
	{
		UINT32 flags = m_poly[polynum].get_flags();
		texture_info *texture = m_poly[polynum].get_texture();
		int newfilter;

		// set the texture if different
		set_texture(texture);

		// set filtering if different
		if (texture != NULL)
		{
			newfilter = FALSE;
			if (PRIMFLAG_GET_SCREENTEX(flags))
				newfilter = video_config.filter;
			set_filter(newfilter);
			set_wrap(PRIMFLAG_GET_TEXWRAP(flags) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
			set_modmode(m_poly[polynum].get_modmode());

			m_shaders->init_effect_info(&m_poly[polynum]);
		}

		// set the blendmode if different
		set_blendmode(PRIMFLAG_GET_BLENDMODE(flags));

		if (vertnum + m_poly[polynum].get_vertcount() > m_numverts)
		{
			osd_printf_error("Error: vertnum (%d) plus poly vertex count (%d) > %d\n", vertnum, m_poly[polynum].get_vertcount(), m_numverts);
			fflush(stdout);
		}

		assert(vertnum + m_poly[polynum].get_vertcount() <= m_numverts);

		if(m_shaders->enabled() && d3dintf->post_fx_available)
		{
			m_shaders->render_quad(&m_poly[polynum], vertnum);
		}
		else
		{
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(m_device, m_poly[polynum].get_type(), vertnum,
														m_poly[polynum].get_count());
			if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
		}

		vertnum += m_poly[polynum].get_vertcount();
	}

	m_shaders->end_draw();

	// reset the vertex count
	m_numverts = 0;
	m_numpolys = 0;
}


//============================================================
//  texture_info destructor
//============================================================

texture_info::~texture_info()
{
	if (m_d3dfinaltex != NULL)
	{
		if (m_d3dtex == m_d3dfinaltex)
		{
			m_d3dtex = NULL;
		}
		(*d3dintf->texture.release)(m_d3dfinaltex);
		m_d3dfinaltex = NULL;
	}
	if (m_d3dtex != NULL)
	{
		(*d3dintf->texture.release)(m_d3dtex);
		m_d3dtex = NULL;
	}
	if (m_d3dsurface != NULL)
	{
		(*d3dintf->surface.release)(m_d3dsurface);
		m_d3dsurface = NULL;
	}
}


//============================================================
//  texture_info constructor
//============================================================

texture_info::texture_info(texture_manager *manager, const render_texinfo* texsource, int prescale, UINT32 flags)
{
	HRESULT result;

	// fill in the core data
	m_texture_manager = manager;
	m_renderer = m_texture_manager->get_d3d();
	m_hash = m_texture_manager->texture_compute_hash(texsource, flags);
	m_flags = flags;
	m_texinfo = *texsource;
	m_xprescale = prescale;
	m_yprescale = prescale;

	m_d3dtex = NULL;
	m_d3dsurface = NULL;
	m_d3dfinaltex = NULL;

	// compute the size
	compute_size(texsource->width, texsource->height);

	// non-screen textures are easy
	if (!PRIMFLAG_GET_SCREENTEX(flags))
	{
		assert(PRIMFLAG_TEXFORMAT(flags) != TEXFORMAT_YUY16);
		result = (*d3dintf->device.create_texture)(m_renderer->get_device(), m_rawdims.c.x, m_rawdims.c.y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_d3dtex);
		if (result != D3D_OK)
			goto error;
		m_d3dfinaltex = m_d3dtex;
		m_type = TEXTURE_TYPE_PLAIN;
	}

	// screen textures are allocated differently
	else
	{
		D3DFORMAT format;
		DWORD usage = m_texture_manager->is_dynamic_supported() ? D3DUSAGE_DYNAMIC : 0;
		D3DPOOL pool = m_texture_manager->is_dynamic_supported() ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
		int maxdim = MAX(m_renderer->get_presentation()->BackBufferWidth, m_renderer->get_presentation()->BackBufferHeight);

		// pick the format
		if (PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_YUY16)
		{
			format = m_texture_manager->get_yuv_format();
		}
		else if (PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_ARGB32 || PRIMFLAG_GET_TEXFORMAT(flags) == TEXFORMAT_PALETTEA16)
		{
			format = D3DFMT_A8R8G8B8;
		}
		else
		{
			format = m_renderer->get_screen_format();
		}

		// don't prescale above screen size
		while (m_xprescale > 1 && m_rawdims.c.x * m_xprescale >= 2 * maxdim)
		{
			m_xprescale--;
		}
		while (m_xprescale > 1 && m_rawdims.c.x * m_xprescale > manager->get_max_texture_width())
		{
			m_xprescale--;
		}
		while (m_yprescale > 1 && m_rawdims.c.y * m_yprescale >= 2 * maxdim)
		{
			m_yprescale--;
		}
		while (m_yprescale > 1 && m_rawdims.c.y * m_yprescale > manager->get_max_texture_height())
		{
			m_yprescale--;
		}

		int prescale = m_renderer->window().prescale();
		if (m_xprescale != prescale || m_yprescale != prescale)
		{
			osd_printf_verbose("Direct3D: adjusting prescale from %dx%d to %dx%d\n", prescale, prescale, m_xprescale, m_yprescale);
		}

		// loop until we allocate something or error
		for (int attempt = 0; attempt < 2; attempt++)
		{
			// second attempt is always 1:1
			if (attempt == 1)
			{
				m_xprescale = m_yprescale = 1;
			}

			// screen textures with no prescaling are pretty easy
			if (m_xprescale == 1 && m_yprescale == 1)
			{
				result = (*d3dintf->device.create_texture)(m_renderer->get_device(), m_rawdims.c.x, m_rawdims.c.y, 1, usage, format, pool, &m_d3dtex);
				if (result == D3D_OK)
				{
					m_d3dfinaltex = m_d3dtex;
					m_type = m_texture_manager->is_dynamic_supported() ? TEXTURE_TYPE_DYNAMIC : TEXTURE_TYPE_PLAIN;
					if (m_renderer->get_shaders()->enabled() && !m_renderer->get_shaders()->register_texture(this))
					{
						goto error;
					}

					break;
				}
			}

			// screen textures with prescaling require two allocations
			else
			{
				// use an offscreen plain surface for stretching if supported
				// (won't work for YUY textures)
				if (m_texture_manager->is_stretch_supported() && PRIMFLAG_GET_TEXFORMAT(flags) != TEXFORMAT_YUY16)
				{
					result = (*d3dintf->device.create_offscreen_plain_surface)(m_renderer->get_device(), m_rawdims.c.x, m_rawdims.c.y, format, D3DPOOL_DEFAULT, &m_d3dsurface);
					if (result != D3D_OK)
					{
						continue;
					}
					m_type = TEXTURE_TYPE_SURFACE;
				}

				// otherwise, we allocate a dynamic texture for the source
				else
				{
					result = (*d3dintf->device.create_texture)(m_renderer->get_device(), m_rawdims.c.x, m_rawdims.c.y, 1, usage, format, pool, &m_d3dtex);
					if (result != D3D_OK)
					{
						continue;
					}
					m_type = m_texture_manager->is_dynamic_supported() ? TEXTURE_TYPE_DYNAMIC : TEXTURE_TYPE_PLAIN;
				}

				// for the target surface, we allocate a render target texture
				int scwidth = m_rawdims.c.x * m_xprescale;
				int scheight = m_rawdims.c.y * m_yprescale;

				// target surfaces typically cannot be YCbCr, so we always pick RGB in that case
				D3DFORMAT finalfmt = (format != m_texture_manager->get_yuv_format()) ? format : D3DFMT_A8R8G8B8;
				result = (*d3dintf->device.create_texture)(m_renderer->get_device(), scwidth, scheight, 1, D3DUSAGE_RENDERTARGET, finalfmt, D3DPOOL_DEFAULT, &m_d3dfinaltex);
				if (result == D3D_OK)
				{
					if (m_renderer->get_shaders()->enabled() && !m_renderer->get_shaders()->register_prescaled_texture(this))
					{
						goto error;
					}
					break;
				}
				(*d3dintf->texture.release)(m_d3dtex);
				m_d3dtex = NULL;
			}
		}
	}

	// copy the data to the texture
	set_data(texsource, flags);

	//texsource->osdhandle = (void*)this;
	// add us to the texture list
	if(m_texture_manager->get_texlist() != NULL)
		m_texture_manager->get_texlist()->m_prev = this;
	m_prev = NULL;
	m_next = m_texture_manager->get_texlist();
	m_texture_manager->set_texlist(this);
	return;

error:
	d3dintf->post_fx_available = false;
	osd_printf_error("Direct3D: Critical warning: A texture failed to allocate. Expect things to get bad quickly.\n");
	if (m_d3dsurface != NULL)
		(*d3dintf->surface.release)(m_d3dsurface);
	if (m_d3dtex != NULL)
		(*d3dintf->texture.release)(m_d3dtex);
}


//============================================================
//  texture_info::compute_size_subroutine
//============================================================

void texture_info::compute_size_subroutine(int texwidth, int texheight, int* p_width, int* p_height)
{
	int finalheight = texheight;
	int finalwidth = texwidth;

	// round width/height up to nearest power of 2 if we need to
	if (!(m_texture_manager->get_texture_caps() & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
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
	if (m_texture_manager->get_texture_caps() & D3DPTEXTURECAPS_SQUAREONLY)
	{
		if (finalwidth < finalheight)
			finalwidth = finalheight;
		else
			finalheight = finalwidth;
	}

	// adjust the aspect ratio if we need to
	while (finalwidth < finalheight && finalheight / finalwidth > m_texture_manager->get_max_texture_aspect())
	{
		finalwidth *= 2;
	}
	while (finalheight < finalwidth && finalwidth / finalheight > m_texture_manager->get_max_texture_aspect())
	{
		finalheight *= 2;
	}

	*p_width = finalwidth;
	*p_height = finalheight;
}


//============================================================
//  texture_info::compute_size
//============================================================

void texture_info::compute_size(int texwidth, int texheight)
{
	int finalheight = texheight;
	int finalwidth = texwidth;

	m_xborderpix = 0;
	m_yborderpix = 0;

	// if we're not wrapping, add a 1-2 pixel border on all sides
	if (ENABLE_BORDER_PIX && !(m_flags & PRIMFLAG_TEXWRAP_MASK))
	{
		// note we need 2 pixels in X for YUY textures
		m_xborderpix = (PRIMFLAG_GET_TEXFORMAT(m_flags) == TEXFORMAT_YUY16) ? 2 : 1;
		m_yborderpix = 1;
	}

	// compute final texture size
	finalwidth += 2 * m_xborderpix;
	finalheight += 2 * m_yborderpix;

	compute_size_subroutine(finalwidth, finalheight, &finalwidth, &finalheight);

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if (finalwidth > m_texture_manager->get_max_texture_width() || finalheight > m_texture_manager->get_max_texture_height())
	{
		finalheight = texheight;
		finalwidth = texwidth;

		m_xborderpix = 0;
		m_yborderpix = 0;

		compute_size_subroutine(finalwidth, finalheight, &finalwidth, &finalheight);
	}

	// if we're above the max width/height, do what?
	if (finalwidth > m_texture_manager->get_max_texture_width() || finalheight > m_texture_manager->get_max_texture_height())
	{
		static int printed = FALSE;
		if (!printed) osd_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth, finalheight, (int)m_texture_manager->get_max_texture_width(), (int)m_texture_manager->get_max_texture_height());
		printed = TRUE;
	}

	// compute the U/V scale factors
	m_start.c.x = (float)m_xborderpix / (float)finalwidth;
	m_start.c.y = (float)m_yborderpix / (float)finalheight;
	m_stop.c.x = (float)(texwidth + m_xborderpix) / (float)finalwidth;
	m_stop.c.y = (float)(texheight + m_yborderpix) / (float)finalheight;

	// set the final values
	m_rawdims.c.x = finalwidth;
	m_rawdims.c.y = finalheight;
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
			rgb_t srcpix = *src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
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
			rgb_t srcpix = *src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
		if (xborderpix)
		{
			rgb_t srcpix = *--src;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
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

void texture_info::set_data(const render_texinfo *texsource, UINT32 flags)
{
	D3DLOCKED_RECT rect;
	HRESULT result;

	// lock the texture
	switch (m_type)
	{
		default:
		case TEXTURE_TYPE_PLAIN:    result = (*d3dintf->texture.lock_rect)(m_d3dtex, 0, &rect, NULL, 0);                 break;
		case TEXTURE_TYPE_DYNAMIC:  result = (*d3dintf->texture.lock_rect)(m_d3dtex, 0, &rect, NULL, D3DLOCK_DISCARD);   break;
		case TEXTURE_TYPE_SURFACE:  result = (*d3dintf->surface.lock_rect)(m_d3dsurface, &rect, NULL, D3DLOCK_DISCARD);  break;
	}
	if (result != D3D_OK)
	{
		return;
	}

	// loop over Y
	int miny = 0 - m_yborderpix;
	int maxy = texsource->height + m_yborderpix;
	for (int dsty = miny; dsty < maxy; dsty++)
	{
		int srcy = (dsty < 0) ? 0 : (dsty >= texsource->height) ? texsource->height - 1 : dsty;
		void *dst = (BYTE *)rect.pBits + (dsty + m_yborderpix) * rect.Pitch;

		// switch off of the format and
		switch (PRIMFLAG_GET_TEXFORMAT(flags))
		{
			case TEXFORMAT_PALETTE16:
				copyline_palette16((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				break;

			case TEXFORMAT_PALETTEA16:
				copyline_palettea16((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				break;

			case TEXFORMAT_RGB32:
				copyline_rgb32((UINT32 *)dst, (UINT32 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				break;

			case TEXFORMAT_ARGB32:
				copyline_argb32((UINT32 *)dst, (UINT32 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				break;

			case TEXFORMAT_YUY16:
				if (m_texture_manager->get_yuv_format() == D3DFMT_YUY2)
					copyline_yuy16_to_yuy2((UINT16 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				else if (m_texture_manager->get_yuv_format() == D3DFMT_UYVY)
					copyline_yuy16_to_uyvy((UINT16 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				else
					copyline_yuy16_to_argb((UINT32 *)dst, (UINT16 *)texsource->base + srcy * texsource->rowpixels, texsource->width, texsource->palette, m_xborderpix);
				break;

			default:
				osd_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
				break;
		}
	}

	// unlock
	switch (m_type)
	{
		default:
		case TEXTURE_TYPE_PLAIN:    result = (*d3dintf->texture.unlock_rect)(m_d3dtex, 0);   break;
		case TEXTURE_TYPE_DYNAMIC:  result = (*d3dintf->texture.unlock_rect)(m_d3dtex, 0);   break;
		case TEXTURE_TYPE_SURFACE:  result = (*d3dintf->surface.unlock_rect)(m_d3dsurface);  break;
	}
	if (result != D3D_OK)
	{
		osd_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);
	}

	// prescale
	prescale();
}


//============================================================
//  texture_info::prescale
//============================================================

void texture_info::prescale()
{
	surface *scale_surface;
	HRESULT result;
	int i;

	// if we don't need to, just skip it
	if (m_d3dtex == m_d3dfinaltex)
		return;

	// for all cases, we need to get the surface of the render target
	result = (*d3dintf->texture.get_surface_level)(m_d3dfinaltex, 0, &scale_surface);
	if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during texture get_surface_level call\n", (int)result);

	// if we have an offscreen plain surface, we can just StretchRect to it
	if (m_type == TEXTURE_TYPE_SURFACE)
	{
		assert(m_d3dsurface != NULL);

		// set the source bounds
		RECT source;
		source.left = source.top = 0;
		source.right = m_texinfo.width + 2 * m_xborderpix;
		source.bottom = m_texinfo.height + 2 * m_yborderpix;

		// set the target bounds
		RECT dest;
		dest = source;
		dest.right *= m_xprescale;
		dest.bottom *= m_yprescale;

		// do the stretchrect
		result = (*d3dintf->device.stretch_rect)(m_renderer->get_device(), m_d3dsurface, &source, scale_surface, &dest, D3DTEXF_POINT);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device stretct_rect call\n", (int)result);
	}

	// if we are using a texture render target, we need to do more preparations
	else
	{
		surface *backbuffer;

		assert(m_d3dtex != NULL);

		// first remember the original render target and set the new one
		result = (*d3dintf->device.get_render_target)(m_renderer->get_device(), 0, &backbuffer);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device get_render_target call\n", (int)result);
		result = (*d3dintf->device.set_render_target)(m_renderer->get_device(), 0, scale_surface);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_target call 1\n", (int)result);
		m_renderer->reset_render_states();

		// start the scene
		result = (*d3dintf->device.begin_scene)(m_renderer->get_device());
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device begin_scene call\n", (int)result);

		// configure the rendering pipeline
		m_renderer->set_filter(FALSE);
		m_renderer->set_blendmode(BLENDMODE_NONE);
		result = (*d3dintf->device.set_texture)(m_renderer->get_device(), 0, m_d3dtex);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_texture call\n", (int)result);

		// lock the vertex buffer
		result = (*d3dintf->vertexbuf.lock)(m_renderer->get_vertex_buffer(), 0, 0, m_renderer->get_locked_buffer_ptr(), D3DLOCK_DISCARD);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during vertex buffer lock call\n", (int)result);

		// configure the X/Y coordinates on the target surface
		vertex *lockedbuf = m_renderer->get_locked_buffer();
		lockedbuf[0].x = -0.5f;
		lockedbuf[0].y = -0.5f;
		lockedbuf[1].x = (float)((m_texinfo.width + 2 * m_xborderpix) * m_xprescale) - 0.5f;
		lockedbuf[1].y = -0.5f;
		lockedbuf[2].x = -0.5f;
		lockedbuf[2].y = (float)((m_texinfo.height + 2 * m_yborderpix) * m_yprescale) - 0.5f;
		lockedbuf[3].x = (float)((m_texinfo.width + 2 * m_xborderpix) * m_xprescale) - 0.5f;
		lockedbuf[3].y = (float)((m_texinfo.height + 2 * m_yborderpix) * m_yprescale) - 0.5f;

		// configure the U/V coordintes on the source texture
		lockedbuf[0].u0 = 0.0f;
		lockedbuf[0].v0 = 0.0f;
		lockedbuf[1].u0 = (float)(m_texinfo.width + 2 * m_xborderpix) / (float)m_rawdims.c.x;
		lockedbuf[1].v0 = 0.0f;
		lockedbuf[2].u0 = 0.0f;
		lockedbuf[2].v0 = (float)(m_texinfo.height + 2 * m_yborderpix) / (float)m_rawdims.c.y;
		lockedbuf[3].u0 = (float)(m_texinfo.width + 2 * m_xborderpix) / (float)m_rawdims.c.x;
		lockedbuf[3].v0 = (float)(m_texinfo.height + 2 * m_yborderpix) / (float)m_rawdims.c.y;

		// reset the remaining vertex parameters
		for (i = 0; i < 4; i++)
		{
			lockedbuf[i].z = 0.0f;
			lockedbuf[i].rhw = 1.0f;
			lockedbuf[i].color = D3DCOLOR_ARGB(0xff,0xff,0xff,0xff);
		}

		// unlock the vertex buffer
		result = (*d3dintf->vertexbuf.unlock)(m_renderer->get_vertex_buffer());
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during vertex buffer unlock call\n", (int)result);
		m_renderer->set_locked_buffer(NULL);

		// set the stream and draw the triangle strip
		result = (*d3dintf->device.set_stream_source)(m_renderer->get_device(), 0, m_renderer->get_vertex_buffer(), sizeof(vertex));
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_stream_source call\n", (int)result);
		result = (*d3dintf->device.draw_primitive)(m_renderer->get_device(), D3DPT_TRIANGLESTRIP, 0, 2);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);

		// end the scene
		result = (*d3dintf->device.end_scene)(m_renderer->get_device());
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device end_scene call\n", (int)result);

		// reset the render target and release our reference to the backbuffer
		result = (*d3dintf->device.set_render_target)(m_renderer->get_device(), 0, backbuffer);
		if (result != D3D_OK) osd_printf_verbose("Direct3D: Error %08X during device set_render_target call 2\n", (int)result);
		(*d3dintf->surface.release)(backbuffer);
		m_renderer->reset_render_states();
	}

	// release our reference to the target surface
	(*d3dintf->surface.release)(scale_surface);
}


//============================================================
//  cache_target::~cache_target
//============================================================

cache_target::~cache_target()
{
	for (int index = 0; index < 11; index++)
	{
		if (bloom_texture[index] != NULL)
		{
			(*d3dintf->texture.release)(bloom_texture[index]);
			bloom_texture[index] = NULL;
		}
		if (bloom_target[index] != NULL)
		{
			(*d3dintf->surface.release)(bloom_target[index]);
			bloom_target[index] = NULL;
		}
	}

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
//  cache_target::init - initializes a target cache
//============================================================

bool cache_target::init(renderer *d3d, base *d3dintf, int width, int height, int prescale_x, int prescale_y)
{
	int bloom_index = 0;
	int bloom_size = (width < height) ? width : height;
	int bloom_width = width;
	int bloom_height = height;
	for (; bloom_size >= 2 && bloom_index < 11; bloom_size >>= 1)
	{
		bloom_width >>= 1;
		bloom_height >>= 1;

		HRESULT result = (*d3dintf->device.create_texture)(d3d->get_device(), bloom_width, bloom_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bloom_texture[bloom_index]);
		if (result != D3D_OK)
		{
			return false;
		}
		(*d3dintf->texture.get_surface_level)(bloom_texture[bloom_index], 0, &bloom_target[bloom_index]);

		bloom_index++;
	}

	HRESULT result = (*d3dintf->device.create_texture)(d3d->get_device(), width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &last_texture);
	if (result != D3D_OK)
	{
		return false;
	}
	(*d3dintf->texture.get_surface_level)(last_texture, 0, &last_target);

	target_width = width * prescale_x;
	target_height = height * prescale_y;

	return true;
}


//============================================================
//  render_target::~render_target
//============================================================

render_target::~render_target()
{
	for (int index = 0; index < 11; index++)
	{
		if (bloom_texture[index] != NULL)
		{
			(*d3dintf->texture.release)(bloom_texture[index]);
			bloom_texture[index] = NULL;
		}
		if (bloom_target[index] != NULL)
		{
			(*d3dintf->surface.release)(bloom_target[index]);
			bloom_target[index] = NULL;
		}
	}

	for (int index = 0; index < 2; index++)
	{
		if (native_texture[index] != NULL)
		{
			(*d3dintf->texture.release)(native_texture[index]);
			native_texture[index] = NULL;
		}
		if (native_target[index] != NULL)
		{
			(*d3dintf->surface.release)(native_target[index]);
			native_target[index] = NULL;
		}
		if (prescale_texture[index] != NULL)
		{
			(*d3dintf->texture.release)(prescale_texture[index]);
			prescale_texture[index] = NULL;
		}
		if (prescale_target[index] != NULL)
		{
			(*d3dintf->surface.release)(prescale_target[index]);
			prescale_target[index] = NULL;
		}
	}
}


//============================================================
//  render_target::init - initializes a render target
//============================================================

bool render_target::init(renderer *d3d, base *d3dintf, int width, int height, int prescale_x, int prescale_y)
{
	HRESULT result;

	for (int index = 0; index < 2; index++)
	{
		result = (*d3dintf->device.create_texture)(d3d->get_device(), width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &native_texture[index]);
		if (result != D3D_OK)
		{
			return false;
		}
		(*d3dintf->texture.get_surface_level)(native_texture[index], 0, &native_target[index]);

		result = (*d3dintf->device.create_texture)(d3d->get_device(), width * prescale_x, height * prescale_y, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &prescale_texture[index]);
		if (result != D3D_OK)
		{
			return false;
		}
		(*d3dintf->texture.get_surface_level)(prescale_texture[index], 0, &prescale_target[index]);
	}

	int bloom_index = 0;
	float bloom_size = (d3d->get_width() < d3d->get_height()) ? d3d->get_width() : d3d->get_height();
	float bloom_width = d3d->get_width();
	float bloom_height = d3d->get_height();
	for (; bloom_size >= 2.0f && bloom_index < 11; bloom_size *= 0.5f)
	{
		bloom_width *= 0.5f;
		bloom_height *= 0.5f;

		result = (*d3dintf->device.create_texture)(d3d->get_device(), (int)bloom_width, (int)bloom_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &bloom_texture[bloom_index]);
		if (result != D3D_OK)
		{
			return false;
		}
		(*d3dintf->texture.get_surface_level)(bloom_texture[bloom_index], 0, &bloom_target[bloom_index]);

		bloom_index++;
	}

	this->width = width;
	this->height = height;

	target_width = width * prescale_x;
	target_height = height * prescale_y;

	return true;
}

}
