// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
/*************************************************************************

    Atari Tetris hardware

*************************************************************************/
#ifndef MAME_INCLUDES_ATETRIS_H
#define MAME_INCLUDES_ATETRIS_H

#pragma once

#include "includes/slapstic.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/gen_latch.h"
#include "sound/sn76496.h"
#include "screen.h"
#include "tilemap.h"

class atetris_state : public driver_device
{
public:
	atetris_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_slapstic(*this, "slapstic"),
		m_videoram(*this, "videoram")
	{
	}

	void atetris_base(machine_config &config);
	void atetris(machine_config &config);
	void atetrisb2(machine_config &config);

	void init_atetris();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<atari_slapstic_device> m_slapstic;

	required_shared_ptr<uint8_t> m_videoram;

	uint8_t *m_slapstic_source;
	uint8_t *m_slapstic_base;
	uint8_t m_current_bank;
	emu_timer *m_interrupt_timer;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(slapstic_r);
	DECLARE_WRITE8_MEMBER(coincount_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_gen);
	void reset_bank();

	void atetrisb2_map(address_map &map);
	void main_map(address_map &map);
};

class atetris_mcu_state : public atetris_state
{
public:
	atetris_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		atetris_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_soundlatch(*this, "soundlatch%u", 1U),
		m_sn(*this, "sn%u", 1U)
	{
	}

	void atetrisb3(machine_config &config);

private:
	DECLARE_READ8_MEMBER(mcu_bus_r);
	DECLARE_WRITE8_MEMBER(mcu_p2_w);
	DECLARE_WRITE8_MEMBER(mcu_reg_w);

	void atetrisb3_map(address_map &map);

	required_device<i8749_device> m_mcu;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device_array<sn76496_base_device, 4> m_sn;
};

#endif // MAME_INCLUDES_ATETRIS_H
