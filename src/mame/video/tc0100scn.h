// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0100SCN_H
#define MAME_VIDEO_TC0100SCN_H

#pragma once

#include "emupal.h"

class tc0100scn_device : public device_t, public device_gfx_interface
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }
	void set_multiscr_xoffs(int xoffs) { m_multiscrn_xoffs = xoffs; }
	void set_multiscr_hack(int hack) { m_multiscrn_hack = hack; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}
	void set_offsets_flip(int x_offset, int y_offset)
	{
		m_flip_xoffs = x_offset;
		m_flip_yoffs = y_offset;
	}
	void set_offsets_fliptx(int x_offset, int y_offset)
	{
		m_flip_text_xoffs = x_offset;
		m_flip_text_yoffs = y_offset;
	}

	static constexpr unsigned SINGLE_VDU = 1024; // for set_multiscr_xoffs

	/* Function to set separate color banks for the three tilemapped layers.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbanks(int bg0, int bg1, int tx);

	/* Function to set bg tilemask < 0xffff */
	void set_bg_tilemask(int mask);

	/* Function to for Mjnquest to select gfx bank */
	void gfxbank_w(u8 data);

	u16 ram_r(offs_t offset, u16 mem_mask);
	void ram_w(offs_t offset, u16 data, u16 mem_mask);
	u16 ctrl_r(offs_t offset, u16 mem_mask);
	void ctrl_w(offs_t offset, u16 data, u16 mem_mask);

	void tilemap_update();
	int tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, uint32_t priority);

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
	uint16_t     m_ctrl[8];

	std::unique_ptr<uint16_t[]>    m_ram;
	uint16_t *   m_bgscroll_ram;
	uint16_t *   m_fgscroll_ram;
	uint16_t *   m_colscroll_ram;

	int          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t    *m_tilemap[3][2];

	int          m_bg_tilemask;
	int32_t      m_gfxbank;
	int32_t      m_bg_colbank[2], m_tx_colbank;
	int          m_dblwidth;

	int          m_gfxnum;
	int          m_x_offset, m_y_offset;
	int          m_flip_xoffs, m_flip_yoffs;
	int          m_flip_text_xoffs, m_flip_text_yoffs;
	int          m_multiscrn_xoffs;
	int          m_multiscrn_hack;

	required_device<gfxdecode_device> m_gfxdecode;

	template<int Offset, int Layer> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	template<int Offset, int Layout> TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void tilemap_draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, uint32_t priority);
	void set_layer_ptrs();
	void restore_scroll();
};

DECLARE_DEVICE_TYPE(TC0100SCN, tc0100scn_device)

#endif // MAME_VIDEO_TC0100SCN_H
