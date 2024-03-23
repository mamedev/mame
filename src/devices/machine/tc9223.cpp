// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// TC9223P/TC9223F PLL-based frequency synthesizer

// Minimal implementation, logs the commands

#include "emu.h"
#include "tc9223.h"

DEFINE_DEVICE_TYPE(TC9223, tc9223_device, "tc9223", "TC9223P/F frequency synthesizer")

tc9223_device::tc9223_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC9223, tag, owner, clock)
{
}

void tc9223_device::device_start()
{
	save_item(NAME(m_shift));
	save_item(NAME(m_stb));
	save_item(NAME(m_clk));
	save_item(NAME(m_dat));
}

void tc9223_device::device_reset()
{
	m_shift = 0;
	m_stb = 0;
	m_clk = 0;
	m_dat = 0;
}

void tc9223_device::stb_w(int state)
{
	if(state == m_stb)
		return;
	m_stb = state;
	if(!m_stb)
		return;
	switch(m_shift >> 14) {
	case 0:
		logerror("gpio=%s lock=%s\n",
				 m_shift & 0x1000 ? "1" : "0",
				 m_shift & 0x2000 ? "unlocked" : "normal");
		break;
	case 1:
		logerror("a=%d n=%d\n", m_shift & 0x3f, (m_shift >> 6) & 0x7ff);
		break;
	case 2:
		logerror("divider %d\n", m_shift & 0x3fff);
		break;
	case 3:
		logerror("%s %s\n",
				 m_shift & 0x1000 ? "falling" : "rising",
				 m_shift & 0x2000 ? "non-inverting" : "inverting");
		break;
	}
}

void tc9223_device::clk_w(int state)
{
	if(state == m_clk)
		return;
	m_clk = state;
	if(!m_clk)
		return;

	m_shift = (m_shift >> 1) | (m_dat ? 0x8000 : 0);
}

void tc9223_device::dat_w(int state)
{
	m_dat = state;
}
