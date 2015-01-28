//============================================================
//
//  draw13.c - SDL 2.0 drawing implementation
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
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
#include "options.h"

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

#define STAT_PIXEL_THRESHOLD (150*150)

enum
{
	TEXTURE_TYPE_NONE,
	TEXTURE_TYPE_PLAIN,
	TEXTURE_TYPE_SURFACE
};


//============================================================
//  Inline functions
//============================================================

static inline bool is_opaque(const float &a)
{
	return (a >= 1.0f);
}

static inline bool is_transparent(const float &a)
{
	return (a <  0.0001f);
}

//============================================================
//  TYPES
//============================================================


struct quad_setup_data
{
	quad_setup_data()
	: dudx(0), dvdx(0), dudy(0), dvdy(0), startu(0), startv(0),
		rotwidth(0), rotheight(0)
	{}
	void compute(const render_primitive &prim);

	INT32           dudx, dvdx, dudy, dvdy;
	INT32           startu, startv;
	INT32           rotwidth, rotheight;
};

class texture_info;

typedef void (*texture_copy_func)(const texture_info *texture, const render_texinfo *texsource);

struct copy_info_t {
	int                 src_fmt;
	Uint32              dst_fmt;
	int                 dst_bpp;
	int                 rotate;
	texture_copy_func   func;
	Uint32              bm_mask;
	const char          *srcname;
	const char          *dstname;
	/* Statistics */
	UINT64              pixel_count;
	INT64               time;
	int                 samples;
	int                 perf;
	/* list */
	copy_info_t           *next;
};

//============================================================
//  Textures
//============================================================

struct sdl_info;

/* texture_info holds information about a texture */
class texture_info
{
	friend class simple_list<texture_info>;
public:
	texture_info(SDL_Renderer *renderer, const render_texinfo &texsource, const quad_setup_data &setup, const UINT32 flags);
	~texture_info();

	void set_data(const render_texinfo &texsource, const UINT32 flags);
	void render_quad(const render_primitive *prim, const int x, const int y);
	bool matches(const render_primitive &prim, const quad_setup_data &setup);

	copy_info_t *compute_size_type();

	void                *m_pixels;            // pixels for the texture
	int                 m_pitch;

	copy_info_t         *m_copyinfo;
	quad_setup_data     m_setup;

	osd_ticks_t         m_last_access;

	int raw_width() const { return m_texinfo.width; }
	int raw_height() const { return m_texinfo.height; }

	texture_info *next() { return m_next; }
	const render_texinfo &texinfo() const { return m_texinfo; }
	render_texinfo &texinfo() { return m_texinfo; }

	const HashT hash() const { return m_hash; }
	const UINT32 flags() const { return m_flags; }
	const bool is_pixels_owned() const { // do we own / allocated it ?
		return false && ((m_sdl_access == SDL_TEXTUREACCESS_STATIC)
				&& (m_copyinfo->func != NULL)) ;
	}

private:
	Uint32              m_sdl_access;
	SDL_Renderer *      m_renderer;
	render_texinfo      m_texinfo;            // copy of the texture info
	HashT               m_hash;               // hash value for the texture (must be >= pointer size)
	UINT32              m_flags;              // rendering flags

	SDL_Texture *       m_texture_id;
	int                 m_is_rotated;

	int                 m_format;             // texture format
	SDL_BlendMode       m_sdl_blendmode;

	texture_info *      m_next;               // next texture in the list
};

/* sdl_info is the information about SDL for the current screen */
struct sdl_info
{
	sdl_info()
	: m_blittimer(0), m_renderer(NULL),
		m_hofs(0), m_vofs(0),
		m_resize_pending(0), m_resize_width(0), m_resize_height(0),
		m_last_blit_time(0), m_last_blit_pixels(0)
	{}

	void render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y);

	texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
	texture_info *texture_update(const render_primitive &prim);

	INT32           m_blittimer;

	SDL_Renderer *  m_renderer;
	simple_list<texture_info>  m_texlist;                // list of active textures

	float           m_hofs;
	float           m_vofs;

	// resize information

	UINT8           m_resize_pending;
	UINT32          m_resize_width;
	UINT32          m_resize_height;

	// Stats
	INT64           m_last_blit_time;
	INT64           m_last_blit_pixels;

	// Original display_mode
	SDL_DisplayMode m_original_mode;
};

//============================================================
//  PROTOTYPES
//============================================================

// core functions

static void drawsdl2_exit(void);
static void drawsdl2_attach(sdl_draw_info *info, sdl_window_info *window);
static int drawsdl2_window_create(sdl_window_info *window, int width, int height);
static void drawsdl2_window_resize(sdl_window_info *window, int width, int height);
static void drawsdl2_window_destroy(sdl_window_info *window);
static int drawsdl2_window_draw(sdl_window_info *window, UINT32 dc, int update);
static void drawsdl2_set_target_bounds(sdl_window_info *window);
static void drawsdl2_destroy_all_textures(sdl_window_info *window);
static void drawsdl2_window_clear(sdl_window_info *window);
static int drawsdl2_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);

//============================================================
//  TEXCOPY FUNCS
//============================================================

#include "blit13.h"

//============================================================
//  STATIC VARIABLES
//============================================================

#define SDL_TEXFORMAT_LAST SDL_TEXFORMAT_PALETTE16A
#define BM_ALL (-1)
//( SDL_BLENDMODE_MASK | SDL_BLENDMODE_BLEND | SDL_BLENDMODE_ADD | SDL_BLENDMODE_MOD)

#define texcopy_NULL NULL
#define ENTRY(a,b,c,d,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, 0}
#define ENTRY_BM(a,b,c,d,f,bm) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, bm, #a, #b, 0, 0, 0, 0}
#define ENTRY_LR(a,b,c,d,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, c, d, texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, -1}

static copy_info_t blit_info_default[] =
{
	/* no rotation */
	ENTRY(ARGB32,           ARGB8888,   4, 0, NULL),
	ENTRY_LR(ARGB32,        RGB888,     4, 0, argb32_rgb32),
	/* Entry primarily for directfb */
	ENTRY_BM(ARGB32,        RGB888,     4, 0, argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,        RGB888,     4, 0, argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,        RGB888,     4, 0, argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,            ARGB8888,   4, 0, rgb32_argb32),
	ENTRY(RGB32,            RGB888,     4, 0, NULL),

	ENTRY(RGB32_PALETTED,   ARGB8888,   4, 0, rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,   RGB888,     4, 0, rgb32pal_argb32),

	ENTRY(YUY16,            UYVY,       2, 0, NULL /* yuv16_uyvy*/),
	ENTRY(YUY16,            YUY2,       2, 0, yuv16_yuy2),
	ENTRY(YUY16,            YVYU,       2, 0, yuv16_yvyu),
	ENTRY(YUY16,            ARGB8888,   4, 0, yuv16_argb32),
	ENTRY(YUY16,            RGB888,     4, 0, yuv16_argb32),

	ENTRY(YUY16_PALETTED,   UYVY,       2, 0, yuv16pal_uyvy),
	ENTRY(YUY16_PALETTED,   YUY2,       2, 0, yuv16pal_yuy2),
	ENTRY(YUY16_PALETTED,   YVYU,       2, 0, yuv16pal_yvyu),
	ENTRY(YUY16_PALETTED,   ARGB8888,   4, 0, yuv16pal_argb32),
	ENTRY(YUY16_PALETTED,   RGB888,     4, 0, yuv16pal_argb32),

	ENTRY(PALETTE16,        ARGB8888,   4, 0, pal16_argb32),
	ENTRY(PALETTE16,        RGB888,     4, 0, pal16_argb32),

	ENTRY(RGB15,            RGB555,     2, 0, NULL /* rgb15_argb1555 */),
	ENTRY(RGB15,            ARGB1555,   2, 0, rgb15_argb1555),
	ENTRY(RGB15,            ARGB8888,   4, 0, rgb15_argb32),
	ENTRY(RGB15,            RGB888,     4, 0, rgb15_argb32),

	ENTRY(RGB15_PALETTED,   ARGB8888,   4, 0, rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,   RGB888,     4, 0, rgb15pal_argb32),

	ENTRY(PALETTE16A,       ARGB8888,   4, 0, pal16a_argb32),
	ENTRY(PALETTE16A,       RGB888,     4, 0, pal16a_rgb32),

	/* rotation */
	ENTRY(ARGB32,           ARGB8888,   4, 1, rot_argb32_argb32),
	ENTRY_LR(ARGB32,        RGB888,     4, 1, rot_argb32_rgb32),
	/* Entry primarily for directfb */
	ENTRY_BM(ARGB32,        RGB888,     4, 1, rot_argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,        RGB888,     4, 1, rot_argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,        RGB888,     4, 1, rot_argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,            ARGB8888,   4, 1, rot_rgb32_argb32),
	ENTRY(RGB32,            RGB888,     4, 1, rot_argb32_argb32),

	ENTRY(RGB32_PALETTED,   ARGB8888,   4, 1, rot_rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,   RGB888,     4, 1, rot_rgb32pal_argb32),

	ENTRY(YUY16,            ARGB8888,   4, 1, rot_yuv16_argb32),
	ENTRY(YUY16,            RGB888,     4, 1, rot_yuv16_argb32),

	ENTRY(YUY16_PALETTED,   ARGB8888,   4, 1, rot_yuv16pal_argb32),
	ENTRY(YUY16_PALETTED,   RGB888,     4, 1, rot_yuv16pal_argb32),

	ENTRY(PALETTE16,        ARGB8888,   4, 1, rot_pal16_argb32),
	ENTRY(PALETTE16,        RGB888,     4, 1, rot_pal16_argb32),

	ENTRY(RGB15,            RGB555,     2, 1, rot_rgb15_argb1555),
	ENTRY(RGB15,            ARGB1555,   2, 1, rot_rgb15_argb1555),
	ENTRY(RGB15,            ARGB8888,   4, 1, rot_rgb15_argb32),
	ENTRY(RGB15,            RGB888,     4, 1, rot_rgb15_argb32),

	ENTRY(RGB15_PALETTED,   ARGB8888,   4, 1, rot_rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,   RGB888,     4, 1, rot_rgb15pal_argb32),

	ENTRY(PALETTE16A,       ARGB8888,   4, 1, rot_pal16a_argb32),
	ENTRY(PALETTE16A,       RGB888,     4, 1, rot_pal16a_rgb32),

{ -1 },
};

static copy_info_t *blit_info[SDL_TEXFORMAT_LAST+1];

static struct
{
	Uint32  format;
	int     status;
} fmt_support[30] = { { 0, 0 } };


//============================================================
//  INLINES
//============================================================


INLINE float round_nearest(float f)
{
	return floor(f + 0.5f);
}

INLINE HashT texture_compute_hash(const render_texinfo &texture, const UINT32 flags)
{
	return (HashT)texture.base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

INLINE SDL_BlendMode map_blendmode(const int blendmode)
{
	switch (blendmode)
	{
		case BLENDMODE_NONE:
			return SDL_BLENDMODE_NONE;
		case BLENDMODE_ALPHA:
			return SDL_BLENDMODE_BLEND;
		case BLENDMODE_RGB_MULTIPLY:
			return SDL_BLENDMODE_MOD;
		case BLENDMODE_ADD:
			return SDL_BLENDMODE_ADD;
		default:
			osd_printf_warning("Unknown Blendmode %d", blendmode);
	}
	return SDL_BLENDMODE_NONE;
}

INLINE void set_coloralphamode(SDL_Texture  *texture_id, const render_color *color)
{
	UINT32 sr = (UINT32)(255.0f * color->r);
	UINT32 sg = (UINT32)(255.0f * color->g);
	UINT32 sb = (UINT32)(255.0f * color->b);
	UINT32 sa = (UINT32)(255.0f * color->a);


	if (color->r >= 1.0f && color->g >= 1.0f && color->b >= 1.0f && is_opaque(color->a))
	{
		SDL_SetTextureColorMod(texture_id, 0xFF, 0xFF, 0xFF);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* coloring-only case */
	else if (is_opaque(color->a))
	{
		SDL_SetTextureColorMod(texture_id, sr, sg, sb);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* alpha and/or coloring case */
	else if (!is_transparent(color->a))
	{
		SDL_SetTextureColorMod(texture_id, sr, sg, sb);
		SDL_SetTextureAlphaMod(texture_id, sa);
	}
	else
	{
		SDL_SetTextureColorMod(texture_id, 0xFF, 0xFF, 0xFF);
		SDL_SetTextureAlphaMod(texture_id, 0x00);
	}
}

void texture_info::render_quad(const render_primitive *prim, const int x, const int y)
{
	SDL_Rect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim->bounds.x1 - prim->bounds.x0);
	target_rect.h = round_nearest(prim->bounds.y1 - prim->bounds.y0);

	SDL_SetTextureBlendMode(m_texture_id, m_sdl_blendmode);
	set_coloralphamode(m_texture_id, &prim->color);
	SDL_RenderCopy(m_renderer,  m_texture_id, NULL, &target_rect);
}

void sdl_info::render_quad(texture_info *texture, const render_primitive *prim, const int x, const int y)
{
	SDL_Rect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim->bounds.x1 - prim->bounds.x0);
	target_rect.h = round_nearest(prim->bounds.y1 - prim->bounds.y0);

	if (texture)
	{
		copy_info_t *copyinfo = texture->m_copyinfo;
		copyinfo->time -= osd_ticks();
		texture->render_quad(prim, x, y);
		copyinfo->time += osd_ticks();

		copyinfo->pixel_count += MAX(STAT_PIXEL_THRESHOLD , (texture->raw_width() * texture->raw_height()));
		if (m_last_blit_pixels)
		{
			copyinfo->time += (m_last_blit_time * (INT64) (texture->raw_width() * texture->raw_height())) / (INT64) m_last_blit_pixels;
		}
		copyinfo->samples++;
		copyinfo->perf = ( texture->m_copyinfo->pixel_count * (osd_ticks_per_second()/1000)) / texture->m_copyinfo->time;
	}
	else
	{
		UINT32 sr = (UINT32)(255.0f * prim->color.r);
		UINT32 sg = (UINT32)(255.0f * prim->color.g);
		UINT32 sb = (UINT32)(255.0f * prim->color.b);
		UINT32 sa = (UINT32)(255.0f * prim->color.a);

		SDL_SetRenderDrawBlendMode(m_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
		SDL_SetRenderDrawColor(m_renderer, sr, sg, sb, sa);
		SDL_RenderFillRect(m_renderer, &target_rect);
	}
}

static int RendererSupportsFormat(SDL_Renderer *renderer, Uint32 format, Uint32 access, const char *sformat)
{
	int i;
	SDL_Texture *texid;
	for (i=0; fmt_support[i].format != 0; i++)
	{
		if (format == fmt_support[i].format)
		{
			return fmt_support[i].status;
		}
	}
	/* not tested yet */
	fmt_support[i].format = format;
	fmt_support[i + 1].format = 0;
	texid = SDL_CreateTexture(renderer, format, access, 16, 16);
	if (texid)
	{
		fmt_support[i].status = 1;
		SDL_DestroyTexture(texid);
		return 1;
	}
	osd_printf_verbose("Pixelformat <%s> error %s \n", sformat, SDL_GetError());
	osd_printf_verbose("Pixelformat <%s> not supported\n", sformat);
	fmt_support[i].status = 0;
	return 0;
}

//============================================================
//  drawsdl2_init
//============================================================

static void add_list(copy_info_t **head, copy_info_t *element, Uint32 bm)
{
	copy_info_t *newci = global_alloc(copy_info_t);
	*newci = *element;

	newci->bm_mask = bm;
	newci->next = *head;
	*head = newci;
}

static void expand_copy_info(copy_info_t *list)
{
	copy_info_t   *bi;

	for (bi = list; bi->src_fmt != -1; bi++)
	{
		if (bi->bm_mask == BM_ALL)
		{
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_NONE);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_ADD);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_MOD);
			add_list(&blit_info[bi->src_fmt], bi, SDL_BLENDMODE_BLEND);
		}
		else
			add_list(&blit_info[bi->src_fmt], bi, bi->bm_mask);
	}
}

// FIXME: machine only used to access options.
int drawsdl2_init(running_machine &machine, sdl_draw_info *callbacks)
{
	const char *stemp;

	// fill in the callbacks
	callbacks->exit = drawsdl2_exit;
	callbacks->attach = drawsdl2_attach;

	osd_printf_verbose("Using SDL native texturing driver (SDL 2.0+)\n");

	expand_copy_info(blit_info_default);

#if USE_OPENGL
	// Load the GL library now - else MT will fail
	stemp = downcast<sdl_options &>(machine.options()).gl_lib();
#else
	stemp = NULL;
#endif
	if (stemp != NULL && strcmp(stemp, SDLOPTVAL_AUTO) == 0)
		stemp = NULL;

	// No fatalerror here since not all video drivers support GL !
	if (SDL_GL_LoadLibrary(stemp) != 0) // Load library (default for e==NULL
		osd_printf_warning("Warning: Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
	else
		osd_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");

	/* Enable bilinear filtering in case it is supported.
	 * This applies to all texture operations. However, artwort is pre-scaled
	 * and thus shouldn't be affected.
	 */
	if (video_config.filter)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	}
	else
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	}

	return 0;
}


//============================================================
//  drawsdl2_exit
//============================================================

static void drawsdl2_exit(void)
{
	int i;
	copy_info_t *bi, *freeme;
	for (i = 0; i <= SDL_TEXFORMAT_LAST; i++)
		for (bi = blit_info[i]; bi != NULL; )
		{
			if (bi->pixel_count)
				osd_printf_verbose("%s -> %s %s blendmode 0x%02x, %d samples: %d KPixel/sec\n", bi->srcname, bi->dstname,
						bi->rotate ? "rot" : "norot", bi->bm_mask, bi->samples,
						(int) bi->perf);
			freeme = bi;
			bi = bi->next;
			global_free(freeme);
		}
}

//============================================================
//  drawsdl2_attach
//============================================================

static void drawsdl2_attach(sdl_draw_info *info, sdl_window_info *window)
{
	// fill in the callbacks
	window->create = drawsdl2_window_create;
	window->resize = drawsdl2_window_resize;
	window->set_target_bounds = drawsdl2_set_target_bounds;
	window->draw = drawsdl2_window_draw;
	window->destroy = drawsdl2_window_destroy;
	window->destroy_all_textures = drawsdl2_destroy_all_textures;
	window->clear = drawsdl2_window_clear;
	window->xy_to_render_target = drawsdl2_xy_to_render_target;
}

//============================================================
//  drawsdl2_window_create
//============================================================

static int drawsdl2_window_create(sdl_window_info *window, int width, int height)
{
	// allocate memory for our structures
	sdl_info *sdl = global_alloc(sdl_info);

	/* FIXME: On Ubuntu and potentially other Linux OS you should use
	 * to disable panning. This has to be done before every invocation of mame.
	 *
	 * xrandr --output HDMI-0 --panning 0x0+0+0 --fb 0x0
	 *
	 */

	osd_printf_verbose("Enter drawsdl2_window_create\n");

	window->dxdata = sdl;

	UINT32 extra_flags = (window->fullscreen() ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

	// create the SDL window
	window->sdl_window = SDL_CreateWindow(window->title, window->monitor()->monitor_x, 0,
			width, height, extra_flags);

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		//SDL_GetCurrentDisplayMode(window->monitor()->handle, &mode);
		SDL_GetWindowDisplayMode(window->sdl_window, &mode);
		sdl->m_original_mode = mode;
		mode.w = width;
		mode.h = height;
		if (window->refresh)
			mode.refresh_rate = window->refresh;
		if (window->depth)
		{
			switch (window->depth)
			{
			case 15:
				mode.format = SDL_PIXELFORMAT_RGB555;
				break;
			case 16:
				mode.format = SDL_PIXELFORMAT_RGB565;
				break;
			case 24:
				mode.format = SDL_PIXELFORMAT_RGB24;
				break;
			case 32:
				mode.format = SDL_PIXELFORMAT_RGB888;
				break;
			default:
				osd_printf_warning("Ignoring depth %d\n", window->depth);
			}
		}
		SDL_SetWindowDisplayMode(window->sdl_window, &mode);    // Try to set mode
#ifndef SDLMAME_WIN32
		/* FIXME: Warp the mouse to 0,0 in case a virtual desktop resolution
		 * is in place after the mode switch - which will most likely be the case
		 * This is a hack to work around a deficiency in SDL2
		 */
		SDL_WarpMouseInWindow(window->sdl_window, 1, 1);
#endif
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL); // Use desktop

	// create renderer

	if (video_config.waitvsync)
		sdl->m_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		sdl->m_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (!sdl->m_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	//SDL_SelectRenderer(window->sdl_window);

	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->window_id, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);

	sdl->m_blittimer = 3;

	SDL_RenderPresent(sdl->m_renderer);
	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  drawsdl2_window_resize
//============================================================

static void drawsdl2_window_resize(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->m_resize_pending = 1;
	sdl->m_resize_height = height;
	sdl->m_resize_width = width;

	window->width = width;
	window->height = height;

	sdl->m_blittimer = 3;

}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawsdl2_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	*xt = x - sdl->m_hofs;
	*yt = y - sdl->m_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *yt >= window->blitheight)
		return 0;
	return 1;
}

//============================================================
//  drawsdl2_window_get_primitives
//============================================================

static void drawsdl2_set_target_bounds(sdl_window_info *window)
{
	window->target->set_bounds(window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor()));
}

//============================================================
//  drawsdl2_window_draw
//============================================================

static int drawsdl2_window_draw(sdl_window_info *window, UINT32 dc, int update)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	render_primitive *prim;
	texture_info *texture=NULL;
	float vofs, hofs;
	int blit_pixels = 0;

	if (video_config.novideo)
	{
		return 0;
	}

	if (sdl->m_resize_pending)
	{
		SDL_SetWindowSize(window->sdl_window, sdl->m_resize_width, sdl->m_resize_height);
		SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
		sdl->m_resize_pending = 0;
		SDL_RenderSetViewport(sdl->m_renderer, NULL);
		//sdlvideo_monitor_refresh(window->monitor());

	}

	//SDL_SelectRenderer(window->sdl_window);

	if (sdl->m_blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawBlendMode(sdl->m_renderer, SDL_BLENDMODE_NONE);
		//SDL_SetRenderDrawColor(0,0,0,255);
		SDL_SetRenderDrawColor(sdl->m_renderer, 0,0,0,0);
		SDL_RenderFillRect(sdl->m_renderer, NULL);
		sdl->m_blittimer--;
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		if ((window->fullscreen()) && (!video_config.switchres))
		{
			ch = window->monitor()->center_height;
			cw = window->monitor()->center_width;
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

	sdl->m_hofs = hofs;
	sdl->m_vofs = vofs;

	window->primlist->acquire_lock();

	// now draw
	for (prim = window->primlist->first(); prim != NULL; prim = prim->next())
	{
		Uint8 sr, sg, sb, sa;

		switch (prim->type)
		{
			case render_primitive::LINE:
				sr = (int)(255.0f * prim->color.r);
				sg = (int)(255.0f * prim->color.g);
				sb = (int)(255.0f * prim->color.b);
				sa = (int)(255.0f * prim->color.a);

				SDL_SetRenderDrawBlendMode(sdl->m_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
				SDL_SetRenderDrawColor(sdl->m_renderer, sr, sg, sb, sa);
				SDL_RenderDrawLine(sdl->m_renderer, prim->bounds.x0 + hofs, prim->bounds.y0 + vofs,
						prim->bounds.x1 + hofs, prim->bounds.y1 + vofs);
				break;
			case render_primitive::QUAD:
				texture = sdl->texture_update(*prim);
				if (texture)
					blit_pixels += (texture->raw_height() * texture->raw_width());
				sdl->render_quad(texture, prim,
						round_nearest(hofs + prim->bounds.x0),
						round_nearest(vofs + prim->bounds.y0));
				break;
			default:
				throw emu_fatalerror("Unexpected render_primitive type\n");
		}
	}

	window->primlist->release_lock();

	sdl->m_last_blit_pixels = blit_pixels;
	sdl->m_last_blit_time = -osd_ticks();
	SDL_RenderPresent(sdl->m_renderer);
	sdl->m_last_blit_time += osd_ticks();

	return 0;
}


//============================================================
//  drawsdl2_window_clear
//============================================================

static void drawsdl2_window_clear(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->m_blittimer = 2;
}


//============================================================
//  drawsdl2_window_destroy
//============================================================

static void drawsdl2_window_destroy(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	// skip if nothing
	if (sdl == NULL)
		return;

	// free the memory in the window

	drawsdl2_destroy_all_textures(window);

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_SetWindowFullscreen(window->sdl_window, 0);    // Try to set mode
		SDL_SetWindowDisplayMode(window->sdl_window, &sdl->m_original_mode);    // Try to set mode
		SDL_SetWindowFullscreen(window->sdl_window, SDL_WINDOW_FULLSCREEN);    // Try to set mode
	}

	SDL_DestroyWindow(window->sdl_window);

	global_free(sdl);
	window->dxdata = NULL;
}

//============================================================
//  texture handling
//============================================================

//============================================================
//  texture_compute_size and type
//============================================================

copy_info_t *texture_info::compute_size_type()
{
	copy_info_t *bi;
	copy_info_t *result = NULL;
	int maxperf = 0;

	for (bi = blit_info[m_format]; bi != NULL; bi = bi->next)
	{
		if ((m_is_rotated == bi->rotate)
				&& (m_sdl_blendmode == bi->bm_mask))
		{
			if (RendererSupportsFormat(m_renderer, bi->dst_fmt, m_sdl_access, bi->dstname))
			{
				int perf = bi->perf;
				if (perf == 0)
					return bi;
				else if (perf > (maxperf * 102) / 100)
				{
					result = bi;
					maxperf = perf;
				}
			}
		}
	}
	if (result)
		return result;
	/* try last resort handlers */
	for (bi = blit_info[m_format]; bi != NULL; bi = bi->next)
	{
		if ((m_is_rotated == bi->rotate)
			&& (m_sdl_blendmode == bi->bm_mask))
			if (RendererSupportsFormat(m_renderer, bi->dst_fmt, m_sdl_access, bi->dstname))
				return bi;
	}
	//FIXME: crash implement a -do nothing handler */
	return NULL;
}

//============================================================
//  texture_info::matches
//============================================================

bool texture_info::matches(const render_primitive &prim, const quad_setup_data &setup)
{
	return  texinfo().base == prim.texture.base &&
			texinfo().width == prim.texture.width &&
			texinfo().height == prim.texture.height &&
			texinfo().rowpixels == prim.texture.rowpixels &&
			m_setup.dudx == setup.dudx &&
			m_setup.dvdx == setup.dvdx &&
			m_setup.dudy == setup.dudy &&
			m_setup.dvdy == setup.dvdy &&
			((flags() ^ prim.flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0;
}

//============================================================
//  texture_create
//============================================================

texture_info::texture_info(SDL_Renderer *renderer, const render_texinfo &texsource, const quad_setup_data &setup, UINT32 flags)
{
	// fill in the core data
	m_renderer = renderer;
	m_hash = texture_compute_hash(texsource, flags);
	m_flags = flags;
	m_texinfo = texsource;
	m_texinfo.seqid = -1; // force set data
	m_is_rotated = FALSE;
	m_setup = setup;
	m_sdl_blendmode = map_blendmode(PRIMFLAG_GET_BLENDMODE(flags));
	m_pitch = 0;

	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			m_format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
			m_format = texsource.palette() ? SDL_TEXFORMAT_RGB32_PALETTED : SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			m_format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_PALETTEA16:
			m_format = SDL_TEXFORMAT_PALETTE16A;
			break;
		case TEXFORMAT_YUY16:
			m_format = texsource.palette() ? SDL_TEXFORMAT_YUY16_PALETTED : SDL_TEXFORMAT_YUY16;
			break;

		default:
			osd_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	if (setup.rotwidth != m_texinfo.width || setup.rotheight != m_texinfo.height
			|| setup.dudx < 0 || setup.dvdy < 0)
		m_is_rotated = TRUE;
	else
		m_is_rotated = FALSE;

	//m_sdl_access = SDL_TEXTUREACCESS_STATIC;
	m_sdl_access = SDL_TEXTUREACCESS_STREAMING;

	// Watch out for 0x0 textures ...
	if (!m_setup.rotwidth || !m_setup.rotheight)
		osd_printf_warning("Trying to create texture with zero dim\n");

	// set copy_info

	m_copyinfo = compute_size_type();

	m_texture_id = SDL_CreateTexture(m_renderer, m_copyinfo->dst_fmt, m_sdl_access,
			m_setup.rotwidth, m_setup.rotheight);

	if (!m_texture_id)
		osd_printf_error("Error creating texture: %d x %d, pixelformat %s error: %s\n", m_setup.rotwidth, m_setup.rotheight,
				m_copyinfo->dstname, SDL_GetError());

	if (m_sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if (m_copyinfo->func != NULL)
			m_pixels = malloc(m_setup.rotwidth * m_setup.rotheight * m_copyinfo->dst_bpp);
		else
			m_pixels = NULL;
	}
	m_last_access = osd_ticks();

}

texture_info::~texture_info()
{
	if ( is_pixels_owned() && (m_pixels != NULL) )
		free(m_pixels);
	SDL_DestroyTexture(m_texture_id);
}

//============================================================
//  texture_set_data
//============================================================

void texture_info::set_data(const render_texinfo &texsource, const UINT32 flags)
{
	m_copyinfo->time -= osd_ticks();
	if (m_sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if ( m_copyinfo->func )
		{
			m_pitch = m_setup.rotwidth * m_copyinfo->dst_bpp;
			m_copyinfo->func(this, &texsource);
		}
		else
		{
			m_pixels = texsource.base;
			m_pitch = m_texinfo.rowpixels * m_copyinfo->dst_bpp;
		}
		SDL_UpdateTexture(m_texture_id, NULL, m_pixels, m_pitch);
	}
	else
	{
		SDL_LockTexture(m_texture_id, NULL, (void **) &m_pixels, &m_pitch);
		if ( m_copyinfo->func )
			m_copyinfo->func(this, &texsource);
		else
		{
			UINT8 *src = (UINT8 *) texsource.base;
			UINT8 *dst = (UINT8 *) m_pixels;
			int spitch = texsource.rowpixels * m_copyinfo->dst_bpp;
			int num = texsource.width * m_copyinfo->dst_bpp;
			int h = texsource.height;
			while (h--) {
				memcpy(dst, src, num);
				src += spitch;
				dst += m_pitch;
			}
		}
		SDL_UnlockTexture(m_texture_id);
	}
	m_copyinfo->time += osd_ticks();
}

//============================================================
//  compute rotation setup
//============================================================

void quad_setup_data::compute(const render_primitive &prim)
{
	const render_quad_texuv *texcoords = &prim.texcoords;
	int texwidth = prim.texture.width;
	int texheight = prim.texture.height;
	float fdudx, fdvdx, fdudy, fdvdy;
	float width, height;
	float fscale;
	/* determine U/V deltas */
	if ((PRIMFLAG_GET_SCREENTEX(prim.flags)))
		fscale = (float) video_config.prescale;
	else
		fscale = 1.0f;

	fdudx = (texcoords->tr.u - texcoords->tl.u) / fscale; // a a11
	fdvdx = (texcoords->tr.v - texcoords->tl.v) / fscale; // c a21
	fdudy = (texcoords->bl.u - texcoords->tl.u) / fscale; // b a12
	fdvdy = (texcoords->bl.v - texcoords->tl.v) / fscale; // d a22

#if 0
	printf("tl.u %f tl.v %f\n", texcoords->tl.u, texcoords->tl.v);
	printf("tr.u %f tr.v %f\n", texcoords->tr.u, texcoords->tr.v);
	printf("bl.u %f bl.v %f\n", texcoords->bl.u, texcoords->bl.v);
	printf("br.u %f br.v %f\n", texcoords->br.u, texcoords->br.v);
	/* compute start and delta U,V coordinates now */
#endif

	dudx = round_nearest(65536.0f * fdudx);
	dvdx = round_nearest(65536.0f * fdvdx);
	dudy = round_nearest(65536.0f * fdudy);
	dvdy = round_nearest(65536.0f * fdvdy);
	startu = round_nearest(65536.0f * (float) texwidth * texcoords->tl.u);
	startv = round_nearest(65536.0f * (float) texheight * texcoords->tl.v);

	/* clamp to integers */

	width = fabs((fdudx * (float) (texwidth) + fdvdx * (float) (texheight)) * fscale * fscale);
	height = fabs((fdudy * (float)(texwidth) + fdvdy * (float) (texheight)) * fscale * fscale);

	rotwidth = width;
	rotheight = height;

	startu += (dudx + dudy) / 2;
	startv += (dvdx + dvdy) / 2;

}

//============================================================
//  texture_find
//============================================================

texture_info *sdl_info::texture_find(const render_primitive &prim, const quad_setup_data &setup)
{
	HashT texhash = texture_compute_hash(prim.texture, prim.flags);
	texture_info *texture;
	osd_ticks_t now = osd_ticks();

	// find a match
	for (texture = m_texlist.first(); texture != NULL; )
		if (texture->hash() == texhash &&
			texture->matches(prim, setup))
		{
			/* would we choose another blitter based on performance ? */
			if ((texture->m_copyinfo->samples & 0x7f) == 0x7f)
			{
				if (texture->m_copyinfo != texture->compute_size_type())
					return NULL;
			}
			texture->m_last_access = now;
			return texture;
		}
		else
		{
			/* free resources not needed any longer? */
			texture_info *expire = texture;
			texture = texture->next();
			if (now - expire->m_last_access > osd_ticks_per_second())
				m_texlist.remove(*expire);
		}

	// nothing found
	return NULL;
}

//============================================================
//  texture_update
//============================================================

texture_info * sdl_info::texture_update(const render_primitive &prim)
{
	quad_setup_data setup;
	texture_info *texture;

	setup.compute(prim);

	texture = texture_find(prim, setup);

	// if we didn't find one, create a new texture
	if (texture == NULL && prim.texture.base != NULL)
	{
		texture = global_alloc(texture_info(m_renderer, prim.texture, setup, prim.flags));
		/* add us to the texture list */
		m_texlist.prepend(*texture);

	}

	if (texture != NULL)
	{
		if (prim.texture.base != NULL && texture->texinfo().seqid != prim.texture.seqid)
		{
			texture->texinfo().seqid = prim.texture.seqid;
			// if we found it, but with a different seqid, copy the data
			texture->set_data(prim.texture, prim.flags);
		}

	}
	return texture;
}


static void drawsdl2_destroy_all_textures(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	if (sdl == NULL)
		return;

	if(window->primlist)
	{
		window->primlist->acquire_lock();
		sdl->m_texlist.reset();
		window->primlist->release_lock();
	}
	else
		sdl->m_texlist.reset();
}
