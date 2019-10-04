// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas

#ifndef MAME_INCLUDES_BWIDOW_H
#define MAME_INCLUDES_BWIDOW_H

#include "machine/er2055.h"

#define MASTER_CLOCK (XTAL(12'096'000))
#define CLOCK_3KHZ   (MASTER_CLOCK / 4096)


class bwidow_state : public driver_device
{
public:
	bwidow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_in3(*this, "IN3")
		, m_in4(*this, "IN4")
		, m_dsw2(*this, "DSW2")
		, m_leds(*this, "led%u", 0U)
	{ }

	void spacduel(machine_config &config);
	void gravitar(machine_config &config);
	void bwidowp(machine_config &config);
	void bwidow(machine_config &config);
	void lunarbat(machine_config &config);
	void bwidow_audio(machine_config &config);
	void gravitar_audio(machine_config &config);

	DECLARE_READ_LINE_MEMBER(clock_r);

protected:
	DECLARE_READ8_MEMBER(spacduel_IN3_r);
	DECLARE_READ8_MEMBER(bwidowp_in_r);
	DECLARE_WRITE8_MEMBER(bwidow_misc_w);
	DECLARE_WRITE8_MEMBER(spacduel_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(earom_read);
	DECLARE_WRITE8_MEMBER(earom_write);
	DECLARE_WRITE8_MEMBER(earom_control_w);

	void bwidow_map(address_map &map);
	void bwidowp_map(address_map &map);
	void spacduel_map(address_map &map);

	virtual void machine_start() override { m_leds.resolve(); }
	virtual void machine_reset() override;

	int m_lastdata;
	required_device<cpu_device> m_maincpu;
	required_device<er2055_device> m_earom;
	optional_ioport m_in3;
	optional_ioport m_in4;
	optional_ioport m_dsw2;
	output_finder<2> m_leds;
};

#endif // MAME_INCLUDES_BWIDOW_H
