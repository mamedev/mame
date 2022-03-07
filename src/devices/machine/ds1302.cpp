// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Dallas DS1302 Trickle-Charge Timekeeping Chip emulation

**********************************************************************/

/*

    TODO:

    - 12 hour format
    - synchronize user buffers on falling edge of CE after write

*/

#include "emu.h"
#include "ds1302.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

enum
{
	STATE_COMMAND,
	STATE_INPUT,
	STATE_OUTPUT
};

enum
{
	REGISTER_SECONDS = 0,
	REGISTER_MINUTES,
	REGISTER_HOUR,
	REGISTER_DATE,
	REGISTER_MONTH,
	REGISTER_DAY,
	REGISTER_YEAR,
	REGISTER_CONTROL,
	REGISTER_TRICKLE
};


#define COMMAND_READ    (m_cmd & 0x01)
#define COMMAND_RAM     (m_cmd & 0x40)
#define COMMAND_VALID   (m_cmd & 0x80)
#define COMMAND_BURST   (((m_cmd >> 1) & 0x1f) == 0x1f)
#define CLOCK_HALT      (m_reg[REGISTER_SECONDS] & 0x80)
#define WRITE_PROTECT   (m_reg[REGISTER_CONTROL] & 0x80)
#define BURST_END       (COMMAND_RAM ? m_ram_size : 0x09)



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(DS1202, ds1202_device, "ds1202", "Dallas DS1202 Serial Timekeeping Chip")
DEFINE_DEVICE_TYPE(DS1302, ds1302_device, "ds1302", "Dallas DS1302 Trickle-Charge Timekeeping Chip")


//-------------------------------------------------
//  ds1302_device - constructor
//-------------------------------------------------

ds1302_device::ds1302_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ram_size)
	: device_t(mconfig, type, tag, owner, clock),
		device_rtc_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_ram_size(ram_size)
{
}

ds1302_device::ds1302_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ds1302_device(mconfig, DS1302, tag, owner, clock, 0x1f)
{
}


//-------------------------------------------------
//  ds1202_device - constructor
//-------------------------------------------------

ds1202_device::ds1202_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ds1302_device(mconfig, DS1202, tag, owner, clock, 0x18)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds1302_device::device_start()
{
	// allocate timers
	m_clock_timer = timer_alloc();
	m_clock_timer->adjust(attotime::from_hz(clock() / 32768), 0, attotime::from_hz(clock() / 32768));

	m_clk = 0;
	m_ce = 0;
	m_state = STATE_COMMAND;
	m_bits = 0;
	m_cmd = 0;

	for (auto & elem : m_reg)
		elem = 0;

	// state saving
	save_item(NAME(m_ce));
	save_item(NAME(m_clk));
	save_item(NAME(m_io));
	save_item(NAME(m_state));
	save_item(NAME(m_bits));
	save_item(NAME(m_cmd));
	save_item(NAME(m_data));
	save_item(NAME(m_addr));
	save_item(NAME(m_reg));
	save_item(NAME(m_user));
	save_pointer(NAME(m_ram), m_ram_size);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void ds1302_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (!CLOCK_HALT)
	{
		advance_seconds();
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void ds1302_device::nvram_default()
{
	std::fill_n(&m_ram[0], m_ram_size, 0);
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool ds1302_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool ds1302_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_ram[0], m_ram_size, actual) && actual == m_ram_size;
}


//-------------------------------------------------
//  rtc_clock_updated -
//-------------------------------------------------

void ds1302_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_reg[REGISTER_YEAR] = convert_to_bcd(year);
	m_reg[REGISTER_DAY] = day_of_week;
	m_reg[REGISTER_MONTH] = convert_to_bcd(month);
	m_reg[REGISTER_DATE] = convert_to_bcd(day);
	m_reg[REGISTER_HOUR] = convert_to_bcd(hour);
	m_reg[REGISTER_MINUTES] = convert_to_bcd(minute);
	m_reg[REGISTER_SECONDS] = (m_reg[REGISTER_SECONDS] & 0x80) | convert_to_bcd(second);
}


//-------------------------------------------------
//  ce_w - chip enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds1302_device::ce_w )
{
	if (state && !m_ce)
	{
		LOG("Data Transfer Initiated\n");

		// synchronize user buffers
		for (int i = 0; i < 9; i++)
		{
			m_user[i] = m_reg[i];
		}
	}
	else if (!state && m_ce)
	{
		LOG("Data Transfer Terminated\n");

		// terminate data transfer
		m_state = STATE_COMMAND;
		m_bits = 0;
	}

	m_ce = state;
}


//-------------------------------------------------
//  load_shift_register -
//-------------------------------------------------

void ds1302_device::load_shift_register()
{
	if (COMMAND_READ)
	{
		if (COMMAND_RAM)
		{
			m_data = m_addr < m_ram_size ? m_ram[m_addr] : 0;

			LOG("Read RAM %u: %02x\n", m_addr, m_data);
		}
		else
		{
			m_data = m_addr < 9 ? m_user[m_addr] : 0;

			LOG("Read Clock %u: %02x\n", m_addr, m_data);
		}
	}
	else
	{
		if (COMMAND_RAM)
		{
			LOG("Write RAM %u: %02x\n", m_addr, m_data);

			if (m_addr < m_ram_size)
				m_ram[m_addr] = m_data;
		}
		else if (m_addr < 9)
		{
			LOG("Write Clock %u: %02x\n", m_addr, m_data);

			m_reg[m_addr] = m_data;
		}
	}
}


//-------------------------------------------------
//  input_bit -
//-------------------------------------------------

void ds1302_device::input_bit()
{
	switch (m_state)
	{
	case STATE_COMMAND:
		m_cmd >>= 1;
		m_cmd |= (m_io << 7);
		m_bits++;

		LOG("Serial Input Bit: %u\n", m_io);

		if (m_bits == 8)
		{
			LOG("Command Received: %02x\n", m_cmd);

			m_bits = 0;
			m_addr = (m_cmd >> 1) & 0x1f;

			if (COMMAND_VALID)
			{
				if (COMMAND_BURST)
				{
					m_addr = 0;
				}

				if (COMMAND_READ)
				{
					load_shift_register();

					m_state = STATE_OUTPUT;
				}
				else
				{
					m_state = STATE_INPUT;
				}
			}
			else
			{
				m_state = STATE_COMMAND;
			}
		}
		break;

	case STATE_INPUT:
		m_data >>= 1;
		m_data |= (m_io << 7);
		m_bits++;

		LOG("Serial Input Bit: %u\n", m_io);

		if (m_bits == 8)
		{
			LOG("Data Received: %02x\n", m_data);

			m_bits = 0;

			if (!WRITE_PROTECT)
			{
				load_shift_register();
			}

			if (COMMAND_BURST)
			{
				m_addr++;

				if (m_addr == BURST_END)
				{
					m_state = STATE_COMMAND;
				}
			}
			else
			{
				m_state = STATE_COMMAND;
			}
		}
		break;
	}
}


//-------------------------------------------------
//  output_bit -
//-------------------------------------------------

void ds1302_device::output_bit()
{
	if (m_state != STATE_OUTPUT) return;

	m_io = BIT(m_data, 0);
	m_data >>= 1;
	m_bits++;

	if (m_bits == 8)
	{
		m_bits = 0;

		if (COMMAND_BURST)
		{
			m_addr++;

			if (m_addr == BURST_END)
			{
				m_state = STATE_COMMAND;
			}
			else
			{
				load_shift_register();
			}
		}
		else
		{
			m_state = STATE_COMMAND;
		}
	}

	LOG("Serial Output Bit: %u\n", m_io);
}


//-------------------------------------------------
//  sclk_w - serial clock write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds1302_device::sclk_w )
{
	//LOG("Serial CLK: %u\n", state);

	if (m_ce)
	{
		if (!m_clk && state) // rising edge
			input_bit();
		else if (m_clk && !state) // falling edge
			output_bit();
	}

	m_clk = state;
}


//-------------------------------------------------
//  io_w - I/O write
//-------------------------------------------------

WRITE_LINE_MEMBER( ds1302_device::io_w )
{
	//LOG("Serial I/O: %u\n", state);

	m_io = state;
}


//-------------------------------------------------
//  io_r - I/O read
//-------------------------------------------------

READ_LINE_MEMBER( ds1302_device::io_r )
{
	return m_io;
}
