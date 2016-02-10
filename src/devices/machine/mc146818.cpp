// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    mc146818.c

    Implementation of the MC146818 chip

    Real time clock chip with CMOS battery backed ram
    Used in IBM PC/AT, several PC clones, Amstrad NC200, Apollo workstations

*********************************************************************/

#include "coreutil.h"
#include "machine/mc146818.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_MC146818        0



// device type definition
const device_type MC146818 = &device_creator<mc146818_device>;

//-------------------------------------------------
//  mc146818_device - constructor
//-------------------------------------------------

mc146818_device::mc146818_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC146818, "MC146818 RTC", tag, owner, clock, "mc146818", __FILE__),
		device_nvram_interface(mconfig, *this),
		m_index(0),
		m_last_refresh(attotime::zero), m_clock_timer(nullptr), m_periodic_timer(nullptr),
		m_write_irq(*this),
		m_century_index(-1),
		m_epoch(0),
		m_use_utc(false),
		m_binary(false),
		m_hour(false),
		m_binyear(false)
{
}

mc146818_device::mc146818_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_nvram_interface(mconfig, *this),
		m_index(0),
		m_last_refresh(attotime::zero), m_clock_timer(nullptr), m_periodic_timer(nullptr),
		m_write_irq(*this),
		m_century_index(-1),
		m_epoch(0),
		m_use_utc(false),
		m_binary(false),
		m_hour(false),
		m_binyear(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc146818_device::device_start()
{
	m_data.resize(data_size());
	m_last_refresh = machine().time();
	m_clock_timer = timer_alloc(TIMER_CLOCK);
	m_periodic_timer = timer_alloc(TIMER_PERIODIC);

	m_write_irq.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc146818_device::device_reset()
{
	m_data[REG_B] &= ~(REG_B_UIE | REG_B_AIE | REG_B_PIE | REG_B_SQWE);
	m_data[REG_C] = 0;

	update_irq();
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mc146818_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PERIODIC:
		m_data[REG_C] |= REG_C_PF;
		update_irq();
		break;

	case TIMER_CLOCK:
		if (!(m_data[REG_B] & REG_B_SET))
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

								set_year((get_year() + 1) % 100);
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

			// set the update-ended interrupt Flag UF
			m_data[REG_C] |=  REG_C_UF;
			update_irq();

			m_last_refresh = machine().time();
		}
		break;
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void mc146818_device::nvram_default()
{
	// populate from a memory region if present
	if (m_region != nullptr)
	{
		UINT32 bytes = m_region->bytes();

		if (bytes > data_size())
			bytes = data_size();

		memcpy(&m_data[0], m_region->base(), bytes);
	}
	else
	{
		memset(&m_data[0], 0, data_size());
	}

	if(m_binary)
		m_data[REG_B] |= REG_B_DM;
	if(m_hour)
		m_data[REG_B] |= REG_B_24_12;

	set_base_datetime();
	update_timer();
	update_irq();
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void mc146818_device::nvram_read(emu_file &file)
{
	file.read(&m_data[0], data_size());

	set_base_datetime();
	update_timer();
	update_irq();
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void mc146818_device::nvram_write(emu_file &file)
{
	file.write(&m_data[0], data_size());
}


//-------------------------------------------------
//  to_ram - convert value to current ram format
//-------------------------------------------------

int mc146818_device::to_ram(int a)
{
	if (!(m_data[REG_B] & REG_B_DM))
		return dec_2_bcd(a);

	return a;
}


//-------------------------------------------------
//  from_ram - convert value from current ram format
//-------------------------------------------------

int mc146818_device::from_ram(int a)
{
	if (!(m_data[REG_B] & REG_B_DM))
		return bcd_2_dec(a);

	return a;
}


int mc146818_device::get_seconds()
{
	return from_ram(m_data[REG_SECONDS]);
}

void mc146818_device::set_seconds(int seconds)
{
	m_data[REG_SECONDS] = to_ram(seconds);
}

int mc146818_device::get_minutes()
{
	return from_ram(m_data[REG_MINUTES]);
}

void mc146818_device::set_minutes(int minutes)
{
	m_data[REG_MINUTES] = to_ram(minutes);
}

int mc146818_device::get_hours()
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

int mc146818_device::get_dayofweek()
{
	return from_ram(m_data[REG_DAYOFWEEK]);
}

void mc146818_device::set_dayofweek(int dayofweek)
{
	m_data[REG_DAYOFWEEK] = to_ram(dayofweek);
}

int mc146818_device::get_dayofmonth()
{
	return from_ram(m_data[REG_DAYOFMONTH]);
}

void mc146818_device::set_dayofmonth(int dayofmonth)
{
	m_data[REG_DAYOFMONTH] = to_ram(dayofmonth);
}

int mc146818_device::get_month()
{
	return from_ram(m_data[REG_MONTH]);
}

void mc146818_device::set_month(int month)
{
	m_data[REG_MONTH] = to_ram(month);
}

int mc146818_device::get_year()
{
	return from_ram(m_data[REG_YEAR]);
}

void mc146818_device::set_year(int year)
{
	m_data[REG_YEAR] = to_ram(year);
}



//-------------------------------------------------
//  set_base_datetime - update clock with real time
//-------------------------------------------------

void mc146818_device::set_base_datetime()
{
	system_time systime;
	system_time::full_time current_time;

	machine().base_datetime(systime);

	current_time = (m_use_utc) ? systime.utc_time: systime.local_time;

//  logerror("mc146818_set_base_datetime %02d/%02d/%02d %02d:%02d:%02d\n",
//          current_time.year % 100, current_time.month + 1, current_time.mday,
//          current_time.hour,current_time.minute, current_time.second);

	set_seconds(current_time.second);
	set_minutes(current_time.minute);
	set_hours(current_time.hour);
	set_dayofweek(current_time.weekday + 1);
	set_dayofmonth(current_time.mday);
	set_month(current_time.month + 1);

	if(m_binyear)
		set_year((current_time.year - m_epoch) % (m_data[REG_B] & REG_B_DM ? 0x100 : 100)); // pcd actually depends on this
	else
		set_year((current_time.year - m_epoch) % 100);

	if (m_century_index >= 0)
		m_data[m_century_index] = to_ram(current_time.year / 100);
}


//-------------------------------------------------
//  update_timer - update timer based on A register
//-------------------------------------------------

void mc146818_device::update_timer()
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
			periodic_period = attotime::from_hz(periodic_hz * 2);
			periodic_interval = attotime::from_hz(periodic_hz);
		}
	}

	m_clock_timer->adjust(update_period, 0, update_interval);
	m_periodic_timer->adjust(periodic_period, 0, periodic_interval);
}


//-------------------------------------------------
//  update_irq - Update irq based on B & C register
//-------------------------------------------------

void mc146818_device::update_irq()
{
	// IRQ line is active low
	if (((m_data[REG_C] & REG_C_UF) && (m_data[REG_B] & REG_B_UIE)) ||
		((m_data[REG_C] & REG_C_AF) && (m_data[REG_B] & REG_B_AIE)) ||
		((m_data[REG_C] & REG_C_PF) && (m_data[REG_B] & REG_B_PIE)))
	{
		m_data[REG_C] |= REG_C_IRQF;
		m_write_irq(CLEAR_LINE);
	}
	else
	{
		m_data[REG_C] &= REG_C_IRQF;
		m_write_irq(ASSERT_LINE);
	}
}



//-------------------------------------------------
//  read - I/O handler for reading
//-------------------------------------------------

READ8_MEMBER( mc146818_device::read )
{
	UINT8 data = 0;
	switch (offset)
	{
	case 0:
		data = m_index;
		break;

	case 1:
		switch (m_index)
		{
		case REG_A:
			data = m_data[REG_A];
			// Update In Progress (UIP) time for 32768 Hz is 244+1984usec
			/// TODO: support other dividers
			/// TODO: don't set this if update is stopped
			if ((space.machine().time() - m_last_refresh) < attotime::from_usec(244+1984))
				data |= REG_A_UIP;
			break;

		case REG_C:
			// the unused bits b0 ... b3 are always read as 0
			data = m_data[REG_C] & (REG_C_IRQF | REG_C_PF | REG_C_AF | REG_C_UF);
			// read 0x0c will clear all IRQ flags in register 0x0c
			m_data[REG_C] &= ~(REG_C_IRQF | REG_C_PF | REG_C_AF | REG_C_UF);
			update_irq();
			break;

		case REG_D:
			/* battery ok */
			data = m_data[REG_D] | REG_D_VRT;
			break;

		default:
			data = m_data[m_index];
			break;
		}
		break;
	}

	if (LOG_MC146818)
		logerror("mc146818_port_r(): index=0x%02x data=0x%02x\n", m_index, data);

	return data;
}


//-------------------------------------------------
//  write - I/O handler for writing
//-------------------------------------------------

WRITE8_MEMBER( mc146818_device::write )
{
	if (LOG_MC146818)
		logerror("mc146818_port_w(): index=0x%02x data=0x%02x\n", m_index, data);

	switch (offset)
	{
	case 0:
		m_index = data % data_size();
		break;

	case 1:
		switch (m_index)
		{
		case REG_SECONDS:
			// top bit of SECONDS is read only
			m_data[REG_SECONDS] = data & ~0x80;
			break;

		case REG_A:
			// top bit of A is read only
			m_data[REG_A] = data & ~REG_A_UIP;
			update_timer();
			break;

		case REG_B:
			if ((data & REG_B_SET) && !(m_data[REG_B] & REG_B_SET))
				data &= ~REG_B_UIE;

			m_data[REG_B] = data;
			update_irq();
			break;

		case REG_C:
		case REG_D:
			// register C & D is readonly
			break;

		default:
			m_data[m_index] = data;
			break;
		}
		break;
	}
}
