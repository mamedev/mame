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

------------------------

Atomiswave ROM board specs from Cah4e3 @ http://cah4e3.wordpress.com/2009/07/26/some-atomiswave-info/

 AW_EPR_OFFSETL                                          Register addres: 0x5f7000
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                         EPR data offset low word                              |
 +-------------------------------------------------------------------------------+

 AW_EPR_OFFSETH                                          Register addres: 0x5f7004
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                          EPR data offset hi word                              |
 +-------------------------------------------------------------------------------+

  Both low and high words of 32-bit offset from start of EPR-ROM area. Used for
  reading header and programm code data, cannot be used for reading MPR-ROMs data.

 AW_MPR_RECORD_INDEX                                     Register addres: 0x5f700c
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                          File system record index                             |
 +-------------------------------------------------------------------------------+

  This register contains index of MPR-ROM file system record (64-bytes in size) to
  read throught DMA. Internal DMA offset register is assigned as AW_MPR_RECORD_INDEX<<6
  from start of MPR-ROM area. Size of DMA transaction not limited, it is possible
  to read any number of records or just part of it.

 AW_MPR_FIRST_FILE_INDEX                                 Register addres: 0x5f7010
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                           First file record index                             |
 +-------------------------------------------------------------------------------+

  This register assign for internal cart circuit index of record in MPR-ROM file
  system sub-area that contain information about first file of MPR-ROM files
  sub-area. Internal circuit using this record to read absolute first file offset
  from start of MPR-ROM area and calculate normal offset for each other file
  requested, since MPR-ROM file data sub-area can be assighed only with relative
  offsets from start of such sub-area.

 AW_MPR_FILE_OFFSETL                                     Register addres: 0x5f7014
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                         MPR file offset low word                              |
 +-------------------------------------------------------------------------------+

 AW_MPR_FILE_OFFSETH                                     Register addres: 0x5f7018
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                          MPR file offset hi word                              |
 +-------------------------------------------------------------------------------+

  Both low and high words of 32-bit relative offset from start of MPR-ROM files
  sub-area. Used by internal circuit to calculate absolute offset using data
  from AW_MPR_FIRST_FILE_INDEX register. Cannot be used for reading EPR-ROM
  data nor even MPR-ROM file system sub-area data.

 In short:

     EPR-ROM
 +--------------+ 0x00000000
 |              |
 |    HEADER    +- AW_EPR_OFFSET << 1
 |              |
 +--------------+
 |              |
 |     CODE     +- AW_EPR_OFFSET << 1
 |              |
 |              |
 +--------------+ 0x007fffff

     MPR-ROMS
 +--------------+ 0x00000000
 | FS_HEADER    |
 | FS_RECORD[1] +- (AW_MPR_RECORD_INDEX << 6)
 | FS_RECORD[2] |
 | FS_RECORD[3] +- (AW_MPR_FIRST_FILE_INDEX << 6)
 |     ...      |
 | FS_RECORD[N] |
 +--------------+- FS_RECORD[AW_MPR_FIRST_FILE_INDEX].FILE_ABS_OFFSET
 | FILE_0       |
 | FILE_1       +- (AW_MPR_FILE_OFFSET << 1) + FS_RECORD[AW_MPR_FIRST_FILE_INDEX].FILE_ABS_OFFSET
 |     ...      |
 | FILE_N       |
 +--------------+ 0x07ffffff

*/

// NOTE: all accesses are 16 or 32 bits wide but only 16 bits are valid

#include "emu.h"
#include "profiler.h"
#include "machine/x76f100.h"
#include "cdrom.h"
#include "includes/naomi.h"
#include "includes/naomibd.h"

#define NAOMIBD_FLAG_AUTO_ADVANCE	(8)	// address auto-advances on read
#define NAOMIBD_FLAG_SPECIAL_MODE	(4)	// used to access protection registers
#define NAOMIBD_FLAG_ADDRESS_SHUFFLE	(2)	// 0 to let protection chip en/decrypt, 1 for normal

#define NAOMIBD_PRINTF_PROTECTION	(0)	// 1 to printf protection access details

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
	int reverse_bytes;
	int live_key;
	UINT32	transtbl[MAX_PROT_REGIONS*3];
};

typedef struct _naomibd_state naomibd_state;
struct _naomibd_state
{
	UINT8				index;					/* index of board */
	UINT8				type;
	running_device *device;				/* pointer to our containing device */

	UINT8 *				memory;
	UINT8 *				protdata;
	chd_file *			gdromchd;
	UINT8 *				picdata;
	UINT32				rom_offset, rom_offset_flags, dma_count;
	UINT32				dma_offset, dma_offset_flags;
	UINT32				prot_offset, prot_key;
	UINT32				aw_offset, aw_file_base, aw_file_offset;

	INT32				prot_sum;

	const UINT32			*prot_translate;
	int				prot_reverse_bytes;

	// live decrypt vars
	UINT32				dc_gamekey, dc_seqkey, dc_seed;
	UINT8				dc_cart_ram[128*1024];	// internal cartridge RAM
	INT32				dc_m3_ptr, dc_m2_ptr, dc_readback;

	#if NAOMIBD_PRINTF_PROTECTION
	int				prot_pio_count;
	#endif
};

// maps protection offsets to real addresses
// format of array: encryption key, address written, address to switch out with.  if key is -1 it's ignored and address written is the match.
// if key is not -1, it's used for the match instead of the address written.
static const naomibd_config_table naomibd_translate_tbl[] =
{
	// games where on-the-fly decryption works (many of these are fully playable in MAME, just slow)
	{ "18wheelr", 0, 0x7cf54, { 0x1502, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "alpilota", 0, 0x70e41, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "alpiltdx", 0, 0x70e41, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "capsnk", 0, 0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "capsnka", 0, 0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "crzytaxi", 0, 0xd2f45, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "csmash", 1, 0x03347, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "csmasho", 1, 0x03347, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "cspike", 0, 0xe2010, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "deathcox", 0, 0xb64d0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "dybb99", 0, 0x48a01, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "f355twin", 0, 0x6efd4, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "f355twn2", 0, 0x666c6, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "ggram2", 0, 0x74a61, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "gwing2",  0, 0xb25d0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "hmgeo",   0, 0x38510, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "jambo",    0, 0xfab95, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "otrigger", 0, 0xfea94, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "pjustic", 0, 0x725d0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "pstone", 0, 0xe69c1, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "pstone2", 0, 0xb8dc0, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "samba", 0, 0xa8b5d, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "slasho", 0, 0xa66ca, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "smlg99", 0, 0x48a01, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "spawn", 0, 0x78d01, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "suchie3", 0, 0x368e1, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "toyfight", 0, 0x2ca85, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "vtennis", 0, 0x3eb15, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "wwfroyal",0, 0x627c3, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "zombrvn", 0, 0x12b41, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },

	// games where the encryption is stacked with the ASIC's compression
	{ "doa2", 0, -1, { -1, 0x500, 0, -1, 0x20504, 0x20000, -1, 0x40508, 0x40000, -1, 0x6050c, 0x60000, -1, 0x80510, 0x80000,	// 0x8ad01, has compression
		    -1, 0xa0514, 0xa0000, -1, 0xc0518, 0xc0000, -1, 0xe051c, 0xe0000, -1, 0x100520,0x100000, -1, 0x118a3a, 0x120000,
		    -1, 0x12c0d8, 0x140000, -1, 0x147e22, 0x160000, -1, 0x1645ce, 0x180000, -1, 0x17c6b2, 0x1a0000,
		    -1, 0x19902e, 0x1c0000, -1, 0x1b562a, 0x1e0000, -1, 0xffffffff, 0xffffffff } },
	{ "doa2m", 0, -1, { -1, 0x500, 0, -1, 0x20504, 0x20000, -1, 0x40508, 0x40000, -1, 0x6050c, 0x60000, -1, 0x80510, 0x80000,
		    -1, 0xa0514, 0xa0000, -1, 0xc0518, 0xc0000, -1, 0xe051c, 0xe0000, -1, 0x100520,0x100000, -1, 0x11a5b4, 0x120000,
		    -1, 0x12e7c4, 0x140000, -1, 0x1471f6, 0x160000, -1, 0x1640c4, 0x180000, -1, 0x1806ca, 0x1a0000,
		    -1, 0x199df4, 0x1c0000, -1, 0x1b5d0a, 0x1e0000, 0xffffffff, 0xffffffff } },
	{ "ggx",      0, -1, { -1, 0x200000, 0x100000, -1, 0x210004, 0x110000, -1, 0x220008, 0x120000, -1, 0x228000, 0x130000,	// 0x76110, uses compression
		          0x3af9, 0, 0x000000, 0x2288, 0, 0x010000, 0xe5e6, 0, 0x020000, 0xebb0, 0, 0x030000,
			  0x0228, 0, 0x040000, 0x872c, 0, 0x050000, 0xbba0, 0, 0x060000, 0x772f, 0, 0x070000,
			  0x2924, 0, 0x080000, 0x3222, 0, 0x090000, 0x7954, 0, 0x0a0000, 0x5acd, 0, 0x0b0000,
			  0xdd19, 0, 0x0c0000, 0x2428, 0, 0x0d0000, 0x3329, 0, 0x0e0000, 0x2142, 0, 0x0f0000,
		          0xffffffff, 0xffffffff, 0xffffffff } },
	{ "sgtetris", 0, -1, { 0x1234, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },	// 0x8ae51, uses compression
//  { "virnbao", 0, 0x68b58, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },     // note: "virnba" set doesn't have protection
//  { "vs2_2k", 0, 0x88b08, { 0, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
};

// forward declaration for decrypt function
static void stream_decrypt(UINT32 game_key, UINT32 sequence_key, UINT16 seed, UINT8* ciphertext, UINT8* plaintext, int length);
static UINT16 block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a naomibd device
-------------------------------------------------*/

INLINE naomibd_state *get_safe_token(running_device *device)
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

int naomibd_interrupt_callback(running_device *device, naomibd_interrupt_func callback)
{
	naomibd_config *config = (naomibd_config *)device->baseconfig().inline_config;
	//naomibd_state *v = get_safe_token(device);

	config->interrupt = callback;
	return 0;
}

int naomibd_get_type(running_device *device)
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


static void init_save_state(running_device *device)
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
	state_save_register_device_item(device, 0, v->aw_offset);
	state_save_register_device_item(device, 0, v->aw_file_base);
	state_save_register_device_item(device, 0, v->aw_file_offset);
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static void soft_reset(naomibd_state *v)
{
	v->prot_sum = 0;

	v->dc_m3_ptr = 0;
	v->dc_seqkey = 0;
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

	// AW board is different, shouldn't ever be read
	if (v->type == AW_ROM_BOARD)
	{
		mame_printf_debug("AW_ROM_BOARD read @ %x mask %" I64FMT "x\n", offset, mem_mask);
		return U64(0xffffffffffffffff);
	}

	// ROM_DATA
	if ((offset == 1) && ACCESSING_BITS_0_15)
	{
		UINT64 ret = 0;

		if (v->rom_offset_flags & NAOMIBD_FLAG_SPECIAL_MODE)
		{
			// can we live-decrypt this game?
			if (v->dc_gamekey != -1)
			{
				ret = (UINT64)(v->dc_cart_ram[v->dc_readback+1] | (v->dc_cart_ram[v->dc_readback]<<8));
				v->dc_readback += 2;
			}
			else
			{
				if (v->rom_offset == 0x1fffe)
				{
					UINT8 *prot = (UINT8 *)v->protdata;
					UINT32 byte_offset = v->prot_offset*2;

					// this is a good time to clear the prot_sum
					v->prot_sum = 0;

					if (v->prot_translate == NULL)
					{
						#if NAOMIBD_PRINTF_PROTECTION
						v->prot_pio_count += 2;
						printf("naomibd: reading protection data, but none was supplied (now %x bytes)\n", v->prot_pio_count);
						#endif
						return 0;
					}

					#if NAOMIBD_PRINTF_PROTECTION
					v->prot_pio_count += 2;
					printf("naomibd: PIO read count %x\n", v->prot_pio_count);
					#endif

					if (v->prot_reverse_bytes)
					{
						ret = (UINT64)(prot[byte_offset+1] | (prot[byte_offset]<<8));
					}
					else
					{
						ret = (UINT64)(prot[byte_offset] | (prot[byte_offset+1]<<8));
					}

					v->prot_offset++;
				}
				#if NAOMIBD_PRINTF_PROTECTION
				else
				{
					printf("Bad protection offset read %x\n", v->rom_offset);
				}
				#endif
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
	else if ((offset == 2) && ACCESSING_BITS_32_63)
	{
		//  Actel FPGA ID, used on some games for a "special" ROM test.
		//
		//  without this (by returning 0xffff) some games will do a rom test where
		//  the IC numbers tested do not relate to the actual ROMs on the cart,
		//  and a fake 'IC1' will be tested, which returns mirrored data from the
		//  other roms in order to pass if enabled on the real hardware.
		//  (certain bios / board combinations will also cause this, so it is
		//   important that we mirror the data in the rom loading using ROM_COPY)

		//return (UINT64)0xffff << 32;
		return (UINT64)actel_id << 32;
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
		//mame_printf_verbose("%s:ROM: read mask %" I64FMT "x @ %x\n", cpuexec_describe_context(machine), mem_mask, offset);
	}

	return U64(0xffffffffffffffff);
}

WRITE64_DEVICE_HANDLER( naomibd_w )
{
	naomibd_state *v = get_safe_token(device);

	// AW board
	if (v->type == AW_ROM_BOARD)
	{
		//printf("AW: %" I64FMT "x to ROM board @ %x (mask %" I64FMT "x)\n", data, offset, mem_mask);

		switch (offset)
		{
			case 0:
			{
				if(ACCESSING_BITS_0_15)
				{
					// EPR_OFFSETL
					v->aw_offset &= 0xffff0000;
					v->aw_offset |= (data & 0xffff);
					v->dma_offset = v->aw_offset*2;
					//printf("EPR_OFFSETL = %x, dma_offset %x\n", (UINT32)data, v->dma_offset);
				}
				else if(ACCESSING_BITS_32_47 || ACCESSING_BITS_32_63)
				{
					// EPR_OFFSETH
					v->aw_offset &= 0xffff;
					v->aw_offset |= ((data>>16) & 0xffff0000);
					v->dma_offset = v->aw_offset*2;
					v->dma_offset_flags = NAOMIBD_FLAG_ADDRESS_SHUFFLE|NAOMIBD_FLAG_AUTO_ADVANCE;	// force normal DMA mode
					//printf("EPR_OFFSETH = %x, dma_offset %x\n", (UINT32)(data>>32), v->dma_offset);
				}

			}
			break;

			case 1:
			{
				if(ACCESSING_BITS_32_47 || ACCESSING_BITS_32_63)
				{
					// MPR_RECORD_INDEX
					//printf("%x to RECORD_INDEX\n", (UINT32)(data>>32));
					v->dma_offset = 0x1000000 + (0x40 * (data>>32));
				}
			}
			break;

			case 2:
			{
				if(ACCESSING_BITS_0_15)
				{
					UINT8 *ROM = (UINT8 *)v->memory;
					UINT32 base;

					// MPR_FIRST_FILE_INDEX (usually 3)
					base = data * 64;
					v->aw_file_base = ROM[0x100000b+base]<<24 | ROM[0x100000a+base]<<16 | ROM[0x1000009+base]<<8 | ROM[0x1000008+base];
					v->aw_file_base += 0x1000000;
					//printf("%x to FIRST_FILE_INDEX, file_base = %x\n", (UINT32)data, v->aw_file_base);
				}
				else if(ACCESSING_BITS_32_47 || ACCESSING_BITS_32_63)
				{
					// MPR_FILE_OFFSETL
					v->aw_file_offset &= 0xffff0000;
					v->aw_file_offset |= (data>>32) & 0xffff;
					v->dma_offset = v->aw_file_base + (v->aw_file_offset*2);
					//printf("%x to FILE_OFFSETL, file_offset %x, dma_offset %x\n", (UINT32)(data>>32), v->aw_file_offset, v->dma_offset);
				}
			}
			break;

			case 3:
			{
				if(ACCESSING_BITS_0_15)
				{
					// MPR_FILE_OFFSETH
					v->aw_file_offset &= 0xffff;
					v->aw_file_offset |= (data & 0xffff)<<16;
					v->dma_offset = v->aw_file_base + (v->aw_file_offset*2);
					//printf("%x to FILE_OFFSETH, file_offset %x, dma_offset %x\n", (UINT32)data, v->aw_file_offset, v->dma_offset);
				}
			}
			break;

			default:
				logerror("AW: unhandled %" I64FMT "x to ROM board @ %x (mask %" I64FMT "x)\n", data, offset, mem_mask);
			break;
		}

		return;
	}

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

			#if NAOMIBD_PRINTF_PROTECTION
			printf("PIO: offset to %x\n", v->rom_offset);
			#endif
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

						#if NAOMIBD_PRINTF_PROTECTION
						printf("Protection: set up read @ %x, key %x sum %x (PIO %x DMA %x) [%s]\n", v->prot_offset*2, v->prot_key, v->prot_sum, v->rom_offset, v->dma_offset, cpuexec_describe_context(device->machine));

						v->prot_pio_count = 0;
						#endif

						// if dc_gamekey isn't -1, we can live-decrypt this one
						if (v->dc_gamekey != -1)
						{
							UINT8 temp_ram[128*1024];
							UINT8 *ROM = (UINT8 *)v->memory;

							v->dc_seed = 0;
							v->dc_readback = 0;
							v->dc_seqkey = v->prot_key;

							#if NAOMIBD_PRINTF_PROTECTION
							printf("M2/M3 decrypt: gamekey %x seqkey %x seed %x length %x\n", v->dc_gamekey, v->dc_seqkey, v->dc_seed, v->dc_m3_ptr);
							#endif

							// M2: just decrypt up to the size limit since we don't know the size in advance
							if (v->prot_offset != 0x2000000/2)
							{
								// decrypt to temp buffer
								stream_decrypt(v->dc_gamekey, v->dc_seqkey, v->prot_offset&0xffff, &ROM[v->prot_offset*2], temp_ram, 128*1024);
							}
							else
							{
								// decrypt cart ram to temp buffer
								stream_decrypt(v->dc_gamekey, v->dc_seqkey, v->dc_seed, v->dc_cart_ram, temp_ram, v->dc_m3_ptr);
							}

							#if NAOMIBD_PRINTF_PROTECTION
							printf("result: %02x %02x %02x %02x %02x %02x %02x %02x\n",
								temp_ram[0], temp_ram[1], temp_ram[2], temp_ram[3],
								temp_ram[4], temp_ram[5], temp_ram[6], temp_ram[7]);
							#endif

							// copy results to cart ram for readback
							memcpy(v->dc_cart_ram, temp_ram, 128*1024);

							v->dc_m3_ptr = 0;
							v->prot_sum = 0;
						}
						else
						{
							// translate address if necessary
							if (v->prot_translate != NULL)
							{
								int i = 0;
								while (v->prot_translate[i+1] != 0xffffffff)
								{
									// should we match by key, address, or sum?
									if (v->prot_translate[i] == -2)	// match sum
									{
										if (v->prot_translate[i+1] == v->prot_sum)
										{
											#if NAOMIBD_PRINTF_PROTECTION
											printf("Protection: got sum %x, translated to %x\n", v->prot_sum, v->prot_translate[i+2]);
											#endif
											v->prot_offset = v->prot_translate[i+2]/2;
											break;
										}
										else
										{
											i+= 3;
										}
									}
									else if (v->prot_translate[i] == -1)	// match address
									{
										if (v->prot_translate[i+1] == (v->prot_offset*2))
										{
											#if NAOMIBD_PRINTF_PROTECTION
											printf("Protection: got offset %x, translated to %x\n", v->prot_offset, v->prot_translate[i+2]);
											#endif
											v->prot_offset = v->prot_translate[i+2]/2;
											break;
										}
										else
										{
											i += 3;
										}
									}
									else	// match key
									{
										if (v->prot_translate[i] == v->prot_key)
										{
											#if NAOMIBD_PRINTF_PROTECTION
											printf("Protection: got key %x, translated to %x\n", v->prot_key, v->prot_translate[i+2]);
											#endif
											v->prot_offset = v->prot_translate[i+2]/2;
											break;
										}
										else
										{
											i+= 3;
										}
									}
								}
							}
							#if NAOMIBD_PRINTF_PROTECTION
							else
							{
								printf("naomibd: protection not handled for this game\n");
							}
							#endif
						}
						break;

					case 0x2000000:
					case 0x2020000:
						#if NAOMIBD_PRINTF_PROTECTION
						printf("Protection write %04x to upload @ %x\n", (UINT32)(data&0xffff), v->dc_m3_ptr);
						#endif
						v->prot_sum += (INT16)(data&0xffff);

						v->dc_cart_ram[v->dc_m3_ptr] = (data&0xff);
						v->dc_cart_ram[v->dc_m3_ptr+1] = (data>>8)&0xff;
						v->dc_m3_ptr += 2;
						break;

					default:
						#if NAOMIBD_PRINTF_PROTECTION
						printf("naomibd: unknown protection write %x @ %x\n", (UINT32)data, v->rom_offset);
						#endif
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
			mame_printf_verbose("%s: ROM: write %" I64FMT "x to %x, mask %" I64FMT "x\n", cpuexec_describe_context(device->machine), data, offset, mem_mask);
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
	UINT8* realpic; // todo, add to device

	memset(name,'\0',128);

	realpic = memory_region(machine,"pic");

	if (realpic)
	{
		//printf("Real PIC binary found\n");
		int i;
		for (i=0;i<7;i++)
		{
			name[i] = realpic[0x7c0+i*2];
		}
		for (i=0;i<7;i++)
		{
			name[i+7] = realpic[0x7e0+i*2];
		}
	}
	else
	{
		// use extracted pic data
		logerror("This PIC key hasn't been converted to a proper PIC binary yet!\n");
		memcpy(name, v->picdata+33, 7);
		memcpy(name+7, v->picdata+25, 7);
	}

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
	logerror("Looking for file %s\n", name);
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

			logerror("start %08x size %08x\n", start,size);
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

		logerror("Looking for file %s\n", name);
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

				logerror("start %08x size %08x\n", start,size);
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
	realpic = memory_region(machine,"pic");

	if (realpic)
	{
		int i;
		key = 0;
		for (i=0;i<7;i++)
		{
			key |= (UINT64)realpic[0x780+i*2] << (56 - i*8);
		}

		key |= (UINT64)realpic[0x7a0];
	}
	else
	{
		key =(((UINT64)v->picdata[0x31] << 56) |
			  ((UINT64)v->picdata[0x32] << 48) |
			  ((UINT64)v->picdata[0x33] << 40) |
			  ((UINT64)v->picdata[0x34] << 32) |
			  ((UINT64)v->picdata[0x35] << 24) |
			  ((UINT64)v->picdata[0x36] << 16) |
			  ((UINT64)v->picdata[0x37] << 8)  |
			  ((UINT64)v->picdata[0x29] << 0));
	}

	logerror("key is %08x%08x\n", (UINT32)((key & 0xffffffff00000000ULL)>>32), (UINT32)(key & 0x00000000ffffffffULL));

	// decrypt loaded data
	naomi_game_decrypt(machine, key, v->memory, size);
	cdrom_close(gdromfile);
}

/***************************************************************************
    DECRYPTION EMULATION

By convention, we label the three known cart protection methods this way (using Deunan Knute's wording):
M1: DMA read of protected ROM area
M2: special read of ROM area which supplies decryption key first
M3: normal read followed by write to cart's decryption buffer (up to 64kB), followed by M2 but from buffer area

M1's working is still unclear (more on this later), so we will be speaking of M2 & M3 most of the time.

The encryption is done by a stream cipher operating in counter mode, which use a 16-bits internal block cipher.

There are 2 "control bits" at the start of the decrypted stream which control the mode of operation: bit #1 set to 1 means
that the decrypted stream needs to be decompressed after being decrypted. More on this later.

The next 16-bits are part of the header (they don't belong to the plaintext), but his meaning is unclear. It has been
conjectured that it could stablish when to "reset" the process and start processing a new stream (based on some tests
on WWFROYAL, in which the decryption's output doesn't seem to be valid for more than some dozens of words), but some
more testing would be needed for clarifying that.

After those 18 heading bits, we find the proper plaintext. It must be noted that, due to the initial 2 special bits,
the 16-bits words of the plaintext are shifted 2 bits respect to the word-boundaries of the output stream of the
internal block-cipher. So, at a given step, the internal block cipher will output 16-bits, 14 of which will go to a
given plaintext word, and the remaining 2 to the next plaintext word.

The underlying block cipher consists of two 4-round Feistel Networks (FN): the first one takes the counter (16 bits),
the game-key (20 bits) and the sequence-key (16 bits) and output a middle result (16 bits) which will act as another key
for the second one. The second FN will take the encrypted word (16 bits), the game-key, the sequence-key and the result
from the first FN and will output the decrypted word (16 bits).

Each round of the Feistel Networks use four substitution sboxes, each having 6 inputs and 2 outputs. The input can be the
XOR of at most two "sources bits", being source bits the bits from the previous round and the bits from the different keys.

The underlying block cipher has the same structure than the one used by the CPS-2 (Capcom Play System 2) and,
indeed, some of the used sboxes are exactly the same and appear in the same FN/round in both systems (this is not evident,
as you need to apply a bitswapping and some XORs to the input & output of the sboxes to get the same values due). However,
the key scheduling used by this implementation is much weaker than the CPS-2's one. Many s-boxes inputs are XORed with any
key bit and, indeed, the cart-specific key is just 20-bits long.

Due to the small key-length, no sophisticated attacks are needed to recover the keys; a brute-force attack knowing just
one or two (encrypted word-decrypted word) pairs suffice.

The only difference in the decryption process between M2 and M3 is the initialization of the counter. In M3, the counter is
always set to 0 at the beginning of the decryption while, in M2, the bits #1-#16 of the ciphertext's address are used
to initialize the counter.

Due to the nature of the cipher, there are some degrees of freedom when choosing the s-boxes and keys values; by example,
you could apply a fixed bitswapping and XOR to the keys and the decryption would remain the same as long as you change
accordingly the s-boxes' definitions. So the order of the bits in the keys is arbitrary, and the s-boxes values have been
chosen so as to make the key for CAPSNK equal to 0.

It can be observed that some sboxes have incomplete tables (a 255 value indicate an unknown value). In most of the cases,
they are apparently unused by the cipher (due to the weak key scheduling mentioned above). As of february/2010, the only
s-box which have a incomplete table which could be begin used is the 4th s-box of the 1st round of the 2nd FN. It's
incomplete because we haven't located any game using that part of the s-box till now, but definitively it could be being
used by some still-not-analyzed carts.

When bit #1 of the heading control bits is set to 1, an additional decompression step seems to be carried out. As of
february/2010, Deunan Knute has put some work on analyzing the decompression algorithm, but probably much more work will
be needed to completely reverse engineer it. Interested devs with quick access to hardware tests are welcomed to join
the task.

Guilty Gear X & Sega Tetris are examples of games using the decompression ingame.

Due to technical details, it's more difficult to get custom decryption data from M1 carts, which hinders some types of
analysis. The only M1 cart which have received attention until now have been AH! MY GODDESS. The available data clearly
doesn't have the same structure than M2&M3 carts using only the decryption step. However, due to that some hardware tests
show cycled structures similar to the ones returned by M2&M3 carts using the decompression algo, it's conjectured that M1
carts will be using the same decompression algorithm seen in M2&M3 ones.

****************************************************************************************/

struct sbox
{
    UINT8 table[64];
    int inputs[6];      // positions of the inputs bits, -1 means no input except from key
    int outputs[2];     // positions of the output bits
};


static const struct sbox fn1_sboxes[4][4] =
{
    {   // 1st round
        {
            {
                0,3,2,2,1,3,1,2,3,2,1,2,1,2,3,1,3,2,2,0,2,1,3,0,0,3,2,3,2,1,2,0,
                // unused
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {3,4,5,7,-1,-1},
            {0,4}
        },

        {
            {
                2,2,2,0,3,3,0,1,2,2,3,2,3,0,2,2,1,1,0,3,3,2,0,2,0,1,0,1,2,3,1,1,
                0,1,3,3,1,3,3,1,2,3,2,0,0,0,2,2,0,3,1,3,0,3,2,2,0,3,0,3,1,1,0,2,
            },
            {0,1,2,5,6,7},
            {1,6}
        },

        {
            {
                0,1,3,0,3,1,1,1,1,2,3,1,3,0,2,3,3,2,0,2,1,1,2,1,1,3,1,0,0,2,0,1,
                1,3,1,0,0,3,2,3,2,0,3,3,0,0,0,0,1,2,3,3,2,0,3,2,1,0,0,0,2,2,3,3,
            },
            {0,2,5,6,7,-1},
            {2,3}
        },

        {
            {
                3,2,1,2,1,2,3,2,0,3,2,2,3,1,3,3,0,2,3,0,3,3,2,1,1,1,2,0,2,2,0,1,
                1,3,3,0,0,3,0,3,0,2,1,3,2,1,0,0,0,1,1,2,0,1,0,0,0,1,3,3,2,0,3,3,
            },
            {1,2,3,4,6,7},
            {5,7}
        },
    },
    {   // 2nd round
        {
            {
                3,3,1,2,0,0,2,2,2,1,2,1,3,1,1,3,3,0,0,3,0,3,3,2,1,1,3,2,3,2,1,3,
                2,3,0,1,3,2,0,1,2,1,3,1,2,2,3,3,3,1,2,2,0,3,1,2,2,1,3,0,3,0,1,3,
            },
            {0,1,3,4,5,7},
            {0,4}
        },

        {
            {
                2,0,1,0,0,3,2,0,3,3,1,2,1,3,0,2,0,2,0,0,0,2,3,1,3,1,1,2,3,0,3,0,
                // unused
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {0,1,3,4,6,-1},
            {1,5}
        },

        {
            {
                2,2,2,3,1,1,0,1,0,1,2,2,3,3,0,2,0,3,2,3,3,0,2,1,0,3,1,0,0,2,3,2,
                3,2,0,3,2,0,1,0,3,3,1,1,2,2,2,0,2,1,3,1,1,1,1,2,2,2,3,0,1,3,0,0,
            },
            {1,2,5,6,7,-1},
            {2,7}
        },

        {
            {
                0,1,3,3,3,1,3,3,1,0,2,0,2,0,0,3,1,2,1,3,1,2,3,2,2,0,1,3,0,3,3,3,
                0,0,0,2,1,1,2,3,2,2,3,1,1,2,0,2,0,2,1,3,1,1,3,3,1,1,3,0,2,3,0,0,
            },
            {2,3,4,5,6,7},
            {3,6}
        },
    },
    {   // 3rd round
        {
            {
                0,0,1,0,1,0,0,3,2,0,0,3,0,1,0,2,0,3,0,0,2,0,3,2,2,1,3,2,2,1,1,2,
                0,0,0,3,0,1,1,0,0,2,1,0,3,1,2,2,2,0,3,1,3,0,1,2,2,1,1,1,0,2,3,1,
            },
            {1,2,3,4,5,7},
            {0,5}
        },

        {
            {
                1,2,1,0,3,1,1,2,0,0,2,3,2,3,1,3,2,0,3,2,2,3,1,1,1,1,0,3,2,0,0,1,
                1,0,0,1,3,1,2,3,0,0,2,3,3,0,1,0,0,2,3,0,1,2,0,1,3,3,3,1,2,0,2,1,
            },
            {0,2,4,5,6,7},
            {1,6}
        },

        {
            {
                0,3,0,2,1,2,0,0,1,1,0,0,3,1,1,0,0,3,0,0,2,3,3,2,3,1,2,0,0,2,3,0,
                // unused
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {0,2,4,6,7,-1},
            {2,3}
        },

        {
            {
                0,0,1,0,0,1,0,2,3,3,0,3,3,2,3,0,2,2,2,0,3,2,0,3,1,0,0,3,3,0,0,0,
                2,2,1,0,2,0,3,2,0,0,3,1,3,3,0,0,2,1,1,2,1,0,1,1,0,3,1,2,0,2,0,3,
            },
            {0,1,2,3,6,-1},
            {4,7}
        },
    },
    {   // 4th round
        {
            {
                0,3,3,3,3,3,2,0,0,1,2,0,2,2,2,2,1,1,0,2,2,1,3,2,3,2,0,1,2,3,2,1,
                3,2,2,3,1,0,1,0,0,2,0,1,2,1,2,3,1,2,1,1,2,2,1,0,1,3,2,3,2,0,3,1,
            },
            {0,1,3,4,5,6},
            {0,5}
        },

        {
            {
                0,3,0,0,2,0,3,1,1,1,2,2,2,1,3,1,2,2,1,3,2,2,3,3,0,3,1,0,3,2,0,1,
                3,0,2,0,1,0,2,1,3,3,1,2,2,0,2,3,3,2,3,0,1,1,3,3,0,2,1,3,0,2,2,3,
            },
            {0,1,2,3,5,7},
            {1,7}
        },

        {
            {
                0,1,2,3,3,3,3,1,2,0,2,3,2,1,0,1,2,2,1,2,0,3,2,0,1,1,0,1,3,1,3,1,
                3,1,0,0,1,0,0,0,0,1,2,2,1,1,3,3,1,2,3,3,3,2,3,0,2,2,1,3,3,0,2,0,
            },
            {2,3,4,5,6,7},
            {2,3}
        },

        {
            {
                0,2,1,1,3,2,0,3,1,0,1,0,3,2,1,1,2,2,0,3,1,0,1,2,2,2,3,3,0,0,0,0,
                1,2,1,0,2,1,2,2,2,3,2,3,0,1,3,0,0,1,3,0,0,1,1,0,1,0,0,0,0,2,0,1,
            },
            {0,1,2,4,6,7},
            {4,6}
        },
    },
};


static const struct sbox fn2_sboxes[4][4] =
{
    {   // 1st round
        {
            {
                3,3,0,1,0,1,0,0,0,3,0,0,1,3,1,2,0,3,3,3,2,1,0,1,1,1,2,2,2,3,2,2,
                2,1,3,3,1,3,1,1,0,0,1,2,0,2,2,1,1,2,3,1,2,1,3,1,2,2,0,1,3,0,2,2,
            },
            {1,3,4,5,6,7},
            {0,7}
        },

        {
            {
                0,2,3,2,1,1,0,0,2,1,0,3,3,0,0,0,3,2,0,2,1,1,2,1,0,0,3,1,2,2,3,1,
                3,1,3,0,0,0,1,3,1,0,0,3,2,2,3,1,1,3,0,0,2,1,3,3,1,3,1,2,3,1,2,1,
            },
            {0,3,5,6,-1,-1},
            {1,2}
        },

        {
            {
                0,2,2,1,0,1,2,1,2,0,1,2,3,3,0,1,3,1,1,2,1,2,1,3,3,2,3,3,2,1,0,1,
                // unused
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {0,2,3,4,7,-1},
            {3,4}
        },

        {
            {
                2,3,1,3,2,0,1,2,0,0,3,3,3,3,3,1,2,0,2,1,2,3,0,2,0,1,0,3,0,2,1,0,
                2,3,0,1,3,0,3,2,3,1,2,0,3,1,1,2,
                // potentially used, but we haven't located any game using it
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {1,2,5,6,-1,-1},
            {5,6}
        },
    },
    {   // 2nd round
        {
            {
                2,3,1,3,1,0,3,3,3,2,3,3,2,0,0,3,2,3,0,3,1,1,2,3,1,1,2,2,0,1,0,0,
                2,1,0,1,2,0,1,2,0,3,1,1,2,3,1,2,0,2,0,1,3,0,1,0,2,2,3,0,3,2,3,0,
            },
            {0,1,4,5,6,7},
            {0,7}
        },

        {
            {
                0,2,2,0,2,2,0,3,2,3,2,1,3,2,3,3,1,1,0,0,3,0,2,1,1,3,3,2,3,2,0,1,
                1,2,3,0,1,0,3,0,3,1,0,2,1,2,0,3,2,3,1,2,2,0,3,2,3,0,0,1,2,3,3,3,
            },
            {0,2,3,6,7,-1},
            {1,5}
        },

        {
            {
                1,2,3,2,0,3,2,3,0,1,1,0,0,2,2,3,2,0,0,3,0,2,3,3,2,2,1,0,2,1,0,3,
                1,0,2,0,1,1,0,1,0,0,1,0,3,0,3,3,2,2,0,2,1,1,1,0,3,0,1,3,2,3,2,1,
            },
            {2,3,4,6,7,-1},
            {2,3}
        },

        {
            {
                2,3,1,3,1,1,2,3,3,1,1,0,1,0,2,3,2,1,0,0,2,2,0,1,0,2,2,2,0,2,1,0,
                3,1,2,3,1,3,0,2,1,0,1,0,0,1,2,2,3,2,3,1,3,2,1,1,2,0,2,1,3,3,1,0,
            },
            {1,2,3,4,5,6},
            {4,6}
        },
    },
    {   // 3rd round
        {
            {
                0,3,0,1,0,2,3,3,1,0,1,3,2,2,1,1,3,3,3,0,2,0,2,0,0,0,2,3,1,1,0,0,
                3,3,0,3,3,0,0,2,1,1,1,0,2,2,2,0,3,0,3,1,2,2,0,3,0,0,3,2,0,3,2,1,
            },
            {1,4,5,6,7,-1},
            {0,5}
        },

        {
            {
                0,3,0,1,3,0,3,1,3,2,2,2,3,0,3,2,2,1,2,2,0,3,2,2,0,0,2,1,1,3,2,3,
                2,3,3,1,2,0,1,2,2,1,0,0,0,0,2,3,1,2,0,3,1,3,1,2,3,2,1,0,3,0,0,2,
            },
            {0,2,3,4,6,7},
            {1,7}
        },

        {
            {
                2,2,3,2,0,3,2,3,1,1,2,0,2,3,1,3,0,0,0,3,2,0,1,0,1,3,2,3,3,3,1,0,
                // unused
                255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
            },
            {1,2,4,7,-1,-1},
            {2,4}
        },

        {
            {
                0,2,3,1,3,1,1,0,0,1,3,0,2,1,3,3,2,0,2,1,1,2,3,3,0,0,0,2,0,2,3,0,
                3,3,3,3,2,3,3,2,3,0,1,0,2,3,3,2,0,1,3,1,0,1,2,3,3,0,2,0,3,0,3,3,
            },
            {0,1,2,3,5,7},
            {3,6}
        },
    },
    {   // 4th round
        {
            {
                0,1,1,0,0,1,0,2,3,3,0,1,2,3,0,2,1,0,3,3,2,0,3,0,0,2,1,0,1,0,1,3,
                0,3,3,1,2,0,3,0,1,3,2,0,3,3,1,3,0,2,3,3,2,1,1,2,2,1,2,1,2,0,1,1,
            },
            {0,1,2,4,7,-1},
            {0,5}
        },

        {
            {
                2,0,0,2,3,0,2,3,3,1,1,1,2,1,1,0,0,2,1,0,0,3,1,0,0,3,3,0,1,0,1,2,
                0,2,0,2,0,1,2,3,2,1,1,0,3,3,3,3,3,3,1,0,3,0,0,2,0,3,2,0,2,2,0,1,
            },
            {0,1,3,5,6,-1},
            {1,3}
        },

        {
            {
                0,1,1,2,1,3,1,1,0,0,3,1,1,1,2,0,3,2,0,1,1,2,3,3,3,0,3,0,0,2,0,3,
                3,2,0,0,3,2,3,1,2,3,0,3,2,0,1,2,2,2,0,2,0,1,2,2,3,1,2,2,1,1,1,1,
            },
            {0,2,3,4,5,7},
            {2,7}
        },

        {
            {
                0,1,2,0,3,3,0,3,2,1,3,3,0,3,1,1,3,2,3,2,3,0,0,0,3,0,2,2,3,2,2,3,
                2,2,3,1,2,3,1,2,0,3,0,2,3,1,0,0,3,2,1,2,1,2,1,3,1,0,2,3,3,1,3,2,
            },
            {2,3,4,5,6,7},
            {4,6}
        },
    },
};

static const int fn1_game_key_scheduling[30][2] =
{
    {1,29},  {1,71},  {1,81},  {2,4},   {2,54},  {3,8},   {4,56},  {4,73},
    {5,11},  {6,51},  {7,92},  {8,89},  {9,9},   {9,10},  {9,39},  {9,41},
    {9,58},  {9,59},  {9,86},  {10,90}, {11,6},  {12,64}, {13,49}, {14,44},
    {15,40}, {16,69}, {17,15}, {18,23}, {18,43}, {19,82},
};

static const int fn2_game_key_scheduling[27][2] =
{
    {0,0},   {1,3},   {1,35},  {2,11},  {3,20},  {4,22},  {5,23},  {6,29},
    {7,38},  {8,39},  {9,47},  {9,55},  {9,86},  {9,87},  {9,90},  {10,50},
    {10,53}, {11,57}, {12,59}, {13,61}, {13,64}, {14,63}, {15,67}, {16,72},
    {17,83}, {18,88}, {19,94},
};

static const int fn1_sequence_key_scheduling[20][2] =
{
    {0,52},  {1,34},  {2,17},  {3,36}, {4,84},  {4,88},  {5,57},  {6,48},
    {6,68},  {7,76},  {8,83},  {9,30}, {10,22}, {10,41}, {11,38}, {12,55},
    {13,74}, {14,19}, {14,80}, {15,26}
};

static const int fn2_sequence_key_scheduling[16] = {77,34,8,42,36,27,69,66,13,9,79,31,49,7,24,64};

static const int fn2_middle_result_scheduling[16] = {1,10,44,68,74,78,81,95,2,4,30,40,41,51,53,58};

static int feistel_function(int input, const struct sbox* sboxes, UINT32 subkeys)
{
    int k,m;
    int aux;
    int result=0;

    for (m=0; m<4; ++m)  // 4 sboxes
    {
        for (k=0, aux=0; k<6; ++k)
        {
            if (sboxes[m].inputs[k]!=-1)
            {
                aux |= (BIT(input, sboxes[m].inputs[k])<<k);
            }
        }

        aux = sboxes[m].table[(aux^subkeys)&0x3f];

        for (k=0; k<2; ++k)
        {
            result |= (BIT(aux,k)<<sboxes[m].outputs[k]);
        }

        subkeys >>=6;
    }

    return result;
}

/**************************
This implementation is an "educational" version. It must be noted that it can be speed-optimized in a number of ways.
The most evident one is to factor out the parts of the key-scheduling that must only be done once (like the game-key &
sequence-key parts) as noted in the comments inlined in the function. More sophisticated speed-ups can be gained by
noticing that the weak key-scheduling would allow to create some pregenerated look-up tables for doing most of the work
of the function. Even so, it would still be pretty slow, so caching techniques could be a wiser option here.
**************************/

static UINT16 block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data)
{
    int j;
    int aux,aux2;
    int A,B;
    int middle_result;
    UINT32 fn1_subkeys[4];
    UINT32 fn2_subkeys[4];

    /* Game-key scheduling; this could be done just once per game at initialization time */
    memset(fn1_subkeys,0,sizeof(UINT32)*4);
    memset(fn2_subkeys,0,sizeof(UINT32)*4);

    for (j=0; j<30; ++j)
    {
        if (BIT(game_key, fn1_game_key_scheduling[j][0])!=0)
        {
            aux = fn1_game_key_scheduling[j][1]%24;
            aux2 = fn1_game_key_scheduling[j][1]/24;
            fn1_subkeys[aux2] ^= (1<<aux);
        }
    }

    for (j=0; j<27; ++j)
    {
        if (BIT(game_key, fn2_game_key_scheduling[j][0])!=0)
        {
            aux = fn2_game_key_scheduling[j][1]%24;
            aux2 = fn2_game_key_scheduling[j][1]/24;
            fn2_subkeys[aux2] ^= (1<<aux);
        }
    }
    /********************************************************/

    /* Sequence-key scheduling; this could be done just once per decryption run */
    for (j=0; j<20; ++j)
    {
        if (BIT(sequence_key,fn1_sequence_key_scheduling[j][0])!=0)
        {
            aux = fn1_sequence_key_scheduling[j][1]%24;
            aux2 = fn1_sequence_key_scheduling[j][1]/24;
            fn1_subkeys[aux2] ^= (1<<aux);
        }
    }

    for (j=0; j<16; ++j)
    {
        if (BIT(sequence_key,j)!=0)
        {
            aux = fn2_sequence_key_scheduling[j]%24;
            aux2 = fn2_sequence_key_scheduling[j]/24;
            fn2_subkeys[aux2] ^= (1<<aux);
        }
    }

    // subkeys bits 10 & 41
    fn2_subkeys[0] ^= (BIT(sequence_key,2)<<10);
    fn2_subkeys[1] ^= (BIT(sequence_key,4)<<17);
    /**************************************************************/

    // First Feistel Network

    aux = BITSWAP16(counter,5,12,14,13,9,3,6,4,    8,1,15,11,0,7,10,2);

    // 1st round
    B = aux >> 8;
    A = (aux & 0xff) ^ feistel_function(B,fn1_sboxes[0],fn1_subkeys[0]);

    // 2nd round
    B = B ^ feistel_function(A,fn1_sboxes[1],fn1_subkeys[1]);

    // 3rd round
    A = A ^ feistel_function(B,fn1_sboxes[2],fn1_subkeys[2]);

    // 4th round
    B = B ^ feistel_function(A,fn1_sboxes[3],fn1_subkeys[3]);

    middle_result = (B<<8)|A;


    /* Middle-result-key sheduling */
    for (j=0; j<16; ++j)
    {
        if (BIT(middle_result,j)!=0)
        {
            aux = fn2_middle_result_scheduling[j]%24;
            aux2 = fn2_middle_result_scheduling[j]/24;
            fn2_subkeys[aux2] ^= (1<<aux);
        }
    }
    /*********************/

    // Second Feistel Network

    aux = BITSWAP16(data,14,3,8,12,13,7,15,4,    6,2,9,5,11,0,1,10);

    // 1st round
    B = aux >> 8;
    A = (aux & 0xff) ^ feistel_function(B,fn2_sboxes[0],fn2_subkeys[0]);

    // 2nd round
    B = B ^ feistel_function(A,fn2_sboxes[1],fn2_subkeys[1]);

    // 3rd round
    A = A ^ feistel_function(B,fn2_sboxes[2],fn2_subkeys[2]);

    // 4th round
    B = B ^ feistel_function(A,fn2_sboxes[3],fn2_subkeys[3]);

    aux = (B<<8)|A;

    aux = BITSWAP16(aux,15,7,6,14,13,12,5,4,    3,2,11,10,9,1,0,8);

    return aux;
}

static void stream_decrypt(UINT32 game_key, UINT32 sequence_key, UINT16 seed, UINT8* ciphertext, UINT8* plaintext, int length)
{
    UINT16 counter = seed;
    UINT16 last_word;
    UINT16 plain_word;
    UINT16 aux_word;
    int control_bits;
    UINT16 heading_word;

    last_word = block_decrypt(game_key, sequence_key, counter, *ciphertext<<8 | *(ciphertext+1));
    control_bits = last_word&3;
    ++counter; ciphertext+=2;

    aux_word = block_decrypt(game_key, sequence_key, counter, *ciphertext<<8 | *(ciphertext+1));
    heading_word = (last_word&~3) | (aux_word&3);
    last_word = aux_word;
    ++counter; ciphertext+=2;

    if (BIT(control_bits,1)==0)  // no decompression, just decryption
    {
        for (; length>0; length-=2, ++counter, ciphertext+=2)
        {
            aux_word = block_decrypt(game_key, sequence_key, counter, *ciphertext<<8 | *(ciphertext+1));
            plain_word = (last_word&~3) | (aux_word&3);
            last_word = aux_word;

            *(plaintext++) = plain_word>>8;
            *(plaintext++) = plain_word&0xff;
        }
    }
    else  // decryption plus decompression
    {
    	fatalerror("NAOMI ASIC compression unsupported\n");
        return;  // not implemented, decompression has not been fully reverse engineered as of february/2010
    }
}

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( naomibd )
{
	const naomibd_config *config = (const naomibd_config *)device->baseconfig().inline_config;
	naomibd_state *v = get_safe_token(device);
	int i;

	/* validate some basic stuff */
	assert(device->baseconfig().static_config == NULL);
	assert(device->baseconfig().inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* validate configuration */
	assert(config->type >= ROM_BOARD && config->type < MAX_NAOMIBD_TYPES);

	/* store a pointer back to the device */
	v->device = device;

	/* find the protection address translation for this game */
	v->prot_translate = (UINT32 *)0;
	#if NAOMIBD_PRINTF_PROTECTION
	v->prot_pio_count = 0;
	#endif
	for (i=0; i<ARRAY_LENGTH(naomibd_translate_tbl); i++)
	{
		if (!strcmp(device->machine->gamedrv->name, naomibd_translate_tbl[i].name))
		{
			v->prot_translate = &naomibd_translate_tbl[i].transtbl[0];
			v->prot_reverse_bytes = naomibd_translate_tbl[i].reverse_bytes;
			v->dc_gamekey = naomibd_translate_tbl[i].live_key;
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

		case AW_ROM_BOARD:
			v->memory = (UINT8 *)memory_region(device->machine, config->regiontag);
			break;

		case DIMM_BOARD:
			v->memory = (UINT8 *)memory_region(device->machine, config->regiontag);
			v->gdromchd = get_disk_handle(device->machine, config->gdromregiontag);
			v->picdata = (UINT8 *)memory_region(device->machine, config->picregiontag);
			if (v->memory != NULL && v->gdromchd != NULL && v->picdata != NULL)
				load_rom_gdrom(device->machine, v);
			break;

		default:
			fatalerror("Unsupported plug-in board in naomibd_start!");
			break;
	}

	/* set the type */
	v->index = device->machine->devicelist.index(device->type, device->tag);
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
	static const UINT8 eeprom_romboard[20+48] =
	{
		0x19,0x00,0xaa,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x69,0x79,0x68,0x6b,0x74,0x6d,0x68,0x6d,
		0xa1,0x09,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
		0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
		0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30
	};
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
			UINT8 *eeprom = auto_alloc_array_clear(device->machine, UINT8, 0x84);
			memcpy(eeprom, eeprom_romboard, sizeof(eeprom_romboard));
			x76f100_init( device->machine, 0, eeprom );
		}

	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_RUNTIME_INFO( naomibd )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DMAOFFSET:
			#if NAOMIBD_PRINTF_PROTECTION
		        printf("DMA source %08x, flags %x\n", get_safe_token(device)->dma_offset, get_safe_token(device)->dma_offset_flags);
			#endif

			// if the flag is cleared that lets the protection chip go,
			// we need to handle this specially.  but not on DIMM boards.
			if (!(get_safe_token(device)->dma_offset_flags & NAOMIBD_FLAG_ADDRESS_SHUFFLE) && (get_safe_token(device)->type == ROM_BOARD))
			{
				if (!strcmp(device->machine->gamedrv->name, "qmegamis"))
				{
					info->i = 0x9000000;
					break;
				}
				else if (!strcmp(device->machine->gamedrv->name, "mvsc2"))
				{
					switch (get_safe_token(device)->dma_offset)
					{
						case 0x08000000: info->i = 0x8800000;	break;
						case 0x08026440: info->i = 0x8830000;	break;
						case 0x0803bda0: info->i = 0x8850000;	break;
						case 0x0805a560: info->i = 0x8870000;	break;
						case 0x0805b720: info->i = 0x8880000;	break;
						case 0x0808b7e0: info->i = 0x88a0000;	break;
						default:
							info->i = get_safe_token(device)->dma_offset;
							break;
					}

					return;
				}
				else
				{
					logerror("Protected DMA not handled for this game (dma_offset %x)\n", get_safe_token(device)->dma_offset);
				}
			}

			info->i = get_safe_token(device)->dma_offset;
			break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_MEMORY:				info->p = get_safe_token(device)->memory;		break;

		/* default to the standard info */
		default:								DEVICE_GET_INFO_NAME(naomibd)(&device->baseconfig(), state, info);	break;
	}
}

DEVICE_GET_INFO( naomibd )
{
	const naomibd_config *config = (device != NULL) ? (const naomibd_config *)device->inline_config : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(naomibd_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(naomibd_config);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = NULL;							break;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = NULL;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(naomibd);		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(naomibd);			break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(naomibd);		break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(naomibd);		break;
		case DEVINFO_FCT_GET_RUNTIME_INFO:		info->get_runtime_info = DEVICE_GET_RUNTIME_INFO_NAME(naomibd);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:
			switch (config->type)
			{
				default:
				case ROM_BOARD:					strcpy(info->s, "Naomi Rom Board");				break;
				case AW_ROM_BOARD:				strcpy(info->s, "Atomiswave Rom Board");				break;
				case DIMM_BOARD:				strcpy(info->s, "Naomi Dimm Board");			break;
			}
			break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Naomi/Atomiswave plug-in board");			break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.1");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
