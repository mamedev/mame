// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#pragma once

#include "window.h"
#include <climits>

class renderer_bgfx;
class bgfx_target;

class bgfx_view {
public:
	bgfx_view(renderer_bgfx *renderer, uint32_t index, bgfx_target *backbuffer, std::vector<uint32_t> &seen_views)
		: m_renderer(renderer)
		, m_index(index)
		, m_backbuffer(backbuffer)
		, m_seen_views(seen_views)
		, m_window_index(UINT_MAX)
		, m_view_width(0)
		, m_view_height(0)
		, m_z_near(0.0f)
		, m_z_far(100.0f)
		, m_clear_color(0)
		, m_clear_depth(1.0f)
		, m_clear_stencil(0)
		, m_do_clear_color(true)
		, m_do_clear_depth(true)
		, m_do_clear_stencil(false) {
	}
	virtual ~bgfx_view() { }

	void update();

	void set_backbuffer(bgfx_target *backbuffer) { m_backbuffer = backbuffer; }
	void set_index(uint32_t index) { m_index = index; }
	uint32_t get_index() const { return m_index; }

	virtual void setup() = 0;
	virtual void setup_matrices() = 0;

protected:
	renderer_bgfx *m_renderer;
	uint32_t m_index;
	bgfx_target *m_backbuffer;
	std::vector<uint32_t> &m_seen_views;

	uint32_t m_window_index;
	uint32_t m_view_width;
	uint32_t m_view_height;
	float m_z_near;
	float m_z_far;

	uint32_t m_clear_color;
	float m_clear_depth;
	uint8_t m_clear_stencil;

	bool m_do_clear_color;
	bool m_do_clear_depth;
	bool m_do_clear_stencil;
};

class bgfx_ortho_view : public bgfx_view {
public:
	bgfx_ortho_view(renderer_bgfx *renderer, uint32_t index, bgfx_target *backbuffer, std::vector<uint32_t> &seen_views) : bgfx_view(renderer, index, backbuffer, seen_views) { }

	void setup() override;
	void setup_matrices() override;
};
