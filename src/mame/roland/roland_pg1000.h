// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

#ifndef MAME_ROLAND_ROLAND_PG1000_H
#define MAME_ROLAND_ROLAND_PG1000_H

#include "cpu/upd7810/upd7810.h"
#include "video/hd44780.h"
#include "bus/midi/midi.h"
#include "emupal.h"
#include "screen.h"


class pg1000_state : public driver_device
{
public:
	pg1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_row(*this, "ROW%u", 0U)
		, m_led(*this, "LED%u", 0U)
		, m_top_slider(*this, "top_slider_%u", 0U)
		, m_middle_slider(*this, "middle_slider_%u", 0U)
		, m_bottom_slider(*this, "bottom_slider_%u", 0U)
		, m_mdout(*this, "mdout")
		, m_scan(0)
		, m_an_select(0)
		, m_mdin_bit(false)
		, m_paramin_bit(false)
		, m_midi_in_enable(false)
		, m_param_in_enable(false)
	{
	}

	void pg1000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 sw_r();
	void led_w(u8 data);
	void mem_map(address_map &map) ATTR_COLD;
	void palette_init(palette_device &palette);

	required_device<upd7810_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_ioport_array<2> m_row;
	output_finder<6> m_led;
	required_ioport_array<13> m_top_slider;
	required_ioport_array<23> m_middle_slider;
	required_ioport_array<20> m_bottom_slider;
	optional_device<midi_port_device> m_mdout;

	u8 m_scan;
	u8 m_an_select;
	bool m_mdin_bit;
	bool m_paramin_bit;
	bool m_midi_in_enable;
	bool m_param_in_enable;
};

#endif // MAME_ROLAND_ROLAND_PG1000_H
