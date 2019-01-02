// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#ifndef MAME_INCLUDES_XORWORLD_H
#define MAME_INCLUDES_XORWORLD_H

#pragma once

#include "machine/eepromser.h"
#include "emupal.h"

class xorworld_state : public driver_device
{
public:
	xorworld_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void xorworld(machine_config &config);

	void init_xorworld();

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE16_MEMBER(irq2_ack_w);
	DECLARE_WRITE16_MEMBER(irq6_ack_w);
	DECLARE_WRITE16_MEMBER(videoram_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;
	void xorworld_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	void xorworld_map(address_map &map);
};

#endif // MAME_INCLUDES_XORWORLD_H
