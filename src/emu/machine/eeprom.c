#include "driver.h"
#include "eeprom.h"

#define VERBOSE 0

#define SERIAL_BUFFER_LENGTH 40
#define MEMORY_SIZE 1024

static struct EEPROM_interface *intf;

static int serial_count;
static UINT8 serial_buffer[SERIAL_BUFFER_LENGTH];
static UINT8 eeprom_data[MEMORY_SIZE];
static int eeprom_data_bits;
static int eeprom_read_address;
static int eeprom_clock_count;
static int latch,reset_line,clock_line,sending;
static int locked;
static int reset_delay;

/*
    EEPROM_command_match:

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
static int EEPROM_command_match(const char *buf, const char *cmd, int len)
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


struct EEPROM_interface eeprom_interface_93C46 =
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

struct EEPROM_interface eeprom_interface_93C66B =
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

NVRAM_HANDLER( 93C46 )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface_93C46);
		if (file)	EEPROM_load(file);
	}
}

NVRAM_HANDLER( 93C66B )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface_93C66B);
		if (file)	EEPROM_load(file);
	}
}

void EEPROM_init(struct EEPROM_interface *interface)
{
	intf = interface;

	if ((1 << intf->address_bits) * intf->data_bits / 8 > MEMORY_SIZE)
	{
		fatalerror("EEPROM larger than eeprom.c allows");
	}

	memset(eeprom_data,0xff,(1 << intf->address_bits) * intf->data_bits / 8);
	serial_count = 0;
	latch = 0;
	reset_line = ASSERT_LINE;
	clock_line = ASSERT_LINE;
	eeprom_read_address = 0;
	sending = 0;
	if (intf->cmd_unlock) locked = 1;
	else locked = 0;

	state_save_register_global_array(eeprom_data);
	state_save_register_global_array(serial_buffer);
	state_save_register_global(clock_line);
	state_save_register_global(reset_line);
	state_save_register_global(locked);
	state_save_register_global(serial_count);
	state_save_register_global(latch);
	state_save_register_global(reset_delay);
	state_save_register_global(eeprom_clock_count);
	state_save_register_global(eeprom_data_bits);
	state_save_register_global(eeprom_read_address);
}

static void EEPROM_write(int bit)
{
#if VERBOSE
logerror("EEPROM write bit %d\n",bit);
#endif

	if (serial_count >= SERIAL_BUFFER_LENGTH-1)
	{
logerror("error: EEPROM serial buffer overflow\n");
		return;
	}

	serial_buffer[serial_count++] = (bit ? '1' : '0');
	serial_buffer[serial_count] = 0;	/* nul terminate so we can treat it as a string */

	if ( (serial_count > intf->address_bits) &&
	      EEPROM_command_match((char*)serial_buffer,intf->cmd_read,strlen((char*)serial_buffer)-intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = serial_count-intf->address_bits;i < serial_count;i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}
		if (intf->data_bits == 16)
			eeprom_data_bits = (eeprom_data[2*address+0] << 8) + eeprom_data[2*address+1];
		else
			eeprom_data_bits = eeprom_data[address];
		eeprom_read_address = address;
		eeprom_clock_count = 0;
		sending = 1;
		serial_count = 0;
logerror("EEPROM read %04x from address %02x\n",eeprom_data_bits,address);
	}
	else if ( (serial_count > intf->address_bits) &&
	           EEPROM_command_match((char*)serial_buffer,intf->cmd_erase,strlen((char*)serial_buffer)-intf->address_bits) )
	{
		int i,address;

		address = 0;
		for (i = serial_count-intf->address_bits;i < serial_count;i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}
logerror("EEPROM erase address %02x\n",address);
		if (locked == 0)
		{
			if (intf->data_bits == 16)
			{
				eeprom_data[2*address+0] = 0x00;
				eeprom_data[2*address+1] = 0x00;
			}
			else
				eeprom_data[address] = 0x00;
		}
		else
logerror("Error: EEPROM is locked\n");
		serial_count = 0;
	}
	else if ( (serial_count > (intf->address_bits + intf->data_bits)) &&
	           EEPROM_command_match((char*)serial_buffer,intf->cmd_write,strlen((char*)serial_buffer)-(intf->address_bits + intf->data_bits)) )
	{
		int i,address,data;

		address = 0;
		for (i = serial_count-intf->data_bits-intf->address_bits;i < (serial_count-intf->data_bits);i++)
		{
			address <<= 1;
			if (serial_buffer[i] == '1') address |= 1;
		}
		data = 0;
		for (i = serial_count-intf->data_bits;i < serial_count;i++)
		{
			data <<= 1;
			if (serial_buffer[i] == '1') data |= 1;
		}
logerror("EEPROM write %04x to address %02x\n",data,address);
		if (locked == 0)
		{
			if (intf->data_bits == 16)
			{
				eeprom_data[2*address+0] = data >> 8;
				eeprom_data[2*address+1] = data & 0xff;
			}
			else
				eeprom_data[address] = data;
		}
		else
logerror("Error: EEPROM is locked\n");
		serial_count = 0;
	}
	else if ( EEPROM_command_match((char*)serial_buffer,intf->cmd_lock,strlen((char*)serial_buffer)) )
	{
logerror("EEPROM lock\n");
		locked = 1;
		serial_count = 0;
	}
	else if ( EEPROM_command_match((char*)serial_buffer,intf->cmd_unlock,strlen((char*)serial_buffer)) )
	{
logerror("EEPROM unlock\n");
		locked = 0;
		serial_count = 0;
	}
}

static void EEPROM_reset(void)
{
if (serial_count)
	logerror("EEPROM reset, buffer = %s\n",serial_buffer);

	serial_count = 0;
	sending = 0;
	reset_delay = intf->reset_delay;	/* delay a little before returning setting data to 1 (needed by wbeachvl) */
}


void EEPROM_write_bit(int bit)
{
#if VERBOSE
logerror("write bit %d\n",bit);
#endif
	latch = bit;
}

int EEPROM_read_bit(void)
{
	int res;

	if (sending)
		res = (eeprom_data_bits >> intf->data_bits) & 1;
	else
	{
		if (reset_delay > 0)
		{
			/* this is needed by wbeachvl */
			reset_delay--;
			res = 0;
		}
		else
			res = 1;
	}

#if VERBOSE
logerror("read bit %d\n",res);
#endif

	return res;
}

void EEPROM_set_cs_line(int state)
{
#if VERBOSE
logerror("set reset line %d\n",state);
#endif
	reset_line = state;

	if (reset_line != CLEAR_LINE)
		EEPROM_reset();
}

void EEPROM_set_clock_line(int state)
{
#if VERBOSE
logerror("set clock line %d\n",state);
#endif
	if (state == PULSE_LINE || (clock_line == CLEAR_LINE && state != CLEAR_LINE))
	{
		if (reset_line == CLEAR_LINE)
		{
			if (sending)
			{
				if (eeprom_clock_count == intf->data_bits && intf->enable_multi_read)
				{
					eeprom_read_address = (eeprom_read_address + 1) & ((1 << intf->address_bits) - 1);
					if (intf->data_bits == 16)
						eeprom_data_bits = (eeprom_data[2*eeprom_read_address+0] << 8) + eeprom_data[2*eeprom_read_address+1];
					else
						eeprom_data_bits = eeprom_data[eeprom_read_address];
					eeprom_clock_count = 0;
logerror("EEPROM read %04x from address %02x\n",eeprom_data_bits,eeprom_read_address);
				}
				eeprom_data_bits = (eeprom_data_bits << 1) | 1;
				eeprom_clock_count++;
			}
			else
				EEPROM_write(latch);
		}
	}

	clock_line = state;
}


void EEPROM_load(mame_file *f)
{
	mame_fread(f,eeprom_data,(1 << intf->address_bits) * intf->data_bits / 8);
}

void EEPROM_save(mame_file *f)
{
	mame_fwrite(f,eeprom_data,(1 << intf->address_bits) * intf->data_bits / 8);
}

void EEPROM_set_data(const UINT8 *data, int length)
{
	memcpy(eeprom_data, data, length);
}

UINT8 * EEPROM_get_data_pointer(int * length)
{
	if(length)
		*length = MEMORY_SIZE;

	return eeprom_data;
}
