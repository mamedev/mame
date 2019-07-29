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
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen"),
		m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_mcu_p3(0xff)
	{ }

	void bionicc(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<i8751_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<buffered_spriteram16_device> m_spriteram;

	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void mcu_io(address_map &map);

	// video
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	static rgb_t RRRRGGGGBBBBIIII(uint32_t raw);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	DECLARE_WRITE16_MEMBER(bgvideoram_w);
	DECLARE_WRITE16_MEMBER(fgvideoram_w);
	DECLARE_WRITE16_MEMBER(txvideoram_w);
	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_WRITE16_MEMBER(gfxctrl_w);

	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint16_t    m_scroll[4];

	// audio
	void audiocpu_nmi_w(u8 data);

	// protection mcu
	u8 m_audiocpu_to_mcu; // ls374 at 4a
	u8 m_mcu_to_audiocpu; // ls374 at 5a
	u8 m_mcu_p1;
	u8 m_mcu_p3;

	void dmaon_w(u16 data);
	u8 mcu_dma_r(offs_t offset);
	void mcu_dma_w(offs_t offset, u8 data);
	void mcu_p3_w(u8 data);
};

#endif // MAME_INCLUDES_BIONICC_H
