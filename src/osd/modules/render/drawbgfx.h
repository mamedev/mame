// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#pragma once

#ifndef __RENDER_BGFX__
#define __RENDER_BGFX__

#include <bgfx/bgfx.h>

#include <map>
#include <vector>

#include "binpacker.h"
#include "bgfx/vertex.h"
#include "bgfx/chain.h"
#include "bgfx/chainmanager.h"
#include "sliderdirtynotifier.h"
#include "../frontend/mame/ui/menuitem.h"

class texture_manager;
class target_manager;
class shader_manager;
class effect_manager;
class chain_manager;
class bgfx_texture;
class bgfx_effect;
class bgfx_target;
class bgfx_chain;
class osd_options;
class avi_write;

/* sdl_info is the information about SDL for the current screen */
class renderer_bgfx : public osd_renderer, public slider_dirty_notifier
{
public:
	renderer_bgfx(std::shared_ptr<osd_window> w);
	virtual ~renderer_bgfx();

	static void init(running_machine &machine) { }
	static void exit();

	virtual int create() override;
	virtual int draw(const int update) override;

	virtual void add_audio_to_recording(const INT16 *buffer, int samples_this_frame) override;
	virtual std::vector<ui::menu_item> get_slider_list() override;
	virtual void set_sliders_dirty() override;

#ifdef OSD_SDL
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#endif

	virtual void save() override { }
	virtual void record() override;
	virtual void toggle_fsfx() override { }

	virtual render_primitive_list *get_primitives() override
	{
		auto win = try_getwindow();
		if (win == nullptr)
			return nullptr;

		// determines whether the screen container is transformed by the chain's shaders
		bool chain_transform = false;

		// check the first chain
		bgfx_chain* chain = this->m_chains->screen_chain(0);
		if (chain != nullptr)
		{
			chain_transform = chain->transform();
		}

		osd_dim wdim = win->get_size();
		win->target()->set_bounds(wdim.width(), wdim.height(), win->pixel_aspect());
		win->target()->set_transform_container(!chain_transform);
		return &win->target()->get_primitives();
	}

	static const char* WINDOW_PREFIX;

private:
	void vertex(ScreenVertex* vertex, float x, float y, float z, uint32_t rgba, float u, float v);
	void render_avi_quad();
	void update_recording();

	bool update_dimensions();

	void setup_view(uint32_t view_index, bool screen);
	void init_ui_view();

	void setup_matrices(uint32_t view_index, bool screen);

	void allocate_buffer(render_primitive *prim, UINT32 blend, bgfx::TransientVertexBuffer *buffer);
	enum buffer_status
	{
		BUFFER_PRE_FLUSH,
		BUFFER_FLUSH,
		BUFFER_SCREEN,
		BUFFER_EMPTY,
		BUFFER_DONE
	};
	buffer_status buffer_primitives(bool atlas_valid, render_primitive** prim, bgfx::TransientVertexBuffer* buffer, int32_t screen);

	void render_textured_quad(render_primitive* prim, bgfx::TransientVertexBuffer* buffer);
	void render_post_screen_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int32_t screen);

	void put_packed_quad(render_primitive *prim, UINT32 hash, ScreenVertex* vertex);
	void put_packed_line(render_primitive *prim, ScreenVertex* vertex);
	void put_polygon(const float* coords, UINT32 num_coords, float r, UINT32 rgba, ScreenVertex* vertex);
	void put_line(float x0, float y0, float x1, float y1, float r, UINT32 rgba, ScreenVertex* vertex, float fth = 1.0f);

	void set_bgfx_state(UINT32 blend);

	static uint32_t u32Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

	bool check_for_dirty_atlas();
	bool update_atlas();
	void process_atlas_packs(std::vector<std::vector<rectangle_packer::packed_rectangle>>& packed);
	UINT32 get_texture_hash(render_primitive *prim);

	osd_options& m_options;

	bgfx_target* m_framebuffer;
	bgfx_texture* m_texture_cache;

	// Original display_mode
	osd_dim m_dimensions;

	texture_manager* m_textures;
	target_manager* m_targets;
	shader_manager* m_shaders;
	effect_manager* m_effects;
	chain_manager* m_chains;

	bgfx_effect* m_gui_effect[4];
	bgfx_effect* m_screen_effect[4];
	std::vector<uint32_t> m_seen_views;

	std::map<UINT32, rectangle_packer::packed_rectangle> m_hash_to_entry;
	std::vector<rectangle_packer::packable_rectangle> m_texinfo;
	rectangle_packer m_packer;

	uint32_t m_width[16];
	uint32_t m_height[16];
	uint32_t m_white[16*16];
	int32_t m_ui_view;
	uint32_t m_max_view;

	avi_write* m_avi_writer;
	bgfx_target* m_avi_target;
	bgfx::TextureHandle m_avi_texture;
	bitmap_rgb32 m_avi_bitmap;
	uint8_t* m_avi_data;

	static const uint16_t CACHE_SIZE;
	static const uint32_t PACKABLE_SIZE;
	static const uint32_t WHITE_HASH;

	static bool s_window_set;
	static uint32_t s_current_view;
};

#endif
