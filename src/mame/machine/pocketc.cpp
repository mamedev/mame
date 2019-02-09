// license:GPL-2.0+
// copyright-holders:Peter Trauner

#include "emu.h"
#include "includes/pocketc.h"

WRITE8_MEMBER(pocketc_state::out_a_w)
{
	m_outa = data;
}

READ_LINE_MEMBER(pocketc_state::brk_r)
{
	return BIT(m_extra->read(), 0);
}

void pocketc_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_POWER_UP:
		m_power = 0;
		break;
	default:
		assert_always(false, "Unknown id in pocketc_state::device_timer");
	}
}

void pocketc_state::machine_start()
{
	m_cpu_nvram->set_base(m_maincpu->internal_ram(), 96);
	m_power_timer = timer_alloc(TIMER_POWER_UP);
}

void pocketc_state::machine_reset()
{
	m_power = 1;
	m_power_timer->adjust(attotime::from_seconds(1));
}
