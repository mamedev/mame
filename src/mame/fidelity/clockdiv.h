// license:BSD-3-Clause
// copyright-holders:hap
/*

  Fidelity Electronics 6502 dynamic CPU clock divider

*/

#ifndef MAME_FIDELITY_FIDEL_CLOCKDIV_H
#define MAME_FIDELITY_FIDEL_CLOCKDIV_H

#pragma once


class fidel_clockdiv_state : public driver_device
{
public:
	fidel_clockdiv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(div_changed) { div_refresh(newval); }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { div_refresh(); }
	virtual void device_post_load() override { div_refresh(); }

	// devices/pointers
	required_device<cpu_device> m_maincpu;

	// dynamic cpu divider
	void div_refresh(ioport_value val = 0xff);

private:
	memory_passthrough_handler m_read_tap;
	memory_passthrough_handler m_write_tap;

	u16 m_div_status = 0;
	double m_div_scale = 0;
	emu_timer *m_div_timer = nullptr;

	inline void div_prep_cpu_freq(offs_t offset);
	TIMER_CALLBACK_MEMBER(div_set_cpu_freq);
};


INPUT_PORTS_EXTERN( fidel_clockdiv_2 );
INPUT_PORTS_EXTERN( fidel_clockdiv_4 );

#endif // MAME_FIDELITY_FIDEL_CLOCKDIV_H
