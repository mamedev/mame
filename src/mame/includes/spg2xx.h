// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood
#pragma once

#ifndef MAME_INCLUDES_SPG2XX_H
#define MAME_INCLUDES_SPG2XX_H

#include "emu.h"

#include "cpu/unsp/unsp.h"
#include "machine/i2cmem.h"
#include "machine/spg2xx.h"


#include "screen.h"
#include "softlist.h"
#include "speaker.h"



class spg2xx_game_state : public driver_device
{
public:
	spg2xx_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_bank(*this, "cartbank"),
		m_io_p1(*this, "P1"),
		m_io_p2(*this, "P2"),
		m_io_p3(*this, "P3"),
		m_i2cmem(*this, "i2cmem")
	{ }

	void spg2xx_base(machine_config &config);
	void rad_skat(machine_config &config);
	void rad_skatp(machine_config &config);
	void rad_sktv(machine_config &config);
	void rad_crik(machine_config &config);
	void non_spg_base(machine_config &config);
	void abltenni(machine_config &config);

	void init_crc();
	void init_wiwi18();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void switch_bank(uint32_t bank);

	DECLARE_WRITE8_MEMBER(i2c_w);
	DECLARE_READ8_MEMBER(i2c_r);

	virtual DECLARE_WRITE16_MEMBER(porta_w);
	virtual DECLARE_WRITE16_MEMBER(portb_w);
	virtual DECLARE_WRITE16_MEMBER(portc_w);

	DECLARE_READ16_MEMBER(rad_porta_r);
	DECLARE_READ16_MEMBER(rad_portb_r);
	DECLARE_READ16_MEMBER(rad_portc_r);


	required_device<spg2xx_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_memory_bank m_bank;


	virtual void mem_map_4m(address_map &map);
	virtual void mem_map_2m(address_map &map);
	virtual void mem_map_1m(address_map &map);

	uint32_t m_current_bank;

	required_ioport m_io_p1;
	optional_ioport m_io_p2;
	optional_ioport m_io_p3;
	optional_device<i2cmem_device> m_i2cmem;
};




#endif // MAME_INCLUDES_SPG2XX_H
