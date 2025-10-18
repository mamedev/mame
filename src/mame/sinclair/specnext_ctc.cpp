// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next CTC
**********************************************************************/

#include "emu.h"
#include "specnext_ctc.h"


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_CTC, specnext_ctc_device, "specnext_ctc", "Spectrum Next CTC")


specnext_ctc_device::specnext_ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80ctc_device(mconfig, SPECNEXT_CTC, tag, owner, clock)
{
}

int specnext_ctc_device::z80daisy_irq_ack()
{
	int const channel = (z80ctc_device::z80daisy_irq_ack() - m_vector) / 2;
	return ((channel > 0) || (channel_int_state(0) == Z80_DAISY_IEO))
		? (m_vector + ((channel + 3) << 1))
		: m_vector;
}
