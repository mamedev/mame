//============================================================
//
//  drawsdl.c - SDL software and OpenGL implementation
//
//  Copyright (c) 1996-2011, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

// standard C headers
#include <math.h>
#include <stdio.h>

// MAME headers
#include "emu.h"
#include "ui/ui.h"
#include "rendersw.inc"

// standard SDL headers
#include "sdlinc.h"

// OSD headers
#include "osdsdl.h"
#include "window.h"

//============================================================
//  DEBUGGING
//============================================================

//============================================================
//  CONSTANTS
//============================================================

//============================================================
//  TYPES
//============================================================

struct sdl_scale_mode;

#if (SDLMAME_SDL2)
#define DRAW2_SCALEMODE_NEAREST "0"
#define DRAW2_SCALEMODE_LINEAR  "1"
#define DRAW2_SCALEMODE_BEST    "2"
#endif

/* sdl_info is the information about SDL for the current screen */
class sdl_info : public osd_renderer
{
public:

	sdl_info(sdl_window_info *w)
	: osd_renderer(w, FLAG_NONE),
	m_blittimer(0),
	m_extra_flags(0),

	#if (SDLMAME_SDL2)
	m_sdl_renderer(NULL),
	m_texture_id(NULL),
	#else
	m_sdlsurf(NULL),
	m_yuvsurf(NULL),
	#endif
	m_yuv_lookup(NULL),
	m_yuv_bitmap(NULL),
	//m_hw_scale_width(0),
	//m_hw_scale_height(0),
	m_last_hofs(0),
	m_last_vofs(0),
	m_old_blitwidth(0),
	m_old_blitheight(0)
	{ }

	/* virtual */ int create(const int width, const int height);
	/* virtual */ void resize(const int width, const int height);
	/* virtual */ int draw(const UINT32 dc, const int update);
	/* virtual */ int xy_to_render_target(const int x, const int y, int *xt, int *yt);
	/* virtual */ void destroy_all_textures();
	/* virtual */ void destroy();
	/* virtual */ void clear();

	void yuv_init();
#if (SDLMAME_SDL2)
	void setup_texture(int tempwidth, int tempheight);
#endif
	void yuv_lookup_set(unsigned int pen, unsigned char red,
				unsigned char green, unsigned char blue);

#if (!SDLMAME_SDL2)
	void yuv_overlay_init();
#endif

	INT32               m_blittimer;
	UINT32              m_extra_flags;

#if (SDLMAME_SDL2)
	// Original display_mode
	SDL_DisplayMode 	m_original_mode;

	SDL_Renderer        *m_sdl_renderer;
	SDL_Texture         *m_texture_id;
#else
	// SDL surface
	SDL_Surface         *m_sdlsurf;
	SDL_Overlay         *m_yuvsurf;
#endif

	// YUV overlay
	UINT32              *m_yuv_lookup;
	UINT16              *m_yuv_bitmap;

	// if we leave scaling to SDL and the underlying driver, this
	// is the render_target_width/height to use

	int                 m_last_hofs;
	int                 m_last_vofs;
	int                 m_old_blitwidth;
	int                 m_old_blitheight;
};

struct sdl_scale_mode
{
	const char      *name;
	int             is_scale;           /* Scale mode?           */
	int             is_yuv;             /* Yuv mode?             */
	int             mult_w;             /* Width multiplier      */
	int             mult_h;             /* Height multiplier     */
#if (!SDLMAME_SDL2)
	int             m_extra_flags;        /* Texture/surface flags */
#else
	const char      *sdl_scale_mode;        /* what to use as a hint ? */
#endif
	int             pixel_format;       /* Pixel/Overlay format  */
	void            (*yuv_blit)(const UINT16 *bitmap, UINT8 *ptr, const int pitch, const UINT32 *lookup, const int width, const int height);
};

//============================================================
//  INLINES
//============================================================

//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawsdl_exit(void);

// YUV overlays

static void yuv_RGB_to_YV12(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height);
static void yuv_RGB_to_YV12X2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height);
static void yuv_RGB_to_YUY2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height);
static void yuv_RGB_to_YUY2X2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height);

// Static declarations

#if (!SDLMAME_SDL2)
static int shown_video_info = 0;

static const sdl_scale_mode scale_modes[] =
{
		{ "none",    0, 0, 1, 1, SDL_DOUBLEBUF, 0, 0 },
		{ "async",   0, 0, 1, 1, SDL_DOUBLEBUF | SDL_ASYNCBLIT, 0, 0 },
		{ "yv12",    1, 1, 1, 1, 0,              SDL_YV12_OVERLAY, yuv_RGB_to_YV12 },
		{ "yv12x2",  1, 1, 2, 2, 0,              SDL_YV12_OVERLAY, yuv_RGB_to_YV12X2 },
		{ "yuy2",    1, 1, 1, 1, 0,              SDL_YUY2_OVERLAY, yuv_RGB_to_YUY2 },
		{ "yuy2x2",  1, 1, 2, 1, 0,              SDL_YUY2_OVERLAY, yuv_RGB_to_YUY2X2 },
		{ NULL }
};
#else
static const sdl_scale_mode scale_modes[] =
{
		{ "none",    0, 0, 1, 1, DRAW2_SCALEMODE_NEAREST, 0, 0 },
		{ "hwblit",  1, 0, 1, 1, DRAW2_SCALEMODE_LINEAR, 0, 0 },
		{ "hwbest",  1, 0, 1, 1, DRAW2_SCALEMODE_BEST, 0, 0 },
		{ "yv12",    1, 1, 1, 1, DRAW2_SCALEMODE_NEAREST, SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12 },
		{ "yv12x2",  1, 1, 2, 2, DRAW2_SCALEMODE_NEAREST, SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12X2 },
		{ "yuy2",    1, 1, 1, 1, DRAW2_SCALEMODE_NEAREST, SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2 },
		{ "yuy2x2",  1, 1, 2, 1, DRAW2_SCALEMODE_NEAREST, SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2X2 },
		{ NULL }
};
#endif

//============================================================
//  drawsdl_scale_mode
//============================================================

const char *drawsdl_scale_mode_str(int index)
{
	const sdl_scale_mode *sm = scale_modes;

	while (index>0)
	{
		if (sm->name == NULL)
			return NULL;
		index--;
		sm++;
	}
	return sm->name;
};

int drawsdl_scale_mode(const char *s)
{
	const sdl_scale_mode *sm = scale_modes;
	int index;

	index = 0;
	while (sm->name != NULL)
	{
		if (strcmp(sm->name, s) == 0)
			return index;
		index++;
		sm++;
	}
	return -1;
}

//============================================================
//  drawsdl_init
//============================================================

static osd_renderer *drawsdl_create(sdl_window_info *window)
{
	return global_alloc(sdl_info(window));
}


int drawsdl_init(sdl_draw_info *callbacks)
{
	// fill in the callbacks
	callbacks->create = drawsdl_create;
	callbacks->exit = drawsdl_exit;

	if (SDLMAME_SDL2)
		osd_printf_verbose("Using SDL multi-window soft driver (SDL 2.0+)\n");
	else
		osd_printf_verbose("Using SDL single-window soft driver (SDL 1.2)\n");

	return 0;
}

//============================================================
//  drawsdl_exit
//============================================================

static void drawsdl_exit(void)
{
}

//============================================================
//  drawsdl_destroy_all_textures
//============================================================

void sdl_info::destroy_all_textures()
{
	/* nothing to be done in soft mode */
}

//============================================================
//  setup_texture for window
//============================================================

#if (SDLMAME_SDL2)
void sdl_info::setup_texture(int tempwidth, int tempheight)
{
	const sdl_scale_mode *sdl_sm = &scale_modes[video_config.scale_mode];
	SDL_DisplayMode mode;
	UINT32 fmt;

	// Determine preferred pixelformat and set up yuv if necessary
	SDL_GetCurrentDisplayMode(window().monitor()->handle(), &mode);

	if (m_yuv_bitmap)
	{
		global_free_array(m_yuv_bitmap);
		m_yuv_bitmap = NULL;
	}

	fmt = (sdl_sm->pixel_format ? sdl_sm->pixel_format : mode.format);

	if (sdl_sm->is_scale)
	{
		int m_hw_scale_width =0;
		int m_hw_scale_height = 0;

		window().m_target->compute_minimum_size(m_hw_scale_width, m_hw_scale_height);
		if (video_config.prescale)
		{
			m_hw_scale_width *= video_config.prescale;
			m_hw_scale_height *= video_config.prescale;

			/* This must be a multiple of 2 */
			m_hw_scale_width = (m_hw_scale_width + 1) & ~1;
		}
		if (sdl_sm->is_yuv)
			m_yuv_bitmap = global_alloc_array(UINT16, m_hw_scale_width * m_hw_scale_height);

		int w = m_hw_scale_width * sdl_sm->mult_w;
		int h = m_hw_scale_height * sdl_sm->mult_h;

		m_texture_id = SDL_CreateTexture(m_sdl_renderer, fmt, SDL_TEXTUREACCESS_STREAMING, w, h);

	}
	else
	{
		m_texture_id = SDL_CreateTexture(m_sdl_renderer,fmt, SDL_TEXTUREACCESS_STREAMING,
				tempwidth, tempheight);
	}
}
#endif

//============================================================
//  yuv_overlay_init
//============================================================

#if (!SDLMAME_SDL2)
void sdl_info::yuv_overlay_init()
{
	const sdl_scale_mode *sdl_sm = &scale_modes[video_config.scale_mode];
	int minimum_width, minimum_height;

	window().m_target->compute_minimum_size(minimum_width, minimum_height);

	if (video_config.prescale)
	{
		minimum_width *= video_config.prescale;
		minimum_height *= video_config.prescale;
	}

	if (m_yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(m_yuvsurf);
		m_yuvsurf = NULL;
	}

	if (m_yuv_bitmap != NULL)
	{
		global_free_array(m_yuv_bitmap);
	}

	osd_printf_verbose("SDL: Creating %d x %d YUV-Overlay ...\n", minimum_width, minimum_height);

	m_yuv_bitmap = global_alloc_array(UINT16, minimum_width*minimum_height);

	m_yuvsurf = SDL_CreateYUVOverlay(minimum_width * sdl_sm->mult_w, minimum_height * sdl_sm->mult_h,
			sdl_sm->pixel_format, m_sdlsurf);

	if ( m_yuvsurf == NULL ) {
		osd_printf_error("SDL: Couldn't create SDL_yuv_overlay: %s\n", SDL_GetError());
		//return 1;
	}

	if (!shown_video_info)
	{
		osd_printf_verbose("YUV Mode         : %s\n", sdl_sm->name);
		osd_printf_verbose("YUV Overlay Size : %d x %d\n", minimum_width, minimum_height);
		osd_printf_verbose("YUV Acceleration : %s\n", m_yuvsurf->hw_overlay ? "Hardware" : "Software");
		shown_video_info = 1;
	}
}
#endif

//============================================================
//  drawsdl_show_info
//============================================================

#if (SDLMAME_SDL2)
static void drawsdl_show_info(struct SDL_RendererInfo *render_info)
{
#define RF_ENTRY(x) {x, #x }
	static struct {
		int flag;
		const char *name;
	} rflist[] =
		{
#if 0
			RF_ENTRY(SDL_RENDERER_SINGLEBUFFER),
			RF_ENTRY(SDL_RENDERER_PRESENTCOPY),
			RF_ENTRY(SDL_RENDERER_PRESENTFLIP2),
			RF_ENTRY(SDL_RENDERER_PRESENTFLIP3),
			RF_ENTRY(SDL_RENDERER_PRESENTDISCARD),
#endif
			RF_ENTRY(SDL_RENDERER_PRESENTVSYNC),
			RF_ENTRY(SDL_RENDERER_ACCELERATED),
			{-1, NULL}
		};
	int i;

	osd_printf_verbose("window: using renderer %s\n", render_info->name ? render_info->name : "<unknown>");
	for (i = 0; rflist[i].name != NULL; i++)
		if (render_info->flags & rflist[i].flag)
			osd_printf_verbose("renderer: flag %s\n", rflist[i].name);
}
#endif

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

int sdl_info::create(int width, int height)
{
	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */
	const sdl_scale_mode *sm = &scale_modes[video_config.scale_mode];

	osd_printf_verbose("Enter sdl_info::create\n");

#if (SDLMAME_SDL2)

	if (check_flag(FLAG_NEEDS_OPENGL))
	{
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		/* FIXME: A reminder that gamma is wrong throughout MAME. Currently, SDL2.0 doesn't seem to
			* support the following attribute although my hardware lists GL_ARB_framebuffer_sRGB as an extension.
			*
			* SDL_GL_SetAttribute( SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1 );
			*
			*/
		m_extra_flags = SDL_WINDOW_OPENGL;
	}
	else
				m_extra_flags = 0;

	/* set hints ... */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, sm->sdl_scale_mode);

	// create the SDL window
	// soft driver also used | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_MOUSE_FOCUS
	m_extra_flags |= (window().fullscreen() ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

#if defined(SDLMAME_WIN32)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#endif
	// create the SDL window
	window().m_sdl_window = SDL_CreateWindow(window().m_title,
			window().monitor()->position_size().x, window().monitor()->position_size().y,
			width, height, m_extra_flags);
	//window().m_sdl_window = SDL_CreateWindow(window().m_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	//		width, height, m_extra_flags);

	if  (!window().m_sdl_window )
	{
		if (check_flag(FLAG_NEEDS_OPENGL))
			osd_printf_error("OpenGL not supported on this driver: %s\n", SDL_GetError());
		else
		osd_printf_error("Window creation failed: %s\n", SDL_GetError());
		return 1;
	}

	if (window().fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		//SDL_GetCurrentDisplayMode(window().monitor()->handle, &mode);
		SDL_GetWindowDisplayMode(window().m_sdl_window, &mode);
		m_original_mode = mode;
		mode.w = width;
		mode.h = height;
		if (window().m_refresh)
			mode.refresh_rate = window().m_refresh;

		SDL_SetWindowDisplayMode(window().m_sdl_window, &mode);    // Try to set mode
#ifndef SDLMAME_WIN32
		/* FIXME: Warp the mouse to 0,0 in case a virtual desktop resolution
		 * is in place after the mode switch - which will most likely be the case
		 * This is a hack to work around a deficiency in SDL2
		 */
		SDL_WarpMouseInWindow(window().m_sdl_window, 1, 1);
#endif
	}
	else
	{
		//SDL_SetWindowDisplayMode(window().m_sdl_window, NULL); // Use desktop
	}

	// show window

	SDL_ShowWindow(window().m_sdl_window);
	//SDL_SetWindowFullscreen(window().m_sdl_window, window().fullscreen);
	SDL_RaiseWindow(window().m_sdl_window);

	SDL_GetWindowSize(window().m_sdl_window, &window().m_width, &window().m_height);

	// create renderer 

	if (video_config.waitvsync)
		m_sdl_renderer = SDL_CreateRenderer(window().m_sdl_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		m_sdl_renderer = SDL_CreateRenderer(window().m_sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (!m_sdl_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	{
		struct SDL_RendererInfo render_info;

		SDL_GetRendererInfo(m_sdl_renderer, &render_info);
		drawsdl_show_info(&render_info);

		// Check scale mode

		if (sm->pixel_format)
		{
			int i;
			int found = 0;

			for (i=0; i < render_info.num_texture_formats; i++)
				if (sm->pixel_format == render_info.texture_formats[i])
					found = 1;

			if (!found)
			{
				fatalerror("window: Scale mode %s not supported!", sm->name);
			}
		}
	}

	setup_texture(width, height);
#else
	m_extra_flags = (window().fullscreen() ?  SDL_FULLSCREEN : SDL_RESIZABLE);

	m_extra_flags |= sm->m_extra_flags;

	if (check_flag(FLAG_NEEDS_OPENGL))
	{
		m_extra_flags |= SDL_DOUBLEBUF | SDL_OPENGL;
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		#if (SDL_VERSION_ATLEAST(1,2,10)) && (!defined(SDLMAME_EMSCRIPTEN))
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, video_config.waitvsync ? 1 : 0);
		#endif
		//load_gl_lib(window().machine());
	}

	// create the SDL surface (which creates the window in windowed mode)
	m_sdlsurf = SDL_SetVideoMode(width, height,
					0, SDL_SWSURFACE | SDL_ANYFORMAT | m_extra_flags);

	if (!m_sdlsurf)
		return 1;
	if ( (video_config.mode  == VIDEO_MODE_OPENGL) && !(m_sdlsurf->flags & SDL_OPENGL) )
	{
		osd_printf_error("OpenGL not supported on this driver!\n");
		return 1;
	}

	window().m_width = m_sdlsurf->w;
	window().m_height = m_sdlsurf->h;
	if (sm->is_yuv)
		yuv_overlay_init();

	// set the window title
	SDL_WM_SetCaption(window().m_title, "SDLMAME");

#endif

	m_yuv_lookup = NULL;
	m_blittimer = 0;

	yuv_init();
	osd_printf_verbose("Leave sdl_info::create\n");
	return 0;
}

//============================================================
//  sdl_info::resize
//============================================================

void sdl_info::resize(int width, int height)
{
#if (!SDLMAME_SDL2)
	const sdl_scale_mode *sdl_sm = &scale_modes[video_config.scale_mode];
#endif
#if (SDLMAME_SDL2)
	SDL_SetWindowSize(window().m_sdl_window, width, height);
	SDL_GetWindowSize(window().m_sdl_window, &window().m_width, &window().m_height);

#else
	if (m_yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(m_yuvsurf);
		m_yuvsurf = NULL;
	}
	SDL_FreeSurface(m_sdlsurf);
	
	m_sdlsurf = SDL_SetVideoMode(width, height, 0,
			SDL_SWSURFACE | SDL_ANYFORMAT | m_extra_flags);

	window().m_width = m_sdlsurf->w;
	window().m_height = m_sdlsurf->h;

		if (sdl_sm->is_yuv)
	{
		yuv_overlay_init();
	}

#endif
}


//============================================================
//  sdl_info::destroy
//============================================================

void sdl_info::destroy()
{

#if (SDLMAME_SDL2)
	//SDL_SelectRenderer(window().sdl_window);
	SDL_DestroyTexture(m_texture_id);
	//SDL_DestroyRenderer(window().sdl_window);
	if (window().fullscreen() && video_config.switchres)
	{
		SDL_SetWindowFullscreen(window().m_sdl_window, 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window().m_sdl_window, &m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window().m_sdl_window, SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}

	SDL_DestroyWindow(window().m_sdl_window);
#else
	if (m_yuvsurf != NULL)
	{
		SDL_FreeYUVOverlay(m_yuvsurf);
		m_yuvsurf = NULL;
	}

	if (m_sdlsurf)
	{
		SDL_FreeSurface(m_sdlsurf);
		m_sdlsurf = NULL;
	}
#endif
	// free the memory in the window

	if (m_yuv_lookup != NULL)
	{
		global_free_array(m_yuv_lookup);
		m_yuv_lookup = NULL;
	}
	if (m_yuv_bitmap != NULL)
	{
		global_free_array(m_yuv_bitmap);
		m_yuv_bitmap = NULL;
	}
}

//============================================================
//  sdl_info::clear
//============================================================

void sdl_info::clear()
{
	//FIXME: Handled in sdl_info::draw as well
	m_blittimer = 3;
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

int sdl_info::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= window().m_blitwidth)
		return 0;
	if (*yt<0 || *yt >= window().m_blitheight)
		return 0;
	return 1;
}

//============================================================
//  sdl_info::draw
//============================================================

int sdl_info::draw(UINT32 dc, int update)
{
	const sdl_scale_mode *sm = &scale_modes[video_config.scale_mode];
	UINT8 *surfptr;
	INT32 pitch;
	Uint32 rmask, gmask, bmask;
#if (SDLMAME_SDL2)
	Uint32 amask;
#endif
	INT32 vofs, hofs, blitwidth, blitheight, ch, cw;
	int bpp;

	if (video_config.novideo)
	{
		return 0;
	}

	// lock it if we need it
#if (!SDLMAME_SDL2)

	pitch = m_sdlsurf->pitch;
	bpp = m_sdlsurf->format->BytesPerPixel;
	rmask = m_sdlsurf->format->Rmask;
	gmask = m_sdlsurf->format->Gmask;
	bmask = m_sdlsurf->format->Bmask;
//  amask = sdlsurf->format->Amask;

	if (window().m_blitwidth != m_old_blitwidth || window().m_blitheight != m_old_blitheight)
	{
		if (sm->is_yuv)
			yuv_overlay_init();
		m_old_blitwidth = window().m_blitwidth;
		m_old_blitheight = window().m_blitheight;
		m_blittimer = 3;
	}

	if (SDL_MUSTLOCK(m_sdlsurf))
		SDL_LockSurface(m_sdlsurf);

	// Clear if necessary
	if (m_blittimer > 0)
	{
		memset(m_sdlsurf->pixels, 0, window().m_height * m_sdlsurf->pitch);
		m_blittimer--;
	}


	if (sm->is_yuv)
	{
		SDL_LockYUVOverlay(m_yuvsurf);
		surfptr = m_yuvsurf->pixels[0]; // (UINT8 *) m_yuv_bitmap;
		pitch = m_yuvsurf->pitches[0]; // (UINT8 *) m_yuv_bitmap;
#if 0
		printf("abcd %d\n", m_yuvsurf->h);
		printf("abcd %d %d %d\n", m_yuvsurf->pitches[0], m_yuvsurf->pitches[1], m_yuvsurf->pitches[2]);
		printf("abcd %p %p %p\n", m_yuvsurf->pixels[0], m_yuvsurf->pixels[1], m_yuvsurf->pixels[2]);
		printf("abcd %ld %ld\n", m_yuvsurf->pixels[1] - m_yuvsurf->pixels[0], m_yuvsurf->pixels[2] - m_yuvsurf->pixels[1]);
#endif
	}
	else
		surfptr = (UINT8 *)m_sdlsurf->pixels;
#else
	//SDL_SelectRenderer(window().sdl_window);

	if (window().m_blitwidth != m_old_blitwidth || window().m_blitheight != m_old_blitheight)
	{
		SDL_RenderSetViewport(m_sdl_renderer, NULL);

		SDL_DestroyTexture(m_texture_id);
		setup_texture(window().m_blitwidth, window().m_blitheight);
		m_old_blitwidth = window().m_blitwidth;
		m_old_blitheight = window().m_blitheight;
		m_blittimer = 3;
	}

	{
		Uint32 format;
		int access, w, h;

		SDL_QueryTexture(m_texture_id, &format, &access, &w, &h);
		SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask);
		bpp = bpp / 8; /* convert to bytes per pixels */
	}

	// Clear if necessary
	if (m_blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawColor(m_sdl_renderer,0,0,0,0);
		SDL_RenderFillRect(m_sdl_renderer,NULL);
		//SDL_RenderFill(0,0,0,0 /*255*/,NULL);
		m_blittimer--;
	}

	SDL_LockTexture(m_texture_id, NULL, (void **) &surfptr, &pitch);

#endif
	// get ready to center the image
	vofs = hofs = 0;
	blitwidth = window().m_blitwidth;
	blitheight = window().m_blitheight;

	// figure out what coordinate system to use for centering - in window mode it's always the
	// SDL surface size.  in fullscreen the surface covers all monitors, so center according to
	// the first one only
	if ((window().fullscreen()) && (!video_config.switchres))
	{
		ch = window().monitor()->center_height();
		cw = window().monitor()->center_width();
	}
	else
	{
		ch = window().m_height;
		cw = window().m_width;
	}

	// do not crash if the window's smaller than the blit area
	if (blitheight > ch)
	{
			blitheight = ch;
	}
	else if (video_config.centerv)
	{
		vofs = (ch - window().m_blitheight) / 2;
	}

	if (blitwidth > cw)
	{
			blitwidth = cw;
	}
	else if (video_config.centerh)
	{
		hofs = (cw - window().m_blitwidth) / 2;
	}

	m_last_hofs = hofs;
	m_last_vofs = vofs;

	window().m_primlist->acquire_lock();

	int mamewidth, mameheight;

#if !SDLMAME_SDL2
		if (!sm->is_yuv)
		{
			surfptr += ((vofs * pitch) + (hofs * bpp));
			mamewidth = blitwidth; //m_sdlsurf->w;
			mameheight = blitheight; //m_sdlsurf->h;
		}
		else
		{
			mamewidth = m_yuvsurf->w / sm->mult_w;
			mameheight = m_yuvsurf->h / sm->mult_h;
		}
#else
		Uint32 fmt = 0;
		int access = 0;
		SDL_QueryTexture(m_texture_id, &fmt, &access, &mamewidth, &mameheight);
		mamewidth /= sm->mult_w;
		mameheight /= sm->mult_h;
#endif
	//printf("w h %d %d %d %d\n", mamewidth, mameheight, blitwidth, blitheight);

	// rescale bounds
	float fw = (float) mamewidth / (float) blitwidth;
	float fh = (float) mameheight / (float) blitheight;

	// FIXME: this could be a lot easier if we get the primlist here!
	//			Bounds would be set fit for purpose and done!

	for (render_primitive *prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
	{
		prim->bounds.x0 *= fw;
		prim->bounds.x1 *= fw;
		prim->bounds.y0 *= fh;
		prim->bounds.y1 *= fh;
	}

	// render to it
	if (!sm->is_yuv)
	{
		switch (rmask)
		{
			case 0x0000ff00:
				software_renderer<UINT32, 0,0,0, 8,16,24>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x00ff0000:
				software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x000000ff:
				software_renderer<UINT32, 0,0,0, 0,8,16>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0xf800:
				software_renderer<UINT16, 3,2,3, 11,5,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			case 0x7c00:
				software_renderer<UINT16, 3,3,3, 10,5,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			default:
				osd_printf_error("SDL: ERROR! Unknown video mode: R=%08X G=%08X B=%08X\n", rmask, gmask, bmask);
				break;
		}
	}
	else
	{
		assert (m_yuv_bitmap != NULL);
		assert (surfptr != NULL);
		software_renderer<UINT16, 3,3,3, 10,5,0>::draw_primitives(*window().m_primlist, m_yuv_bitmap, mamewidth, mameheight, mamewidth);
		sm->yuv_blit((UINT16 *)m_yuv_bitmap, surfptr, pitch, m_yuv_lookup, mamewidth, mameheight);
	}

	window().m_primlist->release_lock();

	// unlock and flip
#if (!SDLMAME_SDL2)
	if (SDL_MUSTLOCK(m_sdlsurf)) SDL_UnlockSurface(m_sdlsurf);
	if (!sm->is_yuv)
	{
		SDL_Flip(m_sdlsurf);
	}
	else
	{
		SDL_Rect r;

		SDL_UnlockYUVOverlay(m_yuvsurf);
		r.x = hofs;
		r.y = vofs;
		r.w = blitwidth;
		r.h = blitheight;
		SDL_DisplayYUVOverlay(m_yuvsurf, &r);
	}
#else
	SDL_UnlockTexture(m_texture_id);
	{
		SDL_Rect r;

		r.x=hofs;
		r.y=vofs;
		r.w=blitwidth;
		r.h=blitheight;
		//printf("blitwidth %d %d - %d %d\n", blitwidth, blitheight, window().width, window().height);
		//SDL_UpdateTexture(sdltex, NULL, sdlsurf->pixels, pitch);
		SDL_RenderCopy(m_sdl_renderer,m_texture_id, NULL, &r);
		SDL_RenderPresent(m_sdl_renderer);
	}
#endif
	return 0;
}
//============================================================
// YUV Blitting
//============================================================

#define CU_CLAMP(v, a, b)   ((v < a)? a: ((v > b)? b: v))
#define RGB2YUV_F(r,g,b,y,u,v) \
		(y) = (0.299*(r) + 0.587*(g) + 0.114*(b) ); \
		(u) = (-0.169*(r) - 0.331*(g) + 0.5*(b) + 128); \
		(v) = (0.5*(r) - 0.419*(g) - 0.081*(b) + 128); \
	(y) = CU_CLAMP(y,0,255); \
	(u) = CU_CLAMP(u,0,255); \
	(v) = CU_CLAMP(v,0,255)

#define RGB2YUV(r,g,b,y,u,v) \
		(y) = ((  8453*(r) + 16594*(g) +  3223*(b) +  524288) >> 15); \
		(u) = (( -4878*(r) -  9578*(g) + 14456*(b) + 4210688) >> 15); \
		(v) = (( 14456*(r) - 12105*(g) -  2351*(b) + 4210688) >> 15)

#ifdef LSB_FIRST
#define Y1MASK  0x000000FF
#define  UMASK  0x0000FF00
#define Y2MASK  0x00FF0000
#define  VMASK  0xFF000000
#define Y1SHIFT  0
#define  USHIFT  8
#define Y2SHIFT 16
#define  VSHIFT 24
#else
#define Y1MASK  0xFF000000
#define  UMASK  0x00FF0000
#define Y2MASK  0x0000FF00
#define  VMASK  0x000000FF
#define Y1SHIFT 24
#define  USHIFT 16
#define Y2SHIFT  8
#define  VSHIFT  0
#endif

#define YMASK  (Y1MASK|Y2MASK)
#define UVMASK (UMASK|VMASK)

void sdl_info::yuv_lookup_set(unsigned int pen, unsigned char red,
			unsigned char green, unsigned char blue)
{
	UINT32 y,u,v;

	RGB2YUV(red,green,blue,y,u,v);

	/* Storing this data in YUYV order simplifies using the data for
	   YUY2, both with and without smoothing... */
	m_yuv_lookup[pen]=(y<<Y1SHIFT)|(u<<USHIFT)|(y<<Y2SHIFT)|(v<<VSHIFT);
}

void sdl_info::yuv_init()
{
	unsigned char r,g,b;
	if (m_yuv_lookup == NULL)
		m_yuv_lookup = global_alloc_array(UINT32, 65536);
	for (r = 0; r < 32; r++)
		for (g = 0; g < 32; g++)
			for (b = 0; b < 32; b++)
			{
				int idx = (r << 10) | (g << 5) | b;
				yuv_lookup_set(idx,
					(r << 3) | (r >> 2),
					(g << 3) | (g >> 2),
					(b << 3) | (b >> 2));
			}
}

//UINT32 *lookup = sdl->m_yuv_lookup;

static void yuv_RGB_to_YV12(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height)
{
	int x, y;
	UINT8 *pixels[3];
	int u1,v1,y1,u2,v2,y2,u3,v3,y3,u4,v4,y4;      /* 12 */

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * height;
	pixels[2] = pixels[1] + pitch * height / 4;

	for(y=0;y<height;y+=2)
	{
		const UINT16 *src=bitmap + (y * width) ;
		const UINT16 *src2=src + width;

		UINT8 *dest_y = pixels[0] + y * pitch;
		UINT8 *dest_v = pixels[1] + (y>>1) * pitch / 2;
		UINT8 *dest_u = pixels[2] + (y>>1) * pitch / 2;

		for(x=0;x<width;x+=2)
		{
			v1 = lookup[src[x]];
			y1 = (v1>>Y1SHIFT) & 0xff;
			u1 = (v1>>USHIFT)  & 0xff;
			v1 = (v1>>VSHIFT)  & 0xff;

			v2 = lookup[src[x+1]];
			y2 = (v2>>Y1SHIFT) & 0xff;
			u2 = (v2>>USHIFT)  & 0xff;
			v2 = (v2>>VSHIFT)  & 0xff;

			v3 = lookup[src2[x]];
			y3 = (v3>>Y1SHIFT) & 0xff;
			u3 = (v3>>USHIFT)  & 0xff;
			v3 = (v3>>VSHIFT)  & 0xff;

			v4 = lookup[src2[x+1]];
			y4 = (v4>>Y1SHIFT) & 0xff;
			u4 = (v4>>USHIFT)  & 0xff;
			v4 = (v4>>VSHIFT)  & 0xff;

			dest_y[x] = y1;
			dest_y[x+pitch] = y3;
			dest_y[x+1] = y2;
			dest_y[x+pitch+1] = y4;

			dest_u[x>>1] = (u1+u2+u3+u4)/4;
			dest_v[x>>1] = (v1+v2+v3+v4)/4;

		}
	}
}

static void yuv_RGB_to_YV12X2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int x,y;
	int u1,v1,y1;
	UINT8 *pixels[3];

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * height * 2;
#if (SDLMAME_SDL2)
	int p2 = (pitch >> 1);
#else
	int p2 = (pitch + 7) & ~ 7;;
	p2 = (p2 >> 1);
#endif
	pixels[2] = pixels[1] + p2 * height;

	for(y=0;y<height;y++)
	{
		const UINT16 *src = bitmap + (y * width) ;

		UINT16 *dest_y = (UINT16 *)(pixels[0] + 2 * y * pitch);
		UINT8 *dest_v = pixels[1] + y * p2;
		UINT8 *dest_u = pixels[2] + y * p2;
		for(x=0;x<width;x++)
		{
			v1 = lookup[src[x]];
			y1 = (v1 >> Y1SHIFT) & 0xff;
			u1 = (v1 >> USHIFT)  & 0xff;
			v1 = (v1 >> VSHIFT)  & 0xff;

			dest_y[x + pitch/2] = y1 << 8 | y1;
			dest_y[x] = y1 << 8 | y1;
			dest_u[x] = u1;
			dest_v[x] = v1;
		}
	}
}

static void yuv_RGB_to_YUY2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int y;
	UINT32 p1,p2,uv;
	const int yuv_pitch = pitch/4;

	for(y=0;y<height;y++)
	{
		const UINT16 *src=bitmap + (y * width) ;
		const UINT16 *end=src+width;

		UINT32 *dest = (UINT32 *) ptr;
		dest += y * yuv_pitch;
		for(; src<end; src+=2)
		{
			p1  = lookup[src[0]];
			p2  = lookup[src[1]];
			uv = (p1&UVMASK)>>1;
			uv += (p2&UVMASK)>>1;
			*dest++ = (p1&Y1MASK)|(p2&Y2MASK)|(uv&UVMASK);
		}
	}
}

static void yuv_RGB_to_YUY2X2(const UINT16 *bitmap, UINT8 *ptr, const int pitch, \
		const UINT32 *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int y;
	int yuv_pitch = pitch / 4;

	for(y=0;y<height;y++)
	{
		const UINT16 *src=bitmap + (y * width) ;
		const UINT16 *end=src+width;

		UINT32 *dest = (UINT32 *) ptr;
		dest += (y * yuv_pitch);
		for(; src<end; src++)
		{
			dest[0] = lookup[src[0]];
			dest++;
		}
	}
}
