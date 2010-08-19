/***************************************************************************

    eeprom.h

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


static ADDRESS_MAP_START( eeprom_map8, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( eeprom_map16, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  eeprom_device_config - constructor
//-------------------------------------------------

eeprom_device_config::eeprom_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "EEPROM", tag, owner, clock),
	  device_config_memory_interface(mconfig, *this),
	  device_config_nvram_interface(mconfig, *this),
	  m_default_data(NULL),
	  m_default_data_size(0),
	  m_default_value(0)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *eeprom_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(eeprom_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *eeprom_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, eeprom_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void eeprom_device_config::device_config_complete()
{
	// extract inline configuration from raw data
	const eeprom_interface *intf = reinterpret_cast<const eeprom_interface *>(m_inline_data[INLINE_INTERFACE]);
	m_default_data = reinterpret_cast<const UINT8 *>(m_inline_data[INLINE_DATAPTR]);
	m_default_data_size = m_inline_data[INLINE_DATASIZE];
	m_default_value = m_inline_data[INLINE_DEFVALUE];

	// inherit a copy of the static data
	if (intf != NULL)
		*static_cast<eeprom_interface *>(this) = *intf;

	// now describe our address space
	if (m_data_bits == 8)
		m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 8,  m_address_bits, 0, *ADDRESS_MAP_NAME(eeprom_map8));
	else
		m_space_config = address_space_config("eeprom", ENDIANNESS_BIG, 16, m_address_bits * 2, 0, *ADDRESS_MAP_NAME(eeprom_map16));
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool eeprom_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;

	if (m_inline_data[INLINE_INTERFACE] == 0)
	{
		mame_printf_error("%s: %s eeprom device '%s' did not specify an interface\n", driver.source_file, driver.name, tag());
		error = true;
	}
	else if (m_data_bits != 8 && m_data_bits != 16)
	{
		mame_printf_error("%s: %s eeprom device '%s' specified invalid data width %d\n", driver.source_file, driver.name, tag(), m_data_bits);
		error = true;
	}

	return error;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *eeprom_device_config::memory_space_config(int spacenum) const
{
	return (spacenum == 0) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  eeprom_device - constructor
//-------------------------------------------------

eeprom_device::eeprom_device(running_machine &_machine, const eeprom_device_config &config)
	: device_t(_machine, config),
	  device_memory_interface(_machine, config, *this),
	  device_nvram_interface(_machine, config, *this),
	  m_config(config),
	  m_serial_count(0),
	  m_data_bits(0),
	  m_read_address(0),
	  m_clock_count(0),
	  m_latch(0),
	  m_reset_line(CLEAR_LINE),
	  m_clock_line(CLEAR_LINE),
	  m_sending(0),
	  m_locked(m_config.m_cmd_unlock != NULL),
	  m_reset_delay(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eeprom_device::device_start()
{
	state_save_register_device_item_pointer(this, 0, m_serial_buffer, SERIAL_BUFFER_LENGTH);
	state_save_register_device_item(this, 0, m_clock_line);
	state_save_register_device_item(this, 0, m_reset_line);
	state_save_register_device_item(this, 0, m_locked);
	state_save_register_device_item(this, 0, m_serial_count);
	state_save_register_device_item(this, 0, m_latch);
	state_save_register_device_item(this, 0, m_reset_delay);
	state_save_register_device_item(this, 0, m_clock_count);
	state_save_register_device_item(this, 0, m_data_bits);
	state_save_register_device_item(this, 0, m_read_address);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void eeprom_device::device_reset()
{
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void eeprom_device::nvram_default()
{
	UINT32 eeprom_length = 1 << m_config.m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_config.m_data_bits / 8;

	/* initialize to the default value */
	UINT16 default_value = 0xffff;
	if (m_config.m_default_value != 0)
		default_value = m_config.m_default_value;
	for (offs_t offs = 0; offs < eeprom_length; offs++)
		if (m_config.m_data_bits == 8)
			m_addrspace[0]->write_byte(offs, default_value);
		else
			m_addrspace[0]->write_word(offs * 2, default_value);

	/* handle hard-coded data from the driver */
	if (m_config.m_default_data != NULL)
		for (offs_t offs = 0; offs < m_config.m_default_data_size; offs++)
			m_addrspace[0]->write_byte(offs, m_config.m_default_data[offs]);

	/* populate from a memory region if present */
	if (m_region != NULL)
	{
		if (m_region->bytes() != eeprom_bytes)
			fatalerror("eeprom region '%s' wrong size (expected size = 0x%X)", tag(), eeprom_bytes);
		if (m_config.m_data_bits == 8 && m_region->width() != 1)
			fatalerror("eeprom region '%s' needs to be an 8-bit region", tag());
		if (m_config.m_data_bits == 16 && (m_region->width() != 2 || m_region->endianness() != ENDIANNESS_BIG))
			fatalerror("eeprom region '%s' needs to be a 16-bit big-endian region (flags=%08x)", tag(), m_region->flags());

		for (offs_t offs = 0; offs < eeprom_length; offs++)
			if (m_config.m_data_bits == 8)
				m_addrspace[0]->write_byte(offs, m_region->u8(offs));
			else
				m_addrspace[0]->write_word(offs * 2, m_region->u16(offs));
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void eeprom_device::nvram_read(mame_file &file)
{
	UINT32 eeprom_length = 1 << m_config.m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_config.m_data_bits / 8;

	UINT8 *buffer = auto_alloc_array(&m_machine, UINT8, eeprom_bytes);
	mame_fread(&file, buffer, eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		m_addrspace[0]->write_byte(offs, buffer[offs]);
	auto_free(&m_machine, buffer);
}


//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void eeprom_device::nvram_write(mame_file &file)
{
	UINT32 eeprom_length = 1 << m_config.m_address_bits;
	UINT32 eeprom_bytes = eeprom_length * m_config.m_data_bits / 8;

	UINT8 *buffer = auto_alloc_array(&m_machine, UINT8, eeprom_bytes);
	for (offs_t offs = 0; offs < eeprom_bytes; offs++)
		buffer[offs] = m_addrspace[0]->read_byte(offs);
	mame_fwrite(&file, buffer, eeprom_bytes);
	auto_free(&m_machine, buffer);
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( eeprom_write_bit )
{
	downcast<eeprom_device *>(device)->write_bit(state);
}

void eeprom_device::write_bit(int state)
{
	LOG(("write bit %d\n",state));
	m_latch = state;
}


READ_LINE_DEVICE_HANDLER( eeprom_read_bit )
{
	return downcast<eeprom_device *>(device)->read_bit();
}

int eeprom_device::read_bit()
{
	int res;

	if (m_sending)
		res = (m_data_bits >> m_config.m_data_bits) & 1;
	else
	{
		if (m_reset_delay > 0)
		{
			/* this is needed by wbeachvl */
			m_reset_delay--;
			res = 0;
		}
		else
			res = 1;
	}

	LOG(("read bit %d\n",res));

	return res;
}



WRITE_LINE_DEVICE_HANDLER( eeprom_set_cs_line )
{
	downcast<eeprom_device *>(device)->set_cs_line(state);
}

void eeprom_device::set_cs_line(int state)
{
	LOG(("set reset line %d\n",state));
	m_reset_line = state;

	if (m_reset_line != CLEAR_LINE)
	{
		if (m_serial_count)
			logerror("EEPROM reset, buffer = %s\n",m_serial_buffer);

		m_serial_count = 0;
		m_sending = 0;
		m_reset_delay = m_config.m_reset_delay;	/* delay a little before returning setting data to 1 (needed by wbeachvl) */
	}
}



WRITE_LINE_DEVICE_HANDLER( eeprom_set_clock_line )
{
	downcast<eeprom_device *>(device)->set_clock_line(state);
}

void eeprom_device::set_clock_line(int state)
{
	LOG(("set clock line %d\n",state));
	if (state == PULSE_LINE || (m_clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		if (m_reset_line == CLEAR_LINE)
		{
			if (m_sending)
			{
				if (m_clock_count == m_config.m_data_bits && m_config.m_enable_multi_read)
				{
					m_read_address = (m_read_address + 1) & ((1 << m_config.m_address_bits) - 1);
					if (m_config.m_data_bits == 16)
						m_data_bits = m_addrspace[0]->read_word(m_read_address * 2);
					else
						m_data_bits = m_addrspace[0]->read_byte(m_read_address);
					m_clock_count = 0;
logerror("EEPROM read %04x from address %02x\n",m_data_bits,m_read_address);
				}
				m_data_bits = (m_data_bits << 1) | 1;
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
	LOG(("EEPROM write bit %d\n",bit));

	if (m_serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
		logerror("error: EEPROM serial buffer overflow\n");
		return;
	}

	m_serial_buffer[m_serial_count++] = (bit ? '1' : '0');
	m_serial_buffer[m_serial_count] = 0;	/* nul terminate so we can treat it as a string */

	if ( (m_serial_count > m_config.m_address_bits) &&
	      command_match((char*)(m_serial_buffer),m_config.m_cmd_read,strlen((char*)(m_serial_buffer))-m_config.m_address_bits) )
	{
		int i,address;

		address = 0;
		for (i = m_serial_count-m_config.m_address_bits;i < m_serial_count;i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
		if (m_config.m_data_bits == 16)
			m_data_bits = m_addrspace[0]->read_word(address * 2);
		else
			m_data_bits = m_addrspace[0]->read_byte(address);
		m_read_address = address;
		m_clock_count = 0;
		m_sending = 1;
		m_serial_count = 0;
logerror("EEPROM read %04x from address %02x\n",m_data_bits,address);
	}
	else if ( (m_serial_count > m_config.m_address_bits) &&
	           command_match((char*)(m_serial_buffer),m_config.m_cmd_erase,strlen((char*)(m_serial_buffer))-m_config.m_address_bits) )
	{
		int i,address;

		address = 0;
		for (i = m_serial_count-m_config.m_address_bits;i < m_serial_count;i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
logerror("EEPROM erase address %02x\n",address);
		if (m_locked == 0)
		{
			if (m_config.m_data_bits == 16)
				m_addrspace[0]->write_word(address * 2, 0x0000);
			else
				m_addrspace[0]->write_byte(address, 0x00);
		}
		else
logerror("Error: EEPROM is m_locked\n");
		m_serial_count = 0;
	}
	else if ( (m_serial_count > (m_config.m_address_bits + m_config.m_data_bits)) &&
	           command_match((char*)(m_serial_buffer),m_config.m_cmd_write,strlen((char*)(m_serial_buffer))-(m_config.m_address_bits + m_config.m_data_bits)) )
	{
		int i,address,data;

		address = 0;
		for (i = m_serial_count-m_config.m_data_bits-m_config.m_address_bits;i < (m_serial_count-m_config.m_data_bits);i++)
		{
			address <<= 1;
			if (m_serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = m_serial_count-m_config.m_data_bits;i < m_serial_count;i++)
		{
			data <<= 1;
			if (m_serial_buffer[i] == '1') data |= 1;
		}
logerror("EEPROM write %04x to address %02x\n",data,address);
		if (m_locked == 0)
		{
			if (m_config.m_data_bits == 16)
				m_addrspace[0]->write_word(address * 2, data);
			else
				m_addrspace[0]->write_byte(address, data);
		}
		else
logerror("Error: EEPROM is m_locked\n");
		m_serial_count = 0;
	}
	else if ( command_match((char*)(m_serial_buffer),m_config.m_cmd_lock,strlen((char*)(m_serial_buffer))) )
	{
logerror("EEPROM lock\n");
		m_locked = 1;
		m_serial_count = 0;
	}
	else if ( command_match((char*)(m_serial_buffer),m_config.m_cmd_unlock,strlen((char*)(m_serial_buffer))) )
	{
logerror("EEPROM unlock\n");
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

const device_type EEPROM = eeprom_device_config::static_alloc_device_config;
