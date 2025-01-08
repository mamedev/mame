// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_PC080SN_H
#define MAME_TAITO_PC080SN_H

#pragma once

#include "tilemap.h"


class pc080sn_device : public device_t, public device_gfx_interface
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: pc080sn_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	void set_yinvert(int y_inv) { m_y_invert = y_inv; }
	void set_dblwidth(bool dblwidth) { m_dblwidth = dblwidth; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void xscroll_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void yscroll_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void set_scroll(int tilemap_num, int scrollx, int scrolly);
	void set_trans_pen(int tilemap_num, int pen);
	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_draw_offset(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, int xoffs, int yoffs, u8 pmask = 0xff);
	void topspeed_custom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u16 *color_ctrl_ram, u8 pmask = 0xff);

	/* For Topspeed */
	void tilemap_draw_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u16 *ram, u8 pmask = 0xff);

	void restore_scroll();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	template <unsigned N>
	TILE_GET_INFO_MEMBER(get_tile_info);

	// internal state
	u16            m_ctrl[8];

	std::unique_ptr<u16[]>         m_ram;
	u16            *m_bg_ram[2];
	u16            *m_bgscroll_ram[2];

	int            m_bgscrollx[2], m_bgscrolly[2];

	tilemap_t      *m_tilemap[2];

	int            m_x_offset, m_y_offset;
	int            m_y_invert;
	bool           m_dblwidth;
};

DECLARE_DEVICE_TYPE(PC080SN, pc080sn_device)

#endif // MAME_TAITO_PC080SN_H
