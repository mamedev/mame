// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************
    Casio GT913 I/O (HLE)

    TODO:
    - timer behavior is unverified (see comment in timer_control_w and timer_adjust)
    - various other unemulated registers

***************************************************************************/

#include "emu.h"
#include "gt913_io.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(GT913_IO_HLE, gt913_io_hle_device, "gt913_io_hle", "Casio GT913F I/O (HLE)")

gt913_io_hle_device::gt913_io_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GT913_IO_HLE, tag, owner, clock),
	m_cpu(*this, DEVICE_SELF_OWNER),
	m_cpu_io(nullptr), m_intc(nullptr), m_intc_tag(nullptr)
{
	m_timer_irq[0] = m_timer_irq[1] = 0;
}

void gt913_io_hle_device::device_start()
{
	m_cpu_io = &m_cpu->space(AS_IO);
	m_intc = siblingdevice<h8_intc_device>(m_intc_tag);

	m_timer[0] = timer_alloc(FUNC(gt913_io_hle_device::irq_timer_tick), this);
	m_timer[1] = timer_alloc(FUNC(gt913_io_hle_device::irq_timer_tick), this);

	save_item(NAME(m_timer_control));
	save_item(NAME(m_timer_rate));
	save_item(NAME(m_timer_irq_pending));
	save_item(NAME(m_adc_enable));
	save_item(NAME(m_adc_channel));
	save_item(NAME(m_adc_data));
}

void gt913_io_hle_device::device_reset()
{
	m_timer_control[0] = m_timer_control[1] = 0x00;
	m_timer_rate[0] = m_timer_rate[1] = 0;
	m_timer_irq_pending[0] = m_timer_irq_pending[1] = false;

	m_adc_enable = false;
	m_adc_channel = false;
	m_adc_data[0] = m_adc_data[1] = 0;
}

TIMER_CALLBACK_MEMBER(gt913_io_hle_device::irq_timer_tick)
{
	m_timer_irq_pending[param] = true;
	timer_check_irq((offs_t)param);
}

void gt913_io_hle_device::timer_control_w(offs_t offset, uint8_t data)
{
	assert(offset < 2);
	// TODO: ctk551 clears and sets bit 4 during the respective timer's IRQ, what should this do? pause/restart the timer?
	m_timer_control[offset] = data;
	timer_check_irq((int)offset);
}

uint8_t gt913_io_hle_device::timer_control_r(offs_t offset)
{
	assert(offset < 2);
	return m_timer_control[offset];
}

void gt913_io_hle_device::timer_rate0_w(uint16_t data)
{
	m_timer_rate[0] = data;
	timer_adjust(0);
}

void gt913_io_hle_device::timer_rate1_w(uint8_t data)
{
	m_timer_rate[1] = data;
	timer_adjust(1);
}

void gt913_io_hle_device::timer_adjust(offs_t num)
{
	assert(num < 2);

	/*
	On the CTK-551, this behavior provides the expected rate for timer 0, which is the MIDI PPQN timer.
	For timer 1, this is less certain, but it seems to provide an auto power off delay only a little
	longer than the "about six minutes" mentioned in the user manual.
	*/
	u64 clocks = m_timer_rate[num];
	if (!clocks)
	{
		m_timer[num]->adjust(attotime::never);
	}
	else
	{
		switch (m_timer_control[num] & 0x7)
		{
		default:
			logerror("unknown timer %u prescaler %u (pc = %04x)\n", num, m_timer_control[num] & 0x7, m_cpu->pc());
			break;
		case 0:
			break;
		case 2:
			clocks <<= 9;
			break;
		}

		attotime period = m_cpu->clocks_to_attotime(clocks);
		m_timer[num]->adjust(period, (int)num, period);
	}
}

void gt913_io_hle_device::timer_check_irq(offs_t num)
{
	assert(num < 2);

	if (BIT(m_timer_control[num], 3) && m_timer_irq_pending[num])
	{
		m_intc->internal_interrupt(m_timer_irq[num]);
		m_timer_irq_pending[num] = false;
	}
}

void gt913_io_hle_device::adc_control_w(uint8_t data)
{
	m_adc_enable = BIT(data, 2);
	m_adc_channel = BIT(data, 3);
	if (m_adc_enable && BIT(data, 0))
	{
		if (!m_adc_channel)
			m_adc_data[0] = m_cpu_io->read_word(h8_device::ADC_0);
		else
			m_adc_data[1] = m_cpu_io->read_word(h8_device::ADC_1);
	}
}

uint8_t gt913_io_hle_device::adc_control_r()
{
	return (m_adc_enable << 2) | (m_adc_channel << 3);
}

uint8_t gt913_io_hle_device::adc_data_r()
{
	if (!m_adc_channel)
		return m_adc_data[0];
	else
		return m_adc_data[1];
}
