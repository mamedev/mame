// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/kramermc.h
 *
 ****************************************************************************/
#ifndef MAME_INCLUDES_KRAMERMC_H
#define MAME_INCLUDES_KRAMERMC_H

#pragma once

#include "machine/z80pio.h"
#include "emupal.h"

class kramermc_state : public driver_device
{
public:
	kramermc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void kramermc(machine_config &config);

	void init_kramermc();

private:
	uint8_t m_key_row;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(kramermc_port_a_r);
	DECLARE_READ8_MEMBER(kramermc_port_b_r);
	DECLARE_WRITE8_MEMBER(kramermc_port_a_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void kramermc_io(address_map &map);
	void kramermc_mem(address_map &map);
};

/*----------- defined in video/kramermc.c -----------*/

extern const gfx_layout kramermc_charlayout;


#endif // MAME_INCLUDES_KRAMERMC_H
