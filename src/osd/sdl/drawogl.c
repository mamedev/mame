//============================================================
//
//  drawogl.c - SDL software and OpenGL implementation
//
//  Copyright (c) 1996-2011, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
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
#include "options.h"
#include "emuopts.h"

// standard SDL headers
#include <SDL/SDL.h>

// OpenGL headers
#include "osd_opengl.h"
#include "sdlos.h"


#include "gl_shader_tool.h"
#include "gl_shader_mgr.h"

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
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
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
#define GL_FRAMEBUFFER_EXT				0x8D40
#define GL_FRAMEBUFFER_COMPLETE_EXT			0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT	0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT	0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT	0x8CD8
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT		0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT		0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT	0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT	0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT			0x8CDD
#define GL_RENDERBUFFER_EXT				0x8D41
#define GL_DEPTH_COMPONENT16				0x81A5
#define GL_DEPTH_COMPONENT24				0x81A6
#define GL_DEPTH_COMPONENT32				0x81A7
#endif

//#define OLD_CODE  1

#ifndef OLD_CODE
#define HASH_SIZE		((1<<10)+1)
#define OVERFLOW_SIZE	(1<<10)
#endif

// OSD headers
#include "osdsdl.h"
#include "window.h"

//============================================================
//  DEBUGGING
//============================================================

#define DEBUG_MODE_SCORES	0
#define USE_WIN32_STYLE_LINES   0	// use the same method baseline does - yields somewhat nicer vectors but a little buggy

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

#define FSWAP(var1, var2) do { float temp = var1; var1 = var2; var2 = temp; } while (0)
#define GL_NO_PRIMITIVE -1

//============================================================
//  TYPES
//============================================================

typedef struct _texture_info texture_info;

#if USE_OPENGL
typedef void (*texture_copy_func)(texture_info *texture, const render_texinfo *texsource);
#endif

/* texture_info holds information about a texture */
struct _texture_info
{
#ifdef OLD_CODE
	texture_info *		next;				// next texture in the list
#endif
	HashT				hash;				// hash value for the texture (must be >= pointer size)
	UINT32				flags;				// rendering flags
	render_texinfo		texinfo;			// copy of the texture info
	int					rawwidth, rawheight;	// raw width/height of the texture
	int					rawwidth_create;	// raw width/height, pow2 compatible, if needed
	int                 rawheight_create;	// (create and initial set the texture, not for copy!)
	int					type;				// what type of texture are we?
	int					format;				// texture format
	int					borderpix;			// do we have a 1 pixel border?
	int					xprescale;			// what is our X prescale factor?
	int					yprescale;			// what is our Y prescale factor?
	int					nocopy;				// must the texture date be copied?

	UINT32				texture;			// OpenGL texture "name"/ID

	const GLint *		texProperties;		// texture properties
	texture_copy_func	texCopyFn;			// texture copy function, !=NULL if !nocopy
	GLenum				texTarget;			// OpenGL texture target
	int					texpow2;			// Is this texture pow2

	UINT32				mpass_dest_idx;			// Multipass dest idx [0..1]
	UINT32				mpass_textureunit[2];	// texture unit names for GLSL

	UINT32				mpass_texture_mamebm[2];// Multipass OpenGL texture "name"/ID for the shader
	UINT32				mpass_fbo_mamebm[2];	// framebuffer object for this texture, multipass
	UINT32				mpass_texture_scrn[2];	// Multipass OpenGL texture "name"/ID for the shader
	UINT32				mpass_fbo_scrn[2];		// framebuffer object for this texture, multipass

	UINT32				lut_texture;			// LUT OpenGL texture "name"/ID for the shader
	int					lut_table_width;		// LUT table width
	int					lut_table_height;		// LUT table height

	UINT32				pbo;					// pixel buffer object for this texture (DYNAMIC only!)
	UINT32				*data;					// pixels for the texture
	int					data_own;				// do we own / allocated it ?
	GLfloat				texCoord[8];
	GLuint				texCoordBufferName;

};

/* sdl_info is the information about SDL for the current screen */
typedef struct _sdl_info sdl_info;
struct _sdl_info
{
	INT32			blittimer;
	UINT32			extra_flags;

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_GLContext	gl_context_id;
#else
	// SDL surface
	SDL_Surface 		*sdlsurf;
#endif

	int				initialized;		// is everything well initialized, i.e. all GL stuff etc.
	// 3D info (GL mode only)
#ifdef OLD_CODE
	texture_info *	texlist;		// list of active textures
#else
	texture_info *	texhash[HASH_SIZE + OVERFLOW_SIZE];
#endif
	int				last_blendmode;		// previous blendmode
	INT32			texture_max_width;  	// texture maximum width
	INT32			texture_max_height; 	// texture maximum height
	int				texpoweroftwo;	        // must textures be power-of-2 sized?
	int				usevbo;			// runtime check if VBO is available
	int				usepbo;			// runtime check if PBO is available
	int				usefbo;			// runtime check if FBO is available
	int				useglsl;		// runtime check if GLSL is available

	glsl_shader_info *glsl;				// glsl_shader_info

	GLhandleARB		glsl_program[2*GLSL_SHADER_MAX];  // GLSL programs, or 0
	int				glsl_program_num;	// number of GLSL programs
	int				glsl_program_mb2sc;	// GLSL program idx, which transforms
			                            // the mame-bitmap -> screen-bitmap (size/rotation/..)
										// All progs <= glsl_program_mb2sc using the mame bitmap
										// as input, otherwise the screen bitmap.
										// All progs >= glsl_program_mb2sc using the screen bitmap
										// as output, otherwise the mame bitmap.
	int				glsl_vid_attributes;// glsl brightness, contrast and gamma for RGB bitmaps
	int				usetexturerect;		// use ARB_texture_rectangle for non-power-of-2, general use

	int				init_context;		// initialize context before next draw

	int				totalColors;		// total colors from machine/sdl_window_config/sdl_window_info

	float			last_hofs;
	float			last_vofs;

	// Static vars from draogl_window_dra
	INT32			surf_w;
	INT32			surf_h;
    GLfloat 		texVerticex[8];
};

/* line_aa_step is used for drawing antialiased lines */
typedef struct _line_aa_step line_aa_step;
struct _line_aa_step
{
	float		xoffs, yoffs;				// X/Y deltas
	float		weight;					// weight contribution
};

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

#ifdef OLD_CODE
INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	return (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}
#else
INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	HashT h = (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
	//printf("hash %d\n", (int) h % HASH_SIZE);
	return (h >> 8) % HASH_SIZE;
}
#endif

INLINE void set_blendmode(sdl_info *sdl, int blendmode)
{
	// try to minimize texture state changes
	if (blendmode != sdl->last_blendmode)
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

		sdl->last_blendmode = blendmode;
	}
}

//============================================================
//  PROTOTYPES
//============================================================

// core functions

static void drawogl_exit(void);
static void drawogl_attach(sdl_draw_info *info, sdl_window_info *window);
static int drawogl_window_create(sdl_window_info *window, int width, int height);
static void drawogl_window_resize(sdl_window_info *window, int width, int height);
static void drawogl_window_destroy(sdl_window_info *window);
static int drawogl_window_draw(sdl_window_info *window, UINT32 dc, int update);
static render_primitive_list &drawogl_window_get_primitives(sdl_window_info *window);
static void drawogl_destroy_all_textures(sdl_window_info *window);
static void drawogl_window_clear(sdl_window_info *window);
static int drawogl_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);
static void load_gl_lib(running_machine &machine);



// OGL 1.3
#ifdef GL_ARB_multitexture
static PFNGLACTIVETEXTUREARBPROC pfn_glActiveTexture	= NULL;
#else
static PFNGLACTIVETEXTUREPROC pfn_glActiveTexture	= NULL;
#endif

// VBO
static PFNGLGENBUFFERSPROC pfn_glGenBuffers		= NULL;
static PFNGLDELETEBUFFERSPROC pfn_glDeleteBuffers	= NULL;
static PFNGLBINDBUFFERPROC pfn_glBindBuffer		= NULL;
static PFNGLBUFFERDATAPROC pfn_glBufferData		= NULL;
static PFNGLBUFFERSUBDATAPROC pfn_glBufferSubData	= NULL;

// PBO
static PFNGLMAPBUFFERPROC     pfn_glMapBuffer		= NULL;
static PFNGLUNMAPBUFFERPROC   pfn_glUnmapBuffer		= NULL;

// FBO
static PFNGLISFRAMEBUFFEREXTPROC   pfn_glIsFramebuffer			= NULL;
static PFNGLBINDFRAMEBUFFEREXTPROC pfn_glBindFramebuffer		= NULL;
static PFNGLDELETEFRAMEBUFFERSEXTPROC pfn_glDeleteFramebuffers		= NULL;
static PFNGLGENFRAMEBUFFERSEXTPROC pfn_glGenFramebuffers		= NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC pfn_glCheckFramebufferStatus	= NULL;
static PFNGLFRAMEBUFFERTEXTURE2DEXTPROC pfn_glFramebufferTexture2D	= NULL;

static int glsl_shader_feature = GLSL_SHADER_FEAT_PLAIN;

//============================================================
//  TEXCOPY FUNCS
//============================================================

static void texcopy_argb32(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb32(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb32_paletted(texture_info *texture, const render_texinfo *texsource);
static void texcopy_palette16(texture_info *texture, const render_texinfo *texsource);
static void texcopy_palette16a(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb15(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb15_paletted(texture_info *texture, const render_texinfo *texsource);
static void texcopy_yuv16(texture_info *texture, const render_texinfo *texsource);
static void texcopy_yuv16_paletted(texture_info *texture, const render_texinfo *texsource);
#if 0 //def SDLMAME_MACOSX
static void texcopy_yuv16_apple(texture_info *texture, const render_texinfo *texsource);
static void texcopy_yuv16_paletted_apple(texture_info *texture, const render_texinfo *texsource);
#endif
// 16 bpp destination texture texcopy functions
static void texcopy_palette16_argb1555(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb15_argb1555(texture_info *texture, const render_texinfo *texsource);
static void texcopy_rgb15_paletted_argb1555(texture_info *texture, const render_texinfo *texsource);

//============================================================
//  Textures
//============================================================

static void texture_set_data(texture_info *texture, const render_texinfo *texsource);
static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, UINT32 flags);
static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim);
static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim, int shaderIdx);
static void texture_all_disable(sdl_info *sdl);
static void texture_disable(sdl_info *sdl, texture_info * texture);

//============================================================
//  Static Variables
//============================================================

static int shown_video_info = 0;
static int dll_loaded = 0;

//============================================================
//  drawogl_init
//============================================================

int drawogl_init(running_machine &machine, sdl_draw_info *callbacks)
{
	// fill in the callbacks
	callbacks->exit = drawogl_exit;
	callbacks->attach = drawogl_attach;

	dll_loaded = 0;

	if (SDL_VERSION_ATLEAST(1,3,0))
		mame_printf_verbose("Using SDL multi-window OpenGL driver (SDL 1.3+)\n");
	else
		mame_printf_verbose("Using SDL single-window OpenGL driver (SDL 1.2)\n");

#if (SDL_VERSION_ATLEAST(1,3,0))
	load_gl_lib(machine);
#endif

	return 0;
}


//============================================================
//  drawogl_attach
//============================================================

static void drawogl_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = drawogl_window_create;
	window->resize = drawogl_window_resize;
	window->get_primitives = drawogl_window_get_primitives;
	window->draw = drawogl_window_draw;
	window->destroy = drawogl_window_destroy;
	window->destroy_all_textures = drawogl_destroy_all_textures;
	window->clear = drawogl_window_clear;
	window->xy_to_render_target = drawogl_xy_to_render_target;
}


//============================================================
// Load the OGL function addresses
//============================================================

static void loadgl_functions(void)
{
#ifdef USE_DISPATCH_GL

	int err_count = 0;

	/* the following is tricky ... #func will be expanded to glBegin
     * while func will be expanded to disp_p->glBegin
     */

	#define OSD_GL(ret,func,params) \
	if (!( func = (ret (APIENTRY *)params) SDL_GL_GetProcAddress( #func ) )) \
		{ err_count++; mame_printf_error("GL function %s not found!\n", #func ); }

	#define OSD_GL_UNUSED(ret,func,params)

	#define GET_GLFUNC 1
	#include "osd_opengl.h"
	#undef GET_GLFUNC

	if (err_count)
		fatalerror("Error loading GL library functions, giving up\n");

#endif
}

//============================================================
// Load GL library
//============================================================

static void load_gl_lib(running_machine &machine)
{
#ifdef USE_DISPATCH_GL
	if (!dll_loaded)
	{
		/*
         *  directfb and and x11 use this env var
         *   SDL_VIDEO_GL_DRIVER
         */
		const char *stemp;

		stemp = downcast<sdl_options &>(machine.options()).gl_lib();
		if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) == 0)
			stemp = NULL;

		if (SDL_GL_LoadLibrary(stemp) != 0) // Load library (default for e==NULL
		{
			fatalerror("Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
		}
    	mame_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");
    	/* FIXME: must be freed as well */
    	gl_dispatch = (osd_gl_dispatch *) osd_malloc(sizeof(osd_gl_dispatch));
        dll_loaded=1;
	}
#endif
}

//============================================================
//  drawogl_window_create
//============================================================

static int drawogl_window_create(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl;
	char *extstr;
	char *vendor;
	int has_and_allow_texturerect = 0;

	// allocate memory for our structures
	sdl = (sdl_info *) osd_malloc(sizeof(*sdl));
	memset(sdl, 0, sizeof(*sdl));

	window->dxdata = sdl;

#if (SDL_VERSION_ATLEAST(1,3,0))
	sdl->extra_flags = (window->fullscreen ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);
	sdl->extra_flags |= SDL_WINDOW_OPENGL;

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	//Moved into init
	//load_gl_lib(window->machine());

	// create the SDL window

	if (window->fullscreen && video_config.switchres)
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(window->monitor->handle, &mode);
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;
		SDL_SetWindowDisplayMode(window->sdl_window, &mode);	// Try to set mode
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL);	// Use desktop

	window->sdl_window = SDL_CreateWindow(window->title, SDL_WINDOWPOS_UNDEFINED_DISPLAY(window->monitor->handle), SDL_WINDOWPOS_UNDEFINED,
			width, height, sdl->extra_flags);

	if  (!window->sdl_window )
	{
		mame_printf_error("OpenGL not supported on this driver: %s\n", SDL_GetError());
		return 1;
	}

	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->sdl_window, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);

	sdl->gl_context_id = SDL_GL_CreateContext(window->sdl_window);
	if  (!sdl->gl_context_id)
	{
		mame_printf_error("OpenGL not supported on this driver: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetSwapInterval(video_config.waitvsync ? 2 : 0);

#else
	sdl->extra_flags = (window->fullscreen ?  SDL_FULLSCREEN : SDL_RESIZABLE);
	sdl->extra_flags |= SDL_OPENGL | SDL_DOUBLEBUF;

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	#if (SDL_VERSION_ATLEAST(1,2,10))
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, video_config.waitvsync ? 1 : 0);
	#endif

	load_gl_lib(window->machine());

	// create the SDL surface (which creates the window in windowed mode)
	sdl->sdlsurf = SDL_SetVideoMode(width, height,
						   0, SDL_SWSURFACE  | SDL_ANYFORMAT | sdl->extra_flags);

	if (!sdl->sdlsurf)
		return 1;

	window->width = sdl->sdlsurf->w;
	window->height = sdl->sdlsurf->h;

	if ( (video_config.mode  == VIDEO_MODE_OPENGL) && !(sdl->sdlsurf->flags & SDL_OPENGL) )
	{
		mame_printf_error("OpenGL not supported on this driver!\n");
		return 1;
	}

	// set the window title
	SDL_WM_SetCaption(window->title, "SDLMAME");

#endif
	sdl->totalColors = window->totalColors;
	sdl->blittimer = 0;
	sdl->surf_w = 0;
	sdl->surf_h = 0;

	sdl->initialized = 0;

	// in case any textures try to come up before these are validated,
	// OpenGL guarantees all implementations can handle something this size.
	sdl->texture_max_width = 64;
	sdl->texture_max_height = 64;

	/* load any GL function addresses
     * this must be done here because we need a context
     */
	loadgl_functions();

	extstr = (char *)glGetString(GL_EXTENSIONS);
	vendor = (char *)glGetString(GL_VENDOR);

	// print out the driver info for debugging
	if (!shown_video_info)
	{
		mame_printf_verbose("OpenGL: %s\nOpenGL: %s\nOpenGL: %s\n", vendor, (char *)glGetString(GL_RENDERER), (char *)glGetString(GL_VERSION));
	}

	sdl->usetexturerect = 0;
	sdl->texpoweroftwo = 1;
	sdl->usevbo = 0;
	sdl->usepbo = 0;
	sdl->usefbo = 0;
	sdl->useglsl = 0;

	if ( video_config.allowtexturerect &&
	     ( strstr(extstr, "GL_ARB_texture_rectangle") ||  strstr(extstr, "GL_EXT_texture_rectangle") )
	   )
	{
		has_and_allow_texturerect = 1;
                    if (!shown_video_info)
                    {
                            mame_printf_verbose("OpenGL: texture rectangle supported\n");
                    }
	}

	// does this card support non-power-of-two sized textures?  (they're faster, so use them if possible)
	if ( !video_config.forcepow2texture && strstr(extstr, "GL_ARB_texture_non_power_of_two"))
	{
		if (!shown_video_info)
		{
			mame_printf_verbose("OpenGL: non-power-of-2 textures supported (new method)\n");
		}
                    sdl->texpoweroftwo = 0;
	}
	else
	{
		// second chance: GL_ARB_texture_rectangle or GL_EXT_texture_rectangle (old version)
		if (has_and_allow_texturerect)
		{
			if (!shown_video_info)
			{
				mame_printf_verbose("OpenGL: non-power-of-2 textures supported (old method)\n");
			}
			sdl->usetexturerect = 1;
		}
		else
		{
			if (!shown_video_info)
			{
				mame_printf_verbose("OpenGL: forcing power-of-2 textures (creation, not copy)\n");
			}
		}
	}

	if (strstr(extstr, "GL_ARB_vertex_buffer_object"))
	{
                    sdl->usevbo = video_config.vbo;
		if (!shown_video_info)
		{
			if(sdl->usevbo)
				mame_printf_verbose("OpenGL: vertex buffer supported\n");
			else
				mame_printf_verbose("OpenGL: vertex buffer supported, but disabled\n");
		}
	}

	if (strstr(extstr, "GL_ARB_pixel_buffer_object"))
	{
		if( sdl->usevbo )
		{
			sdl->usepbo = video_config.pbo;
			if (!shown_video_info)
			{
				if(sdl->usepbo)
					mame_printf_verbose("OpenGL: pixel buffers supported\n");
				else
					mame_printf_verbose("OpenGL: pixel buffers supported, but disabled\n");
			}
		} else {
			if (!shown_video_info)
			{
				mame_printf_verbose("OpenGL: pixel buffers supported, but disabled due to disabled vbo\n");
			}
		}
	}
	else
	{
		if (!shown_video_info)
		{
			mame_printf_verbose("OpenGL: pixel buffers not supported\n");
		}
	}

	if (strstr(extstr, "GL_EXT_framebuffer_object"))
	{
        sdl->usefbo = 1;
		if (!shown_video_info)
		{
			if(sdl->usefbo)
				mame_printf_verbose("OpenGL: framebuffer object supported\n");
			else
				mame_printf_verbose("OpenGL: framebuffer object not supported\n");
		}
	}

	if (strstr(extstr, "GL_ARB_shader_objects") &&
	    strstr(extstr, "GL_ARB_shading_language_100") &&
	    strstr(extstr, "GL_ARB_vertex_shader") &&
	    strstr(extstr, "GL_ARB_fragment_shader")
	   )
	{
		sdl->useglsl = video_config.glsl;
		if (!shown_video_info)
		{
			if(sdl->useglsl)
				mame_printf_verbose("OpenGL: GLSL supported\n");
			else
				mame_printf_verbose("OpenGL: GLSL supported, but disabled\n");
		}
	} else {
		if (!shown_video_info)
		{
			mame_printf_verbose("OpenGL: GLSL not supported\n");
		}
	}

	sdl->glsl_vid_attributes = video_config.glsl_vid_attributes;

	if (osd_getenv(SDLENV_VMWARE) != NULL)
	{
		sdl->usetexturerect = 1;
		sdl->texpoweroftwo = 1;
	}
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&sdl->texture_max_width);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint *)&sdl->texture_max_height);
	if (!shown_video_info)
	{
		mame_printf_verbose("OpenGL: max texture size %d x %d\n", sdl->texture_max_width, sdl->texture_max_height);
	}

	shown_video_info = 1;

	sdl->init_context = 0;

	return 0;
}

//============================================================
//  drawogl_window_resize
//============================================================

static void drawogl_window_resize(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

#if (SDL_VERSION_ATLEAST(1,3,0))
	//SDL_GL_MakeCurrent(window->sdl_window, sdl->gl_context_id);
	SDL_SetWindowSize(window->sdl_window, width, height);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
	sdl->blittimer = 3;
#else
	SDL_FreeSurface(sdl->sdlsurf);

	sdl->sdlsurf = SDL_SetVideoMode(width, height, 0,
			SDL_SWSURFACE | SDL_ANYFORMAT | sdl->extra_flags);
	window->width = sdl->sdlsurf->w;
	window->height = sdl->sdlsurf->h;
#endif
	sdl->init_context = 1;

}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawogl_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info *sdl =(sdl_info *)  window->dxdata;

	*xt = x - sdl->last_hofs;
	*yt = y - sdl->last_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *yt >= window->blitheight)
		return 0;
	return 1;
}

//============================================================
//  drawogl_window_get_primitives
//============================================================

static render_primitive_list &drawogl_window_get_primitives(sdl_window_info *window)
{
	if ((!window->fullscreen) || (video_config.switchres))
	{
		sdlwindow_blit_surface_size(window, window->width, window->height);
	}
	else
	{
		sdlwindow_blit_surface_size(window, window->monitor->center_width, window->monitor->center_height);
	}
	window->target->set_bounds(window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor));
	return window->target->get_primitives();
}

//============================================================
//  loadGLExtensions
//============================================================

static void loadGLExtensions(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	static int _once = 1;

	// sdl->usevbo=FALSE; // You may want to switch VBO and PBO off, by uncommenting this statement
	// sdl->usepbo=FALSE; // You may want to switch PBO off, by uncommenting this statement
	// sdl->useglsl=FALSE; // You may want to switch GLSL off, by uncommenting this statement

	if (! sdl->usevbo)
	{
		if(sdl->usepbo) // should never ever happen ;-)
		{
			if (_once)
			{
				mame_printf_warning("OpenGL: PBO not supported, no VBO support. (sdlmame error)\n");
			}
			sdl->usepbo=FALSE;
		}
		if(sdl->useglsl) // should never ever happen ;-)
		{
			if (_once)
			{
				mame_printf_warning("OpenGL: GLSL not supported, no VBO support. (sdlmame error)\n");
			}
			sdl->useglsl=FALSE;
		}
	}

	// Get Pointers To The GL Functions
	// VBO:
	if( sdl->usevbo )
	{
		pfn_glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers");
		pfn_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SDL_GL_GetProcAddress("glDeleteBuffers");
		pfn_glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer");
		pfn_glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData");
		pfn_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glBufferSubData");
	}
	// PBO:
	if ( sdl->usepbo )
	{
		pfn_glMapBuffer  = (PFNGLMAPBUFFERPROC) SDL_GL_GetProcAddress("glMapBuffer");
		pfn_glUnmapBuffer= (PFNGLUNMAPBUFFERPROC) SDL_GL_GetProcAddress("glUnmapBuffer");
	}
	// FBO:
	if ( sdl->usefbo )
	{
		pfn_glIsFramebuffer = (PFNGLISFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glIsFramebufferEXT");
		pfn_glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glBindFramebufferEXT");
		pfn_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC) SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
		pfn_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC) SDL_GL_GetProcAddress("glGenFramebuffersEXT");
		pfn_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
		pfn_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
	}

	if ( sdl->usevbo &&
	        ( !pfn_glGenBuffers || !pfn_glDeleteBuffers ||
	        !pfn_glBindBuffer || !pfn_glBufferData || !pfn_glBufferSubData
	 ) )
	{
		sdl->usepbo=FALSE;
		if (_once)
		{
			mame_printf_warning("OpenGL: VBO not supported, missing: ");
			if (!pfn_glGenBuffers)
			{
				mame_printf_warning("glGenBuffers, ");
			}
			if (!pfn_glDeleteBuffers)
			{
				mame_printf_warning("glDeleteBuffers");
			}
			if (!pfn_glBindBuffer)
			{
				mame_printf_warning("glBindBuffer, ");
			}
			if (!pfn_glBufferData)
			{
				mame_printf_warning("glBufferData, ");
			}
			if (!pfn_glBufferSubData)
			{
				mame_printf_warning("glBufferSubData, ");
			}
			mame_printf_warning("\n");
		}
		if ( sdl->usevbo )
		{
			if (_once)
			{
				mame_printf_warning("OpenGL: PBO not supported, no VBO support.\n");
			}
			sdl->usepbo=FALSE;
		}
	}

	if ( sdl->usepbo && ( !pfn_glMapBuffer || !pfn_glUnmapBuffer ) )
	{
		sdl->usepbo=FALSE;
		if (_once)
		{
			mame_printf_warning("OpenGL: PBO not supported, missing: ");
			if (!pfn_glMapBuffer)
			{
				mame_printf_warning("glMapBuffer, ");
			}
			if (!pfn_glUnmapBuffer)
			{
				mame_printf_warning("glUnmapBuffer, ");
			}
			mame_printf_warning("\n");
		}
	}

	if ( sdl->usefbo &&
		( !pfn_glIsFramebuffer || !pfn_glBindFramebuffer || !pfn_glDeleteFramebuffers ||
		  !pfn_glGenFramebuffers || !pfn_glCheckFramebufferStatus || !pfn_glFramebufferTexture2D
	  ))
	{
		sdl->usefbo=FALSE;
		if (_once)
		{
			mame_printf_warning("OpenGL: FBO not supported, missing: ");
			if (!pfn_glIsFramebuffer)
			{
				mame_printf_warning("pfn_glIsFramebuffer, ");
			}
			if (!pfn_glBindFramebuffer)
			{
				mame_printf_warning("pfn_glBindFramebuffer, ");
			}
			if (!pfn_glDeleteFramebuffers)
			{
				mame_printf_warning("pfn_glDeleteFramebuffers, ");
			}
			if (!pfn_glGenFramebuffers)
			{
				mame_printf_warning("pfn_glGenFramebuffers, ");
			}
			if (!pfn_glCheckFramebufferStatus)
			{
				mame_printf_warning("pfn_glCheckFramebufferStatus, ");
			}
			if (!pfn_glFramebufferTexture2D)
			{
				mame_printf_warning("pfn_glFramebufferTexture2D, ");
			}
			mame_printf_warning("\n");
		}
	}

	if (_once)
	{
		if ( sdl->usevbo )
		{
			mame_printf_verbose("OpenGL: VBO supported\n");
		}
		else
		{
			mame_printf_warning("OpenGL: VBO not supported\n");
		}

		if ( sdl->usepbo )
		{
			mame_printf_verbose("OpenGL: PBO supported\n");
		}
		else
		{
			mame_printf_warning("OpenGL: PBO not supported\n");
		}

		if ( sdl->usefbo )
		{
			mame_printf_verbose("OpenGL: FBO supported\n");
		}
		else
		{
			mame_printf_warning("OpenGL: FBO not supported\n");
		}
	}

	if ( sdl->useglsl )
	{
		#ifdef GL_ARB_multitexture
		pfn_glActiveTexture = (PFNGLACTIVETEXTUREARBPROC) SDL_GL_GetProcAddress("glActiveTextureARB");
		#else
		pfn_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture");
		#endif
		if (!pfn_glActiveTexture)
		{
			if (_once)
			{
				mame_printf_warning("OpenGL: GLSL disabled, glActiveTexture(ARB) not supported\n");
			}
			sdl->useglsl = 0;
		}
	}

	if ( sdl->useglsl )
	{
		sdl->glsl = glsl_shader_init();
		sdl->useglsl = (sdl->glsl != NULL ? 1 : 0);

		if ( ! sdl->useglsl )
		{
			if (_once)
			{
				mame_printf_warning("OpenGL: GLSL supported, but shader instantiation failed - disabled\n");
			}
		}
	}

	if ( sdl->useglsl )
	{
		if ( window->prescale != 1 )
		{
			sdl->useglsl = 0;
			if (_once)
			{
				mame_printf_warning("OpenGL: GLSL supported, but disabled due to: prescale !=1 \n");
			}
		}
	}

	if ( sdl->useglsl )
	{
		int i;
		video_config.filter = FALSE;
		glsl_shader_feature = GLSL_SHADER_FEAT_PLAIN;
		sdl->glsl_program_num = 0;
		sdl->glsl_program_mb2sc = 0;

		for(i=0; i<video_config.glsl_shader_mamebm_num; i++)
		{
			if ( !sdl->usefbo && sdl->glsl_program_num==1 )
			{
				if (_once)
				{
					mame_printf_verbose("OpenGL: GLSL multipass not supported, due to unsupported FBO. Skipping followup shader\n");
				}
				break;
			}

			if ( glsl_shader_add_mamebm(sdl->glsl, video_config.glsl_shader_mamebm[i], sdl->glsl_program_num) )
			{
				mame_printf_error("OpenGL: GLSL loading mame bitmap shader %d failed (%s)\n",
					i, video_config.glsl_shader_mamebm[i]);
			} else {
				glsl_shader_feature = GLSL_SHADER_FEAT_CUSTOM;
				if (_once)
				{
					mame_printf_verbose("OpenGL: GLSL using mame bitmap shader filter %d: '%s'\n",
						sdl->glsl_program_num, video_config.glsl_shader_mamebm[i]);
				}
				sdl->glsl_program_mb2sc = sdl->glsl_program_num; // the last mame_bitmap (mb) shader does it.
				sdl->glsl_program_num++;
			}
		}

		if ( video_config.glsl_shader_scrn_num > 0 && sdl->glsl_program_num==0 )
		{
			mame_printf_verbose("OpenGL: GLSL cannot use screen bitmap shader without bitmap shader\n");
		}

		for(i=0; sdl->usefbo && sdl->glsl_program_num>0 && i<video_config.glsl_shader_scrn_num; i++)
		{
			if ( glsl_shader_add_scrn(sdl->glsl, video_config.glsl_shader_scrn[i],
			                              sdl->glsl_program_num-1-sdl->glsl_program_mb2sc) )
			{
				mame_printf_error("OpenGL: GLSL loading screen bitmap shader %d failed (%s)\n",
					i, video_config.glsl_shader_scrn[i]);
			} else {
				if (_once)
				{
					mame_printf_verbose("OpenGL: GLSL using screen bitmap shader filter %d: '%s'\n",
						sdl->glsl_program_num, video_config.glsl_shader_scrn[i]);
				}
				sdl->glsl_program_num++;
			}
		}

		if ( 0==sdl->glsl_program_num &&
		     0 <= video_config.glsl_filter && video_config.glsl_filter < GLSL_SHADER_FEAT_INT_NUMBER )
		{
			sdl->glsl_program_mb2sc = sdl->glsl_program_num; // the last mame_bitmap (mb) shader does it.
			sdl->glsl_program_num++;
			glsl_shader_feature = video_config.glsl_filter;

			if (_once)
			{
				mame_printf_verbose("OpenGL: GLSL using shader filter '%s', idx: %d, num %d (vid filter: %d)\n",
					glsl_shader_get_filter_name_mamebm(glsl_shader_feature),
					glsl_shader_feature, sdl->glsl_program_num, video_config.filter);
			}
		}

		if (_once)
		{
			if ( sdl->glsl_vid_attributes )
			{
				mame_printf_verbose("OpenGL: GLSL direct brightness, contrast setting for RGB games\n");
			}
			else
			{
				mame_printf_verbose("OpenGL: GLSL paletted gamma, brightness, contrast setting for RGB games\n");
			}
		}
	} else {
		if (_once)
		{
			mame_printf_verbose("OpenGL: using vid filter: %d\n", video_config.filter);
		}
	}

	_once = 0;
}

//============================================================
//  drawogl_window_draw
//============================================================

static int drawogl_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	render_primitive *prim;
	texture_info *texture=NULL;
	float vofs, hofs;
	int  pendingPrimitive=GL_NO_PRIMITIVE, curPrimitive=GL_NO_PRIMITIVE, scrnum, is_vector;
	const screen_device *screen;

	if (video_config.novideo)
	{
		return 0;
	}

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_GL_MakeCurrent(window->sdl_window, sdl->gl_context_id);
#endif
	if (sdl->init_context)
	{
		// do some one-time OpenGL setup
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}

	// figure out if we're vector
	scrnum = is_vector = 0;
	for (screen = window->machine().config().first_screen(); screen != NULL; screen = screen->next_screen())
	{
		if (scrnum == window->index)
		{
			is_vector = (screen->screen_type() == SCREEN_TYPE_VECTOR) ? 1 : 0;
			break;
		}
		else
		{
			scrnum++;
		}
	}

	// only clear if the geometry changes (and for 2 frames afterward to clear double and triple buffers)
	if ((sdl->blittimer > 0) || (is_vector))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		sdl->blittimer--;
	}

	if ( !sdl->initialized ||
			window->width!= sdl->surf_w || window->height!= sdl->surf_h )
	{
		if ( !sdl->initialized )
		{
			loadGLExtensions(window);
		}

		sdl->surf_w=window->width;
		sdl->surf_h=window->height;

		// we're doing nothing 3d, so the Z-buffer is currently not interesting
		glDisable(GL_DEPTH_TEST);

		if (window->machine().options().antialias())
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
		sdl->last_blendmode = BLENDMODE_ALPHA;

		// set lines and points just barely above normal size to get proper results
		glLineWidth(video_config.beamwidth);
		glPointSize(video_config.beamwidth);

		// set up a nice simple 2D coordinate system, so GL behaves exactly how we'd like.
		//
		// (0,0)     (w,0)
		//   |~~~~~~~~~|
		//   |         |
		//   |         |
		//   |         |
		//   |_________|
		// (0,h)     (w,h)

		glViewport(0.0, 0.0, (GLsizei)window->width, (GLsizei)window->height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)window->width, (GLdouble)window->height, 0.0, 0.0, -1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if ( ! sdl->initialized )
		{
			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(2, GL_FLOAT, 0, sdl->texVerticex); // no VBO, since it's too volatile

			sdl->initialized = 1;
		}
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		if ((window->fullscreen) && (!video_config.switchres))
		{
			ch = window->monitor->center_height;
			cw = window->monitor->center_width;
		}
		else
		{
			ch = window->height;
			cw = window->width;
		}

		if (video_config.centerv)
		{
			vofs = (ch - window->blitheight) / 2.0f;
		}
		if (video_config.centerh)
		{
			hofs = (cw - window->blitwidth) / 2.0f;
		}
	}

	sdl->last_hofs = hofs;
	sdl->last_vofs = vofs;

	window->primlist->acquire_lock();

	// now draw
	for (prim = window->primlist->first(); prim != NULL; prim = prim->next())
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
		                	set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim->flags));
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
					effwidth = prim->width;
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

				set_blendmode(sdl, PRIMFLAG_GET_BLENDMODE(prim->flags));

				texture = texture_update(window, prim, 0);

				if ( texture && texture->type==TEXTURE_TYPE_SHADER )
				{
					for(i=0; i<sdl->glsl_program_num; i++)
					{
						if ( i==sdl->glsl_program_mb2sc )
						{
							// i==sdl->glsl_program_mb2sc -> transformation mamebm->scrn
							sdl->texVerticex[0]=prim->bounds.x0 + hofs;
							sdl->texVerticex[1]=prim->bounds.y0 + vofs;
							sdl->texVerticex[2]=prim->bounds.x1 + hofs;
							sdl->texVerticex[3]=prim->bounds.y0 + vofs;
							sdl->texVerticex[4]=prim->bounds.x1 + hofs;
							sdl->texVerticex[5]=prim->bounds.y1 + vofs;
							sdl->texVerticex[6]=prim->bounds.x0 + hofs;
							sdl->texVerticex[7]=prim->bounds.y1 + vofs;
						} else {
							// 1:1 tex coord CCW (0/0) (1/0) (1/1) (0/1) on texture dimensions
							sdl->texVerticex[0]=(GLfloat)0.0;
							sdl->texVerticex[1]=(GLfloat)0.0;
							sdl->texVerticex[2]=(GLfloat)window->width;
							sdl->texVerticex[3]=(GLfloat)0.0;
							sdl->texVerticex[4]=(GLfloat)window->width;
							sdl->texVerticex[5]=(GLfloat)window->height;
							sdl->texVerticex[6]=(GLfloat)0.0;
							sdl->texVerticex[7]=(GLfloat)window->height;
						}

						if(i>0) // first fetch already done
						{
							texture = texture_update(window, prim, i);
						}
						glDrawArrays(GL_QUADS, 0, 4);
					}
				} else {
					sdl->texVerticex[0]=prim->bounds.x0 + hofs;
					sdl->texVerticex[1]=prim->bounds.y0 + vofs;
					sdl->texVerticex[2]=prim->bounds.x1 + hofs;
					sdl->texVerticex[3]=prim->bounds.y0 + vofs;
					sdl->texVerticex[4]=prim->bounds.x1 + hofs;
					sdl->texVerticex[5]=prim->bounds.y1 + vofs;
					sdl->texVerticex[6]=prim->bounds.x0 + hofs;
					sdl->texVerticex[7]=prim->bounds.y1 + vofs;

					glDrawArrays(GL_QUADS, 0, 4);
				}

				if ( texture )
				{
					texture_disable(sdl, texture);
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

	window->primlist->release_lock();
	sdl->init_context = 0;

#if (!SDL_VERSION_ATLEAST(1,3,0))
	SDL_GL_SwapBuffers();
#else
	SDL_GL_SwapWindow(window->sdl_window);
#endif
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

// 6 properties (per format)
// right order according to glTexImage2D: internal, format, type, ..
enum { SDL_TEXFORMAT_INTERNAL, SDL_TEXFORMAT_FORMAT, SDL_TEXFORMAT_TYPE, SDL_TEXFORMAT_PIXEL_SIZE };

static const GLint texture_gl_properties_srcNative_intNative[9][6] = {
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_ARGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32_PALETTED
#if 0 //def SDLMAME_MACOSX
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16_PALETTED
#else
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16_PALETTED
#endif
	{ GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, sizeof(UINT16) }, // SDL_TEXFORMAT_PALETTE16
	{ GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, sizeof(UINT16) }, // SDL_TEXFORMAT_RGB15
	{ GL_RGB5_A1, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, sizeof(UINT16) }, // SDL_TEXFORMAT_RGB15_PALETTED
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) }    // SDL_TEXFORMAT_PALETTE16A
};

static const GLint texture_gl_properties_srcNative_int32bpp[9][6] = {
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_ARGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32_PALETTED
#if 0 //def SDLMAME_MACOSX
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16_PALETTED
#else
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16_PALETTED
#endif
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_PALETTE16
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, sizeof(UINT16) },   // SDL_TEXFORMAT_RGB15
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, sizeof(UINT16) },   // SDL_TEXFORMAT_RGB15_PALETTED
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) }    // SDL_TEXFORMAT_PALETTE16A
};

static const GLint texture_gl_properties_srcCopy_int32bpp[9][6] = {
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_ARGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },    // SDL_TEXFORMAT_RGB32_PALETTED
#if 0 //def SDLMAME_MACOSX
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16
	{ GL_RGB8, GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_REV_APPLE, sizeof(UINT16) }, // SDL_TEXFORMAT_YUY16_PALETTED
#else
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_YUY16_PALETTED
#endif
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_PALETTE16
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_RGB15
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) },   // SDL_TEXFORMAT_RGB15_PALETTED
	{ GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sizeof(UINT32) }    // SDL_TEXFORMAT_PALETTE16A
};

static const texture_copy_func texcopy_dstNative_f[9] = {
	texcopy_argb32,
	texcopy_rgb32,
	texcopy_rgb32_paletted,
#if 0 //def SDLMAME_MACOSX
	texcopy_yuv16_apple,
	texcopy_yuv16_paletted_apple,
#else
	texcopy_yuv16,
	texcopy_yuv16_paletted,
#endif

	texcopy_palette16_argb1555,
	texcopy_rgb15_argb1555,
	texcopy_rgb15_paletted_argb1555,
	texcopy_palette16a
};

static const texture_copy_func texcopy_dst32bpp_f[9] = {
	texcopy_argb32,
	texcopy_rgb32,
	texcopy_rgb32_paletted,
#if 0 //def SDLMAME_MACOSX
	texcopy_yuv16_apple,
	texcopy_yuv16_paletted_apple,
#else
	texcopy_yuv16,
	texcopy_yuv16_paletted,
#endif

	texcopy_palette16,
	texcopy_rgb15,
	texcopy_rgb15_paletted,
	texcopy_palette16a
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
//  drawogl_window_destroy
//============================================================

static void drawogl_window_destroy(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	// skip if nothing
	if (sdl == NULL)
		return;

	// free the memory in the window

	drawogl_destroy_all_textures(window);

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_GL_DeleteContext(sdl->gl_context_id);
	SDL_DestroyWindow(window->sdl_window);
#else
	if (sdl->sdlsurf)
	{
		SDL_FreeSurface(sdl->sdlsurf);
		sdl->sdlsurf = NULL;
	}
#endif

	osd_free(sdl);
	window->dxdata = NULL;
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
static void texture_compute_type_subroutine(sdl_info *sdl, const render_texinfo *texsource, texture_info *texture, UINT32 flags)
{
	texture->type = TEXTURE_TYPE_NONE;
	texture->nocopy = FALSE;

	if ( texture->type == TEXTURE_TYPE_NONE &&
	     !PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->type = TEXTURE_TYPE_PLAIN;
                texture->texTarget = (sdl->usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
                texture->texpow2   = (sdl->usetexturerect)?0:sdl->texpoweroftwo;
	}

	// currently glsl supports idx and rgb palette lookups,
	// no special quality scaling, so we could drop the prescale criteria below ..
	if ( texture->type == TEXTURE_TYPE_NONE &&
	     sdl->useglsl &&
	     (
		 texture->format==SDL_TEXFORMAT_PALETTE16 ||       // glsl idx16 lut
	         texture->format==SDL_TEXFORMAT_RGB32_PALETTED ||  // glsl rgb32 lut/direct
             texture->format==SDL_TEXFORMAT_RGB32 ||
             texture->format==SDL_TEXFORMAT_RGB15_PALETTED ||    // glsl rgb15 lut/direct
             texture->format==SDL_TEXFORMAT_RGB15
	     ) &&
	     texture->xprescale == 1 && texture->yprescale == 1 &&
	     texsource->rowpixels <= sdl->texture_max_width
	    )
	{
		texture->type      = TEXTURE_TYPE_SHADER;
		texture->nocopy    = TRUE;
                texture->texTarget = GL_TEXTURE_2D;
                texture->texpow2   = sdl->texpoweroftwo;
	}

	// determine if we can skip the copy step
	// if this was not already decided by the shader condition above
	if ( !texture->nocopy &&
	      texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST] &&
	     !texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE] &&
	      texture->xprescale == 1 && texture->yprescale == 1 &&
	     !texture->borderpix &&
	      texsource->rowpixels <= sdl->texture_max_width )
	{
		texture->nocopy = TRUE;
        }

	if( texture->type == TEXTURE_TYPE_NONE &&
	    sdl->usepbo && !texture->nocopy )
	{

		texture->type      = TEXTURE_TYPE_DYNAMIC;
                texture->texTarget = (sdl->usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
                texture->texpow2   = (sdl->usetexturerect)?0:sdl->texpoweroftwo;
	}

	if( texture->type == TEXTURE_TYPE_NONE )
	{
		texture->type      = TEXTURE_TYPE_SURFACE;
                texture->texTarget = (sdl->usetexturerect)?GL_TEXTURE_RECTANGLE_ARB:GL_TEXTURE_2D;
                texture->texpow2   = (sdl->usetexturerect)?0:sdl->texpoweroftwo;
        }

        if ( texture->type!=TEXTURE_TYPE_SHADER && video_config.prefer16bpp_tex )
        {
                texture->texProperties = texture_gl_properties_srcNative_intNative[texture->format];
                texture->texCopyFn     = texcopy_dstNative_f[texture->format];
        } else if ( texture->nocopy )
        {
                texture->texProperties = texture_gl_properties_srcNative_int32bpp[texture->format];
                texture->texCopyFn     = NULL;
        } else {
                texture->texProperties = texture_gl_properties_srcCopy_int32bpp[texture->format];
                texture->texCopyFn     = texcopy_dst32bpp_f[texture->format];
        }
}

INLINE int get_valid_pow2_value(int v, int needPow2)
{
	return (needPow2)?gl_round_to_pow2(v):v;
}

static void texture_compute_size_subroutine(sdl_window_info *window, texture_info *texture, UINT32 flags,
                                            UINT32 width, UINT32 height,
                                            int* p_width, int* p_height, int* p_width_create, int* p_height_create)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
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
	while (texture->xprescale > 1 && width_create * texture->xprescale > sdl->texture_max_width)
		texture->xprescale--;
	while (texture->yprescale > 1 && height_create * texture->yprescale > sdl->texture_max_height)
		texture->yprescale--;
	if (PRIMFLAG_GET_SCREENTEX(flags) && (texture->xprescale != window->prescale || texture->yprescale != window->prescale))
		mame_printf_warning("SDL: adjusting prescale from %dx%d to %dx%d\n", window->prescale, window->prescale, texture->xprescale, texture->yprescale);

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

static void texture_compute_size_type(sdl_window_info *window, const render_texinfo *texsource, texture_info *texture, UINT32 flags)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	int finalheight, finalwidth;
	int finalheight_create, finalwidth_create;

	// if we're not wrapping, add a 1 pixel border on all sides
	texture->borderpix = 0; //!(texture->flags & PRIMFLAG_TEXWRAP_MASK);
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->borderpix = 0;	// don't border the screen right now, there's a bug
	}

	texture_compute_type_subroutine(sdl, texsource, texture, flags);

	texture_compute_size_subroutine(window, texture, flags, texsource->width, texsource->height,
	                                &finalwidth, &finalheight, &finalwidth_create, &finalheight_create);

	// if we added pixels for the border, and that just barely pushed us over, take it back
	if (texture->borderpix &&
		((finalwidth > sdl->texture_max_width && finalwidth - 2 <= sdl->texture_max_width) ||
		 (finalheight > sdl->texture_max_height && finalheight - 2 <= sdl->texture_max_height)))
	{
		texture->borderpix = FALSE;

		texture_compute_type_subroutine(sdl, texsource, texture, flags);

		texture_compute_size_subroutine(window, texture, flags, texsource->width, texsource->height,
		                                &finalwidth, &finalheight, &finalwidth_create, &finalheight_create);
	}

	// if we're above the max width/height, do what?
	if (finalwidth_create > sdl->texture_max_width || finalheight_create > sdl->texture_max_height)
	{
		static int printed = FALSE;
		if (!printed)
			mame_printf_warning("Texture too big! (wanted: %dx%d, max is %dx%d)\n", finalwidth_create, finalheight_create, sdl->texture_max_width, sdl->texture_max_height);
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
		mame_printf_verbose("GL texture: copy %d, shader %d, dynamic %d, %dx%d %dx%d [%s, Equal: %d, Palette: %d,\n"
	                "            scale %dx%d, border %d, pitch %d,%d/%d], colors: %d, bytes/pix %d\n",
			!texture->nocopy, texture->type==TEXTURE_TYPE_SHADER, texture->type==TEXTURE_TYPE_DYNAMIC,
			finalwidth, finalheight, finalwidth_create, finalheight_create,
			texfmt_to_string[texture->format],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_EQUALS_DEST],
			(int)texture_copy_properties[texture->format][SDL_TEXFORMAT_SRC_HAS_PALETTE],
			texture->xprescale, texture->yprescale,
			texture->borderpix, texsource->rowpixels, finalwidth, sdl->texture_max_width,
			sdl->totalColors, (int)texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]
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
            mame_printf_error("GL FBO: incomplete,incomplete attachment\n");
            return -1;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            mame_printf_error("GL FBO: Unsupported framebuffer format\n");
            return -1;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            mame_printf_error("GL FBO: incomplete,missing attachment\n");
            return -1;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            mame_printf_error("GL FBO: incomplete,attached images must have same dimensions\n");
            return -1;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
             mame_printf_error("GL FBO: incomplete,attached images must have same format\n");
            return -1;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            mame_printf_error("GL FBO: incomplete,missing draw buffer\n");
            return -1;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            mame_printf_error("GL FBO: incomplete,missing read buffer\n");
            return -1;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT
        case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
            mame_printf_error("GL FBO: incomplete, duplicate attachment\n");
            return -1;
#endif
        case 0:
            mame_printf_error("GL FBO: incomplete, implementation fault\n");
            return -1;
        default:
            mame_printf_error("GL FBO: incomplete, implementation ERROR\n");
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
			mame_printf_error("cannot create fbo texture, req: %dx%d, avail: %dx%d - bail out\n",
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
		mame_printf_error("FBO error fbo texture - bail out\n");
		return -1;
	}

	return 0;
}

static int texture_shader_create(sdl_window_info *window,
                                 const render_texinfo *texsource, texture_info *texture, UINT32 flags)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	int uniform_location;
	int lut_table_width_pow2=0;
	int lut_table_height_pow2=0;
	int glsl_shader_type_rgb32 = ( sdl->glsl_vid_attributes ) ? GLSL_SHADER_TYPE_RGB32_DIRECT : GLSL_SHADER_TYPE_RGB32_LUT;
	int glsl_shader_type, i;
	int lut_texture_width;
	int surf_w_pow2  = get_valid_pow2_value (window->width, texture->texpow2);
	int surf_h_pow2  = get_valid_pow2_value (window->height, texture->texpow2);

	assert ( texture->type==TEXTURE_TYPE_SHADER );

	texture->lut_table_height = 1; // default ..

	switch(texture->format)
	{
		case SDL_TEXFORMAT_RGB32_PALETTED:
		case SDL_TEXFORMAT_RGB32:
			glsl_shader_type          = glsl_shader_type_rgb32;
			texture->lut_table_width  = 1 << 8; // 8 bits per component
			texture->lut_table_width *= 3;      // BGR ..
			break;

		case SDL_TEXFORMAT_RGB15_PALETTED:
		case SDL_TEXFORMAT_RGB15:
			glsl_shader_type          = glsl_shader_type_rgb32;
			texture->lut_table_width  = 1 << 5; // 5 bits per component
			texture->lut_table_width *= 3;      // BGR ..
			break;

		case SDL_TEXFORMAT_PALETTE16:
			glsl_shader_type          = GLSL_SHADER_TYPE_IDX16;
		    texture->lut_table_width  = sdl->totalColors;
			break;

		default:
			// should never happen
			assert(0);
			exit(1);
	}

	/**
     * We experience some GLSL LUT calculation inaccuracy on some GL drivers.
     * while using the correct lut calculations.
     * This error is due to the color index value to GLSL/texture passing process:
     *   mame:uint16_t -> OpenGL: GLfloat(alpha texture) -> GLSL:uint16_t (value regeneration)
     * The latter inaccurate uint16_t value regeneration is buggy on some drivers/cards,
     * therefor we always widen the lut size to pow2,
     * and shape it equaly into 2D space (max texture size restriction).
     * This is a practical GL driver workaround to minimize the chance for
     * floating point arithmetic errors in the GLSL engine.
     *
     * Shape the lut texture to achieve texture max size compliance and equal 2D partitioning
     */

	if ( texture->format == SDL_TEXFORMAT_PALETTE16 )
	{
		lut_texture_width  = sqrt((double)(texture->lut_table_width));
		lut_texture_width  = get_valid_pow2_value (lut_texture_width, 1);

		texture->lut_table_height = texture->lut_table_width / lut_texture_width;

		if ( lut_texture_width*texture->lut_table_height < texture->lut_table_width )
		{
			texture->lut_table_height  += 1;
		}

		texture->lut_table_width   = lut_texture_width;
	}
	else
	{
		lut_texture_width = texture->lut_table_width;
		texture->lut_table_height = 1;
	}

	/**
     * always use pow2 for LUT, to minimize the chance for floating point arithmetic errors
     * (->buggy GLSL engine)
     */
	lut_table_height_pow2 = get_valid_pow2_value (texture->lut_table_height, 1 /* texture->texpow2 */);
	lut_table_width_pow2  = get_valid_pow2_value (texture->lut_table_width,  1 /* texture->texpow2 */);

	if ( !sdl->glsl_vid_attributes || texture->format==SDL_TEXFORMAT_PALETTE16 )
	{
		mame_printf_verbose("GL texture: lut_texture_width %d, lut_table_sz %dx%d, lut_table_sz_pow2 %dx%d\n",
				lut_texture_width, texture->lut_table_width, texture->lut_table_height,
				lut_table_width_pow2, lut_table_height_pow2);
	}


	if ( lut_table_width_pow2 > sdl->texture_max_width || lut_table_height_pow2 > sdl->texture_max_height )
	{
		mame_printf_error("Need lut size %dx%d, but max text size is %dx%d, bail out\n",
			lut_table_width_pow2, lut_table_height_pow2,
			sdl->texture_max_width, sdl->texture_max_height);
		return -1;
	}

	GL_CHECK_ERROR_QUIET();

	if( sdl->glsl_program_num > 1 )
	{
		// multipass mode
		assert(sdl->usefbo);

		// GL_TEXTURE3 GLSL Uniforms
		texture->mpass_dest_idx = 0;
		texture->mpass_textureunit[0] = GL_TEXTURE3;
		texture->mpass_textureunit[1] = GL_TEXTURE2;
	}

	for(i=0; i<sdl->glsl_program_num; i++)
	{
		if ( i<=sdl->glsl_program_mb2sc )
		{
			sdl->glsl_program[i] = glsl_shader_get_program_mamebm(glsl_shader_type, glsl_shader_feature, i);
		} else {
			sdl->glsl_program[i] = glsl_shader_get_program_scrn(i-1-sdl->glsl_program_mb2sc);
		}
		pfn_glUseProgramObjectARB(sdl->glsl_program[i]);

		if ( i<=sdl->glsl_program_mb2sc && !(sdl->glsl_vid_attributes && texture->format!=SDL_TEXFORMAT_PALETTE16) )
		{
			// GL_TEXTURE1 GLSL Uniforms
			uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "colortable_texture");
			pfn_glUniform1iARB(uniform_location, 1);
			GL_CHECK_ERROR_NORMAL();

			{
				GLfloat colortable_sz[2] = { (GLfloat)texture->lut_table_width, (GLfloat)texture->lut_table_height };
				uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "colortable_sz");
				pfn_glUniform2fvARB(uniform_location, 1, &(colortable_sz[0]));
				GL_CHECK_ERROR_NORMAL();
			}

			{
				GLfloat colortable_pow2_sz[2] = { (GLfloat)lut_table_width_pow2, (GLfloat)lut_table_height_pow2 };
				uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "colortable_pow2_sz");
				pfn_glUniform2fvARB(uniform_location, 1, &(colortable_pow2_sz[0]));
				GL_CHECK_ERROR_NORMAL();
			}
		}

		if ( i<=sdl->glsl_program_mb2sc )
		{
			// GL_TEXTURE0 GLSL Uniforms
			uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "color_texture");
			pfn_glUniform1iARB(uniform_location, 0);
			GL_CHECK_ERROR_NORMAL();
		}

		{
			GLfloat color_texture_sz[2] = { (GLfloat)texture->rawwidth, (GLfloat)texture->rawheight };
			uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "color_texture_sz");
			pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_sz[0]));
			GL_CHECK_ERROR_NORMAL();
		}

		{
			GLfloat color_texture_pow2_sz[2] = { (GLfloat)texture->rawwidth_create, (GLfloat)texture->rawheight_create };
			uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "color_texture_pow2_sz");
			pfn_glUniform2fvARB(uniform_location, 1, &(color_texture_pow2_sz[0]));
			GL_CHECK_ERROR_NORMAL();
		}
		if ( i>sdl->glsl_program_mb2sc )
		{
			{
				GLfloat screen_texture_sz[2] = { (GLfloat)window->width, (GLfloat)window->height };
				uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "screen_texture_sz");
				pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_sz[0]));
				GL_CHECK_ERROR_NORMAL();
			}

			{
				GLfloat screen_texture_pow2_sz[2] = { (GLfloat)surf_w_pow2, (GLfloat)surf_h_pow2 };
				uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[i], "screen_texture_pow2_sz");
				pfn_glUniform2fvARB(uniform_location, 1, &(screen_texture_pow2_sz[0]));
				GL_CHECK_ERROR_NORMAL();
			}
		}
	}

	pfn_glUseProgramObjectARB(sdl->glsl_program[0]); // start with 1st shader

	if( sdl->glsl_program_num > 1 )
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

		mame_printf_verbose("GL texture: mpass mame-bmp   2x %dx%d (pow2 %dx%d)\n",
			texture->rawwidth, texture->rawheight, texture->rawwidth_create, texture->rawheight_create);
	}

	if( sdl->glsl_program_num > 1 && sdl->glsl_program_mb2sc < sdl->glsl_program_num - 1 )
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

		mame_printf_verbose("GL texture: mpass screen-bmp 2x %dx%d (pow2 %dx%d)\n",
			window->width, window->height, surf_w_pow2, surf_h_pow2);
	}

	if ( !(sdl->glsl_vid_attributes && texture->format!=SDL_TEXFORMAT_PALETTE16) )
	{
		// GL_TEXTURE1
		glGenTextures(1, (GLuint *)&texture->lut_texture);
		pfn_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture->lut_texture);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, lut_table_width_pow2);

		{
			GLint _width, _height;
			if ( gl_texture_check_size(GL_TEXTURE_2D, 0, GL_RGBA8, lut_table_width_pow2, lut_table_height_pow2,
					  0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &_width, &_height, 1) )
			{
				mame_printf_error("cannot create lut table texture, req: %dx%d, avail: %dx%d - bail out\n",
					lut_table_width_pow2, lut_table_height_pow2, (int)_width, (int)_height);
				return -1;
			}
		}

		{
			UINT32 * dummy = (UINT32 *) malloc(lut_table_width_pow2*lut_table_height_pow2 * sizeof(UINT32)); // blank out the whole pal.
			memset(dummy, 0, lut_table_width_pow2*lut_table_height_pow2 * sizeof(UINT32));
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, lut_table_width_pow2, lut_table_height_pow2,
				 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, dummy );
			glFinish(); // should not be necessary, .. but make sure we won't access the memory after free
			free(dummy);
		}

		glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->lut_table_width);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture->lut_table_width, texture->lut_table_height,
			     GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texsource->palette );

		// non-screen textures will never be filtered
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		assert ( texture->lut_texture );
	}

	// GL_TEXTURE0
	// get a name for this texture
	glGenTextures(1, (GLuint *)&texture->texture);
	pfn_glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->texture);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth_create);

	if(texture->format!=SDL_TEXFORMAT_PALETTE16)
	{
		UINT32 * dummy = NULL;
		GLint _width, _height;
		if ( gl_texture_check_size(GL_TEXTURE_2D, 0, texture->texProperties[SDL_TEXFORMAT_INTERNAL],
					   texture->rawwidth_create, texture->rawheight_create,
					   0,
					   texture->texProperties[SDL_TEXFORMAT_FORMAT],
					   texture->texProperties[SDL_TEXFORMAT_TYPE],
					   &_width, &_height, 1) )
		{
			mame_printf_error("cannot create bitmap texture, req: %dx%d, avail: %dx%d - bail out\n",
				texture->rawwidth_create, texture->rawheight_create, (int)_width, (int)_height);
			return -1;
		}

		dummy = (UINT32 *) malloc(texture->rawwidth_create * texture->rawheight_create *
                               texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]);
		memset(dummy, 0, texture->rawwidth_create * texture->rawheight_create *
                               texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]);
		glTexImage2D(GL_TEXTURE_2D, 0, texture->texProperties[SDL_TEXFORMAT_INTERNAL],
		     texture->rawwidth_create, texture->rawheight_create,
		     0,
		     texture->texProperties[SDL_TEXFORMAT_FORMAT],
		     texture->texProperties[SDL_TEXFORMAT_TYPE], dummy);
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
	} else {
		UINT32 * dummy = NULL;
		GLint _width, _height;
		if ( gl_texture_check_size(GL_TEXTURE_2D, 0, GL_ALPHA16, texture->rawwidth_create, texture->rawheight_create,
				  0, GL_ALPHA, GL_UNSIGNED_SHORT, &_width, &_height, 1) )
		{
			mame_printf_error("cannot create lut bitmap texture, req: %dx%d, avail: %dx%d - bail out\n",
				texture->rawwidth_create, texture->rawheight_create,
				(int)_width, (int)_height);
			return -1;
		}
		dummy = (UINT32 *) malloc(texture->rawwidth_create * texture->rawheight_create * sizeof(UINT16));
		memset(dummy, 0, texture->rawwidth_create * texture->rawheight_create * sizeof(UINT16));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA16,
		     texture->rawwidth_create, texture->rawheight_create,
		     0,
		     GL_ALPHA, GL_UNSIGNED_SHORT, dummy);
                glFinish(); // should not be necessary, .. but make sure we won't access the memory after free
		free(dummy);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

	GL_CHECK_ERROR_NORMAL();

	return 0;
}

static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, UINT32 flags)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *texture;

	// allocate a new texture
	texture = (texture_info *) malloc(sizeof(*texture));
	memset(texture, 0, sizeof(*texture));

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	if (PRIMFLAG_GET_SCREENTEX(flags))
	{
		texture->xprescale = window->prescale;
		texture->yprescale = window->prescale;
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
        //      BITMAP_FORMAT_INDEXED16, BITMAP_FORMAT_RGB15 and BITMAP_FORMAT_RGB32
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
		case TEXFORMAT_RGB15:
            if (texsource->palette != NULL)
                texture->format = SDL_TEXFORMAT_RGB15_PALETTED;
            else
                texture->format = SDL_TEXFORMAT_RGB15;
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
			mame_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	// compute the size
	texture_compute_size_type(window, texsource, texture, flags);

    texture->pbo=0;

	if ( texture->type != TEXTURE_TYPE_SHADER && sdl->useglsl)
	{
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	}

	if ( texture->type==TEXTURE_TYPE_SHADER )
	{
		if ( texture_shader_create(window, texsource, texture, flags) )
        {
            free(texture);
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
		glTexImage2D(texture->texTarget, 0, texture->texProperties[SDL_TEXFORMAT_INTERNAL],
		     texture->rawwidth_create, texture->rawheight_create,
		     texture->borderpix ? 1 : 0,
		     texture->texProperties[SDL_TEXFORMAT_FORMAT],
		     texture->texProperties[SDL_TEXFORMAT_TYPE], NULL);

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
        assert(sdl->usepbo);

        // create the PBO
        pfn_glGenBuffers(1, (GLuint *)&texture->pbo);

        pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, texture->pbo);

        // set up the PBO dimension, ..
		pfn_glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB,
		                 texture->rawwidth * texture->rawheight * texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE],
				 NULL, GL_STREAM_DRAW);
    }

    if ( !texture->nocopy && texture->type!=TEXTURE_TYPE_DYNAMIC )
    {
        texture->data = (UINT32 *) malloc(texture->rawwidth* texture->rawheight * texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]);
        texture->data_own=TRUE;
    }

	// add us to the texture list
#ifdef OLD_CODE
	texture->next = sdl->texlist;
	sdl->texlist = texture;
#else
	if (sdl->texhash[texture->hash] == NULL)
		sdl->texhash[texture->hash] = texture;
	else
	{
		int i;
		for (i = HASH_SIZE; i < HASH_SIZE + OVERFLOW_SIZE; i++)
			if (sdl->texhash[i] == NULL)
			{
				sdl->texhash[i] = texture;
				break;
			}
		assert(i < HASH_SIZE + OVERFLOW_SIZE);
	}
#endif
	if(sdl->usevbo)
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
//  texture_set_data
//============================================================

static void texture_set_data(texture_info *texture, const render_texinfo *texsource)
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
		texture->data = (UINT32 *) texsource->base;

	// always fill non-wrapping textures with an extra pixel on the top
	if (texture->borderpix)
	{
		memset(texture->data, 0,
				(texsource->width * texture->xprescale + 2) * texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]);
	}

	// when nescesarry copy (and convert) the data
	if (!texture->nocopy)
	{
		assert(texture->texCopyFn);
		texture->texCopyFn(texture, texsource);
	}

	// always fill non-wrapping textures with an extra pixel on the bottom
	if (texture->borderpix)
	{
		memset((UINT8 *)texture->data +
		       (texsource->height + 1) * texture->rawwidth * texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE],
		       0,
			(texsource->width * texture->xprescale + 2) * texture->texProperties[SDL_TEXFORMAT_PIXEL_SIZE]);
	}

	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		if ( texture->lut_texture )
		{
			pfn_glActiveTexture(GL_TEXTURE1);
			glBindTexture(texture->texTarget, texture->lut_texture);

			glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->lut_table_width);

			// give the card a hint
			glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->lut_table_width, texture->lut_table_height,
				     GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, texsource->palette );
		}
		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, texture->texture);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->texinfo.rowpixels);

		// and upload the image
		if(texture->format!=SDL_TEXFORMAT_PALETTE16)
		{
			glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
					texture->texProperties[SDL_TEXFORMAT_FORMAT],
					texture->texProperties[SDL_TEXFORMAT_TYPE], texture->data);
		}
		else
		{
			glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
					GL_ALPHA, GL_UNSIGNED_SHORT, texture->data);
		}
	}
	else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		glBindTexture(texture->texTarget, texture->texture);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, texture->rawwidth);

		// unmap the buffer from the CPU space so it can DMA
		pfn_glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);

		// kick off the DMA
		glTexSubImage2D(texture->texTarget, 0, 0, 0, texture->rawwidth, texture->rawheight,
			        texture->texProperties[SDL_TEXFORMAT_FORMAT],
				texture->texProperties[SDL_TEXFORMAT_TYPE], NULL);
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
		        texture->texProperties[SDL_TEXFORMAT_FORMAT],
		texture->texProperties[SDL_TEXFORMAT_TYPE], texture->data);
	}
}

//============================================================
//  texture_find
//============================================================

#ifdef OLD_CODE
static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;

	// find a match
	for (texture = sdl->texlist; texture != NULL; texture = texture->next)
		if (texture->hash == texhash &&
			texture->texinfo.base == prim->texture.base &&
			texture->texinfo.width == prim->texture.width &&
			texture->texinfo.height == prim->texture.height &&
			texture->texinfo.rowpixels == prim->texture.rowpixels &&
			((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
			return texture;

	// nothing found
	return NULL;
}
#else

#if 0
static int compare_texinfo(render_texinfo *t1, render_texinfo *t2)
{
	if (t1->base == t2->base &&
			t1->width == t2->width &&
			t1->height == t2->height &&
			t1->rowpixels == t2->rowpixels)
		return 1;
	else
		return 0;
}
#endif

static int compare_texture_primitive(const texture_info *texture, const render_primitive *prim)
{
	if (texture->texinfo.base == prim->texture.base &&
		texture->texinfo.width == prim->texture.width &&
		texture->texinfo.height == prim->texture.height &&
		texture->texinfo.rowpixels == prim->texture.rowpixels &&
		((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		return 1;
	else
		return 0;
}

static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;

	texture = sdl->texhash[texhash];
	if (texture != NULL)
	{
		int i;
		if (compare_texture_primitive(texture, prim))
			return texture;
		for (i=HASH_SIZE; i<HASH_SIZE + OVERFLOW_SIZE; i++)
		{
			texture = sdl->texhash[i];
			if (texture != NULL && compare_texture_primitive(texture, prim))
				return texture;
		}
	}
	return NULL;
}

#endif

//============================================================
//  texture_update
//============================================================

static void texture_coord_update(sdl_window_info *window,
                                 texture_info *texture, const render_primitive *prim, int shaderIdx)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	float ustart, ustop;			// beginning/ending U coordinates
	float vstart, vstop;			// beginning/ending V coordinates
	float du, dv;

	if ( texture->type != TEXTURE_TYPE_SHADER ||
	     ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx<=sdl->glsl_program_mb2sc ) )
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
			ustart = 0.0f;
			ustop  = (float)(prim->texture.width*texture->xprescale) / (float)texture->rawwidth_create;
			vstart = 0.0f;
			vstop  = (float)(prim->texture.height*texture->yprescale) / (float)texture->rawheight_create;
		}
	}
	else if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx>sdl->glsl_program_mb2sc )
	{
		int surf_w_pow2  = get_valid_pow2_value (window->width, texture->texpow2);
		int surf_h_pow2  = get_valid_pow2_value (window->height, texture->texpow2);

		ustart = 0.0f;
		ustop  = (float)(window->width) / (float)surf_w_pow2;
		vstart = 0.0f;
		vstop  = (float)(window->height) / (float)surf_h_pow2;
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

	if ( texture->type == TEXTURE_TYPE_SHADER && shaderIdx!=sdl->glsl_program_mb2sc )
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

static void texture_mpass_flip(sdl_info *sdl, texture_info *texture, int shaderIdx)
{
	UINT32 mpass_src_idx = texture->mpass_dest_idx;

	texture->mpass_dest_idx = (mpass_src_idx+1) % 2;

	if ( shaderIdx>0 )
	{
		int uniform_location;
		uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[shaderIdx], "mpass_texture");
		pfn_glUniform1iARB(uniform_location, texture->mpass_textureunit[mpass_src_idx]-GL_TEXTURE0);
		GL_CHECK_ERROR_NORMAL();
	}

	pfn_glActiveTexture(texture->mpass_textureunit[mpass_src_idx]);
	if ( shaderIdx<=sdl->glsl_program_mb2sc )
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

	if ( shaderIdx<sdl->glsl_program_num-1 )
	{
		if ( shaderIdx>=sdl->glsl_program_mb2sc )
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
		else if ( shaderIdx==sdl->glsl_program_mb2sc )
		{
			assert ( sdl->glsl_program_mb2sc < sdl->glsl_program_num-1 );
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window->width, (GLsizei)window->height)
			GL_CHECK_ERROR_NORMAL();
		}
		glClear(GL_COLOR_BUFFER_BIT); // make sure the whole texture is redrawn ..
	}
	else
	{
		glBindTexture(texture->texTarget, 0);
		pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

		if ( sdl->glsl_program_mb2sc == sdl->glsl_program_num-1 )
		{
			glPopAttrib(); // glViewport(0.0, 0.0, (GLsizei)window->width, (GLsizei)window->height)
			GL_CHECK_ERROR_NORMAL();
		}

		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(texture->texTarget, 0);
	}
}

static void texture_shader_update(sdl_window_info *window, texture_info *texture, int shaderIdx)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	if ( !texture->lut_texture )
	{
		int uniform_location, scrnum;
		render_container *container;
		GLfloat vid_attributes[4]; // gamma, contrast, brightness, effect

		assert ( sdl->glsl_vid_attributes && texture->format!=SDL_TEXFORMAT_PALETTE16 );

		scrnum = 0;
		container = (render_container *)NULL;
		for (screen_device *screen = window->machine().first_screen(); screen != NULL; screen = screen->next_screen())
		{
			if (scrnum == window->start_viewscreen)
			{
				container = &screen->container();
			}

			scrnum++;
		}

		if (container!=NULL)
		{
			render_container::user_settings settings;
			container->get_user_settings(settings);
			//FIXME: Intended behaviour
#if 1
			vid_attributes[0] = window->machine().options().gamma();
			vid_attributes[1] = window->machine().options().contrast();
			vid_attributes[2] = window->machine().options().brightness();
#else
			vid_attributes[0] = settings.gamma;
			vid_attributes[1] = settings.contrast;
			vid_attributes[2] = settings.brightness;
#endif
			vid_attributes[3] = 0.0f;
			uniform_location = pfn_glGetUniformLocationARB(sdl->glsl_program[shaderIdx], "vid_attributes");
			pfn_glUniform4fvARB(uniform_location, 1, &(vid_attributes[shaderIdx]));
			if ( GL_CHECK_ERROR_QUIET() ) {
				mame_printf_verbose("GLSL: could not set 'vid_attributes' for shader prog idx %d\n", shaderIdx);
			}
		}
		else
		{
			mame_printf_verbose("GLSL: could not get render container for screen %d\n", window->start_viewscreen);
		}
	}
}

static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim, int shaderIdx)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *texture = texture_find(sdl, prim);
	int texBound = 0;

	// if we didn't find one, create a new texture
	if (texture == NULL && prim->texture.base != NULL)
    {
            texture = texture_create(window, &prim->texture, prim->flags);

    }
	else if (texture != NULL)
    {
		if ( texture->type == TEXTURE_TYPE_SHADER )
		{
			pfn_glUseProgramObjectARB(sdl->glsl_program[shaderIdx]); // back to our shader
        }
		else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
        {
	        assert ( sdl->usepbo ) ;
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
			texture_shader_update(window, texture, shaderIdx);
			if ( sdl->glsl_program_num>1 )
			{
				texture_mpass_flip(sdl, texture, shaderIdx);
			}
		}

		if ( shaderIdx==0 ) // redundant for subsequent multipass shader
		{
			if (prim->texture.base != NULL && texture->texinfo.seqid != prim->texture.seqid)
			{
				texture->texinfo.seqid = prim->texture.seqid;

				// if we found it, but with a different seqid, copy the data
				texture_set_data(texture, &prim->texture);
				texBound=1;
			}
		}

		if (!texBound) {
			glBindTexture(texture->texTarget, texture->texture);
		}
		texture_coord_update(window, texture, prim, shaderIdx);

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(sdl->usevbo)
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

static void texture_disable(sdl_info *sdl, texture_info * texture)
{
	if ( texture->type == TEXTURE_TYPE_SHADER )
	{
		assert ( sdl->useglsl );
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline
	} else if ( texture->type == TEXTURE_TYPE_DYNAMIC )
	{
		pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		glDisable(texture->texTarget);
	} else {
		glDisable(texture->texTarget);
	}
}

static void texture_all_disable(sdl_info *sdl)
{
	if ( sdl->useglsl )
        {
		pfn_glUseProgramObjectARB(0); // back to fixed function pipeline

		pfn_glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( sdl->usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( sdl->usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( sdl->usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
		pfn_glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if ( sdl->usefbo ) pfn_glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
        }
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

        if(sdl->usetexturerect)
        {
                glDisable(GL_TEXTURE_RECTANGLE_ARB);
        }
        glDisable(GL_TEXTURE_2D);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if(sdl->usevbo)
        {
		pfn_glBindBuffer( GL_ARRAY_BUFFER_ARB, 0); // unbind ..
        }
	if ( sdl->usepbo )
	{
		pfn_glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0);
        }
}

static void drawogl_destroy_all_textures(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *texture = NULL;
	int lock=FALSE;
#ifdef OLD_CODE
	texture_info *next_texture=NULL;
#else
	int i;
#endif

	if (sdl == NULL)
		return;

	if ( !sdl->initialized )
		return;

#if (SDL_VERSION_ATLEAST(1,3,0))
	SDL_GL_MakeCurrent(window->sdl_window, sdl->gl_context_id);
#endif

	if(window->primlist)
	{
		lock=TRUE;
		window->primlist->acquire_lock();
	}

	glFinish();

	texture_all_disable(sdl);
	glFinish();
	glDisableClientState(GL_VERTEX_ARRAY);

#ifdef OLD_CODE
	texture = sdl->texlist;
	while (texture)
	{
		next_texture = texture->next;
#else
	i=0;
	while (i<HASH_SIZE+OVERFLOW_SIZE)
	{
		texture = sdl->texhash[i];
		sdl->texhash[i] = NULL;
		if (texture != NULL)
		{
#endif

		if(sdl->usevbo)
		{
			pfn_glDeleteBuffers( 1, &(texture->texCoordBufferName) );
			texture->texCoordBufferName=0;
    	}

		if(sdl->usepbo && texture->pbo)
		{
			pfn_glDeleteBuffers( 1, (GLuint *)&(texture->pbo) );
			texture->pbo=0;
		}

		if( sdl->glsl_program_num > 1 )
		{
			assert(sdl->usefbo);
			pfn_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_mamebm[0]);
			glDeleteTextures(2, (GLuint *)&texture->mpass_texture_mamebm[0]);
		}

		if ( sdl->glsl_program_mb2sc < sdl->glsl_program_num - 1 )
		{
			assert(sdl->usefbo);
			pfn_glDeleteFramebuffers(2, (GLuint *)&texture->mpass_fbo_scrn[0]);
			glDeleteTextures(2, (GLuint *)&texture->mpass_texture_scrn[0]);
		}

		if(texture->lut_texture)
			glDeleteTextures(1, (GLuint *)&texture->lut_texture);

		glDeleteTextures(1, (GLuint *)&texture->texture);
		if ( texture->data_own )
		{
			free(texture->data);
			texture->data=NULL;
			texture->data_own=FALSE;
		}
		free(texture);
#ifdef OLD_CODE
		texture = next_texture;
	}
    sdl->texlist = NULL;
#else
		}
		i++;
	}
#endif
	if ( sdl->useglsl )
	{
		glsl_shader_free(sdl->glsl);
		sdl->glsl = NULL;
	}

	sdl->initialized = 0;

	if (lock)
		window->primlist->release_lock();
}

//============================================================
//  TEXCOPY FUNCS
//============================================================

static void drawogl_window_clear(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	//FIXME: Handled in drawogl_window_draw as well
	sdl->blittimer = 3;
}


//============================================================
//  TEXCOPY FUNCS
//============================================================

#include "texcopy.c"
