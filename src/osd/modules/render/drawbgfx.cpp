// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Ryan Holtz,Dario Manesku,Branimir Karadzic,Aaron Giles
//============================================================
//
//  drawbgfx.cpp - BGFX renderer
//
//============================================================
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS)
// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(SDLMAME_WIN32)
#include <SDL2/SDL_syswm.h>
#endif
#else
#include "sdlinc.h"
#endif

// MAMEOS headers
#include "emu.h"
#include "window.h"
#include "rendutil.h"

#include <bgfx/bgfxplatform.h>
#include <bgfx/bgfx.h>
#include <bx/fpumath.h>
#include <bx/readerwriter.h>
#include <algorithm>

#include "drawbgfx.h"
#include "copyutil.h"
#include "bgfx/texturemanager.h"
#include "bgfx/targetmanager.h"
#include "bgfx/shadermanager.h"
#include "bgfx/effectmanager.h"
#include "bgfx/chainmanager.h"
#include "bgfx/effect.h"
#include "bgfx/texture.h"
#include "bgfx/target.h"
#include "bgfx/chain.h"
#include "bgfx/vertex.h"
#include "bgfx/uniform.h"
#include "bgfx/slider.h"

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

const uint16_t renderer_bgfx::CACHE_SIZE = 1024;
const uint32_t renderer_bgfx::PACKABLE_SIZE = 128;
const uint32_t renderer_bgfx::WHITE_HASH = 0x87654321;
const char* renderer_bgfx::WINDOW_PREFIX = "Window 0, ";

//============================================================
//  MACROS
//============================================================

#define GIBBERISH   	(0)

//============================================================
//  TYPES
//============================================================

bool renderer_bgfx::s_window_set = false;

//============================================================
//  renderer_bgfx::create
//============================================================

#ifdef OSD_SDL
static void* sdlNativeWindowHandle(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi))
	{
		return nullptr;
	}

#   if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	return (void*)wmi.info.x11.window;
#   elif BX_PLATFORM_OSX
	return wmi.info.cocoa.window;
#   elif BX_PLATFORM_WINDOWS
	return wmi.info.win.window;
#   elif BX_PLATFORM_STEAMLINK
	return wmi.info.vivante.window;
#   elif BX_PLATFORM_EMSCRIPTEN
	return nullptr;
#   endif // BX_PLATFORM_
}
#endif

int renderer_bgfx::create()
{
	// create renderer

    osd_options& options = downcast<osd_options &>(window().machine().options());
	osd_dim wdim = window().get_size();
	m_width[window().m_index] = wdim.width();
	m_height[window().m_index] = wdim.height();
	if (window().m_index == 0)
	{
		if (!s_window_set)
		{
			s_window_set = true;
			ScreenVertex::init();
		}
		else
		{
			bgfx::shutdown();
			bgfx::PlatformData blank_pd;
			memset(&blank_pd, 0, sizeof(bgfx::PlatformData));
			bgfx::setPlatformData(blank_pd);
		}
#ifdef OSD_WINDOWS
		bgfx::winSetHwnd(window().m_hwnd);
#else
		bgfx::sdlSetWindow(window().sdl_window());
#endif
		std::string backend(options.bgfx_backend());
		if (backend == "auto")
		{
			bgfx::init();
		}
		else if (backend == "dx9" || backend == "d3d9")
		{
			bgfx::init(bgfx::RendererType::Direct3D9);
		}
		else if (backend == "dx11" || backend == "d3d11")
		{
			bgfx::init(bgfx::RendererType::Direct3D11);
		}
		else if (backend == "gles")
		{
			bgfx::init(bgfx::RendererType::OpenGLES);
		}
		else if (backend == "glsl" || backend == "opengl")
		{
			bgfx::init(bgfx::RendererType::OpenGL);
		}
		else if (backend == "metal")
		{
			bgfx::init(bgfx::RendererType::Metal);
		}
		else
		{
			osd_printf_verbose("Unknown backend type '%s'\n", backend.c_str());
			assert(false);
		}
		bgfx::reset(m_width[window().m_index], m_height[window().m_index], video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
		// Enable debug text.
		bgfx::setDebug(options.bgfx_debug() ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
		m_dimensions = osd_dim(m_width[0], m_height[0]);
	}

	m_textures = new texture_manager();
	m_targets = new target_manager(options, *m_textures);

	m_shaders = new shader_manager(options);
	m_effects = new effect_manager(options, *m_shaders);

	if (window().m_index != 0)
	{
#ifdef OSD_WINDOWS
		m_framebuffer = m_targets->create_backbuffer(window().m_hwnd, m_width[window().m_index], m_height[window().m_index]);
#else
		m_framebuffer = m_targets->create_backbuffer(sdlNativeWindowHandle(window().sdl_window()), m_width[window().m_index], m_height[window().m_index]);
#endif
		bgfx::touch(window().m_index);
	}

	// Create program from shaders.
	m_gui_effect[0] = m_effects->effect("gui_opaque");
	m_gui_effect[1] = m_effects->effect("gui_blend");
	m_gui_effect[2] = m_effects->effect("gui_multiply");
	m_gui_effect[3] = m_effects->effect("gui_add");

	m_screen_effect[0] = m_effects->effect("screen_opaque");
	m_screen_effect[1] = m_effects->effect("screen_blend");
	m_screen_effect[2] = m_effects->effect("screen_multiply");
	m_screen_effect[3] = m_effects->effect("screen_add");

	m_chains = new chain_manager(options, *m_textures, *m_targets, *m_effects, m_width[window().m_index], m_height[window().m_index]);
	m_screen_chain = m_chains->chain(options.bgfx_screen_chain(), window().machine(), window().m_index);
    m_sliders_dirty = true;

	uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
	m_texture_cache = m_textures->create_texture("#cache", bgfx::TextureFormat::RGBA8, CACHE_SIZE, CACHE_SIZE, nullptr, flags);

    uint32_t shadow_flags = 0;//BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
    m_textures->create_png_texture(window().machine().options().art_path(), options.bgfx_shadow_mask(), "shadow", shadow_flags);

    memset(m_white, 0xff, sizeof(uint32_t) * 16 * 16);
	m_texinfo.push_back(rectangle_packer::packable_rectangle(WHITE_HASH, PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32), 16, 16, 16, nullptr, m_white));

	return 0;
}

//============================================================
//  destructor
//============================================================

renderer_bgfx::~renderer_bgfx()
{
	// Cleanup.
	delete m_chains;
	delete m_effects;
	delete m_shaders;
	delete m_textures;
	delete m_targets;
}

void renderer_bgfx::exit()
{
	bgfx::shutdown();
	s_window_set = false;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

#ifdef OSD_SDL
int renderer_bgfx::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x;
	*yt = y;
	if (*xt<0 || *xt >= m_dimensions.width())
		return 0;
	if (*yt<0 || *yt >= m_dimensions.height())
		return 0;
	return 1;
}
#endif

//============================================================
//  drawbgfx_window_draw
//============================================================

bgfx::VertexDecl ScreenVertex::ms_decl;

void renderer_bgfx::put_packed_quad(render_primitive *prim, UINT32 hash, ScreenVertex* vertex)
{
	rectangle_packer::packed_rectangle& rect = m_hash_to_entry[hash];
	float u0 = float(rect.x()) / float(CACHE_SIZE);
	float v0 = float(rect.y()) / float(CACHE_SIZE);
	float u1 = u0 + float(rect.width()) / float(CACHE_SIZE);
	float v1 = v0 + float(rect.height()) / float(CACHE_SIZE);
	u1 -= 0.5f / float(CACHE_SIZE);
	v1 -= 0.5f / float(CACHE_SIZE);
	u0 += 0.5f / float(CACHE_SIZE);
	v0 += 0.5f / float(CACHE_SIZE);
	UINT32 rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { u0, u1, u0, u1 };
	float v[4] = { v0, v0, v1, v1 };

	if (PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_SWAP_XY)
	{
		std::swap(u[1], u[2]);
		std::swap(v[1], v[2]);
	}

	if (PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_FLIP_X)
	{
		std::swap(u[0], u[1]);
		std::swap(v[0], v[1]);
		std::swap(u[2], u[3]);
		std::swap(v[2], v[3]);
	}

	if (PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_FLIP_Y)
	{
		std::swap(u[0], u[2]);
		std::swap(v[0], v[2]);
		std::swap(u[1], u[3]);
		std::swap(v[1], v[3]);
	}

	vertex[0].m_x = x[0]; // 0
	vertex[0].m_y = y[0];
	vertex[0].m_z = 0;
	vertex[0].m_rgba = rgba;
	vertex[0].m_u = u[0];
	vertex[0].m_v = v[0];

	vertex[1].m_x = x[1]; // 1
	vertex[1].m_y = y[1];
	vertex[1].m_z = 0;
	vertex[1].m_rgba = rgba;
	vertex[1].m_u = u[1];
	vertex[1].m_v = v[1];

	vertex[2].m_x = x[3]; // 3
	vertex[2].m_y = y[3];
	vertex[2].m_z = 0;
	vertex[2].m_rgba = rgba;
	vertex[2].m_u = u[3];
	vertex[2].m_v = v[3];

	vertex[3].m_x = x[3]; // 3
	vertex[3].m_y = y[3];
	vertex[3].m_z = 0;
	vertex[3].m_rgba = rgba;
	vertex[3].m_u = u[3];
	vertex[3].m_v = v[3];

	vertex[4].m_x = x[2]; // 2
	vertex[4].m_y = y[2];
	vertex[4].m_z = 0;
	vertex[4].m_rgba = rgba;
	vertex[4].m_u = u[2];
	vertex[4].m_v = v[2];

	vertex[5].m_x = x[0]; // 0
	vertex[5].m_y = y[0];
	vertex[5].m_z = 0;
	vertex[5].m_rgba = rgba;
	vertex[5].m_u = u[0];
	vertex[5].m_v = v[0];
}

void renderer_bgfx::process_screen_quad(int view, render_primitive* prim)
{
    uint32_t texture_flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
    if (video_config.filter == 0)
    {
        texture_flags |= BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
    }

    uint16_t tex_width(prim->texture.width);
    uint16_t tex_height(prim->texture.height);

    const bgfx::Memory* mem = mame_texture_data_to_bgfx_texture_data(prim->flags & PRIMFLAG_TEXFORMAT_MASK,
        tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);

    bgfx_texture *texture = new bgfx_texture("screen", bgfx::TextureFormat::RGBA8, tex_width, tex_height, mem);
    m_textures->add_provider("screen", texture);

    int screens = 0;
    screen_device_iterator iter(window().machine().root_device());
    for (const screen_device *screen = iter.first(); screen != nullptr; screen = iter.next())
    {
        screens++;
    }
    m_targets->update_guest_targets(tex_width, tex_height);
    m_targets->update_window_count(screens);
    
    m_screen_chain->process(prim, view, view / m_screen_chain->applicable_passes(), *m_textures, window(), get_blend_state(PRIMFLAG_GET_BLENDMODE(prim->flags)));

    m_textures->add_provider("screen", nullptr);
    delete texture;
}

void renderer_bgfx::render_post_screen_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
    ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer->data);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { prim->texcoords.tl.u, prim->texcoords.tr.u, prim->texcoords.bl.u, prim->texcoords.br.u };
	float v[4] = { prim->texcoords.tl.v, prim->texcoords.tr.v, prim->texcoords.bl.v, prim->texcoords.br.v };

	if (false)//PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_SWAP_XY)
	{
		std::swap(u[1], u[2]);
		std::swap(v[1], v[2]);
	}

	if (false)//PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_FLIP_X)
	{
		std::swap(u[0], u[1]);
		std::swap(v[0], v[1]);
		std::swap(u[2], u[3]);
		std::swap(v[2], v[3]);
	}

	if (false)//PRIMFLAG_GET_TEXORIENT(prim->flags) & ORIENTATION_FLIP_Y)
	{
		std::swap(u[0], u[2]);
		std::swap(v[0], v[2]);
		std::swap(u[1], u[3]);
		std::swap(v[1], v[3]);
	}

    vertex[0].m_x = x[0];
    vertex[0].m_y = y[0];
    vertex[0].m_z = 0;
    vertex[0].m_rgba = 0xffffffff;
    vertex[0].m_u = u[0];
    vertex[0].m_v = v[0];

    vertex[1].m_x = x[1];
    vertex[1].m_y = y[1];
    vertex[1].m_z = 0;
    vertex[1].m_rgba = 0xffffffff;
    vertex[1].m_u = u[1];
    vertex[1].m_v = v[1];

    vertex[2].m_x = x[3];
    vertex[2].m_y = y[3];
    vertex[2].m_z = 0;
    vertex[2].m_rgba = 0xffffffff;
    vertex[2].m_u = u[3];
    vertex[2].m_v = v[3];

    vertex[3].m_x = x[3];
    vertex[3].m_y = y[3];
    vertex[3].m_z = 0;
    vertex[3].m_rgba = 0xffffffff;
    vertex[3].m_u = u[3];
    vertex[3].m_v = v[3];

    vertex[4].m_x = x[2];
    vertex[4].m_y = y[2];
    vertex[4].m_z = 0;
    vertex[4].m_rgba = 0xffffffff;
    vertex[4].m_u = u[2];
    vertex[4].m_v = v[2];

    vertex[5].m_x = x[0];
    vertex[5].m_y = y[0];
    vertex[5].m_z = 0;
    vertex[5].m_rgba = 0xffffffff;
    vertex[5].m_u = u[0];
    vertex[5].m_v = v[0];

    UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
    bgfx::setVertexBuffer(buffer);
    bgfx::setTexture(0, m_screen_effect[blend]->uniform("s_tex")->handle(), m_targets->target("output" + std::to_string(screen))->texture());
    m_screen_effect[blend]->submit(view);
}

void renderer_bgfx::render_textured_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer)
{
	ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer->data);

	UINT32 rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

	vertex[0].m_x = prim->bounds.x0;
	vertex[0].m_y = prim->bounds.y0;
	vertex[0].m_z = 0;
	vertex[0].m_rgba = rgba;
	vertex[0].m_u = prim->texcoords.tl.u;
	vertex[0].m_v = prim->texcoords.tl.v;

	vertex[1].m_x = prim->bounds.x1;
	vertex[1].m_y = prim->bounds.y0;
	vertex[1].m_z = 0;
	vertex[1].m_rgba = rgba;
	vertex[1].m_u = prim->texcoords.tr.u;
	vertex[1].m_v = prim->texcoords.tr.v;

	vertex[2].m_x = prim->bounds.x1;
	vertex[2].m_y = prim->bounds.y1;
	vertex[2].m_z = 0;
	vertex[2].m_rgba = rgba;
	vertex[2].m_u = prim->texcoords.br.u;
	vertex[2].m_v = prim->texcoords.br.v;

	vertex[3].m_x = prim->bounds.x1;
	vertex[3].m_y = prim->bounds.y1;
	vertex[3].m_z = 0;
	vertex[3].m_rgba = rgba;
	vertex[3].m_u = prim->texcoords.br.u;
	vertex[3].m_v = prim->texcoords.br.v;

	vertex[4].m_x = prim->bounds.x0;
	vertex[4].m_y = prim->bounds.y1;
	vertex[4].m_z = 0;
	vertex[4].m_rgba = rgba;
	vertex[4].m_u = prim->texcoords.bl.u;
	vertex[4].m_v = prim->texcoords.bl.v;

	vertex[5].m_x = prim->bounds.x0;
	vertex[5].m_y = prim->bounds.y0;
	vertex[5].m_z = 0;
	vertex[5].m_rgba = rgba;
	vertex[5].m_u = prim->texcoords.tl.u;
	vertex[5].m_v = prim->texcoords.tl.v;

	uint32_t texture_flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
	if (video_config.filter == 0)
	{
		texture_flags |= BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
	}

    uint16_t tex_width(prim->texture.width);
    uint16_t tex_height(prim->texture.height);

    const bgfx::Memory* mem = mame_texture_data_to_bgfx_texture_data(prim->flags & PRIMFLAG_TEXFORMAT_MASK,
		tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);

	bgfx::TextureHandle texture = bgfx::createTexture2D(tex_width, tex_height, 1, bgfx::TextureFormat::RGBA8, texture_flags, mem);

	bgfx_effect** effects = PRIMFLAG_GET_SCREENTEX(prim->flags) ? m_screen_effect : m_gui_effect;

	UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
    bgfx::setVertexBuffer(buffer);
	bgfx::setTexture(0, effects[blend]->uniform("s_tex")->handle(), texture);
	effects[blend]->submit(view);

	bgfx::destroyTexture(texture);
}

#define MAX_TEMP_COORDS 100

void renderer_bgfx::put_polygon(const float* coords, UINT32 num_coords, float r, UINT32 rgba, ScreenVertex* vertex)
{
	float tempCoords[MAX_TEMP_COORDS * 3];
	float tempNormals[MAX_TEMP_COORDS * 2];

	rectangle_packer::packed_rectangle& rect = m_hash_to_entry[WHITE_HASH];
	float u0 = float(rect.x()) / float(CACHE_SIZE);
	float v0 = float(rect.y()) / float(CACHE_SIZE);

	num_coords = num_coords < MAX_TEMP_COORDS ? num_coords : MAX_TEMP_COORDS;

	for (uint32_t ii = 0, jj = num_coords - 1; ii < num_coords; jj = ii++)
	{
		const float* v0 = &coords[jj * 3];
		const float* v1 = &coords[ii * 3];
		float dx = v1[0] - v0[0];
		float dy = v1[1] - v0[1];
		float d = sqrtf(dx * dx + dy * dy);
		if (d > 0)
		{
			d = 1.0f / d;
			dx *= d;
			dy *= d;
		}

		tempNormals[jj * 2 + 0] = dy;
		tempNormals[jj * 2 + 1] = -dx;
	}

	for (uint32_t ii = 0, jj = num_coords - 1; ii < num_coords; jj = ii++)
	{
		float dlx0 = tempNormals[jj * 2 + 0];
		float dly0 = tempNormals[jj * 2 + 1];
		float dlx1 = tempNormals[ii * 2 + 0];
		float dly1 = tempNormals[ii * 2 + 1];
		float dmx = (dlx0 + dlx1) * 0.5f;
		float dmy = (dly0 + dly1) * 0.5f;
		float dmr2 = dmx * dmx + dmy * dmy;
		if (dmr2 > 0.000001f)
		{
			float scale = 1.0f / dmr2;
			if (scale > 10.0f)
			{
				scale = 10.0f;
			}

			dmx *= scale;
			dmy *= scale;
		}

		tempCoords[ii * 3 + 0] = coords[ii * 3 + 0] + dmx * r;
		tempCoords[ii * 3 + 1] = coords[ii * 3 + 1] + dmy * r;
		tempCoords[ii * 3 + 2] = coords[ii * 3 + 2];
	}

	int vertIndex = 0;
	UINT32 trans = rgba & 0x00ffffff;
	for (uint32_t ii = 0, jj = num_coords - 1; ii < num_coords; jj = ii++)
	{
		vertex[vertIndex].m_x = coords[ii * 3 + 0];
		vertex[vertIndex].m_y = coords[ii * 3 + 1];
		vertex[vertIndex].m_z = coords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = coords[jj * 3 + 0];
		vertex[vertIndex].m_y = coords[jj * 3 + 1];
		vertex[vertIndex].m_z = coords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[jj * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[jj * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[jj * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[jj * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[ii * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[ii * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = coords[ii * 3 + 0];
		vertex[vertIndex].m_y = coords[ii * 3 + 1];
		vertex[vertIndex].m_z = coords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;
	}

	for (uint32_t ii = 2; ii < num_coords; ++ii)
	{
		vertex[vertIndex].m_x = coords[0];
		vertex[vertIndex].m_y = coords[1];
		vertex[vertIndex].m_z = coords[2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = coords[(ii - 1) * 3 + 0];
		vertex[vertIndex].m_y = coords[(ii - 1) * 3 + 1];
		vertex[vertIndex].m_z = coords[(ii - 1) * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;

		vertex[vertIndex].m_x = coords[ii * 3 + 0];
		vertex[vertIndex].m_y = coords[ii * 3 + 1];
		vertex[vertIndex].m_z = coords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertex[vertIndex].m_u = u0;
		vertex[vertIndex].m_v = v0;
		vertIndex++;
	}
}

void renderer_bgfx::put_line(float x0, float y0, float x1, float y1, float r, UINT32 rgba, ScreenVertex* vertex, float fth)
{
	float dx = x1 - x0;
	float dy = y1 - y0;
	float d = sqrtf(dx * dx + dy * dy);
	if (d > 0.0001f)
	{
		d = 1.0f / d;
		dx *= d;
		dy *= d;
	}

	float nx = dy;
	float ny = -dx;
	float verts[4 * 3];
	r -= fth;
	r *= 0.5f;
	if (r < 0.01f)
	{
		r = 0.01f;
	}

	dx *= r;
	dy *= r;
	nx *= r;
	ny *= r;

	verts[0] = x0 - dx - nx;
	verts[1] = y0 - dy - ny;
	verts[2] = 0;

	verts[3] = x0 - dx + nx;
	verts[4] = y0 - dy + ny;
	verts[5] = 0;

	verts[6] = x1 + dx + nx;
	verts[7] = y1 + dy + ny;
	verts[8] = 0;

	verts[9] = x1 + dx - nx;
	verts[10] = y1 + dy - ny;
	verts[11] = 0;

	put_polygon(verts, 4, fth, rgba, vertex);
}

uint32_t renderer_bgfx::u32Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a = 255)
{
	return (a << 24) | (b << 16) | (g << 8) | r;
}

const bgfx::Memory* renderer_bgfx::mame_texture_data_to_bgfx_texture_data(UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
{
	const bgfx::Memory* mem = bgfx::alloc(width * height * 4);
	for (int y = 0; y < height; y++)
	{
		switch (format)
		{
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
                copy_util::copyline_palette16((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTEA16):
                copy_util::copyline_palettea16((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
                copy_util::copyline_yuy16_to_argb((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette, 1);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
                copy_util::copyline_argb32((UINT32*)mem->data + y * width, (UINT32*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
                copy_util::copyline_rgb32((UINT32*)mem->data + y * width, (UINT32*)base + y * rowpixels, width, palette);
				break;
			default:
				break;
		}
	}
	return mem;
}

int renderer_bgfx::handle_screen_chains()
{
	if (!m_screen_chain)
	{
		return 0;
	}

    window().m_primlist->acquire_lock();

    // process
    render_primitive *prim = window().m_primlist->first();
    
    int seen = 0;
    while (prim != nullptr)
    {
        if (PRIMFLAG_GET_SCREENTEX(prim->flags))
        {
			process_screen_quad(m_screen_chain->applicable_passes() * seen, prim);
            seen++;
        }
        prim = prim->next();
    }

    window().m_primlist->release_lock();

    uint32_t total_passes = seen * m_screen_chain->applicable_passes();
    bgfx::setViewFrameBuffer(total_passes, BGFX_INVALID_HANDLE);

    return total_passes;
}

int renderer_bgfx::draw(int update)
{
	int window_index = window().m_index;
    int post_view_index = handle_screen_chains();
    int view_index = window_index;
    int first_view_index = 0;
    if (m_screen_chain && post_view_index > 0)
    {
        view_index += post_view_index;
        first_view_index = post_view_index;
    }

    // Set view 0 default viewport.
	osd_dim wdim = window().get_size();
	m_width[window_index] = wdim.width();
	m_height[window_index] = wdim.height();
	if (window_index == 0)
	{
		if ((m_dimensions != osd_dim(m_width[window_index], m_height[window_index])))
		{
			bgfx::reset(m_width[window_index], m_height[window_index], video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			m_dimensions = osd_dim(m_width[window_index], m_height[window_index]);
		}
	}
	else
	{
		if ((m_dimensions != osd_dim(m_width[window_index], m_height[window_index])))
		{
			bgfx::reset(window().m_main->get_size().width(), window().m_main->get_size().height(), video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			delete m_framebuffer;
#ifdef OSD_WINDOWS
			m_framebuffer = m_targets->create_backbuffer(window().m_hwnd, m_width[window_index], m_height[window_index]);
#else
			m_framebuffer = m_targets->create_backbuffer(sdlNativeWindowHandle(window().sdl_window()), m_width[window_index], m_height[window_index]);
#endif
			bgfx::setViewFrameBuffer(view_index, m_framebuffer->target());
			m_dimensions = osd_dim(m_width[window_index], m_height[window_index]);
			bgfx::setViewClear(view_index
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x000000ff
				, 1.0f
				, 0
				);
			bgfx::touch(view_index);
			bgfx::frame();
			return 0;
		}
	}

	if (view_index != first_view_index)
	{
		bgfx::setViewFrameBuffer(view_index, m_framebuffer->target());
	}
	bgfx::setViewSeq(view_index, true);
	bgfx::setViewRect(view_index, 0, 0, m_width[window_index], m_height[window_index]);

	// Setup view transform.
	float proj[16];
	bx::mtxOrtho(proj, 0.0f, m_width[window_index], m_height[window_index], 0.0f, 0.0f, 100.0f);
	bgfx::setViewTransform(view_index, nullptr, proj);

	bgfx::setViewClear(view_index
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);

	window().m_primlist->acquire_lock();

	// Mark our texture atlas as dirty if we need to do so
	bool atlas_valid = update_atlas();

	render_primitive *prim = window().m_primlist->first();
    int32_t screen = 0;
	while (prim != nullptr)
	{
		UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);

		bgfx::TransientVertexBuffer buffer;
		allocate_buffer(prim, blend, &buffer);

		buffer_status status = buffer_primitives(view_index, atlas_valid, &prim, &buffer, screen);
        
        if (status == BUFFER_SCREEN)
        {
            screen++;
        }

		if (status != BUFFER_EMPTY && status != BUFFER_SCREEN)
		{
			bgfx::setVertexBuffer(&buffer);
			bgfx::setTexture(0, m_gui_effect[blend]->uniform("s_tex")->handle(), m_texture_cache->texture());
			m_gui_effect[blend]->submit(view_index);
		}

		if (status != BUFFER_DONE && status != BUFFER_PRE_FLUSH)
		{
			prim = prim->next();
		}
	}

	window().m_primlist->release_lock();

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(view_index);

	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	if (view_index == first_view_index)
    {
        bgfx::frame();
    }

	return 0;
}

renderer_bgfx::buffer_status renderer_bgfx::buffer_primitives(int view, bool atlas_valid, render_primitive** prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
	int vertices = 0;

	UINT32 blend = PRIMFLAG_GET_BLENDMODE((*prim)->flags);
	while (*prim != nullptr)
	{
		switch ((*prim)->type)
		{
			case render_primitive::LINE:
				put_line((*prim)->bounds.x0, (*prim)->bounds.y0, (*prim)->bounds.x1, (*prim)->bounds.y1, 1.0f, u32Color((*prim)->color.r * 255, (*prim)->color.g * 255, (*prim)->color.b * 255, (*prim)->color.a * 255), (ScreenVertex*)buffer->data + vertices, 1.0f);
				vertices += 30;
				break;

			case render_primitive::QUAD:
				if ((*prim)->texture.base == nullptr)
				{
					put_packed_quad(*prim, WHITE_HASH, (ScreenVertex*)buffer->data + vertices);
					vertices += 6;
				}
				else
				{
					const UINT32 hash = get_texture_hash(*prim);
					if (atlas_valid && (*prim)->packable(PACKABLE_SIZE) && hash != 0 && m_hash_to_entry[hash].hash())
					{
						put_packed_quad(*prim, hash, (ScreenVertex*)buffer->data + vertices);
						vertices += 6;
					}
					else
					{
						if (vertices > 0)
						{
							return BUFFER_PRE_FLUSH;
						}

                        if (PRIMFLAG_GET_SCREENTEX((*prim)->flags) && m_screen_chain != nullptr)
                        {
                            render_post_screen_quad(view, *prim, buffer, screen);
                            return BUFFER_SCREEN;
                        }
                        else
                        {
                            render_textured_quad(view, *prim, buffer);
                            return BUFFER_EMPTY;
                        }
					}
				}
				break;

			default:
				// Unhandled
				break;
		}

		if ((*prim)->next() != nullptr && PRIMFLAG_GET_BLENDMODE((*prim)->next()->flags) != blend)
		{
			break;
		}

		*prim = (*prim)->next();
	}

	if (*prim == nullptr)
	{
		return BUFFER_DONE;
	}
	if (vertices == 0)
	{
		return BUFFER_EMPTY;
	}
	return BUFFER_FLUSH;
}

uint64_t renderer_bgfx::get_blend_state(UINT32 blend)
{
	switch (blend)
	{
		case BLENDMODE_ALPHA:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		case BLENDMODE_RGB_MULTIPLY:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
		case BLENDMODE_ADD:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		default:
			return 0L;
	}
	return 0L;
}

void renderer_bgfx::set_bgfx_state(UINT32 blend)
{
	uint64_t flags = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_DEPTH_TEST_ALWAYS;
	bgfx::setState(flags | get_blend_state(blend));
}

bool renderer_bgfx::update_atlas()
{
	bool atlas_dirty = check_for_dirty_atlas();

	if (atlas_dirty)
	{
		m_hash_to_entry.clear();

		std::vector<std::vector<rectangle_packer::packed_rectangle>> packed;
		if (m_packer.pack(m_texinfo, packed, CACHE_SIZE))
		{
			process_atlas_packs(packed);
		}
		else
		{
			packed.clear();

			m_texinfo.clear();
			m_texinfo.push_back(rectangle_packer::packable_rectangle(WHITE_HASH, PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32), 16, 16, 16, nullptr, m_white));

			m_packer.pack(m_texinfo, packed, CACHE_SIZE);
			process_atlas_packs(packed);

			return false;
		}
	}
	return true;
}

void renderer_bgfx::process_atlas_packs(std::vector<std::vector<rectangle_packer::packed_rectangle>>& packed)
{
	for (std::vector<rectangle_packer::packed_rectangle> pack : packed)
	{
		for (rectangle_packer::packed_rectangle rect : pack)
		{
			if (rect.hash() == 0xffffffff)
			{
				continue;
			}
			m_hash_to_entry[rect.hash()] = rect;
			const bgfx::Memory* mem = mame_texture_data_to_bgfx_texture_data(rect.format(), rect.width(), rect.height(), rect.rowpixels(), rect.palette(), rect.base());
			bgfx::updateTexture2D(m_texture_cache->texture(), 0, rect.x(), rect.y(), rect.width(), rect.height(), mem);
		}
	}
}

UINT32 renderer_bgfx::get_texture_hash(render_primitive *prim)
{
#if GIBBERISH
	UINT32 xor_value = 0x87;
	UINT32 hash = 0xdabeefed;

	int bpp = 2;
	UINT32 format = PRIMFLAG_GET_TEXFORMAT(prim->flags);
	if (format == TEXFORMAT_ARGB32 || format == TEXFORMAT_RGB32)
	{
		bpp = 4;
	}

	for (int y = 0; y < prim->texture.height; y++)
	{
		UINT8 *base = reinterpret_cast<UINT8*>(prim->texture.base) + prim->texture.rowpixels * y;
		for (int x = 0; x < prim->texture.width * bpp; x++)
		{
			hash += base[x] ^ xor_value;
		}
	}
	return hash;
#else
	return (reinterpret_cast<size_t>(prim->texture.base)) & 0xffffffff;
#endif
}

bool renderer_bgfx::check_for_dirty_atlas()
{
	bool atlas_dirty = false;

	std::map<UINT32, rectangle_packer::packable_rectangle> acquired_infos;
	for (render_primitive *prim = window().m_primlist->first(); prim != nullptr; prim = prim->next())
	{
		bool pack = prim->packable(PACKABLE_SIZE);
		if (prim->type == render_primitive::QUAD && prim->texture.base != nullptr && pack)
		{
			const UINT32 hash = get_texture_hash(prim);
			// If this texture is packable and not currently in the atlas, prepare the texture for putting in the atlas
			if ((hash != 0 && m_hash_to_entry[hash].hash() == 0 && acquired_infos[hash].hash() == 0)
				|| (hash != 0 && m_hash_to_entry[hash].hash() != hash && acquired_infos[hash].hash() == 0))
			{   // Create create the texture and mark the atlas dirty
				atlas_dirty = true;

				m_texinfo.push_back(rectangle_packer::packable_rectangle(hash, prim->flags & PRIMFLAG_TEXFORMAT_MASK,
					prim->texture.width, prim->texture.height,
					prim->texture.rowpixels, prim->texture.palette, prim->texture.base));
				acquired_infos[hash] = m_texinfo[m_texinfo.size() - 1];
			}
		}
	}

	if (m_texinfo.size() == 1)
	{
		atlas_dirty = true;
	}

	return atlas_dirty;
}

void renderer_bgfx::allocate_buffer(render_primitive *prim, UINT32 blend, bgfx::TransientVertexBuffer *buffer)
{
	int vertices = 0;
	bool mode_switched = false;
	while (prim != nullptr && !mode_switched)
	{
		switch (prim->type)
		{
			case render_primitive::LINE:
				vertices += 30;
				break;

			case render_primitive::QUAD:
				if (!prim->packable(PACKABLE_SIZE))
				{
					if (prim->texture.base == nullptr)
					{
						vertices += 6;
					}
					else
					{
						if (vertices == 0)
						{
							vertices += 6;
						}
						mode_switched = true;
					}
				}
				else
				{
					vertices += 6;
				}
				break;
			default:
				// Do nothing
				break;
		}

		prim = prim->next();

		if (prim != nullptr && PRIMFLAG_GET_BLENDMODE(prim->flags) != blend)
		{
			mode_switched = true;
		}
	}

	if (vertices > 0 && bgfx::checkAvailTransientVertexBuffer(vertices, ScreenVertex::ms_decl))
	{
		bgfx::allocTransientVertexBuffer(buffer, vertices, ScreenVertex::ms_decl);
	}
}

slider_state* renderer_bgfx::get_slider_list()
{
	if (!m_screen_chain)
	{
		return nullptr;
	}

    slider_state *listhead = nullptr;
    slider_state **tailptr = &listhead;
    std::vector<bgfx_slider*> sliders = m_screen_chain->sliders();
    for (bgfx_slider* slider : sliders)
    {
        if (*tailptr == nullptr)
        {
            *tailptr = slider->core_slider();
        }
        else
        {
            (*tailptr)->next = slider->core_slider();
            tailptr = &(*tailptr)->next;
        }
    }
    if (*tailptr != nullptr)
    {
    	(*tailptr)->next = nullptr;
	}
    m_sliders_dirty = false;
    return listhead;
}
