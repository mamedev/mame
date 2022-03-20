// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Bank Panic

***************************************************************************/
#ifndef MAME_INCLUDES_BANKP_H
#define MAME_INCLUDES_BANKP_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class bankp_state : public driver_device
{
public:
	bankp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void bankp(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_colorram2;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	int     m_scroll_x = 0;
	int     m_priority = 0;

	uint8_t m_nmi_mask;
	void scroll_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void videoram2_w(offs_t offset, uint8_t data);
	void colorram2_w(offs_t offset, uint8_t data);
	void out_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	void bankp_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void bankp_io_map(address_map &map);
	void bankp_map(address_map &map);
};

#endif // MAME_INCLUDES_BANKP_H
