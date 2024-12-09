// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Interrupt Controller/EI2OS

***************************************************************************/

#include "emu.h"
#include "f2mc16.h"
#include "f2mc16_intc.h"

namespace {

struct ICR { enum : uint16_t
{
	IL = 7 << 0,
	ISE = 1 << 3,
	S = 3 << 4,
	S_COUNT_COMPLETION = 1 << 4,
	S_COMPLETION_REQUEST = 3 << 4,
	ICS = 15 << 4,
}; };

struct ISCS { enum : uint8_t
{
	RESERVED = 7 << 5,
	IF = 1 << 4,
	BW = 1 << 3,
	BF = 1 << 2,
	DIR = 1 << 1,
	SE = 1 << 0
}; };

struct DIRR { enum : uint16_t
{
	R0 = 1 << 0
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_INTC, f2mc16_intc_device, "f2mc16_intc", "F2MC16 INTC")

f2mc16_intc_device::f2mc16_intc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_INTC, tag, owner, clock),
	m_cpu(nullptr),
	m_i2osclr_cb(*this),
	m_digm_vector(0x2a),
	m_irq_state(0),
	m_completion_request_state(0),
	m_active_irq(0)
{
}

void f2mc16_intc_device::device_start()
{
	m_cpu = downcast<f2mc16_device *>(owner());
	std::fill_n(m_icr, std::size(m_icr), ICR::IL);
	std::fill_n(m_ics, std::size(m_ics), 0);
	save_item(NAME(m_icr));
	save_item(NAME(m_dirr));
	save_item(NAME(m_enir));
	save_item(NAME(m_eirr));
	save_item(NAME(m_elvr));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_completion_request_state));
	save_item(NAME(m_active_irq));
}

void f2mc16_intc_device::device_reset()
{
	std::fill_n(m_icr, std::size(m_icr), ICR::IL);
	m_enir = 0;
	m_eirr = 0;
	m_elvr = 0;
	m_dirr = 0;
}

uint8_t f2mc16_intc_device::icr_r(offs_t offset)
{
	return m_icr[offset];
}

void f2mc16_intc_device::icr_w(offs_t offset, uint8_t data)
{
	m_ics[offset] = (data & ICR::ICS) >> 4;
	m_icr[offset] = (m_icr[offset] & ICR::ICS) | (data & ~ICR::ICS);
	update();
}

uint8_t f2mc16_intc_device::enir_r()
{
	return m_enir;
}

void f2mc16_intc_device::enir_w(uint8_t data)
{
	m_enir = data;
}

uint8_t f2mc16_intc_device::eirr_r()
{
	if (m_cpu->rmw())
		return 0xff;
	return m_eirr;
}

void f2mc16_intc_device::eirr_w(uint8_t data)
{
	m_eirr &= data;
}

uint8_t f2mc16_intc_device::elvr_r(offs_t offset)
{
	if (offset == 0)
		return m_elvr >> 0;
	else
		return m_elvr >> 8;
}

void f2mc16_intc_device::elvr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_elvr);
}

uint8_t f2mc16_intc_device::dirr_r()
{
	return m_dirr;
}

void f2mc16_intc_device::dirr_w(uint8_t data)
{
	data &= DIRR::R0;

	if (m_dirr != data)
	{
		m_dirr = data;

		set_irq(m_digm_vector, m_dirr & DIRR::R0);
	}
}

void f2mc16_intc_device::set_irq(uint8_t vector, int state)
{
	int irq = vector - m_external_interrupt_vector;

	if (BIT(m_irq_state, irq) != state)
	{
		if (state)
			m_irq_state |= 1 << irq;
		else
			m_irq_state &= ~(1 << irq);

		update();
	}
}

void f2mc16_intc_device::set_completion_request(uint8_t vector, int state)
{
	int irq = vector - m_external_interrupt_vector;

	if (state)
		m_completion_request_state |= 1 << irq;
	else
		m_completion_request_state &= ~(1 << irq);
}

void f2mc16_intc_device::update()
{
	int level = 7;

	if (m_irq_state)
		for (int i = 0; i < 32; i++)
		{
			if (BIT(m_irq_state, i))
			{
				int il = m_icr[i / 2] & ICR::IL;
				if (level > il)
				{
					m_active_irq = i;
					level = il;
				}
			}
		}

	m_cpu->set_irq_level(level);
}

IRQ_CALLBACK_MEMBER(f2mc16_intc_device::irq_acknowledge_callback)
{
	int irq = m_active_irq;
	int icr = irq / 2;

	if (m_icr[icr] & ICR::ISE)
	{
		offs_t isd = 0x100 + (m_ics[icr] * 8);
		address_space &space = m_cpu->space(AS_PROGRAM);
		uint8_t iscs = space.read_byte(isd + 3); m_cpu->adjust_icount(-1);

		m_icr[icr] &= ~ICR::S;

		if ((iscs & ISCS::SE) && BIT(m_completion_request_state, irq))
		{
			m_icr[icr] &= ~ICR::ISE;
			m_icr[icr] |= ICR::S_COMPLETION_REQUEST;
		}
		else
		{
			uint32_t bap = space.read_word(isd + 0) | (space.read_byte(isd + 2) << 16); m_cpu->adjust_icount(-1);
			uint16_t ioa = space.read_word(isd + 4); m_cpu->adjust_icount(-1);
			uint16_t dc = space.read_word(isd + 6); m_cpu->adjust_icount(-1);

			uint16_t data = (iscs & ISCS::DIR) ?
				((iscs & ISCS::BW) ?
					space.read_word(bap) :
					space.read_byte(bap)) :
				((iscs & ISCS::BW) ?
					space.read_word(ioa) :
					space.read_byte(ioa));
			m_cpu->adjust_icount(-1);

			if (iscs & ISCS::DIR)
			{
				if (iscs & ISCS::BW)
					space.write_word(ioa, data);
				else
					space.write_byte(ioa, data);
			}
			else
			{
				if (iscs & ISCS::BW)
					space.write_word(bap, data);
				else
					space.write_byte(bap, data);
			}

			m_cpu->adjust_icount(-1);

			if (!(iscs & ISCS::BF))
			{
				space.write_word(isd + 0, bap + ((iscs & ISCS::BW) ? 2 : 1)); m_cpu->adjust_icount(-1);
			}

			if (!(iscs & ISCS::IF))
			{
				space.write_word(isd + 4, ioa + ((iscs & ISCS::BW) ? 2 : 1)); m_cpu->adjust_icount(-1);
			}

			dc--;
			space.write_word(isd + 6, dc); m_cpu->adjust_icount(-1);

			if (!m_i2osclr_cb[irq].isunset())
			{
				m_i2osclr_cb[irq](1);
				m_i2osclr_cb[irq](0);
			}

			if (dc == 0)
			{
				m_icr[icr] &= ~ICR::ISE;
				m_icr[icr] |= ICR::S_COUNT_COMPLETION;
			}
			else
				return 0;
		}
	}

	return irq + m_external_interrupt_vector;
}
