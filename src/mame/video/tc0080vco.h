// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0080VCO_H
#define MAME_VIDEO_TC0080VCO_H

#pragma once

class tc0080vco_device : public device_t
{
public:
	tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int gfxnum) { m_gfxnum = gfxnum; }
	void set_tx_region(int txnum) { m_txnum = txnum; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_bg_xoffs = x_offset;
		m_bg_yoffs = y_offset;
	}
	void set_bgflip_yoffs(int offs) { m_bg_flip_yoffs = offs; }

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask = 0xff);
	void set_fg0_debug(bool debug) { m_has_fg0 = debug ? 0 : 1; }

	u16 cram_0_r(int offset);
	u16 cram_1_r(int offset);
	u16 sprram_r(int offset);
	u16 scrram_r(int offset);
	DECLARE_WRITE16_MEMBER( scrollram_w );
	READ_LINE_MEMBER( flipscreen_r );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;

private:
	// internal state
	std::unique_ptr<u16[]>       m_ram;
	u16 *       m_bg0_ram_0;
	u16 *       m_bg0_ram_1;
	u16 *       m_bg1_ram_0;
	u16 *       m_bg1_ram_1;
	u16 *       m_tx_ram_0;
	u16 *       m_tx_ram_1;
	u16 *       m_char_ram;
	u16 *       m_bgscroll_ram;

/* FIXME: This sprite related stuff still needs to be accessed in video/taito_h */
	u16 *       m_chain_ram_0;
	u16 *       m_chain_ram_1;
	u16 *       m_spriteram;
	u16 *       m_scroll_ram;

	u16         m_bg0_scrollx;
	u16         m_bg0_scrolly;
	u16         m_bg1_scrollx;
	u16         m_bg1_scrolly;

	tilemap_t      *m_tilemap[3];

	s32          m_flipscreen;

	int            m_gfxnum;
	int            m_txnum;
	int            m_bg_xoffs, m_bg_yoffs;
	int            m_bg_flip_yoffs;
	int            m_has_fg0; // for debug, it can be enabled with set_fg0_debug(true)

	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	void bg0_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
	void bg1_tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, u8 priority, u8 pmask = 0xff);
};

DECLARE_DEVICE_TYPE(TC0080VCO, tc0080vco_device)

#endif // MAME_VIDEO_TC0080VCO_H
