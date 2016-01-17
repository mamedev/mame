// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    eepromser.c

    Serial EEPROM devices.

****************************************************************************

    Serial EEPROMs generally work the same across manufacturers and models,
    varying largely by the size of the EEPROM and the packaging details.

    At a basic level, there are 5 signals involved:

        * CS = chip select
        * CLK = serial data clock
        * DI = serial data in
        * DO = serial data out
        * RDY/BUSY = ready (1) or busy (0) status

    Data is read or written via serial commands. A command is begun on a
    low-to-high transition of the CS line, following by clocking a start
    bit (1) on the DI line. After the start bit, subsequent clocks
    assemble one of the following commands:

        Start   Opcode  Address     Data
          1       01    aaaaaaaaa   ddddddd     WRITE data
          1       10    aaaaaaaaa               READ data
          1       11    aaaaaaaaa               ERASE data
          1       00    00xxxxxxx               WREN = WRite ENable
          1       00    01xxxxxxx   ddddddd     WRAL = WRite ALl cells
          1       00    10xxxxxxx               ERAL = ERase ALl cells
          1       00    11xxxxxxx               WRDS = WRite DiSable

    The number of address bits (a) clocked varies based on the size of the
    chip, though it does not always map 1:1 with the size of the chip.
    For example, the 93C06 has 16 cells, which only needs 4 address bits;
    but commands to the 93C06 require 6 address bits (the top two must
    be 0).

    The number of data bits (d) clocked varies based on the chip and at
    times on the state of a pin on the chip which selects between multiple
    sizes (e.g., 8-bit versus 16-bit).

****************************************************************************

    Most EEPROMs are based on the 93Cxx design (and have similar part
    designations):

                                +--v--+
                             CS |1   8| Vcc
                            CLK |2   7| NC
                             DI |3   6| NC
                             DO |4   5| GND
                                +-----+

    Note the lack of a READY/BUSY pin. On the 93Cxx series, the DO pin
    serves double-duty, returning READY/BUSY during a write/erase cycle,
    and outputting data during a read cycle.

    Some manufacturers have released "enhanced" versions with additional
    features:

        * Several manufacturers (ST) map pin 6 to "ORG", specifying the
          logical organization of the data. Connecting ORG to ground
          makes the EEPROM work as an 8-bit device, while connecting it
          to Vcc makes it work as a 16-bit device with one less
          address bit.

        * Other manufacturers (ST) have enhanced the read operations to
          allow serially streaming more than one cell. Essentially, after
          reading the first cell, keep CS high and keep clocking, and
          data from following cells will be read as well.

    The ER5911 is only slightly different:

                                +--v--+
                             CS |1   8| Vcc
                            CLK |2   7| RDY/BUSY
                             DI |3   6| ORG
                             DO |4   5| GND
                                +-----+

    Here we have an explicit RDY/BUSY signal, and the ORG flag as described
    above.

    From a command perspective, the ER5911 is also slightly different:

        93Cxx has ERASE command; this maps to WRITE on ER5911
        93Cxx has WRITEALL command; no equivalent on ER5911

****************************************************************************

    Issues with:

    kickgoal.c - code seems wrong, clock logic writes 0-0-0 instead of 0-1-0 as expected
    overdriv.c - drops CS, raises CS, keeps DI=1, triggering extraneous start bit

***************************************************************************/

#include "emu.h"
#include "machine/eepromser.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

// logging levels:
//  0 = errors and warnings only
//  1 = commands
//  2 = state machine
//  3 = DI/DO/READY reads & writes
//  4 = all reads & writes

#define VERBOSE_PRINTF 0
#define VERBOSE_LOGERROR 0

#define LOG0(x) do { if (VERBOSE_PRINTF >= 1) printf x; logerror x; } while (0)
#define LOG1(x) do { if (VERBOSE_PRINTF >= 1) printf x; if (VERBOSE_LOGERROR >= 1) logerror x; } while (0)
#define LOG2(x) do { if (VERBOSE_PRINTF >= 2) printf x; if (VERBOSE_LOGERROR >= 2) logerror x; } while (0)
#define LOG3(x) do { if (VERBOSE_PRINTF >= 3) printf x; if (VERBOSE_LOGERROR >= 3) logerror x; } while (0)
#define LOG4(x) do { if (VERBOSE_PRINTF >= 4) printf x; if (VERBOSE_LOGERROR >= 4) logerror x; } while (0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

ALLOW_SAVE_TYPE(eeprom_serial_base_device::eeprom_command);
ALLOW_SAVE_TYPE(eeprom_serial_base_device::eeprom_state);



//**************************************************************************
//  BASE DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_serial_base_device - constructor
//-------------------------------------------------

eeprom_serial_base_device::eeprom_serial_base_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_base_device(mconfig, devtype, name, tag, owner, shortname, source),
		m_command_address_bits(0),
		m_streaming_enabled(false),
		m_state(STATE_IN_RESET),
		m_cs_state(CLEAR_LINE),
		m_last_cs_rising_edge_time(attotime::zero),
		m_oe_state(CLEAR_LINE),
		m_clk_state(CLEAR_LINE),
		m_di_state(CLEAR_LINE),
		m_locked(true),
		m_bits_accum(0),
		m_command_address_accum(0),
		m_command(COMMAND_INVALID),
		m_address(0),
		m_shift_register(0)
{
}


//-------------------------------------------------
//  static_set_address_bits - configuration helper
//  to set the number of address bits in the
//  serial commands
//-------------------------------------------------

void eeprom_serial_base_device::static_set_address_bits(device_t &device, int addrbits)
{
	downcast<eeprom_serial_base_device &>(device).m_command_address_bits = addrbits;
}


//-------------------------------------------------
//  static_enable_streaming - configuration helper
//  to enable streaming data
//-------------------------------------------------

void eeprom_serial_base_device::static_enable_streaming(device_t &device)
{
	downcast<eeprom_serial_base_device &>(device).m_streaming_enabled = true;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_serial_base_device::device_start()
{
	// if no command address bits set, just inherit from the address bits
	if (m_command_address_bits == 0)
		m_command_address_bits = m_address_bits;

	// start the base class
	eeprom_base_device::device_start();

	// save the current state
	save_item(NAME(m_state));
	save_item(NAME(m_cs_state));
	save_item(NAME(m_oe_state));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_di_state));
	save_item(NAME(m_locked));
	save_item(NAME(m_bits_accum));
	save_item(NAME(m_command_address_accum));
	save_item(NAME(m_command));
	save_item(NAME(m_address));
	save_item(NAME(m_shift_register));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void eeprom_serial_base_device::device_reset()
{
	// reset the base class
	eeprom_base_device::device_reset();

	// reset the state
	set_state(STATE_IN_RESET);
	m_locked = true;
	m_bits_accum = 0;
	m_command_address_accum = 0;
	m_command = COMMAND_INVALID;
	m_address = 0;
	m_shift_register = 0;
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  base_cs_write - set the state of the chip
//  select (CS) line
//-------------------------------------------------

void eeprom_serial_base_device::base_cs_write(int state)
{
	// ignore if the state is not changing
	state &= 1;
	if (state == m_cs_state)
		return;

	// set the new state
	LOG4(("  cs_write(%d)\n", state));
	m_cs_state = state;

	// remember the rising edge time so we don't process CLK signals at the same time
	if (state == ASSERT_LINE)
		m_last_cs_rising_edge_time = machine().time();
	handle_event((m_cs_state == ASSERT_LINE) ? EVENT_CS_RISING_EDGE : EVENT_CS_FALLING_EDGE);
}


//-------------------------------------------------
//  base_clk_write - set the state of the clock
//  (CLK) line
//-------------------------------------------------

void eeprom_serial_base_device::base_clk_write(int state)
{
	// ignore if the state is not changing
	state &= 1;
	if (state == m_clk_state)
		return;

	// set the new state
	LOG4(("  clk_write(%d)\n", state));
	m_clk_state = state;
	handle_event((m_clk_state == ASSERT_LINE) ? EVENT_CLK_RISING_EDGE : EVENT_CLK_FALLING_EDGE);
}


//-------------------------------------------------
//  base_di_write - set the state of the data input
//  (DI) line
//-------------------------------------------------

void eeprom_serial_base_device::base_di_write(int state)
{
	if (state != 0 && state != 1)
		LOG0(("EEPROM: Unexpected data at input 0x%X treated as %d\n", state, state & 1));
	LOG3(("  di_write(%d)\n", state));
	m_di_state = state & 1;
}


//-------------------------------------------------
//  base_do_read - read the state of the data
//  output (DO) line
//-------------------------------------------------

int eeprom_serial_base_device::base_do_read()
{
	// in most states, the output is tristated, and generally connected to a pull up
	// to send back a 1 value; the only exception is if reading data and the current output
	// bit is a 0
	int result = (m_state == STATE_READING_DATA && ((m_shift_register & 0x80000000) == 0)) ? CLEAR_LINE : ASSERT_LINE;
	LOG3(("  do_read(%d)\n", result));
	return result;
}


//-------------------------------------------------
//  base_ready_read - read the state of the
//  READY/BUSY line
//-------------------------------------------------

int eeprom_serial_base_device::base_ready_read()
{
	// ready by default, except during long operations
	int result = ready() ? ASSERT_LINE : CLEAR_LINE;
	LOG3(("  ready_read(%d)\n", result));
	return result;
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_state - update the state to a new one
//-------------------------------------------------

void eeprom_serial_base_device::set_state(eeprom_state newstate)
{
#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	static const struct { eeprom_state state; const char *string; } s_state_names[] =
	{
		{ STATE_IN_RESET, "IN_RESET" },
		{ STATE_WAIT_FOR_START_BIT, "WAIT_FOR_START_BIT" },
		{ STATE_WAIT_FOR_COMMAND, "WAIT_FOR_COMMAND" },
		{ STATE_READING_DATA, "READING_DATA" },
		{ STATE_WAIT_FOR_DATA, "WAIT_FOR_DATA" },
		{ STATE_WAIT_FOR_COMPLETION, "WAIT_FOR_COMPLETION" },
	};
	const char *newstate_string = "UNKNOWN";
	for (int index = 0; index < ARRAY_LENGTH(s_state_names); index++)
		if (s_state_names[index].state == newstate)
			newstate_string = s_state_names[index].string;
	LOG2(("New state: %s\n", newstate_string));
#endif

	// switch to the new state
	m_state = newstate;
}


//-------------------------------------------------
//  handle_event - handle an event via the state
//  machine
//-------------------------------------------------

void eeprom_serial_base_device::handle_event(eeprom_event event)
{
#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	if ((event & EVENT_CS_RISING_EDGE) != 0) LOG2(("Event: CS rising\n"));
	if ((event & EVENT_CS_FALLING_EDGE) != 0) LOG2(("Event: CS falling\n"));
	if ((event & EVENT_CLK_RISING_EDGE) != 0)
	{
		if (m_state == STATE_WAIT_FOR_COMMAND || m_state == STATE_WAIT_FOR_DATA)
			LOG2(("Event: CLK rising (%d, DI=%d)\n", m_bits_accum + 1, m_di_state));
		else if (m_state == STATE_READING_DATA)
			LOG2(("Event: CLK rising (%d, DO=%d)\n", m_bits_accum + 1, (m_shift_register >> 30) & 1));
		else if (m_state == STATE_WAIT_FOR_START_BIT)
			LOG2(("Event: CLK rising (%d)\n", m_di_state));
		else
			LOG2(("Event: CLK rising\n"));
	}
	if ((event & EVENT_CLK_FALLING_EDGE) != 0) LOG4(("Event: CLK falling\n"));
#endif

	// switch off the current state
	switch (m_state)
	{
		// CS is not asserted; wait for a rising CS to move us forward, ignoring all clocks
		case STATE_IN_RESET:
			if (event == EVENT_CS_RISING_EDGE)
				set_state(STATE_WAIT_FOR_START_BIT);
			break;

		// CS is asserted; wait for rising clock with a 1 start bit; falling CS will reset us
		// note that because each bit is written independently, it is possible for us to receive
		// a false rising CLK edge at the exact same time as a rising CS edge; it appears we
		// should ignore these edges (makes sense really)
		case STATE_WAIT_FOR_START_BIT:
			if (event == EVENT_CLK_RISING_EDGE && m_di_state == ASSERT_LINE && ready() && machine().time() > m_last_cs_rising_edge_time)
			{
				m_command_address_accum = m_bits_accum = 0;
				set_state(STATE_WAIT_FOR_COMMAND);
			}
			else if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;

		// CS is asserted; wait for a command to come through; falling CS will reset us
		case STATE_WAIT_FOR_COMMAND:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				// if we have enough bits for a command + address, check it out
				m_command_address_accum = (m_command_address_accum << 1) | m_di_state;
				if (++m_bits_accum == 2 + m_command_address_bits)
					execute_command();
			}
			else if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;

		// CS is asserted; reading data, clock the shift register; falling CS will reset us
		case STATE_READING_DATA:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				int bit_index = m_bits_accum++;

				// wrapping the address on multi-read is required by pacslot(cave.c)
				if (bit_index % m_data_bits == 0 && (bit_index == 0 || m_streaming_enabled))
					m_shift_register = read((m_address + m_bits_accum / m_data_bits) & ((1 << m_address_bits) - 1)) << (32 - m_data_bits);
				else
					m_shift_register = (m_shift_register << 1) | 1;
			}
			else if (event == EVENT_CS_FALLING_EDGE)
			{
				set_state(STATE_IN_RESET);
				if (m_streaming_enabled)
					LOG1(("  (%d cells read)\n", m_bits_accum / m_data_bits));
				if (!m_streaming_enabled && m_bits_accum > m_data_bits + 1)
					LOG0(("EEPROM: Overclocked read by %d bits\n", m_bits_accum - m_data_bits));
				else if (m_streaming_enabled && m_bits_accum > m_data_bits + 1 && m_bits_accum % m_data_bits > 2)
					LOG0(("EEPROM: Overclocked read by %d bits\n", m_bits_accum % m_data_bits));
				else if (m_bits_accum < m_data_bits)
					LOG0(("EEPROM: CS deasserted in READING_DATA after %d bits\n", m_bits_accum));
			}
			break;

		// CS is asserted; waiting for data; clock data through until we accumulate enough; falling CS will reset us
		case STATE_WAIT_FOR_DATA:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				m_shift_register = (m_shift_register << 1) | m_di_state;
				if (++m_bits_accum == m_data_bits)
					execute_write_command();
			}
			else if (event == EVENT_CS_FALLING_EDGE)
			{
				set_state(STATE_IN_RESET);
				LOG0(("EEPROM: CS deasserted in STATE_WAIT_FOR_DATA after %d bits\n", m_bits_accum));
			}
			break;

		// CS is asserted; waiting for completion; watch for CS falling
		case STATE_WAIT_FOR_COMPLETION:
			if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;
	}
}


//-------------------------------------------------
//  execute_command - execute a command once we
//  have enough bits for one
//-------------------------------------------------

void eeprom_serial_base_device::execute_command()
{
	// parse into a generic command and reset the accumulator count
	parse_command_and_address();
	m_bits_accum = 0;

#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	static const struct { eeprom_command command; const char *string; } s_command_names[] =
	{
		{ COMMAND_INVALID, "Execute command: INVALID\n" },
		{ COMMAND_READ, "Execute command:READ 0x%X\n" },
		{ COMMAND_WRITE, "Execute command:WRITE 0x%X\n" },
		{ COMMAND_ERASE, "Execute command:ERASE 0x%X\n" },
		{ COMMAND_LOCK, "Execute command:LOCK\n" },
		{ COMMAND_UNLOCK, "Execute command:UNLOCK\n" },
		{ COMMAND_WRITEALL, "Execute command:WRITEALL\n" },
		{ COMMAND_ERASEALL, "Execute command:ERASEALL\n" },
	};
	const char *command_string = s_command_names[0].string;
	for (int index = 0; index < ARRAY_LENGTH(s_command_names); index++)
		if (s_command_names[index].command == m_command)
			command_string = s_command_names[index].string;
	LOG1((command_string, m_address));
#endif

	// each command advances differently
	switch (m_command)
	{
		// advance to the READING_DATA state; data is fetched after first CLK
		// reset the shift register to 0 to simulate the dummy 0 bit that happens prior
		// to the first clock
		case COMMAND_READ:
			m_shift_register = 0;
			set_state(STATE_READING_DATA);
			break;

		// reset the shift register and wait for enough data to be clocked through
		case COMMAND_WRITE:
		case COMMAND_WRITEALL:
			m_shift_register = 0;
			set_state(STATE_WAIT_FOR_DATA);
			break;

		// erase the parsed address (unless locked) and wait for it to complete
		case COMMAND_ERASE:
			if (m_locked)
			{
				LOG0(("EEPROM: Attempt to erase while locked\n"));
				set_state(STATE_IN_RESET);
				break;
			}
			erase(m_address);
			set_state(STATE_WAIT_FOR_COMPLETION);
			break;

		// lock the chip; return to IN_RESET state
		case COMMAND_LOCK:
			m_locked = true;
			set_state(STATE_IN_RESET);
			break;

		// unlock the chip; return to IN_RESET state
		case COMMAND_UNLOCK:
			m_locked = false;
			set_state(STATE_IN_RESET);
			break;

		// erase the entire chip (unless locked) and wait for it to complete
		case COMMAND_ERASEALL:
			if (m_locked)
			{
				LOG0(("EEPROM: Attempt to erase all while locked\n"));
				set_state(STATE_IN_RESET);
				break;
			}
			erase_all();
			set_state(STATE_WAIT_FOR_COMPLETION);
			break;

		default:
			throw emu_fatalerror("execute_command called with invalid command %d\n", m_command);
	}
}


//-------------------------------------------------
//  execute_write_command - execute a write
//  command after receiving the data bits
//-------------------------------------------------

void eeprom_serial_base_device::execute_write_command()
{
#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	static const struct { eeprom_command command; const char *string; } s_command_names[] =
	{
		{ COMMAND_WRITE, "Execute write command: WRITE 0x%X = 0x%X\n" },
		{ COMMAND_WRITEALL, "Execute write command: WRITEALL (%X) = 0x%X\n" },
	};
	const char *command_string = "UNKNOWN";
	for (int index = 0; index < ARRAY_LENGTH(s_command_names); index++)
		if (s_command_names[index].command == m_command)
			command_string = s_command_names[index].string;
	LOG1((command_string, m_address, m_shift_register));
#endif

	// each command advances differently
	switch (m_command)
	{
		// reset the shift register and wait for enough data to be clocked through
		case COMMAND_WRITE:
			if (m_locked)
			{
				LOG0(("EEPROM: Attempt to write to address 0x%X while locked\n", m_address));
				set_state(STATE_IN_RESET);
				break;
			}
			write(m_address, m_shift_register);
			set_state(STATE_WAIT_FOR_COMPLETION);
			break;

		// write the entire EEPROM with the same data; ERASEALL is required before so we
		// AND against the already-present data
		case COMMAND_WRITEALL:
			if (m_locked)
			{
				LOG0(("EEPROM: Attempt to write all while locked\n"));
				set_state(STATE_IN_RESET);
				break;
			}
			write_all(m_shift_register);
			set_state(STATE_WAIT_FOR_COMPLETION);
			break;

		default:
			throw emu_fatalerror("execute_write_command called with invalid command %d\n", m_command);
	}
}



//**************************************************************************
//  STANDARD INTERFACE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_serial_93cxx_device - constructor
//-------------------------------------------------

eeprom_serial_93cxx_device::eeprom_serial_93cxx_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_serial_base_device(mconfig, devtype, name, tag, owner, shortname, source)
{
}


//-------------------------------------------------
//  parse_command_and_address - extract the
//  command and address from a bitstream
//-------------------------------------------------

void eeprom_serial_93cxx_device::parse_command_and_address()
{
	// set the defaults
	m_command = COMMAND_INVALID;
	m_address = m_command_address_accum & ((1 << m_command_address_bits) - 1);

	// extract the command portion and handle it
	switch (m_command_address_accum >> m_command_address_bits)
	{
		// opcode 0 needs two more bits to decode the operation
		case 0:
			switch (m_address >> (m_command_address_bits - 2))
			{
				case 0: m_command = COMMAND_LOCK;       break;
				case 1: m_command = COMMAND_WRITEALL;   break;
				case 2: m_command = COMMAND_ERASEALL;   break;
				case 3: m_command = COMMAND_UNLOCK;     break;
			}
			m_address = 0;
			break;
		case 1: m_command = COMMAND_WRITE;  break;
		case 2: m_command = COMMAND_READ;   break;
		case 3: m_command = COMMAND_ERASE;  break;
	}

	// warn about out-of-range addresses
	if (m_address >= (1 << m_address_bits))
		LOG0(("EEPROM: out-of-range address 0x%X provided (maximum should be 0x%X)\n", m_address, (1 << m_address_bits) - 1));
}


//-------------------------------------------------
//  do_read - read handlers
//-------------------------------------------------

READ_LINE_MEMBER(eeprom_serial_93cxx_device::do_read) { return base_do_read() & ((m_state == STATE_WAIT_FOR_START_BIT) ? base_ready_read() : 1); }


//-------------------------------------------------
//  cs_write/clk_write/di_write - write handlers
//-------------------------------------------------

WRITE_LINE_MEMBER(eeprom_serial_93cxx_device::cs_write) { base_cs_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_93cxx_device::clk_write) { base_clk_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_93cxx_device::di_write) { base_di_write(state); }



//**************************************************************************
//  ER5911 DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_serial_er5911_device - constructor
//-------------------------------------------------

eeprom_serial_er5911_device::eeprom_serial_er5911_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_serial_base_device(mconfig, devtype, name, tag, owner, shortname, source)
{
}


//-------------------------------------------------
//  parse_command_and_address - extract the
//  command and address from a bitstream
//-------------------------------------------------

void eeprom_serial_er5911_device::parse_command_and_address()
{
	// set the defaults
	m_command = COMMAND_INVALID;
	m_address = m_command_address_accum & ((1 << m_command_address_bits) - 1);

	// extract the command portion and handle it
	switch (m_command_address_accum >> m_command_address_bits)
	{
		// opcode 0 needs two more bits to decode the operation
		case 0:
			switch (m_address >> (m_command_address_bits - 2))
			{
				case 0: m_command = COMMAND_LOCK;       break;
				case 1: m_command = COMMAND_INVALID;    break;  // not on ER5911
				case 2: m_command = COMMAND_ERASEALL;   break;
				case 3: m_command = COMMAND_UNLOCK;     break;
			}
			m_address = 0;
			break;
		case 1: m_command = COMMAND_WRITE;  break;
		case 2: m_command = COMMAND_READ;   break;
		case 3: m_command = COMMAND_WRITE;  break;  // WRITE instead of ERASE on ER5911
	}

	// warn about out-of-range addresses
	if (m_address >= (1 << m_address_bits))
		LOG0(("EEPROM: out-of-range address 0x%X provided (maximum should be 0x%X)\n", m_address, (1 << m_address_bits) - 1));
}


//-------------------------------------------------
//  do_read/ready_read - read handlers
//-------------------------------------------------

READ_LINE_MEMBER(eeprom_serial_er5911_device::do_read) { return base_do_read(); }
READ_LINE_MEMBER(eeprom_serial_er5911_device::ready_read) { return base_ready_read(); }


//-------------------------------------------------
//  cs_write/clk_write/di_write - write handlers
//-------------------------------------------------

WRITE_LINE_MEMBER(eeprom_serial_er5911_device::cs_write) { base_cs_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_er5911_device::clk_write) { base_clk_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_er5911_device::di_write) { base_di_write(state); }



//**************************************************************************
//  X24c44 DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_serial_x24c44_device - constructor
//-------------------------------------------------

eeprom_serial_x24c44_device::eeprom_serial_x24c44_device(const machine_config &mconfig, device_type devtype, std::string name, std::string tag, device_t *owner, std::string shortname, std::string source)
	: eeprom_serial_base_device(mconfig, devtype, name, tag, owner, shortname, source)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_serial_x24c44_device::device_start()
{
	// if no command address bits set, just inherit from the address bits
	if (m_command_address_bits == 0)
		m_command_address_bits = m_address_bits;

	// start the base class
	eeprom_base_device::device_start();

	INT16 i=0;
	m_ram_length=0xf;

	for (i=0;i<16;i++){
		m_ram_data[i]=read(i);  //autoreload at power up
	}
	m_reading=0;
	m_store_latch=0;
	// save the current state
	save_item(NAME(m_state));
	save_item(NAME(m_cs_state));
	save_item(NAME(m_oe_state));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_di_state));
	save_item(NAME(m_locked));
	save_item(NAME(m_bits_accum));
	save_item(NAME(m_command_address_accum));
	save_item(NAME(m_command));
	save_item(NAME(m_address));
	save_item(NAME(m_shift_register));
	save_item(NAME(m_ram_data));
	save_item(NAME(m_reading));
	save_item(NAME(m_store_latch));
}

void eeprom_serial_x24c44_device::copy_eeprom_to_ram(){
	UINT16 i=0;
	LOG1(("EEPROM TO RAM COPY!!!\n"));
	for (i=0;i<16;i++){
		m_ram_data[i]=read(i);
	}
	m_store_latch=1;
}



void eeprom_serial_x24c44_device::copy_ram_to_eeprom(){
	UINT16 i=0;
	if (m_store_latch){
		LOG1(("RAM TO EEPROM COPY\n"));
		for (i=0;i<16;i++){
			write(i, m_ram_data[i]);
		}
		m_store_latch=0;
	}else{
		LOG0(("Store command with store latch not set!\n"));
	}

}

//-------------------------------------------------
//  execute_command - execute a command once we
//  have enough bits for one
//-------------------------------------------------

void eeprom_serial_x24c44_device::execute_command()
{
	// parse into a generic command and reset the accumulator count
	parse_command_and_address();
	m_bits_accum = 0;

#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	static const struct { eeprom_command command; const char *string; } s_command_names[] =
	{
		{ COMMAND_INVALID, "Execute command: INVALID\n" },
		{ COMMAND_READ, "Execute command:READ 0x%X\n" },
		{ COMMAND_WRITE, "Execute command:WRITE 0x%X\n" },
		{ COMMAND_ERASE, "Execute command:ERASE 0x%X\n" },
		{ COMMAND_LOCK, "Execute command:LOCK\n" },
		{ COMMAND_UNLOCK, "Execute command:UNLOCK\n" },
		{ COMMAND_WRITEALL, "Execute command:WRITEALL\n" },
		{ COMMAND_ERASEALL, "Execute command:ERASEALL\n" },
		{ COMMAND_COPY_EEPROM_TO_RAM, "Execute command:COPY_EEPROM_TO_RAM\n" },
		{ COMMAND_COPY_RAM_TO_EEPROM, "Execute command:COPY_RAM_TO_EEPROM\n" },
	};
	const char *command_string = s_command_names[0].string;
	for (int index = 0; index < ARRAY_LENGTH(s_command_names); index++)
		if (s_command_names[index].command == m_command)
			command_string = s_command_names[index].string;
	LOG1((command_string, m_address));
#endif

	// each command advances differently
	switch (m_command)
	{
		// advance to the READING_DATA state; data is fetched after first CLK
		// reset the shift register to 0 to simulate the dummy 0 bit that happens prior
		// to the first clock

		// reset the shift register and wait for enough data to be clocked through
		case COMMAND_WRITE:
			m_shift_register = 0;
			set_state(STATE_WAIT_FOR_DATA);
			break;

		// lock the chip; return to IN_RESET state
		case COMMAND_LOCK:
			m_locked = true;
			m_store_latch=0;
			set_state(STATE_IN_RESET);
			break;

		// unlock the chip; return to IN_RESET state
		case COMMAND_UNLOCK:
			m_locked = false;
			m_store_latch=1;
			set_state(STATE_IN_RESET);
			break;

		// copy eeprom to ram
		case COMMAND_COPY_EEPROM_TO_RAM:
			copy_eeprom_to_ram();
			set_state(STATE_IN_RESET);
			break;

		// copy ram into eeprom
		case COMMAND_COPY_RAM_TO_EEPROM:
			copy_ram_to_eeprom();
			set_state(STATE_IN_RESET);
			break;

		default:
			throw emu_fatalerror("execute_command called with invalid command %d\n", m_command);
	}
}


void eeprom_serial_x24c44_device::handle_event(eeprom_event event)
{
//UINT32 tmp=0;
#if (VERBOSE_PRINTF > 0 || VERBOSE_LOGERROR > 0)
	// for debugging purposes
	if ((event & EVENT_CS_RISING_EDGE) != 0) LOG2(("Event: CS rising\n"));
	if ((event & EVENT_CS_FALLING_EDGE) != 0) LOG2(("Event: CS falling\n"));
	if ((event & EVENT_CLK_RISING_EDGE) != 0)
	{
		if (m_state == STATE_WAIT_FOR_COMMAND || m_state == STATE_WAIT_FOR_DATA)
			LOG2(("Event: CLK rising (%d, DI=%d)\n", m_bits_accum + 1, m_di_state));
		else if (m_state == STATE_READING_DATA)
			LOG2(("Event: CLK rising (%d, DO=%d)\n", m_bits_accum + 1, (m_shift_register >> 30) & 1));
		else if (m_state == STATE_WAIT_FOR_START_BIT)
			LOG2(("Event: CLK rising (%d)\n", m_di_state));
		else
			LOG2(("Event: CLK rising\n"));
	}
	if ((event & EVENT_CLK_FALLING_EDGE) != 0) LOG4(("Event: CLK falling\n"));
#endif

	// switch off the current state
	switch (m_state)
	{
		// CS is not asserted; wait for a rising CS to move us forward, ignoring all clocks
		case STATE_IN_RESET:
			if (event == EVENT_CS_RISING_EDGE)
				set_state(STATE_WAIT_FOR_START_BIT);
			break;

		// CS is asserted; wait for rising clock with a 1 start bit; falling CS will reset us
		// note that because each bit is written independently, it is possible for us to receive
		// a false rising CLK edge at the exact same time as a rising CS edge; it appears we
		// should ignore these edges (makes sense really)
		case STATE_WAIT_FOR_START_BIT:
			if (event == EVENT_CLK_RISING_EDGE && m_di_state == ASSERT_LINE && ready() && machine().time() > m_last_cs_rising_edge_time)
			{
				m_command_address_accum = m_bits_accum = 0;
				set_state(STATE_WAIT_FOR_COMMAND);
			}
			else if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;

		// CS is asserted; wait for a command to come through; falling CS will reset us
		case STATE_WAIT_FOR_COMMAND:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				// if we have enough bits for a command + address, check it out
				m_command_address_accum = (m_command_address_accum << 1) | m_di_state;

				m_bits_accum=m_bits_accum+1;

				if (m_bits_accum == 2 + m_command_address_bits){
					//read command is only 2 bits all other are 3 bits!!!

						parse_command_and_address_2_bit();

				}

				if (!m_reading){
				if (m_bits_accum == 3 + m_command_address_bits){
					execute_command();
				}
				}
			}
			else if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;

		// CS is asserted; reading data, clock the shift register; falling CS will reset us
		case STATE_READING_DATA:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				int bit_index = m_bits_accum++;

				if (bit_index % m_data_bits == 0 && (bit_index == 0 || m_streaming_enabled)){
					m_shift_register=m_ram_data[m_address];

					//m_shift_register=BITSWAP16(m_shift_register,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
					//m_shift_register=BITSWAP16(m_shift_register,7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8);
					m_shift_register= BITSWAP16(m_shift_register,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);

					m_shift_register=m_shift_register<<16;

					LOG1(("read from RAM addr %02X data(from ram) %04X ,m_shift_register vale %04X \n",m_address,m_ram_data[m_address],m_shift_register));
					}
				else{
					m_shift_register = (m_shift_register << 1) | 1;

				}
			}
			else if (event == EVENT_CS_FALLING_EDGE)
			{
				set_state(STATE_IN_RESET);
				m_reading=0;
				if (m_streaming_enabled)
					LOG1(("  (%d cells read)\n", m_bits_accum / m_data_bits));
				if (!m_streaming_enabled && m_bits_accum > m_data_bits + 1)
					LOG1(("EEPROM: Overclocked read by %d bits\n", m_bits_accum - m_data_bits));
				else if (m_streaming_enabled && m_bits_accum > m_data_bits + 1 && m_bits_accum % m_data_bits > 2)
					LOG1(("EEPROM: Overclocked read by %d bits\n", m_bits_accum % m_data_bits));
				else if (m_bits_accum < m_data_bits)
					LOG1(("EEPROM: CS deasserted in READING_DATA after %d bits\n", m_bits_accum));
			}
			break;

		// CS is asserted; waiting for data; clock data through until we accumulate enough; falling CS will reset us
		case STATE_WAIT_FOR_DATA:
			if (event == EVENT_CLK_RISING_EDGE)
			{
				m_shift_register = (m_shift_register << 1) | m_di_state;
				if (++m_bits_accum == m_data_bits){
				//m_shift_register=BITSWAP16(m_shift_register, 0, 1, 2, 3, 4, 5,6,7, 8, 9,10,11,12,13,14,15);
				//m_shift_register=BITSWAP16(m_shift_register, 7, 6, 5, 4, 3, 2,1,0,15,14,13,12,11,10, 9, 8);
				m_shift_register=BITSWAP16(m_shift_register,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7);
				m_ram_data[m_address]=m_shift_register;

				LOG1(("write to RAM addr=%02X data=%04X\n",m_address,m_shift_register));
				}
			}
			else if (event == EVENT_CS_FALLING_EDGE)
			{
				set_state(STATE_IN_RESET);
				LOG1(("EEPROM: CS deasserted in STATE_WAIT_FOR_DATA after %d bits\n", m_bits_accum));
			}
			break;


		// CS is asserted; waiting for completion; watch for CS falling
		case STATE_WAIT_FOR_COMPLETION:
			if (event == EVENT_CS_FALLING_EDGE)
				set_state(STATE_IN_RESET);
			break;
	}
}


//-------------------------------------------------
//  parse_command_and_address - extract the
//  command and address from a bitstream
//-------------------------------------------------

void eeprom_serial_x24c44_device::parse_command_and_address()
{
	//command is start_bit - 4bit_address - 3bit_command

	// set the defaults
	m_command = COMMAND_INVALID;

	m_address = (m_command_address_accum >> 3) & 0x0f;

	LOG1(("EEPROM: command= %04X, address %02X\n", m_command_address_accum& 0x07, m_address));

	switch (m_command_address_accum & 0x07)
	{
		case 0: //reset write enable latch
				LOG0(("Lock eeprom\n"));
				m_command = COMMAND_LOCK;   break;
		case 3: //write data into ram
				LOG0(("Write to ram\n"));
				m_command = COMMAND_WRITE;  break;
		case 4: //set write enable latch
				LOG0(("Unlock eeprom\n"));
				m_command = COMMAND_UNLOCK; break;
		case 1: //store ram data in eeprom
				LOG0(("copy ram to eeprom\n"));
				m_command = COMMAND_COPY_RAM_TO_EEPROM;   break;
		case 5: //reload eeprom data into ram
				LOG0(("copy eeprom to ram\n"));
				m_command = COMMAND_COPY_EEPROM_TO_RAM; break;
		case 2: //reserved (Sleep on x2444)
			m_command = COMMAND_INVALID;
				break;

	}

}

void eeprom_serial_x24c44_device::parse_command_and_address_2_bit()
{
	if ((m_command_address_accum & 0x03) == 0x03){
		m_command = COMMAND_READ;
		m_address = ((m_command_address_accum >> 2) & 0x0f);
		m_shift_register = 0;
		set_state(STATE_READING_DATA);
		LOG1(("parse command_and_address_2_bit found a read command\n"));
		m_reading=1;
		m_bits_accum=0;
	}

	// warn about out-of-range addresses
	if (m_address >= (1 << m_address_bits))
		LOG1(("EEPROM: out-of-range address 0x%X provided (maximum should be 0x%X)\n", m_address, (1 << m_address_bits) - 1));
}


//-------------------------------------------------
//  do_read/ready_read - read handlers
//-------------------------------------------------

READ_LINE_MEMBER(eeprom_serial_x24c44_device::do_read) { return base_do_read(); }


//-------------------------------------------------
//  cs_write/clk_write/di_write - write handlers
//-------------------------------------------------

WRITE_LINE_MEMBER(eeprom_serial_x24c44_device::cs_write) { base_cs_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_x24c44_device::clk_write) { base_clk_write(state); }
WRITE_LINE_MEMBER(eeprom_serial_x24c44_device::di_write) { base_di_write(state); }


//**************************************************************************
//  DERIVED TYPES
//**************************************************************************

// macro for defining a new device class
#define DEFINE_SERIAL_EEPROM_DEVICE(_baseclass, _lowercase, _uppercase, _bits, _cells, _addrbits) \
eeprom_serial_##_lowercase##_##_bits##bit_device::eeprom_serial_##_lowercase##_##_bits##bit_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) \
	: eeprom_serial_##_baseclass##_device(mconfig, EEPROM_SERIAL_##_uppercase##_##_bits##BIT, "Serial EEPROM " #_uppercase " (" #_cells "x" #_bits ")", tag, owner, #_lowercase "_" #_bits, __FILE__) \
{ \
	static_set_size(*this, _cells, _bits); \
	static_set_address_bits(*this, _addrbits); \
} \
const device_type EEPROM_SERIAL_##_uppercase##_##_bits##BIT = &device_creator<eeprom_serial_##_lowercase##_##_bits##bit_device>;
// standard 93CX6 class of 16-bit EEPROMs
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c06, 93C06, 16, 16, 6)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c46, 93C46, 16, 64, 6)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c56, 93C56, 16, 128, 8)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c57, 93C57, 16, 128, 7)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c66, 93C66, 16, 256, 8)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c76, 93C76, 16, 512, 10)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c86, 93C86, 16, 1024, 10)

// some manufacturers use pin 6 as an "ORG" pin which, when pulled low, configures memory for 8-bit accesses
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c46, 93C46, 8, 128, 7)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c56, 93C56, 8, 256, 9)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c57, 93C57, 8, 256, 8)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c66, 93C66, 8, 512, 9)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c76, 93C76, 8, 1024, 11)
DEFINE_SERIAL_EEPROM_DEVICE(93cxx, 93c86, 93C86, 8, 2048, 11)

// ER5911 has a separate ready pin, a reduced command set, and supports 8/16 bit out of the box
DEFINE_SERIAL_EEPROM_DEVICE(er5911, er5911, ER5911, 8, 128, 9)
DEFINE_SERIAL_EEPROM_DEVICE(er5911, er5911, ER5911, 16, 64, 8)
DEFINE_SERIAL_EEPROM_DEVICE(er5911, msm16911, MSM16911, 8, 128, 9)
DEFINE_SERIAL_EEPROM_DEVICE(er5911, msm16911, MSM16911, 16, 64, 8)

// X24c44 8 bit 32byte ram/eeprom combo
DEFINE_SERIAL_EEPROM_DEVICE(x24c44, x24c44, X24C44, 16, 16, 4)
