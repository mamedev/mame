// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
#include "emu.h"
#include "includes/tigeroad.h"

READ16_MEMBER(pushman_state::mcu_comm_r)
{
	switch (offset & 0x03)
	{
	case 0: // read and acknowledge MCU reply
		if (!machine().side_effects_disabled())
			m_mcu_semaphore = false;
		return m_mcu_latch;
	case 2: // expects bit 0 to be high when MCU has accepted command (other bits ignored)
		return m_host_semaphore ? 0xfffe : 0xffff;
	case 3: // expects bit 0 to be low when MCU has sent response (other bits ignored)
		return m_mcu_semaphore ? 0xfffe : 0xffff;
	}
	logerror("unknown MCU read offset %X & %04X\n", offset, mem_mask);
	return 0xffff;
}

WRITE16_MEMBER(pushman_state::pushman_mcu_comm_w)
{
	switch (offset & 0x01)
	{
	case 0:
		m_host_latch = swapendian_int16(data);
		break;
	case 1:
		m_mcu->pd_w(space, 0, data & 0x00ff);
		m_host_semaphore = true;
		m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
		break;
	}
}

WRITE16_MEMBER(pushman_state::bballs_mcu_comm_w)
{
	m_host_latch = data;
	m_host_semaphore = true;
	m_mcu->set_input_line(M68705_IRQ_LINE, ASSERT_LINE);
}

WRITE8_MEMBER(pushman_state::mcu_pa_w)
{
	m_mcu_output = (m_mcu_output & 0xff00) | (u16(data) & 0x00ff);
}

WRITE8_MEMBER(pushman_state::mcu_pb_w)
{
	m_mcu_output = (m_mcu_output & 0x00ff) | (u16(data) << 8);
}

WRITE8_MEMBER(pushman_state::mcu_pc_w)
{
	if (BIT(data, 0))
	{
		m_mcu->pa_w(space, 0, 0xff);
		m_mcu->pb_w(space, 0, 0xff);
	}
	else
	{
		m_host_semaphore = false;
		m_mcu->set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
		m_mcu->pa_w(space, 0, (m_host_latch >> 0) & 0x00ff);
		m_mcu->pb_w(space, 0, (m_host_latch >> 8) & 0x00ff);
	}

	if (BIT(m_mcu_latch_ctl, 1) && !BIT(data, 1))
	{
		m_mcu_latch = m_mcu_output & (BIT(m_mcu_latch_ctl, 0) ? 0xffff : m_host_latch);
		m_mcu_semaphore = true;
	}

	m_mcu_latch_ctl = data;
}
