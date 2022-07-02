// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont
#ifndef MAME_INCLUDES_GCPINBAL_H
#define MAME_INCLUDES_GCPINBAL_H

#pragma once

#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "sound/es8712.h"
#include "sound/okim6295.h"
#include "excellent_spr.h"
#include "emupal.h"
#include "machine/timer.h"
#include "screen.h"
#include "tilemap.h"

class gcpinbal_state : public driver_device
{
public:
	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tilemapram(*this, "tilemapram")
		, m_d80010_ram(*this, "d80010")
		, m_d80060_ram(*this, "d80060")
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_watchdog(*this, "watchdog")
		, m_oki(*this, "oki")
		, m_essnd(*this, "essnd")
		, m_sprgen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{ }

	void gcpinbal(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_tilemapram;
	required_shared_ptr<u16> m_d80010_ram;
	required_shared_ptr<u16> m_d80060_ram;

	/* video-related */
	tilemap_t     *m_tilemap[3]{};
	u16      m_scrollx[3]{};
	u16      m_scrolly[3]{};
	u16      m_bg0_gfxset = 0U;
	u16      m_bg1_gfxset = 0U;
#ifdef MAME_DEBUG
	u8       m_dislayer[4] = { 0, 0, 0, 0 };
#endif

	/* sound-related */
	u32      m_msm_bank = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<mb3773_device> m_watchdog;
	required_device<okim6295_device> m_oki;
	required_device<es8712_device> m_essnd;
	required_device<excellent_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void d80010_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void d80040_w(offs_t offset, u8 data);
	void d80060_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bank_w(u8 data);
	void eeprom_w(u8 data);
	void es8712_reset_w(u8 data);
	void tilemaps_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void gcpinbal_colpri_cb(u32 &colour, u32 &pri_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	void gcpinbal_map(address_map &map);
};

#endif // MAME_INCLUDES_GCPINBAL_H
