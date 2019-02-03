// license:BSD-3-Clause
// copyright-holders:Yochizo
/*************************************************************************

    Taito H system

*************************************************************************/
#ifndef MAME_INCLUDES_TAITO_H_H
#define MAME_INCLUDES_TAITO_H_H

#pragma once

#include "machine/taitoio.h"
#include "video/tc0080vco.h"
#include "emupal.h"


class taitoh_state : public driver_device
{
public:
	taitoh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_m68000_mainram(*this, "m68000_mainram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_tc0080vco(*this, "tc0080vco"),
		m_tc0040ioc(*this, "tc0040ioc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void recordbr(machine_config &config);
	void syvalion(machine_config &config);
	void dleague(machine_config &config);
	void tetristh(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_m68000_mainram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<tc0080vco_device> m_tc0080vco;
	optional_device<tc0040ioc_device> m_tc0040ioc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(coin_control_w);
	DECLARE_READ8_MEMBER(syvalion_input_bypass_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	uint32_t screen_update_syvalion(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_recordbr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dleague(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void syvalion_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void recordbr_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void dleague_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void taitoh_log_vram();
	void dleague_map(address_map &map);
	void recordbr_map(address_map &map);
	void sound_map(address_map &map);
	void syvalion_map(address_map &map);
	void tetristh_map(address_map &map);
};

#endif // MAME_INCLUDES_TAITO_H_H
