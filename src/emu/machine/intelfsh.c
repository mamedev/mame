/*
    Flash ROM emulation

    Explicitly supports:
    Intel 28F016S5 (byte-wide)
    AMD/Fujitsu 29F016 (byte-wide)
    Sharp LH28F400 (word-wide)

    Flash ROMs use a standardized command set accross manufacturers,
    so this emulation should work even for non-Intel and non-Sharp chips
    as long as the game doesn't query the maker ID.
*/

#include "emu.h"
#include "intelfsh.h"

enum
{
	FM_NORMAL,	// normal read/write
	FM_READID,	// read ID
	FM_READSTATUS,	// read status
	FM_WRITEPART1,	// first half of programming, awaiting second
	FM_CLEARPART1,	// first half of clear, awaiting second
	FM_SETMASTER,	// first half of set master lock, awaiting on/off
	FM_READAMDID1,	// part 1 of alt ID sequence
	FM_READAMDID2,	// part 2 of alt ID sequence
	FM_READAMDID3,	// part 3 of alt ID sequence
	FM_ERASEAMD1,	// part 1 of AMD erase sequence
	FM_ERASEAMD2,	// part 2 of AMD erase sequence
	FM_ERASEAMD3,	// part 3 of AMD erase sequence
	FM_ERASEAMD4,	// part 4 of AMD erase sequence
	FM_BYTEPROGRAM,
};

struct flash_chip
{
	int type;
	int size;
	int bits;
	int status;
	int erase_sector;
	int sector_is_4k;
	INT32 flash_mode;
	INT32 flash_master_lock;
	int device_id;
	int maker_id;
	emu_timer *timer;
	void *flash_memory;
};

static struct flash_chip chips[FLASH_CHIPS_MAX];

static TIMER_CALLBACK( erase_finished )
{
	struct flash_chip *c = (struct flash_chip *)ptr;

	switch( c->flash_mode )
	{
	case FM_READSTATUS:
		c->status = 0x80;
		break;

	case FM_ERASEAMD4:
		c->flash_mode = FM_NORMAL;
		break;
	}
}

void* intelflash_getmemptr(int chip)
{
	struct flash_chip *c;
	if( chip >= FLASH_CHIPS_MAX )
	{
		logerror( "intelflash_init: invalid chip %d\n", chip );
		return 0;
	}
	c = &chips[ chip ];

	return c->flash_memory;
}

void intelflash_init(running_machine *machine, int chip, int type, void *data)
{
	struct flash_chip *c;
	if( chip >= FLASH_CHIPS_MAX )
	{
		logerror( "intelflash_init: invalid chip %d\n", chip );
		return;
	}
	c = &chips[ chip ];

	c->type = type;
	c->sector_is_4k = 0;
	switch( c->type )
	{
	case FLASH_INTEL_28F016S5:
	case FLASH_SHARP_LH28F016S:
		c->bits = 8;
		c->size = 0x200000;
		c->maker_id = 0x89;
		c->device_id = 0xaa;
		break;
	case FLASH_SHARP_LH28F400:
	case FLASH_INTEL_E28F400:
		c->bits = 16;
		c->size = 0x80000;
		c->maker_id = 0xb0;
		c->device_id = 0xed;
		break;
	case FLASH_FUJITSU_29F016A:
		c->bits = 8;
		c->size = 0x200000;
		c->maker_id = 0x04;
		c->device_id = 0xad;
		break;
	case FLASH_INTEL_E28F008SA:
		c->bits = 8;
		c->size = 0x100000;
		c->maker_id = 0x89;
		c->device_id = 0xa2;
		break;
	case FLASH_INTEL_TE28F160:
		c->bits = 16;
		c->size = 0x200000;
		c->maker_id = 0xb0;
		c->device_id = 0xd0;
		break;
	case FLASH_SHARP_UNK128MBIT:
		c->bits = 16;
		c->size = 0x800000;
		c->maker_id = 0xb0;
		c->device_id = 0xb0;
		break;
	case FLASH_MACRONIX_29L001MC:
		c->bits = 8;
		c->size = 0x20000;
		c->maker_id = 0xc2;
		c->device_id = 0x51;
		break;

	case FLASH_PANASONIC_MN63F805MNP:
		c->bits = 8;
		c->size = 0x10000;
		c->maker_id = 0x32;
		c->device_id = 0x1b;
		c->sector_is_4k = 1;
		break;

	case FLASH_SANYO_LE26FV10N1TS:
		c->bits = 8;
		c->size = 0x20000;
		c->maker_id = 0x62;
		c->device_id = 0x13;
		c->sector_is_4k = 1;
		break;
	}
	if( data == NULL )
	{
		data = auto_alloc_array( machine, UINT8, c->size );
		memset( data, 0xff, c->size );
	}

	c->status = 0x80;
	c->flash_mode = FM_NORMAL;
	c->flash_master_lock = 0;
	c->timer = timer_alloc(machine, erase_finished, c);
	c->flash_memory = data;

	state_save_register_item( machine, "intelfsh", NULL, chip, c->status );
	state_save_register_item( machine, "intelfsh", NULL, chip, c->flash_mode );
	state_save_register_item( machine, "intelfsh", NULL, chip, c->flash_master_lock );
	state_save_register_memory( machine, "intelfsh", NULL, chip, "flash_memory", c->flash_memory, c->bits/8, c->size / (c->bits/8), __FILE__, __LINE__ );
}

UINT32 intelflash_read(int chip, UINT32 address)
{
	UINT32 data = 0;
	struct flash_chip *c;
	if( chip >= FLASH_CHIPS_MAX )
	{
		logerror( "intelflash_read: invalid chip %d\n", chip );
		return 0;
	}
	c = &chips[ chip ];

	switch( c->flash_mode )
	{
	default:
	case FM_NORMAL:
		switch( c->bits )
		{
		case 8:
			{
				UINT8 *flash_memory = (UINT8 *)c->flash_memory;
				data = flash_memory[ address ];
			}
			break;
		case 16:
			{
				UINT16 *flash_memory = (UINT16 *)c->flash_memory;
				data = flash_memory[ address ];
			}
			break;
		}
		break;
	case FM_READSTATUS:
		data = c->status;
		break;
	case FM_READAMDID3:
		switch (address)
		{
			case 0:	data = c->maker_id; break;
			case 1: data = c->device_id; break;
			case 2: data = 0; break;
		}
		break;
	case FM_READID:
		switch (address)
		{
		case 0:	// maker ID
			data = c->maker_id;
			break;
		case 1:	// chip ID
			data = c->device_id;
			break;
		case 2:	// block lock config
			data = 0; // we don't support this yet
			break;
		case 3: // master lock config
			if (c->flash_master_lock)
			{
				data = 1;
			}
			else
			{
				data = 0;
			}
			break;
		}
		break;
	case FM_ERASEAMD4:
		// reads outside of the erasing sector return normal data
		if ((address < c->erase_sector) || (address >= c->erase_sector+(64*1024)))
		{
			switch( c->bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)c->flash_memory;
					data = flash_memory[ address ];
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)c->flash_memory;
					data = flash_memory[ address ];
				}
				break;
			}
		}
		else
		{
			c->status ^= ( 1 << 6 ) | ( 1 << 2 );
			data = c->status;
		}
		break;
	}

//	logerror( "intelflash_read( %d, %08x ) %08x\n", chip, address, data );

	return data;
}

void intelflash_write(int chip, UINT32 address, UINT32 data)
{
	struct flash_chip *c;
	if( chip >= FLASH_CHIPS_MAX )
	{
		logerror( "intelflash_write: invalid chip %d\n", chip );
		return;
	}
	c = &chips[ chip ];

//	logerror( "intelflash_write( %d, %08x, %08x )\n", chip, address, data );

	switch( c->flash_mode )
	{
	case FM_NORMAL:
	case FM_READSTATUS:
	case FM_READID:
	case FM_READAMDID3:
		switch( data & 0xff )
		{
		case 0xf0:
		case 0xff:	// reset chip mode
			c->flash_mode = FM_NORMAL;
			break;
		case 0x90:	// read ID
			c->flash_mode = FM_READID;
			break;
		case 0x40:
		case 0x10:	// program
			c->flash_mode = FM_WRITEPART1;
			break;
		case 0x50:	// clear status reg
			c->status = 0x80;
			c->flash_mode = FM_READSTATUS;
			break;
		case 0x20:	// block erase
			c->flash_mode = FM_CLEARPART1;
			break;
		case 0x60:	// set master lock
			c->flash_mode = FM_SETMASTER;
			break;
		case 0x70:	// read status
			c->flash_mode = FM_READSTATUS;
			break;
		case 0xaa:	// AMD ID select part 1
			if( ( address & 0xfff ) == 0x555 )
			{
				c->flash_mode = FM_READAMDID1;
			}
			break;
		default:
			logerror( "Unknown flash mode byte %x\n", data & 0xff );
			break;
		}
		break;
	case FM_READAMDID1:
		if( ( address & 0xffff ) == 0x2aa && ( data & 0xff ) == 0x55 )
		{
			c->flash_mode = FM_READAMDID2;
		}
		else if( ( address & 0xffff ) == 0x2aaa && ( data & 0xff ) == 0x55 )
		{
			c->flash_mode = FM_READAMDID2;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_READAMDID1\n", address, data & 0xff );
			c->flash_mode = FM_NORMAL;
		}
		break;
	case FM_READAMDID2:
		if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0x90 )
		{
			c->flash_mode = FM_READAMDID3;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0x90 )
		{
			c->flash_mode = FM_READAMDID3;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0x80 )
		{
			c->flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0x80 )
		{
			c->flash_mode = FM_ERASEAMD1;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0xa0 )
		{
			c->flash_mode = FM_BYTEPROGRAM;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xa0 )
		{
			c->flash_mode = FM_BYTEPROGRAM;
		}
		else if( ( address & 0xffff ) == 0x555 && ( data & 0xff ) == 0xf0 )
		{
			c->flash_mode = FM_NORMAL;
		}
		else if( ( address & 0xffff ) == 0x5555 && ( data & 0xff ) == 0xf0 )
		{
			c->flash_mode = FM_NORMAL;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_READAMDID2\n", address, data & 0xff );
			c->flash_mode = FM_NORMAL;
		}
		break;
	case FM_ERASEAMD1:
		if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0xaa )
		{
			c->flash_mode = FM_ERASEAMD2;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD1\n", address, data & 0xff );
		}
		break;
	case FM_ERASEAMD2:
		if( ( address & 0xffff ) == 0x2aa && ( data & 0xff ) == 0x55 )
		{
			c->flash_mode = FM_ERASEAMD3;
		}
		else if( ( address & 0xffff ) == 0x2aaa && ( data & 0xff ) == 0x55 )
		{
			c->flash_mode = FM_ERASEAMD3;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD2\n", address, data & 0xff );
		}
		break;
	case FM_ERASEAMD3:
		if( ( address & 0xfff ) == 0x555 && ( data & 0xff ) == 0x10 )
		{
			// chip erase
			memset( c->flash_memory, 0xff, c->size);

			c->status = 1 << 3;
			c->flash_mode = FM_ERASEAMD4;

			timer_adjust_oneshot( c->timer, ATTOTIME_IN_SEC( 17 ), 0 );
		}
		else if( ( data & 0xff ) == 0x30 )
		{
			// sector erase
			// clear the 4k/64k block containing the current address to all 0xffs
			switch( c->bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)c->flash_memory;
					if (c->sector_is_4k)
					{
						memset( &flash_memory[ address & ~0xfff ], 0xff, 4 * 1024 );
						c->erase_sector = address & ~0xfff;
						timer_adjust_oneshot( c->timer, ATTOTIME_IN_MSEC( 125 ), 0 );
					}
					else
					{
						memset( &flash_memory[ address & ~0xffff ], 0xff, 64 * 1024 );
						c->erase_sector = address & ~0xffff;
						timer_adjust_oneshot( c->timer, ATTOTIME_IN_SEC( 1 ), 0 );
					}
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)c->flash_memory;
					if (c->sector_is_4k)
					{
				 		memset( &flash_memory[ address & ~0x7ff ], 0xff, 4 * 1024 );
						c->erase_sector = address & ~0x7ff;
						timer_adjust_oneshot( c->timer, ATTOTIME_IN_MSEC( 125 ), 0 );
					}
					else
					{
				 		memset( &flash_memory[ address & ~0x7fff ], 0xff, 64 * 1024 );
						c->erase_sector = address & ~0x7fff;
						timer_adjust_oneshot( c->timer, ATTOTIME_IN_SEC( 1 ), 0 );
					}
				}
				break;
			}

			c->status = 1 << 3;
			c->flash_mode = FM_ERASEAMD4;
		}
		else
		{
			logerror( "unexpected %08x=%02x in FM_ERASEAMD3\n", address, data & 0xff );
		}
		break;
	case FM_BYTEPROGRAM:
		switch( c->bits )
		{
		case 8:
			{
				UINT8 *flash_memory = (UINT8 *)c->flash_memory;
				flash_memory[ address ] = data;
			}
			break;
		default:
			logerror( "FM_BYTEPROGRAM not supported when c->bits == %d\n", c->bits );
			break;
		}
		c->flash_mode = FM_NORMAL;
		break;
	case FM_WRITEPART1:
		switch( c->bits )
		{
		case 8:
			{
				UINT8 *flash_memory = (UINT8 *)c->flash_memory;
				flash_memory[ address ] = data;
			}
			break;
		case 16:
			{
				UINT16 *flash_memory = (UINT16 *)c->flash_memory;
				flash_memory[ address ] = data;
			}
			break;
		default:
			logerror( "FM_WRITEPART1 not supported when c->bits == %d\n", c->bits );
			break;
		}
		c->status = 0x80;
		c->flash_mode = FM_READSTATUS;
		break;
	case FM_CLEARPART1:
		if( ( data & 0xff ) == 0xd0 )
		{
			// clear the 64k block containing the current address to all 0xffs
			switch( c->bits )
			{
			case 8:
				{
					UINT8 *flash_memory = (UINT8 *)c->flash_memory;
					memset( &flash_memory[ address & ~0xffff ], 0xff, 64 * 1024 );
				}
				break;
			case 16:
				{
					UINT16 *flash_memory = (UINT16 *)c->flash_memory;
					memset( &flash_memory[ address & ~0x7fff ], 0xff, 64 * 1024 );
				}
				break;
			default:
				logerror( "FM_CLEARPART1 not supported when c->bits == %d\n", c->bits );
				break;
			}
			c->status = 0x00;
			c->flash_mode = FM_READSTATUS;

			timer_adjust_oneshot( c->timer, ATTOTIME_IN_SEC( 1 ), 0 );
			break;
		}
		else
		{
			logerror( "unexpected %02x in FM_CLEARPART1\n", data & 0xff );
		}
		break;
	case FM_SETMASTER:
		switch( data & 0xff )
		{
		case 0xf1:
			c->flash_master_lock = 1;
			break;
		case 0xd0:
			c->flash_master_lock = 0;
			break;
		default:
			logerror( "unexpected %08x=%02x in FM_SETMASTER:\n", address, data & 0xff );
			break;
		}
		c->flash_mode = FM_NORMAL;
		break;
	}
}

void nvram_handler_intelflash(running_machine *machine, int chip,mame_file *file,int read_or_write)
{
	struct flash_chip *c;
	if( chip >= FLASH_CHIPS_MAX )
	{
		logerror( "intelflash_nvram: invalid chip %d\n", chip );
		return;
	}
	c = &chips[ chip ];

	switch( c->bits )
	{
	case 8:
		if (read_or_write)
		{
			mame_fwrite( file, c->flash_memory, c->size );
		}
		else if (file)
		{
			mame_fread( file, c->flash_memory, c->size );
		}
		break;
	case 16:
		if (read_or_write)
		{
			/// TODO: endian conversion
			mame_fwrite( file, c->flash_memory, c->size );
		}
		else if (file)
		{
			mame_fread( file, c->flash_memory, c->size );
			/// TODO: endian conversion
		}
		break;
	}
}
