// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
//============================================================
//
//  drawogl.c - SDL software and OpenGL implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  Note: D3D9 goes to a lot of trouble to fiddle with MODULATE
//        mode on textures.  That is the default in OpenGL so we
//        don't have to touch it.
//
//============================================================

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "osdcomm.h"
#include "emu.h"
#include "emuopts.h"

#ifndef OSD_WINDOWS
// standard SDL headers
#define TOBEMIGRATED 1
#include "sdlinc.h"
#endif

#include "modules/lib/osdlib.h"
#include "modules/lib/osdobj_common.h"

#if defined(OSD_WINDOWS) && !defined(SDLMAME_SDL2)
#define SDLMAME_SDL2 0
#endif

// OpenGL headers
#include "modules/opengl/osd_opengl.h"

#include "modules/opengl/gl_shader_tool.h"
#include "modules/opengl/gl_shader_mgr.h"

#if defined(SDLMAME_MACOSX)
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum, GLuint);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERPROC) (GLenum, GLenum);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei, const GLuint *);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum m_target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum m_target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum m_target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum m_target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum m_target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum m_target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
#endif

// make sure the extensions compile OK everywhere
#ifndef GL_TEXTURE_STORAGE_HINT_APPLE
#define GL_TEXTURE_STORAGE_HINT_APPLE     0x85bc
#endif

#ifndef GL_STORAGE_CACHED_APPLE
#define GL_STORAGE_CACHED_APPLE           0x85be
#endif

#ifndef GL_UNPACK_CLIENT_STORAGE_APPLE
#define GL_UNPACK_CLIENT_STORAGE_APPLE    0x85b2
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#define GL_TEXTURE_RECTANGLE_ARB          0x84F5
#endif

#ifndef GL_PIXEL_UNPACK_BUFFER_ARB
#define GL_PIXEL_UNPACK_BUFFER_ARB        0x88EC
#endif

#ifndef GL_STREAM_DRAW
#define GL_STREAM_DRAW                    0x88E0
#endif

#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY                     0x88B9
#endif

#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB               0x8892
#endif

#ifndef GL_PIXEL_UNPACK_BUFFER_ARB
#define GL_PIXEL_UNPACK_BUFFER_ARB        0x88EC
#endif

#ifndef GL_FRAMEBUFFER_EXT
#define GL_FRAMEBUFFER_EXT              0x8D40
#define GL_FRAMEBUFFER_COMPLETE_EXT         0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT    0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT    0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT  0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT        0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT       0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT   0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT   0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT          0x8CDD
#define GL_RENDERBUFFER_EXT             0x8D41
#define GL_DEPTH_COMPONENT16                0x81A5
#define GL_DEPTH_COMPONENT24                0x81A6
#define GL_DEPTH_COMPONENT32                0x81A7
#endif

#define HASH_SIZE       ((1<<10)+1)
#define OVERFLOW_SIZE   (1<<10)

// OSD headers
#ifndef OSD_WINDOWS
#include "osdsdl.h"
#include "window.h"
#else
#include "../windows/window.h"
typedef UINT64 HashT;
#endif

//============================================================
//  DEBUGGING
//============================================================

#define DEBUG_MODE_SCORES   0
#define USE_WIN32_STYLE_LINES   0   // use the same method baseline does - yields somewhat nicer vectors but a little buggy

//============================================================
//  CONSTANTS
//============================================================

enum
{
	TEXTURE_TYPE_NONE,
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_DYNAMIC,
	TEXTURE_TYPE_SHADER,
	TEXTURE_TYPE_SURFACE
};


//============================================================
//  MACROS
//============================================================

// texture formats
// This used to be an enum, but these are now defines so we can use them as
// preprocessor conditionals
#define SDL_TEXFORMAT_ARGB32            (0) // non-16-bit textures or specials
#define SDL_TEXFORMAT_RGB32             (1)
#define SDL_TEXFORMAT_RGB32_PALETTED    (2)
#define SDL_TEXFORMAT_YUY16             (3)
#define SDL_TEXFORMAT_YUY16_PALETTED    (4)
#define SDL_TEXFORMAT_PALETTE16         (5)
#define SDL_TEXFORMAT_RGB15             (6)
#define SDL_TEXFORMAT_RGB15_PALETTED    (7)
#define SDL_TEXFORMAT_PALETTE16A        (8)
#if 0
// special texture formats for 16bpp texture destination support, do not use
// to address the tex properties / tex functions arrays!
#define SDL_TEXFORMAT_PALETTE16_ARGB1555    (16)
#define SDL_TEXFORMAT_RGB15_ARGB1555        (17)
#define SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555   (18)
#endif

#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)
#define GL_NO_PRIMITIVE -1

//============================================================
//  TYPES
//============================================================

#if defined(OSD_WINDOWS)

class win_gl_context : public osd_gl_context
{
public:
	win_gl_context(HWND window) : osd_gl_context(), m_context(0), m_window(NULL), m_hdc(0)
	{
		m_error[0] = 0;

		this->pfn_wglGetProcAddress = (PROC (WINAPI *)(LPCSTR lpszProc)) GetProcAddress(m_module, "wglGetProcAddress");
		this->pfn_wglCreateContext = (HGLRC (WINAPI *)(HDC hdc)) GetProcAddress(m_module, "wglCreateContext");
		this->pfn_wglDeleteContext = (BOOL (WINAPI *)(HGLRC hglrc)) GetProcAddress(m_module, "wglDeleteContext");
		this->pfn_wglMakeCurrent = (BOOL (WINAPI *)(HDC hdc, HGLRC hglrc)) GetProcAddress(m_module, "wglMakeCurrent");

		this->pfn_wglGetExtensionsStringEXT = (const char *(WINAPI *) (void)) pfn_wglGetProcAddress("wglGetExtensionsStringEXT");

		if (WGLExtensionSupported("WGL_EXT_swap_control"))
		{
			this->pfn_wglSwapIntervalEXT = (BOOL (WINAPI *) (int)) getProcAddress("wglSwapIntervalEXT");
			this->pfn_wglGetSwapIntervalEXT = (int (WINAPI *) (void)) getProcAddress("wglGetSwapIntervalEXT");
		}
		else
		{
			pfn_wglSwapIntervalEXT = NULL;
			pfn_wglGetSwapIntervalEXT = NULL;
		}

		m_hdc = GetDC(window);
		if (!setupPixelFormat(m_hdc))
		{
			m_context = this->pfn_wglCreateContext(m_hdc);
			if  (!m_context)
			{
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), 0, m_error, 255, NULL);
				return;
			}
			this->pfn_wglMakeCurrent(m_hdc, m_context);
		}
	}

	virtual ~win_gl_context()
	{
		this->pfn_wglDeleteContext(m_context);
		ReleaseDC(m_window, m_hdc);
	}

	virtual void MakeCurrent() override
	{
		this->pfn_wglMakeCurrent(m_hdc, m_context);
	}

	virtual const char *LastErrorMsg() override
	{
		if (m_error[0] == 0)
			return NULL;
		else
			return m_error;
	}

	virtual void *getProcAddress(const char *proc) override
	{
		void *ret = (void *) GetProcAddress(m_module, proc);
		if (ret == NULL)
			ret = (void *) this->pfn_wglGetProcAddress(proc);
		return ret;
	}

	virtual int SetSwapInterval(const int swap) override
	{
		if (this->pfn_wglSwapIntervalEXT != NULL)
		{
			this->pfn_wglSwapIntervalEXT(swap ? 1 : 0);
		}
		return 0;
	}

	virtual void SwapBuffer() override
	{
		SwapBuffers(m_hdc);
		//wglSwapLayerBuffers(GetDC(window().m_hwnd), WGL_SWAP_MAIN_PLANE);
	}

	static void load_library()
	{
		m_module = LoadLibraryA("opengl32.dll");
	}

private:

	int setupPixelFormat(HDC hDC)
	{
		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),  /* size */
			1,                              /* version */
			PFD_SUPPORT_OPENGL |
			PFD_DRAW_TO_WINDOW |
			PFD_DOUBLEBUFFER,               /* support double-buffering */
			PFD_TYPE_RGBA,                  /* color type */
			32,                             /* prefered color depth */
			0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
			0,                              /* no alpha buffer */
			0,                              /* alpha bits (ignored) */
			0,                              /* no accumulation buffer */
			0, 0, 0, 0,                     /* accum bits (ignored) */
			16,                             /* depth buffer */
			0,                              /* no stencil buffer */
			0,                              /* no auxiliary buffers */
			PFD_MAIN_PLANE,                 /* main layer */
			0,                              /* reserved */
			0, 0, 0,                        /* no layer, visible, damage masks */
		};
		int pixelFormat;

		pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0) {
			strcpy(m_error, "ChoosePixelFormat failed");
			return 1;
		}

		if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
			strcpy(m_error, "SetPixelFormat failed.");
			return 1;
		}
		return 0;
	}

	bool WGLExtensionSupported(const char *extension_name)
	{
		//if (pfn_wglGetExtensionsStringEXT != NULL)
		//  printf("%s\n", this->pfn_wglGetExtensionsStringEXT());

		if (pfn_wglGetExtensionsStringEXT != NULL && strstr(pfn_wglGetExtensionsStringEXT(), extension_name) != NULL)
			return true;
		else
			return false;
	}

	HGLRC m_context;
	HWND m_window;
	HDC m_hdc;
	char m_error[256];

	PROC (WINAPI *pfn_wglGetProcAddress)(LPCSTR lpszProc);
	HGLRC (WINAPI *pfn_wglCreateContext)(HDC hdc);
	BOOL (WINAPI *pfn_wglDeleteContext)(HGLRC hglrc);
	BOOL (WINAPI *pfn_wglMakeCurrent)(HDC hdc, HGLRC hglrc);

	const char *(WINAPI *pfn_wglGetExtensionsStringEXT) (void);
	BOOL (WINAPI *pfn_wglSwapIntervalEXT) (int interval);
	int (WINAPI * pfn_wglGetSwapIntervalEXT) (void);

	static HMODULE m_module;
};

HMODULE win_gl_context::m_module;


#elif SDLMAME_SDL2

class sdl_gl_context : public osd_gl_context
{
public:
	sdl_gl_context(SDL_Window *window) : osd_gl_context(), m_context(0), m_window(window)
	{
		m_error[0] = 0;
		m_context = SDL_GL_CreateContext(window);
		if  (!m_context)
		{
			snprintf(m_error,255, "OpenGL not supported on this driver: %s", SDL_GetError());
		}
	}
	virtual ~sdl_gl_context()
	{
		SDL_GL_DeleteContext(m_context);
	}
	virtual void MakeCurrent() override
	{
		SDL_GL_MakeCurrent(m_window, m_context);
	}

	virtual int SetSwapInterval(const int swap) override
	{
		return SDL_GL_SetSwapInterval(swap);
	}

	virtual const char *LastErrorMsg() override
	{
		if (m_error[0] == 0)
			return NULL;
		else
			return m_error;
	}
	virtual void *getProcAddress(const char *proc) override
	{
		return SDL_GL_GetProcAddress(proc);
	}

	virtual void SwapBuffer() override
	{
		SDL_GL_SwapWindow(m_window);
	}

private:
	SDL_GLContext m_context;
	SDL_Window *m_window;
	char m_error[256];
};

#else
// SDL 1.2
class sdl12_gl_context : public osd_gl_context
{
public:
	sdl12_gl_context(SDL_Surface *window) : osd_gl_context(), m_window(window)
	{
		m_error[0] = 0;
	}
	virtual ~sdl12_gl_context()
	{
	}
	virtual void MakeCurrent()
	{
	}

	virtual int SetSwapInterval(const int swap)
	{
		// Not supported on 1.2
		return 0;
	}

	virtual const char *LastErrorMsg()
	{
		if (m_error[0] == 0)
			return NULL;
		else
			return m_error;
	}

	virtual void *getProcAddress(const char *proc)
	{
		return SDL_GL_GetProcAddress(proc);
	}

	virtual void SwapBuffer()
	{
		SDL_GL_SwapBuffers();
	}

private:
	SDL_Surface *m_window;
	char m_error[256];
};

#endif

//============================================================
//  Textures
//============================================================

/* ogl_texture_info holds information about a texture */
class ogl_texture_info
{
public:
	ogl_texture_info()
	:   hash(0), flags(0), rawwidth(0), rawheight(0),
		rawwidth_create(0), rawheight_create(0),
		type(0), format(0), borderpix(0), xprescale(0), yprescale(0), nocopy(0),
		texture(0), texTarget(0), texpow2(0), mpass_dest_idx(0), pbo(0), data(NULL),
		data_own(0), texCoordBufferName(0)
	{
		for (int i=0; i<2; i++)
		{
			mpass_textureunit[i] = 0;
			mpass_texture_mamebm[i] = 0;
			mpass_fbo_mamebm[i] = 0;
			mpass_texture_scrn[i] = 0;
			mpass_fbo_scrn[i] = 0;
		}
		for (int i=0; i<8; i++)
			texCoord[i] = 0.0f;
	}

	HashT               hash;               // hash value for the texture (must be >= pointer size)
	UINT32              flags;              // rendering flags
	render_texinfo      texinfo;            // copy of the texture info
	int                 rawwidth, rawheight;    // raw width/height of the texture
	int                 rawwidth_create;    // raw width/height, pow2 compatible, if needed
	int                 rawheight_create;   // (create and initial set the texture, not for copy!)
	int                 type;               // what type of texture are we?
	int                 format;             // texture format
	int                 borderpix;          // do we have a 1 pixel border?
	int                 xprescale;          // what is our X prescale factor?
	int                 yprescale;          // what is our Y prescale factor?
	int                 nocopy;             // must the texture date be copied?

	UINT32              texture;            // OpenGL texture "name"/ID

	GLenum              texTarget;          // OpenGL texture target
	int                 texpow2;            // Is this texture pow2

	UINT32              mpass_dest_idx;         // Multipass dest idx [0..1]
	UINT32              mpass_textureunit[2];   // texture unit names for GLSL

	UINT32              mpass_texture_mamebm[2];// Multipass OpenGL texture "name"/ID for the shader
	UINT32              mpass_fbo_mamebm[2];    // framebuffer object for this texture, multipass
	UINT32              mpass_texture_scrn[2];  // Multipass OpenGL texture "name"/ID for the shader
	UINT32              mpass_fbo_scrn[2];      // framebuffer object for this texture, multipass

	UINT32              pbo;                    // pixel buffer object for this texture (DYNAMIC only!)
	UINT32              *data;                  // pixels for the texture
	int                 data_own;               // do we own / allocated it ?
	GLfloat             texCoord[8];
	GLuint              texCoordBufferName;

};

/* sdl_info is the information about SDL for the current screen */
class sdl_info_ogl : public osd_renderer
{
public:
	sdl_info_ogl(osd_window *window)
	: osd_renderer(window, FLAG_NEEDS_OPENGL), m_blittimer(0),
		m_width(0), m_height(0),
		m_blit_dim(0, 0),
		m_gl_context(NULL),
		m_initialized(0),
		m_last_blendmode(0),
		m_texture_max_width(0),
		m_texture_max_height(0),
		m_texpoweroftwo(0),
		m_usevbo(0), m_usepbo(0), m_usefbo(0), m_useglsl(0), m_glsl(NULL),
		m_glsl_program_num(0),
		m_glsl_program_mb2sc(0),
		m_usetexturerect(0),
		m_init_context(0),
		m_last_hofs(0.0f),
		m_last_vofs(0.0f),
		m_surf_w(0),
		m_surf_h(0)
	{
		for (int i=0; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			m_texhash[i] = NULL;
		for (int i=0; i < 2*GLSL_SHADER_MAX; i++)
			m_glsl_program[i] = 0;
		for (int i=0; i < 8; i++)
			m_texVerticex[i] = 0.0f;
	}

	virtual int create() override;
	virtual int draw(const int update) override;

#ifndef OSD_WINDOWS
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#endif
	virtual void destroy() override;
	virtual render_primitive_list *get_primitives() override
	{
#ifdef OSD_WINDOWS
		osd_dim nd = window().get_size();
#else
		osd_dim nd = window().blit_surface_size();
#endif
		if (nd != m_blit_dim)
		{
			m_blit_dim = nd;
			notify_changed();
		}
		window().target()->set_bounds(m_blit_dim.width(), m_blit_dim.height(), window().aspect());
		return &window().target()->get_primitives();
	}

#ifdef OSD_WINDOWS
	virtual void save() override { }
	virtual void record() override { }
	virtual void toggle_fsfx() override { }
#endif

private:
	void destroy_all_textures();

	void loadGLExtensions();
	void initialize_gl();
	void set_blendmode(int blendmode);
	void texture_compute_type_subroutine(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	void texture_compute_size_subroutine(ogl_texture_info *texture, UINT32 flags,
				UINT32 width, UINT32 height,
				int* p_width, int* p_height, int* p_width_create, int* p_height_create);
	void texture_compute_size_type(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	ogl_texture_info *texture_create(const render_texinfo *texsource, UINT32 flags);
	int texture_shader_create(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags);
	ogl_texture_info *texture_find(const render_primitive *prim);
	void texture_coord_update(ogl_texture_info *texture, const render_primitive *prim, int shaderIdx);
	void texture_mpass_flip(ogl_texture_info *texture, int shaderIdx);
	void texture_shader_update(ogl_texture_info *texture, render_container *container,  int shaderIdx);
	ogl_texture_info * texture_update(const render_primitive *prim, int shaderIdx);
	void texture_disable(ogl_texture_info * texture);
	void texture_all_disable();

	INT32           m_blittimer;
	int             m_width;
	int             m_height;
	osd_dim         m_blit_dim;

	osd_gl_context  *m_gl_context;

	int             m_initialized;        // is everything well initialized, i.e. all GL stuff etc.
	// 3D info (GL mode only)
	ogl_texture_info *  m_texhash[HASH_SIZE + OVERFLOW_SIZE];
	int             m_last_blendmode;     // previous blendmode
	INT32           m_texture_max_width;      // texture maximum width
	INT32           m_texture_max_height;     // texture maximum height
	int             m_texpoweroftwo;          // must textures be power-of-2 sized?
	int             m_usevbo;         // runtime check if VBO is available
	int             m_usepbo;         // runtime check if PBO is available
	int             m_usefbo;         // runtime check if FBO is available
	int             m_useglsl;        // runtime check if GLSL is available

	glsl_shader_info *m_glsl;             // glsl_shader_info

	GLhandleARB     m_glsl_program[2*GLSL_SHADER_MAX];  // GLSL programs, or 0
	int             m_glsl_program_num;   // number of GLSL programs
	int             m_glsl_program_mb2sc; // GLSL program idx, which transforms
										// the mame-bitmap. screen-bitmap (size/rotation/..)
										// All progs <= glsl_program_mb2sc using the mame bitmap
										// as input, otherwise the screen bitmap.
										// All progs >= glsl_program_mb2sc using the screen bitmap
										// as output, otherwise the mame bitmap.
	int             m_usetexturerect;     // use ARB_texture_rectangle for non-power-of-2, general use

	int             m_init_context;       // initialize context before next draw

	float           m_last_hofs;
	float           m_last_vofs;

	// Static vars from draogl_window_dra
	INT32           m_surf_w;
	INT32           m_surf_h;
	GLfloat         m_texVerticex[8];
};

/* line_aa_step is used for drawing antialiased lines */
struct line_aa_step
{
	float       xoffs, yoffs;               // X/Y deltas
	float       weight;                 // weight contribution
};

#if 0
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
#endif

//============================================================
//  INLINES
//============================================================

INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	HashT h = (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
	//printf("hash %d\n", (int) h % HASH_SIZE);
	return (h >> 8) % HASH_SIZE;
}

void sdl_info_ogl::set_blendmode(int blendmode)
{
	// try to minimize texture state changes
	if (blendmode != m_last_blendmode)
	{
		switch (blendmode)
		{
			case BLENDMODE_NONE:
				glDisable(GL_BLEND);
				break;
			case BLENDMODE_ALPHA:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BLENDMODE_RGB_MULTIPLY:
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				break;
			case BLENDMODE_ADD:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
		}

		m_last_blendmode = blendmode;
	}
}

//============================================================
//  PROTOTYPES
//============================================================

// core functions

//============================================================
//  STATIC VARIABLES
//============================================================

static void drawogl_exit(void);
static void load_gl_lib(running_machine &machine);



// OGL 1.3
#ifdef GL_ARB_multitexture
static PFNGLACTIVETEXTUREARBPROC pfn_glActiveTexture    = NULL;
#else
static PFNGLACTIVETEXTUREPROC pfn_glActiveTexture   = NULL;
#endif

// VBO
static PFNGLGENBUFFERSPROC pfn_glGenBuffers     = NULL;
static PFNGLDELETEBUFFERSPROC pfn_glDeleteBuffers   = NULL;
static PFNGLBINDBUFFERPROC pfn_glBindBuffer     = NULL;
static PFNGLBUFFERDATAPROC pfn_glBufferData     = NULL;
static PFNGLBUFFERSUBDATAPROC pfn_glBufferSubData   = NULL;

// PBO
static PFNGLMAPBUFFERPROC     pfn_glMapBuffer       = NULL;
static PFNGLUNMAPBUFFERPROC   pfn_glUnmapBuffer     = NULL;

// FBO
static PFNGLISFRAMEBUFFEREXTPROC   pfn_glIsFramebuffer          = NULL;
static PFNGLBINDFRAMEBUFFEREXTPROC pfn_glBindFramebuffer        = NULL;
static PFNGLDELETEFRAMEBUFFERSEXTPROC pfn_glDeleteFramebuffers      = NULL;
static PFNGLGENFRAMEBUFFERSEXTPROC pfn_glGenFramebuffers        = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC pfn_glCheckFramebufferStatus  = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC pfn_glFramebufferTexture2D  = NULL;

static int glsl_shader_feature = GLSL_SHADER_FEAT_PLAIN;

//============================================================
//  Textures
//============================================================

static void texture_set_data(ogl_texture_info *texture, const render_texinfo *texsource, UINT32 flags);

//============================================================
//  Static Variables
//============================================================

static int shown_video_info = 0;
static int dll_loaded = 0;

//============================================================
//  drawsdl_init
//============================================================

static osd_renderer *drawogl_create(osd_window *window)
{
	return global_alloc(sdl_info_ogl(window));
}

int drawogl_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawogl_exit;
	callbacks->create = drawogl_create;

	dll_loaded = 0;

	load_gl_lib(machine);
#if defined(OSD_WINDOWS)
	osd_printf_verbose("Using Windows OpenGL driver\n");
#elif SDLMAME_SDL2
	osd_printf_verbose("Using SDL multi-window OpenGL driver (SDL 2.0+)\n");
#else
	osd_printf_verbose("Using SDL single-window OpenGL driver (SDL 1.2)\n");
#endif

	return 0;
}

//============================================================
// Load the OGL function addresses
//============================================================

static void loadgl_functions(osd_gl_context *context)
{
#ifdef USE_DISPATCH_GL

	int err_count = 0;

	/* the following is tricky ... #func will be expanded to glBegin
	 * while func will be expanded to disp_p->glBegin
	 */

	#define OSD_GL(ret,func,params) \
	if (!( func = (ret (APIENTRY *)params) context->getProcAddress( #func ) )) \
		{ err_count++; osd_printf_error("GL function %s not found!\n", #func ); }

	#define OSD_GL_UNUSED(ret,func,params)

	#define GET_GLFUNC 1
	#include "modules/opengl/osd_opengl.h"
	#undef GET_GLFUNC

	if (err_count)
		fatalerror("Error loading GL library functions, giving up\n");

#endif
}

//============================================================
// Load GL library
//============================================================

#ifdef USE_DISPATCH_GL
osd_gl_dispatch *gl_dispatch;
#endif

static void load_gl_lib(running_machine &machine)
{
	if (!dll_loaded)
	{
#ifdef OSD_WINDOWS
		win_gl_context::load_library();
#else
#ifdef USE_DISPATCH_GL
		/*
		 *  directfb and and x11 use this env var
		 *   SDL_VIDEO_GL_DRIVER
		 */
		const char *stemp;

		stemp = downcast<sdl_options &>(machine.options()).gl_lib();
		if (stemp != NULL && strcmp(stemp, OSDOPTVAL_AUTO) == 0)
			stemp = NULL;

		if (SDL_GL_LoadLibrary(stemp) != 0) // Load library (default for e==NULL
		{
			fatalerror("Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
		}
		osd_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");
		/* FIXME: must be freed as well */
#endif
#endif
#ifdef USE_DISPATCH_GL
		gl_dispatch = (osd_gl_dispatch *) osd_malloc(sizeof(osd_gl_dispatch));
#endif
		dll_loaded=1;
	}
}

void sdl_info_ogl::initialize_gl()
{
	int has_and_allow_texturerect = 0;

	char *extstr = (char *)glGetString(GL_EXTENSIONS);
	char *vendor = (char *)glGetString(GL_VENDOR);

	//printf("%p\n", extstr);
#ifdef OSD_WINDOWS
	if (!extstr)
		extstr = (char *)"";
#endif
	// print out the driver info for debugging
	if (!shown_video_info)
	{
		osd_printf_verbose("OpenGL: %s\nOpenGL: %s\nOpenGL: %s\n", vendor, (char *)glGetString(GL_RENDERER), (char *)glGetString(GL_VERSION));
	}

	m_usetexturerect = 0;
	m_texpoweroftwo = 1;
	m_usevbo = 0;
	m_usepbo = 0;
	m_usefbo = 0;
	m_useglsl = 0;

	if ( video_config.allowtexturerect &&
			( strstr(extstr, "GL_ARB_texture_rectangle") ||  strstr(extstr, "GL_EXT_texture_rectangle") )
		)
	{
		has_and_allow_texturerect = 1;
					if (!shown_video_info)
					{
							osd_printf_verbose("OpenGL: texture rectangle supported\n");
					}
	}

	// does this card support non-power-of-two sized textures?  (they're faster, so use them if possible)
	if ( !video_config.forcepow2texture && strstr(extstr, "GL_ARB_texture_non_power_of_two"))
	{
		if (!shown_video_info)
		{
			osd_printf_verbose("OpenGL: non-power-of-2 textures supported (new method)\n");
		}
					m_texpoweroftwo = 0;
	}
	else
	{
		// second chance: GL_ARB_texture_rectangle or GL_EXT_texture_rectangle (old version)
		if (has_and_allow_texturerect)
		{
			if (!shown_video_info)
			{
				osd_printf_verbose("OpenGL: non-power-of-2 textures supported (old method)\n");
			}
			m_usetexturerect = 1;
		}
		else
		{
			if (!shown_video_info)
			{
				osd_printf_verbose("OpenGL: forcing power-of-2 textures (creation, not copy)\n");
			}
		}
	}

	if (strstr(extstr, "GL_ARB_vertex_buffer_object"))
	{
					m_usevbo = video_config.vbo;
		if (!shown_video_info)
		{
			if(m_usevbo)
				osd_printf_verbose("OpenGL: vertex buffer supported\n");
			else
				osd_printf_verbose("OpenGL: vertex buffer supported, but disabled\n");
		}
	}

	if (strstr(extstr, "GL_ARB_pixel_buffer_object"))
	{
		if( m_usevbo )
		{
			m_usepbo = video_config.pbo;
			if (!shown_video_info)
			{
				if(m_usepbo)
					osd_printf_verbose("OpenGL: pixel buffers supported\n");
				else
					osd_printf_verbose("OpenGL: pixel buffers supported, but disabled\n");
			}
		} else {
			if (!shown_video_info)
			{
				osd_printf_verbose("OpenGL: pixel buffers supported, but disabled due to disabled vbo\n");
			}
		}
	}
	else
	{
		if (!shown_video_info)
		{
			osd_printf_verbose("OpenGL: pixel buffers not supported\n");
		}
	}

	if (strstr(extstr, "GL_EXT_framebuffer_object"))
	{
		m_usefbo = 1;
		if (!shown_video_info)
		{
			if(m_usefbo)
				osd_printf_verbose("OpenGL: framebuffer object supported\n");
			else
				osd_printf_verbose("OpenGL: framebuffer object not supported\n");
		}
	}

	if (strstr(extstr, "GL_ARB_shader_objects") &&
		strstr(extstr, "GL_ARB_shading_language_100") &&
		strstr(extstr, "GL_ARB_vertex_shader") &&
		strstr(extstr, "GL_ARB_fragment_shader")
		)
	{
		m_useglsl = video_config.glsl;
		if (!shown_video_info)
		{
			if(m_useglsl)
				osd_printf_verbose("OpenGL: GLSL supported\n");
			else
				osd_printf_verbose("OpenGL: GLSL supported, but disabled\n");
		}
	} else {
		if (!shown_video_info)
		{
			osd_printf_verbose("OpenGL: GLSL not supported\n");
		}
	}

#ifdef TOBEMIGRATED
	if (osd_getenv(SDLENV_VMWARE) != NULL)
	{
		m_usetexturerect = 1;
		m_texpoweroftwo = 1;
	}
#endif
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&m_texture_max_width);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&m_texture_max_height);
	if (!shown_video_info)
	{
		osd_printf_verbose("OpenGL: max texture size %d x %d\n", m_texture_max_width, m_texture_max_height);
	}

	shown_video_info = 1;

}
//============================================================
//  sdl_info::create
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
// a
//============================================================

int sdl_info_ogl::create()
{
	// create renderer
#if defined(OSD_WINDOWS)
	m_gl_context = global_alloc(win_gl_context(window().m_hwnd));
#elif SDLMAME_SDL2
	m_gl_context = global_alloc(sdl_gl_context(window().sdl_window()));
#else
	m_gl_context = global_alloc(sdl12_gl_context(window().sdl_surface()));
#endif
	if  (m_gl_context->LastErrorMsg() != NULL)
	{
		osd_printf_error("%s\n", m_gl_context->LastErrorMsg());
		return 1;
	}
	m_gl_context->SetSwapInterval(video_config.waitvsync ? 1 : 0);


	m_blittimer = 0;
	m_surf_w = 0;
	m_surf_h = 0;

	m_initialized = 0;

	// in case any textures try to come up before these are validated,
	// OpenGL guarantees all implementations can handle something this size.
	m_texture_max_width = 64;
	m_texture_max_height = 64;

	/* load any GL function addresses
	 * this must be done here because we need a context
	 */
	loadgl_functions(m_gl_context);
	initialize_gl();


	m_init_context = 0;

	osd_printf_verbose("Leave sdl_info_ogl::create\n");
	return 0;
}


//============================================================
//  sdl_info::destroy
//============================================================

void sdl_info_ogl::destroy()
{
	// free the memory in the window

	destroy_all_textures();

	global_free(m_gl_context);
	m_gl_context = NULL;
}


//============================================================
//  drawsdl_xy_to_render_target
//============================================================
#ifndef OSD_WINDOWS
int sdl_info_ogl::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= m_blit_dim.width())
		return 0;
	if (*yt<0 || *yt >= m_blit_dim.height())
		return 0;
	return 1;
}
#endif
//============================================================
//  drawsdl_destroy_all_textures
//============================================================

void sdl_info_ogl::destroy_all_textures()
{
	ogl_texture_info *texture = NULL;
	int lock=FALSE;
	int i;

	if ( !m_initialized )
		return;

	m_gl_context->MakeCurrent();

	if(window().m_primlist)
	{
		lock=TRUE;
		window().m_primlist->acquire_lock();
	}

	glFinish();

	texture_all_disable();
	glFinish();
	glDisableClientState(GL_VERTEX_ARRAY);

	i=0;
	while (i<HASH_SIZE+OVERFLOW_SIZE)
	{
		texture = m_texhash[i];
		m_texhash[i] = NULL;
		if (texture != NULL)
		{
			if(m_usevbo)
			{
				pfn_glDeleteBuffers( 1, &(texture->texCoordBufferName) );
				texture->texCoordBufferName=0;
			}

			if(m_usepbo && texture->pbo)
			{
				pfn_glDeleteBuffers( 1, (GLuint *)&(texture->pbo) );
				texture->pbo=0;
			}

			if( m_glsl_program_num > 1 )
			{
				assert(m_usefbo);
				pfn_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_mamebm[0]);
				glDeleteTextures(2, (GLuint *)&texture->mpass_texture_mamebm[0]);
			}

			if ( m_glsl_program_mb2sc < m_glsl_program_num - 1 )
			{
				assert(m_usefbo);
				pfn_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_scrn[0]);
				glDeleteTextures(2, (GLuint *)&texture->mpass_texture_scrn[0]);
			}

			glDeleteTextures(1, (GLuint *)&texture->texture);
			if ( texture->data_own )
			{
				free(texture->data);
				texture->data=NULL;
				texture->data_own=FALSE;
			}
			global_free(texture);
		}
		i++;
	}
	if ( m_useglsl )
	{
		glsl_shader_free(m_glsl);
		m_glsl = NULL;
	}

	m_initialized = 0;

	if (lock)
		window().m_primlist->release_lock();
}
//============================================================
//  loadGLExtensions
//============================================================

void sdl_info_ogl::loadGLExtensions()
{
	static int _once = 1;

	// usevbo=FALSE; // You may want to switch VBO and PBO off, by uncommenting this statement
	// usepbo=FALSE; // You may want to switch PBO off, by uncommenting this statement
	// useglsl=FALSE; // You may want to switch GLSL off, by uncommenting this statement

	if (! m_usevbo)
	{
		if(m_usepbo) // should never ever happen ;-)
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: PBO not supported, no VBO support. (sdlmame error)\n");
			}
			m_usepbo=FALSE;
		}
		if(m_useglsl) // should never ever happen ;-)
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL not supported, no VBO support. (sdlmame error)\n");
			}
			m_useglsl=FALSE;
		}
	}

	// Get Pointers To The GL Functions
	// VBO:
	if( m_usevbo )
	{
		pfn_glGenBuffers = (PFNGLGENBUFFERSPROC) m_gl_context->getProcAddress("glGenBuffers");
		pfn_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) m_gl_context->getProcAddress("glDeleteBuffers");
		pfn_glBindBuffer = (PFNGLBINDBUFFERPROC) m_gl_context->getProcAddress("glBindBuffer");
		pfn_glBufferData = (PFNGLBUFFERDATAPROC) m_gl_context->getProcAddress("glBufferData");
		pfn_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) m_gl_context->getProcAddress("glBufferSubData");
	}
	// PBO:
	if ( m_usepbo )
	{
		pfn_glMapBuffer  = (PFNGLMAPBUFFERPROC) m_gl_context->getProcAddress("glMapBuffer");
		pfn_glUnmapBuffer= (PFNGLUNMAPBUFFERPROC) m_gl_context->getProcAddress("glUnmapBuffer");
	}
	// FBO:
	if ( m_usefbo )
	{
		pfn_glIsFramebuffer = (PFNGLISFRAMEBUFFEREXTPROC) m_gl_context->getProcAddress("glIsFramebufferEXT");
		pfn_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC) m_gl_context->getProcAddress("glBindFramebufferEXT");
		pfn_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC) m_gl_context->getProcAddress("glDeleteFramebuffersEXT");
		pfn_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC) m_gl_context->getProcAddress("glGenFramebuffersEXT");
		pfn_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) m_gl_context->getProcAddress("glCheckFramebufferStatusEXT");
		pfn_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) m_gl_context->getProcAddress("glFramebufferTexture2DEXT");
	}

	if ( m_usevbo &&
			( !pfn_glGenBuffers || !pfn_glDeleteBuffers ||
			!pfn_glBindBuffer || !pfn_glBufferData || !pfn_glBufferSubData
		) )
	{
		m_usepbo=FALSE;
		if (_once)
		{
			osd_printf_warning("OpenGL: VBO not supported, missing: ");
			if (!pfn_glGenBuffers)
			{
				osd_printf_warning("glGenBuffers, ");
			}
			if (!pfn_glDeleteBuffers)
			{
				osd_printf_warning("glDeleteBuffers");
			}
			if (!pfn_glBindBuffer)
			{
				osd_printf_warning("glBindBuffer, ");
			}
			if (!pfn_glBufferData)
			{
				osd_printf_warning("glBufferData, ");
			}
			if (!pfn_glBufferSubData)
			{
				osd_printf_warning("glBufferSubData, ");
			}
			osd_printf_warning("\n");
		}
		if ( m_usevbo )
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: PBO not supported, no VBO support.\n");
			}
			m_usepbo=FALSE;
		}
	}

	if ( m_usepbo && ( !pfn_glMapBuffer || !pfn_glUnmapBuffer ) )
	{
		m_usepbo=FALSE;
		if (_once)
		{
			osd_printf_warning("OpenGL: PBO not supported, missing: ");
			if (!pfn_glMapBuffer)
			{
				osd_printf_warning("glMapBuffer, ");
			}
			if (!pfn_glUnmapBuffer)
			{
				osd_printf_warning("glUnmapBuffer, ");
			}
			osd_printf_warning("\n");
		}
	}

	if ( m_usefbo &&
		( !pfn_glIsFramebuffer || !pfn_glBindFramebuffer || !pfn_glDeleteFramebuffers ||
			!pfn_glGenFramebuffers || !pfn_glCheckFramebufferStatus || !pfn_glFramebufferTexture2D
		))
	{
		m_usefbo=FALSE;
		if (_once)
		{
			osd_printf_warning("OpenGL: FBO not supported, missing: ");
			if (!pfn_glIsFramebuffer)
			{
				osd_printf_warning("pfn_glIsFramebuffer, ");
			}
			if (!pfn_glBindFramebuffer)
			{
				osd_printf_warning("pfn_glBindFramebuffer, ");
			}
			if (!pfn_glDeleteFramebuffers)
			{
				osd_printf_warning("pfn_glDeleteFramebuffers, ");
			}
			if (!pfn_glGenFramebuffers)
			{
				osd_printf_warning("pfn_glGenFramebuffers, ");
			}
			if (!pfn_glCheckFramebufferStatus)
			{
				osd_printf_warning("pfn_glCheckFramebufferStatus, ");
			}
			if (!pfn_glFramebufferTexture2D)
			{
				osd_printf_warning("pfn_glFramebufferTexture2D, ");
			}
			osd_printf_warning("\n");
		}
	}

	if (_once)
	{
		if ( m_usevbo )
		{
			osd_printf_verbose("OpenGL: VBO supported\n");
		}
		else
		{
			osd_printf_warning("OpenGL: VBO not supported\n");
		}

		if ( m_usepbo )
		{
			osd_printf_verbose("OpenGL: PBO supported\n");
		}
		else
		{
			osd_printf_warning("OpenGL: PBO not supported\n");
		}

		if ( m_usefbo )
		{
			osd_printf_verbose("OpenGL: FBO supported\n");
		}
		else
		{
			osd_printf_warning("OpenGL: FBO not supported\n");
		}
	}

	if ( m_useglsl )
	{
		#ifdef GL_ARB_multitexture
		pfn_glActiveTexture = (PFNGLACTIVETEXTUREARBPROC) m_gl_context->getProcAddress("glActiveTextureARB");
		#else
		pfn_glActiveTexture = (PFNGLACTIVETEXTUREPROC) m_gl_context->getProcAddress("glActiveTexture");
		#endif
		if (!pfn_glActiveTexture)
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL disabled, glActiveTexture(ARB) not supported\n");
			}
			m_useglsl = 0;
		}
	}

	if ( m_useglsl )
	{
		m_glsl = glsl_shader_init(m_gl_context);
		m_useglsl = (m_glsl != NULL ? 1 : 0);

		if ( ! m_useglsl )
		{
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL supported, but shader instantiation failed - disabled\n");
			}
		}
	}

	if ( m_useglsl )
	{
		if ( window().prescale() != 1 )
		{
			m_useglsl = 0;
			if (_once)
			{
				osd_printf_warning("OpenGL: GLSL supported, but disabled due to: prescale !=1 \n");
			}
		}
	}

	if ( m_useglsl )
	{
		int i;
		video_config.filter = FALSE;
		glsl_shader_feature = GLSL_SHADER_FEAT_PLAIN;
		m_glsl_program_num = 0;
		m_glsl_program_mb2sc = 0;

		for(i=0; i<video_config.glsl_shader_mamebm_num; i++)
		{
			if ( !m_usefbo && m_glsl_program_num==1 )
			{
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL multipass not supported, due to unsupported FBO. Skipping followup shader\n");
				}
				break;
			}

			if ( glsl_shader_add_mamebm(m_glsl, video_config.glsl_shader_mamebm[i], m_glsl_program_num) )
			{
				osd_printf_error("OpenGL: GLSL loading mame bitmap shader %d failed (%s)\n",
					i, video_config.glsl_shader_mamebm[i]);
			} else {
				glsl_shader_feature = GLSL_SHADER_FEAT_CUSTOM;
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL using mame bitmap shader filter %d: '%s'\n",
						m_glsl_program_num, video_config.glsl_shader_mamebm[i]);
				}
				m_glsl_program_mb2sc = m_glsl_program_num; // the last mame_bitmap (mb) shader does it.
				m_glsl_program_num++;
			}
		}

		if ( video_config.glsl_shader_scrn_num > 0 && m_glsl_program_num==0 )
		{
			osd_printf_verbose("OpenGL: GLSL cannot use screen bitmap shader without bitmap shader\n");
		}

		for(i=0; m_usefbo && m_glsl_program_num>0 && i<video_config.glsl_shader_scrn_num; i++)
		{
			if ( glsl_shader_add_scrn(m_glsl, video_config.glsl_shader_scrn[i],
											m_glsl_program_num-1-m_glsl_program_mb2sc) )
			{
				osd_printf_error("OpenGL: GLSL loading screen bitmap shader %d failed (%s)\n",
					i, video_config.glsl_shader_scrn[i]);
			} else {
				if (_once)
				{
					osd_printf_verbose("OpenGL: GLSL using screen bitmap shader filter %d: '%s'\n",
						m_glsl_program_num, video_config.glsl_shader_scrn[i]);
				}
				m_glsl_program_num++;
			}
		}

		if ( 0==m_glsl_program_num &&
				0 <= video_config.glsl_filter && video_config.glsl_filter < GLSL_SHADER_FEAT_INT_NUMBER )
		{
			m_glsl_program_mb2sc = m_glsl_program_num; // the last mame_bitmap (mb) shader does it.
			m_glsl_program_num++;
			glsl_shader_feature = video_config.glsl_filter;

			if (_once)
			{
				osd_printf_verbose("OpenGL: GLSL using shader filter '%s', idx: %d, num %d (vid filter: %d)\n",
					glsl_shader_get_filter_name_mamebm(glsl_shader_feature),
					glsl_shader_feature, m_glsl_program_num, video_config.filter);
			}
		}

	} else {
		if (_once)
		{
			osd_printf_verbose("OpenGL: using vid filter: %d\n", video_config.filter);
		}
	}

	_once = 0;
}

//============================================================
//  sdl_info::draw
//============================================================

int sdl_info_ogl::draw(const int update)
{
	render_primitive *prim;
	ogl_texture_info *texture=NULL;
	float vofs, hofs;
	int  pendingPrimitive=GL_NO_PRIMITIVE, curPrimitive=GL_NO_PRIMITIVE;

#ifdef TOBEMIGRATED
	if (video_config.novideo)
	{
		return 0;
	}
#endif

	osd_dim wdim = window().get_size();

	if (has_flags(FI_CHANGED) || (wdim.width() != m_width) || (wdim.height() != m_height))
	{
		destroy_all_textures();
		m_width = wdim.width();
		m_height = wdim.height();
		m_blittimer = 3;
		m_init_context = 1;
		clear_flags(FI_CHANGED);
	}

	m_gl_context->MakeCurrent();

	if (m_init_context)
	{
		// do some one-time OpenGL setup
#if SDLMAME_SDL2
		// FIXME: SRGB conversion is working on SDL2, may be of use
		// when we eventually target gamma and monitor profiles.
		//glEnable(GL_FRAMEBUFFER_SRGB);
#endif
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}

	// only clear if the geometry changes (and for 2 frames afterward to clear double and triple buffers)
	if ((m_blittimer > 0) || has_flags(FLAG_HAS_VECTOR_SCREEN))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		m_blittimer--;
	}

	// FIXME: remove m_surf_w and m_surf_h
	if ( !m_initialized ||
			m_width != m_surf_w || m_height != m_surf_h )
	{
		// FIXME:: this can be done in create!
		if ( !m_initialized )
		{
			loadGLExtensions();
		}

#if !defined(OSD_WINDOWS) && !SDLMAME_SDL2
		// force all textures to be regenerated
		destroy_all_textures();
#endif

		m_surf_w = m_width;
		m_surf_h = m_height;

		// we're doing nothing 3d, so the Z-buffer is currently not interesting
		glDisable(GL_DEPTH_TEST);

		if (window().machine().options().antialias())
		{
			// enable antialiasing for lines
			glEnable(GL_LINE_SMOOTH);
			// enable antialiasing for points
			glEnable(GL_POINT_SMOOTH);

			// prefer quality to speed
			glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		}
		else
		{
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POINT_SMOOTH);
		}

		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		m_last_blendmode = BLENDMODE_ALPHA;

#ifdef TOBEMIGRATED
		// set lines and points just barely above normal size to get proper results
		glLineWidth(video_config.beamwidth);
		glPointSize(video_config.beamwidth);
#endif

		// set up a nice simple 2D coordinate system, so GL behaves exactly how we'd like.
		//
		// (0,0)     (w,0)
		//   |~~~~~~~~~|
		//   |         |
		//   |         |
		//   |         |
		//   |_________|
		// (0,h)     (w,h)

		glViewport(0.0, 0.0, (GLsizei) m_width, (GLsizei) m_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (GLdouble) m_width, (GLdouble) m_height, 0.0, 0.0, -1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if ( ! m_initialized )
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, m_texVerticex); // no VBO, since it's too volatile

			m_initialized = 1;
		}
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

#ifdef TOBEMIGRATED
	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		ch = m_height;
		cw = m_width;

		if (video_config.centerv)
		{
			vofs = (ch - m_blit_dim.height()) / 2.0f;
		}
		if (video_config.centerh)
		{
			hofs = (cw - m_blit_dim.width()) / 2.0f;
		}
	}
#else
#endif

	m_last_hofs = hofs;
	m_last_vofs = vofs;

	window().m_primlist->acquire_lock();

	// now draw
	for (prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		int i;

		switch (prim->type)
		{
			/**
			 * Try to stay in one Begin/End block as long as possible,
			 * since entering and leaving one is most expensive..
			 */
			case render_primitive::LINE:
				#if !USE_WIN32_STYLE_LINES
				// check if it's really a point
				if (((prim->bounds.x1 - prim->bounds.x0) == 0) && ((prim->bounds.y1 - prim->bounds.y0) == 0))
				{
					curPrimitive=GL_POINTS;
				} else {
					curPrimitive=GL_LINES;
				}

				if(pendingPrimitive!=GL_NO_PRIMITIVE && pendingPrimitive!=curPrimitive)
				{
					glEnd();
					pendingPrimitive=GL_NO_PRIMITIVE;
				}

						if ( pendingPrimitive==GL_NO_PRIMITIVE )
				{
							set_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags));
				}

				glColor4f(prim->color.r, prim->color.g, prim->color.b, prim->color.a);

				if(pendingPrimitive!=curPrimitive)
				{
					glBegin(curPrimitive);
					pendingPrimitive=curPrimitive;
				}

				// check if it's really a point
				if (curPrimitive==GL_POINTS)
				{
					glVertex2f(prim->bounds.x0+hofs, prim->bounds.y0+vofs);
				}
				else
				{
					glVertex2f(prim->bounds.x0+hofs, prim->bounds.y0+vofs);
					glVertex2f(prim->bounds.x1+hofs, prim->bounds.y1+vofs);
				}
				#else
				{
					const line_aa_step *step = line_aa_4step;
					render_bounds b0, b1;
					float r, g, b, a;
					float effwidth;

					// we're not gonna play fancy here.  close anything pending and let's go.
					if (pendingPrimitive!=GL_NO_PRIMITIVE && pendingPrimitive!=curPrimitive)
					{
						glEnd();
						pendingPrimitive=GL_NO_PRIMITIVE;
					}

					set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim->flags));

					// compute the effective width based on the direction of the line
					effwidth = prim->width();
					if (effwidth < 0.5f)
						effwidth = 0.5f;

					// determine the bounds of a quad to draw this line
					render_line_to_quad(&prim->bounds, effwidth, &b0, &b1);

					// fix window position
					b0.x0 += hofs;
					b0.x1 += hofs;
					b1.x0 += hofs;
					b1.x1 += hofs;
					b0.y0 += vofs;
					b0.y1 += vofs;
					b1.y0 += vofs;
					b1.y1 += vofs;

					// iterate over AA steps
					for (step = PRIMFLAG_GET_ANTIALIAS(prim->flags) ? line_aa_4step : line_aa_1step; step->weight != 0; step++)
					{
						glBegin(GL_TRIANGLE_STRIP);

						// rotate the unit vector by 135 degrees and add to point 0
						glVertex2f(b0.x0 + step->xoffs, b0.y0 + step->yoffs);

						// rotate the unit vector by -135 degrees and add to point 0
						glVertex2f(b0.x1 + step->xoffs, b0.y1 + step->yoffs);

						// rotate the unit vector by 45 degrees and add to point 1
						glVertex2f(b1.x0 + step->xoffs, b1.y0 + step->yoffs);

						// rotate the unit vector by -45 degrees and add to point 1
						glVertex2f(b1.x1 + step->xoffs, b1.y1 + step->yoffs);

						// determine the color of the line
						r = (prim->color.r * step->weight);
						g = (prim->color.g * step->weight);
						b = (prim->color.b * step->weight);
						a = (prim->color.a * 255.0f);
						if (r > 1.0) r = 1.0;
						if (g > 1.0) g = 1.0;
						if (b > 1.0) b = 1.0;
						if (a > 1.0) a = 1.0;
						glColor4f(r, g, b, a);

//                      texture = texture_update(window, prim, 0);
//                      if (texture) printf("line has texture!\n");

						// if we have a texture to use for the vectors, use it here
						#if 0
						if (d3d->vector_texture != NULL)
						{
							printf("SDL: textured lines unsupported\n");
							vertex[0].u0 = d3d->vector_texture->ustart;
							vertex[0].v0 = d3d->vector_texture->vstart;

							vertex[2].u0 = d3d->vector_texture->ustop;
							vertex[2].v0 = d3d->vector_texture->vstart;

							vertex[1].u0 = d3d->vector_texture->ustart;
							vertex[1].v0 = d3d->vector_texture->vstop;

							vertex[3].u0 = d3d->vector_texture->ustop;
							vertex[3].v0 = d3d->vector_texture->vstop;
						}
						#endif
						glEnd();
					}
				}
				#endif
				break;

			case render_primitive::QUAD:

				if(pendingPrimitive!=GL_NO_PRIMITIVE)
				{
					glEnd();
					pendingPrimitive=GL_NO_PRIMITIVE;
				}

				glColor4f(prim->color.r, prim->color.g, prim->color.b, prim->color.a);

				set_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags));

				texture = texture_update(prim, 0);

				if ( texture && texture->type==TEXTURE_TYPE_SHADER )
				{
					for(i=0; i<m_glsl_program_num; i++)
					{
						if ( i==m_glsl_program_mb2sc )
						{
							// i==glsl_program_mb2sc -> transformation mamebm->scrn
							m_texVerticex[0]=prim->bounds.x0 + hofs;
							m_texVerticex[1]=prim->bounds.y0 + vofs;
							m_texVerticex[2]=prim->bounds.x1 + hofs;
							m_texVerticex[3]=prim->bounds.y0 + vofs;
							m_texVerticex[4]=prim->bounds.x1 + hofs;
							m_texVerticex[5]=prim->bounds.y1 + vofs;
							m_texVerticex[6]=prim->bounds.x0 + hofs;
							m_texVerticex[7]=prim->bounds.y1 + vofs;
						} else {
							// 1:1 tex coord CCW (0/0) (1/0) (1/1) (0/1) on texture dimensions
							m_texVerticex[0]=(GLfloat)0.0;
							m_texVerticex[1]=(GLfloat)0.0;
							m_texVerticex[2]=(GLfloat)m_width;
							m_texVerticex[3]=(GLfloat)0.0;
							m_texVerticex[4]=(GLfloat)m_width;
							m_texVerticex[5]=(GLfloat)m_height;
							m_texVerticex[6]=(GLfloat)0.0;
							m_texVerticex[7]=(GLfloat)m_height;
						}

						if(i>0) // first fetch already done
						{
							texture = texture_update(prim, i);
						}
						glDrawArrays(GL_QUADS, 0, 4);
					}
				} else {
					m_texVerticex[0]=prim->bounds.x0 + hofs;
					m_texVerticex[1]=prim->bounds.y0 + vofs;
					m_texVerticex[2]=prim->bounds.x1 + hofs;
					m_texVerticex[3]=prim->bounds.y0 + vofs;
					m_texVerticex[4]=prim->bounds.x1 + hofs;
					m_texVerticex[5]=prim->bounds.y1 + vofs;
					m_texVerticex[6]=prim->bounds.x0 + hofs;
					m_texVerticex[7]=prim->bounds.y1 + vofs;

					glDrawArrays(GL_QUADS, 0, 4);
				}

				if ( texture )
				{
					texture_disable(texture);
					texture=NULL;
				}
				break;

			default:
				throw emu_fatalerror("Unexpected render_primitive type");
		}
	}

	if(pendingPrimitive!=GL_NO_PRIMITIVE)
	{
		glEnd();
		pendingPrimitive=GL_NO_PRIMITIVE;
	}

	window().m_primlist->release_lock();
	m_init_context = 0;

	m_gl_context->SwapBuffer();

	return 0;
}

//============================================================
//  texture handling
//============================================================

static const char * texfmt_to_string[9] = {
		"ARGB32",
		"RGB32",
		"RGB32_PALETTED",
		"YUV16",
		"YUV16_PALETTED",
		"PALETTE16",
		"RGB15",
		"RGB15_PALETTE",
		"PALETTE16A"
		};

//
// Note: if you change the following array order, change the matching defines in texsrc.h
//

enum { SDL_TEXFORMAT_SRC_EQUALS_DEST, SDL_TEXFORMAT_SRC_HAS_PALETTE };

static const GLint texture_copy_properties[9][2] = {
	{ TRUE,  FALSE },   // SDL_TEXFORMAT_ARGB32
	{ TRUE,  FALSE },   // SDL_TEXFORMAT_RGB32
	{ TRUE,  TRUE  },   // SDL_TEXFORMAT_RGB32_PALETTED
	{ FALSE, FALSE },   // SDL_TEXFORMAT_YUY16
	{ FALSE, TRUE  },   // SDL_TEXFORMAT_YUY16_PALETTED
	{ FALSE, TRUE  },   // SDL_TEXFORMAT_PALETTE16
	{ TRUE,  FALSE },   // SDL_TEXFORMAT_RGB15
	{ TRUE,  TRUE  },   // SDL_TEXFORMAT_RGB15_PALETTED
	{ FALSE, TRUE  }    // SDL_TEXFORMAT_PALETTE16A
};

//============================================================
//  drawogl_exit
//============================================================

static void drawogl_exit(void)
{
	int i;

	for(i=0; i<video_config.glsl_shader_mamebm_num; i++)
	{
		if ( NULL!=video_config.glsl_shader_mamebm[i])
		{
			free(video_config.glsl_shader_mamebm[i]);
			video_config.glsl_shader_mamebm[i] = NULL;
		}
	}
	for(i=0; i<video_config.glsl_shader_scrn_num; i++)
	{
		if ( NULL!=video_config.glsl_shader_scrn[i])
		{
			free(video_config.glsl_shader_scrn[i]);
			video_config.glsl_shader_scrn[i] = NULL;
		}
	}
}

//============================================================
//  texture_compute_size and type
//============================================================

//
// glBufferData to push a nocopy texture to the GPU is slower than TexSubImage2D,
// so don't use PBO here
//
// we also don't want to use PBO's in the case of nocopy==TRUE,
// since we now might have GLSL shaders - this decision simplifies out life ;-)
//
void sdl_info_ogl::texture_compute_type_subroutine(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags)
{
	texture->type = TEXTURE_TYPE_NONE;
	texture->nocopy = FALSE;

	if ( texture->type == TEXTURE_TYPE_NONE &&
			!PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->type = TEXTURE_TYPE_PLAIN;
				texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
				texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}

	if ( texture->type == TEXTURE_TYPE_NONE && m_useglsl &&
			texture->xprescale == 1 && texture->yprescale == 1 &&
			texsource->rowpixels <= m_texture_max_width )
		{
			texture->type      = TEXTURE_TYPE_SHADER;
			texture->texTarget = GL_TEXTURE_2D;
			texture->texpow2   = m_texpoweroftwo;
		}

	// determine if we can skip the copy step
	// if this was not already decided by the shader condition above
	if    ( texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST] &&
			!texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE] &&
			texture->xprescale == 1 && texture->yprescale == 1 &&
			!texture->borderpix && !texsource->palette &&
			texsource->rowpixels <= m_texture_max_width )
	{
		texture->nocopy = TRUE;
	}

	if( texture->type == TEXTURE_TYPE_NONE &&
		m_usepbo && !texture->nocopy )
	{
		texture->type      = TEXTURE_TYPE_DYNAMIC;
		texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
		texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}

	if( texture->type == TEXTURE_TYPE_NONE )
	{
		texture->type      = TEXTURE_TYPE_SURFACE;
		texture->texTarget = (m_usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
		texture->texpow2   = (m_usetexturerect)?0:m_texpoweroftwo;
	}
}

INLINE int get_valid_pow2_value(int v, int needPow2)
{
	return (needPow2)?gl_round_to_pow2(v):v;
}

void sdl_info_ogl::texture_compute_size_subroutine(ogl_texture_info *texture, UINT32 flags,
											UINT32 width, UINT32 height,
											int* p_width, int* p_height, int* p_width_create, int* p_height_create)
{
	int width_create;
	int height_create;

	if ( texture->texpow2 )
		{
				width_create  = gl_round_to_pow2 (width);
				height_create = gl_round_to_pow2 (height);
		} else if ( texture->type==TEXTURE_TYPE_SHADER )
		{
				/**
				 * at least use a multiple of 8 for shader .. just in case
				 */
				width_create  = ( width  & ~0x07 ) + ( (width  & 0x07)? 8 : 0 ) ;
				height_create = ( height & ~0x07 ) + ( (height & 0x07)? 8 : 0 ) ;
		} else {
				width_create  = width  ;
				height_create = height ;
		}

	// don't prescale above max texture size
	while (texture->xprescale > 1 && width_create * texture->xprescale > m_texture_max_width)
		texture->xprescale--;
	while (texture->yprescale > 1 && height_create * texture->yprescale > m_texture_max_height)
		texture->yprescale--;
	if (PRIMFLAG_GET_SCREENTEX(flags) && (texture->xprescale != window().prescale() || texture->yprescale != window().prescale()))
		osd_printf_warning("SDL: adjusting prescale from %dx%d to %dx%d\n", window().prescale(), window().prescale(), texture->xprescale, texture->yprescale);

	width  *= texture->xprescale;
	height *= texture->yprescale;
	width_create  *= texture->xprescale;
	height_create *= texture->yprescale;

	// adjust the size for the border (must do this *after* the power of 2 clamp to satisfy
	// OpenGL semantics)
	if (texture->borderpix)
	{
		width += 2;
		height += 2;
		width_create += 2;
		height_create += 2;
	}
		*p_width=width;
		*p_height=height;
		*p_width_create=width_create;
		*p_height_create=height_create;
}

void sdl_info_ogl::texture_compute_size_type(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags)
{
	int finalheight, finalwidth;
	int finalheight_create, finalwidth_create;

	// if we're not wrapping, add a 1 pixel border on all sides
	texture->borderpix = 0; //!(texture->flags & PRIMFLAG_TEXWRAP_MASK);
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->borderpix = 0; // don't border the screen right now, there's a bug
	}

	texture_compute_type_subroutine(texsource, texture, flags);

	texture_compute_size_subroutine(texture, flags, texsource->width, texsource->height,
									&finalwidth, &finalheight, &finalwidth_create, &finalheight_create);

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if (texture->borderpix &&
		((finalwidth > m_texture_max_width && finalwidth - 2 <= m_texture_max_width) ||
			(finalheight > m_texture_max_height && finalheight - 2 <= m_texture_max_height)))
	{
		texture->borderpix = FALSE;

		texture_compute_type_subroutine(texsource, texture, flags);

		texture_compute_size_subroutine(texture, flags, texsource->width, texsource->height,
										&finalwidth, &finalheight, &finalwidth_create, &finalheight_create);
	}

	// if we're above the max width/height, do what?
	if (finalwidth_create > m_texture_max_width || finalheight_create > m_texture_max_height)
	{
		static int printed = FALSE;
		if (!printed)
			osd_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth_create, finalheight_create, m_texture_max_width, m_texture_max_height);
		printed = TRUE;
	}

	if(!texture->nocopy || texture->type==TEXTURE_TYPE_DYNAMIC || texture->type==TEXTURE_TYPE_SHADER ||
		// any of the mame core's device generated bitmap types:
		texture->format==SDL_TEXFORMAT_RGB32  ||
		texture->format==SDL_TEXFORMAT_RGB32_PALETTED  ||
		texture->format==SDL_TEXFORMAT_RGB15  ||
		texture->format==SDL_TEXFORMAT_RGB15_PALETTED  ||
		texture->format==SDL_TEXFORMAT_PALETTE16  ||
		texture->format==SDL_TEXFORMAT_PALETTE16A
		)
	{
		osd_printf_verbose("GL texture: copy %d, shader %d, dynamic %d, %dx%d %dx%d [%s, Equal: %d, Palette: %d,\n"
					"            scale %dx%d, border %d, pitch %d,%d/%d], bytes/pix %d\n",
			!texture->nocopy, texture->type==TEXTURE_TYPE_SHADER, texture->type==TEXTURE_TYPE_DYNAMIC,
			finalwidth, finalheight, finalwidth_create, finalheight_create,
			texfmt_to_string[texture->format],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE],
			texture->xprescale, texture->yprescale,
			texture->borderpix, texsource->rowpixels, finalwidth, m_texture_max_width,
			(int)sizeof(UINT32)
			);
	}

	// set the final values
	texture->rawwidth = finalwidth;
	texture->rawheight = finalheight;
	texture->rawwidth_create = finalwidth_create;
	texture->rawheight_create = finalheight_create;
}

//============================================================
//  texture_create
//============================================================

static int gl_checkFramebufferStatus(void)
{
	GLenum status;
	status=(GLenum)pfn_glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
	switch(status) {
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			return 0;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete,incomplete attachment\n");
			return -1;
		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			osd_printf_error("GL FBO: Unsupported framebuffer format\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete,missing attachment\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			osd_printf_error("GL FBO: incomplete,attached images must have same dimensions\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
				osd_printf_error("GL FBO: incomplete,attached images must have same format\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			osd_printf_error("GL FBO: incomplete,missing draw buffer\n");
			return -1;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			osd_printf_error("GL FBO: incomplete,missing read buffer\n");
			return -1;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			osd_printf_error("GL FBO: incomplete, duplicate attachment\n");
			return -1;
#endif
		case 0:
			osd_printf_error("GL FBO: incomplete, implementation fault\n");
			return -1;
		default:
			osd_printf_error("GL FBO: incomplete, implementation ERROR\n");
			/* fall through */
	}
	return -1;
}

static int texture_fbo_create(UINT32 text_unit, UINT32 text_name, UINT32 fbo_name, int width, int height)
{
	pfn_glActiveTexture(text_unit);
	pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo_name);
	glBindTexture(GL_TEXTURE_2D, text_name);
	{
		GLint _width, _height;
		if ( gl_texture_check_size(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
						0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &_width, &_height, 1) )
		{
			osd_printf_error("cannot create fbo texture, req: %dx%d, avail: %dx%d - bail out\n",
						width, height, (int)_width, (int)_height);
			return -1;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
				0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL );
	}
	// non-screen textures will never be filtered
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	pfn_glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
					GL_TEXTURE_2D, text_name, 0);

	if ( gl_checkFramebufferStatus() )
	{
		osd_printf_error("FBO error fbo texture - bail out\n");
		return -1;
	}

	return 0;
}

int sdl_info_ogl::texture_shader_create(const render_texinfo *texsource, ogl_texture_info *texture, UINT32 flags)
{
	int uniform_location;
	int i;
	int surf_w_pow2  = get_valid_pow2_value (m_blit_dim.width(), texture->texpow2);
	int surf_h_pow2  = get_valid_pow2_value (m_blit_dim.height(), texture->texpow2);

	assert ( texture->type==TEXTURE_TYPE_SHADER );

	GL_CHECK_ERROR_QUIET();

	if( m_glsl_program_num > 1 )
	{
		// multipass mode
		assert(m_usefbo);

		// GL_TEXTURE3 GLSL Uniforms
		texture->mpass_dest_idx = 0;
		texture->mpass_textureunit[0] = GL_TEXTURE3;
		texture->mpass_textureunit[1] = GL_TEXTURE2;
	}

	for(i=0; i<m_glsl_program_num; i++)
	{
		if ( i<=m_glsl_program_mb2sc )
		{
			m_glsl_program[i] = glsl_shader_get_program_mamebm(glsl_shader_feature, i);
		} else {
			m_glsl_program[i] = glsl_shader_get_program_scrn(i-1-m_glsl_program_mb2sc);
		}
		pfn_glUseProgramObjectARB(m_glsl_program[i]);

		if ( i<=m_glsl_program_mb2sc )
		{
			// GL_TEXTURE0 GLSL Uniforms
			uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture");
			pfn_glUniform1iARB(uniform_location, 0);
			GL_CHECK_ERROR_NORMAL();
		}

		{
			GLfloat color_texture_sz[2] = { (GLfloat)texture->rawwidth, (GLfloat)texture->rawheight };
			uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture_sz");
			pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_sz[0]));
			GL_CHECK_ERROR_NORMAL();
		}

		GLfloat color_texture_pow2_sz[2] = { (GLfloat)texture->rawwidth_create, (GLfloat)texture->rawheight_create };
		uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[i], "color_texture_pow2_sz");
		pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_pow2_sz[0]));
		GL_CHECK_ERROR_NORMAL();

		GLfloat screen_texture_sz[2] = { (GLfloat) m_blit_dim.width(), (GLfloat) m_blit_dim.height() };
		uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[i], "screen_texture_sz");
		pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_sz[0]));
		GL_CHECK_ERROR_NORMAL();

		GLfloat screen_texture_pow2_sz[2] = { (GLfloat)surf_w_pow2, (GLfloat)surf_h_pow2 };
		uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[i], "screen_texture_pow2_sz");
		pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_pow2_sz[0]));
		GL_CHECK_ERROR_NORMAL();
	}

	pfn_glUseProgramObjectARB(m_glsl_program[0]); // start with 1st shader

	if( m_glsl_program_num > 1 )
	{
		// multipass mode
		// GL_TEXTURE2/GL_TEXTURE3
		pfn_glGenFramebuffers(2, (GLuint *)&texture->mpass_fbo_mamebm[0]);
		glGenTextures(2, (GLuint *)&texture->mpass_texture_mamebm[0]);

		for (i=0; i<2; i++)
		{
			if ( texture_fbo_create(texture->mpass_textureunit[i],
									texture->mpass_texture_mamebm[i],
						texture->mpass_fbo_mamebm[i],
						texture->rawwidth_create, texture->rawheight_create) )
			{
				return -1;
			}
		}

		pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		osd_printf_verbose("GL texture: mpass mame-bmp   2x %dx%d (pow2 %dx%d)\n",
			texture->rawwidth, texture->rawheight, texture->rawwidth_create, texture->rawheight_create);
	}

	if( m_glsl_program_num > 1 && m_glsl_program_mb2sc < m_glsl_program_num - 1 )
	{
		// multipass mode
		// GL_TEXTURE2/GL_TEXTURE3
		pfn_glGenFramebuffers(2, (GLuint *)&texture->mpass_fbo_scrn[0]);
		glGenTextures(2, (GLuint *)&texture->mpass_texture_scrn[0]);

		for (i=0; i<2; i++)
		{
			if ( texture_fbo_create(texture->mpass_textureunit[i],
									texture->mpass_texture_scrn[i],
						texture->mpass_fbo_scrn[i],
						surf_w_pow2, surf_h_pow2) )
			{
				return -1;
			}
		}

		osd_printf_verbose("GL texture: mpass screen-bmp 2x %dx%d (pow2 %dx%d)\n",
			m_width, m_height, surf_w_pow2, surf_h_pow2);
	}

	// GL_TEXTURE0
	// get a name for this texture
	glGenTextures(1, (GLuint *)&texture->texture);
	pfn_glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->texture);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth_create);

	UINT32 * dummy = NULL;
	GLint _width, _height;
	if ( gl_texture_check_size(GL_TEXTURE_2D, 0, GL_RGBA8,
					texture->rawwidth_create, texture->rawheight_create,
					0,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
					&_width, &_height, 1) )
	{
		osd_printf_error("cannot create bitmap texture, req: %dx%d, avail: %dx%d - bail out\n",
			texture->rawwidth_create, texture->rawheight_create, (int)_width, (int)_height);
		return -1;
	}

	dummy = (UINT32 *) malloc(texture->rawwidth_create * texture->rawheight_create * sizeof(UINT32));
	memset(dummy, 0, texture->rawwidth_create * texture->rawheight_create * sizeof(UINT32));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
			texture->rawwidth_create, texture->rawheight_create,
			0,
			GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, dummy);
			glFinish(); // should not be necessary, .. but make sure we won't access the memory after free
	free(dummy);

	if ((PRIMFLAG_GET_SCREENTEX(flags)) && video_config.filter)
	{
		assert( glsl_shader_feature == GLSL_SHADER_FEAT_PLAIN );

		// screen textures get the user's choice of filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		// non-screen textures will never be filtered
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	// set wrapping mode appropriately
	if (texture->flags & PRIMFLAG_TEXWRAP_MASK)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}

	GL_CHECK_ERROR_NORMAL();

	return 0;
}

ogl_texture_info *sdl_info_ogl::texture_create(const render_texinfo *texsource, UINT32 flags)
{
	ogl_texture_info *texture;

	// allocate a new texture
	texture = global_alloc(ogl_texture_info);

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->xprescale = window().prescale();
		texture->yprescale = window().prescale();
	}
	else
	{
		texture->xprescale = 1;
		texture->yprescale = 1;
	}

	// set the texture_format
		//
		// src/emu/validity.c:validate_display() states,
		// an emulated driver can only produce
		//      BITMAP_FORMAT_IND16 and BITMAP_FORMAT_RGB32
		// where only the first original paletted.
		//
		// other paletted formats, i.e.:
		//   SDL_TEXFORMAT_RGB32_PALETTED, SDL_TEXFORMAT_RGB15_PALETTED and SDL_TEXFORMAT_YUY16_PALETTED
		// add features like brightness etc by the mame core
		//
		// all palette lookup may be implemented using shaders later on ..
		// that's why we keep the EQUAL flag TRUE, for all original true color bitmaps.
		//
	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			texture->format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
			if (texsource->palette != NULL)
				texture->format = SDL_TEXFORMAT_RGB32_PALETTED;
			else
				texture->format = SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			texture->format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_PALETTEA16:
			texture->format = SDL_TEXFORMAT_PALETTE16A;
			break;
		case TEXFORMAT_YUY16:
			if (texsource->palette != NULL)
				texture->format = SDL_TEXFORMAT_YUY16_PALETTED;
			else
				texture->format = SDL_TEXFORMAT_YUY16;
			break;

		default:
			osd_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	// compute the size
	texture_compute_size_type(texsource, texture, flags);

	texture->pbo=0;

	if ( texture->type != TEXTURE_TYPE_SHADER && m_useglsl)
	{
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	}

	if ( texture->type==TEXTURE_TYPE_SHADER )
	{
		if ( texture_shader_create(texsource, texture, flags) )
		{
			global_free(texture);
			return NULL;
		}
	}
	else
	{
		// get a name for this texture
		glGenTextures(1, (GLuint *)&texture->texture);

		glEnable(texture->texTarget);

		// make sure we're operating on *this* texture
		glBindTexture(texture->texTarget, texture->texture);

		// this doesn't actually upload, it just sets up the PBO's parameters
		glTexImage2D(texture->texTarget, 0, GL_RGBA8,
				texture->rawwidth_create, texture->rawheight_create,
				texture->borderpix ? 1 : 0,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		if ((PRIMFLAG_GET_SCREENTEX(flags)) && video_config.filter)
		{
			// screen textures get the user's choice of filtering
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			// non-screen textures will never be filtered
			glTexParameteri(texture->texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(texture->texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		if( texture->texTarget==GL_TEXTURE_RECTANGLE_ARB )
		{
			// texture rectangles can't wrap
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		} else {
			// set wrapping mode appropriately
			if (texture->flags & PRIMFLAG_TEXWRAP_MASK)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			}
		}
	}

	if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		assert(m_usepbo);

		// create the PBO
		pfn_glGenBuffers(1, (GLuint *)&texture->pbo);

		pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, texture->pbo);

		// set up the PBO dimension, ..
		pfn_glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
							texture->rawwidth * texture->rawheight * sizeof(UINT32),
					NULL, GL_STREAM_DRAW);
	}

	if ( !texture->nocopy && texture->type!=TEXTURE_TYPE_DYNAMIC )
	{
		texture->data = (UINT32 *) malloc(texture->rawwidth* texture->rawheight * sizeof(UINT32));
		texture->data_own=TRUE;
	}

	// add us to the texture list
	if (m_texhash[texture->hash] == NULL)
		m_texhash[texture->hash] = texture;
	else
	{
		int i;
		for (i = HASH_SIZE; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			if (m_texhash[i] == NULL)
			{
				m_texhash[i] = texture;
				break;
			}
		assert_always(i < HASH_SIZE + OVERFLOW_SIZE, "texture hash exhausted ...");
	}

	if(m_usevbo)
	{
		// Generate And Bind The Texture Coordinate Buffer
		pfn_glGenBuffers( 1, &(texture->texCoordBufferName) );
		pfn_glBindBuffer( GL_ARRAY_BUFFER_ARB, texture->texCoordBufferName );
		// Load The Data
		pfn_glBufferData( GL_ARRAY_BUFFER_ARB, 4*2*sizeof(GLfloat), texture->texCoord, GL_STREAM_DRAW );
		glTexCoordPointer( 2, GL_FLOAT, 0, (char *) NULL ); // we are using ARB VBO buffers
	}
	else
	{
		glTexCoordPointer(2, GL_FLOAT, 0, texture->texCoord);
	}

	return texture;
}

//============================================================
//  copyline_palette16
//============================================================

INLINE void copyline_palette16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*src];
	for (x = 0; x < width; x++)
	{
		int srcpix = *src++;
		for (int x2 = 0; x2 < xprescale; x2++)
			*dst++ = 0xff000000 | palette[srcpix];
	}
	if (xborderpix)
		*dst++ = 0xff000000 | palette[*--src];
}



//============================================================
//  copyline_palettea16
//============================================================

INLINE void copyline_palettea16(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
{
	int x;

	assert(xborderpix == 0 || xborderpix == 1);
	if (xborderpix)
		*dst++ = palette[*src];
	for (x = 0; x < width; x++)
	{
		int srcpix = *src++;
		for (int x2 = 0; x2 < xprescale; x2++)
			*dst++ = palette[srcpix];
	}
	if (xborderpix)
		*dst++ = palette[*--src];
}



//============================================================
//  copyline_rgb32
//============================================================

INLINE void copyline_rgb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
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
			for (int x2 = 0; x2 < xprescale; x2++)
			{
				*dst++ = 0xff000000 | palette[0x200 + srcpix.r()] | palette[0x100 + srcpix.g()] | palette[srcpix.b()];
			}
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
		{
			rgb_t srcpix = *src++;

			for (int x2 = 0; x2 < xprescale; x2++)
			{
				*dst++ = 0xff000000 | srcpix;
			}
		}
		if (xborderpix)
			*dst++ = 0xff000000 | *--src;
	}
}

//============================================================
//  copyline_argb32
//============================================================

INLINE void copyline_argb32(UINT32 *dst, const UINT32 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
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
			for (int x2 = 0; x2 < xprescale; x2++)
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
		{
			rgb_t srcpix = *src++;
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = srcpix;
		}
		if (xborderpix)
			*dst++ = *--src;
	}
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
//  copyline_yuy16_to_argb
//============================================================

INLINE void copyline_yuy16_to_argb(UINT32 *dst, const UINT16 *src, int width, const rgb_t *palette, int xborderpix, int xprescale)
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
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(palette[0x000 + (srcpix0 >> 8)], cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
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
			for (int x2 = 0; x2 < xprescale; x2++)
				*dst++ = ycc_to_rgb(srcpix0 >> 8, cb, cr);
			for (int x2 = 0; x2 < xprescale; x2++)
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

static void texture_set_data(ogl_texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
	if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		assert(texture->pbo);
		assert(!texture->nocopy);

		texture->data = (UINT32 *) pfn_glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY);
	}

	// note that nocopy and borderpix are mutually exclusive, IOW
	// they cannot be both true, thus this cannot lead to the
	// borderpix code below writing to texsource->base .
	if (texture->nocopy)
	{
		texture->data = (UINT32 *) texsource->base;
	}

	// always fill non-wrapping textures with an extra pixel on the top
	if (texture->borderpix)
	{
		memset(texture->data, 0,
				(texsource->width * texture->xprescale + 2) * sizeof(UINT32));
	}

	// when necessary copy (and convert) the data
	if (!texture->nocopy)
	{
		int y, y2;
		UINT8 *dst;

		for (y = 0; y < texsource->height; y++)
		{
			for (y2 = 0; y2 < texture->yprescale; y2++)
			{
				dst = (UINT8 *)(texture->data + (y * texture->yprescale + texture->borderpix + y2) * texture->rawwidth);

				switch (PRIMFLAG_GET_TEXFORMAT(flags))
				{
					case TEXFORMAT_PALETTE16:
						copyline_palette16((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_PALETTEA16:
						copyline_palettea16((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_RGB32:
						copyline_rgb32((UINT32 *)dst, (UINT32 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_ARGB32:
						copyline_argb32((UINT32 *)dst, (UINT32 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					case TEXFORMAT_YUY16:
						copyline_yuy16_to_argb((UINT32 *)dst, (UINT16 *)texsource->base + y * texsource->rowpixels, texsource->width, texsource->palette, texture->borderpix, texture->xprescale);
						break;

					default:
						osd_printf_error("Unknown texture blendmode=%d format=%d\n", PRIMFLAG_GET_BLENDMODE(flags), PRIMFLAG_GET_TEXFORMAT(flags));
						break;
				}
			}
		}
	}

	// always fill non-wrapping textures with an extra pixel on the bottom
	if (texture->borderpix)
	{
		memset((UINT8 *)texture->data +
				(texsource->height + 1) * texture->rawwidth * sizeof(UINT32),
				0,
			(texsource->width * texture->xprescale + 2) * sizeof(UINT32));
	}

	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, texture->texture);

		if (texture->nocopy)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->texinfo.rowpixels);
		else
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// and upload the image
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
				GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->data);
	}
	else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		glBindTexture(texture->texTarget, texture->texture);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// unmap the buffer from the CPU space so it can DMA
		pfn_glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

		// kick off the DMA
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
					GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	}
	else
	{
		glBindTexture(texture->texTarget, texture->texture);

		// give the card a hint
		if (texture->nocopy)
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->texinfo.rowpixels);
		else
			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// and upload the image
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
						GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texture->data);
	}
}

//============================================================
//  texture_find
//============================================================

static int compare_texture_primitive(const ogl_texture_info *texture, const render_primitive *prim)
{
	if (texture->texinfo.base == prim->texture.base &&
		texture->texinfo.width == prim->texture.width &&
		texture->texinfo.height == prim->texture.height &&
		texture->texinfo.rowpixels == prim->texture.rowpixels &&
		/* texture->texinfo.palette == prim->texture.palette && */
		((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		return 1;
	else
		return 0;
}

ogl_texture_info *sdl_info_ogl::texture_find(const render_primitive *prim)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	ogl_texture_info *texture;

	texture = m_texhash[texhash];
	if (texture != NULL)
	{
		int i;
		if (compare_texture_primitive(texture, prim))
			return texture;
		for (i=HASH_SIZE; i<HASH_SIZE + OVERFLOW_SIZE; i++)
		{
			texture = m_texhash[i];
			if (texture != NULL && compare_texture_primitive(texture, prim))
				return texture;
		}
	}
	return NULL;
}

//============================================================
//  texture_update
//============================================================

void sdl_info_ogl::texture_coord_update(ogl_texture_info *texture, const render_primitive *prim, int shaderIdx)
{
	float ustart = 0.0f, ustop = 0.0f;            // beginning/ending U coordinates
	float vstart = 0.0f, vstop = 0.0f;            // beginning/ending V coordinates
	float du, dv;

	if ( texture->type != TEXTURE_TYPE_SHADER ||
			( texture->type == TEXTURE_TYPE_SHADER && shaderIdx<=m_glsl_program_mb2sc ) )
	{
		// compute the U/V scale factors
		if (texture->borderpix)
		{
			int unscaledwidth = (texture->rawwidth_create-2) / texture->xprescale + 2;
			int unscaledheight = (texture->rawheight_create-2) / texture->yprescale + 2;
			ustart = 1.0f / (float)(unscaledwidth);
			ustop = (float)(prim->texture.width + 1) / (float)(unscaledwidth);
			vstart = 1.0f / (float)(unscaledheight);
			vstop = (float)(prim->texture.height + 1) / (float)(unscaledheight);
		}
		else
		{
			ustop  = (float)(prim->texture.width*texture->xprescale) / (float)texture->rawwidth_create;
			vstop  = (float)(prim->texture.height*texture->yprescale) / (float)texture->rawheight_create;
		}
	}
	else if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx>m_glsl_program_mb2sc )
	{
		int surf_w_pow2  = get_valid_pow2_value (m_width, texture->texpow2);
		int surf_h_pow2  = get_valid_pow2_value (m_height, texture->texpow2);

		ustop  = (float)(m_width) / (float)surf_w_pow2;
		vstop  = (float)(m_height) / (float)surf_h_pow2;

	}
	else
	{
		assert(0); // ??
	}

	du = ustop - ustart;
	dv = vstop - vstart;

	if ( texture->texTarget == GL_TEXTURE_RECTANGLE_ARB )
	{
		// texture coordinates for TEXTURE_RECTANGLE are 0,0 -> w,h
		// rather than 0,0 -> 1,1 as with normal OpenGL texturing
		du *= (float)texture->rawwidth;
		dv *= (float)texture->rawheight;
	}

	if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx!=m_glsl_program_mb2sc )
	{
		// 1:1 tex coord CCW (0/0) (1/0) (1/1) (0/1)
		// we must go CW here due to the mame bitmap order
		texture->texCoord[0]=ustart + du * 0.0f;
		texture->texCoord[1]=vstart + dv * 1.0f;
		texture->texCoord[2]=ustart + du * 1.0f;
		texture->texCoord[3]=vstart + dv * 1.0f;
		texture->texCoord[4]=ustart + du * 1.0f;
		texture->texCoord[5]=vstart + dv * 0.0f;
		texture->texCoord[6]=ustart + du * 0.0f;
		texture->texCoord[7]=vstart + dv * 0.0f;
	}
	else
	{
		// transformation: mamebm -> scrn
		texture->texCoord[0]=ustart + du * prim->texcoords.tl.u;
		texture->texCoord[1]=vstart + dv * prim->texcoords.tl.v;
		texture->texCoord[2]=ustart + du * prim->texcoords.tr.u;
		texture->texCoord[3]=vstart + dv * prim->texcoords.tr.v;
		texture->texCoord[4]=ustart + du * prim->texcoords.br.u;
		texture->texCoord[5]=vstart + dv * prim->texcoords.br.v;
		texture->texCoord[6]=ustart + du * prim->texcoords.bl.u;
		texture->texCoord[7]=vstart + dv * prim->texcoords.bl.v;
	}
}

void sdl_info_ogl::texture_mpass_flip(ogl_texture_info *texture, int shaderIdx)
{
	UINT32 mpass_src_idx = texture->mpass_dest_idx;

	texture->mpass_dest_idx = (mpass_src_idx+1) % 2;

	if ( shaderIdx>0 )
	{
		int uniform_location;
		uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[shaderIdx], "mpass_texture");
		pfn_glUniform1iARB(uniform_location, texture->mpass_textureunit[mpass_src_idx]-GL_TEXTURE0);
		GL_CHECK_ERROR_NORMAL();
	}

	pfn_glActiveTexture(texture->mpass_textureunit[mpass_src_idx]);
	if ( shaderIdx<=m_glsl_program_mb2sc )
	{
		glBindTexture(texture->texTarget, texture->mpass_texture_mamebm[mpass_src_idx]);
	}
	else
	{
		glBindTexture(texture->texTarget, texture->mpass_texture_scrn[mpass_src_idx]);
	}
	pfn_glActiveTexture(texture->mpass_textureunit[texture->mpass_dest_idx]);
	glBindTexture(texture->texTarget, 0);

	pfn_glActiveTexture(texture->mpass_textureunit[texture->mpass_dest_idx]);

	if ( shaderIdx<m_glsl_program_num-1 )
	{
		if ( shaderIdx>=m_glsl_program_mb2sc )
		{
			glBindTexture(texture->texTarget, texture->mpass_texture_scrn[texture->mpass_dest_idx]);
			pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, texture->mpass_fbo_scrn[texture->mpass_dest_idx]);
		}
		else
		{
			glBindTexture(texture->texTarget, texture->mpass_texture_mamebm[texture->mpass_dest_idx]);
			pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, texture->mpass_fbo_mamebm[texture->mpass_dest_idx]);
		}

		if ( shaderIdx==0 )
		{
			glPushAttrib(GL_VIEWPORT_BIT);
			GL_CHECK_ERROR_NORMAL();
			glViewport(0.0, 0.0, (GLsizei)texture->rawwidth, (GLsizei)texture->rawheight);
		}

		if ( shaderIdx==m_glsl_program_mb2sc )
		{
			assert ( m_glsl_program_mb2sc < m_glsl_program_num-1 );
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window().width, (GLsizei)window().height)
			GL_CHECK_ERROR_NORMAL();
		}
		glClear(GL_COLOR_BUFFER_BIT); // make sure the whole texture is redrawn ..
	}
	else
	{
		glBindTexture(texture->texTarget, 0);
		pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		if ( m_glsl_program_mb2sc == m_glsl_program_num-1 )
		{
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window().width, (GLsizei)window().height)
			GL_CHECK_ERROR_NORMAL();
		}

		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, 0);
	}
}

void sdl_info_ogl::texture_shader_update(ogl_texture_info *texture, render_container *container, int shaderIdx)
{
	int uniform_location;
	GLfloat vid_attributes[4];

	if (container!=NULL)
	{
		render_container::user_settings settings;
		container->get_user_settings(settings);
		/* FIXME: the code below is in just for illustration issue on
		 * how to set shader variables. gamma, contrast and brightness are
		 * handled already by the core
		 */
		vid_attributes[0] = settings.m_gamma;
		vid_attributes[1] = settings.m_contrast;
		vid_attributes[2] = settings.m_brightness;
		vid_attributes[3] = 0.0f;
		uniform_location = pfn_glGetUniformLocationARB(m_glsl_program[shaderIdx], "vid_attributes");
		pfn_glUniform4fvARB(uniform_location, 1, &(vid_attributes[shaderIdx]));
		if ( GL_CHECK_ERROR_QUIET() ) {
			osd_printf_verbose("GLSL: could not set 'vid_attributes' for shader prog idx %d\n", shaderIdx);
		}
	}
}

ogl_texture_info * sdl_info_ogl::texture_update(const render_primitive *prim, int shaderIdx)
{
	ogl_texture_info *texture = texture_find(prim);
	int texBound = 0;

	// if we didn't find one, create a new texture
	if (texture == NULL && prim->texture.base != NULL)
	{
		texture = texture_create(&prim->texture, prim->flags);
	}
	else if (texture != NULL)
	{
		if ( texture->type == TEXTURE_TYPE_SHADER )
		{
			pfn_glUseProgramObjectARB(m_glsl_program[shaderIdx]); // back to our shader
		}
		else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
		{
			assert ( m_usepbo ) ;
			pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, texture->pbo);
			glEnable(texture->texTarget);
		}
		else
		{
			glEnable(texture->texTarget);
		}
	}

	if (texture != NULL)
	{
		if ( texture->type == TEXTURE_TYPE_SHADER )
		{
			texture_shader_update(texture, prim->container, shaderIdx);
			if ( m_glsl_program_num>1 )
			{
				texture_mpass_flip(texture, shaderIdx);
			}
		}

		if ( shaderIdx==0 ) // redundant for subsequent multipass shader
		{
			if (prim->texture.base != NULL && texture->texinfo.seqid != prim->texture.seqid)
			{
				texture->texinfo.seqid = prim->texture.seqid;

				// if we found it, but with a different seqid, copy the data
				texture_set_data(texture, &prim->texture, prim->flags);
				texBound=1;
			}
		}

		if (!texBound) {
			glBindTexture(texture->texTarget, texture->texture);
		}
		texture_coord_update(texture, prim, shaderIdx);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(m_usevbo)
		{
			pfn_glBindBuffer( GL_ARRAY_BUFFER_ARB, texture->texCoordBufferName );
			// Load The Data
			pfn_glBufferSubData( GL_ARRAY_BUFFER_ARB, 0, 4*2*sizeof(GLfloat), texture->texCoord );
			glTexCoordPointer( 2, GL_FLOAT, 0, (char *) NULL ); // we are using ARB VBO buffers
		}
		else
		{
			glTexCoordPointer(2, GL_FLOAT, 0, texture->texCoord);
		}
	}

		return texture;
}

void sdl_info_ogl::texture_disable(ogl_texture_info * texture)
{
	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		assert ( m_useglsl );
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	} else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		glDisable(texture->texTarget);
	} else {
		glDisable(texture->texTarget);
	}
}

void sdl_info_ogl::texture_all_disable()
{
	if ( m_useglsl )
	{
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline

		pfn_glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( m_usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( m_usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( m_usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( m_usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

	if(m_usetexturerect)
	{
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
	}
	glDisable(GL_TEXTURE_2D);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if(m_usevbo)
	{
		pfn_glBindBuffer( GL_ARRAY_BUFFER_ARB, 0); // unbind ..
	}
	if ( m_usepbo )
	{
		pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	}
}
