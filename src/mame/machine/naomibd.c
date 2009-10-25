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

#include "driver.h"
#include "eminline.h"
#include "profiler.h"
#include "machine/eeprom.h"
#include "machine/x76f100.h"
#include "cdrom.h"
#include "includes/naomi.h"
#include "naomibd.h"

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
	UINT32	transtbl[MAX_PROT_REGIONS*3];
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
	UINT32				aw_offset, aw_file_base, aw_file_offset;

	INT32				prot_sum;

	const UINT32			*prot_translate;
	int				prot_reverse_bytes;
	#if NAOMIBD_PRINTF_PROTECTION
	int				prot_pio_count;
	#endif
};

// maps protection offsets to real addresses
// format of array: encryption key, address written, address to switch out with.  if key is -1 it's ignored and address written is the match.
// if key is not -1, it's used for the match instead of the address written.
static const naomibd_config_table naomibd_translate_tbl[] =
{
	{ "doa2", 0, { -1, 0x500, 0, -1, 0x20504, 0x20000, -1, 0x40508, 0x40000, -1, 0x6050c, 0x60000, -1, 0x80510, 0x80000,
		    -1, 0xa0514, 0xa0000, -1, 0xc0518, 0xc0000, -1, 0xe051c, 0xe0000, -1, 0x100520,0x100000, -1, 0x118a3a, 0x120000,
		    -1, 0x12c0d8, 0x140000, -1, 0x147e22, 0x160000, -1, 0x1645ce, 0x180000, -1, 0x17c6b2, 0x1a0000,
		    -1, 0x19902e, 0x1c0000, -1, 0x1b562a, 0x1e0000, -1, 0xffffffff, 0xffffffff } },
	{ "doa2m", 0, { -1, 0x500, 0, -1, 0x20504, 0x20000, -1, 0x40508, 0x40000, -1, 0x6050c, 0x60000, -1, 0x80510, 0x80000,
		    -1, 0xa0514, 0xa0000, -1, 0xc0518, 0xc0000, -1, 0xe051c, 0xe0000, -1, 0x100520,0x100000, -1, 0x11a5b4, 0x120000,
		    -1, 0x12e7c4, 0x140000, -1, 0x1471f6, 0x160000, -1, 0x1640c4, 0x180000, -1, 0x1806ca, 0x1a0000,
		    -1, 0x199df4, 0x1c0000, -1, 0x1b5d0a, 0x1e0000, 0xffffffff, 0xffffffff } },
	{ "csmash", 1, { -1, 0x2000000, 0xbb614, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "csmasho", 1, { -1, 0x2000000, 0xbb5b4, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "capsnk", 0, { 0x8c2a, 0, 0, 0x3d3e, 0, 0x10000, 0x65b7, 0, 0x20000, 0x5896, 0, 0x30000, 0x16d2, 0, 0x40000,
			0x9147, 0, 0x50000, 0x7ac, 0, 0x60000, 0xee67, 0, 0x70000, 0xeb63, 0, 0x80000, 0x2a04, 0, 0x90000,
			0x3e41, 0, 0xa0000, 0xb7af, 0, 0xb0000, 0x9651, 0, 0xc0000, 0xd208, 0, 0xd0000, 0x4769, 0, 0xe0000,
			0xad8c, 0, 0xf0000, 0x923d, 0, 0x100000, 0x4a65, 0, 0x110000, 0x9958, 0, 0x120000, 0x8216, 0, 0x130000,
			0xaa91, 0, 0x140000, 0xd007, 0, 0x150000, 0xead, 0, 0x160000, 0x492, 0, 0x170000,
			0xffffffff, 0xffffffff, 0xffffffff } },
	{ "capsnka", 0, { 0x8c2a, 0, 0, 0x3d3e, 0, 0x10000, 0x65b7, 0, 0x20000, 0x5896, 0, 0x30000, 0x16d2, 0, 0x40000,
			0x9147, 0, 0x50000, 0x7ac, 0, 0x60000, 0xee67, 0, 0x70000, 0xeb63, 0, 0x80000, 0x2a04, 0, 0x90000,
			0x3e41, 0, 0xa0000, 0xb7af, 0, 0xb0000, 0x9651, 0, 0xc0000, 0xd208, 0, 0xd0000, 0x4769, 0, 0xe0000,
			0xad8c, 0, 0xf0000, 0x923d, 0, 0x100000, 0x4a65, 0, 0x110000, 0x9958, 0, 0x120000, 0x8216, 0, 0x130000,
			0xaa91, 0, 0x140000, 0xd007, 0, 0x150000, 0xead, 0, 0x160000, 0x492, 0, 0x170000,
			0xffffffff, 0xffffffff, 0xffffffff } },
	{ "pjustic", 0, { 0x923d, 0, 0, 0x3e41, 0, 0x10000, 0xb7af, 0, 0x20000,
			  0x9651, 0, 0x30000, 0xad8c, 0, 0x40000, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "hmgeo",   0, { 0x6cc8, 0, 0x000000, 0x7b92, 0, 0x010000, 0x69bc, 0, 0x020000,
			  0x6d16, 0, 0x030000, 0x6134, 0, 0x040000, 0x1340, 0, 0x050000,
			  0x7716, 0, 0x060000, 0x2e1a, 0, 0x070000, 0x3030, 0, 0x080000,
			  0x0870, 0, 0x090000, 0x2856, 0, 0x0a0000, 0x4224, 0, 0x0b0000,
			  0x6df0, 0, 0x0c0000, 0x0dd8, 0, 0x0d0000, 0x576c, 0, 0x0e0000,
			  0x0534, 0, 0x0f0000, 0x0904, 0, 0x100000, 0x2f14, 0, 0x110000,
			  0x1792, 0, 0x120000, 0x6866, 0, 0x130000, 0x06fa, 0, 0x140000,
			  0x2842, 0, 0x150000, 0x7cc8, 0, 0x160000, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "wwfroyal",0, { 0xaaaa, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "gwing2",  0, { -1, 0x85ddc0, 0, 0xd567, 0, 0x10000, 0xe329, 0, 0x30000, 0xc112, 0, 0x50000,
			  0xabcd, 0, 0x70000, 0xef01, 0, 0x90000, 0x1234, 0, 0xb0000, 0x5678, 0, 0xd0000,
			  0x5555, 0, 0xf0000, 0x6666, 0, 0x110000, 0xa901, 0, 0x130000, 0xa802, 0, 0x150000,
			  0x3232, 0, 0x170000, 0x8989, 0, 0x190000, 0x6655, 0, 0x1a0000,
			  0x3944, 0, 0x1c0000, 0x655a, 0, 0x1d0000, 0xf513, 0, 0x1e0000,
			  0xb957, 0, 0, 0x37ca, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "pstone2", 0, { -2, 0x14db3f4,  0x000000, -2, 0xfbd0179d, 0x010000, -2, 0x9827117, 0x020000, -2, 0x69358f, 0x030000,
			  -2, 0x193954e, 0x040000, -2, 0xba50eb, 0x050000, -2, 0x9f1523, 0x060000, -2, 0xcb7b03, 0x070000,
			  -2, 0x8f712b, 0x080000, -2, 0x120f246, 0x090000, -2, 0xacc9fc, 0x0a0000, -2, 0x4eb319, 0x0b0000,
			  -2, 0x19d0c41, 0x0c0000, -2, 0x1077853, 0x0d0000, -2, 0x100019d, 0x0e0000, -2, 0xfd91596b, 0x0f0000,
			  -2, 0x63bae7, 0x100000, -2, 0x3e3685, 0x110000, -2, 0x6d08a9, 0x120000, -2, 0xfff85c5d, 0x130000,
			  -2, 0x5263bf, 0x140000, -2, 0x396180, 0x150000, -2, 0x73af6c, 0x160000, -2, 0xfffa8a76, 0x170000,
			  -2, 0xc2d9e0, 0x180000, -2, 0x33be72, 0x190000,
			  0xffffffff, 0xffffffff, 0xffffffff } },
	{ "toyfight", 0,{ 0x0615, 0, 0x8f058, 0x1999, 0, 0x8ec58, 0x7510, 0, 0x8f458, 0x5736, 0, 0x8e858,
		          0xffffffff, 0xffffffff, 0xffffffff } },
	{ "ggx",      0,{ -1, 0x200000, 0x100000, -1, 0x210004, 0x110000, -1, 0x220008, 0x120000, -1, 0x228000, 0x130000,
		          0x3af9, 0, 0x000000, 0x2288, 0, 0x010000, 0xe5e6, 0, 0x020000, 0xebb0, 0, 0x030000,
			  0x0228, 0, 0x040000, 0x872c, 0, 0x050000, 0xbba0, 0, 0x060000, 0x772f, 0, 0x070000,
			  0x2924, 0, 0x080000, 0x3222, 0, 0x090000, 0x7954, 0, 0x0a0000, 0x5acd, 0, 0x0b0000,
			  0xdd19, 0, 0x0c0000, 0x2428, 0, 0x0d0000, 0x3329, 0, 0x0e0000, 0x2142, 0, 0x0f0000,
		          0xffffffff, 0xffffffff, 0xffffffff } },
	{ "crzytaxi", 0,{ 0x0219, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "jambo",    0,{ 0x0223, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
	{ "18wheelr", 0,{ 0x1502, 0, 0, 0xffffffff, 0xffffffff, 0xffffffff } },
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
		//mame_printf_verbose("%s:ROM: read mask %" I64FMT "x @ %x\n", cpuexec_describe_context(machine), mem_mask, offset);
	}

	return U64(0xffffffffffffffff);
}

WRITE64_DEVICE_HANDLER( naomibd_w )
{
	naomibd_state *v = get_safe_token(device);
	INT32 i;

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

						// translate address if necessary
						if (v->prot_translate != NULL)
						{
							i = 0;
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
						break;

					case 0x2000000:
					case 0x2020000:
						#if NAOMIBD_PRINTF_PROTECTION
						printf("Protection write %04x to upload\n", (UINT32)(data&0xffff));
						#endif
						v->prot_sum += (INT16)(data&0xffff);
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
	#if NAOMIBD_PRINTF_PROTECTION
	v->prot_pio_count = 0;
	#endif
	for (i=0; i<ARRAY_LENGTH(naomibd_translate_tbl); i++)
	{
		if (!strcmp(device->machine->gamedrv->name, naomibd_translate_tbl[i].name))
		{
			v->prot_translate = &naomibd_translate_tbl[i].transtbl[0];
			v->prot_reverse_bytes = naomibd_translate_tbl[i].reverse_bytes;
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

DEVICE_GET_INFO( naomibd )
{
	const naomibd_config *config = (device != NULL) ? (const naomibd_config *)device->inline_config : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(naomibd_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(naomibd_config);				break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;
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
