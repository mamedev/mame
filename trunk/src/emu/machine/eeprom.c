/***************************************************************************

    eeprom.c

    Serial eeproms.

***************************************************************************/

#include "emu.h"
#include "machine/eeprom.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type EEPROM = &device_creator<eeprom_device>;

const eeprom_interface eeprom_interface_93C46 =
{
	6,				// address bits 6
	16,				// data bits    16
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,				// enable_multi_read
	0				// reset_delay
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddddddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

const eeprom_interface eeprom_interface_93C46_8bit =
{
	7,				// address bits 7
	8,				// data bits    8
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,				// enable_multi_read
	0				// reset_delay
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

const eeprom_interface eeprom_interface_93C66B =
{
	8,				// address bits
	16,				// data bits
	"*110",			// read command
	"*101",			// write command
	"*111",			// erase command
	"*10000xxxxxx",	// lock command
	"*10011xxxxxx", // unlock command
	1,				// enable_multi_read
	0				// reset_delay
//  "*10001xxxxxx", // write all
//  "*10010xxxxxx", // erase all
};


static ADDRESS_MAP_START( eeprom_map8, AS_PROGRAM, 8, eeprom_device )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( eeprom_map16, AS_PROGRAM, 16, eeprom_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  eeprom_device - constructor
//-------------------------------------------------

eeprom_device::eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, EEPROM, "EEPROM", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  device_nvram_interface(mconfig, *this),
	  m_default_data_size(0),
	  m_default_value(0),
	  m_serial_count(0),
	  m_data_buffer(0),
	  m_read_address(0),
	  m_clock_count(0),
	  m_latch(0),
	  m_reset_line(CLEAR_LINE),
	  m_clock_line(CLEAR_LINE),
	  m_sending(0),
	  m_locked(false),
	  m_reset_counter(0)
{
	m_default_data.u8 = NULL;
	memset(downcast<eeprom_interface *>(this), 0, sizeof(eeprom_interface));
}


//-------------------------------------------------
//  static_set_interface - configuration helper
//  to set the interface
//-------------------------------------------------

void eeprom_device::static_set_interface(device_t &device, const eeprom_interface &interface)
{
	eeprom_device &eeprom = downcast<eeprom_device &>(device);
	static_cast<eeprom_interface &>(eeprom) = interface;

	// describe our address space
	if (eeprom.m_data_bits == 8)
		eeprom.m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 8,  eeprom.m_address_bits, 0, *ADDRESS_MAP_NAME(eeprom_map8));
	else
		eeprom.m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 16, eeprom.m_address_bits * 2, 0, *ADDRESS_MAP_NAME(eeprom_map16));
}


//-------------------------------------------------
//  static_set_default_data - configuration helpers
//  to set the default data
//-------------------------------------------------

void eeprom_device::static_set_default_data(device_t &device, const UINT8 *data, UINT32 size)
{
	eeprom_device &eeprom = downcast<eeprom_device &>(device);
	assert(eeprom.m_data_bits == 8);
	eeprom.m_default_data.u8 = const_cast<UINT8 *>(data);
	eeprom.m_default_data_size = size;
}

void eeprom_device::static_set_default_data(device_t &device, const UINT16 *data, UINT32 size)
{
	eeprom_device &eeprom = downcast<eeprom_device &>(device);
	assert(eeprom.m_data_bits == 16);
	eeprom.m_default_data.u16 = const_cast<UINT16 *>(data);
	eeprom.m_default_data_size = size / 2;
}


//-------------------------------------------------
//  static_set_default_value - configuration helper
//  to set the default value
//-------------------------------------------------

void eeprom_device::static_set_default_value(device_t &device, UINT16 value)
{
	downcast<eeprom_device &>(device).m_default_value = 0x10000 | value;
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void eeprom_device::device_validity_check(validity_checker &valid) const
{
	if (m_data_bits != 8 && m_data_bits != 16)
		mame_printf_error("Invalid data width %d specified\n", m_data_bits);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_device::device_start()
{
	m_locked = (m_cmd_unlock != NULL);

	save_pointer(NAME(m_serial_buffer), SERIAL_BUFFER_LENGTH);
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

void eeprom_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *eeprom_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void eeprom_device::nvram_default()
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	/* initialize to the default value */
	UINT16 default_value = 0xffff;
	if (m_default_value != 0)
		default_value = m_default_value;
	for (offs_t offs = 0; offs < eeprom_length; offs++)
		if (m_data_bits == 8)
			m_addrspace[0]->write_byte(offs, default_value);
		else
			m_addrspace[0]->write_word(offs * 2, default_value);

	/* handle hard-coded data from the driver */
	if (m_default_data.u8 != NULL)
		for (offs_t offs = 0; offs < m_default_data_size; offs++) {
			if (m_data_bits == 8)
				m_addrspace[0]->write_byte(offs, m_default_data.u8[offs]);
			else
				m_addrspace[0]->write_word(offs * 2, m_default_data.u16[offs]);
		}

	/* populate from a memory region if present */
	if (m_region != NULL)
	{
		if (m_region->bytes() != eeprom_bytes)
			fatalerror("eeprom region '%s' wrong size (expected size = 0x%X)", tag(), eeprom_bytes);
		if (m_data_bits == 8 && m_region->width() != 1)
			fatalerror("eeprom region '%s' needs to be an 8-bit region", tag());
		if (m_data_bits == 16 && (m_region->width() != 2 || m_region->endianness() != ENDIANNESS_BIG))
			fatalerror("eeprom region '%s' needs to be a 16-bit big-endian region", tag());

		for (offs_t offs = 0; offs < eeprom_length; offs++)
			if (m_data_bits == 8)
				m_addrspace[0]->write_byte(offs, m_region->u8(offs));
			else
				m_addrspace[0]->write_word(offs * 2, m_region->u16(offs));
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void eeprom_device::nvram_read(emu_file &file)
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	UINT8 *buffer = auto_alloc_array(machine(), UINT8, eeprom_bytes);
	file.read(buffer, eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		m_addrspace[0]->write_byte(offs, buffer[offs]);
	auto_free(machine(), buffer);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void eeprom_device::nvram_write(emu_file &file)
{
	UINT32 eeprom_length = 1 << m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_data_bits / 8;

	UINT8 *buffer = auto_alloc_array(machine(), UINT8, eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		buffer[offs] = m_addrspace[0]->read_byte(offs);
	file.write(buffer, eeprom_bytes);
	auto_free(machine(), buffer);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER( eeprom_device::write_bit )
{
	LOG(("write bit %d\n",state));
	m_latch = state;
}


READ_LINE_DEVICE_HANDLER( eeprom_read_bit )
{
	return downcast<eeprom_device *>(device)->read_bit();
}

READ_LINE_MEMBER( eeprom_device::read_bit )
{
	int res;

	if (m_sending)
		res = (m_data_buffer >> m_data_bits) & 1;
	else
	{
		if (m_reset_counter > 0)
		{
			/* this is needed by wbeachvl */
			m_reset_counter--;
			res = 0;
		}
		else
			res = 1;
	}

	LOG(("read bit %d\n",res));

	return res;
}



WRITE_LINE_MEMBER( eeprom_device::set_cs_line )
{
	LOG(("set reset line %d\n",state));
	m_reset_line = state;

	if (m_reset_line != CLEAR_LINE)
	{
		if (m_serial_count)
			logerror("EEPROM %s reset, buffer = %s\n", tag(), m_serial_buffer);

		m_serial_count = 0;
		m_sending = 0;
		m_reset_counter = m_reset_delay;	/* delay a little before returning setting data to 1 (needed by wbeachvl) */
	}
}



WRITE_LINE_MEMBER( eeprom_device::set_clock_line )
{
	LOG(("set clock line %d\n",state));
	if (state == PULSE_LINE || (m_clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		if (m_reset_line == CLEAR_LINE)
		{
			if (m_sending)
			{
				if (m_clock_count == m_data_bits && m_enable_multi_read)
				{
					m_read_address = (m_read_address + 1) & ((1 << m_address_bits) - 1);
					if (m_data_bits == 16)
						m_data_buffer = m_addrspace[0]->read_word(m_read_address * 2);
					else
						m_data_buffer = m_addrspace[0]->read_byte(m_read_address);
					m_clock_count = 0;
					logerror("EEPROM %s read %04x from address %02x\n", tag(), m_data_buffer, m_read_address);
				}
				m_data_buffer = (m_data_buffer << 1) | 1;
				m_clock_count++;
			}
			else
				write(m_latch);
		}
	}

	m_clock_line = state;
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

void eeprom_device::write(int bit)
{
	LOG(("EEPROM %s write bit %d\n", tag(), bit));

	if (m_serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
		logerror("error: EEPROM %s serial buffer overflow\n", tag());
		return;
	}

	m_serial_buffer[m_serial_count++] = (bit ? '1' : '0');
	m_serial_buffer[m_serial_count] = 0;	/* nul terminate so we can treat it as a string */

	if ( (m_serial_count > m_address_bits) &&
	      command_match((char*)(m_serial_buffer),m_cmd_read,strlen((char*)(m_serial_buffer))-m_address_bits) )
	{
		int i,address;

		address = 0;
		for (i = m_serial_count-m_address_bits;i < m_serial_count;i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
		if (m_data_bits == 16)
			m_data_buffer = m_addrspace[0]->read_word(address * 2);
		else
			m_data_buffer = m_addrspace[0]->read_byte(address);
		m_read_address = address;
		m_clock_count = 0;
		m_sending = 1;
		m_serial_count = 0;
		logerror("EEPROM %s read %04x from address %02x\n", tag(), m_data_buffer,address);
	}
	else if ( (m_serial_count > m_address_bits) &&
	           command_match((char*)(m_serial_buffer),m_cmd_erase,strlen((char*)(m_serial_buffer))-m_address_bits) )
	{
		int i,address;

		address = 0;
		for (i = m_serial_count-m_address_bits;i < m_serial_count;i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
		logerror("EEPROM %s erase address %02x\n", tag(), address);
		if (m_locked == 0)
		{
			if (m_data_bits == 16)
				m_addrspace[0]->write_word(address * 2, 0xFFFF);
			else
				m_addrspace[0]->write_byte(address, 0xFF);
		}
		else
			logerror("Error: EEPROM %s is locked\n", tag());
		m_serial_count = 0;
	}
	else if ( (m_serial_count > (m_address_bits + m_data_bits)) &&
	           command_match((char*)(m_serial_buffer),m_cmd_write,strlen((char*)(m_serial_buffer))-(m_address_bits + m_data_bits)) )
	{
		int i,address,data;

		address = 0;
		for (i = m_serial_count-m_data_bits-m_address_bits;i < (m_serial_count-m_data_bits);i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = m_serial_count-m_data_bits;i < m_serial_count;i++)
		{
			data <<= 1;
			if (m_serial_buffer[i] == '1') data |= 1;
		}
		logerror("EEPROM %s write %04x to address %02x\n", tag(), data, address);
		if (m_locked == 0)
		{
			if (m_data_bits == 16)
				m_addrspace[0]->write_word(address * 2, data);
			else
				m_addrspace[0]->write_byte(address, data);
		}
		else
			logerror("Error: EEPROM %s is locked\n", tag());
		m_serial_count = 0;
	}
	else if ( command_match((char*)(m_serial_buffer),m_cmd_lock,strlen((char*)(m_serial_buffer))) )
	{
		logerror("EEPROM %s lock\n", tag());
		m_locked = 1;
		m_serial_count = 0;
	}
	else if ( command_match((char*)(m_serial_buffer),m_cmd_unlock,strlen((char*)(m_serial_buffer))) )
	{
		logerror("EEPROM %s unlock\n", tag());
		m_locked = 0;
		m_serial_count = 0;
	}
}


/*
    command_match:

    Try to match the first (len) digits in the EEPROM serial buffer
    string (*buf) with  an EEPROM command string (*cmd).
    Return non zero if a match was found.

    The serial buffer only contains '0' or '1' (e.g. "1001").
    The command can contain: '0' or '1' or these wildcards:

    'x' :   match both '0' and '1'
    "*1":   match "1", "01", "001", "0001" etc.
    "*0":   match "0", "10", "110", "1110" etc.

    Note: (cmd) may be NULL. Return 0 (no match) in this case.
*/
bool eeprom_device::command_match(const char *buf, const char *cmd, int len)
{
	if ( cmd == 0 )	return false;
	if ( len == 0 )	return false;

	for (;len>0;)
	{
		char b = *buf;
		char c = *cmd;

		if ((b==0) || (c==0))
			return (b==c);

		switch ( c )
		{
			case '0':
			case '1':
				if (b != c)	return false;
			case 'X':
			case 'x':
				buf++;
				len--;
				cmd++;
				break;

			case '*':
				c = cmd[1];
				switch( c )
				{
					case '0':
					case '1':
						if (b == c)	{	cmd++;			}
						else		{	buf++;	len--;	}
						break;
					default:	return false;
				}
		}
	}
	return (*cmd==0);
}
