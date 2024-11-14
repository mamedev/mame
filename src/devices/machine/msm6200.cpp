// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    OKI MSM6200 keyboard controller (HLE)
***************************************************************************/

#include "emu.h"
#include "msm6200.h"

DEFINE_DEVICE_TYPE(MSM6200, msm6200_device, "msm6200", "OKI MSM6200 keyboard controller")

/**************************************************************************/
msm6200_device::msm6200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MSM6200, tag, owner, clock),
	m_keys(*this, "KI%u", 1u),
	m_velocity(*this, "VELOCITY"),
	m_irq_cb(*this)
{
}

/**************************************************************************/
void msm6200_device::device_start()
{
	m_cmd = 0xf;

	save_item(NAME(m_cmd));
	save_item(NAME(m_row));
	save_item(NAME(m_key_data));
	save_item(NAME(m_key_state));
	save_item(NAME(m_last_state));
}

/**************************************************************************/
void msm6200_device::device_reset()
{
	m_row = 0;
	m_key_data = 0;
	m_key_state = 0;
	std::fill(std::begin(m_last_state), std::end(m_last_state), 0);
}

/**************************************************************************/
void msm6200_device::write(offs_t offset, u8 data)
{
	// 8-bit multiplexed address/data bus, upper 4 bits are output only
	// on write, the lower 4 bits of the address are latched and the data is ignored
	m_cmd = offset & 0xf;
	m_key_data = 0xff;

	switch (m_cmd)
	{
	case 0: // read key number
		for (int i = 0; i < 2; i++)
		{
			if (BIT(m_key_state ^ m_last_state[m_row], i))
			{
				m_last_state[m_row] ^= (1 << i);
				m_key_data = (BIT(m_key_state, i) << 7) | ((m_row + 1) << 1) | i;
				break;
			}
		}
		if (m_key_state == m_last_state[m_row])
			m_irq_cb(0);
		break;

	case 1: // read velocity
		m_key_data = m_velocity.read_safe(0x3f);
		break;

	case 2: // next row?
		(++m_row) %= m_keys.size();
		// TODO: what should this one actually be?
		// the cz1/ht6000 key MCU code outputs the result to port 1 for debugging
		m_key_data = m_row;
		break;

	case 7: // capture current row?
		m_key_state = m_keys[m_row].read_safe(0);
		if (m_key_state != m_last_state[m_row])
			m_irq_cb(1);
		break;

	case 8: // init all rows
		for (int i = 0; i < m_keys.size(); i++)
			m_last_state[i] = m_keys[i].read_safe(0);
		m_irq_cb(0);
		break;

	default:
		logerror("%s: unknown cmd 0x%x\n", machine().describe_context(), m_cmd);
		break;
	}
}

/**************************************************************************/
u8 msm6200_device::read()
{
	return m_key_data;
}
