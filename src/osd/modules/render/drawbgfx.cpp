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
#if (SDLMAME_SDL2)
#include <SDL2/SDL_syswm.h>
#else
#include <SDL/SDL_syswm.h>
#endif
#endif
#else
#include "sdlinc.h"
#endif

// MAMEOS headers
#include "emu.h"
#include "window.h"

#include <bgfx/bgfxplatform.h>
#include <bgfx/bgfx.h>
#include <bx/fpumath.h>
#include <bx/readerwriter.h>
#include <algorithm>

#include "drawbgfx.h"

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

//============================================================
//  MACROS
//============================================================

//============================================================
//  INLINES
//============================================================


//============================================================
//  TYPES
//============================================================


//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawbgfx_exit(void);

//============================================================
//  drawnone_create
//============================================================

static osd_renderer *drawbgfx_create(osd_window *window)
{
	return global_alloc(renderer_bgfx(window));
}

//============================================================
//  drawbgfx_init
//============================================================

int drawbgfx_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	//memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawbgfx_exit;
	callbacks->create = drawbgfx_create;

	return 0;
}

//============================================================
//  drawbgfx_exit
//============================================================

static void drawbgfx_exit(void)
{
	// Shutdown bgfx.
	bgfx::shutdown();
}

//============================================================
//  renderer_bgfx::create
//============================================================
bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);


#ifdef OSD_SDL
static void* sdlNativeWindowHandle(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi))
	{
		return nullptr;
	}

#	if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	return (void*)wmi.info.x11.window;
#	elif BX_PLATFORM_OSX
	return wmi.info.cocoa.window;
#	elif BX_PLATFORM_WINDOWS
	return wmi.info.win.window;
#	endif // BX_PLATFORM_
}
#endif

int renderer_bgfx::create()
{
	// create renderer

	osd_dim wdim = window().get_size();
	if (window().m_index == 0)
	{
#ifdef OSD_WINDOWS
		bgfx::winSetHwnd(window().m_hwnd);
#else
		bgfx::sdlSetWindow(window().sdl_window());
#endif
		bgfx::init();
		bgfx::reset(wdim.width(), wdim.height(), video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
		// Enable debug text.
		bgfx::setDebug(BGFX_DEBUG_TEXT); //BGFX_DEBUG_STATS
		m_dimensions = osd_dim(wdim.width(), wdim.height());
	}
	else
	{
#ifdef OSD_WINDOWS
		fbh = bgfx::createFrameBuffer(window().m_hwnd, wdim.width(), wdim.height());
#else
		fbh = bgfx::createFrameBuffer(sdlNativeWindowHandle(window().sdl_window()), wdim.width(), wdim.height());
#endif
		bgfx::touch(window().m_index);
	}

	PosColorTexCoord0Vertex::init();
	PosColorVertex::init();

	// Create program from shaders.
	m_progQuad = loadProgram("vs_quad", "fs_quad");
	m_progQuadTexture = loadProgram("vs_quad_texture", "fs_quad_texture");
	m_s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);

	uint32_t flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP | BGFX_TEXTURE_MIN_ANISOTROPIC | BGFX_TEXTURE_MAG_ANISOTROPIC;
	m_texture_cache = bgfx::createTexture2D(CACHE_SIZE, CACHE_SIZE, 1, bgfx::TextureFormat::BGRA8, flags);

	return 0;
}

//============================================================
//  drawbgfx_window_destroy
//============================================================

void renderer_bgfx::destroy()
{
	if (window().m_index > 0)
	{
		bgfx::destroyFrameBuffer(fbh);
	}
	bgfx::destroyUniform(m_s_texColor);
	// Cleanup.
	bgfx::destroyProgram(m_progQuad);
	bgfx::destroyProgram(m_progQuadTexture);
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

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
	if (bx::open(_reader, _filePath))
	{
		uint32_t size = (uint32_t)bx::getSize(_reader);
		const bgfx::Memory* mem = bgfx::alloc(size + 1);
		bx::read(_reader, mem->data, size);
		bx::close(_reader);
		mem->data[mem->size - 1] = '\0';
		return mem;
	}

	return nullptr;
}
static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
	char filePath[512];

	const char* shaderPath = "shaders/dx9/";

	switch (bgfx::getRendererType())
	{
	case bgfx::RendererType::Direct3D11:
	case bgfx::RendererType::Direct3D12:
		shaderPath = "shaders/dx11/";
		break;

	case bgfx::RendererType::OpenGL:
		shaderPath = "shaders/glsl/";
		break;

	case bgfx::RendererType::Metal:
		shaderPath = "shaders/metal/";
		break;

	case bgfx::RendererType::OpenGLES:
		shaderPath = "shaders/gles/";
		break;

	default:
		break;
	}

	strcpy(filePath, shaderPath);
	strcat(filePath, _name);
	strcat(filePath, ".bin");

	return bgfx::createShader(loadMem(_reader, filePath));
}

bgfx::ProgramHandle renderer_bgfx::loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
	bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
	bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
	if (nullptr != _fsName)
	{
		fsh = loadShader(_reader, _fsName);
	}

	return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}
static auto s_fileReader = new bx::CrtFileReader;

bgfx::ProgramHandle renderer_bgfx::loadProgram(const char* _vsName, const char* _fsName)
{
	return loadProgram(s_fileReader, _vsName, _fsName);
}

//============================================================
//  drawbgfx_window_draw
//============================================================

bgfx::VertexDecl renderer_bgfx::PosColorTexCoord0Vertex::ms_decl;

void renderer_bgfx::put_packed_quad(render_primitive *prim, UINT32 hash, PosColorTexCoord0Vertex* vertex)
{
	rectangle_packer::packed_rectangle& rect = m_hash_to_entry[hash];
	float u0 = float(rect.x()) / float(CACHE_SIZE);
	float v0 = float(rect.y()) / float(CACHE_SIZE);
	float u1 = u0 + float(rect.width()) / float(CACHE_SIZE);
	float v1 = v0 + float(rect.height()) / float(CACHE_SIZE);
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

void renderer_bgfx::render_textured_quad(int view, render_primitive* prim)
{
	if (bgfx::checkAvailTransientVertexBuffer(6, PosColorTexCoord0Vertex::ms_decl))
	{
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 6, PosColorTexCoord0Vertex::ms_decl);
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

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
		bgfx::setVertexBuffer(&vb);

		uint32_t texture_flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
		if (video_config.filter == 0)
		{
			texture_flags |= BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
		}

		const bgfx::Memory* mem = mame_texture_data_to_bgfx_texture_data(prim->flags & PRIMFLAG_TEXFORMAT_MASK,
			prim->texture.width, prim->texture.height, prim->texture.rowpixels, prim->texture.palette, prim->texture.base);

		bgfx::TextureHandle texture = bgfx::createTexture2D((uint16_t)prim->texture.width, (uint16_t)prim->texture.height, 1, bgfx::TextureFormat::BGRA8, texture_flags, mem);

		bgfx::setTexture(0, m_s_texColor, texture);

		set_bgfx_state(PRIMFLAG_GET_BLENDMODE(prim->flags));
		bgfx::submit(view, m_progQuadTexture);

		bgfx::destroyTexture(texture);
	}
}

bgfx::VertexDecl renderer_bgfx::PosColorVertex::ms_decl;

#define MAX_TEMP_COORDS 100

void renderer_bgfx::put_polygon(const float* coords, UINT32 num_coords, float r, UINT32 rgba, PosColorVertex* vertex)
{
	float tempCoords[MAX_TEMP_COORDS * 3];
	float tempNormals[MAX_TEMP_COORDS * 2];

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
		vertIndex++;

		vertex[vertIndex].m_x = coords[jj * 3 + 0];
		vertex[vertIndex].m_y = coords[jj * 3 + 1];
		vertex[vertIndex].m_z = coords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[jj * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[jj * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[jj * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[jj * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[jj * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertIndex++;

		vertex[vertIndex].m_x = tempCoords[ii * 3 + 0];
		vertex[vertIndex].m_y = tempCoords[ii * 3 + 1];
		vertex[vertIndex].m_z = tempCoords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = trans;
		vertIndex++;

		vertex[vertIndex].m_x = coords[ii * 3 + 0];
		vertex[vertIndex].m_y = coords[ii * 3 + 1];
		vertex[vertIndex].m_z = coords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertIndex++;
	}

	for (uint32_t ii = 2; ii < num_coords; ++ii)
	{
		vertex[vertIndex].m_x = coords[0];
		vertex[vertIndex].m_y = coords[1];
		vertex[vertIndex].m_z = coords[2];
		vertex[vertIndex].m_rgba = rgba;
		vertIndex++;

		vertex[vertIndex].m_x = coords[(ii - 1) * 3 + 0];
		vertex[vertIndex].m_y = coords[(ii - 1) * 3 + 1];
		vertex[vertIndex].m_z = coords[(ii - 1) * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertIndex++;

		vertex[vertIndex].m_x = coords[ii * 3 + 0];
		vertex[vertIndex].m_y = coords[ii * 3 + 1];
		vertex[vertIndex].m_z = coords[ii * 3 + 2];
		vertex[vertIndex].m_rgba = rgba;
		vertIndex++;
	}
}

void renderer_bgfx::put_line(float x0, float y0, float x1, float y1, float r, UINT32 rgba, PosColorVertex* vertex, float fth)
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

//============================================================
//  copyline_palette16
//============================================================

static inline void copyline_palette16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette)
{
	for (int x = 0; x < width; x++)
		*dst++ = 0xff000000 | palette[*src++];
}


//============================================================
//  copyline_palettea16
//============================================================

static inline void copyline_palettea16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette)
{
	for (int x = 0; x < width; x++)
		*dst++ = palette[*src++];
}


//============================================================
//  copyline_rgb32
//============================================================

static inline void copyline_rgb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette)
{
	int x;

	// palette (really RGB map) case
	if (palette != nullptr)
	{
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		for (x = 0; x < width; x++)
			*dst++ = 0xff000000 | *src++;
	}
}


//============================================================
//  copyline_argb32
//============================================================

static inline void copyline_argb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette)
{
	int x;
	// palette (really RGB map) case
	if (palette != nullptr)
	{
		for (x = 0; x < width; x++)
		{
			rgb_t srcpix = *src++;
			*dst++ = (srcpix & 0xff000000) | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
		}
	}

	// direct case
	else
	{
		for (x = 0; x < width; x++)
			*dst++ = *src++;
	}
}

static inline UINT32 ycc_to_rgb(UINT8 y, UINT8 cb, UINT8 cr)
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
	r = (common + 409 * cr - 409 * 128 + 128) >> 8;
	g = (common - 100 * cb + 100 * 128 - 208 * cr + 208 * 128 + 128) >> 8;
	b = (common + 516 * cb - 516 * 128 + 128) >> 8;

	if (r < 0) r = 0;
	else if (r > 255) r = 255;
	if (g < 0) g = 0;
	else if (g > 255) g = 255;
	if (b < 0) b = 0;
	else if (b > 255) b = 255;

	return rgb_t(0xff, r, g, b);
}

//============================================================
//  copyline_yuy16_to_argb
//============================================================

static inline void copyline_yuy16_to_argb(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xprescale)
{
	int x;

	assert(width % 2 == 0);

	// palette (really RGB map) case
	if (palette != nullptr)
	{
		for (x = 0; x < width / 2; x++)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(palette[0x000 + (srcpix1 >> 8)], cb, cr);
		}
	}

	// direct case
	else
	{
		for (x = 0; x < width; x += 2)
		{
			UINT16 srcpix0 = *src++;
			UINT16 srcpix1 = *src++;
			UINT8 cb = srcpix0 & 0xff;
			UINT8 cr = srcpix1 & 0xff;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(srcpix1 >> 8, cb, cr);
		}
	}
}

const bgfx::Memory* renderer_bgfx::mame_texture_data_to_bgfx_texture_data(UINT32 format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
{
	const bgfx::Memory* mem = bgfx::alloc(width * height * 4);
	for (int y = 0; y < height; y++)
	{
		switch (format)
		{
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
				copyline_palette16((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTEA16):
				copyline_palettea16((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
				copyline_yuy16_to_argb((UINT32*)mem->data + y * width, (UINT16*)base + y * rowpixels, width, palette, 1);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
				copyline_argb32((UINT32*)mem->data + y * width, (UINT32*)base + y * rowpixels, width, palette);
				break;
			case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
				copyline_rgb32((UINT32*)mem->data + y * width, (UINT32*)base + y * rowpixels, width, palette);
				break;
			default:
				break;
		}
	}
	return mem;
}

int renderer_bgfx::draw(int update)
{
	int index = window().m_index;
	// Set view 0 default viewport.
	osd_dim wdim = window().get_size();
	int width = wdim.width();
	int height = wdim.height();
	if (index == 0)
	{
		if ((m_dimensions != osd_dim(width, height))) {
			bgfx::reset(width, height, video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			m_dimensions = osd_dim(width, height);
		}
	}
	else {
		if ((m_dimensions != osd_dim(width, height))) {
			bgfx::reset(window().m_main->get_size().width(), window().m_main->get_size().height(), video_config.waitvsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
			if (bgfx::isValid(fbh))
			{
				bgfx::destroyFrameBuffer(fbh);
			}
#ifdef OSD_WINDOWS
			fbh = bgfx::createFrameBuffer(window().m_hwnd, width, height);
#else
			fbh = bgfx::createFrameBuffer(sdlNativeWindowHandle(window().sdl_window()), width, height);
#endif
			bgfx::setViewFrameBuffer(index, fbh);
			m_dimensions = osd_dim(width, height);
			bgfx::setViewClear(index
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x000000ff
				, 1.0f
				, 0
				);
			bgfx::touch(index);
			bgfx::frame();
			return 0;
		}
	}
	if (index != 0) bgfx::setViewFrameBuffer(index, fbh);
	bgfx::setViewSeq(index, true);
	bgfx::setViewRect(index, 0, 0, width, height);

	// Setup view transform.
	{
		float view[16];
		bx::mtxIdentity(view);

		float left = 0.0f;
		float top = 0.0f;
		float right = width;
		float bottom = height;
		float proj[16];
		bx::mtxOrtho(proj, left, right, bottom, top, 0.0f, 100.0f);
		bgfx::setViewTransform(index, view, proj);
	}
	bgfx::setViewClear(index
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x000000ff
		, 1.0f
		, 0
		);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::touch(index);

	window().m_primlist->acquire_lock();

	bgfx::TransientVertexBuffer flat_buffer[4];
	bgfx::TransientVertexBuffer textured_buffer[4];

	allocate_buffers(flat_buffer, textured_buffer);

	int flat_vertices[4] = { 0, 0, 0, 0 };
	int textured_vertices[4] = { 0, 0, 0, 0 };

	// Mark our texture atlas as dirty if we need to do so
	bool atlas_valid = update_atlas();

	memset(flat_vertices, 0, sizeof(int) * 4);
	memset(textured_vertices, 0, sizeof(int) * 4);

	for (render_primitive *prim = window().m_primlist->first(); prim != nullptr; prim = prim->next())
	{
		UINT32 blend = PRIMFLAG_GET_BLENDMODE(prim->flags);

		switch (prim->type)
		{
		case render_primitive::LINE:
			put_line(prim->bounds.x0, prim->bounds.y0, prim->bounds.x1, prim->bounds.y1, 1.0f, u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255), (PosColorVertex*)flat_buffer[blend].data + flat_vertices[blend], 1.0f);
			flat_vertices[blend] += 30;
			break;

		case render_primitive::QUAD:
			if (prim->texture.base == nullptr)
			{
				render_flat_quad(index, prim);
			}
			else
			{
				if (atlas_valid && (prim->flags & PRIMFLAG_PACKABLE) && prim->texture.hash != 0 && m_hash_to_entry[prim->texture.hash].hash())
				{
					put_packed_quad(prim, prim->texture.hash, (PosColorTexCoord0Vertex*)textured_buffer[blend].data + textured_vertices[blend]);
					textured_vertices[blend] += 6;
				}
				else
				{
					render_textured_quad(index, prim);
				}
			}
			break;

		default:
			throw emu_fatalerror("Unexpected render_primitive type");
		}
	}

	for (UINT32 blend_mode = 0; blend_mode < BLENDMODE_COUNT; blend_mode++)
	{
		if (flat_vertices[blend_mode] > 0)
		{
			set_bgfx_state(blend_mode);
			bgfx::setVertexBuffer(&flat_buffer[blend_mode]);
			bgfx::submit(index, m_progQuad);
		}
	}

	for (UINT32 blend_mode = 0; blend_mode < BLENDMODE_COUNT; blend_mode++)
	{
		if (textured_vertices[blend_mode] > 0)
		{
			set_bgfx_state(blend_mode);
			bgfx::setVertexBuffer(&textured_buffer[blend_mode]);
			bgfx::setTexture(0, m_s_texColor, m_texture_cache);
			bgfx::submit(index, m_progQuadTexture);
		}
	}

	window().m_primlist->release_lock();
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	if (index==0) bgfx::frame();

	return 0;
}

void renderer_bgfx::set_bgfx_state(UINT32 blend)
{
	uint64_t flags = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_DEPTH_TEST_ALWAYS;

	switch (blend)
	{
		case BLENDMODE_NONE:
			bgfx::setState(flags);
			break;
		case BLENDMODE_ALPHA:
			bgfx::setState(flags | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA));
			break;
		case BLENDMODE_RGB_MULTIPLY:
			bgfx::setState(flags | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO));
			break;
		case BLENDMODE_ADD:
			bgfx::setState(flags | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE));
			break;
	}
}

void renderer_bgfx::render_flat_quad(int view, render_primitive *prim)
{
	if (bgfx::checkAvailTransientVertexBuffer(6, PosColorVertex::ms_decl))
	{
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 6, PosColorVertex::ms_decl);
		PosColorVertex* vertex = (PosColorVertex*)vb.data;

		UINT32 rgba = u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255);

		vertex[0].m_x = prim->bounds.x0;
		vertex[0].m_y = prim->bounds.y0;
		vertex[0].m_z = 0;
		vertex[0].m_rgba = rgba;

		vertex[1].m_x = prim->bounds.x1;
		vertex[1].m_y = prim->bounds.y0;
		vertex[1].m_z = 0;
		vertex[1].m_rgba = rgba;

		vertex[2].m_x = prim->bounds.x1;
		vertex[2].m_y = prim->bounds.y1;
		vertex[2].m_z = 0;
		vertex[2].m_rgba = rgba;

		vertex[3].m_x = prim->bounds.x1;
		vertex[3].m_y = prim->bounds.y1;
		vertex[3].m_z = 0;
		vertex[3].m_rgba = rgba;

		vertex[4].m_x = prim->bounds.x0;
		vertex[4].m_y = prim->bounds.y1;
		vertex[4].m_z = 0;
		vertex[4].m_rgba = rgba;

		vertex[5].m_x = prim->bounds.x0;
		vertex[5].m_y = prim->bounds.y0;
		vertex[5].m_z = 0;
		vertex[5].m_rgba = rgba;
		bgfx::setVertexBuffer(&vb);

		set_bgfx_state(PRIMFLAG_GET_BLENDMODE(prim->flags));
		bgfx::submit(view, m_progQuad);
	}
}

bool renderer_bgfx::update_atlas()
{
	bool atlas_dirty = check_for_dirty_atlas();

	if (atlas_dirty)
	{
		m_hash_to_entry.clear();

		std::vector<std::vector<rectangle_packer::packed_rectangle>> packed;
		if (m_packer.pack(m_texinfo, packed, 1024))
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
					bgfx::updateTexture2D(m_texture_cache, 0, rect.x(), rect.y(), rect.width(), rect.height(), mem);
				}
			}
		}
		else
		{
			m_texinfo.clear();
			return false;
		}
	}
	return true;
}

bool renderer_bgfx::check_for_dirty_atlas()
{
	bool atlas_dirty = false;

	std::map<UINT32, rectangle_packer::packable_rectangle> acquired_infos;
	for (render_primitive *prim = window().m_primlist->first(); prim != nullptr; prim = prim->next())
	{
		bool pack = prim->flags & PRIMFLAG_PACKABLE;
		if (prim->type == render_primitive::QUAD && prim->texture.base != nullptr && pack)
		{
			const UINT32 hash = prim->texture.hash;

			// If this texture is packable and not currently in the atlas, prepare the texture for putting in the atlas
			if (hash != 0 && m_hash_to_entry[hash].hash() == 0 && acquired_infos[hash].hash() == 0)
			{	// Create create the texture and mark the atlas dirty
				atlas_dirty = true;

				m_texinfo.push_back(rectangle_packer::packable_rectangle(hash, prim->flags & PRIMFLAG_TEXFORMAT_MASK,
					prim->texture.width, prim->texture.height,
					prim->texture.rowpixels, prim->texture.palette, prim->texture.base));
				acquired_infos[hash] = m_texinfo[m_texinfo.size() - 1];
			}
		}
	}

	return atlas_dirty;
}

void renderer_bgfx::allocate_buffers(bgfx::TransientVertexBuffer *flat_buffer, bgfx::TransientVertexBuffer *textured_buffer)
{
	int flat_vertices[4] = { 0, 0, 0, 0 };
	int textured_vertices[4] = { 0, 0, 0, 0 };

	for (render_primitive *prim = window().m_primlist->first(); prim != nullptr; prim = prim->next())
	{
		switch (prim->type)
		{
			case render_primitive::LINE:
				flat_vertices[PRIMFLAG_GET_BLENDMODE(prim->flags)] += 30;
				break;

			case render_primitive::QUAD:
				if (prim->flags & PRIMFLAG_PACKABLE && prim->texture.base != nullptr && prim->texture.hash != 0)
				{
					textured_vertices[PRIMFLAG_GET_BLENDMODE(prim->flags)] += 6;
				}
				break;
			default:
				// Do nothing
				break;
		}
	}

	for (int blend_mode = 0; blend_mode < 4; blend_mode++)
	{
		if (flat_vertices[blend_mode] > 0 && bgfx::checkAvailTransientVertexBuffer(flat_vertices[blend_mode], PosColorVertex::ms_decl))
		{
			bgfx::allocTransientVertexBuffer(&flat_buffer[blend_mode], flat_vertices[blend_mode], PosColorVertex::ms_decl);
		}
		if (textured_vertices[blend_mode] > 0 && bgfx::checkAvailTransientVertexBuffer(textured_vertices[blend_mode], PosColorTexCoord0Vertex::ms_decl))
		{
			bgfx::allocTransientVertexBuffer(&textured_buffer[blend_mode], textured_vertices[blend_mode], PosColorTexCoord0Vertex::ms_decl);
		}
	}
}
