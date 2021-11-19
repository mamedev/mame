// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/
#ifndef MAME_INCLUDES_FLKATCK_H
#define MAME_INCLUDES_FLKATCK_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "machine/k007452.h"
#include "sound/k007232.h"
#include "video/k007121.h"
#include "tilemap.h"

class flkatck_state : public driver_device
{
public:
	flkatck_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121(*this, "k007121"),
		m_k007232(*this, "k007232"),
		m_k007452(*this, "k007452"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void flkatck(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_k007121_tilemap[2];
	int        m_flipscreen;

	/* misc */
	int        m_irq_enabled;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121;
	required_device<k007232_device> m_k007232;
	required_device<k007452_device> m_k007452;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	void flkatck_bankswitch_w(uint8_t data);
	uint8_t flkatck_ls138_r(offs_t offset);
	void flkatck_ls138_w(offs_t offset, uint8_t data);
	void vram_w(offs_t offset, uint8_t data);
	void flkatck_k007121_regs_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info_A);
	TILE_GET_INFO_MEMBER(get_tile_info_B);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_flkatck(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(flkatck_interrupt);
	void volume_callback(uint8_t data);
	void flkatck_map(address_map &map);
	void flkatck_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_FLKATCK_H
