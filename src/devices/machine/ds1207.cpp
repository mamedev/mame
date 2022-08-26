// license:BSD-3-Clause
// copyright-holders:Paul-Arnold
/*
 * ds1207.c
 *
 * Time Key
 *
 * Based on ds1204 by smf.
 *
 * File format is as follows:
 * 00-01 unique command pattern
 * 02-09 identification pattern
 * 0a-11 security match
 * 12-41 secure memory data
 * 42-43 days left
 * 44-4b start time (from time_t)
 * 4c    control
 *
 * Control bits:
 * bit 0 - OSC ENABLED
 * bit 1 - OSC RUNNING
 * bit 2 - DAYS LOCKED
 * bit 3 - DAYS EXPIRED
 *
 * The unique command pattern can be user specific but a number of off the shelf devices exist.
 * For these devices the pattern should be as follows:
 *   DS1207-G01 0x00 0xb0
 *   DS1207-G02 0x04 0xb0
 *   DS1207-G03 0x08 0xb0
 *   DS1207-G04 0x0c 0xb0
 *   DS1207-G05 0x10 0xb0
 */
#include "emu.h"
#include "ds1207.h"

#define LOG_LINES    (1U << 1)
#define LOG_STATE    (1U << 2)
#define LOG_DATA     (1U << 3)

//#define VERBOSE (LOG_LINES | LOG_STATE | LOG_DATA)
#include "logmacro.h"

#define LOGLINES(...)    LOGMASKED(LOG_LINES, __VA_ARGS__)
#define LOGSTATE(...)    LOGMASKED(LOG_STATE, __VA_ARGS__)
#define LOGDATA(...)     LOGMASKED(LOG_DATA, __VA_ARGS__)

// device type definition
DEFINE_DEVICE_TYPE(DS1207, ds1207_device, "ds1207", "DS1207 Time Key")

ds1207_device::ds1207_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DS1207, tag, owner, clock),
	  device_nvram_interface(mconfig, *this),
	  device_rtc_interface(mconfig, *this),
	  m_region(*this, DEVICE_SELF),
	  m_rst(0),
	  m_clk(0),
	  m_dqw(0), m_dqr(0), m_state(0), m_bit(0)
{
}

void ds1207_device::device_reset()
{
	adjust_days_left(); // compensate for time machine has been turned off
}

void ds1207_device::device_start()
{
	new_state(STATE_STOP);
	m_dqr = DQ_HIGH_IMPEDANCE;

	std::fill_n(m_command, std::size(m_command), 0);
	std::fill_n(m_compare_register, std::size(m_compare_register), 0);
	m_last_update_time = 0;
	m_startup_time = 0;
	std::fill_n(m_day_clock, std::size(m_day_clock), 0);

	save_item(NAME(m_rst));
	save_item(NAME(m_clk));
	save_item(NAME(m_dqw));
	save_item(NAME(m_dqr));
	save_item(NAME(m_state));
	save_item(NAME(m_bit));
	save_item(NAME(m_command));
	save_item(NAME(m_compare_register));
	save_item(NAME(m_unique_pattern));
	save_item(NAME(m_identification));
	save_item(NAME(m_security_match));
	save_item(NAME(m_secure_memory));
	save_item(NAME(m_day_clock));
	save_item(NAME(m_days_left));
	save_item(NAME(m_start_time));
	save_item(NAME(m_device_state));
	save_item(NAME(m_startup_time));
	save_item(NAME(m_last_update_time));
}

void ds1207_device::nvram_default()
{
	std::fill_n(m_unique_pattern, std::size(m_unique_pattern), 0);
	std::fill_n(m_identification, std::size(m_identification), 0);
	std::fill_n(m_security_match, std::size(m_security_match), 0);
	std::fill_n(m_secure_memory, std::size(m_secure_memory), 0);
	std::fill_n(m_days_left, std::size(m_days_left), 0);
	std::fill_n(m_start_time, std::size(m_start_time), 0);
	m_device_state = 0;

	int expected_bytes = sizeof(m_unique_pattern) + sizeof(m_identification) + sizeof(m_security_match) + sizeof(m_secure_memory)
	                     + sizeof(m_days_left) + sizeof(m_start_time) + sizeof(m_device_state);

	if(!m_region.found())
	{
		logerror("ds1207(%s) region not found\n", tag());
	}
	else if(m_region->bytes() != expected_bytes)
	{
		logerror("ds1207(%s) region length 0x%x expected 0x%x\n", tag(), m_region->bytes(), expected_bytes);
	}
	else
	{
		uint8_t *region = m_region->base();

		memcpy(m_unique_pattern, region, sizeof(m_unique_pattern));
		region += sizeof(m_unique_pattern);
		memcpy(m_identification, region, sizeof(m_identification));
		region += sizeof(m_identification);
		memcpy(m_security_match, region, sizeof(m_security_match));
		region += sizeof(m_security_match);
		memcpy(m_secure_memory, region, sizeof(m_secure_memory));
		region += sizeof(m_secure_memory);
		memcpy(m_days_left, region, sizeof(m_days_left));
		region += sizeof(m_days_left);
		memcpy(m_start_time, region, sizeof(m_start_time));
		region += sizeof(m_start_time);
		memcpy(&m_device_state, region, sizeof(m_device_state));
		region += sizeof(m_device_state);
	}
}

bool ds1207_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	bool result = !file.read(m_unique_pattern, sizeof(m_unique_pattern), actual) && actual == sizeof(m_unique_pattern);
	result = result && !file.read(m_identification, sizeof(m_identification), actual) && actual == sizeof(m_identification);
	result = result && !file.read(m_security_match, sizeof(m_security_match), actual) && actual == sizeof(m_security_match);
	result = result && !file.read(m_secure_memory, sizeof(m_secure_memory), actual) && actual == sizeof(m_secure_memory);
	result = result && !file.read(m_days_left, sizeof(m_days_left), actual) && actual == sizeof(m_days_left);
	result = result && !file.read(m_start_time, sizeof(m_start_time), actual) && actual == sizeof(m_start_time);
	result = result && !file.read(&m_device_state, sizeof(m_device_state), actual) && actual == sizeof(m_device_state);
	return result;
}

bool ds1207_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	bool result = !file.write(m_unique_pattern, sizeof(m_unique_pattern), actual) && actual == sizeof(m_unique_pattern);
	result = result && !file.write(m_identification, sizeof(m_identification), actual) && actual == sizeof(m_identification);
	result = result && !file.write(m_security_match, sizeof(m_security_match), actual) && actual == sizeof(m_security_match);
	result = result && !file.write(m_secure_memory, sizeof(m_secure_memory), actual) && actual == sizeof(m_secure_memory);
	result = result && !file.write(m_days_left, sizeof(m_days_left), actual) && actual == sizeof(m_days_left);
	result = result && !file.write(m_start_time, sizeof(m_start_time), actual) && actual == sizeof(m_start_time);
	result = result && !file.write(&m_device_state, sizeof(m_device_state), actual) && actual == sizeof(m_device_state);
	return result;
}

void ds1207_device::new_state(uint8_t state)
{
	m_state = state;
	m_bit = 0;
}

void ds1207_device::writebit(uint8_t *buffer)
{
	if(m_clk)
	{
		uint16_t index = m_bit / 8;
		uint8_t mask = 1 << (m_bit % 8);

		if(m_dqw)
		{
			buffer[ index ] |= mask;
		}
		else
		{
			buffer[ index ] &= ~mask;
		}

		m_bit++;
	}
}

void ds1207_device::readbit(uint8_t *buffer)
{
	if(!m_clk)
	{
		uint16_t index = m_bit / 8;
		uint8_t mask = 1 << (m_bit % 8);

		if(buffer[ index ] & mask)
		{
			m_dqr = 1;
		}
		else
		{
			m_dqr = 0;
		}
	}
	else
	{
		m_bit++;
	}
}

WRITE_LINE_MEMBER(ds1207_device::write_rst)
{
	const uint8_t this_state = state ? 1 : 0;
	if(m_rst != this_state)
	{
		m_rst = this_state;
		LOGLINES("%s: DS1270 rst=%d\n", machine().describe_context(), m_rst);

		if(m_rst)
		{
			new_state(STATE_PROTOCOL);
		}
		else
		{
			switch(m_state)
			{
				case STATE_WRITE_IDENTIFICATION:
					LOGSTATE("%s: DS1270 reset during write identification (bit=%u)\n", machine().describe_context(), m_bit);
					break;

				case STATE_WRITE_SECURITY_MATCH:
					LOGSTATE("%s: DS1270 reset during write security match (bit=%u)\n", machine().describe_context(), m_bit);
					break;

				case STATE_WRITE_SECURE_MEMORY:
					LOGSTATE("%s: DS1270 reset during write secure memory (bit=%u)\n", machine().describe_context(), m_bit);
					break;
			}

			new_state(STATE_STOP);
			m_dqr = DQ_HIGH_IMPEDANCE;
		}
	}
}

WRITE_LINE_MEMBER(ds1207_device::write_clk)
{
	const uint8_t this_state = state ? 1 : 0;
	if(m_clk != this_state)
	{
		m_clk = this_state;
		LOGLINES("%s: DS1270 clk=%d (bit=%u)\n", machine().describe_context(), m_clk, m_bit);

		if(m_clk)
		{
			m_dqr = DQ_HIGH_IMPEDANCE;
		}

		switch(m_state)
		{
			case STATE_PROTOCOL:
				writebit(m_command);

				if(m_bit == 24)
				{
					LOGDATA("%s: DS1270 -> command %02x %02x %02x (%02x %02x)\n", machine().describe_context(),
					           m_command[ 0 ], m_command[ 1 ], m_command[ 2 ], m_unique_pattern[ 0 ], m_unique_pattern[ 1 ]);
							   
					if(m_command[ 2 ] == m_unique_pattern[ 1 ] && (m_command[ 1 ] & ~3) == m_unique_pattern[ 0 ])
					{
						set_start_time();
						adjust_time_into_day();

						if(m_command[ 0 ] == COMMAND_READ && (m_command[ 1 ] & 3) == CYCLE_NORMAL)
						{
							new_state(STATE_READ_IDENTIFICATION);
						}
						else if(m_command[ 0 ] == COMMAND_READ_DAY_CLOCK && (m_command[ 1 ] & 3) == CYCLE_PROGRAM)
						{
							new_state(STATE_READ_DAY_CLOCK);
						}
						else if(m_command[ 0 ] == COMMAND_READ_DAYS_REMAINING && (m_command[ 1 ] & 3) == CYCLE_PROGRAM)
						{
							new_state(STATE_READ_DAYS_REMAINING);
						}
						else if(!(m_device_state & DAYS_EXPIRED))
						{
							if(m_command[ 0 ] == COMMAND_WRITE && (m_command[ 1 ] & 3) == CYCLE_NORMAL)
							{
								new_state(STATE_READ_IDENTIFICATION);
							}
							else if((m_command[ 1 ] & 3) == CYCLE_PROGRAM)
							{
								if(m_command[ 0 ] == COMMAND_WRITE)
								{
									new_state(STATE_WRITE_IDENTIFICATION);
								}
								else if(m_command[ 0 ] == COMMAND_WRITE_DAYS_REMAINING)
								{
									if(!(m_device_state & DAYS_LOCKED))
									{
										new_state(STATE_WRITE_DAYS_REMAINING);
									}
									else
									{
										new_state(STATE_STOP);
									}
								}
								else if(m_command[ 0 ] == COMMAND_LOCK_DAYS_COUNT)
								{
									m_device_state |= DAYS_LOCKED;

									new_state(STATE_STOP);
								}
								else if(m_command[ 0 ] == COMMAND_STOP_OSCILLATOR)
								{
									if(!(m_device_state & DAYS_LOCKED))
									{
										m_device_state &= ~(OSC_ENABLED | OSC_RUNNING);
									}

									new_state(STATE_STOP);
								}
								else if(m_command[ 0 ] == COMMAND_ARM_OSCILLATOR)
								{
									m_device_state |= OSC_ENABLED;
								}
								else
								{
									new_state(STATE_STOP);
								}
							}
							else
							{
								new_state(STATE_STOP);
							}
						}
						else
						{
							new_state(STATE_STOP);
						}
					}
					else
					{
						new_state(STATE_STOP);
					}
				}
				break;

			case STATE_READ_IDENTIFICATION:
				readbit(m_identification);

				if(m_bit == 64)
				{
					LOGDATA("%s: DS1270 <- identification %02x %02x %02x %02x %02x %02x %02x %02x\n", machine().describe_context(),
					           m_identification[ 0 ], m_identification[ 1 ], m_identification[ 2 ], m_identification[ 3 ],
					           m_identification[ 4 ], m_identification[ 5 ], m_identification[ 6 ], m_identification[ 7 ]);

					new_state(STATE_WRITE_COMPARE_REGISTER);
				}
				break;

			case STATE_WRITE_COMPARE_REGISTER:
				writebit(m_compare_register);

				if(m_bit == 64)
				{
					LOGDATA("%s: DS1207 -> compare register %02x %02x %02x %02x %02x %02x %02x %02x (%02x %02x %02x %02x %02x %02x %02x %02x)\n", machine().describe_context(),
					           m_compare_register[ 0 ], m_compare_register[ 1 ], m_compare_register[ 2 ], m_compare_register[ 3 ],
					           m_compare_register[ 4 ], m_compare_register[ 5 ], m_compare_register[ 6 ], m_compare_register[ 7 ],
					           m_security_match[ 0 ], m_security_match[ 1 ], m_security_match[ 2 ], m_security_match[ 3 ],
					           m_security_match[ 4 ], m_security_match[ 5 ], m_security_match[ 6 ], m_security_match[ 7 ]);

					if(memcmp(m_compare_register, m_security_match, sizeof(m_compare_register)) == 0)
					{
						if(m_command[ 0 ] == COMMAND_READ)
						{
							new_state(STATE_READ_SECURE_MEMORY);
						}
						else
						{
							new_state(STATE_WRITE_SECURE_MEMORY);
						}
					}
					else
					{
						new_state(STATE_OUTPUT_GARBLED_DATA);
					}
				}
				break;

			case STATE_READ_SECURE_MEMORY:
				readbit(m_secure_memory);

				if(m_bit == 384)
				{
					new_state(STATE_STOP);
				}
				break;

			case STATE_WRITE_SECURE_MEMORY:
				writebit(m_secure_memory);

				if(m_bit == 384)
				{
					new_state(STATE_STOP);
				}
				break;

			case STATE_WRITE_IDENTIFICATION:
				writebit(m_identification);

				if(m_bit == 64)
				{
					LOGDATA("%s: DS1207 -> identification %02x %02x %02x %02x %02x %02x %02x %02x\n", machine().describe_context(),
					           m_identification[ 0 ], m_identification[ 1 ], m_identification[ 2 ], m_identification[ 3 ],
					           m_identification[ 4 ], m_identification[ 5 ], m_identification[ 6 ], m_identification[ 7 ]);

					new_state(STATE_WRITE_SECURITY_MATCH);
				}
				break;

			case STATE_WRITE_SECURITY_MATCH:
				writebit(m_security_match);

				if(m_bit == 64)
				{
					LOGDATA("%s: DS1207 >- security match %02x %02x %02x %02x %02x %02x %02x %02x\n", machine().describe_context(),
					           m_security_match[ 0 ], m_security_match[ 1 ], m_security_match[ 2 ], m_security_match[ 3 ],
					           m_security_match[ 4 ], m_security_match[ 5 ], m_security_match[ 6 ], m_security_match[ 7 ]);

					new_state(STATE_STOP);
				}
				break;

			case STATE_OUTPUT_GARBLED_DATA:
				if(!m_clk && m_command[ 0 ] == COMMAND_READ)
				{
					m_dqr = machine().rand() & 1;
					m_bit++;
				}
				else if(m_clk && m_command[ 0 ] == COMMAND_WRITE)
				{
					m_bit++;
				}

				if(m_bit == 64)
				{
					if(m_command[ 0 ] == COMMAND_READ)
					{
						LOGDATA("%s: DS1207 <- random\n", machine().describe_context());
					}
					else
					{
						LOGDATA("%s: DS1207 -> ignore\n", machine().describe_context());
					}

					new_state(STATE_STOP);
				}
				break;

			case STATE_READ_DAY_CLOCK:
				readbit(m_day_clock);

				if(m_bit == 20)
				{
					new_state(STATE_STOP);
				}
				break;

			case STATE_READ_DAYS_REMAINING:
				readbit(m_days_left);

				if(m_bit == 9)
				{
					new_state(STATE_STOP);
				}
				break;

			case STATE_WRITE_DAYS_REMAINING:
				writebit(m_days_left);

				if(m_bit == 9)
				{
					new_state(STATE_STOP);
				}
				break;
		}
	}
}

WRITE_LINE_MEMBER(ds1207_device::write_dq)
{
	const uint8_t this_state = state ? 1 : 0;
	if(m_dqw != this_state)
	{
		m_dqw = this_state;

		LOGLINES("%s: DS1270 dqw=%u\n", machine().describe_context(), m_dqw);
	}
}

READ_LINE_MEMBER(ds1207_device::read_dq)
{
	if(m_dqr == DQ_HIGH_IMPEDANCE)
	{
		LOGLINES("%s: DS1270 dqr=high impedance\n", machine().describe_context());
		return 0;
	}

	LOGLINES("%s: DS1270 dqr=%d (bit=%u)\n", machine().describe_context(), m_dqr, m_bit);
	return m_dqr;
}

void ds1207_device::adjust_time_into_day()
{
	if(!(m_device_state & DAYS_EXPIRED) && (m_device_state & OSC_ENABLED) && (m_device_state & OSC_RUNNING))
	{
		uint64_t day_clock = ((uint64_t)m_day_clock[ 0 ]) | (((uint64_t)m_day_clock[ 1 ]) << 8) | (((uint64_t)m_day_clock[ 2 ]) << 16);
		
		const uint64_t cur_time = machine().time().as_ticks(32768) / 2700;
		const uint64_t diff_time = cur_time - m_last_update_time;
		m_last_update_time = cur_time;

		day_clock += diff_time;

		m_day_clock[ 0 ] = day_clock & 0xff;
		m_day_clock[ 1 ] = (day_clock >> 8) & 0xff;
		m_day_clock[ 2 ] = (day_clock >> 16) & 0xff;

		if(day_clock >= 1048576)
		{
			adjust_days_left();
		}
	}
}

void ds1207_device::adjust_days_left()
{
	if(!(m_device_state & DAYS_EXPIRED) && (m_device_state & OSC_ENABLED) && (m_device_state & OSC_RUNNING))
	{
		const uint64_t current_time = m_startup_time + machine().time().as_ticks(1);

		uint64_t start_time = 0;

		for(int i = 0; i < 8 ; i++)
		{
			start_time <<= 8;
			start_time |= m_start_time[ 7 - i ];
		}

		if(current_time > start_time)
		{
			uint64_t time_diff = current_time - start_time;

			const uint16_t days_elapsed = time_diff / (24*60*60);

			time_diff %= (24*60*60);// seconds into day

			const uint32_t day_clock = (time_diff * 32768)/2700;// time into day

			m_day_clock[ 0 ] = day_clock & 0xff;
			m_day_clock[ 1 ] = (day_clock >> 8) & 0xff;
			m_day_clock[ 2 ] = (day_clock >> 16) & 0xff;

			if(days_elapsed > 0)
			{
				uint16_t days_left = m_days_left[ 0 ] | (m_days_left[ 1 ] << 8);

				if(days_elapsed > days_left)
				{
					days_left = 0xffff;
					m_device_state |= DAYS_EXPIRED;
				}
				else
				{
					days_left -= days_elapsed;
				}

				m_days_left[ 0 ] = days_left & 0xff;
				m_days_left[ 1 ] = (days_left >> 8) & 0x1;

				start_time += days_elapsed * 24 * 60 * 60;

				for(int i = 0; i < 8 ; i++)
				{
					m_start_time[ i ] = (start_time >> (i * 8)) & 0xff;
				}
			}
		}
	}
}

void ds1207_device::set_start_time()
{
	if(!(m_device_state & DAYS_EXPIRED) && m_device_state & OSC_ENABLED && !(m_device_state & OSC_RUNNING))
	{
		const uint64_t current_time = m_startup_time + machine().time().as_ticks(1);

		for(int i = 0; i < 8 ; i++)
		{
			m_start_time[ i ] = (current_time >> (i * 8)) & 0xff;
		}
		m_day_clock [ 0 ] = m_day_clock[ 1 ] = m_day_clock[ 2 ] = 0;

		m_device_state |= OSC_RUNNING;
	}
}

void ds1207_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	const int month_to_day_conversion[ 12 ] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	// put the seconds
	m_startup_time = second;

	// put the minutes
	m_startup_time += minute * 60;

	// put the hours
	m_startup_time += hour * 60 * 60;

	// put the days (note -1) */
	m_startup_time += (day - 1) * 60 * 60 * 24;

	// take the months - despite popular beliefs, leap years aren't just evenly divisible by 4 */
	if(((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0)) && month > 2)
	{
		m_startup_time += (month_to_day_conversion[ month - 1 ] + 1) * 60 * 60 * 24;
	}
	else
	{
		m_startup_time += (month_to_day_conversion[ month - 1 ]) * 60 * 60 * 24;
	}

	// put the years
	int year_count = (year - 1969);

	for(int i = 0; i < year_count - 1 ; i++)
	{
		m_startup_time += (((((i+1970) % 4) == 0) && (((i+1970) % 100) != 0)) || (((i+1970) % 400) == 0)) ? 60*60*24*366 : 60*60*24*365;
	}
}

