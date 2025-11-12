// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next CTC
**********************************************************************/

#include "emu.h"
#include "specnext_ctc.h"

constexpr u16 CONTROL_WORD      = 0x01;

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
		: 0xff;
}

void specnext_ctc_device::ctrl_int_w(u8 ch_mask)
{
	for (int ch = 0; ch < 4; ch++)
	{
		u8 mode = channel_mode(ch);
		if (ch_mask & (1 << ch))
			mode |= 0x80;
		else
			mode &= ~0x80;

		write(ch, CONTROL_WORD | mode);
	}
}

u8 specnext_ctc_device::ctrl_int_r()
{
	u8 int_mode = 0;
	for (int ch = 0; ch < 4; ch++)
	{
		if (channel_mode(ch) & 0x80)
			int_mode |= (1 << ch);
	}

	return int_mode;
}
