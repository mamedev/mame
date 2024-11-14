// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Dallas Semiconductor DS2430A 256-Bit 1-Wire EEPROM

    The EEPROM is organized as 32 bytes of storage, backed by a
    volatile scratchpad of identical size. The EEPROM itself is not
    addressible, but may be copied to the scratchpad quickly in its
    entirety by the "read memory" command. Copying the scratchpad to
    EEPROM is also a bulk operation but of course takes longer.

    The other addressable memory type in the DS2430A is a 64-bit
    (i.e. 8-byte) "application" register. This can only be programmed
    once, at which time two bits in the status register are zeroed
    permanently. It is likewise backed by a scratchpad area, which
    ceases to be readable once the register has been locked. (This
    register seems to be rarely used and is currently not fully
    emulated.)

    As with most Dallas 1-Wire devices (but not the original DS2430,
    which otherwise had the same organization but used a completely
    incompatible command set), each DS2430A also contains a factory-
    lasered ROM whose contents are an 8-bit device code (14h for this
    type), a unique 48-bit serial number and an 8-bit CRC of the
    preceding 56 bits.

    Unlike most other 1-Wire EEPROMs, the DS2430A memory commands use
    an 8-bit rather than 16-bit address for the EEPROM scratchpad and
    the application register. The addresses wrap continuously from
    1Fh or 07h to 00h for both read and write operations.

    DS1971 contains the same chip as DS2430A, but comes in a small
    round MicroCan package.

**********************************************************************/

#include "emu.h"
#include "ds2430a.h"

#include <numeric> // std::accumulate
#include <tuple> // std::tie

#define LOG_PULSE   (1U << 1)
#define LOG_BITS    (1U << 2)
#define LOG_STATE   (1U << 3)
#define LOG_DATA    (1U << 4)
#define LOG_COMMAND (1U << 5)

#define VERBOSE     (0)
#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(DS2430A, ds2430a_device, "ds2430a", "Dallas DS2430A 1-Wire EEPROM")
DEFINE_DEVICE_TYPE(DS1971, ds1971_device, "ds1971", "Dallas DS1971 EEPROM iButton")


//**************************************************************************
//  1-WIRE PROTOCOL IMPLEMENTATION
//**************************************************************************

// timing constants
static constexpr attoseconds_t tRSTL = 480 * ATTOSECONDS_PER_MICROSECOND; // 480 μs ≤ t < ∞
static constexpr attoseconds_t tRSTH = 480 * ATTOSECONDS_PER_MICROSECOND; // 480 μs ≤ t < ∞
static constexpr attoseconds_t tPDL = 120 * ATTOSECONDS_PER_MICROSECOND; // 60 μs ≤ t < 240 μs
static constexpr attoseconds_t tPDH = 16 * ATTOSECONDS_PER_MICROSECOND; // 15 μs ≤ t < 60 μs (but must exceed 15 μs for Konami Viper games)
static constexpr attoseconds_t tSLOT = 60 * ATTOSECONDS_PER_MICROSECOND; // 60 μs ≤ t < 120 μs
static constexpr attoseconds_t tREC = 1 * ATTOSECONDS_PER_MICROSECOND; // 1 μs ≤ t < ∞
static constexpr attoseconds_t tLOW0 = 60 * ATTOSECONDS_PER_MICROSECOND; // 60 μs ≤ t < 120 μs
static constexpr attoseconds_t tLOW1 = 15 * ATTOSECONDS_PER_MICROSECOND; // 1 μs ≤ t < 15 μs
static constexpr attoseconds_t tRELEASE = 30 * ATTOSECONDS_PER_MICROSECOND; // 1 μs ≤ t < 45 μs
static constexpr attoseconds_t tCOPY = 10 * ATTOSECONDS_PER_MILLISECOND;
static constexpr attoseconds_t tRDV = 15 * ATTOSECONDS_PER_MICROSECOND;
static constexpr attoseconds_t tSLOT_read = tRDV + tRELEASE + tREC; // timing loops may be tighter for reads than for writes?

static const char *const c_state_desc[] =
{
	"presence",
	"ROM command",
	"ROM read",
	"ROM match",
	"ROM search bit",
	"ROM search /bit",
	"ROM search write",
	"memory command",
	"memory read",
	"memory write",
	"memory copy",
	"done"
};

ALLOW_SAVE_TYPE(ds1wire_device::state)


//-------------------------------------------------
//  ds1wire_device - constructor
//-------------------------------------------------

ds1wire_device::ds1wire_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_timing_scale(1.0)
	, m_slot_timer(nullptr)
	, m_data_in(true) // idle state is high
	, m_data_out(true)
	, m_shift_data(0)
	, m_command(0)
	, m_bit_count(0)
	, m_pulse_start_time(attotime::zero)
	, m_current_state(state::DONE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds1wire_device::device_start()
{
	m_slot_timer = timer_alloc(FUNC(ds1wire_device::update_state), this);

	// save state
	save_item(NAME(m_data_in));
	save_item(NAME(m_data_out));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_command));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_pulse_start_time));
	save_item(NAME(m_current_state));
}


//-------------------------------------------------
//  data_r - read signal on data line
//-------------------------------------------------

int ds1wire_device::data_r()
{
	// Open drain output produces wired-AND signal
	return m_data_in && m_data_out;
}


//-------------------------------------------------
//  data_w - write bit to data line
//-------------------------------------------------

void ds1wire_device::data_w(int state)
{
	// Look for transitions
	if (m_data_in && !state)
	{
		m_data_in = false;
		pulse_start(machine().time());
	}
	else if (!m_data_in && state)
	{
		m_data_in = true;
		pulse_end(machine().time());
	}
}


//-------------------------------------------------
//  set_state - handle internal state changes
//-------------------------------------------------

bool ds1wire_device::set_state(ds1wire_device::state new_state)
{
	if (m_current_state != new_state)
	{
		LOGMASKED(LOG_STATE, "New state: %s\n", c_state_desc[int(new_state)]);
		m_current_state = new_state;
		m_bit_count = 0;
		return true;
	}

	return false;
}


//-------------------------------------------------
//  pulse_start - handle falling edge on data line
//-------------------------------------------------

void ds1wire_device::pulse_start(attotime time)
{
	if (m_pulse_start_time <= time)
	{
		m_pulse_start_time = time;
		LOGMASKED(LOG_PULSE, "Pulse started at %s\n", time.to_string());

		switch (m_current_state)
		{
		case state::MEMORY_READ:
		case state::ROM_READ:
		case state::ROM_SEARCH:
			if ((m_bit_count & 7) == 0)
			{
				if (m_current_state == state::MEMORY_READ)
				{
					m_shift_data = ds1wire_read_memory(m_command, m_bit_count >> 3);
					LOGMASKED(LOG_DATA, "%s: Master Rx byte %d = %02Xh (memory command %02Xh)\n", machine().describe_context(), m_bit_count >> 3, m_shift_data, m_command);
				}
				else
				{
					m_shift_data = ds1wire_read_rom(m_bit_count >> 3);
					LOGMASKED(LOG_DATA, "%s: Master Rx ROM byte %d = %02Xh\n", machine().describe_context(), m_bit_count >> 3, m_shift_data);
				}
			}
			LOGMASKED(LOG_BITS, "%s: Master Rx bit %d = %d\n", machine().describe_context(), m_bit_count & 7, BIT(m_shift_data, 0));
			if (!BIT(m_shift_data, 0))
			{
				m_data_out = false;
				m_slot_timer->adjust(scaled_time(tRELEASE));
			}
			break;

		case state::ROM_MATCH:
			if ((m_bit_count & 7) == 0)
				m_shift_data = ds1wire_read_rom(m_bit_count >> 3);
			break;

		case state::ROM_SEARCH_COMPLEMENT:
			LOGMASKED(LOG_BITS, "%s: Master Rx /bit %d = %d\n", machine().describe_context(), m_bit_count & 7, !BIT(m_shift_data, 0));
			if (BIT(m_shift_data, 0))
			{
				m_data_out = false;
				m_slot_timer->adjust(scaled_time(tRELEASE));
			}
			break;

		case state::MEMORY_COPY:
			logerror("Copy interrupted at %s\n", time.to_string());
			(void)set_state(state::DONE);
			break;

		case state::PRESENCE:
		case state::ROM_COMMAND:
		case state::MEMORY_COMMAND:
		case state::MEMORY_WRITE:
		case state::ROM_SEARCH_WRITE:
		case state::DONE:
			// Only compilers care about cases that do nothing
			break;
		}
	}
	else
		LOGMASKED(LOG_PULSE, "Pulse started too early at %s\n", time.to_string());
}


//-------------------------------------------------
//  pulse_end - handle rising edge on data line
//-------------------------------------------------

void ds1wire_device::pulse_end(attotime time)
{
	if (m_pulse_start_time < time)
	{
		// Measure pulse width
		attotime pulse_width = time - m_pulse_start_time;
		LOGMASKED(LOG_PULSE, "Pulse ended at %s (%d us measured width)\n", time.to_string(), int(pulse_width.as_double() * 1.0E6));
		pulse_width = attotime::from_double(pulse_width.as_double() / m_timing_scale);

		if (pulse_width >= attotime(0, tRSTL))
		{
			LOGMASKED(LOG_BITS, "%s: Master reset\n", machine().describe_context());
			(void)set_state(state::PRESENCE);
			m_slot_timer->adjust(scaled_time(tPDH));
			m_pulse_start_time = time + scaled_time(tRSTH);
		}
		else if (pulse_width < attotime(0, tREC))
			LOGMASKED(LOG_PULSE, "Pulse ended too early at %s\n", time.to_string());
		else switch (m_current_state)
		{
		case state::ROM_COMMAND:
		case state::MEMORY_COMMAND:
		case state::MEMORY_WRITE:
			m_shift_data >>= 1;
			if (pulse_width < attotime(0, tLOW0))
				m_shift_data |= 0x80;
			LOGMASKED(LOG_BITS, "%s: Master Tx bit %d = %d\n", machine().describe_context(), m_bit_count & 7, BIT(m_shift_data, 7));
			if ((m_bit_count & 7) == 7)
			{
				if ((m_bit_count >> 3) == 0 && (m_current_state == state::ROM_COMMAND || m_current_state == state::MEMORY_COMMAND))
				{
					LOGMASKED(LOG_COMMAND, "%s: Master Tx %s command = %02Xh\n", machine().describe_context(), m_current_state == state::ROM_COMMAND ? "ROM" : "memory", m_shift_data);
					m_command = m_shift_data;
				}
				else
					LOGMASKED(LOG_DATA, "%s: Master Tx byte %d = %02Xh\n", machine().describe_context(), m_bit_count >> 3, m_shift_data);
				state next_state = ds1wire_next_state(m_current_state, m_command, m_bit_count >> 3, m_shift_data);
				if (set_state(next_state))
				{
					if (next_state == state::MEMORY_COPY)
					{
						m_slot_timer->adjust(scaled_time(tCOPY));
						break;
					}
				}
				else
					++m_bit_count;
			}
			else
				++m_bit_count;
			m_pulse_start_time += scaled_time(tSLOT);
			break;

		case state::MEMORY_READ:
		case state::ROM_READ:
			m_shift_data >>= 1;
			if ((m_bit_count & 7) != 7 || !set_state(ds1wire_next_state(m_current_state, m_command, m_bit_count >> 3, 0)))
				++m_bit_count;
			m_pulse_start_time += scaled_time(tSLOT_read);
			break;

		case state::ROM_SEARCH:
			m_current_state = state::ROM_SEARCH_COMPLEMENT;
			m_pulse_start_time += scaled_time(tSLOT_read);
			break;

		case state::ROM_SEARCH_COMPLEMENT:
			m_current_state = state::ROM_SEARCH_WRITE;
			m_pulse_start_time += scaled_time(tSLOT_read);
			break;

		case state::ROM_MATCH:
		case state::ROM_SEARCH_WRITE:
			if ((pulse_width >= attotime(0, tLOW0)) == BIT(m_shift_data, 0))
			{
				LOGMASKED(LOG_BITS, "%s: Master Tx bit %d not matched; device deselected\n", machine().describe_context(), m_bit_count & 7);
				(void)set_state(state::DONE);
			}
			else
			{
				LOGMASKED(LOG_BITS, "%s: Master Tx bit %d = %d (matched)\n", machine().describe_context(), m_bit_count & 7, BIT(m_shift_data, 0));
				m_shift_data >>= 1;
				if ((m_bit_count & 7) != 7 || !set_state(ds1wire_next_state(m_current_state, m_command, m_bit_count >> 3, 0)))
				{
					++m_bit_count;
					if (m_current_state == state::ROM_SEARCH_WRITE)
						m_current_state = state::ROM_SEARCH;
				}
			}
			m_pulse_start_time += scaled_time(tSLOT);
			break;

		case state::PRESENCE:
		case state::MEMORY_COPY:
		case state::DONE:
			// Only compilers care about cases that do nothing
			break;
		}
	}
}


//-------------------------------------------------
//  update_state - generate timed responses to bus
//  transactions
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(ds1wire_device::update_state)
{
	switch (m_current_state)
	{
	case state::PRESENCE:
		m_data_out = !m_data_out;
		LOGMASKED(LOG_PULSE, "Presence pulse %sactive\n", m_data_out ? "in" : "");
		if (m_data_out)
			(void)set_state(state::ROM_COMMAND);
		else
			m_slot_timer->adjust(scaled_time(tPDL));
		break;

	case state::MEMORY_READ:
	case state::ROM_READ:
	case state::ROM_SEARCH:
	case state::ROM_SEARCH_COMPLEMENT:
	case state::DONE: // pull up data line after reading last bit
		m_data_out = true;
		break;

	case state::MEMORY_COPY:
		ds1wire_memory_copy(m_command);
		m_current_state = state::DONE;
		break;

	case state::ROM_COMMAND:
	case state::ROM_MATCH:
	case state::MEMORY_COMMAND:
	case state::MEMORY_WRITE:
	case state::ROM_SEARCH_WRITE:
		// Only compilers care about cases that do nothing
		break;
	}
}


//**************************************************************************
//  DEVICE EMULATION
//**************************************************************************

//-------------------------------------------------
//  ds2430a_device - constructor
//-------------------------------------------------

ds2430a_device::ds2430a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: ds1wire_device(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_default_data(*this, DEVICE_SELF)
	, m_start_address(0)
{
	std::fill(std::begin(m_scratchpad), std::end(m_scratchpad), 0);
	std::fill(std::begin(m_app_scratchpad), std::end(m_app_scratchpad), 0);
}

ds2430a_device::ds2430a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds2430a_device(mconfig, DS2430A, tag, owner, clock)
{
}


//-------------------------------------------------
//  ds1971_device - constructor
//-------------------------------------------------

ds1971_device::ds1971_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: ds2430a_device(mconfig, DS1971, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ds2430a_device::device_start()
{
	ds1wire_device::device_start();

	save_item(NAME(m_eeprom));
	save_item(NAME(m_scratchpad));
	save_item(NAME(m_app_scratchpad));
	save_item(NAME(m_start_address));
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  specified file
//-------------------------------------------------

bool ds2430a_device::nvram_read(util::read_stream &file)
{
	std::error_condition err;
	size_t actual;
	std::tie(err, actual) = read(file, &m_eeprom[0], 0x20);
	if (err || (0x20 != actual))
		return false;
	std::tie(err, actual) = read(file, &m_rom[0], 8);
	if (err || (8 != actual))
		return false;

	if (m_rom[0] != 0x14)
		osd_printf_error("Incorrect ROM family code (expected 14h, found %02Xh in saved data)\n", m_rom[0]);
	u8 const crc = std::accumulate(std::begin(m_rom), std::end(m_rom) - 1, u8(0), &ds1wire_crc);
	if (m_rom[7] != crc)
		osd_printf_error("Incorrect ROM CRC (expected %02Xh, found %02Xh in saved data)\n", crc, m_rom[7]);

	return true;
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  specified file
//-------------------------------------------------

bool ds2430a_device::nvram_write(util::write_stream &file)
{
	std::error_condition err;
	size_t actual;
	std::tie(err, actual) = write(file, &m_eeprom[0], 0x20);
	if (err)
		return false;
	std::tie(err, actual) = write(file, &m_rom[0], 8);
	if (err)
		return false;

	return true;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void ds2430a_device::nvram_default()
{
	if (m_default_data.found())
	{
		std::copy_n(&m_default_data[0], 0x20, &m_eeprom[0]);
		std::copy_n(&m_default_data[0x20], 8, &m_rom[0]);

		if (m_rom[0] != 0x14)
			osd_printf_error("Incorrect ROM family code (expected 14h, found %02Xh in default data)\n", m_rom[0]);
		u8 crc = std::accumulate(std::begin(m_rom), std::end(m_rom) - 1, u8(0), &ds1wire_crc);
		if (m_rom[7] != crc)
			osd_printf_error("Incorrect ROM CRC (expected %02Xh, found %02Xh in default data)\n", crc, m_rom[7]);
	}
	else
	{
		// Erase EEPROM to ones
		std::fill(std::begin(m_eeprom), std::end(m_eeprom), 0xff);

		// Make up a fake ID (shh, nobody alert the authorities)
		m_rom[0] = 0x14;
		m_rom[1] = 0x11;
		m_rom[2] = 0x22;
		m_rom[3] = 0x33;
		m_rom[4] = 0x44;
		m_rom[5] = 0x55;
		m_rom[6] = 0x66;
		m_rom[7] = std::accumulate(std::begin(m_rom), std::end(m_rom) - 1, u8(0), &ds1wire_crc);
	}
}


//-------------------------------------------------
//  ds1wire_next_state - handle memory byte writes
//  and state changes
//-------------------------------------------------

ds1wire_device::state ds2430a_device::ds1wire_next_state(ds1wire_device::state prev_state, u8 command, u16 index, u8 data)
{
	if (prev_state == state::ROM_COMMAND)
	{
		switch (command)
		{
		case 0x33: // Read ROM
			return state::ROM_READ;

		case 0x55: // Match ROM
			return state::ROM_MATCH;

		case 0xcc: // Skip ROM
			return state::MEMORY_COMMAND;

		case 0xf0: // Search ROM
			return state::ROM_SEARCH;

		default:
			return state::DONE;
		}
	}
	else if (prev_state == state::ROM_READ && index == 7)
		return state::DONE;
	else if ((prev_state == state::ROM_MATCH || prev_state == state::ROM_SEARCH_WRITE) && index == 7)
		return state::MEMORY_COMMAND;
	else if (prev_state == state::MEMORY_COMMAND)
	{
		switch (command)
		{
		case 0x0f: // Write Scratchpad
		case 0x99: // Write Application Register
			if (index == 1)
			{
				m_start_address = data;
				return state::MEMORY_READ;
			}
			else
				return state::MEMORY_COMMAND;

		case 0x55: // Copy Scratchpad
		case 0x5a: // Copy & Lock Application Register
			if (index == 1)
			{
				// Validation key is A5h
				if (data == 0xa5)
					return state::MEMORY_COPY;
				else
					return state::DONE;
			}
			else
				return state::MEMORY_COMMAND;

		case 0x66: // Read Status Register
			if (index == 1)
			{
				// Validation key is 00h
				if (data == 0x00)
					return state::MEMORY_READ;
				else
					return state::DONE;
			}
			else
				return state::MEMORY_COMMAND;

		case 0xf0: // Read Memory
			if (index == 0)
				std::copy(std::begin(m_eeprom), std::end(m_eeprom), std::begin(m_scratchpad));
			[[fallthrough]];

		case 0xaa: // Read Scratchpad
		case 0xc3: // Read Application Register
			if (index == 1)
			{
				m_start_address = data;
				return state::MEMORY_READ;
			}
			else
				return state::MEMORY_COMMAND;

		default:
			logerror("Unrecognized memory command %02Xh\n", command);
			return state::DONE;
		}
	}
	else if (prev_state == state::MEMORY_READ && command == 0x66)
	{
		// Status register does not wrap around
		return state::DONE;
	}
	else
	{
		if (prev_state == state::MEMORY_WRITE)
		{
			if (command == 0x0f)
				m_scratchpad[(m_start_address + index) & 0x1f] = data;
			else if (command == 0x99)
				m_app_scratchpad[(m_start_address + index) & 0x07] = data;
		}
		return prev_state;
	}
}


//-------------------------------------------------
//  ds1wire_read_rom - fetch one byte from ROM
//-------------------------------------------------

u8 ds2430a_device::ds1wire_read_rom(u16 index) const
{
	assert(index <= 7);
	return m_rom[index];
}


//-------------------------------------------------
//  ds1wire_read_memory - fetch one byte from the
//  selected memory area
//-------------------------------------------------

u8 ds2430a_device::ds1wire_read_memory(u8 command, u16 index) const
{
	switch (command)
	{
	case 0x66: // Read Status Register
		return 0xff; // TODO (only low 2 bits may be zero)

	case 0xaa: // Read Scratchpad
	case 0xf0: // Read Memory
		return m_scratchpad[(m_start_address + index) & 0x1f];

	case 0xc3: // Read Application Register
		return m_app_scratchpad[(m_start_address + index) & 0x07];

	default:
		return 0xff;
	}
}


//-------------------------------------------------
//  ds1wire_memory_copy - execute copy command
//  after a delay
//-------------------------------------------------

void ds2430a_device::ds1wire_memory_copy(u8 command)
{
	if (command == 0x55)
		std::copy(std::begin(m_scratchpad), std::end(m_scratchpad), std::begin(m_eeprom));
	else if (command == 0x5a)
		logerror("Copy scratchpad to application register (not supported)\n");
}
