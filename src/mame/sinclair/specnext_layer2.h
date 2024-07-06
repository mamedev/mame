// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SPECNEXT_LAYER2_H
#define MAME_SINCLAIR_SPECNEXT_LAYER2_H

#pragma once

class specnext_layer2_device : public device_t, public device_gfx_interface
{

public:
	specnext_layer2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	specnext_layer2_device &set_raster_offset(u16 offset_h,  u16 offset_v) { m_offset_h = offset_h; m_offset_v = offset_v; return *this; }
	specnext_layer2_device &set_host_ram_ptr(const u8 *host_ram_ptr) { m_host_ram_ptr = host_ram_ptr; return *this; }
	specnext_layer2_device &set_palette(const char *tag, u16 base_offset, u16 alt_offset);

	void set_global_transparent(u8 global_transparent) { m_global_transparent = global_transparent; }
	void layer2_palette_select_w(bool layer2_palette_select) { m_layer2_palette_select = layer2_palette_select; }
	void pen_priority_w(u16 pen, bool priority) { m_pen_priority[pen] = priority; }

	void layer2_en_w(bool layer2_en) { m_layer2_en = layer2_en; }
	void resolution_w(u8 resolution) { m_resolution = resolution & 0x03; }
	void palette_offset_w(u8 palette_offset) { m_palette_offset = palette_offset & 0x0f; }
	void layer2_active_bank_w(u8 layer2_active_bank) { m_layer2_active_bank = layer2_active_bank & 0x7f; }

	void scroll_x_w(u16 scroll_x) { m_scroll_x = scroll_x & 0x1ff; }
	void scroll_y_w(u8 scroll_y) { m_scroll_y = scroll_y; }
	void clip_x1_w(u8 clip_x1) { m_clip_x1 = clip_x1; }
	void clip_x2_w(u8 clip_x2) { m_clip_x2 = clip_x2; }
	void clip_y1_w(u8 clip_y1) { m_clip_y1 = clip_y1; }
	void clip_y2_w(u8 clip_y2) { m_clip_y2 = clip_y2; }

	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 pcode = 0, u8 priority_mask = 0, u8 mixer = 0);
	void draw_mix(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 mixer);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 m_offset_h, m_offset_v;
	const u8 *m_host_ram_ptr;
	u8 m_global_transparent;
	u16 m_palette_base_offset;
	u16 m_palette_alt_offset;
	bool m_layer2_palette_select;
	bool m_pen_priority[512 * 4];

	bool m_layer2_en;
	u8 m_resolution; // u2: 00 = 256x192, 01 = 320x256, 1X = 640x256x4
	u8 m_palette_offset; // u4
	u8 m_layer2_active_bank; // u7

	u16 m_scroll_x; // u9
	u8 m_scroll_y;
	u8 m_clip_x1;
	u8 m_clip_x2;
	u8 m_clip_y1;
	u8 m_clip_y2;
};


DECLARE_DEVICE_TYPE(SPECNEXT_LAYER2, specnext_layer2_device)
#endif // MAME_SINCLAIR_SPECNEXT_LAYER2_H
