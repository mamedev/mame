// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Aaron Giles,Bernd Wiebelt,Allard van der Bas
#ifndef MAME_VIDEO_VECTOR_H
#define MAME_VIDEO_VECTOR_H

#pragma once

#include "notifier.h"

#include <utility>


class vector_device;

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

class vector_device : public device_t, public device_video_interface
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

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void clear_list();

	void add_point(int x, int y, rgb_t color, int intensity);

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

	// notify interested parties about vector-drawing activities
	util::notifier<> m_frame_begin_notifier;
	util::notifier<> m_frame_end_notifier;
	util::notifier<int, int, uint32_t, int, int> m_move_notifier;
	util::notifier<int, int, int, int, uint32_t, int, int, int> m_line_notifier;
};

// device type definition
DECLARE_DEVICE_TYPE(VECTOR, vector_device)

// device iterator
typedef device_type_enumerator<vector_device> vector_device_enumerator;

#endif // MAME_VIDEO_VECTOR_H
