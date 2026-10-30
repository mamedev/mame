// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next IM2
**********************************************************************/

#include "emu.h"
#include "specnext_im2.h"


#define VERBOSE     0
#include "logmacro.h"


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_IM2, specnext_im2_device, "specnext_im2", "Spectrum Next IM2")


specnext_im2_device::specnext_im2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_IM2, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_irq_cb(*this)
{
}


int specnext_im2_device::z80daisy_irq_state()
{
	return m_state;
}

int specnext_im2_device::z80daisy_irq_ack()
{
	m_state = Z80_DAISY_IEO;
	m_irq_cb(CLEAR_LINE);

	return m_vector;
}

void specnext_im2_device::z80daisy_irq_reti()
{
	if (m_state & Z80_DAISY_IEO)
	{
		m_state &= ~Z80_DAISY_IEO;
		m_irq_cb((m_state & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE);
	}
}

void specnext_im2_device::irq_w(int state)
{
	if (state == CLEAR_LINE)
	{
		m_state = 0;
		m_irq_cb(CLEAR_LINE);
	}
	else if (m_state & Z80_DAISY_IEO)
	{
		LOG("IM2: Ignoring IRQ while in IEO\n");
	}
	else
	{
		// FPGA im2_device in S_ISR cannot transition to S_REQ;
		// don't assert INT while being serviced (IEO set), or
		// the daisy chain would ACK this device, consuming the
		// new INT, and a subsequent RETI would clear IEO while
		// the original ISR still runs.
		m_state |= Z80_DAISY_INT;
		m_irq_cb(ASSERT_LINE);
	}
}


void specnext_im2_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_vector));
}

void specnext_im2_device::device_reset()
{
	m_state = 0;
	m_vector = 0xff;
}
