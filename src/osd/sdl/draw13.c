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
//  MACROS
//============================================================

#define IS_OPAQUE(a)        (a >= 1.0f)
#define IS_TRANSPARENT(a)   (a <  0.0001f)

#define MAX4(a, b, c, d) MAX(a, MAX(b, MAX(c, d)))
#define MIN4(a, b, c, d) MIN(a, MIN(b, MIN(c, d)))


//============================================================
//  TYPES
//============================================================


struct quad_setup_data
{
    quad_setup_data()
    : dudx(0), dvdx(0), dudy(0), dvdy(0), startu(0), startv(0),
      rotwidth(0), rotheight(0)
    {}
	INT32           dudx, dvdx, dudy, dvdy;
	INT32           startu, startv;
	INT32           rotwidth, rotheight;
};

struct texture_info;

typedef void (*texture_copy_func)(texture_info *texture, const render_texinfo *texsource);

struct copy_info {
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
	copy_info           *next;
};

/* texture_info holds information about a texture */
struct texture_info
{
    texture_info()
    : next(NULL), hash(0), flags(0), rawwidth(0), rawheight(0), format(0),
      pixels(NULL), pitch(0), pixels_own(0), texture_id(NULL), copyinfo(NULL),
      sdl_access(0), sdl_blendmode(SDL_BLENDMODE_NONE), is_rotated(0), last_access(0)
    {
    }
	texture_info *      next;               // next texture in the list

	HashT               hash;               // hash value for the texture (must be >= pointer size)
	UINT32              flags;              // rendering flags
	render_texinfo      texinfo;            // copy of the texture info

	int                 rawwidth, rawheight;// raw width/height of the texture

	int                 format;             // texture format
	void                *pixels;            // pixels for the texture
	int                 pitch;
	int                 pixels_own;         // do we own / allocated it ?

	SDL_Texture         *texture_id;

	copy_info           *copyinfo;
	Uint32              sdl_access;
	SDL_BlendMode       sdl_blendmode;
	quad_setup_data     setup;
	int                 is_rotated;

	osd_ticks_t         last_access;
};

/* sdl_info is the information about SDL for the current screen */
struct sdl_info
{
    sdl_info()
    : blittimer(0), extra_flags(0), sdl_renderer(NULL), texlist(NULL),
      texture_max_width(0), texture_max_height(0), last_hofs(0), last_vofs(0),
      resize_pending(0), resize_width(0), resize_height(0),
      last_blit_time(0), last_blit_pixels(0)
    {}
	INT32           blittimer;
	UINT32          extra_flags;

	SDL_Renderer    *sdl_renderer;
	texture_info *  texlist;                // list of active textures
	INT32           texture_max_width;      // texture maximum width
	INT32           texture_max_height;     // texture maximum height

	float           last_hofs;
	float           last_vofs;

	// resize information

	UINT8           resize_pending;
	UINT32          resize_width;
	UINT32          resize_height;

	// Stats
	INT64           last_blit_time;
	INT64           last_blit_pixels;
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
static render_primitive_list &drawsdl2_window_get_primitives(sdl_window_info *window);
static void drawsdl2_destroy_all_textures(sdl_window_info *window);
static void drawsdl2_window_clear(sdl_window_info *window);
static int drawsdl2_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt);
static void drawsdl2_destroy_texture(sdl_info *sdl, texture_info *texture);

//============================================================
//  Textures
//============================================================

static void texture_set_data(sdl_info *sdl, texture_info *texture, const render_texinfo *texsource, UINT32 flags);
static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, quad_setup_data *setup, UINT32 flags);
static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup);
static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim);


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

static copy_info blit_info_default[] =
{
	/* no rotation */
	ENTRY(ARGB32,           ARGB8888,   4, 0, NULL),
	ENTRY_LR(ARGB32,        RGB888,     4, 0, argb32_rgb32),
	/* Entry for primarily for directfb */
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
	/* Entry for primarily for directfb */
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

static copy_info *blit_info[SDL_TEXFORMAT_LAST+1];

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

INLINE HashT texture_compute_hash(const render_texinfo *texture, UINT32 flags)
{
	return (HashT)texture->base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

INLINE SDL_BlendMode map_blendmode(int blendmode)
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


	if (color->r >= 1.0f && color->g >= 1.0f && color->b >= 1.0f && IS_OPAQUE(color->a))
	{
		SDL_SetTextureColorMod(texture_id, 0xFF, 0xFF, 0xFF);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* coloring-only case */
	else if (IS_OPAQUE(color->a))
	{
		SDL_SetTextureColorMod(texture_id, sr, sg, sb);
		SDL_SetTextureAlphaMod(texture_id, 0xFF);
	}
	/* alpha and/or coloring case */
	else if (!IS_TRANSPARENT(color->a))
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

INLINE void render_quad(sdl_info *sdl, texture_info *texture, render_primitive *prim, int x, int y)
{
	SDL_Texture *texture_id;
	SDL_Rect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim->bounds.x1 - prim->bounds.x0);
	target_rect.h = round_nearest(prim->bounds.y1 - prim->bounds.y0);

	if (texture)
	{
		texture_id = texture->texture_id;

		texture->copyinfo->time -= osd_ticks();
#if 0
		if ((PRIMFLAG_GET_SCREENTEX(prim->flags)) && video_config.filter)
		{
			SDL_SetTextureScaleMode(texture->texture_id,  DRAW2_SCALEMODE_BEST);
		}
		else
		{
			SDL_SetTextureScaleMode(texture->texture_id,  DRAW2_SCALEMODE_NEAREST);
		}
#endif
		SDL_SetTextureBlendMode(texture_id, texture->sdl_blendmode);
		set_coloralphamode(texture_id, &prim->color);
		SDL_RenderCopy(sdl->sdl_renderer,  texture_id, NULL, &target_rect);
		texture->copyinfo->time += osd_ticks();

		texture->copyinfo->pixel_count += MAX(STAT_PIXEL_THRESHOLD , (texture->rawwidth * texture->rawheight));
		if (sdl->last_blit_pixels)
		{
			texture->copyinfo->time += (sdl->last_blit_time * (INT64) (texture->rawwidth * texture->rawheight)) / (INT64) sdl->last_blit_pixels;
		}
		texture->copyinfo->samples++;
		texture->copyinfo->perf = ( texture->copyinfo->pixel_count * (osd_ticks_per_second()/1000)) / texture->copyinfo->time;
	}
	else
	{
		UINT32 sr = (UINT32)(255.0f * prim->color.r);
		UINT32 sg = (UINT32)(255.0f * prim->color.g);
		UINT32 sb = (UINT32)(255.0f * prim->color.b);
		UINT32 sa = (UINT32)(255.0f * prim->color.a);

		SDL_SetRenderDrawBlendMode(sdl->sdl_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
		SDL_SetRenderDrawColor(sdl->sdl_renderer, sr, sg, sb, sa);
		SDL_RenderFillRect(sdl->sdl_renderer, &target_rect);
	}
}

#if 0
static int RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat)
{
	struct SDL_RendererInfo render_info;
	int i;

	SDL_GetRendererInfo(&render_info);

	for (i=0; i < render_info.num_texture_formats; i++)
	{
		if (format == render_info.texture_formats[i])
			return 1;
	}
	osd_printf_verbose("Pixelformat <%s> not supported\n", sformat);
	return 0;
}
#else
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
#endif

//============================================================
//  drawsdl2_init
//============================================================

static void add_list(copy_info **head, copy_info *element, Uint32 bm)
{
	copy_info *newci = global_alloc(copy_info);
	*newci = *element;

	newci->bm_mask = bm;
	newci->next = *head;
	*head = newci;
}

static void expand_copy_info(copy_info *list)
{
	copy_info   *bi;

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
		osd_printf_verbose("Warning: Unable to load opengl library: %s\n", stemp ? stemp : "<default>");
	else
		osd_printf_verbose("Loaded opengl shared library: %s\n", stemp ? stemp : "<default>");

	return 0;
}


//============================================================
//  drawsdl2_exit
//============================================================

static void drawsdl2_exit(void)
{
	int i;
	copy_info *bi, *freeme;
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
	window->get_primitives = drawsdl2_window_get_primitives;
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

	osd_printf_verbose("Enter drawsdl2_window_create\n");

	window->dxdata = sdl;

	sdl->extra_flags = (window->fullscreen() ?
			SDL_WINDOW_BORDERLESS | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE);

	// create the SDL window
	window->sdl_window = SDL_CreateWindow(window->title, window->monitor()->monitor_x, 0,
			width, height, sdl->extra_flags);

	if (window->fullscreen() && video_config.switchres)
	{
		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(window->monitor()->handle, &mode);
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
	}
	else
		SDL_SetWindowDisplayMode(window->sdl_window, NULL); // Use desktop

	// create renderer

	if (video_config.waitvsync)
		sdl->sdl_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		sdl->sdl_renderer = SDL_CreateRenderer(window->sdl_window, -1, SDL_RENDERER_ACCELERATED);

	if (!sdl->sdl_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	//SDL_SelectRenderer(window->sdl_window);

	SDL_ShowWindow(window->sdl_window);
	//SDL_SetWindowFullscreen(window->window_id, window->fullscreen);
	SDL_RaiseWindow(window->sdl_window);
	SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);


	sdl->blittimer = 3;

	// in case any textures try to come up before these are validated,
	// OpenGL guarantees all implementations can handle something this size.
	sdl->texture_max_width = 64;
	sdl->texture_max_height = 64;

	SDL_RenderPresent(sdl->sdl_renderer);
	osd_printf_verbose("Leave drawsdl2_window_create\n");
	return 0;
}

//============================================================
//  drawsdl2_window_resize
//============================================================

static void drawsdl2_window_resize(sdl_window_info *window, int width, int height)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->resize_pending = 1;
	sdl->resize_height = height;
	sdl->resize_width = width;

	window->width = width;
	window->height = height;

	sdl->blittimer = 3;

}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

static int drawsdl2_xy_to_render_target(sdl_window_info *window, int x, int y, int *xt, int *yt)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	*xt = x - sdl->last_hofs;
	*yt = y - sdl->last_vofs;
	if (*xt<0 || *xt >= window->blitwidth)
		return 0;
	if (*yt<0 || *yt >= window->blitheight)
		return 0;
	return 1;
}

//============================================================
//  drawsdl2_window_get_primitives
//============================================================

static render_primitive_list &drawsdl2_window_get_primitives(sdl_window_info *window)
{
	if ((!window->fullscreen()) || (video_config.switchres))
	{
		window->blit_surface_size(window->width, window->height);
	}
	else
	{
		window->blit_surface_size(window->monitor()->center_width, window->monitor()->center_height);
	}
	window->target->set_bounds(window->blitwidth, window->blitheight, sdlvideo_monitor_get_aspect(window->monitor()));
	return window->target->get_primitives();
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

	if (sdl->resize_pending)
	{
		SDL_SetWindowSize(window->sdl_window, sdl->resize_width, sdl->resize_height);
		SDL_GetWindowSize(window->sdl_window, &window->width, &window->height);
		sdl->resize_pending = 0;
		SDL_RenderSetViewport(sdl->sdl_renderer, NULL);
	}

	//SDL_SelectRenderer(window->sdl_window);

	if (sdl->blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawBlendMode(sdl->sdl_renderer, SDL_BLENDMODE_NONE);
		//SDL_SetRenderDrawColor(0,0,0,255);
		SDL_SetRenderDrawColor(sdl->sdl_renderer, 0,0,0,0);
		SDL_RenderFillRect(sdl->sdl_renderer, NULL);
		sdl->blittimer--;
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

	sdl->last_hofs = hofs;
	sdl->last_vofs = vofs;

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

				SDL_SetRenderDrawBlendMode(sdl->sdl_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim->flags)));
				SDL_SetRenderDrawColor(sdl->sdl_renderer, sr, sg, sb, sa);
				SDL_RenderDrawLine(sdl->sdl_renderer, prim->bounds.x0 + hofs, prim->bounds.y0 + vofs,
						prim->bounds.x1 + hofs, prim->bounds.y1 + vofs);
				break;
			case render_primitive::QUAD:
				texture = texture_update(window, prim);
				if (texture)
					blit_pixels += (texture->rawheight * texture->rawwidth);
				render_quad(sdl, texture, prim,
						round_nearest(hofs + prim->bounds.x0),
						round_nearest(vofs + prim->bounds.y0));
				break;
			default:
				throw emu_fatalerror("Unexpected render_primitive type\n");
		}
	}

	window->primlist->release_lock();

	sdl->last_blit_pixels = blit_pixels;
	sdl->last_blit_time = -osd_ticks();
	SDL_RenderPresent(sdl->sdl_renderer);
	sdl->last_blit_time += osd_ticks();

	return 0;
}


//============================================================
//  drawsdl2_window_clear
//============================================================

static void drawsdl2_window_clear(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;

	sdl->blittimer = 2;
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

static copy_info *texture_compute_size_type(SDL_Renderer *renderer, const render_texinfo *texsource, texture_info *texture, UINT32 flags)
{
	copy_info *bi;
	copy_info *result = NULL;
	int maxperf = 0;
	//int bm = PRIMFLAG_GET_BLENDMODE(flags);

	for (bi = blit_info[texture->format]; bi != NULL; bi = bi->next)
	{
		if ((texture->is_rotated == bi->rotate)
				&& (texture->sdl_blendmode == bi->bm_mask))
		{
			if (RendererSupportsFormat(renderer, bi->dst_fmt, texture->sdl_access, bi->dstname))
			{
				if (bi->perf == 0)
					return bi;
				else if (bi->perf > (maxperf * 102) / 100)
				{
					result = bi;
					maxperf = bi->perf;
				}
			}
		}
	}
	if (result)
		return result;
	/* try last resort handlers */
	for (bi = blit_info[texture->format]; bi != NULL; bi = bi->next)
	{
		if ((texture->is_rotated == bi->rotate)
			&& (texture->sdl_blendmode == bi->bm_mask))
			if (RendererSupportsFormat(renderer, bi->dst_fmt, texture->sdl_access, bi->dstname))
				return bi;
	}
	//FIXME: crash implement a -do nothing handler */
	return NULL;
}

//============================================================
//  texture_create
//============================================================

static texture_info *texture_create(sdl_window_info *window, const render_texinfo *texsource, quad_setup_data *setup, UINT32 flags)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *texture;

	// allocate a new texture
	texture = global_alloc(texture_info);

	// fill in the core data
	texture->hash = texture_compute_hash(texsource, flags);
	texture->flags = flags;
	texture->texinfo = *texsource;
	texture->texinfo.seqid = -1; // force set data
	texture->is_rotated = FALSE;
	texture->setup = *setup;
	texture->sdl_blendmode = map_blendmode(PRIMFLAG_GET_BLENDMODE(flags));

	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			texture->format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
			texture->format = texsource->palette() ? SDL_TEXFORMAT_RGB32_PALETTED : SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			texture->format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_PALETTEA16:
			texture->format = SDL_TEXFORMAT_PALETTE16A;
			break;
		case TEXFORMAT_YUY16:
			texture->format = texsource->palette() ? SDL_TEXFORMAT_YUY16_PALETTED : SDL_TEXFORMAT_YUY16;
			break;

		default:
			osd_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	texture->rawwidth = texsource->width;
	texture->rawheight = texsource->height;
	if (setup->rotwidth != texture->rawwidth || setup->rotheight != texture->rawheight
			|| setup->dudx < 0 )
		texture->is_rotated = TRUE;
	else
		texture->is_rotated = FALSE;

	//texture->sdl_access = SDL_TEXTUREACCESS_STATIC;
	texture->sdl_access = SDL_TEXTUREACCESS_STREAMING;

	// Watch out for 0x0 textures ...
	if (!texture->setup.rotwidth || !texture->setup.rotheight)
		osd_printf_warning("Trying to create texture with zero dim\n");

	// compute the size
	texture->copyinfo = texture_compute_size_type(sdl->sdl_renderer, texsource, texture, flags);

	texture->texture_id = SDL_CreateTexture(sdl->sdl_renderer, texture->copyinfo->dst_fmt, texture->sdl_access,
			texture->setup.rotwidth, texture->setup.rotheight);

	if (!texture->texture_id)
		osd_printf_error("Error creating texture: %d x %d, pixelformat %s error: %s\n", texture->setup.rotwidth, texture->setup.rotheight,
				texture->copyinfo->dstname, SDL_GetError());

	if ( (texture->copyinfo->func != NULL) && (texture->sdl_access == SDL_TEXTUREACCESS_STATIC))
	{
		texture->pixels = malloc(texture->setup.rotwidth * texture->setup.rotheight * texture->copyinfo->dst_bpp);
		texture->pixels_own=TRUE;
	}
	/* add us to the texture list */
	texture->next = sdl->texlist;
	sdl->texlist = texture;

	texture->last_access = osd_ticks();

	return texture;
}

//============================================================
//  texture_set_data
//============================================================

static void texture_set_data(sdl_info *sdl, texture_info *texture, const render_texinfo *texsource, UINT32 flags)
{
	texture->copyinfo->time -= osd_ticks();
	if (texture->sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if ( texture->copyinfo->func )
		{
			texture->pitch = texture->setup.rotwidth * texture->copyinfo->dst_bpp;
			texture->copyinfo->func(texture, texsource);
		}
		else
		{
			texture->pixels = texsource->base;
			texture->pitch = texture->texinfo.rowpixels * texture->copyinfo->dst_bpp;
		}
		SDL_UpdateTexture(texture->texture_id, NULL, texture->pixels, texture->pitch);
	}
	else
	{
		SDL_LockTexture(texture->texture_id, NULL, (void **) &texture->pixels, &texture->pitch);
		if ( texture->copyinfo->func )
			texture->copyinfo->func(texture, texsource);
		else
		{
			UINT8 *src = (UINT8 *) texsource->base;
			UINT8 *dst = (UINT8 *) texture->pixels;
			int spitch = texsource->rowpixels * texture->copyinfo->dst_bpp;
			int num = texsource->width * texture->copyinfo->dst_bpp;
			int h = texsource->height;
			while (h--) {
				memcpy(dst, src, num);
				src += spitch;
				dst += texture->pitch;
			}
		}
		SDL_UnlockTexture(texture->texture_id);
	}
	texture->copyinfo->time += osd_ticks();
}

//============================================================
//  compute rotation setup
//============================================================

static void compute_setup(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup, int flags)
{
	const render_quad_texuv *texcoords = &prim->texcoords;
	int texwidth = prim->texture.width;
	int texheight = prim->texture.height;
	float fdudx, fdvdx, fdudy, fdvdy;
	float width, height;
	float fscale;
	/* determine U/V deltas */
	if ((PRIMFLAG_GET_SCREENTEX(flags)))
		fscale = (float) video_config.prescale;
	else
		fscale = 1.0f;

	fdudx = (texcoords->tr.u - texcoords->tl.u) / fscale; // a a11
	fdvdx = (texcoords->tr.v - texcoords->tl.v) / fscale; // c a21
	fdudy = (texcoords->bl.u - texcoords->tl.u) / fscale; // b a12
	fdvdy = (texcoords->bl.v - texcoords->tl.v) / fscale; // d a22

	/* compute start and delta U,V coordinates now */

	setup->dudx = round_nearest(65536.0f * fdudx);
	setup->dvdx = round_nearest(65536.0f * fdvdx);
	setup->dudy = round_nearest(65536.0f * fdudy);
	setup->dvdy = round_nearest(65536.0f * fdvdy);
	setup->startu = round_nearest(65536.0f * (float) texwidth * texcoords->tl.u);
	setup->startv = round_nearest(65536.0f * (float) texheight * texcoords->tl.v);

	/* clamp to integers */

	width = fabs((fdudx * (float) (texwidth) + fdvdx * (float) (texheight)) * fscale * fscale);
	height = fabs((fdudy * (float)(texwidth) + fdvdy * (float) (texheight)) * fscale * fscale);

	setup->rotwidth = width;
	setup->rotheight = height;

	setup->startu += (setup->dudx + setup->dudy) / 2;
	setup->startv += (setup->dvdx + setup->dvdy) / 2;

}

//============================================================
//  texture_find
//============================================================

static texture_info *texture_find(sdl_info *sdl, const render_primitive *prim, quad_setup_data *setup)
{
	HashT texhash = texture_compute_hash(&prim->texture, prim->flags);
	texture_info *texture;
	osd_ticks_t now = osd_ticks();

	// find a match
	for (texture = sdl->texlist; texture != NULL; )
		if (texture->hash == texhash &&
			texture->texinfo.base == prim->texture.base &&
			texture->texinfo.width == prim->texture.width &&
			texture->texinfo.height == prim->texture.height &&
			texture->texinfo.rowpixels == prim->texture.rowpixels &&
			texture->setup.dudx == setup->dudx &&
			texture->setup.dvdx == setup->dvdx &&
			texture->setup.dudy == setup->dudy &&
			texture->setup.dvdy == setup->dvdy &&
			((texture->flags ^ prim->flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0)
		{
			/* would we choose another blitter ? */
			if ((texture->copyinfo->samples & 0x1f) == 0x1f)
			{
				if (texture->copyinfo != texture_compute_size_type(sdl->sdl_renderer, &texture->texinfo, texture, prim->flags))
					return NULL;
#if 0
				else
				{
					/* reset stats */
					texture->copyinfo->samples = 0;
					texture->copyinfo->time = 0;
					texture->copyinfo->pixel_count = 0;
				}
#endif
			}
			texture->last_access = now;
			return texture;
		}
		else
		{
			/* free resources not needed any longer? */
			texture_info *expire = texture;
			texture = texture->next;
			if (now - expire->last_access > osd_ticks_per_second())
				drawsdl2_destroy_texture(sdl, expire);
		}

	// nothing found
	return NULL;
}

//============================================================
//  texture_update
//============================================================

static texture_info * texture_update(sdl_window_info *window, const render_primitive *prim)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	quad_setup_data setup;
	texture_info *texture;

	compute_setup(sdl, prim, &setup, prim->flags);

	texture = texture_find(sdl, prim, &setup);

	// if we didn't find one, create a new texture
	if (texture == NULL && prim->texture.base != NULL)
	{
		texture = texture_create(window, &prim->texture, &setup, prim->flags);
	}

	if (texture != NULL)
	{
		if (prim->texture.base != NULL && texture->texinfo.seqid != prim->texture.seqid)
		{
			texture->texinfo.seqid = prim->texture.seqid;
			// if we found it, but with a different seqid, copy the data
			texture_set_data(sdl, texture, &prim->texture, prim->flags);
		}

	}
	return texture;
}

static void drawsdl2_destroy_texture(sdl_info *sdl, texture_info *texture)
{
	texture_info *p;

	SDL_DestroyTexture(texture->texture_id);
	if ( texture->pixels_own )
	{
		free(texture->pixels);
		texture->pixels=NULL;
		texture->pixels_own=FALSE;
	}

	for (p=sdl->texlist; p != NULL; p = p->next)
		if (p->next == texture)
			break;
	if (p == NULL)
		sdl->texlist = NULL;
	else
		p->next = texture->next;
	global_free(texture);
}

static void drawsdl2_destroy_all_textures(sdl_window_info *window)
{
	sdl_info *sdl = (sdl_info *) window->dxdata;
	texture_info *next_texture=NULL, *texture = NULL;
	int lock=FALSE;

	if (sdl == NULL)
		return;

	if(window->primlist)
	{
		lock=TRUE;
		window->primlist->acquire_lock();
	}

	texture = sdl->texlist;

	while (texture)
	{
		next_texture = texture->next;
		drawsdl2_destroy_texture(sdl, texture);
		texture = next_texture;
	}

	if (lock)
		window->primlist->release_lock();
}
