// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Dario Manesku,Branimir Karadzic,Aaron Giles
//============================================================
//
//  drawbgfx.c - BGFX drawer
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


/* sdl_info is the information about SDL for the current screen */
class renderer_bgfx : public osd_renderer
{
public:
	renderer_bgfx(osd_window *w)
	: osd_renderer(w, FLAG_NONE),
		m_dimensions(0,0)
	{}

	virtual int create() override;
	virtual int draw(const int update) override;
#ifdef OSD_SDL
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#else
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
#endif
	virtual void destroy() override;
	virtual render_primitive_list *get_primitives() override
	{
		osd_dim wdim = window().get_size();
		window().target()->set_bounds(wdim.width(), wdim.height(), window().aspect());
		return &window().target()->get_primitives();
	}

	bgfx::ProgramHandle m_progQuad;
	bgfx::ProgramHandle m_progQuadTexture;
	bgfx::ProgramHandle m_progLine;
	bgfx::UniformHandle m_s_texColor;
	bgfx::FrameBufferHandle fbh;
	// Original display_mode
	osd_dim			m_dimensions;
};


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
		return NULL;
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
	else {
#ifdef OSD_WINDOWS
		fbh = bgfx::createFrameBuffer(window().m_hwnd, wdim.width(), wdim.height());
#else
		fbh = bgfx::createFrameBuffer(sdlNativeWindowHandle(window().sdl_window()), wdim.width(), wdim.height());
#endif
		bgfx::touch(window().m_index);
	}
	// Create program from shaders.
	m_progQuad = loadProgram("vs_quad", "fs_quad");
	m_progQuadTexture = loadProgram("vs_quad_texture", "fs_quad_texture");
	m_progLine = loadProgram("vs_line", "fs_line");
	m_s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);

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
	bgfx::destroyProgram(m_progLine);
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

	return NULL;
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

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
	bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
	bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
	if (NULL != _fsName)
	{
		fsh = loadShader(_reader, _fsName);
	}

	return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}
static auto s_fileReader = new bx::CrtFileReader;

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
{
	
	return loadProgram(s_fileReader, _vsName, _fsName);
}
//============================================================
//  drawbgfx_window_draw
//============================================================

struct PosColorTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_rgba;
	float m_u;
	float m_v;

	static void init()
	{
		ms_decl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};
bgfx::VertexDecl PosColorTexCoord0Vertex::ms_decl;

void screenQuad(float _x1
	, float _y1
	, float _x2
	, float _y2
	, uint32_t _abgr
	, render_quad_texuv uv
	)
{
	if (bgfx::checkAvailTransientVertexBuffer(6, PosColorTexCoord0Vertex::ms_decl))
	{
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 6, PosColorTexCoord0Vertex::ms_decl);
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

		const float minx = _x1;
		const float miny = _y1;
		const float maxx = _x2;
		const float maxy = _y2;
		const float zz = 0.0f;

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_rgba = _abgr;
		vertex[0].m_u = uv.tl.u;
		vertex[0].m_v = uv.tl.v;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_rgba = _abgr;
		vertex[1].m_u = uv.tr.u;
		vertex[1].m_v = uv.tr.v;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_rgba = _abgr;
		vertex[2].m_u = uv.br.u;
		vertex[2].m_v = uv.br.v;

		vertex[3].m_x = maxx;
		vertex[3].m_y = maxy;
		vertex[3].m_z = zz;
		vertex[3].m_rgba = _abgr;
		vertex[3].m_u = uv.br.u;
		vertex[3].m_v = uv.br.v;

		vertex[4].m_x = minx;
		vertex[4].m_y = maxy;
		vertex[4].m_z = zz;
		vertex[4].m_rgba = _abgr;
		vertex[4].m_u = uv.bl.u;
		vertex[4].m_v = uv.bl.v;

		vertex[5].m_x = minx;
		vertex[5].m_y = miny;
		vertex[5].m_z = zz;
		vertex[5].m_rgba = _abgr;
		vertex[5].m_u = uv.tl.u;
		vertex[5].m_v = uv.tl.v;
		bgfx::setVertexBuffer(&vb);
	}
}


struct PosColorVertex
{
	float m_x;
	float m_y;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	}

	static bgfx::VertexDecl ms_decl;
};
bgfx::VertexDecl PosColorVertex::ms_decl;

#define MAX_TEMP_COORDS 100

void drawPolygon(const float* _coords, uint32_t _numCoords, float _r, uint32_t _abgr)
{
	float tempCoords[MAX_TEMP_COORDS * 2];
	float tempNormals[MAX_TEMP_COORDS * 2];

	_numCoords = _numCoords < MAX_TEMP_COORDS ? _numCoords : MAX_TEMP_COORDS;

	for (uint32_t ii = 0, jj = _numCoords - 1; ii < _numCoords; jj = ii++)
	{
		const float* v0 = &_coords[jj * 2];
		const float* v1 = &_coords[ii * 2];
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

	for (uint32_t ii = 0, jj = _numCoords - 1; ii < _numCoords; jj = ii++)
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

		tempCoords[ii * 2 + 0] = _coords[ii * 2 + 0] + dmx * _r;
		tempCoords[ii * 2 + 1] = _coords[ii * 2 + 1] + dmy * _r;
	}

	uint32_t numVertices = _numCoords * 6 + (_numCoords - 2) * 3;
	if (bgfx::checkAvailTransientVertexBuffer(numVertices, PosColorVertex::ms_decl))
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, numVertices, PosColorVertex::ms_decl);
		uint32_t trans = _abgr & 0xffffff;

		PosColorVertex* vertex = (PosColorVertex*)tvb.data;
		for (uint32_t ii = 0, jj = _numCoords - 1; ii < _numCoords; jj = ii++)
		{
			vertex->m_x = _coords[ii * 2 + 0];
			vertex->m_y = _coords[ii * 2 + 1];
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[jj * 2 + 0];
			vertex->m_y = _coords[jj * 2 + 1];
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = tempCoords[jj * 2 + 0];
			vertex->m_y = tempCoords[jj * 2 + 1];
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = tempCoords[jj * 2 + 0];
			vertex->m_y = tempCoords[jj * 2 + 1];
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = tempCoords[ii * 2 + 0];
			vertex->m_y = tempCoords[ii * 2 + 1];
			vertex->m_abgr = trans;
			++vertex;

			vertex->m_x = _coords[ii * 2 + 0];
			vertex->m_y = _coords[ii * 2 + 1];
			vertex->m_abgr = _abgr;
			++vertex;
		}

		for (uint32_t ii = 2; ii < _numCoords; ++ii)
		{
			vertex->m_x = _coords[0];
			vertex->m_y = _coords[1];
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[(ii - 1) * 2 + 0];
			vertex->m_y = _coords[(ii - 1) * 2 + 1];
			vertex->m_abgr = _abgr;
			++vertex;

			vertex->m_x = _coords[ii * 2 + 0];
			vertex->m_y = _coords[ii * 2 + 1];
			vertex->m_abgr = _abgr;
			++vertex;
		}

		bgfx::setVertexBuffer(&tvb);
	}
}

void drawLine(float _x0, float _y0, float _x1, float _y1, float _r, uint32_t _abgr, float _fth = 1.0f)
{
	float dx = _x1 - _x0;
	float dy = _y1 - _y0;
	float d = sqrtf(dx * dx + dy * dy);
	if (d > 0.0001f)
	{
		d = 1.0f / d;
		dx *= d;
		dy *= d;
	}

	float nx = dy;
	float ny = -dx;
	float verts[4 * 2];
	_r -= _fth;
	_r *= 0.5f;
	if (_r < 0.01f)
	{
		_r = 0.01f;
	}

	dx *= _r;
	dy *= _r;
	nx *= _r;
	ny *= _r;

	verts[0] = _x0 - dx - nx;
	verts[1] = _y0 - dy - ny;

	verts[2] = _x0 - dx + nx;
	verts[3] = _y0 - dy + ny;

	verts[4] = _x1 + dx + nx;
	verts[5] = _y1 + dy + ny;

	verts[6] = _x1 + dx - nx;
	verts[7] = _y1 + dy - ny;

	drawPolygon(verts, 4, _fth, _abgr);
}

void initVertexDecls()
{
	PosColorTexCoord0Vertex::init();
	PosColorVertex::init();
}

static inline
uint32_t u32Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
{
	return 0
		| (uint32_t(_r) << 0)
		| (uint32_t(_g) << 8)
		| (uint32_t(_b) << 16)
		| (uint32_t(_a) << 24)
		;
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
	if (palette != NULL)
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
	if (palette != NULL)
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
	if (palette != NULL)
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
int renderer_bgfx::draw(int update)
{
	initVertexDecls();	
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
	// Draw quad.
	// now draw
	uint32_t texture_flags = BGFX_TEXTURE_U_CLAMP | BGFX_TEXTURE_V_CLAMP;
	if (video_config.filter==0) texture_flags |= BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT | BGFX_TEXTURE_MIP_POINT;
	
	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		uint64_t flags = BGFX_STATE_RGB_WRITE;
		switch (prim->flags & PRIMFLAG_BLENDMODE_MASK)
		{
		case PRIMFLAG_BLENDMODE(BLENDMODE_NONE):
			break;
		case PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA):
			flags |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
			break;
		case PRIMFLAG_BLENDMODE(BLENDMODE_RGB_MULTIPLY):
			flags |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO);
			break;
		case PRIMFLAG_BLENDMODE(BLENDMODE_ADD):
			flags |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		}
		bool alpha = false;
		switch (prim->type)
		{
			/**
			 * Try to stay in one Begin/End block as long as possible,
			 * since entering and leaving one is most expensive..
			 */
		case render_primitive::LINE:

			drawLine(prim->bounds.x0, prim->bounds.y0, prim->bounds.x1, prim->bounds.y1,
				1.0f,
				u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255),
				1.0f);
			bgfx::setState(flags);
			bgfx::submit(index, m_progLine);
			break;

		case render_primitive::QUAD:
			if (prim->texture.base == nullptr) {
				render_quad_texuv uv;
				uv.tl.u = uv.tl.v = uv.tr.u = uv.tr.v = 0;
				uv.bl.u = uv.bl.v = uv.br.u = uv.br.v = 0;
				screenQuad(prim->bounds.x0, prim->bounds.y0, prim->bounds.x1, prim->bounds.y1,
					u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255), uv);
				bgfx::setState(flags);
				bgfx::submit(index, m_progQuad);
			}
			else {
				screenQuad(prim->bounds.x0, prim->bounds.y0, prim->bounds.x1, prim->bounds.y1,
					u32Color(prim->color.r * 255, prim->color.g * 255, prim->color.b * 255, prim->color.a * 255), prim->texcoords);
				bgfx::TextureHandle m_texture;
				// render based on the texture coordinates
				switch (prim->flags & PRIMFLAG_TEXFORMAT_MASK)
				{
					case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTEA16):
						alpha = true;
					case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
					{
						auto mem = bgfx::alloc(prim->texture.width*prim->texture.height * 4);
						if (alpha)
						{
							for (int y = 0; y < prim->texture.height; y++)
							{
								copyline_palettea16((UINT32*)mem->data + y*prim->texture.width, (UINT16*)prim->texture.base + y*prim->texture.rowpixels, prim->texture.width, prim->texture.palette);
							}
						}
						else
						{
							for (int y = 0; y < prim->texture.height; y++)
							{
								copyline_palette16((UINT32*)mem->data + y*prim->texture.width, (UINT16*)prim->texture.base + y*prim->texture.rowpixels, prim->texture.width, prim->texture.palette);
							}
						}

						m_texture = bgfx::createTexture2D((uint16_t)prim->texture.width
							, (uint16_t)prim->texture.height
							, 1
							, bgfx::TextureFormat::BGRA8
							, texture_flags
							, mem
							);
					}
					break;
				case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
					{
						auto mem = bgfx::alloc(prim->texture.width*prim->texture.height * 4);
						for (int y = 0; y < prim->texture.height; y++)
						{
							copyline_yuy16_to_argb((UINT32*)mem->data + y*prim->texture.width, (UINT16*)prim->texture.base + y*prim->texture.rowpixels, prim->texture.width, prim->texture.palette, 1);
						}
						m_texture = bgfx::createTexture2D((uint16_t)prim->texture.width
							, (uint16_t)prim->texture.height
							, 1
							, bgfx::TextureFormat::BGRA8
							, texture_flags
							, mem
							);
					}
					break;
				case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
					alpha = true;
				case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
					{
						if (prim->texture.rowpixels!=prim->texture.width)
						{
							auto mem = bgfx::alloc(prim->texture.width*prim->texture.height * 4);
							if (alpha)
							{
								for (int y = 0; y < prim->texture.height; y++)
								{
									copyline_argb32((UINT32*)mem->data + y*prim->texture.width, (UINT32*)prim->texture.base + y*prim->texture.rowpixels, prim->texture.width, prim->texture.palette);
								}
							}
							else
							{
								for (int y = 0; y < prim->texture.height; y++)
								{
									copyline_rgb32((UINT32*)mem->data + y*prim->texture.width, (UINT32*)prim->texture.base + y*prim->texture.rowpixels, prim->texture.width, prim->texture.palette);
								}
							}

							m_texture = bgfx::createTexture2D((uint16_t)prim->texture.width
								, (uint16_t)prim->texture.height
								, 1
								, bgfx::TextureFormat::BGRA8
								, texture_flags
								, mem
								);
						} else {
							m_texture = bgfx::createTexture2D((uint16_t)prim->texture.width
								, (uint16_t)prim->texture.height
								, 1
								, bgfx::TextureFormat::BGRA8
								, texture_flags
								, bgfx::copy(prim->texture.base, prim->texture.width*prim->texture.height*4)
								);							
						}
					}
					break;

				default:
					break;
				}
				bgfx::setTexture(0, m_s_texColor, m_texture);
				bgfx::setState(flags);
				bgfx::submit(index, m_progQuadTexture);
				bgfx::destroyTexture(m_texture);
			}
			break;

		default:
			throw emu_fatalerror("Unexpected render_primitive type");
		}
	}

	window().m_primlist->release_lock();
	// Advance to next frame. Rendering thread will be kicked to
	// process submitted rendering primitives.
	if (index==0) bgfx::frame();

	return 0;
}
