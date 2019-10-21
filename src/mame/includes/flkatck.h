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
	int        m_multiply_reg[2];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121;
	required_device<k007232_device> m_k007232;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(flkatck_bankswitch_w);
	DECLARE_READ8_MEMBER(flkatck_ls138_r);
	DECLARE_WRITE8_MEMBER(flkatck_ls138_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(flkatck_k007121_regs_w);
	TILE_GET_INFO_MEMBER(get_tile_info_A);
	TILE_GET_INFO_MEMBER(get_tile_info_B);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_flkatck(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(flkatck_interrupt);
	DECLARE_WRITE8_MEMBER(volume_callback);
	void flkatck_map(address_map &map);
	void flkatck_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_FLKATCK_H
