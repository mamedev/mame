// license:GPL-2.0+
// copyright-holders:Peter Trauner

#include "emu.h"
#include "pocketc.h"

void pocketc_state::out_a_w(uint8_t data)
{
	m_outa = data;
}

READ_LINE_MEMBER(pocketc_state::brk_r)
{
	return BIT(m_extra->read(), 0);
}

TIMER_CALLBACK_MEMBER(pocketc_state::power_up_done)
{
	m_power = 0;
}

void pocketc_state::machine_start()
{
	m_cpu_nvram->set_base(m_maincpu->internal_ram(), 96);
	m_power_timer = timer_alloc(FUNC(pocketc_state::power_up_done), this);
}

void pocketc_state::machine_reset()
{
	m_power = 1;
	m_power_timer->adjust(attotime::from_seconds(1));
}
