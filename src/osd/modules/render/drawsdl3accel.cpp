// license:BSD-3-Clause
// copyright-holders: Couriersud, Olivier Galibert, R. Belmont
//============================================================
//
//  drawsdl3accel.cpp - SDL accelerated drawing using SDL's renderer API
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  renderer_sdl2 by Couriersud
//
//============================================================

#include "render_module.h"

#include "modules/osdmodule.h"

#if defined(OSD_SDL) && defined (SDLMAME_SDL3)

// OSD headers
#include "sdlopts.h"
#include "window.h"

// lib/util
#include "options.h"

// emu
#include "emucore.h"
#include "render.h"

// standard SDL headers
#include <SDL3/SDL.h>

// standard C headers
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <list>
namespace osd {

namespace {

struct quad_setup_data
{
	quad_setup_data() = default;

	void compute(const render_primitive &prim, const int prescale);

	int32_t   dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
	int32_t   startu = 0, startv = 0;
	int32_t   rotwidth = 0, rotheight = 0;
};

//============================================================
//  Textures
//============================================================

class renderer_sdl2;
struct copy_info_t;

/* texture_info holds information about a texture */
class texture_info
{
public:
	texture_info(renderer_sdl2 *renderer, const render_texinfo &texsource, const quad_setup_data &setup, const uint32_t flags);
	~texture_info();

	void set_data(const render_texinfo &texsource, const uint32_t flags);
	void render_quad(const render_primitive &prim, const int x, const int y);
	bool matches(const render_primitive &prim, const quad_setup_data &setup);

	copy_info_t const *compute_size_type();

	void                *m_pixels;            // pixels for the texture
	int                 m_pitch;

	copy_info_t const   *m_copyinfo;
	quad_setup_data     m_setup;

	osd_ticks_t         m_last_access;

	int raw_width() const { return m_texinfo.width; }
	int raw_height() const { return m_texinfo.height; }

	const render_texinfo &texinfo() const { return m_texinfo; }
	render_texinfo &texinfo() { return m_texinfo; }

	HashT hash() const { return m_hash; }
	uint32_t flags() const { return m_flags; }

private:
	bool is_pixels_owned() const;

	void set_coloralphamode(SDL_Texture *texture_id, const render_color *color);

	Uint32              m_sdl_access;
	renderer_sdl2 *     m_renderer;
	render_texinfo      m_texinfo;            // copy of the texture info
	HashT               m_hash;               // hash value for the texture (must be >= pointer size)
	uint32_t            m_flags;              // rendering flags

	SDL_Texture *       m_texture_id;
	bool                m_is_rotated;

	int                 m_format;             // texture format
	SDL_BlendMode       m_sdl_blendmode;
};

// inline functions and macros
#include "blit13.ipp"

//============================================================
//  TEXCOPY FUNCS
//============================================================

enum SDL_TEXFORMAT_E
{
	SDL_TEXFORMAT_ARGB32 = 0,
	SDL_TEXFORMAT_RGB32,
	SDL_TEXFORMAT_RGB32_PALETTED,
	SDL_TEXFORMAT_YUY16,
	SDL_TEXFORMAT_YUY16_PALETTED,
	SDL_TEXFORMAT_PALETTE16,
	SDL_TEXFORMAT_RGB15,
	SDL_TEXFORMAT_RGB15_PALETTED,
	SDL_TEXFORMAT_PALETTE16A,
	SDL_TEXFORMAT_PALETTE16_ARGB1555,
	SDL_TEXFORMAT_RGB15_ARGB1555,
	SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555,
	SDL_TEXFORMAT_LAST = SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555
};

struct copy_info_t
{
	int                 src_fmt;
	Uint32              dst_fmt;
	const blit_base     *blitter;
	Uint32              bm_mask;
	const char          *srcname;
	const char          *dstname;
	/* Statistics */
	mutable uint64_t    pixel_count;
	mutable int64_t     time;
	mutable int         samples;
	mutable int         perf;
	/* list */
	copy_info_t const   *next;
};

/* renderer_sdl2 is the information about SDL for the current screen */
class renderer_sdl2 : public osd_renderer
{
public:
	renderer_sdl2(
			osd_window &window,
			copy_info_t const *const (&blit_info)[SDL_TEXFORMAT_LAST + 1]);

	virtual ~renderer_sdl2()
	{
		destroy_all_textures();
		SDL_DestroyRenderer(m_sdl_renderer);
		m_sdl_renderer = nullptr;
	}

	virtual int create() override;
	virtual int draw(const int update) override;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
	virtual render_primitive_list *get_primitives() override;

	int RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat);

	SDL_Renderer *m_sdl_renderer;
	copy_info_t const *const (&m_blit_info)[SDL_TEXFORMAT_LAST + 1];

private:
	void render_quad(texture_info *texture, const render_primitive &prim, const int x, const int y);

	texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
	texture_info *texture_update(const render_primitive &prim);

	void destroy_all_textures();

	int32_t         m_blittimer;

	std::list<texture_info> m_texlist;                // list of active textures

	float           m_last_hofs;
	float           m_last_vofs;

	int             m_width;
	int             m_height;

	osd_dim         m_blit_dim;

	struct
	{
		Uint32  format;
		int     status;
	} fmt_support[30];

	// Stats
	int64_t         m_last_blit_time;
	int64_t         m_last_blit_pixels;
};


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
//  CONSTRUCTOR & DESTRUCTOR
//============================================================

renderer_sdl2::renderer_sdl2(
		osd_window &window,
		copy_info_t const *const (&blit_info)[SDL_TEXFORMAT_LAST + 1])
	: osd_renderer(window)
	, m_sdl_renderer(nullptr)
	, m_blit_info(blit_info)
	, m_blittimer(0)
	, m_last_hofs(0)
	, m_last_vofs(0)
	, m_width(0)
	, m_height(0)
	, m_blit_dim(0, 0)
	, m_last_blit_time(0)
	, m_last_blit_pixels(0)
{
	for (int i = 0; i < 30; i++)
	{
		fmt_support[i].format = 0;
		fmt_support[i].status = 0;
	}
}

//============================================================
//  INLINES
//============================================================


static inline float round_nearest(float f)
{
	return floor(f + 0.5f);
}

static inline HashT texture_compute_hash(const render_texinfo &texture, const uint32_t flags)
{
	return (HashT)texture.base ^ (flags & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK));
}

static inline SDL_BlendMode map_blendmode(const int blendmode)
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

void texture_info::set_coloralphamode(SDL_Texture *texture_id, const render_color *color)
{
	uint32_t sr = (uint32_t)(255.0f * color->r);
	uint32_t sg = (uint32_t)(255.0f * color->g);
	uint32_t sb = (uint32_t)(255.0f * color->b);
	uint32_t sa = (uint32_t)(255.0f * color->a);


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

void texture_info::render_quad(const render_primitive &prim, const int x, const int y)
{
	SDL_FRect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim.bounds.x1) - round_nearest(prim.bounds.x0);
	target_rect.h = round_nearest(prim.bounds.y1) - round_nearest(prim.bounds.y0);

	SDL_SetTextureBlendMode(m_texture_id, m_sdl_blendmode);
	set_coloralphamode(m_texture_id, &prim.color);
	//printf("%d %d %d %d\n", target_rect.x, target_rect.y, target_rect.w, target_rect.h);
	// Arghhh .. Just another bug. SDL_RenderTexture has severe issues with scaling ...
	SDL_RenderTexture(m_renderer->m_sdl_renderer,  m_texture_id, nullptr, &target_rect);
	//SDL_RenderTextureRotated(m_renderer->m_sdl_renderer,  m_texture_id, nullptr, &target_rect, 0, nullptr, SDL_FLIP_NONE);
	//SDL_RenderTextureRotated(m_renderer->m_sdl_renderer,  m_texture_id, nullptr, nullptr, 0, nullptr, SDL_FLIP_NONE);
}

void renderer_sdl2::render_quad(texture_info *texture, const render_primitive &prim, const int x, const int y)
{
	SDL_FRect target_rect;

	target_rect.x = x;
	target_rect.y = y;
	target_rect.w = round_nearest(prim.bounds.x1 - prim.bounds.x0);
	target_rect.h = round_nearest(prim.bounds.y1 - prim.bounds.y0);

	if (texture)
	{
		copy_info_t const *copyinfo = texture->m_copyinfo;
		copyinfo->time -= osd_ticks();
		texture->render_quad(prim, x, y);
		copyinfo->time += osd_ticks();

		copyinfo->pixel_count += std::max(STAT_PIXEL_THRESHOLD , (texture->raw_width() * texture->raw_height()));
		if (m_last_blit_pixels)
		{
			copyinfo->time += (m_last_blit_time * (int64_t) (texture->raw_width() * texture->raw_height())) / (int64_t) m_last_blit_pixels;
		}
		copyinfo->samples++;
		copyinfo->perf = (texture->m_copyinfo->pixel_count * (osd_ticks_per_second()/1000)) / std::max<int64_t>(texture->m_copyinfo->time, 1);
	}
	else
	{
		uint32_t sr = (uint32_t)(255.0f * prim.color.r);
		uint32_t sg = (uint32_t)(255.0f * prim.color.g);
		uint32_t sb = (uint32_t)(255.0f * prim.color.b);
		uint32_t sa = (uint32_t)(255.0f * prim.color.a);

		SDL_SetRenderDrawBlendMode(m_sdl_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim.flags)));
		SDL_SetRenderDrawColor(m_sdl_renderer, sr, sg, sb, sa);
		SDL_RenderFillRect(m_sdl_renderer, &target_rect);
	}
}

int renderer_sdl2::RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat)
{
	int i;
	for (i = 0; fmt_support[i].format != 0; i++)
	{
		if (format == fmt_support[i].format)
		{
			return fmt_support[i].status;
		}
	}
	/* not tested yet */
	fmt_support[i].format = format;
	fmt_support[i + 1].format = 0;
	SDL_Texture *texid = SDL_CreateTexture(m_sdl_renderer, SDL_PixelFormat(format), SDL_TextureAccess(access), 16, 16);
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
//  sdl_info::create
//============================================================
int renderer_sdl2::create()
{
	osd_printf_verbose("Enter renderer_sdl2::create\n");

	// create renderer
	m_sdl_renderer = SDL_CreateRenderer(dynamic_cast<sdl_window_info &>(window()).platform_window(), nullptr);

	if (!m_sdl_renderer)
	{
		fatalerror("Error on creating renderer: %s\n", SDL_GetError());
	}

	if (video_config.waitvsync)
	{
		SDL_SetRenderVSync(m_sdl_renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
	}

	/* Enable bilinear filtering in case it is supported.
	 * This applies to all texture operations. However, artwort is pre-scaled
	 * and thus shouldn't be affected.
	 */
#if SDL_VERSION_ATLEAST(3, 3, 2)
	if (video_config.filter)
	{
		SDL_SetDefaultTextureScaleMode(m_sdl_renderer, SDL_SCALEMODE_LINEAR);
	}
	else
	{
		SDL_SetDefaultTextureScaleMode(m_sdl_renderer, SDL_SCALEMODE_NEAREST);
	}
#endif
	m_blittimer = 3;

	const auto props = SDL_GetRendererProperties(m_sdl_renderer);
	osd_printf_verbose("SDL renderer using driver %s\n", SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING, "Unknown"));

	osd_printf_verbose("Leave renderer_sdl2::create\n");
	return 0;
}


//============================================================
//  drawsdl_xy_to_render_target
//============================================================

int renderer_sdl2::xy_to_render_target(int x, int y, int *xt, int *yt)
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

void renderer_sdl2::destroy_all_textures()
{
	if (window().m_primlist)
	{
		window().m_primlist->acquire_lock();
		m_texlist.clear();
		window().m_primlist->release_lock();
	}
	else
		m_texlist.clear();
}

//============================================================
//  sdl_info::draw
//============================================================

int renderer_sdl2::draw(int update)
{
	texture_info *texture=nullptr;
	float vofs, hofs;
	int blit_pixels = 0;

	osd_dim wdim = window().get_size();

	if (has_flags(FI_CHANGED) || (wdim.width() != m_width) || (wdim.height() != m_height))
	{
		destroy_all_textures();
		m_width = wdim.width();
		m_height = wdim.height();
		SDL_SetRenderViewport(m_sdl_renderer, nullptr);
		m_blittimer = 3;
		clear_flags(FI_CHANGED);
	}

	//SDL_SelectRenderer(window().sdl_window);

	if (m_blittimer > 0)
	{
		/* SDL Underlays need alpha = 0 ! */
		SDL_SetRenderDrawBlendMode(m_sdl_renderer, SDL_BLENDMODE_NONE);
		//SDL_SetRenderDrawColor(0,0,0,255);
		SDL_SetRenderDrawColor(m_sdl_renderer, 0,0,0,0);
		SDL_RenderFillRect(m_sdl_renderer, nullptr);
		m_blittimer--;
	}

	// compute centering parameters
	vofs = hofs = 0.0f;

	if (video_config.centerv || video_config.centerh)
	{
		int ch, cw;

		ch = wdim.height();
		cw = wdim.width();

		if (video_config.centerv)
		{
			vofs = (ch - m_blit_dim.height()) / 2.0f;
		}
		if (video_config.centerh)
		{
			hofs = (cw - m_blit_dim.width()) / 2.0f;
		}
	}

	m_last_hofs = hofs;
	m_last_vofs = vofs;

	window().m_primlist->acquire_lock();

	// now draw
	for (render_primitive &prim : *window().m_primlist)
	{
		Uint8 sr, sg, sb, sa;

		switch (prim.type)
		{
			case render_primitive::LINE:
				sr = (int)(255.0f * prim.color.r);
				sg = (int)(255.0f * prim.color.g);
				sb = (int)(255.0f * prim.color.b);
				sa = (int)(255.0f * prim.color.a);

				SDL_SetRenderDrawBlendMode(m_sdl_renderer, map_blendmode(PRIMFLAG_GET_BLENDMODE(prim.flags)));
				SDL_SetRenderDrawColor(m_sdl_renderer, sr, sg, sb, sa);
				SDL_RenderLine(m_sdl_renderer, prim.bounds.x0 + hofs, prim.bounds.y0 + vofs,
						prim.bounds.x1 + hofs, prim.bounds.y1 + vofs);
				break;
			case render_primitive::QUAD:
				texture = texture_update(prim);
				if (texture)
					blit_pixels += (texture->raw_height() * texture->raw_width());
				render_quad(texture, prim,
						round_nearest(hofs + prim.bounds.x0),
						round_nearest(vofs + prim.bounds.y0));
				break;
			default:
				throw emu_fatalerror("Unexpected render_primitive type\n");
		}
	}

	window().m_primlist->release_lock();

	m_last_blit_pixels = blit_pixels;
	m_last_blit_time = -osd_ticks();
	SDL_RenderPresent(m_sdl_renderer);
	m_last_blit_time += osd_ticks();

	return 0;
}


//============================================================
//  texture handling
//============================================================

//============================================================
//  texture_compute_size and type
//============================================================

copy_info_t const *texture_info::compute_size_type()
{
	copy_info_t const *result = nullptr;
	int maxperf = 0;

	for (copy_info_t const *bi = m_renderer->m_blit_info[m_format]; bi != nullptr; bi = bi->next)
	{
		if ((m_is_rotated == bi->blitter->m_is_rot) && (m_sdl_blendmode == bi->bm_mask))
		{
			if (m_renderer->RendererSupportsFormat(bi->dst_fmt, m_sdl_access, bi->dstname))
			{
				int const perf = bi->perf;
				if (perf == 0)
				{
					return bi;
				}
				else if (perf > ((maxperf * 102) / 100))
				{
					result = bi;
					maxperf = perf;
				}
			}
		}
	}

	if (result)
		return result;

	// try last resort handlers
	for (copy_info_t const *bi = m_renderer->m_blit_info[m_format]; bi != nullptr; bi = bi->next)
	{
		if ((m_is_rotated == bi->blitter->m_is_rot) && (m_sdl_blendmode == bi->bm_mask))
			if (m_renderer->RendererSupportsFormat(bi->dst_fmt, m_sdl_access, bi->dstname))
				return bi;
	}
	//FIXME: crash implement a -do nothing handler
	return nullptr;
}

bool texture_info::is_pixels_owned() const
{
	// do we own / allocated it ?
	return (m_sdl_access == SDL_TEXTUREACCESS_STATIC) && !m_copyinfo->blitter->m_is_passthrough;
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
			m_setup.startu == setup.startu &&
			m_setup.startv == setup.startv &&
			((flags() ^ prim.flags) & (PRIMFLAG_BLENDMODE_MASK | PRIMFLAG_TEXFORMAT_MASK)) == 0;
}

//============================================================
//  texture_create
//============================================================

texture_info::texture_info(renderer_sdl2 *renderer, const render_texinfo &texsource, const quad_setup_data &setup, uint32_t flags)
{
	// fill in the core data
	m_renderer = renderer;
	m_hash = texture_compute_hash(texsource, flags);
	m_flags = flags;
	m_texinfo = texsource;
	m_texinfo.seqid = -1; // force set data
	m_is_rotated = false;
	m_setup = setup;
	m_sdl_blendmode = map_blendmode(PRIMFLAG_GET_BLENDMODE(flags));
	m_pitch = 0;

	switch (PRIMFLAG_GET_TEXFORMAT(flags))
	{
		case TEXFORMAT_ARGB32:
			m_format = SDL_TEXFORMAT_ARGB32;
			break;
		case TEXFORMAT_RGB32:
			m_format = texsource.palette ? SDL_TEXFORMAT_RGB32_PALETTED : SDL_TEXFORMAT_RGB32;
			break;
		case TEXFORMAT_PALETTE16:
			m_format = SDL_TEXFORMAT_PALETTE16;
			break;
		case TEXFORMAT_YUY16:
			m_format = texsource.palette ? SDL_TEXFORMAT_YUY16_PALETTED : SDL_TEXFORMAT_YUY16;
			break;

		default:
			osd_printf_error("Unknown textureformat %d\n", PRIMFLAG_GET_TEXFORMAT(flags));
	}

	if (setup.rotwidth != m_texinfo.width || setup.rotheight != m_texinfo.height
			|| setup.dudx < 0 || setup.dvdy < 0 || (PRIMFLAG_GET_TEXORIENT(flags) != 0))
		m_is_rotated = true;
	else
		m_is_rotated = false;

	m_sdl_access = SDL_TEXTUREACCESS_STREAMING;

	// Watch out for 0x0 textures ...
	if (!m_setup.rotwidth || !m_setup.rotheight)
		osd_printf_warning("Trying to create texture with zero dim\n");

	// set copy_info
	m_copyinfo = compute_size_type();

	m_texture_id = SDL_CreateTexture(m_renderer->m_sdl_renderer, SDL_PixelFormat(m_copyinfo->dst_fmt), SDL_TextureAccess(m_sdl_access),
			m_setup.rotwidth, m_setup.rotheight);

	if (!m_texture_id)
		osd_printf_error("Error creating texture: %d x %d, pixelformat %s error: %s\n", m_setup.rotwidth, m_setup.rotheight,
				m_copyinfo->dstname, SDL_GetError());

#if !SDL_VERSION_ATLEAST(3, 3, 2)
	if (video_config.filter)
	{
		SDL_SetTextureScaleMode(m_texture_id, SDL_SCALEMODE_LINEAR);
	}
	else
	{
		SDL_SetTextureScaleMode(m_texture_id, SDL_SCALEMODE_NEAREST);
	}
#endif

	if (m_sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if (m_copyinfo->blitter->m_is_passthrough)
			m_pixels = nullptr;
		else
			m_pixels = malloc(m_setup.rotwidth * m_setup.rotheight * m_copyinfo->blitter->m_dest_bpp);
	}
	m_last_access = osd_ticks();
}

texture_info::~texture_info()
{
	if (is_pixels_owned() && m_pixels)
		free(m_pixels);
	SDL_DestroyTexture(m_texture_id);
}

//============================================================
//  texture_set_data
//============================================================

void texture_info::set_data(const render_texinfo &texsource, const uint32_t flags)
{
	m_copyinfo->time -= osd_ticks();
	if (m_sdl_access == SDL_TEXTUREACCESS_STATIC)
	{
		if (m_copyinfo->blitter->m_is_passthrough)
		{
			m_pixels = texsource.base;
			m_pitch = m_texinfo.rowpixels * m_copyinfo->blitter->m_dest_bpp;
		}
		else
		{
			m_pitch = m_setup.rotwidth * m_copyinfo->blitter->m_dest_bpp;
			m_copyinfo->blitter->texop(this, &texsource);
		}
		SDL_UpdateTexture(m_texture_id, nullptr, m_pixels, m_pitch);
	}
	else
	{
		SDL_LockTexture(m_texture_id, nullptr, (void **)&m_pixels, &m_pitch);
		if ( m_copyinfo->blitter->m_is_passthrough )
		{
			const uint8_t *src = (uint8_t *)texsource.base;
			uint8_t *dst = (uint8_t *)m_pixels;
			int spitch = texsource.rowpixels * m_copyinfo->blitter->m_dest_bpp;
			int num = texsource.width * m_copyinfo->blitter->m_dest_bpp;
			int h = texsource.height;
			while (h--) {
				memcpy(dst, src, num);
				src += spitch;
				dst += m_pitch;
			}
		}
		else
			m_copyinfo->blitter->texop(this, &texsource);
		SDL_UnlockTexture(m_texture_id);
	}
	m_copyinfo->time += osd_ticks();
}

//============================================================
//  compute rotation setup
//============================================================

inline float signf(const float a)
{
	return (0.0f < a) - (a < 0.0f);
}

void quad_setup_data::compute(const render_primitive &prim, const int prescale)
{
	const render_quad_texuv *texcoords = &prim.texcoords;
	int texwidth = prim.texture.width;
	int texheight = prim.texture.height;
	float fdudx, fdvdx, fdudy, fdvdy;
	float width, height;
	float fscale;
	/* determine U/V deltas */
	if ((PRIMFLAG_GET_SCREENTEX(prim.flags)))
		fscale = (float) prescale;
	else
		fscale = 1.0f;

	fdudx = (texcoords->tr.u - texcoords->tl.u); // a a11
	fdvdx = (texcoords->tr.v - texcoords->tl.v); // c a21
	fdudy = (texcoords->bl.u - texcoords->tl.u); // b a12
	fdvdy = (texcoords->bl.v - texcoords->tl.v); // d a22

	width = fabsf(( fdudx * (float) (texwidth) + fdvdx * (float) (texheight)) ) * fscale;
	height = fabsf((fdudy * (float) (texwidth) + fdvdy * (float) (texheight)) ) * fscale;

	fdudx = signf(fdudx) / fscale;
	fdvdy = signf(fdvdy) / fscale;
	fdvdx = signf(fdvdx) / fscale;
	fdudy = signf(fdudy) / fscale;

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

	rotwidth = round_nearest(width);
	rotheight = round_nearest(height);

	//printf("%d %d rot %d %d\n", texwidth, texheight, rotwidth, rotheight);

	startu += (dudx + dudy) / 2;
	startv += (dvdx + dvdy) / 2;

}

//============================================================
//  texture_find
//============================================================

texture_info *renderer_sdl2::texture_find(const render_primitive &prim, const quad_setup_data &setup)
{
	const HashT texhash = texture_compute_hash(prim.texture, prim.flags);
	const osd_ticks_t now = osd_ticks();

	// find a match
	for (auto texture = m_texlist.begin(); texture != m_texlist.end(); )
	{
		if ((texture->hash() == texhash) && texture->matches(prim, setup))
		{
			// would we choose another blitter based on performance?
			if ((texture->m_copyinfo->samples & 0x7f) == 0x7f)
			{
				if (texture->m_copyinfo != texture->compute_size_type())
					return nullptr;
			}
			texture->m_last_access = now;
			return &*texture;
		}
		else
		{
			// free resources not needed any longer?
			if ((now - texture->m_last_access) > osd_ticks_per_second())
				texture = m_texlist.erase(texture);
			else
				++texture;
		}
	}

	// nothing found
	return nullptr;
}

//============================================================
//  texture_update
//============================================================

texture_info * renderer_sdl2::texture_update(const render_primitive &prim)
{
	quad_setup_data setup;
	texture_info *texture;

	setup.compute(prim, window().prescale());

	texture = texture_find(prim, setup);

	// if we didn't find one, create a new texture
	if (!texture && prim.texture.base)
	{
		// add us to the texture list
		texture = &m_texlist.emplace_front(this, prim.texture, setup, prim.flags);
	}

	if (texture)
	{
		if (prim.texture.base && (texture->texinfo().seqid != prim.texture.seqid))
		{
			texture->texinfo().seqid = prim.texture.seqid;
			// if we found it, but with a different seqid, copy the data
			texture->set_data(prim.texture, prim.flags);
		}

	}
	return texture;
}

render_primitive_list *renderer_sdl2::get_primitives()
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


class video_sdl3_accel : public osd_module, public render_module
{
public:
	video_sdl3_accel()
		: osd_module(OSD_RENDERER_PROVIDER, "accel")
		, m_blit_info_initialized(false)
		, m_gllib_loaded(false)
	{
		std::fill(std::begin(m_blit_info), std::end(m_blit_info), nullptr);
	}
	~video_sdl3_accel()
	{
		free_copy_info();
	}

	virtual int init(osd_interface &osd, osd_options const &options) override;
	virtual void exit() override { free_copy_info(); }

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) override;

protected:
	virtual unsigned flags() const override { return FLAG_INTERACTIVE | FLAG_SDL_NEEDS_OPENGL; }

private:
	static inline constexpr Uint32 BM_ALL = UINT32_MAX; // SDL_BLENDMODE_MASK | SDL_BLENDMODE_BLEND | SDL_BLENDMODE_ADD | SDL_BLENDMODE_MOD

	void expand_copy_info();
	void free_copy_info();

	static void add_list(copy_info_t const *&head, copy_info_t const &element, Uint32 bm);

	copy_info_t const *m_blit_info[SDL_TEXFORMAT_LAST + 1];
	bool m_blit_info_initialized;
	bool m_gllib_loaded;

	static copy_info_t const s_blit_info_default[];
};

int video_sdl3_accel::init(osd_interface &osd, osd_options const &options)
{
	osd_printf_verbose("Using SDL native texturing driver (SDL 3.2+)\n");

	// Load the GL library now - else MT will fail
	char const *libname = nullptr;
#if USE_OPENGL
	libname = dynamic_cast<sdl_options const &>(options).gl_lib();
	if (libname && (!*libname || !std::strcmp(libname, OSDOPTVAL_AUTO)))
		libname = nullptr;
#endif

	if (!m_gllib_loaded)
	{
		// No fatalerror here since not all video drivers support GL!
		if (SDL_GL_LoadLibrary(libname) != 0)
		{
			osd_printf_error("Unable to load OpenGL shared library: %s\n", libname ? libname : "<default>");
			m_gllib_loaded = true;
		}
		else
		{
			osd_printf_verbose("Loaded OpenGL shared library: %s\n", libname ? libname : "<default>");
		}
	}

	return 0;
}

std::unique_ptr<osd_renderer> video_sdl3_accel::create(osd_window &window)
{
	if (!m_blit_info_initialized)
	{
		// On macOS, calling this from drawsdl2_init will prohibit fullscreen toggling.
		// It is than not possible to toggle from fullscreen to window mode.
		expand_copy_info();
		m_blit_info_initialized = true;
	}

	return std::make_unique<renderer_sdl2>(window, m_blit_info);
}

void video_sdl3_accel::expand_copy_info()
{
	for (const copy_info_t *bi = s_blit_info_default; bi->src_fmt != -1; bi++)
	{
		if (bi->bm_mask == BM_ALL)
		{
			add_list(m_blit_info[bi->src_fmt], *bi, SDL_BLENDMODE_NONE);
			add_list(m_blit_info[bi->src_fmt], *bi, SDL_BLENDMODE_ADD);
			add_list(m_blit_info[bi->src_fmt], *bi, SDL_BLENDMODE_MOD);
			add_list(m_blit_info[bi->src_fmt], *bi, SDL_BLENDMODE_BLEND);
		}
		else
		{
			add_list(m_blit_info[bi->src_fmt], *bi, bi->bm_mask);
		}
	}
}

void video_sdl3_accel::free_copy_info()
{
	if (m_blit_info_initialized)
	{
		for (int i = 0; i <= SDL_TEXFORMAT_LAST; i++)
		{
			for (copy_info_t const *bi = m_blit_info[i]; bi != nullptr; )
			{
				if (bi->pixel_count)
				{
					osd_printf_verbose(
							"%s -> %s %s blendmode 0x%02x, %d samples: %d KPixel/sec\n",
							bi->srcname,
							bi->dstname,
							bi->blitter->m_is_rot ? "rot" : "norot",
							bi->bm_mask,
							bi->samples,
							bi->perf);
				}
				delete std::exchange(bi, bi->next);
			}
			m_blit_info[i] = nullptr;
		}
		m_blit_info_initialized = false;
	}
}

void video_sdl3_accel::add_list(copy_info_t const *&head, copy_info_t const &element, Uint32 bm)
{
	copy_info_t *const newci = new copy_info_t(element);

	newci->bm_mask = bm;
	newci->next = head;
	head = newci;
}


//============================================================
//  STATIC VARIABLES
//============================================================

#define ENTRY(a,b,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, &texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, 0}
#define ENTRY_BM(a,b,f,bm) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, &texcopy_ ## f, bm, #a, #b, 0, 0, 0, 0}
#define ENTRY_LR(a,b,f) { SDL_TEXFORMAT_ ## a, SDL_PIXELFORMAT_ ## b, &texcopy_ ## f, BM_ALL, #a, #b, 0, 0, 0, -1}

copy_info_t const video_sdl3_accel::s_blit_info_default[] =
{
	/* no rotation */
	ENTRY(ARGB32,           ARGB8888,   argb32_argb32),
	ENTRY_LR(ARGB32,        XRGB8888,   argb32_rgb32),
	/* Entry primarily for directfb */
	ENTRY_BM(ARGB32,        XRGB8888,     argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,        XRGB8888,     argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,        XRGB8888,     argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,            ARGB8888,   rgb32_argb32),
	ENTRY(RGB32,            XRGB8888,   rgb32_rgb32),

	ENTRY(RGB32_PALETTED,   ARGB8888,   rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,   XRGB8888,   rgb32pal_argb32),

	ENTRY(YUY16,            UYVY,       yuv16_uyvy),
	ENTRY(YUY16,            YUY2,       yuv16_yuy2),
	ENTRY(YUY16,            YVYU,       yuv16_yvyu),
	ENTRY(YUY16,            ARGB8888,   yuv16_argb32),
	ENTRY(YUY16,            XRGB8888,   yuv16_argb32),

	ENTRY(YUY16_PALETTED,   UYVY,       yuv16pal_uyvy),
	ENTRY(YUY16_PALETTED,   YUY2,       yuv16pal_yuy2),
	ENTRY(YUY16_PALETTED,   YVYU,       yuv16pal_yvyu),
	ENTRY(YUY16_PALETTED,   ARGB8888,   yuv16pal_argb32),
	ENTRY(YUY16_PALETTED,   XRGB8888,   yuv16pal_argb32),

	ENTRY(PALETTE16,        ARGB8888,   pal16_argb32),
	ENTRY(PALETTE16,        XRGB8888,   pal16_argb32),

	ENTRY(RGB15,            XRGB1555,     rgb15_rgb555),
	ENTRY(RGB15,            ARGB1555,   rgb15_argb1555),
	ENTRY(RGB15,            ARGB8888,   rgb15_argb32),
	ENTRY(RGB15,            XRGB8888,   rgb15_argb32),

	ENTRY(RGB15_PALETTED,   ARGB8888,   rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,   XRGB8888,   rgb15pal_argb32),

	ENTRY(PALETTE16A,       ARGB8888,   pal16a_argb32),
	ENTRY(PALETTE16A,       XRGB8888,   pal16a_rgb32),

	/* rotation */
	ENTRY(ARGB32,           ARGB8888,   rot_argb32_argb32),
	ENTRY_LR(ARGB32,        XRGB8888,     rot_argb32_rgb32),
	/* Entry primarily for directfb */
	ENTRY_BM(ARGB32,        XRGB8888,     rot_argb32_rgb32, SDL_BLENDMODE_ADD),
	ENTRY_BM(ARGB32,        XRGB8888,     rot_argb32_rgb32, SDL_BLENDMODE_MOD),
	ENTRY_BM(ARGB32,        XRGB8888,     rot_argb32_rgb32, SDL_BLENDMODE_NONE),

	ENTRY(RGB32,            ARGB8888,   rot_rgb32_argb32),
	ENTRY(RGB32,            XRGB8888,     rot_argb32_argb32),

	ENTRY(RGB32_PALETTED,   ARGB8888,   rot_rgb32pal_argb32),
	ENTRY(RGB32_PALETTED,   XRGB8888,     rot_rgb32pal_argb32),

	ENTRY(YUY16,            ARGB8888,   rot_yuv16_argb32rot),
	ENTRY(YUY16,            XRGB8888,     rot_yuv16_argb32rot),

	ENTRY(YUY16_PALETTED,   ARGB8888,   rot_yuv16pal_argb32rot),
	ENTRY(YUY16_PALETTED,   XRGB8888,     rot_yuv16pal_argb32rot),

	ENTRY(PALETTE16,        ARGB8888,   rot_pal16_argb32),
	ENTRY(PALETTE16,        XRGB8888,     rot_pal16_argb32),

	ENTRY(RGB15,            XRGB1555,     rot_rgb15_argb1555),
	ENTRY(RGB15,            ARGB1555,   rot_rgb15_argb1555),
	ENTRY(RGB15,            ARGB8888,   rot_rgb15_argb32),
	ENTRY(RGB15,            XRGB8888,     rot_rgb15_argb32),

	ENTRY(RGB15_PALETTED,   ARGB8888,   rot_rgb15pal_argb32),
	ENTRY(RGB15_PALETTED,   XRGB8888,     rot_rgb15pal_argb32),

	ENTRY(PALETTE16A,       ARGB8888,   rot_pal16a_argb32),
	ENTRY(PALETTE16A,       XRGB8888,     rot_pal16a_rgb32),

	{ -1 },
};

} // anonymous namespace

} // namespace osd


#else // defined(OSD_SDL) && defined (SDLMAME_SDL3)

namespace osd { namespace { MODULE_NOT_SUPPORTED(video_sdl3_accel, OSD_RENDERER_PROVIDER, "accel") } }

#endif // defined(OSD_SDL) && defined (SDLMAME_SDL3)

MODULE_DEFINITION(RENDERER_SDL3ACCEL, osd::video_sdl3_accel)
