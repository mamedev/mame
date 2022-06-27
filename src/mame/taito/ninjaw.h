// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Taito Triple Screen Games

*************************************************************************/
#ifndef MAME_INCLUDES_NINJAW_H
#define MAME_INCLUDES_NINJAW_H

#pragma once

#include "taitosnd.h"
#include "taitoio.h"
#include "sound/flt_vol.h"
#include "tc0100scn.h"
#include "tc0110pcr.h"
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

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

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
	required_shared_ptr<u16> m_spriteram;

	/* memory regions */
	required_memory_bank m_z80bank;

	/* misc */
	u16     m_cpua_ctrl = 0;
	int        m_pandata[4]{};

	void coin_control_w(u8 data);
	void cpua_ctrl_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void pancontrol_w(offs_t offset, u8 data);
	void tc0100scn_triple_screen_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs, int chip);
	void parse_control();
	u32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs, int chip);
	void darius2_master_map(address_map &map);
	void darius2_slave_map(address_map &map);
	void ninjaw_master_map(address_map &map);
	void ninjaw_slave_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_NINJAW_H
