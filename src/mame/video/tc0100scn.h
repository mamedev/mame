// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0100SCN_H
#define MAME_VIDEO_TC0100SCN_H

#pragma once

#include "emupal.h"

class tc0100scn_device : public device_t
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_palette_tag(const char *tag) { m_palette.set_tag(tag); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }
	void set_tx_region(int txregion) { m_txnum = txregion; }
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
	DECLARE_WRITE16_MEMBER(gfxbank_w);

	DECLARE_READ16_MEMBER(word_r);
	DECLARE_WRITE16_MEMBER(word_w);
	DECLARE_READ16_MEMBER(ctrl_word_r);
	DECLARE_WRITE16_MEMBER(ctrl_word_w);

	/* Functions for use with 68020 (Under Fire) */
	DECLARE_READ32_MEMBER(long_r);
	DECLARE_WRITE32_MEMBER(long_w);
	DECLARE_READ32_MEMBER(ctrl_long_r);
	DECLARE_WRITE32_MEMBER(ctrl_long_w);

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
	uint16_t       m_ctrl[8];

	std::unique_ptr<uint16_t[]>    m_ram;
	uint16_t *     m_bg_ram;
	uint16_t *     m_fg_ram;
	uint16_t *     m_tx_ram;
	uint16_t *     m_char_ram;
	uint16_t *     m_bgscroll_ram;
	uint16_t *     m_fgscroll_ram;
	uint16_t *     m_colscroll_ram;

	int          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t      *m_tilemap[3][2];

	int          m_bg_tilemask;
	int32_t        m_gfxbank;
	int32_t        m_bg0_colbank, m_bg1_colbank, m_tx_colbank;
	int          m_dblwidth;

	int          m_gfxnum;
	int          m_txnum;
	int          m_x_offset, m_y_offset;
	int          m_flip_xoffs, m_flip_yoffs;
	int          m_flip_text_xoffs, m_flip_text_yoffs;
	int          m_multiscrn_xoffs;
	int          m_multiscrn_hack;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void common_get_tile_info(tile_data &tileinfo, int tile_index, uint16_t *ram, int colbank);

	void tilemap_draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, uint32_t priority);
	void set_layer_ptrs();
	void dirty_tilemaps();
	void restore_scroll();
};

DECLARE_DEVICE_TYPE(TC0100SCN, tc0100scn_device)


#define MCFG_TC0100SCN_GFX_REGION(_region) \
	downcast<tc0100scn_device &>(*device).set_gfx_region(_region);

#define MCFG_TC0100SCN_TX_REGION(_region) \
	downcast<tc0100scn_device &>(*device).set_tx_region(_region);

#define MCFG_TC0100SCN_OFFSETS(_xoffs, _yoffs) \
	downcast<tc0100scn_device &>(*device).set_offsets(_xoffs, _yoffs);

#define MCFG_TC0100SCN_OFFSETS_FLIP(_xoffs, _yoffs) \
	downcast<tc0100scn_device &>(*device).set_offsets_flip(_xoffs, _yoffs);

#define MCFG_TC0100SCN_OFFSETS_FLIPTX(_xoffs, _yoffs) \
	downcast<tc0100scn_device &>(*device).set_offsets_fliptx(_xoffs, _yoffs);

#define MCFG_TC0100SCN_MULTISCR_XOFFS(_xoffs) \
	downcast<tc0100scn_device &>(*device).set_multiscr_xoffs(_xoffs);

#define MCFG_TC0100SCN_MULTISCR_HACK(_hack) \
	downcast<tc0100scn_device &>(*device).set_multiscr_hack(_hack);

#define MCFG_TC0100SCN_GFXDECODE(_gfxtag) \
	downcast<tc0100scn_device &>(*device).set_gfxdecode_tag(_gfxtag);

#define MCFG_TC0100SCN_PALETTE(_palette_tag) \
	downcast<tc0100scn_device &>(*device).set_palette_tag(_palette_tag);

#endif // MAME_VIDEO_TC0100SCN_H
