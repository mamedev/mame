// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Fast Lane

*************************************************************************/
#ifndef MAME_INCLUDES_FASTLANE_H
#define MAME_INCLUDES_FASTLANE_H

#pragma once

#include "machine/timer.h"
#include "sound/k007232.h"
#include "video/k007121.h"
#include "video/k051733.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class fastlane_state : public driver_device
{
public:
	fastlane_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k007121_regs(*this, "k007121_regs"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k007121(*this, "k007121"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void fastlane(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_k007121_regs;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_videoram2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	/* devices */
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k007121_device> m_k007121;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void k007121_registers_w(offs_t offset, uint8_t data);
	void fastlane_bankswitch_w(uint8_t data);
	void fastlane_vram1_w(offs_t offset, uint8_t data);
	void fastlane_vram2_w(offs_t offset, uint8_t data);
	uint8_t fastlane_k1_k007232_r(offs_t offset);
	void fastlane_k1_k007232_w(offs_t offset, uint8_t data);
	uint8_t fastlane_k2_k007232_r(offs_t offset);
	void fastlane_k2_k007232_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	virtual void machine_start() override;
	virtual void video_start() override;
	void fastlane_palette(palette_device &palette) const;
	uint32_t screen_update_fastlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(fastlane_scanline);
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	void fastlane_map(address_map &map);
};

#endif // MAME_INCLUDES_FASTLANE_H
