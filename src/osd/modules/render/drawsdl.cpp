// license:BSD-3-Clause
// copyright-holders:Couriersud, Olivier Galibert, R. Belmont
//============================================================
//
//  drawsdl.cpp - SDL software and OpenGL implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  yuvmodes by Couriersud
//
//============================================================

#include "render_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_SDL) && !defined(SDLMAME_SDL3)

// from specific OSD implementation
#include "sdlopts.h"
#include "window.h"

// general OSD headers
#include "modules/monitor/monitor_module.h"

// MAME headers
#include "emucore.h"
#include "render.h"
#include "rendersw.hxx"

//#include "ui/uimain.h"

#include <SDL2/SDL.h>

// standard C headers
#include <cmath>
#include <cstdio>
#include <memory>


namespace osd {

namespace {

//============================================================
//  CONSTANTS
//============================================================

#define DRAW2_SCALEMODE_NEAREST "0"
#define DRAW2_SCALEMODE_LINEAR  "1"
#define DRAW2_SCALEMODE_BEST    "2"


struct sdl_scale_mode
{
	const char      *name;
	int             is_scale;           /* Scale mode?           */
	int             is_yuv;             /* Yuv mode?             */
	int             mult_w;             /* Width multiplier      */
	int             mult_h;             /* Height multiplier     */
	const char      *sdl_scale_mode_hint;        /* what to use as a hint ? */
	int             pixel_format;       /* Pixel/Overlay format  */
	void            (*yuv_blit)(const uint16_t *bitmap, uint8_t *ptr, const int pitch, const uint32_t *lookup, const int width, const int height);
};


// renderer_sdl1 is the information about SDL for the current screen
class renderer_sdl1 : public osd_renderer
{
public:

	renderer_sdl1(osd_window &w, sdl_scale_mode const &scale_mode)
		: osd_renderer(w)
		, m_scale_mode(scale_mode)
		, m_sdl_renderer(nullptr)
		, m_texture_id(nullptr)
		, m_yuv_lookup()
		, m_yuv_bitmap()
		//, m_hw_scale_width(0)
		//, m_hw_scale_height(0)
		, m_last_hofs(0)
		, m_last_vofs(0)
		, m_blit_dim(0, 0)
		, m_last_dim(0, 0)
	{
	}
	virtual ~renderer_sdl1();

	virtual int create() override;
	virtual int draw(const int update) override;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
	virtual render_primitive_list *get_primitives() override;

private:
	void show_info(struct SDL_RendererInfo *render_info);

	void destroy_all_textures();
	void yuv_init();
	void setup_texture(const osd_dim &size);
	void yuv_lookup_set(unsigned int pen, unsigned char red,
				unsigned char green, unsigned char blue);

	int32_t               m_blittimer;

	sdl_scale_mode const &m_scale_mode;
	SDL_Renderer        *m_sdl_renderer;
	SDL_Texture         *m_texture_id;

	// YUV overlay
	std::unique_ptr<uint32_t []> m_yuv_lookup;
	std::unique_ptr<uint16_t []> m_yuv_bitmap;

	// if we leave scaling to SDL and the underlying driver, this
	// is the render_target_width/height to use

	int                 m_last_hofs;
	int                 m_last_vofs;
	osd_dim             m_blit_dim;
	osd_dim             m_last_dim;
};


//============================================================
//  PROTOTYPES
//============================================================

// YUV overlays

static void yuv_RGB_to_YV12(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height);
static void yuv_RGB_to_YV12X2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height);
static void yuv_RGB_to_YUY2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height);
static void yuv_RGB_to_YUY2X2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height);

//============================================================
//  setup_texture for window
//============================================================

void renderer_sdl1::setup_texture(const osd_dim &size)
{
	SDL_DisplayMode mode;
	uint32_t fmt;

	// Determine preferred pixelformat and set up yuv if necessary
	SDL_GetCurrentDisplayMode(window().monitor()->oshandle(), &mode);

	m_yuv_bitmap.reset();

	fmt = (m_scale_mode.pixel_format ? m_scale_mode.pixel_format : mode.format);

	if (m_scale_mode.is_scale)
	{
		int m_hw_scale_width = 0;
		int m_hw_scale_height = 0;

		window().target()->compute_minimum_size(m_hw_scale_width, m_hw_scale_height);
		if (window().prescale())
		{
			m_hw_scale_width *= window().prescale();
			m_hw_scale_height *= window().prescale();

			/* This must be a multiple of 2 */
			m_hw_scale_width = (m_hw_scale_width + 1) & ~1;
		}
		if (m_scale_mode.is_yuv)
			m_yuv_bitmap = std::make_unique<uint16_t []>(m_hw_scale_width * m_hw_scale_height);

		int w = m_hw_scale_width * m_scale_mode.mult_w;
		int h = m_hw_scale_height * m_scale_mode.mult_h;

		m_texture_id = SDL_CreateTexture(m_sdl_renderer, fmt, SDL_TEXTUREACCESS_STREAMING, w, h);

	}
	else
	{
		m_texture_id = SDL_CreateTexture(m_sdl_renderer,fmt, SDL_TEXTUREACCESS_STREAMING,
				size.width(), size.height());
	}
}

//============================================================
//  drawsdl_show_info
//============================================================

void renderer_sdl1::show_info(struct SDL_RendererInfo *render_info)
{
#define RF_ENTRY(x) {x, #x }
	static struct
	{
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
			{-1, nullptr}
		};

	osd_printf_verbose("window: using renderer %s\n", render_info->name ? render_info->name : "<unknown>");
	for (int i = 0; rflist[i].name != nullptr; i++)
		if (render_info->flags & rflist[i].flag)
			osd_printf_verbose("renderer: flag %s\n", rflist[i].name);
}

//============================================================
//  renderer_sdl2::create
//============================================================

int renderer_sdl1::create()
{
	// create renderer

	/* set hints ... */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, m_scale_mode.sdl_scale_mode_hint);


	if (video_config.waitvsync)
		m_sdl_renderer = SDL_CreateRenderer(dynamic_cast<sdl_window_info &>(window()).platform_window(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	else
		m_sdl_renderer = SDL_CreateRenderer(dynamic_cast<sdl_window_info &>(window()).platform_window(), -1, SDL_RENDERER_ACCELERATED);

	if (!m_sdl_renderer)
	{
		if (video_config.waitvsync)
			m_sdl_renderer = SDL_CreateRenderer(dynamic_cast<sdl_window_info &>(window()).platform_window(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_SOFTWARE);
		else
			m_sdl_renderer = SDL_CreateRenderer(dynamic_cast<sdl_window_info &>(window()).platform_window(), -1, SDL_RENDERER_SOFTWARE);
	}

	if (!m_sdl_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	struct SDL_RendererInfo render_info;

	SDL_GetRendererInfo(m_sdl_renderer, &render_info);
	show_info(&render_info);

	// Check scale mode

	if (m_scale_mode.pixel_format)
	{
		int i;
		int found = 0;

		for (i=0; i < render_info.num_texture_formats; i++)
			if (m_scale_mode.pixel_format == render_info.texture_formats[i])
				found = 1;

		if (!found)
		{
			fatalerror("window: Scale mode %s not supported!", m_scale_mode.name);
		}
	}

#if 0
	int w = 0, h = 0;
	window().get_size(w, h);
	setup_texture(w, h);
#endif

	m_yuv_lookup = nullptr;
	m_blittimer = 0;

	yuv_init();
	osd_printf_verbose("Leave renderer_sdl1::create\n");
	return 0;
}

//============================================================
//  DESTRUCTOR
//============================================================

renderer_sdl1::~renderer_sdl1()
{
	destroy_all_textures();

	SDL_DestroyRenderer(m_sdl_renderer);
}

//============================================================
//  drawsdl_xy_to_render_target
//============================================================

int renderer_sdl1::xy_to_render_target(int x, int y, int *xt, int *yt)
{
	*xt = x - m_last_hofs;
	*yt = y - m_last_vofs;
	if (*xt<0 || *xt >= m_blit_dim.width())
		return 0;
	if (*yt<0 || *yt >= m_blit_dim.height())
		return 0;
	return 1;
}

//============================================================
//  drawsdl_destroy_all_textures
//============================================================

void renderer_sdl1::destroy_all_textures()
{
	SDL_DestroyTexture(m_texture_id);
	m_texture_id = nullptr;
}


//============================================================
//  renderer_sdl2::draw
//============================================================

int renderer_sdl1::draw(int update)
{
	uint8_t *surfptr;
	int32_t pitch;
	Uint32 rmask, gmask, bmask;
	Uint32 amask;
	int32_t vofs, hofs, blitwidth, blitheight, ch, cw;
	int bpp;

	osd_dim wdim = window().get_size();
	if (has_flags(FI_CHANGED) || (wdim != m_last_dim))
	{
		destroy_all_textures();
		clear_flags(FI_CHANGED);
		m_blittimer = 3;
		m_last_dim = wdim;
		SDL_RenderSetViewport(m_sdl_renderer, nullptr);
		if (m_texture_id != nullptr)
			SDL_DestroyTexture(m_texture_id);
		setup_texture(m_blit_dim);
		m_blittimer = 3;
	}

	// lock it if we need it

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
		SDL_RenderFillRect(m_sdl_renderer,nullptr);
		//SDL_RenderFill(0,0,0,0 /*255*/,nullptr);
		m_blittimer--;
	}

	SDL_LockTexture(m_texture_id, nullptr, (void **) &surfptr, &pitch);

	// get ready to center the image
	vofs = hofs = 0;
	blitwidth = m_blit_dim.width();
	blitheight = m_blit_dim.height();

	ch = wdim.height();
	cw = wdim.width();

	// do not crash if the window's smaller than the blit area
	if (blitheight > ch)
	{
			blitheight = ch;
	}
	else if (video_config.centerv)
	{
		vofs = (ch - m_blit_dim.height()) / 2;
	}

	if (blitwidth > cw)
	{
			blitwidth = cw;
	}
	else if (video_config.centerh)
	{
		hofs = (cw - m_blit_dim.width()) / 2;
	}

	m_last_hofs = hofs;
	m_last_vofs = vofs;

	window().m_primlist->acquire_lock();

	int mamewidth, mameheight;

		Uint32 fmt = 0;
		int access = 0;
		SDL_QueryTexture(m_texture_id, &fmt, &access, &mamewidth, &mameheight);
		mamewidth /= m_scale_mode.mult_w;
		mameheight /= m_scale_mode.mult_h;
	//printf("w h %d %d %d %d\n", mamewidth, mameheight, blitwidth, blitheight);

	// rescale bounds
	float fw = (float) mamewidth / (float) blitwidth;
	float fh = (float) mameheight / (float) blitheight;

	// FIXME: this could be a lot easier if we get the primlist here!
	//          Bounds would be set fit for purpose and done!

	for (render_primitive &prim : *window().m_primlist)
	{
		prim.bounds.x0 = floor(fw * prim.bounds.x0 + 0.5f);
		prim.bounds.x1 = floor(fw * prim.bounds.x1 + 0.5f);
		prim.bounds.y0 = floor(fh * prim.bounds.y0 + 0.5f);
		prim.bounds.y1 = floor(fh * prim.bounds.y1 + 0.5f);
	}

	// render to it
	if (!m_scale_mode.is_yuv)
	{
		switch (rmask)
		{
			case 0xff000000:
				software_renderer<uint32_t, 0,0,0, 24,16,8>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x0000ff00:
				software_renderer<uint32_t, 0,0,0, 8,16,24>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x00ff0000:
				software_renderer<uint32_t, 0,0,0, 16,8,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0x000000ff:
				software_renderer<uint32_t, 0,0,0, 0,8,16>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 4);
				break;

			case 0xf800:
				software_renderer<uint16_t, 3,2,3, 11,5,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			case 0x7c00:
				software_renderer<uint16_t, 3,3,3, 10,5,0>::draw_primitives(*window().m_primlist, surfptr, mamewidth, mameheight, pitch / 2);
				break;

			default:
				osd_printf_error("SDL: ERROR! Unknown video mode: R=%08X G=%08X B=%08X\n", rmask, gmask, bmask);
				break;
		}
	}
	else
	{
		assert (m_yuv_bitmap != nullptr);
		assert (surfptr != nullptr);
		software_renderer<uint16_t, 3,3,3, 10,5,0>::draw_primitives(*window().m_primlist, m_yuv_bitmap.get(), mamewidth, mameheight, mamewidth);
		m_scale_mode.yuv_blit(m_yuv_bitmap.get(), surfptr, pitch, m_yuv_lookup.get(), mamewidth, mameheight);
	}

	window().m_primlist->release_lock();

	// unlock and flip
	SDL_UnlockTexture(m_texture_id);
	{
		SDL_Rect r;

		r.x=hofs;
		r.y=vofs;
		r.w=blitwidth;
		r.h=blitheight;
		//printf("blitwidth %d %d - %d %d\n", blitwidth, blitheight, window().width, window().height);
		//SDL_UpdateTexture(sdltex, nullptr, sdlsurf->pixels, pitch);
		SDL_RenderCopy(m_sdl_renderer,m_texture_id, nullptr, &r);
		SDL_RenderPresent(m_sdl_renderer);
	}
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

void renderer_sdl1::yuv_lookup_set(unsigned int pen, unsigned char red,
			unsigned char green, unsigned char blue)
{
	uint32_t y,u,v;

	RGB2YUV(red,green,blue,y,u,v);

	/* Storing this data in YUYV order simplifies using the data for
	   YUY2, both with and without smoothing... */
	m_yuv_lookup[pen]=(y<<Y1SHIFT)|(u<<USHIFT)|(y<<Y2SHIFT)|(v<<VSHIFT);
}

void renderer_sdl1::yuv_init()
{
	if (!m_yuv_lookup)
		m_yuv_lookup = std::make_unique<uint32_t []>(65536);
	for (unsigned char r = 0; r < 32; r++)
		for (unsigned char g = 0; g < 32; g++)
			for (unsigned char b = 0; b < 32; b++)
			{
				int idx = (r << 10) | (g << 5) | b;
				yuv_lookup_set(idx,
					(r << 3) | (r >> 2),
					(g << 3) | (g >> 2),
					(b << 3) | (b >> 2));
			}
}

//uint32_t *lookup = sdl->m_yuv_lookup;

static void yuv_RGB_to_YV12(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height)
{
	int x, y;
	uint8_t *pixels[3];
	int u1,v1,y1,u2,v2,y2,u3,v3,y3,u4,v4,y4;      /* 12 */

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * height;
	pixels[2] = pixels[1] + pitch * height / 4;

	for(y=0;y<height;y+=2)
	{
		const uint16_t *src=bitmap + (y * width) ;
		const uint16_t *src2=src + width;

		uint8_t *dest_y = pixels[0] + y * pitch;
		uint8_t *dest_v = pixels[1] + (y>>1) * pitch / 2;
		uint8_t *dest_u = pixels[2] + (y>>1) * pitch / 2;

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

static void yuv_RGB_to_YV12X2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int x,y;
	int u1,v1,y1;
	uint8_t *pixels[3];

	pixels[0] = ptr;
	pixels[1] = ptr + pitch * height * 2;
	int p2 = (pitch >> 1);
	pixels[2] = pixels[1] + p2 * height;

	for(y=0;y<height;y++)
	{
		const uint16_t *src = bitmap + (y * width) ;

		uint16_t *dest_y = (uint16_t *)(pixels[0] + 2 * y * pitch);
		uint8_t *dest_v = pixels[1] + y * p2;
		uint8_t *dest_u = pixels[2] + y * p2;
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

static void yuv_RGB_to_YUY2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int y;
	uint32_t p1,p2,uv;
	const int yuv_pitch = pitch/4;

	for(y=0;y<height;y++)
	{
		const uint16_t *src=bitmap + (y * width) ;
		const uint16_t *end=src+width;

		uint32_t *dest = (uint32_t *) ptr;
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

static void yuv_RGB_to_YUY2X2(const uint16_t *bitmap, uint8_t *ptr, const int pitch,
		const uint32_t *lookup, const int width, const int height)
{
	/* this one is used when scale==2 */
	unsigned int y;
	int yuv_pitch = pitch / 4;

	for(y=0;y<height;y++)
	{
		const uint16_t *src=bitmap + (y * width) ;
		const uint16_t *end=src+width;

		uint32_t *dest = (uint32_t *) ptr;
		dest += (y * yuv_pitch);
		for(; src<end; src++)
		{
			dest[0] = lookup[src[0]];
			dest++;
		}
	}
}

render_primitive_list *renderer_sdl1::get_primitives()
{
	osd_dim nd = window().get_size();
	if (nd != m_blit_dim)
	{
		m_blit_dim = nd;
		notify_changed();
	}
	window().target()->set_bounds(m_blit_dim.width(), m_blit_dim.height(), window().pixel_aspect());
	return &window().target()->get_primitives();
}


class video_sdl1 : public osd_module, public render_module
{
public:
	video_sdl1()
		: osd_module(OSD_RENDERER_PROVIDER, "soft")
		, m_scale_mode(-1)
	{
	}

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override { }

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override;

protected:
	virtual unsigned flags() const override { return FLAG_INTERACTIVE; }

private:
	static int get_scale_mode(char const *modestr);

	int m_scale_mode;

	static sdl_scale_mode const s_scale_modes[];
};

int video_sdl1::init(osd_interface &osd, osd_options const &options)
{
	osd_printf_verbose("Using SDL multi-window soft driver (SDL 2.0+)\n");

	// yuv settings ...
	char const *const modestr = dynamic_cast<sdl_options const &>(options).scale_mode();
	m_scale_mode = get_scale_mode(modestr);
	if (m_scale_mode < 0)
	{
		osd_printf_warning("Invalid yuvmode value %s; reverting to none\n", modestr);
		m_scale_mode = 0;
	}

	return 0;
}

std::unique_ptr<osd_renderer> video_sdl1::create(osd_window &window)
{
	return std::make_unique<renderer_sdl1>(window, s_scale_modes[m_scale_mode]);
}

int video_sdl1::get_scale_mode(char const *modestr)
{
	const sdl_scale_mode *sm = s_scale_modes;
	int index = 0;
	while (sm->name)
	{
		if (!strcmp(sm->name, modestr))
			return index;
		index++;
		sm++;
	}
	return -1;
}

sdl_scale_mode const video_sdl1::s_scale_modes[] = {
		{ "none",    0, 0, 1, 1, DRAW2_SCALEMODE_NEAREST, 0,                    nullptr },
		{ "hwblit",  1, 0, 1, 1, DRAW2_SCALEMODE_LINEAR,  0,                    nullptr },
		{ "hwbest",  1, 0, 1, 1, DRAW2_SCALEMODE_BEST,    0,                    nullptr },
		/* SDL1.2 uses interpolation as well */
		{ "yv12",    1, 1, 1, 1, DRAW2_SCALEMODE_BEST,    SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12 },
		{ "yv12x2",  1, 1, 2, 2, DRAW2_SCALEMODE_BEST,    SDL_PIXELFORMAT_YV12, yuv_RGB_to_YV12X2 },
		{ "yuy2",    1, 1, 1, 1, DRAW2_SCALEMODE_BEST,    SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2 },
		{ "yuy2x2",  1, 1, 2, 1, DRAW2_SCALEMODE_BEST,    SDL_PIXELFORMAT_YUY2, yuv_RGB_to_YUY2X2 },
		{ nullptr } };

} // anonymous namespace

} // namespace osd

#else // defined(OSD_SDL) && !defined(SDLMAME_SDL3)

namespace osd { namespace { MODULE_NOT_SUPPORTED(video_sdl1, OSD_RENDERER_PROVIDER, "soft") } }

#endif // defined(OSD_SDL) && !defined(SDLMAME_SDL3)

MODULE_DEFINITION(RENDERER_SDL1, osd::video_sdl1)
