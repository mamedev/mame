// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/kramermc.h
 *
 ****************************************************************************/

#ifndef KRAMERMC_H_
#define KRAMERMC_H_

#include "machine/z80pio.h"

class kramermc_state : public driver_device
{
public:
	kramermc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	UINT8 m_key_row;
	DECLARE_DRIVER_INIT(kramermc);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_kramermc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_READ8_MEMBER(kramermc_port_a_r);
	DECLARE_READ8_MEMBER(kramermc_port_b_r);
	DECLARE_WRITE8_MEMBER(kramermc_port_a_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

/*----------- defined in video/kramermc.c -----------*/

extern const gfx_layout kramermc_charlayout;


#endif /* KRAMERMC_h_ */
