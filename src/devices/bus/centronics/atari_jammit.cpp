// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*
    Atari JAMMIT cabinet I/O and protection interface.

    The host places a command on D0-D7, asserts /STROBE, reads the reply on
    /FAULT, SELECT, PERROR and BUSY, then releases /STROBE.  Sample only the
    falling edge: data pins arrive individually, and identical command bytes
    still need to clock the stream on successive strobe pulses.

    Protocol and beam-counter behaviour are retained from atari/mediagx.cpp;
    physical output functions and complete original-hardware timing remain
    unverified.  The motherboard's Super I/O configuration is not modelled here.
*/

#include "emu.h"
#include "atari_jammit.h"

DEFINE_DEVICE_TYPE(ATARI_JAMMIT, atari_jammit_device, "atari_jammit", "Atari JAMMIT cabinet interface")

atari_jammit_device::atari_jammit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ATARI_JAMMIT, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_system_cb(*this, 0),
	m_starts_cb(*this, 0),
	m_gun_cb(*this, 0),
	m_output_cb(*this)
{
}

void atari_jammit_device::device_start()
{
	save_item(NAME(m_data));
	save_item(NAME(m_strobe));
	save_item(NAME(m_pointer));
	save_item(NAME(m_reply));
	save_item(NAME(m_lfsr));
	save_item(NAME(m_previous));
	save_item(NAME(m_output_latch));
	save_item(NAME(m_gun_latch));
	save_item(NAME(m_gun_flags));
	machine().save().register_postload(save_prepost_delegate(FUNC(atari_jammit_device::postload), this));
}

void atari_jammit_device::device_reset()
{
	// Keep electrical input levels: these are driven by the LPT output latches.
	m_pointer = 0;
	m_reply = 0;
	m_lfsr = 0;
	m_previous = 0;
	m_output_latch = 0;
	std::fill(std::begin(m_gun_latch), std::end(m_gun_latch), 0);
	m_gun_flags = 0;
	postload();
}

void atari_jammit_device::postload()
{
	update_outputs();
	update_status();
}

void atari_jammit_device::data_w(unsigned bit, int state)
{
	m_data = (m_data & ~(1U << bit)) | (unsigned(bool(state)) << bit);
}

void atari_jammit_device::input_strobe(int state)
{
	bool const strobe = bool(state);
	if (m_strobe && !strobe)
		clock_command();
	m_strobe = strobe;
}

void atari_jammit_device::update_outputs()
{
	m_output_cb(m_output_latch);
}

void atari_jammit_device::update_status()
{
	output_fault(BIT(m_reply, 0));
	output_select(BIT(m_reply, 1));
	output_perror(BIT(m_reply, 2));
	output_ack(0);
	// PC_LPT inverts the physical BUSY level when reading status bit 7.
	output_busy(BIT(m_reply, 3));
}

uint8_t atari_jammit_device::input_r()
{
	unsigned const pos = m_pointer;
	if (pos == 0)
	{
		m_gun_flags = 0;
		for (unsigned player = 0; player < 2; ++player)
		{
			uint32_t const gun = m_gun_cb(player);
			m_gun_latch[player * 2] = gun & 0x0fff;
			m_gun_latch[player * 2 + 1] = (gun >> 12) & 0x03ff;
			m_gun_flags |= (BIT(gun, 24) << player) | (BIT(gun, 25) << (player + 4));
		}
	}
	if (pos >= 1 && pos <= 3)
		return (m_system_cb() >> ((pos - 1) * 4)) & 15;
	if (pos == 4)
		return m_starts_cb() & 15;
	if (pos == 13)
		return 5;
	if (pos == 14 || pos == 15)
		return (m_gun_flags >> ((pos - 14) * 4)) & 15;
	if (pos >= 16 && pos < 28)
		return (m_gun_latch[(pos - 16) / 3] >> (((pos - 16) % 3) * 4)) & 15;
	return 0;
}

void atari_jammit_device::clock_command()
{
	uint8_t const command = m_data;
	unsigned shift = 0;
	uint16_t mask = 0;
	switch (command & 0xf0)
	{
	case 0x30: shift = 0; mask = 0x000f; break;
	case 0x20: shift = 4; mask = 0x00f0; break;
	case 0x40: shift = 8; mask = 0x0f00; break;
	case 0x50: shift = 12; mask = 0x3000; break;
	}
	if (mask)
	{
		uint16_t const next = (m_output_latch & ~mask) | ((command << shift) & mask);
		if (next != m_output_latch)
		{
			m_output_latch = next;
			update_outputs();
		}
	}
	// Preserve the previous-pointer/previous-LFSR pipeline, including reset.
	m_reply = input_r() ^ (m_previous & 15);
	m_previous = m_lfsr;
	if (command <= 0x10)
	{
		if (command == 0x10)
			m_lfsr = 0xffff;
		else
			m_lfsr |= 1 << command;
	}
	else
	{
		bool const feedback = BIT(m_lfsr, 13) != BIT(m_lfsr, 14);
		m_lfsr = (m_lfsr << 1) | feedback;
	}
	if ((command & 0xfc) == 0x18)
		m_pointer = command & 3;
	else if (m_pointer < 28)
		++m_pointer;
	update_status();
}
