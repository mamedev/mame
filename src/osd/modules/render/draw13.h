// license:BSD-3-Clause
// copyright-holders:Couriersud, Olivier Galibert, R. Belmont
//============================================================
//
//  draw13.h - SDL 2.0 drawing implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#pragma once

#ifndef __DRAW13__
#define __DRAW13__

// OSD headers
#ifndef OSD_WINDOWS
#include "osdsdl.h"
#include "window.h"
#else
#include "../windows/window.h"
typedef UINT64 HashT;
#endif

// standard SDL headers
#include "sdl/sdlinc.h"

struct quad_setup_data
{
	quad_setup_data()
		: dudx(0)
		, dvdx(0)
		, dudy(0)
		, dvdy(0)
		, startu(0)
		, startv(0)
		, rotwidth(0)
		, rotheight(0)
	{
	}

	void compute(const render_primitive &prim, const int prescale);

	INT32   dudx, dvdx, dudy, dvdy;
	INT32   startu, startv;
	INT32   rotwidth, rotheight;
};

//============================================================
//  Textures
//============================================================

class renderer_sdl1;
struct copy_info_t;

/* texture_info holds information about a texture */
class texture_info
{
	friend class simple_list<texture_info>;
public:
	texture_info(renderer_sdl1 *renderer, const render_texinfo &texsource, const quad_setup_data &setup, const UINT32 flags);
	~texture_info();

	void set_data(const render_texinfo &texsource, const UINT32 flags);
	void render_quad(const render_primitive &prim, const int x, const int y);
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

	HashT hash() const { return m_hash; }
	UINT32 flags() const { return m_flags; }
	// FIXME:
	bool is_pixels_owned() const;

private:
	void set_coloralphamode(SDL_Texture *texture_id, const render_color *color);

	Uint32              m_sdl_access;
	renderer_sdl1 *     m_renderer;
	render_texinfo      m_texinfo;            // copy of the texture info
	HashT               m_hash;               // hash value for the texture (must be >= pointer size)
	UINT32              m_flags;              // rendering flags

	SDL_Texture *       m_texture_id;
	bool                m_is_rotated;

	int                 m_format;             // texture format
	SDL_BlendMode       m_sdl_blendmode;

	texture_info *      m_next;               // next texture in the list
};

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

#include "blit13.h"

struct copy_info_t
{
	int                 src_fmt;
	Uint32              dst_fmt;
	const blit_base     *blitter;
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

/* sdl_info is the information about SDL for the current screen */
class renderer_sdl1 : public osd_renderer
{
public:
	renderer_sdl1(osd_window *window, int extra_flags);

	virtual ~renderer_sdl1()
	{
		destroy_all_textures();
		SDL_DestroyRenderer(m_sdl_renderer);
		m_sdl_renderer = nullptr;
	}

	static bool init(running_machine &machine);
	static void exit();

	virtual int create() override;
	virtual int draw(const int update) override;
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
	virtual render_primitive_list *get_primitives() override;

	int RendererSupportsFormat(Uint32 format, Uint32 access, const char *sformat);

	SDL_Renderer *  m_sdl_renderer;

	static copy_info_t* s_blit_info[SDL_TEXFORMAT_LAST+1];

private:
	void expand_copy_info(const copy_info_t *list);
	void add_list(copy_info_t **head, const copy_info_t *element, Uint32 bm);

	void render_quad(texture_info *texture, const render_primitive &prim, const int x, const int y);

	texture_info *texture_find(const render_primitive &prim, const quad_setup_data &setup);
	texture_info *texture_update(const render_primitive &prim);

	void destroy_all_textures();

	INT32           m_blittimer;


	simple_list<texture_info>  m_texlist;                // list of active textures

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
	INT64           m_last_blit_time;
	INT64           m_last_blit_pixels;

	static bool s_blit_info_initialized;
	static const copy_info_t s_blit_info_default[];
};

#endif // __DRAW13__
