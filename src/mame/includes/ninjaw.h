// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/
#ifndef MAME_INCLUDES_NINJAW_H
#define MAME_INCLUDES_NINJAW_H

#pragma once

#include "audio/taitosnd.h"
#include "machine/taitoio.h"
#include "sound/flt_vol.h"
#include "video/tc0100scn.h"
#include "video/tc0110pcr.h"
#include "emupal.h"


class ninjaw_state : public driver_device
{
public:
	ninjaw_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0100scn(*this, "tc0100scn_%u", 1),
		m_tc0110pcr(*this, "tc0110pcr_%u", 1),
		m_2610_l(*this, "2610.%u.l", 1),
		m_2610_r(*this, "2610.%u.r", 1),
		m_gfxdecode(*this, "gfxdecode_%u", 1),
		m_spriteram(*this, "spriteram"),
		m_z80bank(*this, "z80bank")
	{ }

	void darius2(machine_config &config);
	void ninjaw(machine_config &config);

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device_array<tc0100scn_device, 3> m_tc0100scn;
	required_device_array<tc0110pcr_device, 3> m_tc0110pcr;
	required_device_array<filter_volume_device, 2> m_2610_l;
	required_device_array<filter_volume_device, 2> m_2610_r;
	required_device_array<gfxdecode_device, 3> m_gfxdecode;

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* misc */
	uint16_t     m_cpua_ctrl;
	int        m_pandata[4];

	void coin_control_w(u8 data);
	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	void sound_bankswitch_w(u8 data);
	void pancontrol_w(offs_t offset, u8 data);
	DECLARE_WRITE16_MEMBER(tc0100scn_triple_screen_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void postload();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int primask, int x_offs, int y_offs, int chip);
	void parse_control();
	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip);
	void darius2_master_map(address_map &map);
	void darius2_slave_map(address_map &map);
	void ninjaw_master_map(address_map &map);
	void ninjaw_slave_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_NINJAW_H
