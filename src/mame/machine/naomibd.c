/*************************************************************************

    Naomi plug-in board emulator

    emulator by Samuele Zannoli
    protection chip by R. Belmont, reverse engineering by Deunan Knute

**************************************************************************/

/*
    Naomi ROM board info from ElSemi:

    NAOMI_ROM_OFFSETH = 0x5f7000,
    NAOMI_ROM_OFFSETL = 0x5f7004,
    NAOMI_ROM_DATA = 0x5f7008,
    NAOMI_DMA_OFFSETH = 0x5f700C,
    NAOMI_DMA_OFFSETL = 0x5f7010,
    NAOMI_DMA_COUNT = 0x5f7014,
    NAOMI_COMM_OFFSET = 0x5F7050,
    NAOMI_COMM_DATA = 0x5F7054,
    NAOMI_BOARDID_WRITE = 0x5F7078,
    NAOMI_BOARDID_READ = 0x5F707C,
    each port is 16 bit wide, to access the rom in PIO mode, just set an offset in ROM_OFFSETH/L and read from ROM_DATA, each access reads 2 bytes and increases the offset by 2.

    the BOARDID regs access the password protected eeprom in the game board. the main board eeprom is read through port 0x1F800030

    To access the board using DMA, use the DMA_OFFSETL/H. DMA_COUNT is in units of 0x20 bytes. Then trigger a GDROM DMA request.

    Dimm board registers (add more information if you find it):

    Name:                   Naomi   Dimm Bd.
    NAOMI_DIMM_COMMAND    = 5f703c  14000014 (16 bit):
        if bits all 1 no dimm board present and other registers not used
        bit 15: during an interrupt is 1 if the dimm board has a command to be executed
        bit 14-9: 6 bit command number (naomi bios understands 0 1 3 4 5 6 8 9 a)
        bit 7-0: higher 8 bits of 24 bit offset parameter
    NAOMI_DIMM_OFFSETL    = 5f7040  14000018 (16 bit):
        bit 15-0: lower 16 bits of 24 bit offset parameter
    NAOMI_DIMM_PARAMETERL = 5f7044  1400001c (16 bit)
    NAOMI_DIMM_PARAMETERH = 5f7048  14000020 (16 bit)
    NAOMI_DIMM_STATUS     = 5f704c  14000024 (16 bit):
        bit 0: when 0 signal interrupt from naomi to dimm board
        bit 8: when 0 signal interrupt from dimm board to naomi


    Cartridge protection info from Deunan Knute:

    NAOMI cart can hold up to 256MB of data (well, 512 is possible too I guess), so the highest bits are used for other, dark and scary purposes.
    I call those bits "mode selector".

    First it's important to note that DMA and PIO seem to have separate address counters, as well as separate mode selector registers.

    * bit 31 (mode bit 3) is auto-advance bit
    When set to one the address will be automatically incremented when data is read, so you need only set it once and can just keep polling
    the PIO port. When zero it will stay on current address.  Now this works exactly the same for DMA, and even if DMA engine is 32-byte
    per block it will repeatedly read only the first 16-bit word.

    * bit 30 (mode bit 2) is most often as special mode switch
    DMA transfer with this bit set will hang. PIO will return semi-random data (floating bus?). So one function of that bit is "disable".
    PIO read will return all ones if DMA mode has this bit cleared, so it seems you can do either PIO or DMA but not both at the same time.
    In other words, disable DMA once before using PIO (most games using both access types do that when the DMA terminates).
    This bit is also used to reset the chip's internal protection mechanism on "Oh! My Goddess" to a known state.

    * bit 29 (mode bit 1) is address shuffle bit
    It's actually the opposite, when set the addressing is following the chip layout and when cleared the protection chip will have it's fun
    with address lines 10 to 23(?). It's not a simple swap function, rather a lookup table and one with repeating results too.
    The few games I got to work never made any use of that bit, it's always set for all normal reads.

    * bit 28 (mode bit 0) is unused (so far)
    Or it could really be the last address bit to allow up to 512MB of data on a cart?

    Normal address starts with 0xa0000000 to enable auto-advance and standard addressing mode.
*/

// NOTE: all accesses are 16 or 32 bits wide but only 16 bits are valid

#include "driver.h"
#include "eminline.h"
#include "profiler.h"
#include "machine/eeprom.h"
#include "machine/x76f100.h"
#include "cdrom.h"
#include "naomibd.h"

#define NAOMIBD_FLAG_AUTO_ADVANCE	(8)	// address auto-advances on read
#define NAOMIBD_FLAG_SPECIAL_MODE	(4)	// used to access protection registers
#define NAOMIBD_FLAG_ADDRESS_SHUFFLE	(2)	// 0 to let protection chip scramble address lines, 1 for normal

/*************************************
 *
 *  Prototypes
 *
 *************************************/

extern void naomi_game_decrypt(running_machine* machine, UINT64 key, UINT8* region, int length);


/*************************************
 *
 *  Structures
 *
 *************************************/

#define MAX_PROT_REGIONS	(32)

typedef struct _naomibd_config_table naomibd_config_table;
struct _naomibd_config_table
{
	const char *name;
	UINT32	transtbl[MAX_PROT_REGIONS*2];
};

typedef struct _naomibd_state naomibd_state;
struct _naomibd_state
{
	UINT8				index;					/* index of board */
	UINT8				type;
	const device_config *device;				/* pointer to our containing device */

	UINT8 *				memory;
	UINT8 *				protdata;
	chd_file *			gdromchd;
	UINT8 *				picdata;
	UINT32				rom_offset, rom_offset_flags, dma_count;
	UINT32				dma_offset, dma_offset_flags;
	UINT32				prot_offset, prot_key;

	UINT32				*prot_translate;
};

// maps protection offsets to real addresses
static naomibd_config_table naomibd_translate_tbl[] =
{
	{ "doa2", { 0x500, 0, 0x20504, 0x20000, 0x40508, 0x40000, 0x6050c, 0x60000, 0x80510, 0x80000,
		    0xa0514, 0xa0000, 0xc0518, 0xc0000, 0xe051c, 0xe0000, 0x100520,0x100000, 0x118a3a, 0x120000,
		    0x12c0d8, 0x140000, 0x147e22, 0x160000, 0x1645ce, 0x180000, 0x17c6b2, 0x1a0000,
		    0x19902e, 0x1c0000, 0x1b562a, 0x1e0000, 0xffffffff, 0xffffffff } },
	{ "doa2m", { 0x500, 0, 0x20504, 0x20000, 0x40508, 0x40000, 0x6050c, 0x60000, 0x80510, 0x80000,
		    0xa0514, 0xa0000, 0xc0518, 0xc0000, 0xe051c, 0xe0000, 0x100520,0x100000, 0x11a5b4, 0x120000,
		    0x12e7c4, 0x140000, 0x1471f6, 0x160000, 0x1640c4, 0x180000, 0x1806ca, 0x1a0000,
		    0x199df4, 0x1c0000, 0x1b5d0a, 0x1e0000, 0xffffffff, 0xffffffff } },
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a naomibd device
-------------------------------------------------*/

INLINE naomibd_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == NAOMI_BOARD);

	return (naomibd_state *)device->token;
}



/*************************************
 *
 *  Misc. functions
 *
 *************************************/

int naomibd_interrupt_callback(const device_config *device, naomibd_interrupt_func callback)
{
	naomibd_config *config = (naomibd_config *)device->inline_config;
	//naomibd_state *v = get_safe_token(device);

	config->interrupt = callback;
	return 0;
}

int naomibd_get_type(const device_config *device)
{
	naomibd_state *v = get_safe_token(device);
	return v->type;
}



/*************************************
 *
 *  Common initialization
 *
 *************************************/



static STATE_POSTLOAD( naomibd_postload )
{
	//naomibd_state *v = param;
}


static void init_save_state(const device_config *device)
{
	naomibd_state *v = get_safe_token(device);

	state_save_register_postload(device->machine, naomibd_postload, v);

	/* register states */
	state_save_register_device_item(device, 0, v->rom_offset);
	state_save_register_device_item(device, 0, v->rom_offset_flags);
	state_save_register_device_item(device, 0, v->dma_count);
	state_save_register_device_item(device, 0, v->dma_offset);
	state_save_register_device_item(device, 0, v->dma_offset_flags);
	state_save_register_device_item(device, 0, v->prot_offset);
	state_save_register_device_item(device, 0, v->prot_key);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static void soft_reset(naomibd_state *v)
{
}



/*************************************
 *
 *  Handlers
 *
 *************************************/

READ64_DEVICE_HANDLER( naomibd_r )
{
	naomibd_state *v = get_safe_token(device);
	UINT8 *ROM = (UINT8 *)v->memory;

	// ROM_DATA
	if ((offset == 1) && ACCESSING_BITS_0_15)
	{
		UINT64 ret = 0;

		if (v->rom_offset_flags & NAOMIBD_FLAG_SPECIAL_MODE)
		{
			if (v->rom_offset == 0x1fffe)
			{
				UINT8 *prot = (UINT8 *)v->protdata;
				UINT32 byte_offset = v->prot_offset*2;

				if (!prot)
				{
					logerror("naomibd: reading protection data, but none was supplied\n");
					return 0;
				}

				ret = (UINT64)(prot[byte_offset] | (prot[byte_offset+1]<<8));

				v->prot_offset++;
			}
			else
			{
				mame_printf_verbose("Bad protection offset read %x\n", v->rom_offset);
			}
		}
		else
		{
			ret = (UINT64)(ROM[v->rom_offset] | (ROM[v->rom_offset+1]<<8));
		}

		if (v->rom_offset_flags & NAOMIBD_FLAG_AUTO_ADVANCE)
		{
			v->rom_offset += 2;
		}

		return ret;
	}
	else if ((offset == 7) && ACCESSING_BITS_32_47)
	{
		// 5f703c
		mame_printf_verbose("ROM: read 5f703c\n");
		return (UINT64)0xffff << 32;
	}
	else if ((offset == 8) && ACCESSING_BITS_0_15)
	{
		// 5f7040
		mame_printf_verbose("ROM: read 5f7040\n");
		return 0;
	}
	else if ((offset == 8) && ACCESSING_BITS_32_47)
	{
		// 5f7044
		mame_printf_verbose("ROM: read 5f7044\n");
		return 0;
	}
	else if ((offset == 9) && ACCESSING_BITS_0_15)
	{
		// 5f7048
		mame_printf_verbose("ROM: read 5f7048\n");
		return 0;
	}
	else if ((offset == 9) && ACCESSING_BITS_32_47)
	{
		// 5f704c
		mame_printf_verbose("ROM: read 5f704c\n");
		return (UINT64)1 << 32;
	}
	else if ((offset == 15) && ACCESSING_BITS_32_47) // boardid read
	{
		UINT64 ret;

		ret = x76f100_sda_read( device->machine, 0 ) << 15;

		return ret << 32;
	}
	else
	{
		//mame_printf_verbose("%s:ROM: read mask %llx @ %x\n", cpuexec_describe_context(machine), mem_mask, offset);
	}

	return U64(0xffffffffffffffff);
}

WRITE64_DEVICE_HANDLER( naomibd_w )
{
	naomibd_state *v = get_safe_token(device);
	INT32 i;

	switch(offset)
	{
		case 0:
		{
			if(ACCESSING_BITS_0_15)
			{
				// ROM_OFFSETH
				v->rom_offset &= 0xffff;
				v->rom_offset |= (data & 0x1fff)<<16;
				v->rom_offset_flags = data >> 12;
			}
			if(ACCESSING_BITS_32_47)
			{
				// ROM_OFFSETL
				v->rom_offset &= 0xffff0000;
				v->rom_offset |= ((data >> 32) & 0xffff);
			}
		}
		break;
		case 1:
		{
			if(ACCESSING_BITS_32_47 || ACCESSING_BITS_32_63)
			{
				// DMA_OFFSETH
				v->dma_offset &= 0xffff;
				v->dma_offset |= (data >> 16) & 0x1fff0000;
				v->dma_offset_flags = (data>>28);
			}
			if(ACCESSING_BITS_0_15)
			{
				// ROM_DATA - used to access registers in the protection chip
				switch (v->rom_offset)
				{
					case 0x1fff8:	// offset low
						v->prot_offset &= 0xffff0000;
						v->prot_offset |= (UINT32)data;
						break;

					case 0x1fffa:	// offset high
						v->prot_offset &= 0xffff;
						v->prot_offset |= (UINT32)data<<16;
						break;

					case 0x1fffc:	// decryption key
						v->prot_key = data;

						mame_printf_verbose("Protection: set up read @ %x, key %x [%s]\n", v->prot_offset, v->prot_key, cpuexec_describe_context(device->machine));

						// translate address if necessary
						if (v->prot_translate != NULL)
						{
							i = 0;
							while (v->prot_translate[i] != 0xffffffff)
							{
								if (v->prot_translate[i] == (v->prot_offset*2))
								{
									mame_printf_verbose("Protection: got offset %x, translated to %x\n", v->prot_offset, v->prot_translate[i+1]);
									v->prot_offset = v->prot_translate[i+1]/2;
									break;
								}
								else
								{
									i += 2;
								}
							}
						}
						break;

					default:
						mame_printf_verbose("naomibd: unknown protection write %x @ %x\n", (UINT32)data, offset);
						break;
				}
			}
		}
		break;
		case 2:
		{
			if(ACCESSING_BITS_0_15)
			{
				// DMA_OFFSETL
				v->dma_offset &= 0xffff0000;
				v->dma_offset |= (data & 0xffff);
			}
			if(ACCESSING_BITS_32_63)
			{
				// NAOMI_DMA_COUNT
				v->dma_count = data >> 32;
			}
		}
		break;
		case 7:
		{
			if(ACCESSING_BITS_32_47)
				mame_printf_verbose("ROM: write 5f703c\n");
		}
		break;
		case 8:
		{
			if(ACCESSING_BITS_0_15)
				mame_printf_verbose("ROM: write 5f7040\n");
			if(ACCESSING_BITS_32_47)
				mame_printf_verbose("ROM: write 5f7044\n");
		}
		break;
		case 9:
		{
			if(ACCESSING_BITS_0_15)
				mame_printf_verbose("ROM: write 5f7048\n");
			if(ACCESSING_BITS_32_47)
				mame_printf_verbose("ROM: write 5f704c\n");
		}
		break;
		case 15:
		{
			if(ACCESSING_BITS_0_15)
			{
				running_machine *machine = device->machine;

				// NAOMI_BOARDID_WRITE
				x76f100_cs_write(machine, 0, (data >> 2) & 1 );
				x76f100_rst_write(machine, 0, (data >> 3) & 1 );
				x76f100_scl_write(machine, 0, (data >> 1) & 1 );
				x76f100_sda_write(machine, 0, (data >> 0) & 1 );
			}
		}
		break;
		default:
			mame_printf_verbose("%s: ROM: write %llx to %x, mask %llx\n", cpuexec_describe_context(device->machine), data, offset, mem_mask);
			break;
	}
}



/*************************************
 *
 *  Load rom file from gdrom
 *
 *************************************/

#define FILENAME_LENGTH 16

static void load_rom_gdrom(running_machine* machine, naomibd_state *v)
{
	UINT32 result;
	cdrom_file *gdromfile;
	UINT8 buffer[2048];
	UINT8 *ptr;
	UINT32 start,size,sectors,dir;
	int pos,len,a;
	char name[128];
	UINT64 key;

	memset(name,'\0',128);
	memcpy(name, v->picdata+33, 7);
	memcpy(name+7, v->picdata+25, 7);
	gdromfile = cdrom_open(v->gdromchd);
	// primary volume descriptor
	// read frame 0xb06e (frame=sector+150)
	// dimm board firmware starts straight from this frame
	result = cdrom_read_data(gdromfile, 0xb06e - 150, buffer, CD_TRACK_MODE1);
	start=((buffer[0x8c+0] << 0) |
		   (buffer[0x8c+1] << 8) |
		   (buffer[0x8c+2] << 16) |
		   (buffer[0x8c+3] << 24));
	// path table
	result = cdrom_read_data(gdromfile, start, buffer, CD_TRACK_MODE1);
	start=((buffer[0x2+0] << 0) |
		   (buffer[0x2+1] << 8) |
		   (buffer[0x2+2] << 16) |
		   (buffer[0x2+3] << 24));
	dir = start;
	// directory
	result = cdrom_read_data(gdromfile, dir, buffer, CD_TRACK_MODE1);
	// find data of file
	start = 0;
	size = 0;
	printf("Looking for file %s\n", name);
	for (pos = 0;pos < 2048;pos += buffer[pos])
	{
		a=0;
		if (!(buffer[pos+25] & 2))
		{
			len=buffer[pos+32];
			for (a=0;a < FILENAME_LENGTH;a++)
			{
				if ((buffer[pos+33+a] == ';') && (name[a] == 0))
				{
					a=FILENAME_LENGTH+1;
					break;
				}
				if (buffer[pos+33+a] != name[a])
					break;
				if (a == len)
				{
					if (name[a] == 0)
						a = FILENAME_LENGTH+1;
					else
						a = FILENAME_LENGTH;
				}
			}
		}
		if (a == FILENAME_LENGTH+1)
		{
			// start sector and size of file
			start=((buffer[pos+2] << 0) |
				   (buffer[pos+3] << 8) |
				   (buffer[pos+4] << 16) |
				   (buffer[pos+5] << 24));
			size =((buffer[pos+10] << 0) |
				   (buffer[pos+11] << 8) |
				   (buffer[pos+12] << 16) |
				   (buffer[pos+13] << 24));

			printf("start %08x size %08x\n", start,size);
			break;
		}
		if (buffer[pos] == 0)
			break;
	}

	if ((start != 0) && (size == 0x100))
	{
		// read file
		result = cdrom_read_data(gdromfile, start, buffer, CD_TRACK_MODE1);
		// get "rom" file name
		memset(name,'\0', 128);
		memcpy(name, buffer+0xc0, FILENAME_LENGTH-1);


		// directory
		result = cdrom_read_data(gdromfile, dir, buffer, CD_TRACK_MODE1);
		// find data of "rom" file
		start = 0;
		size = 0;

		printf("Looking for file %s\n", name);
		for (pos = 0;pos < 2048;pos += buffer[pos])
		{
			a = 0;
			if (!(buffer[pos+25] & 2))
			{
				len = buffer[pos+32];
				for (a=0;a < FILENAME_LENGTH;a++)
				{
					if ((buffer[pos+33+a] == ';') && (name[a] == 0))
					{
						a=FILENAME_LENGTH+1;
						break;
					}
					if (buffer[pos+33+a] != name[a])
						break;
					if (a == len)
					{
						if (name[a] == 0)
							a = (FILENAME_LENGTH+1);
						else
							a = FILENAME_LENGTH;
					}
				}
			}
			if (a == (FILENAME_LENGTH+1))
			{
				// start sector and size of file
				start=((buffer[pos+2] << 0) |
					   (buffer[pos+3] << 8) |
					   (buffer[pos+4] << 16) |
					   (buffer[pos+5] << 24));
				size =((buffer[pos+10] << 0) |
					   (buffer[pos+11] << 8) |
					   (buffer[pos+12] << 16) |
					   (buffer[pos+13] << 24));

				printf("start %08x size %08x\n", start,size);
				break;
			}
			if (buffer[pos] == 0)
				break;
		}
		if (start != 0)
		{
			// read encrypted data into memory
			ptr = v->memory;
			sectors = (size+2047)/2048;
			while (sectors > 0)
			{
				result = cdrom_read_data(gdromfile, start, ptr, CD_TRACK_MODE1);
				ptr += 2048;
				start++;
				sectors--;
			}
		}
	}
	// get des key
	key =(((UINT64)v->picdata[0x31] << 56) |
		  ((UINT64)v->picdata[0x32] << 48) |
		  ((UINT64)v->picdata[0x33] << 40) |
		  ((UINT64)v->picdata[0x34] << 32) |
		  ((UINT64)v->picdata[0x35] << 24) |
		  ((UINT64)v->picdata[0x36] << 16) |
		  ((UINT64)v->picdata[0x37] << 8)  |
		  ((UINT64)v->picdata[0x29] << 0));

	printf("key is %08x%08x\n", (UINT32)((key & 0xffffffff00000000ULL)>>32), (UINT32)(key & 0x00000000ffffffffULL));

	// decrypt loaded data
	naomi_game_decrypt(machine, key, v->memory, size);
	cdrom_close(gdromfile);
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( naomibd )
{
	const naomibd_config *config = (const naomibd_config *)device->inline_config;
	naomibd_state *v = get_safe_token(device);
	int i;

	/* validate some basic stuff */
	assert(device->static_config == NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* validate configuration */
	assert(config->type >= ROM_BOARD && config->type < MAX_NAOMIBD_TYPES);

	/* store a pointer back to the device */
	v->device = device;

	/* find the protection address translation for this game */
	v->prot_translate = (UINT32 *)0;
	for (i=0; i<ARRAY_LENGTH(naomibd_translate_tbl); i++)
	{
		if (!strcmp(device->machine->gamedrv->name, naomibd_translate_tbl[i].name))
		{
			v->prot_translate = &naomibd_translate_tbl[i].transtbl[0];
			break;
		}
	}

	/* configure type-specific values */
	switch (config->type)
	{
		case ROM_BOARD:
			v->memory = (UINT8 *)memory_region(device->machine, config->regiontag);
			v->protdata = (UINT8 *)memory_region(device->machine, "naomibd_prot");
			break;

		case DIMM_BOARD:
			v->memory = (UINT8 *)memory_region(device->machine, config->regiontag);
			v->gdromchd = get_disk_handle(config->gdromregiontag);
			v->picdata = (UINT8 *)memory_region(device->machine, config->picregiontag);
			load_rom_gdrom(device->machine, v);
			break;

		default:
			fatalerror("Unsupported plug-in board in naomibd_start!");
			break;
	}

	/* set the type */
	v->index = device_list_index(device->machine->config->devicelist, device->type, device->tag);
	v->type = config->type;

	/* initialize some registers */
	v->rom_offset = 0;
	v->rom_offset_flags = 0;
	v->dma_count = 0;
	v->dma_offset = 0;
	v->dma_offset_flags = 0;
	v->prot_offset = 0;

	/* do a soft reset to reset everything else */
	soft_reset(v);

	/* register for save states */
	init_save_state(device);
}


/*-------------------------------------------------
    device exit callback
-------------------------------------------------*/

static DEVICE_STOP( naomibd )
{
	//naomibd_state *v = get_safe_token(device);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( naomibd )
{
	naomibd_state *v = get_safe_token(device);
	soft_reset(v);
}


/*-------------------------------------------------
    device nvram callback
-------------------------------------------------*/


static DEVICE_NVRAM( naomibd )
{
	//naomibd_state *v = get_safe_token(device);
	static UINT8 eeprom_romboard[20+48] = {0x19,0x00,0xaa,0x55,0,0,0,0,0,0,0,0,0x69,0x79,0x68,0x6b,0x74,0x6d,0x68,0x6d};
	UINT8 *games_contents;

	if (read_or_write)
		/*eeprom_save(file)*/;
	else
	{
		/*if (file)
            eeprom_load(file);
        else*/
		games_contents = memory_region(device->machine, "naomibd_eeprom");

		if (games_contents)
		{
			x76f100_init( device->machine, 0, games_contents );
		}
		else
		{
			x76f100_init( device->machine, 0, eeprom_romboard );
			memcpy(eeprom_romboard+20,"\241\011                              0000000000000000",48);
		}

	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( naomibd )
{
	const naomibd_config *config = (device != NULL) ? (const naomibd_config *)device->inline_config : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(naomibd_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(naomibd_config);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;
		case DEVINFO_INT_DMAOFFSET:				info->i = get_safe_token(device)->dma_offset;	break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = NULL;							break;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = NULL;					break;
		case DEVINFO_PTR_MEMORY:				info->p = get_safe_token(device)->memory;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(naomibd);		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(naomibd);			break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(naomibd);		break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(naomibd);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:
			switch (config->type)
			{
				default:
				case ROM_BOARD:					strcpy(info->s, "Naomi Rom Board");				break;
				case DIMM_BOARD:				strcpy(info->s, "Naomi Dimm Board");			break;
			}
			break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Naomi plug-in board");			break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

