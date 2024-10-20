// license:BSD-3-Clause
// copyright-holders:Yochizo
/*************************************************************************

    Taito H system

*************************************************************************/
#ifndef MAME_TAITO_TAITO_H_H
#define MAME_TAITO_TAITO_H_H

#pragma once

#include "taitoio.h"
#include "tc0080vco.h"
#include "emupal.h"


class taitoh_state : public driver_device
{
public:
	taitoh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tc0080vco(*this, "tc0080vco"),
		m_tc0040ioc(*this, "tc0040ioc"),
		m_palette(*this, "palette"),
		m_z80bank(*this, "z80bank")
	{ }

	void recordbr(machine_config &config);
	void dleague(machine_config &config);
	void tetristh(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<tc0080vco_device> m_tc0080vco;
	optional_device<tc0040ioc_device> m_tc0040ioc;
	required_device<palette_device> m_palette;

	required_memory_bank m_z80bank;

	void taitoh_base(machine_config &config);

	void coin_control_w(u8 data);
	void taitoh_log_vram();

private:
	void sound_bankswitch_w(u8 data);
	u32 screen_update_recordbr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_dleague(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void recordbr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void dleague_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

	void dleague_map(address_map &map) ATTR_COLD;
	void recordbr_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void tetristh_map(address_map &map) ATTR_COLD;
};


class syvalion_state : public taitoh_state
{
public:
	syvalion_state(const machine_config &mconfig, device_type type, const char *tag) :
		taitoh_state(mconfig, type, tag),
		m_io_track(*this, { "P2Y", "P2X", "P1Y", "P1X" })
	{ }

	void syvalion(machine_config &config);

private:
	required_ioport_array<4> m_io_track;

	u8 syvalion_input_bypass_r();

	u32 screen_update_syvalion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void syvalion_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void syvalion_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_TAITO_H_H
