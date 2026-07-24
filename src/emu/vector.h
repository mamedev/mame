// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Aaron Giles,Bernd Wiebelt,Allard van der Bas
#ifndef MAME_VIDEO_VECTOR_H
#define MAME_VIDEO_VECTOR_H

#pragma once

#include "render.h"

#include <utility>


class vector_device;

typedef device_delegate<void (vector_device &)> vector_update_delegate;

class vector_options
{
public:
	friend class vector_device;

	static float s_flicker;
	static float s_beam_width_min;
	static float s_beam_width_max;
	static float s_beam_dot_size;
	static float s_beam_intensity_weight;

protected:
	static void init(emu_options& options);
};

class vector_device : public device_t, public device_video_output_interface
{
public:
	using frame_begin_delegate = delegate<void ()>;
	using frame_end_delegate = delegate<void ()>;
	using move_delegate = delegate<void (int, int, uint32_t, int, int)>;
	using line_delegate = delegate<void (int, int, int, int, uint32_t, int, int, int)>;

	template <typename T> static constexpr rgb_t color111(T c) { return rgb_t(pal1bit(c >> 2), pal1bit(c >> 1), pal1bit(c >> 0)); }
	template <typename T> static constexpr rgb_t color222(T c) { return rgb_t(pal2bit(c >> 4), pal2bit(c >> 2), pal2bit(c >> 0)); }
	template <typename T> static constexpr rgb_t color444(T c) { return rgb_t(pal4bit(c >> 8), pal4bit(c >> 4), pal4bit(c >> 0)); }

	// construction/destruction
	vector_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_video_output_interface implementation
	bool has_beam() const override { return false; }
	bool is_vector() const override { return true; }
	virtual const char *output_type_name() const override { return "vector"; }
	attotime frame_period() const override { return m_frame_period; }
	void override_frame_period(attotime period) override;
	bool has_palette() const override { return false; }
	device_palette_interface &palette() const override { throw emu_fatalerror("vector_device has no palette"); }
	rectangle visible_area() const override { return m_visarea; }
	std::pair<unsigned, unsigned> physical_aspect() const override;

	void clear_list();
	void add_point(int x, int y, rgb_t color, int intensity);

	// configuration
	template <typename T> vector_device &set_refresh_hz(T &&hz) { m_frame_period = attotime::from_hz(hz); return *this; }
	void set_visarea(s16 minx, s16 maxx, s16 miny, s16 maxy) { m_visarea = rectangle(minx, maxx, miny, maxy); }
	void set_color(rgb_t color) { m_color = color; }
	auto screen_vblank() { return m_vblank.bind(); }

	template <typename F>
	void set_vector_update(F &&callback, const char *name)
	{
		m_vector_update.set(std::forward<F>(callback), name);
	}

	template <typename T, typename F>
	void set_vector_update(T &&target, F &&callback, const char *name)
	{
		m_vector_update.set(std::forward<T>(target), std::forward<F>(callback), name);
	}

	// getters
	rgb_t color() const { return m_color; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// notifiers
	util::notifier_subscription add_frame_begin_notifier(frame_begin_delegate &&n);
	template <typename T>
	util::notifier_subscription add_frame_begin_notifier(T &&n)
	{ return add_frame_begin_notifier(frame_begin_delegate(std::forward<T>(n))); }

	util::notifier_subscription add_frame_end_notifier(frame_end_delegate &&n);
	template <typename T>
	util::notifier_subscription add_frame_end_notifier(T &&n)
	{ return add_frame_end_notifier(frame_end_delegate(std::forward<T>(n))); }

	util::notifier_subscription add_move_notifier(move_delegate &&n);
	template <typename T>
	util::notifier_subscription add_move_notifier(T &&n)
	{ return add_move_notifier(move_delegate(std::forward<T>(n))); }

	util::notifier_subscription add_line_notifier(line_delegate &&n);
	template <typename T>
	util::notifier_subscription add_line_notifier(T &&n)
	{ return add_line_notifier(line_delegate(std::forward<T>(n))); }

private:
	TIMER_CALLBACK_MEMBER(vblank_timer_callback);
	float normalized_sigmoid(float n, float k);

	/* The vertices are buffered here */
	struct point
	{
		point() : x(0), y(0), col(0), intensity(0) { }

		int x; int y;
		rgb_t col;
		int intensity;
	};

	std::unique_ptr<point[]> m_vector_list;
	int m_vector_index;
	int m_min_intensity;
	int m_max_intensity;

	// vector screen configuration
	rectangle m_visarea;
	attotime m_frame_period;
	rgb_t m_color;
	devcb_write_line m_vblank;
	vector_update_delegate m_vector_update;

	emu_timer *m_vblank_timer;

	// notify interested parties about vector-drawing activities
	util::notifier<> m_frame_begin_notifier;
	util::notifier<> m_frame_end_notifier;
	util::notifier<int, int, uint32_t, int, int> m_move_notifier;
	util::notifier<int, int, int, int, uint32_t, int, int, int> m_line_notifier;

	bool video_output_update() override;
};

// device type definition
DECLARE_DEVICE_TYPE(VECTOR, vector_device)

// device iterator
typedef device_type_enumerator<vector_device> vector_device_enumerator;

#endif // MAME_VIDEO_VECTOR_H
