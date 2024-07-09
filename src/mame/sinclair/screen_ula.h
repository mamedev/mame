// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_SCREEN_ULA_H
#define MAME_SINCLAIR_SCREEN_ULA_H

#pragma once

class screen_ula_device : public device_t, public device_gfx_interface
{

public:
	screen_ula_device &set_raster_offset(u16 offset_h,  u16 offset_v) { m_offset_h = offset_h; m_offset_v = offset_v; return *this; }
	screen_ula_device &set_host_ram_ptr(const u8 *host_ram_ptr) { m_host_ram_ptr = host_ram_ptr; return *this; }
	screen_ula_device &set_palette(const char *tag, u16 base_offset, u16 alt_offset);

	void set_global_transparent(u8 global_transparent) { m_global_transparent = global_transparent; }
	void ula_palette_select_w(bool ula_palette_select) { m_ula_palette_select = ula_palette_select; }

	void ula_shadow_en_w(bool ula_shadow_en) { m_ula_shadow_en = ula_shadow_en; }
	void ulanext_en_w(bool ulanext_en) { m_ulanext_en = ulanext_en; }
	void ulanext_format_w(u8 ulanext_format);
	void ulap_en_w(bool ulap_en) { m_ulap_en = ulap_en; }
	void port_ff_reg_w(u8 port_ff_reg) { m_port_ff_reg = port_ff_reg; }

	void ula_clip_x1_w(u8 ula_clip_x1) { m_ula_clip_x1 = ula_clip_x1; }
	void ula_clip_x2_w(u8 ula_clip_x2) { m_ula_clip_x2 = ula_clip_x2; }
	void ula_clip_y1_w(u8 ula_clip_y1) { m_ula_clip_y1 = ula_clip_y1; }
	void ula_clip_y2_w(u8 ula_clip_y2) { m_ula_clip_y2 = ula_clip_y2; }
	void ula_scroll_x_w(u8 ula_scroll_x) { m_ula_scroll_x = ula_scroll_x; }
	void ula_scroll_y_w(u8 ula_scroll_y) { m_ula_scroll_y = ula_scroll_y; }
	void ula_fine_scroll_x_w (bool ula_fine_scroll_x) { m_ula_fine_scroll_x = ula_fine_scroll_x; }

	void draw_border(bitmap_rgb32 &bitmap, const rectangle &cliprect, u8 border_color);
	void draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bool flash = 0, u8 pcode = 0);

protected:
	screen_ula_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	enum ula_type : u8
	{
		ULA_TYPE_PLUS = 0,
		ULA_TYPE_NEXT
	};
	static constexpr rectangle SCREEN_AREA = { 0, 255, 0, 191 };

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void draw_ula(bitmap_rgb32 &bitmap, const rectangle &clip, bool flash, bitmap_ind8 &priority_bitmap, u8 pcode);
	void draw_hires(bitmap_rgb32 &bitmap, const rectangle &clip, bitmap_ind8 &priority_bitmap, u8 pcode);

	ula_type m_ula_type;

private:
	static inline constexpr u16 UTM_FALLBACK_PEN = 0x800;

	u16 m_offset_h, m_offset_v;
	const u8 *m_host_ram_ptr;
	u8 m_global_transparent;
	u16 m_palette_base_offset;
	u16 m_palette_alt_offset;
	bool m_ula_palette_select;

	bool m_ulanext_en;
	u8 m_ulanext_format;
	u8 m_ink_mask;
	u8 m_pap_shift;
	bool m_ulap_en;
	u8 m_port_ff_reg; // u6
	bool m_ula_shadow_en;

	u8 m_ula_clip_x1;
	u8 m_ula_clip_x2;
	u8 m_ula_clip_y1;
	u8 m_ula_clip_y2;
	u8 m_ula_scroll_x;
	u8 m_ula_scroll_y;
	bool m_ula_fine_scroll_x;

	u8 screen_mode();
	std::pair<rgb_t, rgb_t> parse_attribute(u8 attr);
};

class screen_ula_plus_device : public screen_ula_device
{
public:
	screen_ula_plus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class screen_ula_next_device : public screen_ula_device
{
public:
	screen_ula_next_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(SCREEN_ULA_PLUS, screen_ula_plus_device)
DECLARE_DEVICE_TYPE(SCREEN_ULA_NEXT, screen_ula_next_device)
#endif // MAME_SINCLAIR_SCREEN_ULA_H
