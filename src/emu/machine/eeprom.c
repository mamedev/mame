/*
 * Serial eeproms
 *
 */

#include "emu.h"
#include "machine/eeprom.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define SERIAL_BUFFER_LENGTH 40

typedef struct _eeprom_state eeprom_state;
struct _eeprom_state
{
	const eeprom_interface *intf;
	int serial_count;
	UINT8 serial_buffer[SERIAL_BUFFER_LENGTH];
	int data_bits;
	int read_address;
	int clock_count;
	int latch,reset_line,clock_line,sending;
	int locked;
	int reset_delay;
};

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an I2C memory
-------------------------------------------------*/

INLINE eeprom_state *get_safe_token(running_device *device)
{
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == EEPROM );

	return (eeprom_state *)device->token;
}

/*
    eeprom_command_match:

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
static int eeprom_command_match(const char *buf, const char *cmd, int len)
{
	if ( cmd == 0 )	return 0;
	if ( len == 0 )	return 0;

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
				if (b != c)	return 0;
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
					default:	return 0;
				}
		}
	}
	return (*cmd==0);
}


const eeprom_interface eeprom_interface_93C46 =
{
	6,				// address bits 6
	16,				// data bits    16
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",	// lock         1 00 00xxxx
	"*10011xxxx",	// unlock       1 00 11xxxx
	1,
//  "*10001xxxx"    // write all    1 00 01xxxx dddddddddddddddd
//  "*10010xxxx"    // erase all    1 00 10xxxx
};

const eeprom_interface eeprom_interface_93C66B =
{
	8,				/* address bits */
	16,				/* data bits */
	"*110",			/* read command */
	"*101",			/* write command */
	"*111",			/* erase command */
	"*10000xxxxxx",	/* lock command */
	"*10011xxxxxx", /* unlock command */
	1,
//  "*10001xxxxxx", /* write all */
//  "*10010xxxxxx", /* erase all */
};

static void eeprom_write(running_device *device, int bit)
{
	eeprom_state *eestate = get_safe_token(device);

	LOG(("EEPROM write bit %d\n",bit));

	if (eestate->serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
		logerror("error: EEPROM serial buffer overflow\n");
		return;
	}

	eestate->serial_buffer[eestate->serial_count++] = (bit ? '1' : '0');
	eestate->serial_buffer[eestate->serial_count] = 0;	/* nul terminate so we can treat it as a string */

	if ( (eestate->serial_count > eestate->intf->address_bits) &&
	      eeprom_command_match((char*)(eestate->serial_buffer),eestate->intf->cmd_read,strlen((char*)(eestate->serial_buffer))-eestate->intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = eestate->serial_count-eestate->intf->address_bits;i < eestate->serial_count;i++)
		{
			address <<= 1;
			if (eestate->serial_buffer[i] == '1') address |= 1;
		}
		if (eestate->intf->data_bits == 16)
			eestate->data_bits = memory_read_word(device->space(), address * 2);
		else
			eestate->data_bits = memory_read_byte(device->space(), address);
		eestate->read_address = address;
		eestate->clock_count = 0;
		eestate->sending = 1;
		eestate->serial_count = 0;
logerror("EEPROM read %04x from address %02x\n",eestate->data_bits,address);
	}
	else if ( (eestate->serial_count > eestate->intf->address_bits) &&
	           eeprom_command_match((char*)(eestate->serial_buffer),eestate->intf->cmd_erase,strlen((char*)(eestate->serial_buffer))-eestate->intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = eestate->serial_count-eestate->intf->address_bits;i < eestate->serial_count;i++)
		{
			address <<= 1;
			if (eestate->serial_buffer[i] == '1') address |= 1;
		}
logerror("EEPROM erase address %02x\n",address);
		if (eestate->locked == 0)
		{
			if (eestate->intf->data_bits == 16)
				memory_write_word(device->space(), address * 2, 0x0000);
			else
				memory_write_byte(device->space(), address, 0x00);
		}
		else
logerror("Error: EEPROM is locked\n");
		eestate->serial_count = 0;
	}
	else if ( (eestate->serial_count > (eestate->intf->address_bits + eestate->intf->data_bits)) &&
	           eeprom_command_match((char*)(eestate->serial_buffer),eestate->intf->cmd_write,strlen((char*)(eestate->serial_buffer))-(eestate->intf->address_bits + eestate->intf->data_bits)) )
	{
		int i,address,data;

		address = 0;
		for (i = eestate->serial_count-eestate->intf->data_bits-eestate->intf->address_bits;i < (eestate->serial_count-eestate->intf->data_bits);i++)
		{
			address <<= 1;
			if (eestate->serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = eestate->serial_count-eestate->intf->data_bits;i < eestate->serial_count;i++)
		{
			data <<= 1;
			if (eestate->serial_buffer[i] == '1') data |= 1;
		}
logerror("EEPROM write %04x to address %02x\n",data,address);
		if (eestate->locked == 0)
		{
			if (eestate->intf->data_bits == 16)
				memory_write_word(device->space(), address * 2, data);
			else
				memory_write_byte(device->space(), address, data);
		}
		else
logerror("Error: EEPROM is locked\n");
		eestate->serial_count = 0;
	}
	else if ( eeprom_command_match((char*)(eestate->serial_buffer),eestate->intf->cmd_lock,strlen((char*)(eestate->serial_buffer))) )
	{
logerror("EEPROM lock\n");
		eestate->locked = 1;
		eestate->serial_count = 0;
	}
	else if ( eeprom_command_match((char*)(eestate->serial_buffer),eestate->intf->cmd_unlock,strlen((char*)(eestate->serial_buffer))) )
	{
logerror("EEPROM unlock\n");
		eestate->locked = 0;
		eestate->serial_count = 0;
	}
}

static void eeprom_reset(eeprom_state *eestate)
{
	if (eestate->serial_count)
		logerror("EEPROM reset, buffer = %s\n",eestate->serial_buffer);

	eestate->serial_count = 0;
	eestate->sending = 0;
	eestate->reset_delay = eestate->intf->reset_delay;	/* delay a little before returning setting data to 1 (needed by wbeachvl) */
}


WRITE_LINE_DEVICE_HANDLER( eeprom_write_bit )
{
	eeprom_state *eestate = get_safe_token(device);

	LOG(("write bit %d\n",state));
	eestate->latch = state;
}

READ_LINE_DEVICE_HANDLER( eeprom_read_bit )
{
	eeprom_state *eestate = get_safe_token(device);
	int res;

	if (eestate->sending)
		res = (eestate->data_bits >> eestate->intf->data_bits) & 1;
	else
	{
		if (eestate->reset_delay > 0)
		{
			/* this is needed by wbeachvl */
			eestate->reset_delay--;
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
	eeprom_state *eestate = get_safe_token(device);

	LOG(("set reset line %d\n",state));
	eestate->reset_line = state;

	if (eestate->reset_line != CLEAR_LINE)
		eeprom_reset(eestate);
}

WRITE_LINE_DEVICE_HANDLER( eeprom_set_clock_line )
{
	eeprom_state *eestate = get_safe_token(device);

	LOG(("set clock line %d\n",state));
	if (state == PULSE_LINE || (eestate->clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		if (eestate->reset_line == CLEAR_LINE)
		{
			if (eestate->sending)
			{
				if (eestate->clock_count == eestate->intf->data_bits && eestate->intf->enable_multi_read)
				{
					eestate->read_address = (eestate->read_address + 1) & ((1 << eestate->intf->address_bits) - 1);
					if (eestate->intf->data_bits == 16)
						eestate->data_bits = memory_read_word(device->space(), eestate->read_address * 2);
					else
						eestate->data_bits = memory_read_byte(device->space(), eestate->read_address);
					eestate->clock_count = 0;
logerror("EEPROM read %04x from address %02x\n",eestate->data_bits,eestate->read_address);
				}
				eestate->data_bits = (eestate->data_bits << 1) | 1;
				eestate->clock_count++;
			}
			else
				eeprom_write(device,eestate->latch);
		}
	}

	eestate->clock_line = state;
}


static DEVICE_NVRAM( eeprom )
{
	eeprom_state *eestate = get_safe_token(device);
	UINT32 eeprom_length = 1 << eestate->intf->address_bits;
	UINT32 eeprom_bytes = eeprom_length * eestate->intf->data_bits / 8;
	UINT32 offs;

	if (read_or_write)
	{
		UINT8 *buffer = auto_alloc_array(device->machine, UINT8, eeprom_bytes);
		for (offs = 0; offs < eeprom_bytes; offs++)
			buffer[offs] = memory_read_byte(device->space(), offs);
		mame_fwrite(file, buffer, eeprom_bytes);
		auto_free(device->machine, buffer);
	}
	else if (file != NULL)
	{
		UINT8 *buffer = auto_alloc_array(device->machine, UINT8, eeprom_bytes);
		mame_fread(file, buffer, eeprom_bytes);
		for (offs = 0; offs < eeprom_bytes; offs++)
			memory_write_byte(device->space(), offs, buffer[offs]);
		auto_free(device->machine, buffer);
	}
	else
	{
		const eeprom_config *config = (const eeprom_config *)device->baseconfig().inline_config;
		UINT16 default_value = 0xffff;
		int offs;

		/* initialize to the default value */
		if (config->default_value != 0)
			default_value = config->default_value;
		for (offs = 0; offs < eeprom_length; offs++)
			if (eestate->intf->data_bits == 8)
				memory_write_byte(device->space(), offs, default_value);
			else
				memory_write_word(device->space(), offs * 2, default_value);

		/* handle hard-coded data from the driver */
		if (config->default_data != NULL)
			for (offs = 0; offs < config->default_data_size; offs++)
				memory_write_byte(device->space(), offs, config->default_data[offs]);

		/* populate from a memory region if present */
		if (device->region != NULL)
		{
			if (device->region->length != eeprom_bytes)
				fatalerror("eeprom region '%s' wrong size (expected size = 0x%X)", device->tag(), eeprom_bytes);
			if (eestate->intf->data_bits == 8 && (device->region->flags & ROMREGION_WIDTHMASK) != ROMREGION_8BIT)
				fatalerror("eeprom region '%s' needs to be an 8-bit region", device->tag());
			if (eestate->intf->data_bits == 16 && ((device->region->flags & ROMREGION_WIDTHMASK) != ROMREGION_16BIT || (device->region->flags & ROMREGION_ENDIANMASK) != ROMREGION_BE))
				fatalerror("eeprom region '%s' needs to be a 16-bit big-endian region (flags=%08x)", device->tag(), device->region->flags);

			for (offs = 0; offs < eeprom_length; offs++)
				if (eestate->intf->data_bits == 8)
					memory_write_byte(device->space(), offs, device->region->base.u8[offs]);
				else
					memory_write_word(device->space(), offs * 2, device->region->base.u16[offs]);
		}
	}
}

static DEVICE_START( eeprom )
{
	eeprom_state *eestate = get_safe_token(device);
	const eeprom_config *config;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->baseconfig().inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	config = (const eeprom_config *)device->baseconfig().inline_config;

	eestate->intf = config->pinterface;

	eestate->serial_count = 0;
	eestate->latch = 0;
	eestate->reset_line = CLEAR_LINE;
	eestate->clock_line = CLEAR_LINE;
	eestate->read_address = 0;
	eestate->sending = 0;
	if (eestate->intf->cmd_unlock) eestate->locked = 1;
	else eestate->locked = 0;

	state_save_register_device_item_pointer( device, 0, eestate->serial_buffer, SERIAL_BUFFER_LENGTH);
	state_save_register_device_item( device, 0, eestate->clock_line);
	state_save_register_device_item( device, 0, eestate->reset_line);
	state_save_register_device_item( device, 0, eestate->locked);
	state_save_register_device_item( device, 0, eestate->serial_count);
	state_save_register_device_item( device, 0, eestate->latch);
	state_save_register_device_item( device, 0, eestate->reset_delay);
	state_save_register_device_item( device, 0, eestate->clock_count);
	state_save_register_device_item( device, 0, eestate->data_bits);
	state_save_register_device_item( device, 0, eestate->read_address);
}

static ADDRESS_MAP_START( eeprom_map8, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( eeprom_map16, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
ADDRESS_MAP_END

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)			p##eeprom##s
#define DEVTEMPLATE_FEATURES		DT_HAS_START | DT_HAS_NVRAM | DT_HAS_INLINE_CONFIG | DT_HAS_PROGRAM_SPACE
#define DEVTEMPLATE_NAME			"EEPROM"
#define DEVTEMPLATE_FAMILY			"EEPROM"
#define DEVTEMPLATE_ENDIANNESS		ENDIANNESS_BIG
#define DEVTEMPLATE_PGM_DATAWIDTH	(((const eeprom_config *)device->inline_config)->pinterface->data_bits)
#define DEVTEMPLATE_PGM_ADDRWIDTH	(((const eeprom_config *)device->inline_config)->pinterface->address_bits)
#define DEVTEMPLATE_PGM_ADDRSHIFT	(((const eeprom_config *)device->inline_config)->pinterface->data_bits == 8 ? 0 : -1)
#define DEVTEMPLATE_PGM_DEFMAP		(((const eeprom_config *)device->inline_config)->pinterface->data_bits == 8 ? (const void *)ADDRESS_MAP_NAME(eeprom_map8) : (const void *)ADDRESS_MAP_NAME(eeprom_map16))
#include "devtempl.h"
