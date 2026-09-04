// license:BSD-3-Clause
// copyright-holders:Hans Andersson
//============================================================
//
//  vectorrenderer.h - Persistent GPU vector CRT simulation
//
//============================================================

#ifndef MAME_RENDER_BGFX_VECTORRENDERER_H
#define MAME_RENDER_BGFX_VECTORRENDERER_H

#pragma once

#include <bgfx/bgfx.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


class bgfx_effect;
class effect_manager;
class osd_options;
class render_primitive;
struct slider_state;

namespace ui { class menu_item; }


class bgfx_vector_renderer
{
public:
	bgfx_vector_renderer(effect_manager &effects, osd_options const &options);
	~bgfx_vector_renderer();

	bgfx_vector_renderer(bgfx_vector_renderer const &) = delete;
	bgfx_vector_renderer &operator=(bgfx_vector_renderer const &) = delete;

	// Submit decay, beam deposition, and bloom passes.  The view is advanced
	// for every submitted pass.  Emulated time is used so paused emulation does
	// not repeatedly excite the phosphor with the same display list.
	void prepare(uint32_t &view, render_primitive *first, uint16_t width, uint16_t height, double emu_time);

	// Composite the current HDR phosphor image into an already configured view.
	void composite(uint16_t view);

	bool available() const { return m_available; }
	bool present() const { return m_present; }
	void append_sliders(std::vector<ui::menu_item> &items);

private:
	struct target
	{
		bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
		bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
	};

	enum slider_id : uint8_t
	{
		SLIDER_PERSISTENCE,
		SLIDER_BEAM_WIDTH,
		SLIDER_BEAM_INTENSITY,
		SLIDER_HALO,
		SLIDER_BLOOM_STRENGTH,
		SLIDER_BLOOM_RADIUS,
		SLIDER_EXPOSURE,
		SLIDER_COUNT
	};

	bool create_geometry();
	bool create_targets(uint16_t width, uint16_t height);
	void destroy_geometry();
	void destroy_targets();
	void setup_view(uint16_t view, bgfx::FrameBufferHandle framebuffer, uint16_t width, uint16_t height, bool clear);
	void bind_post_geometry();
	void set_uniform(bgfx_effect *effect, char const *name, float x, float y = 0.0f, float z = 0.0f, float w = 0.0f);
	void draw_post(bgfx_effect *effect, uint16_t view);
	void draw_beams(uint16_t view, double frame_time);
	int32_t slider_changed(slider_id id, std::string *text, int32_t new_value);
	void create_sliders();

	bgfx_effect *m_decay_effect;
	bgfx_effect *m_beam_effect;
	bgfx_effect *m_downsample_effect;
	bgfx_effect *m_blur_effect;
	bgfx_effect *m_composite_effect;

	target m_accumulation[2];
	target m_bloom[2];
	bgfx::VertexBufferHandle m_post_vertices;
	bgfx::VertexBufferHandle m_beam_vertices;
	bgfx::VertexLayout m_beam_layout;

	std::vector<render_primitive *> m_vectors;
	std::vector<std::unique_ptr<slider_state> > m_sliders;

	uint16_t m_width;
	uint16_t m_height;
	uint16_t m_bloom_width;
	uint16_t m_bloom_height;
	uint8_t m_current_accumulation;
	double m_last_emu_time;
	bool m_have_time;
	bool m_reset_accumulation;
	bool m_available;
	bool m_present;

	float m_persistence;
	float m_beam_width;
	float m_beam_intensity;
	float m_halo;
	float m_bloom_strength;
	float m_bloom_radius;
	float m_exposure;
};

#endif // MAME_RENDER_BGFX_VECTORRENDERER_H
