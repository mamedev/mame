// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        SPG290 Timer

*****************************************************************************/

#include "emu.h"
#include "spg290_timer.h"


DEFINE_DEVICE_TYPE(SPG290_TIMER, spg290_timer_device, "spg290_timer", "SPG290 Timer")


spg290_timer_device::spg290_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPG290_TIMER, tag, owner, clock)
	, m_irq_cb(*this)
{
}

void spg290_timer_device::device_start()
{
	m_irq_cb.resolve_safe();
	m_tick_timer = timer_alloc(FUNC(spg290_timer_device::timer_update), this);

	save_item(NAME(m_enabled));
	save_item(NAME(m_control));
	save_item(NAME(m_control2));
	save_item(NAME(m_preload));
	save_item(NAME(m_counter));
	save_item(NAME(m_ccp));
	save_item(NAME(m_upcount));
}

void spg290_timer_device::device_reset()
{
	m_enabled = false;
	m_control = 0;
	m_control2 = 0;
	m_preload = 0;
	m_counter = 0;
	m_ccp = 0;
	m_upcount = 0;

	m_tick_timer->adjust(attotime::never);

	m_irq_cb(CLEAR_LINE);
}

void spg290_timer_device::device_clock_changed()
{
	if (m_enabled)
		m_tick_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	else
		m_tick_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(spg290_timer_device::timer_update)
{
	if (!BIT(m_control, 31))
		return;

	switch ((m_control2 >> 30) & 0x03)
	{
	case 0: // Timer Mode
		if (m_counter == 0xffff)
		{
			m_counter = m_preload & 0xffff;
			if (BIT(m_control, 27))
			{
				m_control |= 0x04000000;
				m_irq_cb(ASSERT_LINE);
			}
		}
		else
			m_counter++;
		break;
	case 1: // Capture Mode
		fatalerror("[%s] %s: unemulated timer Capture Mode\n", tag(), machine().describe_context());
		break;
	case 2: // Comparison Mode
		fatalerror("[%s] %s: unemulated timer Comparison Mode\n", tag(), machine().describe_context());
		break;
	case 3: // PWM Mode
		fatalerror("[%s] %s: unemulated timer PWM Mode\n", tag(), machine().describe_context());
		break;
	}
}

void spg290_timer_device::control_w(uint32_t data)
{
	bool enabled = data & 1;
	if (m_enabled != enabled)
	{
		m_enabled = enabled;
		spg290_timer_device::device_clock_changed();
	}

	if (data & 2)
		m_counter = m_preload & 0xffff;
}

uint32_t spg290_timer_device::read(offs_t offset, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x00:      // Control 1
		return m_control;
	case 0x04:      // Control 2
		return m_control2;
	case 0x08:      // Preload
		return m_preload;
	case 0x0c:      // CCP
		return m_ccp;
	case 0x10:      // Upcount
		return m_upcount;
	default:
		logerror("[%s] %s: unknown read %x\n", tag(), machine().describe_context(), offset << 2);
	}

	return 0;
}


void spg290_timer_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset << 2)
	{
	case 0x00:      // Control 1
		COMBINE_DATA(&m_control);

		if (ACCESSING_BITS_24_31)
		{
			m_control &= ~(data & 0x04000000);  // IRQ ack

			if (!(m_control & 0x04000000))
				m_irq_cb(CLEAR_LINE);
		}
		break;
	case 0x04:      // Control 2
		COMBINE_DATA(&m_control2);
		break;
	case 0x08:      // Preload
		COMBINE_DATA(&m_preload);
		break;
	case 0x0c:      // CCP
		COMBINE_DATA(&m_ccp);
		break;
	case 0x10:      // Upcount
		COMBINE_DATA(&m_upcount);
		break;
	default:
		logerror("[%s] %s: unknown write %x = %x\n", tag(), machine().describe_context(), offset << 2, data);
	}
}
