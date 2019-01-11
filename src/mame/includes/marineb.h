// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
#ifndef MAME_INCLUDES_MARINEB_H
#define MAME_INCLUDES_MARINEB_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"

class marineb_state : public driver_device
{
public:
	marineb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch")
	{ }

	void springer(machine_config &config);
	void wanted(machine_config &config);
	void hopprobo(machine_config &config);
	void marineb(machine_config &config);
	void bcruzm12(machine_config &config);
	void hoccer(machine_config &config);
	void changes(machine_config &config);

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint8_t     m_palette_bank;
	uint8_t     m_column_scroll;
	uint8_t     m_flipscreen_x;
	uint8_t     m_flipscreen_y;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;

	bool     m_irq_mask;
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(marineb_videoram_w);
	DECLARE_WRITE8_MEMBER(marineb_colorram_w);
	DECLARE_WRITE8_MEMBER(marineb_column_scroll_w);
	DECLARE_WRITE8_MEMBER(marineb_palette_bank_0_w);
	DECLARE_WRITE8_MEMBER(marineb_palette_bank_1_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_y_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void marineb_palette(palette_device &palette) const;
	uint32_t screen_update_marineb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_changes(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_springer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hoccer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hopprobo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(marineb_vblank_irq);
	DECLARE_WRITE_LINE_MEMBER(wanted_vblank_irq);
	void set_tilemap_scrolly( int cols );
	void marineb_io_map(address_map &map);
	void marineb_map(address_map &map);
	void wanted_io_map(address_map &map);
};

#endif // MAME_INCLUDES_MARINEB_H
