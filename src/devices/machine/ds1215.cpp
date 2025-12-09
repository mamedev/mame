// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Dallas Semiconductor DS1215 Phantom Time Chip
 *
 * Sources:
 *  - Dallas Semiconductor 1992-1993 Product Data Book
 *  - Dallas Semiconductor DS1215 Phantom Time Chip, Copyright 1997 Dallas Semiconductor Corporation
 *
 * The DS1215 is an integrated circuit which can be optionally coupled with a
 * CMOS static RAM. Its nonvolatile memory control functions may be enabled or
 * disabled through a dedicated input line. This device does not have any
 * address input lines, and is accessed using its chip enable input (/CEI),
 * output enable (/OE) and write enable (/WE) inputs. Data is input or output
 * on dedicated D and Q lines. DS1315 is a drop-in replacement for the DS1215,
 * differing only in offering 3.3V operation and expanded temperature range.
 *
 * The DS1216 SmartWatch/RAM and SmartWatch/ROM devices are DIP sockets with an
 * integrated quartz crystal, lithium battery and CMOS watch function. The
 * internal operation of these devices is identical to the DS1215, however the
 * access method varies between SmartWatch/RAM and SmartWatch/ROM device types.
 * The RAM type operates identically to the DS1215, while the ROM type supports
 * a "read-only" access mechanism using address lines A0 and A2.
 *
 * Address line A2 is treated as an active-low write enable input, while A0 is
 * used for the input data bit when data is being written to the device. When
 * the device is being read, data output is available on D0.
 *
 * TODO:
 *  - DS124xY variants
 */
/*
 * Implementation Notes
 * --------------------
 * The ceo() callback and ceo_r() provide access to the active-low chip enable
 * output (/CEO) signal, which may be used to enable or disable access to a RAM
 * or ROM device which shares the same address decode output. /CEO is negated
 * during the 64 cycles following a successful pattern recognition sequence.
 *
 * The ds1216e_device::read(offs_t offset) handler implements the SmartWatch/ROM
 * interface, decoding the offset as described above to provide both read and
 * write access to the chip. ds1215_device should be used for DS1215/DS1315 and
 * DS1216 SmartWatch/RAM variants.
 *
 */

#include "emu.h"
#include "ds1215.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DS1215, ds1215_device, "ds1215", "Dallas Semiconductor DS1215 Phantom Time Chip")
DEFINE_DEVICE_TYPE(DS1216E, ds1216e_device, "ds1216e", "Dallas Semiconductor DS1216E SmartWatch/ROM")

enum mode : u8
{
	MODE_IDLE,
	MODE_DATA,
};

enum reg3_mask : u8
{
	REG3_12 = 0x80, // enable 12 hour time
	REG3_PM = 0x20, // AM/PM flag (1=PM)
};
enum reg4_mask : u8
{
	REG4_RST = 0x10, // disable reset
	REG4_OSC = 0x20, // disable oscillator
};

ds1215_device_base::ds1215_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_ceo(*this)
	, m_timer(nullptr)
	, m_mode(MODE_IDLE)
	, m_count(0)
	, m_reg{}
	, m_ceo_state(false)
{
}

ds1215_device::ds1215_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ds1215_device_base(mconfig, DS1215, tag, owner, clock)
{
}

ds1216e_device::ds1216e_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: ds1215_device_base(mconfig, DS1216E, tag, owner, clock)
{
}

void ds1215_device_base::device_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_count));
	save_item(NAME(m_reg));
	save_item(NAME(m_ceo_state));

	m_timer = timer_alloc(FUNC(ds1215_device::timer), this);

	update_ceo();
}

void ds1215_device_base::device_reset()
{
	if (!(m_reg[4] & REG4_RST))
	{
		m_mode = MODE_IDLE;
		m_count = 0;

		update_ceo();
	}

	m_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

bool ds1215_device_base::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, &m_reg[0], std::size(m_reg));
	return !err && (actual == std::size(m_reg));
}

bool ds1215_device_base::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, &m_reg[0], std::size(m_reg));
	return !err;
}

void ds1215_device_base::nvram_default()
{
	m_reg[0] = 0; // second/100 = 0
	m_reg[1] = 0; // second = 0
	m_reg[2] = 0; // minute = 0
	m_reg[3] = 0; // 24 hour time, hour = 0
	m_reg[4] = 1; // enable oscillator, enable reset, day of week = 1
	m_reg[5] = 1; // day of month = 1
	m_reg[6] = 1; // month = 1
	m_reg[7] = 0; // year = 0
}

void ds1215_device_base::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_reg[0] = 0; // always zero second/100
	m_reg[1] = convert_to_bcd(second);
	m_reg[2] = convert_to_bcd(minute);
	m_reg[3] &= REG3_12;
	if (m_reg[3] & REG3_12)
	{
		// adjust for PM
		if (hour > 11)
		{
			m_reg[3] |= REG3_PM;
			hour -= 12;
		}

		m_reg[3] |= convert_to_bcd(hour ? hour : 12);
	}
	else
		// 24 hour time
		m_reg[3] |= convert_to_bcd(hour);
	m_reg[4] = (m_reg[4] & (REG4_OSC | REG4_RST)) | convert_to_bcd(day_of_week);
	m_reg[5] = convert_to_bcd(day);
	m_reg[6] = convert_to_bcd(month);
	m_reg[7] = convert_to_bcd(year);
}

u8 ds1215_device_base::read_bit()
{
	u8 data = 0;

	switch (m_mode)
	{
	case MODE_IDLE:
		// read restarts pattern recognition
		if (m_count)
		{
			LOG("pattern recognition restarted\n");
			m_count = 0;
		}
		break;

	case MODE_DATA:
		data = BIT(m_reg[m_count >> 3], m_count & 7);
		if (m_count == 63)
		{
			LOG("data read completed\n");

			m_mode = MODE_IDLE;
			m_count = 0;

			update_ceo();
		}
		else
			m_count++;
		break;
	}

	return data;
}

void ds1215_device_base::write_bit(u8 data)
{
	static constexpr u8 pattern[] = { 0xc5, 0x3a, 0xa3, 0x5c, 0xc5, 0x3a, 0xa3, 0x5c };

	switch (m_mode)
	{
	case MODE_IDLE:
		if (BIT(pattern[m_count >> 3], m_count & 7) == (data & 1))
		{
			// match, check if finished
			if (m_count == 63)
			{
				LOG("pattern recognition completed\n");
				m_mode = MODE_DATA;
				m_count = 0;

				update_ceo();
			}
			else
				m_count++;
		}
		else if (m_count)
		{
			// no match, abort sequence
			LOG("pattern recognition aborted\n");
			m_count = 0;
		}
		break;

	case MODE_DATA:
		if (data & 1)
			m_reg[m_count >> 3] |= 1U << (m_count & 7);
		else
			m_reg[m_count >> 3] &= ~(1U << (m_count & 7));

		if (m_count == 63)
		{
			LOG("data write completed\n");

			// clear reserved bits
			m_reg[1] &= 0x7f;
			m_reg[2] &= 0x7f;
			m_reg[3] &= 0xbf;
			m_reg[4] &= 0x37;
			m_reg[5] &= 0x3f;
			m_reg[6] &= 0x1f;

			// retrieve date/time from registers
			int const year = bcd_to_integer(m_reg[7]);
			int const month = bcd_to_integer(m_reg[6]);
			int const day = bcd_to_integer(m_reg[5]);
			int const day_of_week = bcd_to_integer(m_reg[4] & 0x7);
			int hour = bcd_to_integer(m_reg[3] & 0x3f);
			int const minute = bcd_to_integer(m_reg[2]);
			int const second = bcd_to_integer(m_reg[1]);

			// check for 12 hour mode
			if (m_reg[3] & REG3_12)
			{
				hour = bcd_to_integer(m_reg[3] & 0x1f);

				// adjust for PM
				if (m_reg[3] & REG3_PM)
					hour = (hour + 12) % 24;
			}

			// update clock
			LOG("time/date set %d-%d-%d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
			set_time(false, year, month, day, day_of_week, hour, minute, second);

			m_mode = MODE_IDLE;
			m_count = 0;

			update_ceo();
		}
		else
			m_count++;
		break;
	}
}

void ds1215_device_base::timer(s32 param)
{
	// register 4 bit 5 disables oscillator
	if (m_reg[4] & REG4_OSC)
		return;

	int const hundredths = bcd_to_integer(m_reg[0]);
	if (hundredths < 99)
		m_reg[0] = convert_to_bcd(hundredths + 1);
	else
		advance_seconds();
}

void ds1215_device_base::update_ceo()
{
	// ceo is asserted except when in data i/o mode
	bool const ceo = m_mode != MODE_DATA;

	if (m_ceo_state != ceo)
	{
		m_ceo_state = ceo;

		m_ceo(!m_ceo_state);
	}
}

u8 ds1215_device::read()
{
	if (!machine().side_effects_disabled())
		return read_bit();
	else
		return 0;
}

void ds1215_device::write(u8 data)
{
	write_bit(data & 1);
}

u8 ds1216e_device::read(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (BIT(offset, 2))
			return read_bit();
		else
			write_bit(BIT(offset, 0));

		return BIT(offset, 0);
	}
	else
		return 0;
}
