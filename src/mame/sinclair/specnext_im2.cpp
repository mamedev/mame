// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next IM2
**********************************************************************/

#include "emu.h"
#include "specnext_im2.h"


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_IM2, specnext_im2_device, "specnext_im2", "Spectrum Next IM2")


specnext_im2_device::specnext_im2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_IM2, tag, owner, clock)
	, device_z80daisy_interface(mconfig, *this)
	, m_intr_cb(*this)
{
}


int specnext_im2_device::z80daisy_irq_state()
{
	return m_state;
}

int specnext_im2_device::z80daisy_irq_ack()
{
	m_state |= Z80_DAISY_IEO;
	return m_vector;
}

void specnext_im2_device::z80daisy_irq_reti()
{
	m_state &= ~Z80_DAISY_IEO;
}

TIMER_CALLBACK_MEMBER(specnext_im2_device::irq_off)
{
	m_state &= ~Z80_DAISY_INT;
	m_intr_cb(CLEAR_LINE);
}

void specnext_im2_device::int_w(u8 vector)
{
	m_vector = vector;
	m_state = Z80_DAISY_INT;
	m_intr_cb(ASSERT_LINE);
	m_irq_off_timer->adjust(clocks_to_attotime(32));
}


void specnext_im2_device::device_start()
{
	m_irq_off_timer = timer_alloc(FUNC(specnext_im2_device::irq_off), this);

	save_item(NAME(m_state));
	save_item(NAME(m_vector));
}

void specnext_im2_device::device_reset()
{
	m_irq_off_timer->reset();

	m_state = 0;
	m_vector = 0xff;
}
