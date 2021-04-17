// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Ryan Holtz,Dario Manesku,Branimir Karadzic,Aaron Giles
//============================================================
//
//  drawbgfx.cpp - BGFX renderer
//
//============================================================
#include <bx/math.h>
#include <bx/readerwriter.h>

#if defined(SDLMAME_WIN32) || defined(OSD_WINDOWS) || defined(OSD_UWP)
// standard windows headers
#include <windows.h>
#if defined(SDLMAME_WIN32)
#include <SDL2/SDL_syswm.h>
#endif
#else
#if defined(OSD_MAC)
extern void *GetOSWindow(void *wincontroller);
#else
#include <SDL2/SDL_syswm.h>
#endif
#endif

// MAMEOS headers
#include "emu.h"
#include "window.h"
#include "rendutil.h"
#include "aviwrite.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
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
#include "bgfx/view.h"

#include "imgui/imgui.h"

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

uint16_t const renderer_bgfx::CACHE_SIZE = 1024;
uint32_t const renderer_bgfx::PACKABLE_SIZE = 128;
uint32_t const renderer_bgfx::WHITE_HASH = 0x87654321;
char const *const renderer_bgfx::WINDOW_PREFIX = "Window 0, ";

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
//  renderer_bgfx - constructor
//============================================================

renderer_bgfx::renderer_bgfx(std::shared_ptr<osd_window> w)
	: osd_renderer(w, FLAG_NONE)
	, m_options(downcast<osd_options &>(w->machine().options()))
	, m_framebuffer(nullptr)
	, m_texture_cache(nullptr)
	, m_dimensions(0, 0)
	, m_textures(nullptr)
	, m_targets(nullptr)
	, m_shaders(nullptr)
	, m_effects(nullptr)
	, m_chains(nullptr)
	, m_ortho_view(nullptr)
	, m_max_view(0)
	, m_avi_view(nullptr)
	, m_avi_writer(nullptr)
	, m_avi_target(nullptr)
{
}

//============================================================
//  renderer_bgfx - destructor
//============================================================

renderer_bgfx::~renderer_bgfx()
{
	bgfx::reset(0, 0, BGFX_RESET_NONE);
	//bgfx::touch(0);
	//bgfx::frame();
	if (m_avi_writer != nullptr && m_avi_writer->recording())
	{
		m_avi_writer->stop();

		m_targets->destroy_target("avibuffer0");
		m_avi_target = nullptr;

		bgfx::destroy(m_avi_texture);

		delete m_avi_writer;
		delete [] m_avi_data;
		delete m_avi_view;
	}

	// Cleanup.
	delete m_chains;
	delete m_effects;
	delete m_shaders;
	delete m_textures;
	delete m_targets;
}

//============================================================
//  renderer_bgfx::create
//============================================================

#ifdef OSD_WINDOWS
inline void winSetHwnd(::HWND _window)
{
	bgfx::PlatformData pd;
	pd.ndt          = NULL;
	pd.nwh          = _window;
	pd.context      = NULL;
	pd.backBuffer   = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
}
#elif defined(OSD_MAC)
inline void macSetWindow(void *_window)
{
	bgfx::PlatformData pd;
	pd.ndt          = NULL;
	pd.nwh          = GetOSWindow(_window);
	pd.context      = NULL;
	pd.backBuffer   = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
}
#elif defined(OSD_SDL)
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

inline bool sdlSetWindow(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi) )
	{
		return false;
	}

	bgfx::PlatformData pd;
#   if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt          = wmi.info.x11.display;
	pd.nwh          = (void*)(uintptr_t)wmi.info.x11.window;
#   elif BX_PLATFORM_OSX
	pd.ndt          = NULL;
	pd.nwh          = wmi.info.cocoa.window;
#   elif BX_PLATFORM_WINDOWS
	pd.ndt          = NULL;
	pd.nwh          = wmi.info.win.window;
#   elif BX_PLATFORM_STEAMLINK
	pd.ndt          = wmi.info.vivante.display;
	pd.nwh          = wmi.info.vivante.window;
#   endif // BX_PLATFORM_
	pd.context      = NULL;
	pd.backBuffer   = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);

	return true;
}
#elif defined(OSD_UWP)
inline void winrtSetWindow(::IUnknown* _window)
{
	bgfx::PlatformData pd;
	pd.ndt = NULL;
	pd.nwh = _window;
	pd.context = NULL;
	pd.backBuffer = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
}

IInspectable* AsInspectable(Platform::Agile<Windows::UI::Core::CoreWindow> win)
{
	return reinterpret_cast<IInspectable*>(win.Get());
}
#endif

int renderer_bgfx::create()
{
	// create renderer
	std::shared_ptr<osd_window> win = assert_window();
	osd_dim wdim = win->get_size();
	m_width[win->index()] = wdim.width();
	m_height[win->index()] = wdim.height();
	if (win->index() == 0)
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
		winSetHwnd(std::static_pointer_cast<win_window_info>(win)->platform_window());
#elif defined(OSD_UWP)

		winrtSetWindow(AsInspectable(std::static_pointer_cast<uwp_window_info>(win)->platform_window()));
#elif defined(OSD_MAC)
		macSetWindow(std::static_pointer_cast<mac_window_info>(win)->platform_window());
#else
		sdlSetWindow(std::dynamic_pointer_cast<sdl_window_info>(win)->platform_window());
#endif
		std::string backend(m_options.bgfx_backend());
		bgfx::Init init;
		init.type = bgfx::RendererType::Count;
		init.vendorId = BGFX_PCI_ID_NONE;
		init.resolution.width = wdim.width();
		init.resolution.height = wdim.height();
		init.resolution.reset = BGFX_RESET_NONE;
		if (backend == "auto")
		{
		}
		else if (backend == "dx9" || backend == "d3d9")
		{
			init.type = bgfx::RendererType::Direct3D9;
		}
		else if (backend == "dx11" || backend == "d3d11")
		{
			init.type = bgfx::RendererType::Direct3D11;
		}
		else if (backend == "dx12" || backend == "d3d12")
		{
			init.type = bgfx::RendererType::Direct3D12;
		}
		else if (backend == "gles")
		{
			init.type = bgfx::RendererType::OpenGLES;
		}
		else if (backend == "glsl" || backend == "opengl")
		{
			init.type = bgfx::RendererType::OpenGL;
		}
		else if (backend == "vulkan")
		{
			init.type = bgfx::RendererType::Vulkan;
		}
		else if (backend == "metal")
		{
			init.type = bgfx::RendererType::Metal;
		}
		else
		{
			printf("Unknown backend type '%s', going with auto-detection\n", backend.c_str());
		}
		bgfx::init(init);
		bgfx::reset(m_width[win->index()], m_height[win->index()], video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
		// Enable debug text.
		bgfx::setDebug(m_options.bgfx_debug() ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
		m_dimensions = osd_dim(m_width[0], m_height[0]);
	}

	m_textures = new texture_manager();
	m_targets = new target_manager(*m_textures);

	m_shaders = new shader_manager(m_options);
	m_effects = new effect_manager(m_options, *m_shaders);

	if (win->index() != 0)
	{
#ifdef OSD_WINDOWS
		m_framebuffer = m_targets->create_backbuffer(std::static_pointer_cast<win_window_info>(win)->platform_window(), m_width[win->index()], m_height[win->index()]);
#elif defined(OSD_UWP)
		m_framebuffer = m_targets->create_backbuffer(AsInspectable(std::static_pointer_cast<uwp_window_info>(win)->platform_window()), m_width[win->index()], m_height[win->index()]);
#elif defined(OSD_MAC)
		m_framebuffer = m_targets->create_backbuffer(GetOSWindow(std::static_pointer_cast<mac_window_info>(win)->platform_window()), m_width[win->index()], m_height[win->index()]);
#else
		m_framebuffer = m_targets->create_backbuffer(sdlNativeWindowHandle(std::dynamic_pointer_cast<sdl_window_info>(win)->platform_window()), m_width[win->index()], m_height[win->index()]);
#endif
		bgfx::touch(win->index());

		if (m_ortho_view) {
			m_ortho_view->set_backbuffer(m_framebuffer);
		}
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

	if (   m_gui_effect[0] == nullptr ||    m_gui_effect[1] == nullptr ||    m_gui_effect[2] == nullptr ||    m_gui_effect[3] == nullptr ||
		m_screen_effect[0] == nullptr || m_screen_effect[1] == nullptr || m_screen_effect[2] == nullptr || m_screen_effect[3] == nullptr)
	{
		fatalerror("BGFX: Unable to load required shaders. Please check and reinstall the %s folder\n", m_options.bgfx_path());
	}

	m_chains = new chain_manager(win->machine(), m_options, *m_textures, *m_targets, *m_effects, win->index(), *this);
	m_sliders_dirty = true;

	uint32_t flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
	m_texture_cache = m_textures->create_texture("#cache", bgfx::TextureFormat::RGBA8, CACHE_SIZE, CACHE_SIZE, nullptr, flags);

	memset(m_white, 0xff, sizeof(uint32_t) * 16 * 16);
	m_texinfo.push_back(rectangle_packer::packable_rectangle(WHITE_HASH, PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32), 16, 16, 16, nullptr, m_white));

	imguiCreate();

	return 0;
}

//============================================================
//  renderer_bgfx::record
//============================================================

void renderer_bgfx::record()
{
	std::shared_ptr<osd_window> win = assert_window();

	if (win->index() > 0)
	{
		return;
	}

	if (m_avi_writer == nullptr)
	{
		m_avi_writer = new avi_write(win->machine(), m_width[0], m_height[0]);
		m_avi_data = new uint8_t[m_width[0] * m_height[0] * 4];
		m_avi_bitmap.allocate(m_width[0], m_height[0]);
	}

	if (m_avi_writer->recording())
	{
		m_avi_writer->stop();
		m_targets->destroy_target("avibuffer0");
		m_avi_target = nullptr;
		bgfx::destroy(m_avi_texture);
		delete m_avi_view;
		m_avi_view = nullptr;
	}
	else
	{
		m_avi_writer->record(m_options.bgfx_avi_name());
		m_avi_target = m_targets->create_target("avibuffer", bgfx::TextureFormat::RGBA8, m_width[0], m_height[0], TARGET_STYLE_CUSTOM, false, true, 1, 0);
		m_avi_texture = bgfx::createTexture2D(m_width[0], m_height[0], false, 1, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK);

		if (m_avi_view == nullptr)
		{
			m_avi_view = new bgfx_ortho_view(this, 10, m_avi_target, m_seen_views);
		}
	}
}

bool renderer_bgfx::init(running_machine &machine)
{
	const char *bgfx_path = downcast<osd_options &>(machine.options()).bgfx_path();

	osd::directory::ptr directory = osd::directory::open(bgfx_path);
	if (directory == nullptr)
	{
		osd_printf_verbose("Unable to find the %s folder. Please reinstall it to use the BGFX renderer\n", bgfx_path);
		return true;
	}

	return false;
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

bgfx::VertexLayout ScreenVertex::ms_decl;

void renderer_bgfx::put_packed_quad(render_primitive *prim, uint32_t hash, ScreenVertex* vertices)
{
	rectangle_packer::packed_rectangle& rect = m_hash_to_entry[hash];
	auto size = float(CACHE_SIZE);
	float u0 = (float(rect.x()) + 0.5f) / size;
	float v0 = (float(rect.y()) + 0.5f) / size;
	float u1 = u0 + (float(rect.width()) - 1.0f) / size;
	float v1 = v0 + (float(rect.height()) - 1.0f) / size;
	uint32_t rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { u0, u1, u0, u1 };
	float v[4] = { v0, v0, v1, v1 };

	if (bgfx::getRendererType() == bgfx::RendererType::Direct3D9)
	{
		for (int i = 0; i < 4; i++)
		{
			u[i] += 0.5f / size;
			v[i] += 0.5f / size;
		}
	}

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

	vertex(&vertices[0], x[0], y[0], 0, rgba, u[0], v[0]);
	vertex(&vertices[1], x[1], y[1], 0, rgba, u[1], v[1]);
	vertex(&vertices[2], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[3], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[4], x[2], y[2], 0, rgba, u[2], v[2]);
	vertex(&vertices[5], x[0], y[0], 0, rgba, u[0], v[0]);
}

void renderer_bgfx::vertex(ScreenVertex* vertex, float x, float y, float z, uint32_t rgba, float u, float v)
{
	vertex->m_x = x;
	vertex->m_y = y;
	vertex->m_z = z;
	vertex->m_rgba = rgba;
	vertex->m_u = u;
	vertex->m_v = v;
}

void renderer_bgfx::render_post_screen_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
	auto* vertices = reinterpret_cast<ScreenVertex*>(buffer->data);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { prim->texcoords.tl.u, prim->texcoords.tr.u, prim->texcoords.bl.u, prim->texcoords.br.u };
	float v[4] = { prim->texcoords.tl.v, prim->texcoords.tr.v, prim->texcoords.bl.v, prim->texcoords.br.v };

	vertex(&vertices[0], x[0], y[0], 0, 0xffffffff, u[0], v[0]);
	vertex(&vertices[1], x[1], y[1], 0, 0xffffffff, u[1], v[1]);
	vertex(&vertices[2], x[3], y[3], 0, 0xffffffff, u[3], v[3]);
	vertex(&vertices[3], x[3], y[3], 0, 0xffffffff, u[3], v[3]);
	vertex(&vertices[4], x[2], y[2], 0, 0xffffffff, u[2], v[2]);
	vertex(&vertices[5], x[0], y[0], 0, 0xffffffff, u[0], v[0]);

	uint32_t texture_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	if (video_config.filter == 0)
	{
		texture_flags |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
	}

	uint32_t blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
	bgfx::setVertexBuffer(0,buffer);
	bgfx::setTexture(0, m_screen_effect[blend]->uniform("s_tex")->handle(), m_targets->target(screen, "output")->texture(), texture_flags);
	m_screen_effect[blend]->submit(m_ortho_view->get_index());
}

void renderer_bgfx::render_avi_quad()
{
	m_avi_view->set_index(s_current_view);
	m_avi_view->setup();

	bgfx::setViewRect(s_current_view, 0, 0, m_width[0], m_height[0]);
	bgfx::setViewClear(s_current_view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);

	bgfx::TransientVertexBuffer buffer;
	bgfx::allocTransientVertexBuffer(&buffer, 6, ScreenVertex::ms_decl);
	auto* vertices = reinterpret_cast<ScreenVertex*>(buffer.data);

	float x[4] = { 0.0f, float(m_width[0]), 0.0f, float(m_width[0]) };
	float y[4] = { 0.0f, 0.0f, float(m_height[0]), float(m_height[0]) };
	float u[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float v[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	uint32_t rgba = 0xffffffff;

	vertex(&vertices[0], x[0], y[0], 0, rgba, u[0], v[0]);
	vertex(&vertices[1], x[1], y[1], 0, rgba, u[1], v[1]);
	vertex(&vertices[2], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[3], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[4], x[2], y[2], 0, rgba, u[2], v[2]);
	vertex(&vertices[5], x[0], y[0], 0, rgba, u[0], v[0]);

	bgfx::setVertexBuffer(0,&buffer);
	bgfx::setTexture(0, m_gui_effect[PRIMFLAG_GET_BLENDMODE(BLENDMODE_NONE)]->uniform("s_tex")->handle(), m_avi_target->texture());
	m_gui_effect[PRIMFLAG_GET_BLENDMODE(BLENDMODE_NONE)]->submit(s_current_view);
	s_current_view++;
}

void renderer_bgfx::render_textured_quad(render_primitive* prim, bgfx::TransientVertexBuffer* buffer)
{
	auto* vertices = reinterpret_cast<ScreenVertex*>(buffer->data);
	uint32_t rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

	float x[4] = { prim->bounds.x0, prim->bounds.x1, prim->bounds.x0, prim->bounds.x1 };
	float y[4] = { prim->bounds.y0, prim->bounds.y0, prim->bounds.y1, prim->bounds.y1 };
	float u[4] = { prim->texcoords.tl.u, prim->texcoords.tr.u, prim->texcoords.bl.u, prim->texcoords.br.u };
	float v[4] = { prim->texcoords.tl.v, prim->texcoords.tr.v, prim->texcoords.bl.v, prim->texcoords.br.v };

	vertex(&vertices[0], x[0], y[0], 0, rgba, u[0], v[0]);
	vertex(&vertices[1], x[1], y[1], 0, rgba, u[1], v[1]);
	vertex(&vertices[2], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[3], x[3], y[3], 0, rgba, u[3], v[3]);
	vertex(&vertices[4], x[2], y[2], 0, rgba, u[2], v[2]);
	vertex(&vertices[5], x[0], y[0], 0, rgba, u[0], v[0]);

	uint32_t texture_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	if (video_config.filter == 0)
	{
		texture_flags |= BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;
	}

	const bool is_screen = PRIMFLAG_GET_SCREENTEX(prim->flags);
	uint16_t tex_width(prim->texture.width);
	uint16_t tex_height(prim->texture.height);

	bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
	if (is_screen)
	{
		const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_argb32(prim->flags & PRIMFLAG_TEXFORMAT_MASK
			, tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);
		texture = bgfx::createTexture2D(tex_width, tex_height, false, 1, bgfx::TextureFormat::RGBA8, texture_flags, mem);
	}
	else
	{
		texture = m_textures->create_or_update_mame_texture(prim->flags & PRIMFLAG_TEXFORMAT_MASK
			, tex_width, tex_height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base, prim->texture.seqid
			, texture_flags, prim->texture.unique_id, prim->texture.old_id);
	}

	bgfx_effect** effects = is_screen ? m_screen_effect : m_gui_effect;

	uint32_t blend = PRIMFLAG_GET_BLENDMODE(prim->flags);
	bgfx::setVertexBuffer(0,buffer);
	bgfx::setTexture(0, effects[blend]->uniform("s_tex")->handle(), texture);
	effects[blend]->submit(m_ortho_view->get_index());

	if (is_screen)
	{
		bgfx::destroy(texture);
	}
}

#define MAX_TEMP_COORDS 100

void renderer_bgfx::put_polygon(const float* coords, uint32_t num_coords, float r, uint32_t rgba, ScreenVertex* vertex)
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
	uint32_t trans = rgba & 0x00ffffff;
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

void renderer_bgfx::put_packed_line(render_primitive *prim, ScreenVertex* vertex)
{
	float width = prim->width < 0.5f ? 0.5f : prim->width;
	float x0 = prim->bounds.x0;
	float y0 = prim->bounds.y0;
	float x1 = prim->bounds.x1;
	float y1 = prim->bounds.y1;
	uint32_t rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

	put_line(x0, y0, x1, y1, width, rgba, vertex, 1.0f);
}

void renderer_bgfx::put_line(float x0, float y0, float x1, float y1, float r, uint32_t rgba, ScreenVertex* vertex, float fth)
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

	// create diamond shape for points
	else
	{
		// set distance to unit vector length (1,1)
		dx = dy = 0.70710678f;
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
	std::shared_ptr<osd_window> win = assert_window();

	int window_index = win->index();

	m_seen_views.clear();
	if (m_ortho_view) {
		m_ortho_view->set_index(UINT_MAX);
	}

	osd_dim wdim = win->get_size();
	m_width[window_index] = wdim.width();
	m_height[window_index] = wdim.height();

	// Set view 0 default viewport.
	if (window_index == 0)
	{
		s_current_view = 0;
	}

	win->m_primlist->acquire_lock();
	uint32_t num_screens = m_chains->update_screen_textures(s_current_view, win->m_primlist->first(), *win.get());
	win->m_primlist->release_lock();

	if (num_screens)
	{
		s_current_view += m_chains->process_screen_chains(s_current_view, *win.get());
	}

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

	win->m_primlist->acquire_lock();

	// Mark our texture atlas as dirty if we need to do so
	bool atlas_valid = update_atlas();

	render_primitive *prim = win->m_primlist->first();
	std::vector<void*> sources;
	while (prim != nullptr)
	{
		uint32_t blend = PRIMFLAG_GET_BLENDMODE(prim->flags);

		bgfx::TransientVertexBuffer buffer;
		allocate_buffer(prim, blend, &buffer);

		int32_t screen = -1;
		if (PRIMFLAG_GET_SCREENTEX(prim->flags))
		{
			for (screen = 0; screen < sources.size(); screen++)
			{
				if (sources[screen] == prim)
				{
					break;
				}
			}
			if (screen == sources.size())
			{
				sources.push_back(prim);
			}
		}

		buffer_status status = buffer_primitives(atlas_valid, &prim, &buffer, screen);

		if (status != BUFFER_EMPTY && status != BUFFER_SCREEN)
		{
			bgfx::setVertexBuffer(0,&buffer);
			bgfx::setTexture(0, m_gui_effect[blend]->uniform("s_tex")->handle(), m_texture_cache->texture());
			m_gui_effect[blend]->submit(m_ortho_view->get_index());
		}

		if (status != BUFFER_DONE && status != BUFFER_PRE_FLUSH)
		{
			prim = prim->next();
		}
	}

	win->m_primlist->release_lock();

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(s_current_view > 0 ? s_current_view - 1 : 0);

	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	if (window_index == 0)
	{
		if (m_avi_writer != nullptr && m_avi_writer->recording() && window_index == 0)
		{
			render_avi_quad();
			bgfx::touch(s_current_view);
			update_recording();
		}

		bgfx::frame();
	}

	return 0;
}

void renderer_bgfx::update_recording()
{
	bgfx::blit(s_current_view > 0 ? s_current_view - 1 : 0, m_avi_texture, 0, 0, bgfx::getTexture(m_avi_target->target()));
	bgfx::readTexture(m_avi_texture, m_avi_data);

	int i = 0;
	for (int y = 0; y < m_avi_bitmap.height(); y++)
	{
		uint32_t *dst = &m_avi_bitmap.pix(y);

		for (int x = 0; x < m_avi_bitmap.width(); x++)
		{
			*dst++ = 0xff000000 | (m_avi_data[i + 0] << 16) | (m_avi_data[i + 1] << 8) | m_avi_data[i + 2];
			i += 4;
		}
	}

	m_avi_writer->video_frame(m_avi_bitmap);
}

void renderer_bgfx::add_audio_to_recording(const int16_t *buffer, int samples_this_frame)
{
	std::shared_ptr<osd_window> win = assert_window();
	if (m_avi_writer != nullptr && m_avi_writer->recording() && win->index() == 0)
	{
		m_avi_writer->audio_frame(buffer, samples_this_frame);
	}
}

bool renderer_bgfx::update_dimensions()
{
	std::shared_ptr<osd_window> win = assert_window();

	const uint32_t window_index = win->index();
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
			bgfx::reset(win->main_window()->get_size().width(), win->main_window()->get_size().height(), video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			m_dimensions = osd_dim(width, height);

			delete m_framebuffer;
#ifdef OSD_WINDOWS
			m_framebuffer = m_targets->create_backbuffer(std::static_pointer_cast<win_window_info>(win)->platform_window(), width, height);
#elif defined(OSD_UWP)
			m_framebuffer = m_targets->create_backbuffer(AsInspectable(std::static_pointer_cast<uwp_window_info>(win)->platform_window()), width, height);
#elif defined(OSD_MAC)
			m_framebuffer = m_targets->create_backbuffer(GetOSWindow(std::static_pointer_cast<mac_window_info>(win)->platform_window()), width, height);
#else
			m_framebuffer = m_targets->create_backbuffer(sdlNativeWindowHandle(std::dynamic_pointer_cast<sdl_window_info>(win)->platform_window()), width, height);
#endif
			if (m_ortho_view)
				m_ortho_view->set_backbuffer(m_framebuffer);

			bgfx::setViewFrameBuffer(s_current_view, m_framebuffer->target());
			bgfx::setViewClear(s_current_view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
			bgfx::setViewMode(s_current_view, bgfx::ViewMode::Sequential);
			bgfx::touch(s_current_view);
			bgfx::frame();
			return true;
		}
	}
	return false;
}

void renderer_bgfx::setup_ortho_view()
{
	if (!m_ortho_view)
	{
		m_ortho_view = new bgfx_ortho_view(this, s_current_view, m_framebuffer, m_seen_views);
	}
	m_ortho_view->update();
	if (m_ortho_view->get_index() == UINT_MAX) {
		m_ortho_view->set_index(s_current_view);
		m_ortho_view->setup();
		s_current_view++;
	}
}

renderer_bgfx::buffer_status renderer_bgfx::buffer_primitives(bool atlas_valid, render_primitive** prim, bgfx::TransientVertexBuffer* buffer, int32_t screen)
{
	int vertices = 0;

	uint32_t blend = PRIMFLAG_GET_BLENDMODE((*prim)->flags);
	while (*prim != nullptr)
	{
		switch ((*prim)->type)
		{
			case render_primitive::LINE:
				setup_ortho_view();
				put_packed_line(*prim, (ScreenVertex*)buffer->data + vertices);
				vertices += 30;
				break;

			case render_primitive::QUAD:
				if ((*prim)->texture.base == nullptr)
				{
					setup_ortho_view();
					put_packed_quad(*prim, WHITE_HASH, (ScreenVertex*)buffer->data + vertices);
					vertices += 6;
				}
				else
				{
					const uint32_t hash = get_texture_hash(*prim);
					if (atlas_valid && (*prim)->packable(PACKABLE_SIZE) && hash != 0 && m_hash_to_entry[hash].hash())
					{
						setup_ortho_view();
						put_packed_quad(*prim, hash, (ScreenVertex*)buffer->data + vertices);
						vertices += 6;
					}
					else
					{
						if (vertices > 0)
						{
							return BUFFER_PRE_FLUSH;
						}

						if (PRIMFLAG_GET_SCREENTEX((*prim)->flags) && m_chains->has_applicable_chain(screen))
						{
#if SCENE_VIEW
							setup_view(s_current_view, true);
							render_post_screen_quad(s_current_view, *prim, buffer, screen);
							s_current_view++;
#else
							setup_ortho_view();
							render_post_screen_quad(m_ortho_view->get_index(), *prim, buffer, screen);
#endif
							return BUFFER_SCREEN;
						}
						else
						{
							setup_ortho_view();
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

void renderer_bgfx::set_bgfx_state(uint32_t blend)
{
	uint64_t flags = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS;
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
			bgfx::TextureFormat::Enum dst_format = bgfx::TextureFormat::RGBA8;
			uint16_t pitch = rect.width();
			const bgfx::Memory* mem = bgfx_util::mame_texture_data_to_bgfx_texture_data(dst_format, rect.format(), rect.rowpixels(), rect.height(), rect.palette(), rect.base(), &pitch);
			bgfx::updateTexture2D(m_texture_cache->texture(), 0, 0, rect.x(), rect.y(), rect.width(), rect.height(), mem, pitch);
		}
	}
}

uint32_t renderer_bgfx::get_texture_hash(render_primitive *prim)
{
#if GIBBERISH
	uint32_t xor_value = 0x87;
	uint32_t hash = 0xdabeefed;

	int bpp = 2;
	uint32_t format = PRIMFLAG_GET_TEXFORMAT(prim->flags);
	if (format == TEXFORMAT_ARGB32 || format == TEXFORMAT_RGB32)
	{
		bpp = 4;
	}

	for (int y = 0; y < prim->texture.height; y++)
	{
		uint8_t *base = reinterpret_cast<uint8_t*>(prim->texture.base) + prim->texture.rowpixels * y;
		for (int x = 0; x < prim->texture.width * bpp; x++)
		{
			hash += base[x] ^ xor_value;
		}
	}
	return hash;
#else
	//return (reinterpret_cast<size_t>(prim->texture.base)) & 0xffffffff;
	return (reinterpret_cast<size_t>(prim->texture.base) ^ reinterpret_cast<size_t>(prim->texture.palette)) & 0xffffffff;
#endif
}

bool renderer_bgfx::check_for_dirty_atlas()
{
	bool atlas_dirty = false;

	std::shared_ptr<osd_window> win = assert_window();
	std::map<uint32_t, rectangle_packer::packable_rectangle> acquired_infos;
	for (render_primitive &prim : *win->m_primlist)
	{
		bool pack = prim.packable(PACKABLE_SIZE);
		if (prim.type == render_primitive::QUAD && prim.texture.base != nullptr && pack)
		{
			const uint32_t hash = get_texture_hash(&prim);
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

void renderer_bgfx::allocate_buffer(render_primitive *prim, uint32_t blend, bgfx::TransientVertexBuffer *buffer)
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

	if (vertices > 0 && vertices==bgfx::getAvailTransientVertexBuffer(vertices, ScreenVertex::ms_decl))
	{
		bgfx::allocTransientVertexBuffer(buffer, vertices, ScreenVertex::ms_decl);
	}
}

std::vector<ui::menu_item> renderer_bgfx::get_slider_list()
{
	m_sliders_dirty = false;
	return m_chains->get_slider_list();
}

void renderer_bgfx::set_sliders_dirty()
{
	m_sliders_dirty = true;
}

uint32_t renderer_bgfx::get_window_width(uint32_t index) const {
	return m_width[index];
}

uint32_t renderer_bgfx::get_window_height(uint32_t index) const {
	return m_height[index];
}

