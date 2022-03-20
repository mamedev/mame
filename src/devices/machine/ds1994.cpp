// license:BSD-3-Clause
// copyright-holders:Aaron Giles, smf, Grull Osgo
/*
 * DS1994
 *
 * Dallas Semiconductor
 * 1-Wire Protocol
 * RTC + BACKUP RAM
 *
 */

// FIXME: convert to device_rtc_interface and remove time.h
// FIXME: convert logging to use logmacro.h

#include "emu.h"
#include "machine/ds1994.h"

#include <ctime>

#define VERBOSE_LEVEL 0

inline void ds1994_device::verboselog(int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start(v, s_fmt);
		vsprintf(buf, s_fmt, v);
		va_end(v);
		logerror("ds1994 %s: %s", machine().describe_context(), buf);
	}
}

// device type definition
DEFINE_DEVICE_TYPE(DS1994, ds1994_device, "ds1994", "DS1994 iButton 4Kb Memory Plus Time")

ds1994_device::ds1994_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DS1994, tag, owner, clock), device_nvram_interface(mconfig, *this)
	, m_timer_main(nullptr)
	, m_timer_reset(nullptr)
	, m_timer_clock(nullptr)
	, m_ref_year(0)
	, m_ref_month(0)
	, m_ref_day(0)
	, m_address(0)
	, m_offset(0)
	, m_a1(0)
	, m_a2(0)
	, m_bit(0)
	, m_shift(0)
	, m_byte(0)
	, m_rx(false)
	, m_tx(false)
	, m_state_ptr(0)
	, m_offs_ro(false)
{
	memset(m_ram, 0, sizeof(m_ram));
}

void ds1994_device::device_start()
{
	// Reference time setup
	struct tm ref_tm;

	memset(&ref_tm, 0, sizeof(ref_tm));
	ref_tm.tm_year = m_ref_year - 1900;
	ref_tm.tm_mon = m_ref_month - 1;
	ref_tm.tm_mday = m_ref_day;

	time_t ref_time = mktime(&ref_tm);

	time_t current_time;
	time(&current_time);
	current_time -= ref_time;

	m_rtc[0] = 0x0;
	m_rtc[1] = (current_time >> 0) & 0xff;
	m_rtc[2] = (current_time >> 8) & 0xff;
	m_rtc[3] = (current_time >> 16) & 0xff;
	m_rtc[4] = (current_time >> 24) & 0xff;

	// 1-wire timmings
	t_samp = attotime::from_usec(30);
	t_rdv  = attotime::from_usec(30);
	t_rstl = attotime::from_usec(480);
	t_pdh  = attotime::from_usec(30);
	t_pdl  = attotime::from_usec(120);

	// 1-wire states
	m_rx = true;
	m_tx = true;

	// 1-Wire related
	save_item(NAME(m_bit));
	save_item(NAME(m_byte));
	save_item(NAME(m_shift));
	save_item(NAME(m_rx));
	save_item(NAME(m_tx));

	// ds1994 specific
	save_item(NAME(m_rom));
	save_item(NAME(m_sram));
	save_item(NAME(m_ram));
	save_item(NAME(m_rtc));
	save_item(NAME(m_address));
	save_item(NAME(m_a1));
	save_item(NAME(m_a2));
	save_item(NAME(m_offset));

	// state machine
	save_item(NAME(m_state));
	save_item(NAME(m_state_ptr));
	for (auto & elem : m_state)
	elem = STATE_IDLE;

	// timers
	m_timer_main  = timer_alloc(TIMER_MAIN);
	m_timer_reset = timer_alloc(TIMER_RESET);
	m_timer_clock = timer_alloc(TIMER_CLOCK);

	m_timer_clock->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
}

void ds1994_device::device_reset()
{
	m_bit = 0;
	m_byte = 0;
	m_shift = 0;
	m_rx = true;
	m_tx = true;
	m_state_ptr = 0;
	m_state[m_state_ptr] = STATE_IDLE;

	memory_region *region = memregion(DEVICE_SELF);
	if (region != nullptr)
	{
		if (region->bytes() == ROM_SIZE + SPD_SIZE + DATA_SIZE + RTC_SIZE + REGS_SIZE)
		{
			memcpy(m_rom,  region->base(), ROM_SIZE);
			memcpy(m_ram,  region->base() + ROM_SIZE, SPD_SIZE);
			memcpy(m_sram, region->base() + ROM_SIZE + SPD_SIZE, DATA_SIZE);
			memcpy(m_rtc,  region->base() + ROM_SIZE + SPD_SIZE + DATA_SIZE, RTC_SIZE);
			memcpy(m_regs, region->base() + ROM_SIZE + SPD_SIZE + DATA_SIZE + RTC_SIZE, REGS_SIZE);
			return;
		}
		verboselog(0, "ds1994 %s: Wrong region length for data, expected 0x%x, got 0x%x\n", tag(), ROM_SIZE + SPD_SIZE + DATA_SIZE + RTC_SIZE + REGS_SIZE, region->bytes());
	}
	else
	{
		verboselog(0, "ds1994 %s: Warning, no id provided, answer will be all zeroes.\n", tag());
		memset(m_rom,  0, ROM_SIZE);
		memset(m_ram,  0, SPD_SIZE);
		memset(m_sram, 0, DATA_SIZE);
		memset(m_rtc,  0, RTC_SIZE);
		memset(m_regs, 0, REGS_SIZE);
	}
}

/********************************************/
/*                                          */
/*   1-wire protocol - Tx/Rx Bit Routines   */
/*                                          */
/********************************************/

bool ds1994_device::one_wire_tx_bit(uint8_t value)
{
	if (!m_bit)
	{
		m_shift = value;
		verboselog(1, "one_wire_tx_bit: Byte to send %02x\n", m_shift);
	}
	m_tx = m_shift & 1;
	m_shift >>= 1;
	verboselog(1, "one_wire_tx_bit: State %d\n", m_tx);
	m_bit++;
	if (m_bit == 8) return true;
	else
		return false;
}

bool ds1994_device::one_wire_rx_bit(void)
{
	m_shift >>= 1;
	if (m_rx)
	{
		m_shift |= 0x80;
	}
	verboselog(1, "one_wire_rx_bit: State %d\n", m_rx);
	m_bit++;
	if (m_bit == 8)
	{
		verboselog(1, "one_wire_rx_bit: Byte Received %02x\n", m_shift);
		return true;
	}
	 else
		return false;
}

/********************************************/
/*                                          */
/*   Internal states - Rom Commands         */
/*                                          */
/********************************************/

void ds1994_device::ds1994_rom_cmd(void)
{
	verboselog(2, "timer_main state_rom_command\n");
	if (one_wire_rx_bit())
	{
		switch (m_shift)
		{
			case ROMCMD_READROM:
				verboselog(1, "timer_main rom_cmd readrom\n");
				m_bit = 0;
				m_byte = 0;
				m_state[0] = STATE_READROM;
				m_state_ptr = 0;
				break;
			case ROMCMD_SKIPROM:
				verboselog(1, "timer_main rom_cmd skiprom\n");
				m_bit = 0;
				m_byte = 0;
				m_state[0] = STATE_COMMAND;
				m_state_ptr = 0;
				break;
			case ROMCMD_MATCHROM:
				verboselog(1, "timer_main rom_cmd matchrom\n");
				m_bit = 0;
				m_byte = 0;
				m_state[0] = STATE_MATCHROM;
				m_state_ptr = 0;
				break;
			case ROMCMD_SEARCHROM:
			case ROMCMD_SEARCHINT:
				verboselog(0, "timer_main rom_command not implemented %02x\n", m_shift);
				m_state[m_state_ptr] = STATE_COMMAND;
				break;
			default:
				verboselog(0, "timer_main rom_command not found %02x\n", m_shift);
				m_state[m_state_ptr] = STATE_IDLE;
				break;
		}
	}
}

/********************************************/
/*                                          */
/*   Internal states - DS1994 Commands      */
/*                                          */
/********************************************/

void ds1994_device::ds1994_cmd(void)
{
	verboselog(2, "timer_main state_command\n");
	if (one_wire_rx_bit())
	{
		switch (m_shift)
		{
			case COMMAND_READ_MEMORY:
				verboselog(1, "timer_main cmd read_memory\n");
				m_bit = 0;
				m_byte = 0;
				m_state[0] = STATE_ADDRESS1;
				m_state[1] = STATE_ADDRESS2;
				m_state[2] = STATE_INIT_COMMAND;
				m_state[3] = STATE_READ_MEMORY;
				m_state_ptr = 0;
				break;
			case COMMAND_WRITE_SCRATCHPAD:
				verboselog(1, "timer_main cmd write_scratchpad\n");
				m_bit = 0;
				m_byte = 0;
				m_offs_ro = false;
				m_state[0] = STATE_ADDRESS1;
				m_state[1] = STATE_ADDRESS2;
				m_state[2] = STATE_INIT_COMMAND;
				m_state[3] = STATE_WRITE_SCRATCHPAD;
				m_state_ptr = 0;
				break;
			case COMMAND_READ_SCRATCHPAD:
				verboselog(1, "timer_main cmd read_scratchpad\n");
				m_bit = 0;
				m_byte = 0;
				m_state[0] = STATE_TXADDRESS1;
				m_state[1] = STATE_TXADDRESS2;
				m_state[2] = STATE_TXOFFSET;
				m_state[3] = STATE_INIT_COMMAND;
				m_state[4] = STATE_READ_SCRATCHPAD;
				m_state_ptr = 0;
				break;
			case COMMAND_COPY_SCRATCHPAD:
				verboselog(1, "timer_main cmd copy_scratchpad\n");
				m_bit = 0;
				m_byte = 0;
				m_offs_ro = true;
				m_auth = true;
				m_state[0] = STATE_ADDRESS1;
				m_state[1] = STATE_ADDRESS2;
				m_state[2] = STATE_OFFSET;
				m_state[3] = STATE_INIT_COMMAND;
				m_state[4] = STATE_COPY_SCRATCHPAD;
				m_state_ptr = 0;
				break;
			default:
				verboselog(0, "timer_main command not handled %02x\n", m_shift);
				m_state[m_state_ptr] = STATE_IDLE;
				break;
		}
	}
}

/********************************************/
/*                                          */
/*   Internal Routines - Memory R/W         */
/*                                          */
/********************************************/

uint8_t ds1994_device::ds1994_readmem()
{
	if (m_address < 0x200)
	{
		return m_sram[m_address];
	}
	else
	{
		if (m_address >= 0x202 && m_address <= 0x206)
		{
			return m_rtc[m_address - 0x202];
		}
	}
	return 0;
}

void ds1994_device::ds1994_writemem(uint8_t value)
{
	if (m_address < 0x200)
	{
		m_sram[m_address] = value;
	}
	else
	{
		if (m_address >= 0x202 && m_address <= 0x206)
		{
			m_rtc[m_address - 0x202] = value;
		}
	}
}

/*************************************************/
/*                                               */
/*   Internal states - Timer controlled Events   */
/*                                               */
/*************************************************/

void ds1994_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_CLOCK:
			for (auto & elem : m_rtc)
			{
				elem++;
				if (elem != 0)
				{
					break;
				}
			}
			break;
		case TIMER_RESET:
			verboselog(1, "timer_reset\n");
			m_state[m_state_ptr] = STATE_RESET;
			m_timer_reset->adjust(attotime::never);
			break;
		case TIMER_MAIN:
			switch (m_state[m_state_ptr])
			{
				case STATE_RESET1:
					verboselog(2, "timer_main state_reset1 %d\n", m_rx);
					m_tx = false;
					m_state[m_state_ptr] = STATE_RESET2;
					m_timer_main->adjust(t_pdl);
					break;
				case STATE_RESET2:
					verboselog(2, "timer_main state_reset2 %d\n", m_rx);
					m_tx = true;
					m_bit = 0;
					m_shift = 0;
					m_state[m_state_ptr] = STATE_ROMCMD;
					break;
				case STATE_ROMCMD:
					ds1994_rom_cmd();
					break;
				case STATE_COMMAND:
					ds1994_cmd();
					break;
				case STATE_MATCHROM:
					verboselog(2, "timer_main state_matchrom - Data to match: <- %d\n", m_rx);
					if (one_wire_rx_bit())
					{
						if (m_rom[7- m_byte] == m_shift)
						{
							m_byte++;
							m_bit = 0;
							verboselog(2, "timer_main state_matchrom: datamatch %x - byte=%d\n", m_shift, m_byte);
						}
						else
						{
							m_state[m_state_ptr] = STATE_IDLE;
							verboselog(1, "timer_main state_matchrom: no match rx=%x <> mem=%x - byte:%d\n", m_shift, m_rom[7 - m_byte], m_byte);
						}
					}
					if (m_byte == ROM_SIZE)
					{
						verboselog(2, "timer_main matchrom finished\n");
						m_state[m_state_ptr] = STATE_COMMAND;
					}
					break;
				case STATE_ADDRESS1:
					verboselog(2, "timer_main state_address1\n");
					if (one_wire_rx_bit())
					{
						m_bit = 0;
						if (m_offs_ro)
						{
							if (m_a1 != m_shift) m_auth = false;
							verboselog(1, "timer_main state_address1 - TA1=%02x  - Auth_Code 1=%02x\n", m_a1, m_shift);
						}
						else
						{
							m_a1 = m_shift & 0xff;
							verboselog(2, "timer_main state_address1 - Address1=%02x\n", m_a1);
						}
						m_state_ptr++;
					}
					break;
				case STATE_ADDRESS2:
					verboselog(2, "timer_main state_address2\n");
					if (one_wire_rx_bit())
					{
						m_bit = 0;
						if (m_offs_ro)
						{
							if ( m_a2 != m_shift )
								m_auth = false;
							verboselog(1, "timer_main state_address1 - TA2=%02x  - Auth_Code 2=%02x\n", m_a1, m_shift);
						}
						else
						{
							m_a2 = m_shift & 0xff;
							verboselog(2, "timer_main state_address2 - Address2=%02x\n", m_a2);
						}
						m_state_ptr++;
					}
					break;
				case STATE_OFFSET:
					verboselog(2, "timer_main state_offset\n");
					if (one_wire_rx_bit())
					{
						m_bit = 0;
						if (m_offs_ro)
						{
							if (m_a2 != m_shift)
								m_auth = false;
							verboselog(1, "timer_main state_address1 - OFS_ES=%02x  - Auth_Code 3=%02x\n", m_offset, m_shift);
						}
						else
						{
							m_offset = m_shift & 0x1f;
							verboselog(2, "timer_main state_address2 - Offset=%02x\n", m_offset);
						}
						m_state_ptr++;
					}
					break;
				case STATE_WRITE_SCRATCHPAD:
					verboselog(2, "timer_main state_write_scratchpad\n");
					if (one_wire_rx_bit())
					{
						m_bit = 0;
						m_ram[m_offset & 0x1f] = m_shift & 0xff;
						m_offset++;
					}
					verboselog(2, "timer_main state_write_scratchpad %d Offs=%02x\n", m_rx, m_offset);
					break;
				case STATE_READROM:
					m_tx = true;
					if (m_byte == ROM_SIZE)
					{
						verboselog(1, "timer_main readrom finished\n");
						m_state[m_state_ptr] = STATE_COMMAND;
					}
					else
						verboselog(2, "timer_main readrom window closed\n");
					break;
				case STATE_TXADDRESS1:
					m_tx = true;
					if (m_byte == 1)
					{
						verboselog(1, "timer_main txaddress1 finished  m_byte=%d\n", m_byte);
						m_byte = 0;
						m_state_ptr++;
					}
					else
						verboselog(2, "timer_main txaddress1 window closed\n");
					break;
				case STATE_TXADDRESS2:
					m_tx = true;
					if (m_byte == 1)
					{
						verboselog(1, "timer_main txaddress2 finished m_byte=%d\n", m_byte);
						m_byte = 0;
						m_state_ptr++;
					}
					else
						verboselog(2, "timer_main txaddress2 window closed\n");
					break;
				case STATE_TXOFFSET:
					m_tx = true;
					if (m_byte == 1)
					{
						verboselog(1, "timer_main txoffset finished  - m_byte=%d\n", m_byte);
						m_byte = 0;
						m_state_ptr++;
					}
					else
						verboselog(2, "timer_main txoffset window closed\n");
					break;
				case STATE_READ_MEMORY:
					verboselog(2, "timer_main state_readmemory\n");
					break;
				case STATE_COPY_SCRATCHPAD:
					verboselog(2, "timer_main state_copy_scratchpad\n");
					break;
				case STATE_READ_SCRATCHPAD:
					verboselog(2, "timer_main state_read_scratchpad\n");
					break;
				default:
					verboselog(0, "timer_main state not handled: %d\n", m_state[m_state_ptr]);
					break;
			}
			if (m_state[m_state_ptr] == STATE_INIT_COMMAND)
			{
				switch (m_state[m_state_ptr + 1])
				{
					case STATE_IDLE:
					case STATE_COMMAND:
					case STATE_ADDRESS1:
					case STATE_ADDRESS2:
					case STATE_OFFSET:
						break;
					case STATE_READ_MEMORY:
						verboselog(2, "timer_main (init_cmd) -> state_read_memory - set address\n");
						m_address = (m_a2 << 8) | m_a1;
						break;
					case STATE_WRITE_SCRATCHPAD:
						verboselog(2, "timer_main (init_cmd) -> state_write_scratchpad - set address\n");
						m_offs_ro = false;
						m_offset = 0;
						break;
					case STATE_READ_SCRATCHPAD:
						verboselog(2, "timer_main (init_cmd) -> state_read_scratchpad - set address\n");
						m_address = 0;
						break;
					case STATE_COPY_SCRATCHPAD:
						verboselog(2, "timer_main (init_cmd) -> state_copy_scratchpad - do copy\n");
						if (m_auth)
						{
							m_address = (m_a2 << 8) | m_a1;
							for (int i = 0; i <= m_offset; i++)
							{
								ds1994_writemem(m_ram[i]);
								m_address++;
							}
						}
						else
							verboselog(1, "timer_main (init_cmd) -> state_copy_scratchpad - Auth-Rejected\n");
						break;
				}
				m_state_ptr++;
			}
	}
}

/*********************/
/*                   */
/*   Write Handler   */
/*                   */
/*********************/

WRITE_LINE_MEMBER(ds1994_device::write)
{
	verboselog(1, "write(%d)\n", state);
	if (!state && m_rx)
	{
		switch (m_state[m_state_ptr])
		{
			case STATE_IDLE:
			case STATE_INIT_COMMAND:
				break;
			case STATE_ROMCMD:
				verboselog(2, "state_romcommand\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_COMMAND:
				verboselog(2, "state_command\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_ADDRESS1:
				verboselog(2, "state_address1\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_ADDRESS2:
				verboselog(2, "state_address2\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_OFFSET:
				verboselog(2, "state_offset\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_TXADDRESS1:
				verboselog(2, "state_txaddress1\n");
				if (one_wire_tx_bit(m_a1))
				{
					m_bit = 0;
					m_byte++;
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_TXADDRESS2:
				verboselog(2, "state_txaddress2\n");
				if (one_wire_tx_bit(m_a2))
				{
					m_bit = 0;
					m_byte++;
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_TXOFFSET:
				verboselog(2, "state_txoffset\n");
				if (one_wire_tx_bit(m_offset))
				{
					m_bit = 0;
					m_byte++;
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_READROM:
				verboselog(2, "state_readrom\n");
				if (one_wire_tx_bit(m_rom[7 - m_byte]))
				{
					m_bit = 0;
					m_byte++;
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_READ_MEMORY:
				verboselog(2, "state_read_memory\n");
				if (one_wire_tx_bit(ds1994_readmem()))
				{
					m_bit = 0;
					if (m_address < DATA_SIZE + RTC_SIZE + REGS_SIZE)
						m_address++;
					else
						m_tx = true;
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_MATCHROM:
				verboselog(2, "state_matchrom\n");
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_COPY_SCRATCHPAD:
				if (m_auth)
				{
					verboselog(2, "state_copy_scratchpad Auth_Code Match: %d\n", m_tx);
					m_tx = true;
					m_auth = false;
				}
				else
				{
					m_tx = false;
					verboselog(1, "state_copy_scratchpad Auth_Code No Match: %d\n", m_tx);
				}
				m_timer_main->adjust(t_rdv);
				break;
			case STATE_WRITE_SCRATCHPAD:
				verboselog(2, "state_write_scratchpad\n");
				m_timer_main->adjust(t_samp);
				break;
			case STATE_READ_SCRATCHPAD:
				verboselog(2, "state_read_scratchpad\n");
				if (one_wire_tx_bit(m_ram[m_address]))
				{
					m_bit = 0;
					if (m_address <= m_offset)
						m_address++;
					else
						m_tx = true;
				}
				m_timer_main->adjust(t_rdv);
				break;
			default:
				verboselog(0, "state not handled: %d\n", m_state[m_state_ptr]);
				break;
		}
		m_timer_reset->adjust(t_rstl);
	}
	else
	if (state && !m_rx)
	{
		switch (m_state[m_state_ptr])
		{
		case STATE_RESET:
			m_state[m_state_ptr] = STATE_RESET1;
			m_timer_main->adjust(t_pdh);
			break;
		}
		m_timer_reset->adjust(attotime::never);
	}
	m_rx = state;
}

/*********************/
/*                   */
/*   Read  Handler   */
/*                   */
/*********************/

READ_LINE_MEMBER(ds1994_device::read)
{
	verboselog(2, "read %d\n", m_tx && m_rx);
	return m_tx && m_rx;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void ds1994_device::nvram_default()
{
	memset(m_rom,  0, ROM_SIZE);
	memset(m_ram,  0, SPD_SIZE);
	memset(m_sram, 0, DATA_SIZE);
	memset(m_rtc,  0, RTC_SIZE);
	memset(m_regs, 0, REGS_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool ds1994_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	bool result =      !file.read(m_rom,  ROM_SIZE,  actual) && actual == ROM_SIZE;
	result = result && !file.read(m_ram,  SPD_SIZE,  actual) && actual == SPD_SIZE;
	result = result && !file.read(m_sram, DATA_SIZE, actual) && actual == DATA_SIZE;
	result = result && !file.read(m_rtc,  RTC_SIZE,  actual) && actual == RTC_SIZE;
	result = result && !file.read(m_regs, REGS_SIZE, actual) && actual == REGS_SIZE;
	return result;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool ds1994_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	bool result =      !file.write(m_rom,  ROM_SIZE,  actual) && actual == ROM_SIZE;
	result = result && !file.write(m_ram,  SPD_SIZE,  actual) && actual == SPD_SIZE;
	result = result && !file.write(m_sram, DATA_SIZE, actual) && actual == DATA_SIZE;
	result = result && !file.write(m_rtc,  RTC_SIZE,  actual) && actual == RTC_SIZE;
	result = result && !file.write(m_regs, REGS_SIZE, actual) && actual == REGS_SIZE;
	return result;
}
