/*
 * Serial eeproms
 *
 */

#include "driver.h"
#include "machine/eepromdev.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define SERIAL_BUFFER_LENGTH 40
#define MEMORY_SIZE 1024

/* FIXME: many drivers do not need default_data / default_data_size and put them to 0 in the drivers seems a waste of code */
UINT8 *eeprom_empty_default_data = NULL;

typedef struct _eeprom_state eeprom_state;
struct _eeprom_state
{
	const eeprom_interface *intf;
	int serial_count;
	UINT8 serial_buffer[SERIAL_BUFFER_LENGTH];
	UINT8 eeprom_data[MEMORY_SIZE];
	int eeprom_data_bits;
	int eeprom_read_address;
	int eeprom_clock_count;
	int latch,reset_line,clock_line,sending;
	int locked;
	int reset_delay;
};

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an I2C memory
-------------------------------------------------*/

INLINE eeprom_state *get_safe_token(const device_config *device)
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


const eeprom_interface eepromdev_interface_93C46 =
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

const eeprom_interface eepromdev_interface_93C66B =
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

static void eeprom_write(eeprom_state *c, int bit)
{
	LOG(("EEPROM write bit %d\n",bit));

	if (c->serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
		logerror("error: EEPROM serial buffer overflow\n");
		return;
	}

	c->serial_buffer[c->serial_count++] = (bit ? '1' : '0');
	c->serial_buffer[c->serial_count] = 0;	/* nul terminate so we can treat it as a string */

	if ( (c->serial_count > c->intf->address_bits) &&
	      eeprom_command_match((char*)(c->serial_buffer),c->intf->cmd_read,strlen((char*)(c->serial_buffer))-c->intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = c->serial_count-c->intf->address_bits;i < c->serial_count;i++)
		{
			address <<= 1;
			if (c->serial_buffer[i] == '1') address |= 1;
		}
		if (c->intf->data_bits == 16)
			c->eeprom_data_bits = (c->eeprom_data[2*address+0] << 8) + c->eeprom_data[2*address+1];
		else
			c->eeprom_data_bits = c->eeprom_data[address];
		c->eeprom_read_address = address;
		c->eeprom_clock_count = 0;
		c->sending = 1;
		c->serial_count = 0;
logerror("EEPROM read %04x from address %02x\n",c->eeprom_data_bits,address);
	}
	else if ( (c->serial_count > c->intf->address_bits) &&
	           eeprom_command_match((char*)(c->serial_buffer),c->intf->cmd_erase,strlen((char*)(c->serial_buffer))-c->intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = c->serial_count-c->intf->address_bits;i < c->serial_count;i++)
		{
			address <<= 1;
			if (c->serial_buffer[i] == '1') address |= 1;
		}
logerror("EEPROM erase address %02x\n",address);
		if (c->locked == 0)
		{
			if (c->intf->data_bits == 16)
			{
				c->eeprom_data[2*address+0] = 0x00;
				c->eeprom_data[2*address+1] = 0x00;
			}
			else
				c->eeprom_data[address] = 0x00;
		}
		else
logerror("Error: EEPROM is locked\n");
		c->serial_count = 0;
	}
	else if ( (c->serial_count > (c->intf->address_bits + c->intf->data_bits)) &&
	           eeprom_command_match((char*)(c->serial_buffer),c->intf->cmd_write,strlen((char*)(c->serial_buffer))-(c->intf->address_bits + c->intf->data_bits)) )
	{
		int i,address,data;

		address = 0;
		for (i = c->serial_count-c->intf->data_bits-c->intf->address_bits;i < (c->serial_count-c->intf->data_bits);i++)
		{
			address <<= 1;
			if (c->serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = c->serial_count-c->intf->data_bits;i < c->serial_count;i++)
		{
			data <<= 1;
			if (c->serial_buffer[i] == '1') data |= 1;
		}
logerror("EEPROM write %04x to address %02x\n",data,address);
		if (c->locked == 0)
		{
			if (c->intf->data_bits == 16)
			{
				c->eeprom_data[2*address+0] = data >> 8;
				c->eeprom_data[2*address+1] = data & 0xff;
			}
			else
				c->eeprom_data[address] = data;
		}
		else
logerror("Error: EEPROM is locked\n");
		c->serial_count = 0;
	}
	else if ( eeprom_command_match((char*)(c->serial_buffer),c->intf->cmd_lock,strlen((char*)(c->serial_buffer))) )
	{
logerror("EEPROM lock\n");
		c->locked = 1;
		c->serial_count = 0;
	}
	else if ( eeprom_command_match((char*)(c->serial_buffer),c->intf->cmd_unlock,strlen((char*)(c->serial_buffer))) )
	{
logerror("EEPROM unlock\n");
		c->locked = 0;
		c->serial_count = 0;
	}
}

static void eeprom_reset(eeprom_state *c)
{
	if (c->serial_count)
		logerror("EEPROM reset, buffer = %s\n",c->serial_buffer);

	c->serial_count = 0;
	c->sending = 0;
	c->reset_delay = c->intf->reset_delay;	/* delay a little before returning setting data to 1 (needed by wbeachvl) */
}


WRITE_LINE_DEVICE_HANDLER( eepromdev_write_bit )
{
	eeprom_state *c = get_safe_token(device);

	LOG(("write bit %d\n",state));
	c->latch = state;
}

READ_LINE_DEVICE_HANDLER( eepromdev_read_bit )
{
	eeprom_state *c = get_safe_token(device);
	int res;

	if (c->sending)
		res = (c->eeprom_data_bits >> c->intf->data_bits) & 1;
	else
	{
		if (c->reset_delay > 0)
		{
			/* this is needed by wbeachvl */
			c->reset_delay--;
			res = 0;
		}
		else
			res = 1;
	}

	LOG(("read bit %d\n",res));

	return res;
}

WRITE_LINE_DEVICE_HANDLER( eepromdev_set_cs_line )
{
	eeprom_state *c = get_safe_token(device);

	LOG(("set reset line %d\n",state));
	c->reset_line = state;

	if (c->reset_line != CLEAR_LINE)
		eeprom_reset(c);
}

WRITE_LINE_DEVICE_HANDLER( eepromdev_set_clock_line )
{
	eeprom_state *c = get_safe_token(device);

	LOG(("set clock line %d\n",state));
	if (state == PULSE_LINE || (c->clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		if (c->reset_line == CLEAR_LINE)
		{
			if (c->sending)
			{
				if (c->eeprom_clock_count == c->intf->data_bits && c->intf->enable_multi_read)
				{
					c->eeprom_read_address = (c->eeprom_read_address + 1) & ((1 << c->intf->address_bits) - 1);
					if (c->intf->data_bits == 16)
						c->eeprom_data_bits = (c->eeprom_data[2*c->eeprom_read_address+0] << 8) + c->eeprom_data[2*c->eeprom_read_address+1];
					else
						c->eeprom_data_bits = c->eeprom_data[c->eeprom_read_address];
					c->eeprom_clock_count = 0;
logerror("EEPROM read %04x from address %02x\n",c->eeprom_data_bits,c->eeprom_read_address);
				}
				c->eeprom_data_bits = (c->eeprom_data_bits << 1) | 1;
				c->eeprom_clock_count++;
			}
			else
				eeprom_write(c,c->latch);
		}
	}

	c->clock_line = state;
}


static void eepromdev_load(const device_config *device, mame_file *f)
{
	eeprom_state *c = get_safe_token(device);

	mame_fread(f, c->eeprom_data, (1 << c->intf->address_bits) * c->intf->data_bits / 8);
}

static void eepromdev_save(const device_config *device, mame_file *f)
{
	eeprom_state *c = get_safe_token(device);

	mame_fwrite(f, c->eeprom_data, (1 << c->intf->address_bits) * c->intf->data_bits / 8);
}

void eepromdev_set_data(const device_config *device, const UINT8 *data, int length)
{
	eeprom_state *c = get_safe_token(device);

	assert(length <= ((1 << c->intf->address_bits) * c->intf->data_bits / 8));
	memcpy(c->eeprom_data, data, length);
}

void *eepromdev_get_data_pointer(const device_config *device, UINT32 *length, UINT32 *size)
{
	eeprom_state *c = get_safe_token(device);

	if (length != NULL && c->intf != NULL)
		*length = 1 << c->intf->address_bits;
	if (size != NULL && c->intf != NULL)
		*size = c->intf->data_bits / 8;

	return c->eeprom_data;
}

static DEVICE_NVRAM( eeprom )
{
	const eeprom_config *config = (const eeprom_config *)device->inline_config;

	if (read_or_write)
		eepromdev_save(device, file);
	else
		if (file)
			eepromdev_load(device, file);
		else
			if ((config->default_data != NULL) && (config->default_data_size != 0))
				eepromdev_set_data(device, config->default_data, config->default_data_size);
}

static DEVICE_START(eeprom)
{
	eeprom_state *c = get_safe_token(device);
	const eeprom_config *config;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	config = (const eeprom_config *)device->inline_config;

	c->intf = config->pinterface;

	if ((1 << c->intf->address_bits) * c->intf->data_bits / 8 > MEMORY_SIZE)
	{
		fatalerror("EEPROM larger than eepromdev.c allows");
	}

	memset(c->eeprom_data, 0xff, (1 << c->intf->address_bits) * c->intf->data_bits / 8);
	if ((config->default_data != NULL) && (config->default_data_size != 0))
		eepromdev_set_data(device, config->default_data, config->default_data_size);
	c->serial_count = 0;
	c->latch = 0;
	c->reset_line = ASSERT_LINE;
	c->clock_line = ASSERT_LINE;
	c->eeprom_read_address = 0;
	c->sending = 0;
	if (c->intf->cmd_unlock) c->locked = 1;
	else c->locked = 0;

	state_save_register_device_item_pointer( device, 0, c->eeprom_data, MEMORY_SIZE);
	state_save_register_device_item_pointer( device, 0, c->serial_buffer, SERIAL_BUFFER_LENGTH);
	state_save_register_device_item( device, 0, c->clock_line);
	state_save_register_device_item( device, 0, c->reset_line);
	state_save_register_device_item( device, 0, c->locked);
	state_save_register_device_item( device, 0, c->serial_count);
	state_save_register_device_item( device, 0, c->latch);
	state_save_register_device_item( device, 0, c->reset_delay);
	state_save_register_device_item( device, 0, c->eeprom_clock_count);
	state_save_register_device_item( device, 0, c->eeprom_data_bits);
	state_save_register_device_item( device, 0, c->eeprom_read_address);
}

static DEVICE_RESET(eeprom)
{
}

DEVICE_GET_INFO(eeprom)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(eeprom_state); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(eeprom_config); break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(eeprom); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(eeprom); break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(eeprom); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "EEPROM"); break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "EEPROM"); break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0"); break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
