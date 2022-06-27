// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_INCLUDES_TAOTAIDO_H
#define MAME_INCLUDES_TAOTAIDO_H

#pragma once

#include "vsystem_spr.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "tilemap.h"

class taotaido_state : public driver_device
{
public:
	taotaido_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_scrollram(*this, "scrollram"),
		m_bgram(*this, "bgram"),
		m_soundbank(*this, "soundbank"),
		m_spritebank(*this, "spritebank", 0x08, ENDIANNESS_BIG)
	{ }

	void taotaido(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<mb3773_device> m_watchdog;

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_spriteram2;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_bgram;

	required_memory_bank m_soundbank;

	memory_share_creator<uint8_t> m_spritebank;

	uint8_t m_bgbank[8]{};
	tilemap_t *m_bg_tilemap = nullptr;
	std::unique_ptr<uint16_t[]> m_spriteram_old;
	std::unique_ptr<uint16_t[]> m_spriteram_older;
	std::unique_ptr<uint16_t[]> m_spriteram2_old;
	std::unique_ptr<uint16_t[]> m_spriteram2_older;

	uint16_t pending_command_r();
	void unknown_output_w(uint8_t data);
	void sh_bankswitch_w(uint8_t data);
	void spritebank_w(offs_t offset, uint8_t data);
	void tileregs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bgvideoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(bg_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	uint32_t tile_callback( uint32_t code );
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
};

#endif // MAME_INCLUDES_TAOTAIDO_H
