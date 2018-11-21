// license:BSD-3-Clause
// copyright-holders:David Graves, R. Belmont
#ifndef MAME_INCLUDES_GCPINBAL_H
#define MAME_INCLUDES_GCPINBAL_H

#pragma once

#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "sound/es8712.h"
#include "sound/okim6295.h"
#include "video/excellent_spr.h"
#include "emupal.h"

class gcpinbal_state : public driver_device
{
public:
	gcpinbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_eeprom(*this, "eeprom")
		, m_watchdog(*this, "watchdog")
		, m_oki(*this, "oki")
		, m_essnd(*this, "essnd")
		, m_tilemapram(*this, "tilemapram")
		, m_d80010_ram(*this, "d80010")
		, m_d80060_ram(*this, "d80060")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_sprgen(*this, "spritegen")
	{ }

	void gcpinbal(machine_config &config);

private:
	enum
	{
		TIMER_GCPINBAL_INTERRUPT1
	};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<mb3773_device> m_watchdog;
	required_device<okim6295_device> m_oki;
	required_device<es8712_device> m_essnd;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_tilemapram;
	required_shared_ptr<uint16_t> m_d80010_ram;
	required_shared_ptr<uint16_t> m_d80060_ram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	emu_timer *m_int1_timer;

	/* video-related */
	tilemap_t     *m_tilemap[3];
	uint16_t      m_scrollx[3];
	uint16_t      m_scrolly[3];
	uint16_t      m_bg0_gfxset;
	uint16_t      m_bg1_gfxset;
#ifdef MAME_DEBUG
	uint8_t       m_dislayer[4];
#endif

	/* sound-related */
	uint32_t      m_msm_bank;

	DECLARE_WRITE16_MEMBER(d80010_w);
	DECLARE_WRITE8_MEMBER(d80040_w);
	DECLARE_WRITE16_MEMBER(d80060_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(es8712_reset_w);
	DECLARE_READ16_MEMBER(gcpinbal_tilemaps_word_r);
	DECLARE_WRITE16_MEMBER(gcpinbal_tilemaps_word_w);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_gcpinbal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(gcpinbal_interrupt);
	void gcpinbal_core_vh_start(  );
	DECLARE_WRITE_LINE_MEMBER(gcp_adpcm_int);
	required_device<excellent_spr_device> m_sprgen;

	void gcpinbal_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

#endif // MAME_INCLUDES_GCPINBAL_H
