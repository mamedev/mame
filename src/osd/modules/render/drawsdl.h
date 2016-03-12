// license:BSD-3-Clause
// copyright-holders:Couriersud, Olivier Galibert, R. Belmont
//============================================================
//
//  drawsdl.h - SDL software and OpenGL implementation
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//  yuvmodes by Couriersud
//
//============================================================

#pragma once

#ifndef __DRAWSDL2__
#define __DRAWSDL2__

/* renderer_sdl2 is the information about SDL for the current screen */
class renderer_sdl2 : public osd_renderer
{
public:

	renderer_sdl2(osd_window *w, int extra_flags)
		: osd_renderer(w, extra_flags)
		, m_sdl_renderer(nullptr)
		, m_texture_id(nullptr)
		, m_yuv_lookup(nullptr)
		, m_yuv_bitmap(nullptr)
		//, m_hw_scale_width(0)
		//, m_hw_scale_height(0)
		, m_last_hofs(0)
		, m_last_vofs(0)
		, m_blit_dim(0, 0)
		, m_last_dim(0, 0)
	{
	}
	virtual ~renderer_sdl2();

	static bool init(running_machine &machine);
	static void exit() { }

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

	INT32               m_blittimer;

	SDL_Renderer        *m_sdl_renderer;
	SDL_Texture         *m_texture_id;

	// YUV overlay
	UINT32              *m_yuv_lookup;
	UINT16              *m_yuv_bitmap;

	// if we leave scaling to SDL and the underlying driver, this
	// is the render_target_width/height to use

	int                 m_last_hofs;
	int                 m_last_vofs;
	osd_dim             m_blit_dim;
	osd_dim             m_last_dim;
};

struct sdl_scale_mode
{
	const char      *name;
	int             is_scale;           /* Scale mode?           */
	int             is_yuv;             /* Yuv mode?             */
	int             mult_w;             /* Width multiplier      */
	int             mult_h;             /* Height multiplier     */
	const char      *sdl_scale_mode_hint;        /* what to use as a hint ? */
	int             pixel_format;       /* Pixel/Overlay format  */
	void            (*yuv_blit)(const UINT16 *bitmap, UINT8 *ptr, const int pitch, const UINT32 *lookup, const int width, const int height);
};

#endif // __DRAWSDL2__
