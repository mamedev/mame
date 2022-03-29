// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Bryan McPhail
/***************************************************************************

    Midway MCR-68k system

***************************************************************************/
#ifndef MAME_INCLUDES_MCR68_H
#define MAME_INCLUDES_MCR68_H

#pragma once

#include "machine/timer.h"
#include "machine/watchdog.h"
#include "audio/midway.h"
#include "audio/s11c_bg.h"
#include "machine/6840ptm.h"
#include "machine/adc0844.h"
#include "screen.h"
#include "tilemap.h"

class mcr68_state : public driver_device
{
public:
	mcr68_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_sounds_good(*this, "sg"),
		m_turbo_cheap_squeak(*this, "tcs"),
		m_bg(*this, "bg"),
		m_adc(*this, "adc"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_ptm(*this, "ptm")
	{ }

	void mcr68(machine_config &config);
	void intlaser(machine_config &config);
	void xenophob(machine_config &config);
	void spyhunt2(machine_config &config);
	void trisport(machine_config &config);
	void pigskin(machine_config &config);
	void archrivl(machine_config &config);

	void init_intlaser();
	void init_pigskin();
	void init_blasted();
	void init_trisport();
	void init_xenophob();
	void init_archrivl();
	void init_spyhunt2();
	void init_archrivlb();

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	optional_device<midway_sounds_good_device> m_sounds_good;
	optional_device<midway_turbo_cheap_squeak_device> m_turbo_cheap_squeak;
	optional_device<s11c_bg_device> m_bg;
	optional_device<adc0844_device> m_adc;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<ptm6840_device> m_ptm;

	uint16_t m_control_word = 0;
	uint8_t m_protection_data[5];
	attotime m_timing_factor;
	uint8_t m_sprite_clip = 0;
	int8_t m_sprite_xoffset = 0;
	timer_expired_delegate m_v493_callback;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	void xenophobe_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blasted_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spyhunt2_port_0_r();
	uint16_t spyhunt2_port_1_r();
	void spyhunt2_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t archrivl_port_1_r();
	void archrivl_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pigskin_protection_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t pigskin_protection_r();
	uint16_t pigskin_port_1_r();
	uint16_t pigskin_port_2_r();
	uint16_t trisport_port_1_r();
	void mcr68_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t archrivlb_port_1_r();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update_mcr68(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
	TIMER_CALLBACK_MEMBER(mcr68_493_off_callback);
	TIMER_CALLBACK_MEMBER(mcr68_493_callback);
	void mcr68_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void mcr68_common_init(int clip, int xoffset);
	std::unique_ptr<uint8_t[]> m_srcdata0;
	std::unique_ptr<uint8_t[]> m_srcdata2;

	void mcr68_map(address_map &map);
	void pigskin_map(address_map &map);
	void trisport_map(address_map &map);
};

#endif // MAME_INCLUDES_MCR68_H
