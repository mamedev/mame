// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0100SCN_H
#define MAME_VIDEO_TC0100SCN_H

#pragma once

#include "emupal.h"

typedef device_delegate<void (u32 *code, u16 *color)> tc0100scn_cb_delegate;
#define TC0100SCN_CB_MEMBER(_name)   void _name(u32 *code, u16 *color)

class tc0100scn_device : public device_t, public device_gfx_interface
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_tile_callback(T &&... args) { m_tc0100scn_cb = tc0100scn_cb_delegate(std::forward<T>(args)...); }
	void set_gfx_region(u8 gfxregion) { m_gfxnum = gfxregion; }
	void set_multiscr_xoffs(s16 xoffs) { m_multiscrn_xoffs = xoffs; }
	void set_multiscr_hack(bool hack) { m_multiscrn_hack = hack; }
	void set_offsets(s16 x_offset, s16 y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}
	void set_offsets_flip(s16 x_offset, s16 y_offset)
	{
		m_flip_xoffs = x_offset;
		m_flip_yoffs = y_offset;
	}
	void set_offsets_fliptx(s16 x_offset, s16 y_offset)
	{
		m_flip_text_xoffs = x_offset;
		m_flip_text_yoffs = y_offset;
	}

	static constexpr unsigned SINGLE_VDU = 1024; // for set_multiscr_xoffs

	/* Function to set separate color banks for the three tilemapped layers.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbanks(u8 bg0, u8 bg1, u8 tx);

	// read16s_handler
	u16 ram_r(offs_t offset);
	u16 ctrl_r(offs_t offset);

	// write16s_handler
	void ram_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);
	void ctrl_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	void tilemap_update();
	int tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u32 priority, u32 pri_mask = 0xff);
	void tilemap_set_dirty();

	/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
	register. Use this function to draw tilemaps in the correct order. */
	int bottomlayer();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;

private:
	// internal state
	u16     m_ctrl[8];

	tc0100scn_cb_delegate   m_tc0100scn_cb;

	std::unique_ptr<u16[]>    m_ram;
	u16 *   m_bgscroll_ram;
	u16 *   m_fgscroll_ram;
	u16 *   m_colscroll_ram;

	s16          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t    *m_tilemap[3][2];

	u8           m_bg_colbank[2], m_tx_colbank;
	u8           m_dblwidth;
	bool         m_dirty;

	u8           m_gfxnum;
	s16          m_x_offset, m_y_offset;
	s16          m_flip_xoffs, m_flip_yoffs;
	s16          m_flip_text_xoffs, m_flip_text_yoffs;
	s16          m_multiscrn_xoffs;
	bool         m_multiscrn_hack;

	required_device<gfxdecode_device> m_gfxdecode;

	template<int Offset, int Layer> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	template<int Offset, int Layout> TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void tilemap_draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, u32 priority, u32 pri_mask = 0xff);
	void set_layer_ptrs();
	void restore_scroll();
};

DECLARE_DEVICE_TYPE(TC0100SCN, tc0100scn_device)

#endif // MAME_VIDEO_TC0100SCN_H
