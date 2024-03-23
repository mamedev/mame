// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_RENDER_DRAWBGFX_H
#define MAME_RENDER_DRAWBGFX_H

#pragma once

#include "binpacker.h"
#include "bgfx/chain.h"
#include "bgfx/chainmanager.h"
#include "bgfx/vertex.h"
#include "sliderdirtynotifier.h"

#include "modules/osdwindow.h"

#include "notifier.h"

#include <bgfx/bgfx.h>

#include <map>
#include <memory>
#include <vector>


class texture_manager;
class target_manager;
class shader_manager;
class effect_manager;
class bgfx_texture;
class bgfx_effect;
class bgfx_target;
class bgfx_view;
class osd_options;
class avi_write;

/* renderer_bgfx is the information about BGFX for the current screen */
class renderer_bgfx : public osd_renderer, public slider_dirty_notifier
{
public:
	class parent_module;

	renderer_bgfx(osd_window &window, parent_module &parent_module);
	virtual ~renderer_bgfx();

	virtual int create() override;
	virtual int draw(const int update) override;

	virtual void add_audio_to_recording(const int16_t *buffer, int samples_this_frame) override;
	virtual std::vector<ui::menu_item> get_slider_list() override;
	virtual void set_sliders_dirty() override;

#ifdef OSD_SDL
	virtual int xy_to_render_target(const int x, const int y, int *xt, int *yt) override;
#endif

	virtual void save() override { }
	virtual void record() override;
	virtual void toggle_fsfx() override { }

	uint32_t get_window_width(uint32_t index) const;
	uint32_t get_window_height(uint32_t index) const;

	virtual render_primitive_list *get_primitives() override;

	static char const *const WINDOW_PREFIX;

private:
	enum buffer_status
	{
		BUFFER_PRE_FLUSH,
		BUFFER_FLUSH,
		BUFFER_SCREEN,
		BUFFER_EMPTY,
		BUFFER_DONE
	};

	class parent_module_holder
	{
	public:
		parent_module_holder(parent_module &parent);
		~parent_module_holder();
		parent_module &operator()() const { return m_parent; }
	private:
		parent_module &m_parent;
	};

	void vertex(ScreenVertex* vertex, float x, float y, float z, uint32_t rgba, float u, float v);
	void render_avi_quad();
	void update_recording();

	bool update_dimensions();

	void setup_ortho_view();

	void allocate_buffer(render_primitive *prim, uint32_t blend, bgfx::TransientVertexBuffer *buffer);
	buffer_status buffer_primitives(bool atlas_valid, render_primitive** prim, bgfx::TransientVertexBuffer* buffer, int32_t screen, int window_index);

	void render_textured_quad(render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int window_index);
	void render_post_screen_quad(int view, render_primitive* prim, bgfx::TransientVertexBuffer* buffer, int32_t screen, int window_index);

	void put_packed_quad(render_primitive *prim, uint32_t hash, ScreenVertex* vertex);
	void put_packed_line(render_primitive *prim, ScreenVertex* vertex);
	void put_polygon(const float* coords, uint32_t num_coords, float r, uint32_t rgba, ScreenVertex* vertex);
	void put_line(float x0, float y0, float x1, float y1, float r, uint32_t rgba, ScreenVertex* vertex, float fth = 1.0f);

	void set_bgfx_state(uint32_t blend);

	static uint32_t u32Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

	bool check_for_dirty_atlas();
	bool update_atlas();
	void process_atlas_packs(std::vector<std::vector<rectangle_packer::packed_rectangle>>& packed);
	uint32_t get_texture_hash(render_primitive *prim);

	void load_config(util::xml::data_node const &parentnode);
	void save_config(util::xml::data_node &parentnode);

	parent_module_holder m_module; // keep this where it will be destructed last

	bgfx_target *m_framebuffer;
	bgfx_texture *m_texture_cache;

	// Original display_mode
	osd_dim m_dimensions;

	std::unique_ptr<texture_manager> m_textures;
	std::unique_ptr<target_manager> m_targets;
	std::unique_ptr<shader_manager> m_shaders;
	std::unique_ptr<effect_manager> m_effects;
	std::unique_ptr<chain_manager> m_chains;

	bgfx_effect *m_gui_effect[4];
	bgfx_effect *m_screen_effect[4];
	std::vector<uint32_t> m_seen_views;

	std::map<uint32_t, rectangle_packer::packed_rectangle> m_hash_to_entry;
	std::vector<rectangle_packer::packable_rectangle> m_texinfo;
	rectangle_packer m_packer;

	uint32_t m_white[16*16];
	std::unique_ptr<bgfx_view> m_ortho_view;
	uint32_t m_max_view;

	bgfx_view *m_avi_view;
	avi_write *m_avi_writer;
	bgfx_target *m_avi_target;
	bgfx::TextureHandle m_avi_texture;
	bitmap_rgb32 m_avi_bitmap;
	uint8_t *m_avi_data;

	std::unique_ptr<util::xml::file> m_config;
	const util::notifier_subscription m_load_sub;
	const util::notifier_subscription m_save_sub;

	static const uint16_t CACHE_SIZE;
	static const uint32_t PACKABLE_SIZE;
	static const uint32_t WHITE_HASH;

	static uint32_t s_current_view;
	static uint32_t s_width[16];
	static uint32_t s_height[16];
};

#endif // MAME_RENDER_DRAWBGFX_H
