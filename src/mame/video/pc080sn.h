// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_PC080SN_H
#define MAME_VIDEO_PC080SN_H

#pragma once

#include "tilemap.h"


class pc080sn_device : public device_t
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }
	void set_yinvert(int y_inv) { m_y_invert = y_inv; }
	void set_dblwidth(int dblwidth) { m_dblwidth = dblwidth; }
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

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void common_get_pc080sn_bg_tile_info( tile_data &tileinfo, int tile_index, u16 *ram, int gfxnum );
	void common_get_pc080sn_fg_tile_info( tile_data &tileinfo, int tile_index, u16 *ram, int gfxnum );

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
	virtual void device_start() override;
	virtual void device_post_load() override;

private:
	// internal state
	u16         m_ctrl[8];

	std::unique_ptr<u16[]>         m_ram;
	u16         *m_bg_ram[2];
	u16         *m_bgscroll_ram[2];

	int            m_bgscrollx[2], m_bgscrolly[2];

	tilemap_t      *m_tilemap[2];

	int            m_gfxnum;
	int            m_x_offset, m_y_offset;
	int            m_y_invert;
	int            m_dblwidth;

	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(PC080SN, pc080sn_device)

#endif // MAME_VIDEO_PC080SN_H
