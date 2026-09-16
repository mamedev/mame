// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    mc146818.c

    Implementation of the MC146818 chip

    Real time clock chip with CMOS battery backed ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200, Apollo workstations

*********************************************************************/

#include "emu.h"
#include "mc146818.h"

#include "coreutil.h"

//#define VERBOSE 1
#include "logmacro.h"



// device type definition
DEFINE_DEVICE_TYPE(MC146818, mc146818_device, "mc146818", "MC146818 RTC")
DEFINE_DEVICE_TYPE(DS1287,   ds1287_device,   "ds1287",   "DS1287 RTC")
DEFINE_DEVICE_TYPE(DS1397,   ds1397_device,   "ds1397",   "DS1397 RAMified RTC")

//-------------------------------------------------
//  mc146818_device - constructor
//-------------------------------------------------

mc146818_device::mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc146818_device(mconfig, MC146818, tag, owner, clock)
{
	switch (clock)
	{
	case 4'194'304:
	case 1'048'576:
		m_tuc = 248;
		break;
	case 32'768:
		m_tuc = 1984;
		break;
	}
}

ds1287_device::ds1287_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc146818_device(mconfig, DS1287, tag, owner, clock)
{
}

ds1397_device::ds1397_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mc146818_device(mconfig, DS1397, tag, owner, clock)
{
}

mc146818_device::mc146818_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, device_rtc_interface(mconfig, *this)
	, m_region(*this, DEVICE_SELF)
	, m_index(0)
	, m_clock_timer(nullptr)
	, m_update_timer(nullptr)
	, m_periodic_timer(nullptr)
	, m_write_irq(*this)
	, m_write_sqw(*this)
	, m_century_index(-1)
	, m_epoch(0)
	, m_binary(false)
	, m_hour(false)
	, m_sqw_state(false)
	, m_tuc(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc146818_device::device_start()
{
	m_data = make_unique_clear<uint8_t[]>(data_size());
	m_clock_timer = timer_alloc(FUNC(mc146818_device::clock_tick), this);
	m_update_timer = timer_alloc(FUNC(mc146818_device::time_tick), this);
	m_periodic_timer = timer_alloc(FUNC(mc146818_device::periodic_tick), this);

	save_pointer(NAME(m_data), data_size());
	save_item(NAME(m_index));
	save_item(NAME(m_sqw_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc146818_device::device_reset()
{
	m_data[REG_B] &= ~(REG_B_UIE | REG_B_AIE | REG_B_PIE | REG_B_SQWE);
	m_data[REG_C] = 0;

	// square wave output is disabled
	if (m_sqw_state)
		m_write_sqw(CLEAR_LINE);

	update_irq();
}

//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mc146818_device::periodic_tick)
{
	m_sqw_state = !m_sqw_state;

	if (m_data[REG_B] & REG_B_SQWE)
		m_write_sqw(m_sqw_state);

	// periodic flag/interrupt on rising edge of periodic timer
	if (m_sqw_state)
	{
		m_data[REG_C] |= REG_C_PF;
		update_irq();
	}
}

TIMER_CALLBACK_MEMBER(mc146818_device::clock_tick)
{
	if (!(m_data[REG_B] & REG_B_SET))
	{
		m_data[REG_A] |= REG_A_UIP;

		m_update_timer->adjust(attotime::from_usec(244));
	}
}

TIMER_CALLBACK_MEMBER(mc146818_device::time_tick)
{
	if (!param)
	{
		/// TODO: find out how the real chip deals with updates when binary/bcd values are already outside the normal range
		int seconds = get_seconds() + 1;
		if (seconds < 60)
		{
			set_seconds(seconds);
		}
		else
		{
			set_seconds(0);

			int minutes = get_minutes() + 1;
			if (minutes < 60)
			{
				set_minutes(minutes);
			}
			else
			{
				set_minutes(0);

				int hours = get_hours() + 1;
				if (hours < 24)
				{
					set_hours(hours);
				}
				else
				{
					set_hours(0);

					int dayofweek = get_dayofweek() + 1;
					if (dayofweek <= 7)
					{
						set_dayofweek(dayofweek);
					}
					else
					{
						set_dayofweek(1);
					}

					int dayofmonth = get_dayofmonth() + 1;
					if (dayofmonth <= gregorian_days_in_month(get_month(), get_year() + 2000))
					{
						set_dayofmonth(dayofmonth);
					}
					else
					{
						set_dayofmonth(1);

						int month = get_month() + 1;
						if (month <= 12)
						{
							set_month(month);
						}
						else
						{
							set_month(1);

							int year = get_year() + 1;
							if (year <= 99)
							{
								set_year(year);
							}
							else
							{
								set_year(0);

								if (century_count_enabled())
								{
									set_century((get_century() + 1) % 100);
								}
							}
						}
					}
				}
			}
		}

		if ((m_data[REG_ALARM_SECONDS] == m_data[REG_SECONDS] || (m_data[REG_ALARM_SECONDS] & ALARM_DONTCARE) == ALARM_DONTCARE) &&
			(m_data[REG_ALARM_MINUTES] == m_data[REG_MINUTES] || (m_data[REG_ALARM_MINUTES] & ALARM_DONTCARE) == ALARM_DONTCARE) &&
			(m_data[REG_ALARM_HOURS] == m_data[REG_HOURS] || (m_data[REG_ALARM_HOURS] & ALARM_DONTCARE) == ALARM_DONTCARE))
		{
			// set the alarm interrupt flag AF
			m_data[REG_C] |= REG_C_AF;
		}

		// defer the update end sequence if update cycle time is non-zero
		if (m_tuc)
		{
			m_update_timer->adjust(attotime::from_usec(m_tuc), 1);
			return;
		}
	}

	// clear update in progress and set update ended
	m_data[REG_A] &= ~REG_A_UIP;
	m_data[REG_C] |= REG_C_UF;

	update_irq();
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void mc146818_device::nvram_default()
{
	// populate from a memory region if present
	if (m_region.found())
	{
		uint32_t bytes = m_region->bytes();

		if (bytes > data_size())
			bytes = data_size();

		memcpy(&m_data[0], m_region->base(), bytes);
		m_data[REG_D] |= REG_D_VRT;
	}
	else
	{
		memset(&m_data[0], 0, data_size());
	}

	if(m_binary)
		m_data[REG_B] |= REG_B_DM;
	if(m_hour)
		m_data[REG_B] |= REG_B_24_12;

	update_timer();
	update_irq();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool mc146818_device::nvram_read(util::read_stream &file)
{
	size_t const size = data_size();
	auto const [err, actual] = read(file, &m_data[0], size);
	if (err || (actual != size))
		return false;

	m_data[REG_D] |= REG_D_VRT;

	update_timer();
	update_irq();

	return true;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool mc146818_device::nvram_write(util::write_stream &file)
{
	size_t const size = data_size();
	auto const [err, actual] = write(file, &m_data[0], size);
	return !err;
}


//-------------------------------------------------
//  to_ram - convert value to current ram format
//-------------------------------------------------

int mc146818_device::to_ram(int a) const
{
	if (!(m_data[REG_B] & REG_B_DM))
		return dec_2_bcd(a);

	return a;
}


//-------------------------------------------------
//  from_ram - convert value from current ram format
//-------------------------------------------------

int mc146818_device::from_ram(int a) const
{
	if (!(m_data[REG_B] & REG_B_DM))
		return bcd_2_dec(a);

	return a;
}


int mc146818_device::get_seconds() const
{
	return from_ram(m_data[REG_SECONDS]);
}

void mc146818_device::set_seconds(int seconds)
{
	m_data[REG_SECONDS] = to_ram(seconds);
}

int mc146818_device::get_minutes() const
{
	return from_ram(m_data[REG_MINUTES]);
}

void mc146818_device::set_minutes(int minutes)
{
	m_data[REG_MINUTES] = to_ram(minutes);
}

int mc146818_device::get_hours() const
{
	if (!(m_data[REG_B] & REG_B_24_12))
	{
		int hours = from_ram(m_data[REG_HOURS] & ~HOURS_PM);

		if (hours == 12)
		{
			hours = 0;
		}

		if (m_data[REG_HOURS] & HOURS_PM)
		{
			hours += 12;
		}

		return hours;
	}
	else
	{
		return from_ram(m_data[REG_HOURS]);
	}
}

void mc146818_device::set_hours(int hours)
{
	if (!(m_data[REG_B] & REG_B_24_12))
	{
		int pm = 0;

		if (hours >= 12)
		{
			hours -= 12;
			pm = HOURS_PM;
		}

		if (hours == 0)
		{
			hours = 12;
		}

		m_data[REG_HOURS] = to_ram(hours) | pm;
	}
	else
	{
		m_data[REG_HOURS] = to_ram(hours);
	}
}

int mc146818_device::get_dayofweek() const
{
	return from_ram(m_data[REG_DAYOFWEEK]);
}

void mc146818_device::set_dayofweek(int dayofweek)
{
	m_data[REG_DAYOFWEEK] = to_ram(dayofweek);
}

int mc146818_device::get_dayofmonth() const
{
	return from_ram(m_data[REG_DAYOFMONTH]);
}

void mc146818_device::set_dayofmonth(int dayofmonth)
{
	m_data[REG_DAYOFMONTH] = to_ram(dayofmonth);
}

int mc146818_device::get_month() const
{
	return from_ram(m_data[REG_MONTH]);
}

void mc146818_device::set_month(int month)
{
	m_data[REG_MONTH] = to_ram(month);
}

int mc146818_device::get_year() const
{
	return from_ram(m_data[REG_YEAR]);
}

void mc146818_device::set_year(int year)
{
	m_data[REG_YEAR] = to_ram(year);
}

int mc146818_device::get_century() const
{
	assert(m_century_index != -1);
	return from_ram(m_data[m_century_index]);
}

void mc146818_device::set_century(int century)
{
	assert(m_century_index != -1);
	m_data[m_century_index] = to_ram(century);
}



//-------------------------------------------------
//  rtc_clock_updated - update clock with real time
//-------------------------------------------------

void mc146818_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
//  logerror("mc146818_set_base_datetime %02d/%02d/%02d %02d:%02d:%02d\n",
//          year, month, day,
//          hour, minute, second);

	set_seconds(second);
	set_minutes(minute);
	set_hours(hour);
	set_dayofweek(day_of_week);
	set_dayofmonth(day);
	set_month(month);

	if (m_epoch != 0)
		set_year((year - m_epoch) % (m_data[REG_B] & REG_B_DM ? 0x100 : 100)); // pcd actually depends on this
	else
		set_year(year % 100);

	if (m_century_index >= 0)
		set_century(year / 100);
}


//-------------------------------------------------
//  update_timer - update timer based on A register
//-------------------------------------------------

void mc146818_device::update_timer()
{
	int bypass = get_timer_bypass();

	attotime update_period = attotime::never;
	attotime update_interval = attotime::never;
	attotime periodic_period = attotime::never;
	attotime periodic_interval = attotime::never;

	if (bypass < 22)
	{
		int shift = 22 - bypass;

		double update_hz = (double) clock() / (1 << shift);

		// TODO: take the time since last timer into account
		update_period = attotime::from_hz(update_hz * 2);
		update_interval = attotime::from_hz(update_hz);

		int rate_select = m_data[REG_A] & (REG_A_RS3 | REG_A_RS2 | REG_A_RS1 | REG_A_RS0);
		if (rate_select != 0)
		{
			shift = (rate_select + 6) - bypass;
			if (shift <= 1)
				shift += 7;

			double periodic_hz = (double) clock() / (1 << shift);

			// TODO: take the time since last timer into account
			// periodic frequency is doubled to produce square wave output
			periodic_period = attotime::from_hz(periodic_hz * 4);
			periodic_interval = attotime::from_hz(periodic_hz * 2);
		}
	}

	m_clock_timer->adjust(update_period, 0, update_interval);
	m_periodic_timer->adjust(periodic_period, 0, periodic_interval);
}

//---------------------------------------------------------------
//  get_timer_bypass - get main clock divisor based on A register
//---------------------------------------------------------------

int mc146818_device::get_timer_bypass() const
{
	int bypass;

	switch (m_data[REG_A] & (REG_A_DV2 | REG_A_DV1 | REG_A_DV0))
	{
	case 0:
		bypass = 0;
		break;

	case REG_A_DV0:
		bypass = 2;
		break;

	case REG_A_DV1:
		bypass = 7;
		break;

	case REG_A_DV2 | REG_A_DV1:
	case REG_A_DV2 | REG_A_DV1 | REG_A_DV0:
		bypass = 22;
		break;

	default:
		// TODO: other combinations of divider bits are used for test purposes only
		bypass = 22;
		break;
	}

	return bypass;
}

//-------------------------------------------------
//  update_irq - Update irq based on B & C register
//-------------------------------------------------

void mc146818_device::update_irq()
{
	if (((m_data[REG_C] & REG_C_UF) && (m_data[REG_B] & REG_B_UIE)) ||
		((m_data[REG_C] & REG_C_AF) && (m_data[REG_B] & REG_B_AIE)) ||
		((m_data[REG_C] & REG_C_PF) && (m_data[REG_B] & REG_B_PIE)))
	{
		m_data[REG_C] |= REG_C_IRQF;
		m_write_irq(ASSERT_LINE);
	}
	else
	{
		m_data[REG_C] &= ~REG_C_IRQF;
		m_write_irq(CLEAR_LINE);
	}
}



//-------------------------------------------------
//  read - I/O handler for reading
//-------------------------------------------------

uint8_t mc146818_device::data_r()
{
	uint8_t data = internal_read(m_index);

	if (!machine().side_effects_disabled())
		LOG("mc146818_port_r(): offset=0x%02x data=0x%02x\n", m_index, data);

	return data;
}

uint8_t mc146818_device::read_direct(offs_t offset)
{
	offset %= data_logical_size();
	if (!machine().side_effects_disabled())
		internal_set_address(offset);

	uint8_t data = internal_read(offset);

	if (!machine().side_effects_disabled())
		LOG("mc146818_port_r(): offset=0x%02x data=0x%02x\n", offset, data);

	return data;
}

//-------------------------------------------------
//  write - I/O handler for writing
//-------------------------------------------------

void mc146818_device::address_w(uint8_t data)
{
	internal_set_address(data % data_logical_size());
}

void mc146818_device::data_w(uint8_t data)
{
	LOG("mc146818_port_w(): offset=0x%02x data=0x%02x\n", m_index, data);

	internal_write(m_index, data);
}

void mc146818_device::write_direct(offs_t offset, uint8_t data)
{
	offset %= data_logical_size();
	if (!machine().side_effects_disabled())
		internal_set_address(offset);

	LOG("mc146818_port_w(): offset=0x%02x data=0x%02x\n", offset, data);

	internal_write(offset, data);
}

void mc146818_device::internal_set_address(uint8_t address)
{
	m_index = address;
}

uint8_t mc146818_device::internal_read(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case REG_A:
		data = m_data[REG_A];
		break;

	case REG_C:
		// the unused bits b0 ... b3 are always read as 0
		data = m_data[REG_C] & (REG_C_IRQF | REG_C_PF | REG_C_AF | REG_C_UF);
		// read 0x0c will clear all IRQ flags in register 0x0c
		if (!machine().side_effects_disabled())
		{
			m_data[REG_C] &= ~(REG_C_IRQF | REG_C_PF | REG_C_AF | REG_C_UF);
			update_irq();
		}
		break;

	case REG_D:
		data = m_data[REG_D];
		// valid RAM and time
		m_data[REG_D] |= REG_D_VRT;
		break;

	default:
		data = m_data[offset];
		break;
	}

	return data;
}

void mc146818_device::internal_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case REG_SECONDS:
		// top bit of SECONDS is read only
		m_data[REG_SECONDS] = data & ~0x80;
		break;

	case REG_A:
		// top bit of A is read only
		if ((data ^ m_data[REG_A]) & ~REG_A_UIP)
		{
			m_data[REG_A] = data & ~REG_A_UIP;
			update_timer();
		}
		break;

	case REG_B:
		if ((data & REG_B_SET) && !(m_data[REG_B] & REG_B_SET))
			data &= ~REG_B_UIE;

		if (!(data & REG_B_SQWE) && (m_data[REG_B] & REG_B_SQWE) && m_sqw_state)
			m_write_sqw(CLEAR_LINE);

		m_data[REG_B] = data;
		update_irq();
		break;

	case REG_C:
	case REG_D:
		// register C & D is readonly
		break;

	default:
		m_data[offset] = data;
		break;
	}
}

void ds1397_device::device_start()
{
	mc146818_device::device_start();

	save_item(NAME(m_xram_page));
}

void ds1397_device::device_reset()
{
	mc146818_device::device_reset();

	m_xram_page = 0;
}

u8 ds1397_device::xram_r(offs_t offset)
{
	if (offset < 0x20)
		return m_data[0x40 + m_xram_page * 0x20 + offset];
	else
		return m_xram_page;
}

void ds1397_device::xram_w(offs_t offset, u8 data)
{
	if (offset < 0x20)
		m_data[0x40 + m_xram_page * 0x20 + offset] = data;
	else
		m_xram_page = data & 0x7f;
}
