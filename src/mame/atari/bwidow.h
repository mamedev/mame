// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Bernd Wiebelt, Allard van der Bas

#ifndef MAME_ATARI_BWIDOW_H
#define MAME_ATARI_BWIDOW_H

#include "video/avgdvg.h"
#include "machine/er2055.h"

#define MASTER_CLOCK (XTAL(12'096'000))
#define CLOCK_3KHZ   (MASTER_CLOCK / 4096)


class bwidow_state : public driver_device
{
public:
	bwidow_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_avg(*this, "avg")
		, m_earom(*this, "earom")
		, m_cabinet(*this, "CABINET")
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

	int clock_r();

protected:
	uint8_t spacduel_IN3_r(offs_t offset);
	uint8_t bwidowp_in_r();
	void bwidow_misc_w(uint8_t data);
	void spacduel_coin_counter_w(uint8_t data);
	void irq_ack_w(uint8_t data);
	uint8_t earom_read();
	void earom_write(offs_t offset, uint8_t data);
	void earom_control_w(uint8_t data);

	void bwidow_map(address_map &map) ATTR_COLD;
	void bwidowp_map(address_map &map) ATTR_COLD;
	void spacduel_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override { m_leds.resolve(); }
	virtual void machine_reset() override ATTR_COLD;

	int m_lastdata = 0;
	required_device<cpu_device> m_maincpu;
	required_device<avg_device> m_avg;
	required_device<er2055_device> m_earom;
	optional_ioport m_cabinet;
	optional_ioport m_in3;
	optional_ioport m_in4;
	optional_ioport m_dsw2;
	output_finder<2> m_leds;
};

#endif // MAME_ATARI_BWIDOW_H
