/***************************************************************************

    eepromser.c

    Serial EEPROM devices.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "machine/eepromser.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SERIAL_EEPROM = &device_creator<serial_eeprom_device>;

const serial_eeprom_interface eeprom_interface_93C46_93C66B =
{
	"*110",         // read         1 10 aaaaaa
	"*101",         // write        1 01 aaaaaa dddddddddddddddd
	"*111",         // erase        1 11 aaaaaa
	"*10000xxxx",   // lock         1 00 00xxxx
	"*10011xxxx",   // unlock       1 00 11xxxx
	1,              // enable_multi_read
	0               // reset_delay
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddddddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  serial_eeprom_device - constructor
//-------------------------------------------------

serial_eeprom_device::serial_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_eeprom_device(mconfig, SERIAL_EEPROM, "Serial EEPROM", tag, owner, "seeprom", __FILE__),
		m_serial_count(0),
		m_data_buffer(0),
		m_read_address(0),
		m_clock_count(0),
		m_latch(0),
		m_reset_line(CLEAR_LINE),
		m_clock_line(CLEAR_LINE),
		m_sending(false),
		m_locked(false),
		m_reset_counter(0)
{
	memset(downcast<serial_eeprom_interface *>(this), 0, sizeof(serial_eeprom_interface));
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void serial_eeprom_device::static_set_interface(device_t &device, const serial_eeprom_interface &interface)
{
	serial_eeprom_device &eeprom = downcast<serial_eeprom_device &>(device);
	static_cast<serial_eeprom_interface &>(eeprom) = interface;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void serial_eeprom_device::device_start()
{
	base_eeprom_device::device_start();

	m_locked = (m_cmd_unlock != NULL);

	save_item(NAME(m_serial_buffer));
	save_item(NAME(m_clock_line));
	save_item(NAME(m_reset_line));
	save_item(NAME(m_locked));
	save_item(NAME(m_serial_count));
	save_item(NAME(m_latch));
	save_item(NAME(m_reset_counter));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_data_buffer));
	save_item(NAME(m_read_address));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void serial_eeprom_device::device_reset()
{
	base_eeprom_device::device_reset();

	// make a note if someone reset in the middle of a read or write
	if (m_serial_count)
		logerror("EEPROM %s reset, buffer = %s\n", tag(), m_serial_buffer);

	// reset the state
	m_serial_count = 0;
	m_sending = false;
	m_reset_counter = m_reset_delay;    // delay a little before returning setting data to 1 (needed by wbeachvl)
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  write_bit - latch a bit to write
//-------------------------------------------------

WRITE_LINE_MEMBER( serial_eeprom_device::write_bit )
{
	LOG(("write bit %d\n",state));
	m_latch = state;
}


//-------------------------------------------------
//  read_bit - read a bit from the eeprom
//-------------------------------------------------

READ_LINE_MEMBER( serial_eeprom_device::read_bit )
{
	int res;

	// if sending, pull the next bit off
	if (m_sending)
		res = (m_data_buffer >> m_data_bits) & 1;
	
	// otherwise check for the proper number of bits needed for a reset
	else
	{
		// this is needed by wbeachvl
		if (m_reset_counter > 0)
		{
			m_reset_counter--;
			res = 0;
		}
		else
			res = 1;
	}

	LOG(("read bit %d\n",res));

	return res;
}


//-------------------------------------------------
//  set_cs_line - set the state of the chip
//	select (/CS) line
//-------------------------------------------------

WRITE_LINE_MEMBER( serial_eeprom_device::set_cs_line )
{
	// ignore if the state is not changing
	if (state == m_reset_line)
		return;
		
	LOG(("set reset line %d\n",state));

	// if we're going active, reset things
	m_reset_line = state;
	if (m_reset_line != CLEAR_LINE)
		reset();
}


//-------------------------------------------------
//  set_clock_line - set the state of the clock
//	(CLK) line
//-------------------------------------------------

WRITE_LINE_MEMBER( serial_eeprom_device::set_clock_line )
{
	LOG(("set clock line %d\n",state));
	
	// on a pulse or a rising edge, process
	if (state == PULSE_LINE || (m_clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		// only proceed if we're not held in reset
		if (m_reset_line == CLEAR_LINE)
		{
			// if sending, clock the next bit
			if (m_sending)
			{
				// auto-advance to then next word if supported
				if (m_clock_count == m_data_bits && m_enable_multi_read)
				{
					fill_data_buffer(m_read_address + 1);
					logerror("EEPROM %s read %04x from address %02x\n", tag(), m_data_buffer, m_read_address);
				}
				
				// shift the data buffer
				m_data_buffer = (m_data_buffer << 1) | 1;
				m_clock_count++;
			}
			
			// if not sending, then write the data that was latched
			else
				write(m_latch);
		}
	}

	// remember the news state
	m_clock_line = state;
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  decode_value - convert accumulated bits to
//	a binary value, releative to the end of the
//	accumulation buffer
//-------------------------------------------------

UINT32 serial_eeprom_device::decode_value(int numbits, int bitsfromend)
{
	UINT32 value = 0;
	for (int bitnum = m_serial_count - bitsfromend - numbits; bitnum < m_serial_count - bitsfromend; bitnum++)
		value = (value << 1) | (m_serial_buffer[bitnum] - '0');
	return value;
}


//-------------------------------------------------
//  fill_data_buffer - fill the data buffer with
//	the appropriately-sized chunk of data
//-------------------------------------------------

void serial_eeprom_device::fill_data_buffer(offs_t address)
{
	// make the address to be in range
	address &= (1 << m_address_bits) - 1;
	
	// fetch the appropriately-sized data
	m_data_buffer = read_data(address);

	// remember the address and reset the clock count
	m_read_address = address;
	m_clock_count = 0;
}


//-------------------------------------------------
//  write - process an EEPROM write
//-------------------------------------------------

void serial_eeprom_device::write(int bit)
{
	LOG(("EEPROM %s write bit %d\n", tag(), bit));

	// if too much data was written without seeing a command, log it and return
	if (m_serial_count >= SERIAL_BUFFER_LENGTH - 1)
	{
		logerror("error: EEPROM %s serial buffer overflow\n", tag());
		return;
	}

	// update the buffer
	m_serial_buffer[m_serial_count++] = (bit ? '1' : '0');
	m_serial_buffer[m_serial_count] = 0;    // nul terminate so we can treat it as a string

	// look for a read command
	if (m_cmd_read != NULL && m_serial_count > m_address_bits && command_match(m_cmd_read, m_address_bits))
	{
		fill_data_buffer(decode_value(m_address_bits));
		m_sending = true;
		m_serial_count = 0;
		logerror("EEPROM %s read %04x from address %02x\n", tag(), m_data_buffer, m_read_address);
	}
	
	// look for an erase command
	else if (m_cmd_erase != NULL && m_serial_count > m_address_bits && command_match(m_cmd_erase, m_address_bits))
	{
		offs_t address = decode_value(m_address_bits);
		logerror("EEPROM %s erase address %02x\n", tag(), address);
		if (m_locked == 0)
			write_data(address, ~0);
		else
			logerror("Error: EEPROM %s is locked\n", tag());
		m_serial_count = 0;
	}
	
	// look for a write command
	else if (m_cmd_write != NULL && m_serial_count > m_address_bits + m_data_bits && command_match(m_cmd_write, m_address_bits + m_data_bits))
	{
		offs_t address = decode_value(m_address_bits, m_data_bits);
		UINT32 data = decode_value(m_data_bits);
		logerror("EEPROM %s write %04x to address %02x\n", tag(), data, address);
		if (m_locked == 0)
			write_data(address, data);
		else
			logerror("Error: EEPROM %s is locked\n", tag());
		m_serial_count = 0;
	}
	
	// look for a lock command
	else if (m_cmd_lock != NULL && command_match(m_cmd_lock))
	{
		logerror("EEPROM %s lock\n", tag());
		m_locked = 1;
		m_serial_count = 0;
	}

	// look for an unlock command
	else if (m_cmd_unlock != NULL && command_match(m_cmd_unlock))
	{
		logerror("EEPROM %s unlock\n", tag());
		m_locked = 0;
		m_serial_count = 0;
	}
}


//-------------------------------------------------
//  command_match - try to match incoming data
//	against a command template
//-------------------------------------------------

bool serial_eeprom_device::command_match(const char *cmd, int ignorebits)
{
	//
	//  The serial buffer only contains '0' or '1' (e.g. "1001").
	//  The command can contain: '0' or '1' or these wildcards:
	//      'x' :   match both '0' and '1'
	//      "*1":   match "1", "01", "001", "0001" etc.
	//      "*0":   match "0", "10", "110", "1110" etc.
	//

	int len = m_serial_count - ignorebits;
	const char *buf = m_serial_buffer;
	while (len > 0)
	{
		char bufbit = *buf;
		char cmdbit = *cmd;

		// stop when we hit the end of either string
		if (bufbit == 0 || cmdbit == 0)
			return (bufbit == cmdbit);

		// parse based on the cmdbit
		switch (cmdbit)
		{
			case '0':
			case '1':
				// these require an exact match
				if (bufbit != cmdbit)
					return false;
				
				// fall through...

			case 'X':
			case 'x':
				// this is 'ignore', so just accept anything
				buf++;
				len--;
				cmd++;
				break;

			case '*':
				// for a wildcard, check for the opposit bit
				cmdbit = cmd[1];
				switch (cmdbit)
				{
					case '0':
					case '1':
						if (bufbit == cmdbit)
							cmd++;
						else
							buf++, len--;
						break;
						
					default:
						return false;
				}
		}
	}
	return (*cmd == 0);
}
