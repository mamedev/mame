// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Paul Leaman
// thanks-to: Steven Frew (the author of Slutte)
/***************************************************************************

    Bionic Commando

***************************************************************************/
#ifndef MAME_INCLUDES_BIONICC_H
#define MAME_INCLUDES_BIONICC_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "video/bufsprite.h"
#include "video/tigeroad_spr.h"
#include "emupal.h"

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_ram16(*this, "ram16"),
		m_old_p3(0xff)
	{ }

	void bionicc(machine_config &config);

private:
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void mcu_io(address_map &map);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	static rgb_t RRRRGGGGBBBBIIII(uint32_t raw);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	/* handlers */
	DECLARE_WRITE16_MEMBER(mpu_trigger_w);
	DECLARE_WRITE16_MEMBER(bgvideoram_w);
	DECLARE_WRITE16_MEMBER(fgvideoram_w);
	DECLARE_WRITE16_MEMBER(txvideoram_w);
	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_WRITE16_MEMBER(gfxctrl_w);

	DECLARE_READ8_MEMBER(mcu_shared_r);
	DECLARE_WRITE8_MEMBER(mcu_shared_w);

	DECLARE_WRITE8_MEMBER(out1_w);
	DECLARE_WRITE8_MEMBER(out3_w);

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint16_t    m_scroll[4];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<i8751_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	/* memory */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;
	required_shared_ptr<uint16_t> m_ram16;

	/* misc */
	int m_old_p3;
};

#endif // MAME_INCLUDES_BIONICC_H
