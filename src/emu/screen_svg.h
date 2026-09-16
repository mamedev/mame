// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    screen_svg.h

    Core MAME SVG video output device.

***************************************************************************/

#ifndef MAME_EMU_SCREEN_SVG_H
#define MAME_EMU_SCREEN_SVG_H

#pragma once

#include "divo.h"
#include "nanosvg.h"

class screen_svg_device;

typedef device_delegate<void (screen_svg_device &)> screen_svg_update_delegate;

class screen_svg_device : public device_t, public device_video_output_interface
{
public:
	// construction/destruction
	screen_svg_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	~screen_svg_device();

	bool has_beam() const override { return false; }
	attotime frame_period() const override { return m_frame_period; }
	std::pair<unsigned, unsigned> physical_aspect() const override;
	virtual rectangle visible_area() const override;
	virtual const char *output_type_name() const override { return "svg"; }
	void override_frame_period(attotime period) override;
	bool has_palette() const override { return false; }
	device_palette_interface &palette() const override { throw emu_fatalerror("screen_svg_device has no palette"); }
	template <typename T> screen_svg_device &set_refresh_hz(T &&hz) { m_frame_period = attotime::from_hz(hz); return *this; }
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	template <typename T> screen_svg_device &set_svg_region(T &&tag) { m_svg_region.set_tag(std::forward<T>(tag)); return *this; } // default region is device tag
	auto screen_vblank() { return m_vblank.bind(); }

	// TODO: remove that once we have the rendering telling us what the optimal size is

	// Note: there's an order issue between regions being loaded (to
	// get the aspect ratio of the svg) and the window being opened
	// (which needs the physical_aspect information).

	void set_size(int width, int height) { m_render_width = width; m_render_height = height; }

	template <typename F>
	void set_screen_svg_update(F &&callback, const char *name)
	{
		m_screen_svg_update.set(std::forward<F>(callback), name);
	}

	template <typename T, typename F>
	void set_screen_svg_update(T &&target, F &&callback, const char *name)
	{
		m_screen_svg_update.set(std::forward<T>(target), std::forward<F>(callback), name);
	}

private:
	struct paired_entry {
		int key;
		int cache_entry;
		paired_entry(int k, int c) { key = k; cache_entry = c; }
	};

	struct cached_bitmap {
		int x, y, sx, sy;
		std::vector<u32> image;
		std::vector<paired_entry> pairs;
	};

	struct bbox {
		int x0, y0, x1, y1;
	};

	required_memory_region m_svg_region; // the region in which the svg data is in
	screen_svg_update_delegate m_screen_svg_update;
	util::nsvg_image_ptr m_image;
	util::nsvg_rasterizer_ptr m_rasterizer;
	std::vector<bool> m_key_state;
	std::vector<std::vector<NSVGshape *>> m_keyed_shapes;
	util::transparent_string_unordered_map<std::string, int> m_key_ids;
	int m_key_count;
	u8              m_curbitmap;                // current bitmap index
	u8              m_curtexture;               // current texture index
	bitmap_rgb32    m_bitmap[2];                // 2x bitmaps for rendering
	render_texture *m_texture[2];               // 2x textures for the bitmaps

	int m_sx, m_sy;
	int m_render_width, m_render_height;
	double m_scale;
	std::vector<u32> m_background;

	std::vector<cached_bitmap> m_cache;
	attotime m_frame_period;
	devcb_write_line m_vblank;
	emu_timer *m_vblank_timer;

	TIMER_CALLBACK_MEMBER(vblank_timer_callback);
	void svg_init();
	static void output_notifier(void *param, osd::output_item const &output, s32 seconds, s64 attoseconds);
	void output_change(const osd::output_item &output);
	void render_state(std::vector<u32> &dest, const std::vector<bool> &state);
	void compute_initial_bboxes(std::vector<bbox> &bboxes);
	bool compute_mask_intersection_bbox(int key1, int key2, bbox &bb) const;
	void compute_diff_image(const std::vector<u32> &rend, const bbox &bb, cached_bitmap &dest) const;
	void compute_dual_diff_image(const std::vector<u32> &rend, const bbox &bb, const cached_bitmap &src1, const cached_bitmap &src2, cached_bitmap &dest) const;
	void rebuild_cache();
	void blit(bitmap_rgb32 &bitmap, const cached_bitmap &src) const;
	bool video_output_update() override;
};

// device type definition
DECLARE_DEVICE_TYPE(SCREEN_SVG, screen_svg_device)

#endif // MAME_EMU_SCREEN_SVG_H
