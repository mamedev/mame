// license:BSD-3-Clause
/**********************************************************************

	Spectrum Next CTC

**********************************************************************/

#include "emu.h"
#include "specnext_ctc.h"


specnext_ctc_device::specnext_ctc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80ctc_device(mconfig, SPECNEXT_CTC, tag, owner, clock)
{}

int specnext_ctc_device::z80daisy_irq_ack()
{
	//z80ctc_channel_device &channel = *m_channel[0];
	int chanel = (z80ctc_device::z80daisy_irq_ack() - m_vector) / 2;
	return (chanel > 0 || (get_channel_int_state(0) == Z80_DAISY_IEO))
		? m_vector + (chanel + 3) * 2
		: m_vector;
}

// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_CTC, specnext_ctc_device, "ctc", "Spectrum Next CTC")
