// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Ryan Holtz,Dario Manesku,Branimir Karadzic,Aaron Giles
//============================================================
//
//  drawbgfx.cpp - BGFX renderer
//
//============================================================
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
#include "bgfxutil.h"
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
#include "bgfx/target.h"

#include "imgui/imgui.h"

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

#define GIBBERISH       (0)
#define SCENE_VIEW      (0)

//============================================================
//  STATICS
//============================================================

bool renderer_bgfx::s_window_set = false;
uint32_t renderer_bgfx::s_current_view = 0;

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

#   if BX_PLATFORM_LINUX || BX_PLATFORM_BSD || BX_PLATFORM_RPI
	return (void*)wmi.info.x11.window;
#   elif BX_PLATFORM_OSX
	return wmi.info.cocoa.window;
#   elif BX_PLATFORM_WINDOWS
	return wmi.info.win.window;
#   elif BX_PLATFORM_STEAMLINK
	return wmi.info.vivante.window;
#   elif BX_PLATFORM_EMSCRIPTEN || BX_PLATFORM_ANDROID
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
			printf("Unknown backend type '%s', going with auto-detection\n", backend.c_str());
			bgfx::init();
		}
		bgfx::reset(m_width[window().m_index], m_height[window().m_index], video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
		// Enable debug text.
		bgfx::setDebug(options.bgfx_debug() ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
		m_dimensions = osd_dim(m_width[0], m_height[0]);
	}

	m_textures = new texture_manager();
	m_targets = new target_manager(*m_textures);

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

	m_chains = new chain_manager(window().machine(), options, *m_textures, *m_targets, *m_effects, window().m_index);
	m_sliders_dirty = true;

	uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
	m_texture_cache = m_textures->create_texture("#cache", bgfx::TextureFormat::RGBA8, CACHE_SIZE, CACHE_SIZE, nullptr, flags);

	memset(m_white, 0xff, sizeof(uint32_t) * 16 * 16);
	m_texinfo.push_back(rectangle_packer::packable_rectangle(WHITE_HASH, PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32), 16, 16, 16, nullptr, m_white));

	imguiCreate();

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
	imguiDestroy();

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

	const float x0 = prim->bounds.x0;
	const float x1 = prim->bounds.x1;
	const float y0 = prim->bounds.y0;
	const float y1 = prim->bounds.y1;

	float x[4] = { x0, x1, x0, x1 };
	float y[4] = { y0, y0, y1, y1 };
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

void renderer_bgfx::render_post_screen_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
	ScreenVertex* vertex = reinterpret_cast<ScreenVertex*>(buffer->data);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { prim->texcoords.tl.u, prim->texcoords.tr.u, prim->texcoords.bl.u, prim->texcoords.br.u };
	float v[4] = { prim->texcoords.tl.v, prim->texcoords.tr.v, prim->texcoords.bl.v, prim->texcoords.br.v };

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

	uint32_t texture_flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
	if (video_config.filter == 0)
	{
		texture_flags |= BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
	}

	UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
	bgfx::setVertexBuffer(buffer);
	bgfx::setTexture(0, m_screen_effect[blend]->uniform("s_tex")->handle(), m_targets->target(screen, "output")->texture(), texture_flags);
	m_screen_effect[blend]->submit(view);
}

void renderer_bgfx::render_textured_quad(render_primitive* prim, bgfx::TransientVertexBuffer* buffer)
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

	const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(prim->flags & PRIMFLAG_TEXFORMAT_MASK,
		tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);

	bgfx::TextureHandle texture = bgfx::createTexture2D(tex_width, tex_height, 1, bgfx::TextureFormat::RGBA8, texture_flags, mem);

	bgfx_effect** effects = PRIMFLAG_GET_SCREENTEX(prim->flags) ? m_screen_effect : m_gui_effect;

	UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
	bgfx::setVertexBuffer(buffer);
	bgfx::setTexture(0, effects[blend]->uniform("s_tex")->handle(), texture);
	effects[blend]->submit(m_ui_view);

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

int renderer_bgfx::draw(int update)
{
	int window_index = window().m_index;
	if (window_index == 0)
	{
		s_current_view = 0;
	}

	m_seen_views.clear();
	m_ui_view = -1;

	// Set view 0 default viewport.
	osd_dim wdim = window().get_size();
	m_width[window_index] = wdim.width();
	m_height[window_index] = wdim.height();

    window().m_primlist->acquire_lock();
    s_current_view += m_chains->handle_screen_chains(s_current_view, window().m_primlist->first(), window());
    window().m_primlist->release_lock();

	bool skip_frame = update_dimensions();
	if (skip_frame)
	{
		return 0;
	}

	if (s_current_view > m_max_view)
	{
		m_max_view = s_current_view;
	}
	else
	{
		s_current_view = m_max_view;
	}

	window().m_primlist->acquire_lock();

	// Mark our texture atlas as dirty if we need to do so
	bool atlas_valid = update_atlas();

	render_primitive *prim = window().m_primlist->first();
	std::vector<void*> sources;
	while (prim != nullptr)
	{
		UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);

		bgfx::TransientVertexBuffer buffer;
		allocate_buffer(prim, blend, &buffer);

		int32_t screen = -1;
		if (PRIMFLAG_GET_SCREENTEX(prim->flags))
		{
			for (screen = 0; screen < sources.size(); screen++)
			{
				if (sources[screen] == prim->texture.base)
				{
					break;
				}
			}
			if (screen == sources.size())
			{
				sources.push_back(prim->texture.base);
			}
		}

		buffer_status status = buffer_primitives(atlas_valid, &prim, &buffer, screen);

		if (status != BUFFER_EMPTY && status != BUFFER_SCREEN)
		{
			bgfx::setVertexBuffer(&buffer);
			bgfx::setTexture(0, m_gui_effect[blend]->uniform("s_tex")->handle(), m_texture_cache->texture());
			m_gui_effect[blend]->submit(m_ui_view);
		}

		if (status != BUFFER_DONE && status != BUFFER_PRE_FLUSH)
		{
			prim = prim->next();
		}
	}

	window().m_primlist->release_lock();

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(s_current_view > 0 ? s_current_view - 1 : 0);

	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	if (window_index == 0)
	{
		bgfx::frame();
	}

	return 0;
}

bool renderer_bgfx::update_dimensions()
{
	const uint32_t window_index = window().m_index;
	const uint32_t width = m_width[window_index];
	const uint32_t height = m_height[window_index];

	if (window_index == 0)
	{
		if ((m_dimensions != osd_dim(width, height)))
		{
			bgfx::reset(width, height, video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			m_dimensions = osd_dim(width, height);
		}
	}
	else
	{
		if ((m_dimensions != osd_dim(width, height)))
		{
			bgfx::reset(window().m_main->get_size().width(), window().m_main->get_size().height(), video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			m_dimensions = osd_dim(width, height);

			delete m_framebuffer;
#ifdef OSD_WINDOWS
			m_framebuffer = m_targets->create_backbuffer(window().m_hwnd, width, height);
#else
			m_framebuffer = m_targets->create_backbuffer(sdlNativeWindowHandle(window().sdl_window()), width, height);
#endif

			bgfx::setViewFrameBuffer(s_current_view, m_framebuffer->target());
			bgfx::setViewClear(s_current_view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
			bgfx::touch(s_current_view);
			bgfx::frame();
			return true;
		}
	}
	return false;
}

void renderer_bgfx::setup_view(uint32_t view_index, bool screen)
{
	const uint32_t window_index = window().m_index;
	const uint32_t width = m_width[window_index];
	const uint32_t height = m_height[window_index];

	if (window_index != 0)
	{
		bgfx::setViewFrameBuffer(view_index, m_framebuffer->target());
	}

	bgfx::setViewSeq(view_index, true);
	bgfx::setViewRect(view_index, 0, 0, width, height);

#if SCENE_VIEW
	if (view_index == m_max_view)
	{
#else
	while ((view_index + 1) > m_seen_views.size())
	{
		m_seen_views.push_back(false);
	}

	if (!m_seen_views[view_index])
	{
		m_seen_views[view_index] = true;
#endif
		bgfx::setViewClear(view_index, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
	}

	setup_matrices(view_index, screen);
}

void renderer_bgfx::setup_matrices(uint32_t view_index, bool screen)
{
	const uint32_t window_index = window().m_index;
	const uint32_t width = m_width[window_index];
	const uint32_t height = m_height[window_index];

	float proj[16];
	float view[16];
	if (screen)
	{
		static float offset = 0.0f;
		offset += 0.5f;
		float up[3]  = { 0.0f, -1.0f, 0.0f };
		float cam_z = width * 0.5f * (float(height) / float(width));
		cam_z *= 1.05f;
		float eye_height = height * 0.5f * 1.05f;
		float at[3]  = { width * 0.5f, height * 0.5f, 0.0f };
		float eye[3] = { width * 0.5f, eye_height, cam_z };
		bx::mtxLookAt(view, eye, at, up);

		bx::mtxProj(proj, 90.0f, float(width) / float(height), 0.1f, 5000.0f);
	}
	else
	{
		bx::mtxIdentity(view);
		bx::mtxOrtho(proj, 0.0f, width, height, 0.0f, 0.0f, 100.0f);
	}

	bgfx::setViewTransform(view_index, view, proj);
}

void renderer_bgfx::init_ui_view()
{
	if (m_ui_view < 0)
	{
		m_ui_view = s_current_view;
		setup_view(m_ui_view, false);
		s_current_view++;
	}
}

renderer_bgfx::buffer_status renderer_bgfx::buffer_primitives(bool atlas_valid, render_primitive** prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
	int vertices = 0;

	UINT32 blend = PRIMFLAG_GET_BLENDMODE((*prim)->flags);
	while (*prim != nullptr)
	{
		switch ((*prim)->type)
		{
			case render_primitive::LINE:
				init_ui_view();
				put_line((*prim)->bounds.x0, (*prim)->bounds.y0, (*prim)->bounds.x1, (*prim)->bounds.y1, 1.0f, u32Color((*prim)->color.r * 255, (*prim)->color.g * 255, (*prim)->color.b * 255, (*prim)->color.a * 255), (ScreenVertex*)buffer->data + vertices, 1.0f);
				vertices += 30;
				break;

			case render_primitive::QUAD:
				if ((*prim)->texture.base == nullptr)
				{
					init_ui_view();
					put_packed_quad(*prim, WHITE_HASH, (ScreenVertex*)buffer->data + vertices);
					vertices += 6;
				}
				else
				{
					const UINT32 hash = get_texture_hash(*prim);
					if (atlas_valid && (*prim)->packable(PACKABLE_SIZE) && hash != 0 && m_hash_to_entry[hash].hash())
					{
						init_ui_view();
						put_packed_quad(*prim, hash, (ScreenVertex*)buffer->data + vertices);
						vertices += 6;
					}
					else
					{
						if (vertices > 0)
						{
							return BUFFER_PRE_FLUSH;
						}

						if (PRIMFLAG_GET_SCREENTEX((*prim)->flags) && m_chains->has_applicable_pass(screen))
						{
#if SCENE_VIEW
							setup_view(s_current_view, true);
							render_post_screen_quad(s_current_view, *prim, buffer, screen);
							s_current_view++;
							m_ui_view = -1;
#else
							init_ui_view();
							render_post_screen_quad(m_ui_view, *prim, buffer, screen);
#endif
							return BUFFER_SCREEN;
						}
						else
						{
							init_ui_view();
							render_textured_quad(*prim, buffer);
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

void renderer_bgfx::set_bgfx_state(UINT32 blend)
{
	uint64_t flags = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_DEPTH_TEST_ALWAYS;
	bgfx::setState(flags | bgfx_util::get_blend_state(blend));
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
			const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(rect.format(), rect.width(), rect.height(), rect.rowpixels(), rect.palette(), rect.base());
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
	for (render_primitive &prim : *window().m_primlist)
	{
		bool pack = prim.packable(PACKABLE_SIZE);
		if (prim.type == render_primitive::QUAD && prim.texture.base != nullptr && pack)
		{
			const UINT32 hash = get_texture_hash(&prim);
			// If this texture is packable and not currently in the atlas, prepare the texture for putting in the atlas
			if ((hash != 0 && m_hash_to_entry[hash].hash() == 0 && acquired_infos[hash].hash() == 0)
				|| (hash != 0 && m_hash_to_entry[hash].hash() != hash && acquired_infos[hash].hash() == 0))
			{   // Create create the texture and mark the atlas dirty
				atlas_dirty = true;

				m_texinfo.push_back(rectangle_packer::packable_rectangle(hash, prim.flags & PRIMFLAG_TEXFORMAT_MASK,
					prim.texture.width, prim.texture.height,
					prim.texture.rowpixels, prim.texture.palette, prim.texture.base));
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
    m_sliders_dirty = false;
    return m_chains->get_slider_list();
}
