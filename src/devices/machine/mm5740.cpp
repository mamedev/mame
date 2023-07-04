// license:BSD-3-Clause
// copyright-holders:R. Belmont, Mark Garlanger
/**********************************************************************

    National Semiconductor MM5740 Keyboard Encoder emulation
    (Code originally based on kb3600.cpp)

*********************************************************************/

#include "emu.h"
#include "mm5740.h"

#include <algorithm>


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(MM5740, mm5740_device, "mm5740", "MM5740 Keyboard Encoder")

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

ROM_START( mm5740 )
	ROM_REGION(0x1c2, "internal", 0)
	ROM_LOAD("mm5740aac.ic1", 0x000, 0x1c2, CRC(aed404d3) SHA1(e7b9feba5f789f388d27820b5f3fa591d41b4ab1))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mm5740_device::device_rom_region() const
{
	return ROM_NAME( mm5740 );
}


//-------------------------------------------------
//  mm5740_device - constructor
//-------------------------------------------------

mm5740_device::mm5740_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MM5740, tag, owner, clock),
	m_read_x(*this, 0x3ff),
	m_read_shift(*this, 0),
	m_read_control(*this, 0),
	m_write_data_ready(*this),
	m_rom(*this, "internal")
{
}

u32 mm5740_device::calc_effective_clock_key_debounce(u32 capacitance)
{
	// calculate key debounce based on capacitance in pF
	u32 key_debounce_msec = capacitance / 125;
	if (key_debounce_msec == 0)
	{
		key_debounce_msec = 1;
	}

	return 1000 / key_debounce_msec;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5740_device::device_start()
{
	std::fill(std::begin(m_x_mask), std::end(m_x_mask), 0);
	m_b = -1;
	m_repeat = false;
	m_last_repeat = false;

	// allocate timers
	m_scan_timer = timer_alloc(FUNC(mm5740_device::perform_scan), this);
	m_scan_timer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));

	// state saving
	save_item(NAME(m_b));
	save_item(NAME(m_offset));
	save_item(NAME(m_x_mask));
	save_item(NAME(m_repeat));
	save_item(NAME(m_last_repeat));
}

//-------------------------------------------------
//  perform_scan - scan the keyboard matrix
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mm5740_device::perform_scan)
{
	bool ako = false;

	for (int x = 0; x < 9; x++)
	{
		u16 data = m_read_x[x]() ^ 0x3ff;

		if (data)
		{
			ako = true;
		}

		if ((data ^ m_x_mask[x]) == 0)
		{
			// bail early if nothing has changed.
			continue;
		}

		for (int y = 0; y < 10; y++)
		{
			u16 offset = x * 10 + y;

			if (BIT(data, y))
			{
				u8 *rom = m_rom->base();
				// Common portion
				u16 common = (u16)rom[offset];

				u16 uniq_offset = offset + ((((m_read_shift() ? 1 : 0) + (m_read_control() ? 2 : 0)) + 1) * 90);

				// Unique portion based on shift/ctrl keys.
				u8 uniq = rom[uniq_offset];

				u16 b = (((common & 0x10) << 4) | ((uniq & 0x0f) << 4) | (common & 0x0f)) ^ 0x1ff;

				// Check for a new keypress
				if (!BIT(m_x_mask[x], y))
				{
					m_x_mask[x] |= (1 << y);
					if (m_b != b)
					{
						m_b = b;
						m_offset = offset;
						m_write_data_ready(ASSERT_LINE);

						return;
					}
				}
			}
			else
			{
				// key released, unmark it from the "down" info
				m_x_mask[x] &= ~(1 << y);
				if (m_offset == offset)
				{
					m_write_data_ready(CLEAR_LINE);
					m_b = -1;
				}
			}
		}
	}

	if ((m_repeat) && (!m_last_repeat) && (m_b != -1))
	{
		m_write_data_ready(ASSERT_LINE);
	}

	m_last_repeat = m_repeat;

	if (!ako)
	{
		m_write_data_ready(CLEAR_LINE);
		m_b = -1;
	}
}

void mm5740_device::repeat_line_w(int state)
{
	m_repeat = (state == ASSERT_LINE);
}

//-------------------------------------------------
//  b_r -
//-------------------------------------------------

u16 mm5740_device::b_r()
{
	m_write_data_ready(CLEAR_LINE);
	return m_b;
}

