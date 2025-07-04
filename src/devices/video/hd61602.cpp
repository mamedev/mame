// license:BSD-3-Clause
// copyright-holders:m1macrophage

/*
Not emulated:
- External oscillator (OSC1) [*]
- Effects of resistor between OSC1 and OSC2, when using internal oscillator [*].
- SB pin, halts internal clock.
- SYNC pin for chip cascading.
- READY pin.

[*] The datasheet specifies the frequency of the internal oscillator as
100KHz +/- 30KHz. This requires using the recommended resistor values. Examples
are shown with a 330K and a 360K resistor. An external oscillator can be used
too, and it is also specified as 100Khz +/- 30KHz.
*/

#include "emu.h"
#include "hd61602.h"

DEFINE_DEVICE_TYPE(HD61602, hd61602_device, "hd61602", "Hitachi HD61602 LCD Driver")

hd61602_device::hd61602_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, HD61602, tag, owner, clock)
	, m_refresh_timer(nullptr)
	, m_write_segs(*this)
{
}

void hd61602_device::device_start()
{
	m_refresh_timer = timer_alloc(FUNC(hd61602_device::refresh_timer_tick), this);

	m_count = 0;
	m_data = 0;
	m_ram = {0, 0, 0, 0};
	m_blank = false;
	m_display_mode = DM_STATIC;
	m_drive_mode = DM_STATIC;
	m_com_count = 1;
	m_active_com = 0;

	save_item(NAME(m_count));
	save_item(NAME(m_data));
	save_item(NAME(m_ram));
	save_item(NAME(m_blank));
	save_item(NAME(m_display_mode));
	save_item(NAME(m_drive_mode));
	save_item(NAME(m_com_count));
	save_item(NAME(m_active_com));
}

void hd61602_device::set_ram_bit(u8 com, u8 seg, u8 bit)
{
	if (com >= NCOM || seg >= NSEG)
		return;
	const u64 new_bit = u64(bit ? 1 : 0) << seg;
	const u64 mask = ~(u64(1) << seg);
	m_ram[com] = (m_ram[com] & mask) | new_bit;
}

void hd61602_device::set_drive_mode(u8 drive_mode)
{
	if (drive_mode == m_drive_mode)
		return;

	const u8 old_com_count = m_com_count;
	m_drive_mode = drive_mode;
	m_active_com = 0;

	int frame_rate = 0;
	switch (m_drive_mode)
	{
		// Frame rates are reported on the datasheet for an 100KHz clock.
		// 100KHz is the nominal frequency of the internal oscillator, and the
		// recommended frequency for an external oscillator.
		case DM_STATIC:  m_com_count = 1; break;
		case DM_HALF:    m_com_count = 2; frame_rate =  65; break;
		case DM_THIRD:   m_com_count = 3; frame_rate = 208; break;
		case DM_QUARTER: m_com_count = 4; frame_rate = 223; break;
	}

	if (m_com_count > 1)
	{
		const attotime timer_t = attotime::from_hz(frame_rate * m_com_count);
		m_refresh_timer->adjust(timer_t, 0, timer_t);
	}
	else
	{
		// The frame rate in 'static' mode is actually 33 Hz (assuming a 100KHz
		// clock). But there is no time-multiplexing in that mode, so there is
		// no need for the timer.
		m_refresh_timer->reset();
	}

	// Clear rows that will no longer be updated in the new mode.
	for (u8 com = m_com_count; com < old_com_count; ++com)
		m_write_segs(com, 0);
}

TIMER_CALLBACK_MEMBER(hd61602_device::refresh_timer_tick)
{
	m_write_segs(m_active_com, 0);
	m_active_com = (m_active_com + 1) % m_com_count;
	if (m_blank)
		m_write_segs(m_active_com, 0);
	else
		m_write_segs(m_active_com, m_ram[m_active_com] & make_bitmask<u64>(NSEG));
}

void hd61602_device::data_w(u8 data)
{
	// 1-byte nop command
	if (m_count == 0 && (data & 0xc0) == 0xc0)
		return;

	m_data = (m_data << 8) | data;
	m_count = (m_count + 1) & 1;
	if (m_count != 0)  // Waiting for the next byte.
		return;

	switch (BIT(m_data, 14, 2))
	{
		// Update display RAM byte, using a logical segment address.
		// The address-to-bit mapping depends on the configured display mode.
		case 0:
		{
			u8 seg_count = 8;
			u8 max_address = 6;
			bool skip_seg0_com2 = false;
			switch (m_display_mode)
			{
				case DM_HALF:
					seg_count = 4;
					max_address = 12;
					break;
				case DM_THIRD:
					seg_count = 3;
					max_address = 16;
					skip_seg0_com2 = true;
					break;
				case DM_QUARTER:
					seg_count = 2;
					max_address = 25;
					break;
				default:
					assert(m_display_mode == DM_STATIC);
					break;
			}

			const u8 address = BIT(m_data, 8, 5);
			if (address <= max_address)
			{
				assert(seg_count * m_com_count - (skip_seg0_com2 ? 1 : 0) == 8);
				const u8 start_seg = address * seg_count;
				int bit_offset = 7;
				for (u8 s = 0; s < seg_count; ++s)
					for (u8 com = 0; com < m_com_count; ++com)
						if (!(skip_seg0_com2 && s == 0 && com == 2))
							// When address == max_address, some of the segment
							// addresses won't be valid. set_ram_bit() will
							// ignore those.
							set_ram_bit(com, start_seg + s, BIT(m_data, bit_offset--));
			}
			break;
		}
		// Update a specific bit in display RAM, addressed by COM and SEG.
		case 1:
		{
			const u8 com = BIT(m_data, 8, 2);
			const u8 seg = BIT(m_data, 0, 6);
			set_ram_bit(com, seg, BIT(m_data, 13));
			break;
		}
		// Configure mode.
		case 2:
		{
			if (BIT(m_data, 12) == 0)
			{
				// Drive mode and display mode are configured independently, even
				// though they should generally match. Drive mode controls the
				// actual display signals, whereas display mode controls how byte
				// writes ate interpreted.
				m_display_mode = BIT(m_data, 0, 2);
				m_blank = BIT(m_data, 2);
				// bits 3-7 unused.
				set_drive_mode(BIT(m_data, 8, 2));
				// bit 10: READY mode. Not implemented.
				// bit 11: Set external power supply. Not relevant.
				// bit 12: must be 0 to configure the mode.
				// bit 13: unused.
			}
			break;
		}
		// case 3 is a 1-byte nop command, checked at the start of this function.
	}

	// Updates for modes other than 'static' are handled in refresh_timer_tick().
	if (m_drive_mode == DM_STATIC)
		m_write_segs(0, m_blank ? 0 : (m_ram[0] & make_bitmask<u64>(NSEG)));
}

void hd61602_device::reset_counter_strobe()
{
	m_count = 0;
}
