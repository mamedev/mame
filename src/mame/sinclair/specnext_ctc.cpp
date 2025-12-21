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
	, m_int_cb(*this)
{
}

int specnext_ctc_device::z80daisy_irq_ack()
{
	m_ch_idx = 4;
	z80ctc_device::z80daisy_irq_ack();
	if (m_ch_idx < 4)
	{
		m_int_cb(ASSERT_LINE);
		return m_vector + ((3 + m_ch_idx) << 1);
	}
	else
		return 0xff;
}

void specnext_ctc_device::z80daisy_irq_reti()
{
	m_ch_idx = 4;
	z80ctc_device::z80daisy_irq_reti();
	if (m_ch_idx < 4)
		m_int_cb(CLEAR_LINE);
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

void specnext_ctc_device::channel_int_state_w(int ch, u8 state)
{
	m_ch_idx = ch;
	z80ctc_device::channel_int_state_w(ch, state);
}

u8 specnext_ctc_device::int_channel_r()
{
	assert(m_ch_idx < 4);
	return m_ch_idx;
}

void specnext_ctc_device::device_start()
{
	z80ctc_device::device_start();

	save_item(NAME(m_ch_idx));
}
