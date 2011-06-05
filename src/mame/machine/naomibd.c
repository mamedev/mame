/*************************************************************************

    Naomi plug-in board emulator

    emulator by Samuele Zannoli and R. Belmont
    reverse engineering by ElSemi, Deunan Knute, Andreas Naive, Olivier Galibert, and Cah4e3

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

    * bit 29 (mode bit 1) is "M1" compression bit on Actel carts, other functions on others
    It's actually the opposite, when set the addressing is following the chip layout and when cleared the protection chip will have it's fun
    doing a decompression + XOR on the data for Actel carts.  Non-Actel carts may ignore this bit or remap the address space.

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
#define NAOMIBD_FLAG_DMA_COMPRESSION    (2)	// 0 protection chip decompresses DMA data, 1 for normal DMA reads

#define NAOMIBD_PRINTF_PROTECTION	(0)	// 1 to printf protection access details

/*************************************
 *
 *  Structures
 *
 *************************************/

#define PACK_BUF_SIZE (32768)

enum
{
	CMD_READY = 0,
	CMD_FETCH,
	CMD_REPEAT
};

typedef struct _naomibd_config_table naomibd_config_table;
struct _naomibd_config_table
{
	const char *name;
	int m2m3_key;
    int m1_key;
};

typedef struct _naomibd_prot naomibd_prot;
struct _naomibd_prot
{
	UINT16 last_word, aux_word, pak_word, heading_word;
	UINT16 *ptr;
	int count, pak_bit, control_bits, pak_state, dec_count, pak_buf_size, pak_buf_pos;
    int pak_fetch_ofs;
    UINT8 pak_byte, cmd_byte;
    int seed;
    UINT8 pak_buf[PACK_BUF_SIZE];

    UINT8 *s_input;
    UINT8 s_xor[4];
    UINT8 s_dict[111];
    int s_subst, s_out_len, s_out_cnt, s_shift, s_bits, s_in_len, s_in_pos;
};

typedef struct _naomibd_state naomibd_state;
struct _naomibd_state
{
	UINT8				index;					/* index of board */
	UINT8				type;
	device_t *device;				/* pointer to our containing device */

	UINT8 *				memory;
	chd_file *			gdromchd;
	UINT8 *				picdata;
	UINT32				rom_offset, rom_offset_flags, dma_count;
	UINT32				dma_offset, dma_offset_flags;
	UINT32				prot_offset, prot_key;
	UINT32				aw_offset, aw_file_base, aw_file_offset;

	// live decrypt vars
	UINT32				dc_gamekey, dc_seqkey, dc_dmakey;
	UINT8				dc_cart_ram[256*1024];	// internal cartridge RAM
	INT32				dc_m3_ptr;

	naomibd_prot		prot;
};

// maps protection offsets to real addresses
// format of array: encryption key, address written, address to switch out with.  if key is -1 it's ignored and address written is the match.
// if key is not -1, it's used for the match instead of the address written.
static const naomibd_config_table naomibd_translate_tbl[] =
{
	// games where on-the-fly decryption works (many of these are fully playable in MAME, just slow)
	{ "18wheelr", 0x07cf54, 0 },
	{ "alpilota", 0x070e41, 0 },
	{ "alpiltdx", 0x070e41, 0 },
	{ "capsnk", 0, 0 },
	{ "capsnka", 0, 0 },
	{ "crackndj", 0x1c2347, 0 },
	{ "crzytaxi", 0x0d2f45, 0 },
	{ "csmash", 0x103347, 0 },
	{ "csmasho", 0x103347, 0 },
	{ "cspike", 0x0e2010, 0 },
	{ "deathcox", 0x0b64d0, 0 },
	{ "derbyoc", 0x0fee35, 0 },
	{ "doa2", 0x8ad01, 0 },
	{ "doa2m", 0x8ad01, 0 },
	{ "dybb99", 0x048a01, 0 },
	{ "f355twin", 0x06efd4, 0 },
	{ "f355twn2", 0x1666c6, 0 },
	{ "ggram2", 0x074a61, 0 },
	{ "ggx", 0x076110, 0 },
    { "gram2000", 0, 0x7f805c3f },
	{ "gundmct", 0x0e8010, 0 },
	{ "gwing2",  0x0b25d0, 0 },
	{ "hmgeo",   0x038510, 0 },
	{ "jambo",   0x0fab95, 0 },
    { "kick4csh", 0, 0x820857c9 },
    { "mvsc2", 0, 0xc18b6e7c },
	{ "otrigger", 0x0fea94, 0 },
	{ "pjustic", 0x0725d0, 0 },
	{ "pstone", 0x0e69c1, 0 },
	{ "pstone2", 0x0b8dc0, 0 },
	{ "puyoda", 0x0acd40, 0 },
    { "qmegamis", 0, 0xcd9b4896 },
	{ "ringout", 0x0b1e40, 0 },
	{ "samba", 0x0a8b5d, 0 },
	{ "samba2k", 0x1702cf, 0 },
	{ "sgtetris", 0x8ae51, 0 },
    { "shootopl", 0, 0xa0f37ca7 },
    { "shootpl", 0, 0x9d8de9cd },
    { "shootplm", 0, 0x9d8de9cd },
	{ "slasho", 0x1a66ca, 0 },
	{ "smlg99", 0x048a01, 0 },
	{ "spawn", 0x078d01, 0 },
	{ "sstrkfgt", 0x132303, 0 },
	{ "suchie3", 0x0368e1, 0 },
	{ "toyfight", 0x02ca85, 0 },
	{ "vf4cart", 0x2ef2f96, 0 },
    { "vf4evoct", 0, 0x1e5bb0cd },
    { "virnbao", 0x68b58, 0 },      // note: "virnba" set doesn't have protection
    { "vs2_2k", 0x88b08, 0 },
	{ "vtennis", 0x03eb15, 0 },
	{ "vtenis2c", 0, 0x2d2d4743 },
	{ "vonot", 0x010715, 0 },
	{ "wldkicks", 0xae2901, 0 },
	{ "wwfroyal", 0x1627c3, 0 },
	{ "zerogu2", 0x07c010, 0 },
	{ "zombrvn", 0x012b41, 0 },
};

// forward declaration for decrypt function
static UINT16 block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a naomibd device
-------------------------------------------------*/

INLINE naomibd_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == NAOMI_BOARD);

	return (naomibd_state *)downcast<legacy_device_base *>(device)->token();
}



/*************************************
 *
 *  Misc. functions
 *
 *************************************/

int naomibd_interrupt_callback(device_t *device, naomibd_interrupt_func callback)
{
	naomibd_config *config = (naomibd_config *)downcast<const legacy_device_base *>(device)->inline_config();
	config->interrupt = callback;
	return 0;
}

int naomibd_get_type(device_t *device)
{
	naomibd_state *v = get_safe_token(device);
	return v->type;
}

// M1 decryption/decompression
static UINT8 naomibd_m1dec_readbyte(naomibd_state *naomibd)
{
    UINT8 v = 0;

    switch (naomibd->prot.s_in_pos & 3)
    {
        case 0:
            v = naomibd->prot.s_input[naomibd->prot.s_in_pos + 3];
            v ^= naomibd->prot.s_input[naomibd->prot.s_in_pos + 1];
            break;

        case 1:
            v = naomibd->prot.s_input[naomibd->prot.s_in_pos + 1];
            v ^= naomibd->prot.s_input[naomibd->prot.s_in_pos - 1];
            break;

        case 2:
            v = naomibd->prot.s_input[naomibd->prot.s_in_pos - 1];
            break;

        case 3:
            v = naomibd->prot.s_input[naomibd->prot.s_in_pos - 3];
            break;
    }

    v ^= naomibd->prot.s_xor[naomibd->prot.s_in_pos & 3];
    naomibd->prot.s_in_pos++;
    return v;
}

static void naomibd_m1dec_storebyte (naomibd_state *naomibd, UINT8 b)
{
    if (naomibd->prot.s_subst && naomibd->prot.s_out_cnt >= 2)
    {
        b = naomibd->dc_cart_ram[naomibd->prot.s_out_cnt - 2] - b;
    }
    naomibd->dc_cart_ram[naomibd->prot.s_out_cnt] = b;
    naomibd->prot.s_out_cnt++;

    if (naomibd->prot.s_out_cnt >= (256*1024))
    {
        fatalerror("naomibd: M1 decode exceeds buffer size!\n");
    }
}

static void naomibd_m1dec_shiftin(naomibd_state *naomibd)
{
    naomibd->prot.s_shift <<= 8;
    naomibd->prot.s_shift |= naomibd_m1dec_readbyte(naomibd);
    naomibd->prot.s_bits += 8;
}

static void naomibd_m1_decode(naomibd_state *naomibd)
{
    int i, eos;

    naomibd->prot.s_xor [0] = (UINT8)naomibd->dc_dmakey;
    naomibd->prot.s_xor [1] = (UINT8)(naomibd->dc_dmakey >> 8);
    naomibd->prot.s_xor [2] = (UINT8)(naomibd->dc_dmakey >> 16);
    naomibd->prot.s_xor [3] = (UINT8)(naomibd->dc_dmakey >> 24);

	#if NAOMIBD_PRINTF_PROTECTION
    printf("M1 decode: dma offset %x, key %x\n", naomibd->dma_offset, naomibd->dc_dmakey);
    #endif

    naomibd->prot.s_input = naomibd->memory + naomibd->dma_offset;
    naomibd->prot.s_in_pos = 0;

    // read in the dictionary
    for (i = 0; i < 111; i++)
    {
        naomibd->prot.s_dict [i] = naomibd_m1dec_readbyte(naomibd);
    }

    // control bits
    naomibd->prot.s_subst = (naomibd->prot.s_dict [0] & 64) ? 1 : 0;

    // command stream
    naomibd->prot.s_out_cnt = 0, eos = 0;
    naomibd->prot.s_shift = 0, naomibd->prot.s_bits = 0;
    while (!eos)
    {
        int code, addr, t;

        if (naomibd->prot.s_bits < 2)
            naomibd_m1dec_shiftin(naomibd);

        code = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 2)) & 3;
        switch (code)
        {
            case 0:
                // 00-aa
                if (naomibd->prot.s_bits < 4)
                    naomibd_m1dec_shiftin(naomibd);
                addr = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 4)) & 3;
                naomibd->prot.s_bits -= 4;
                if (addr == 0)
                {
                    // quotation
                    if (naomibd->prot.s_bits < 8)
                        naomibd_m1dec_shiftin (naomibd);
                    t = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 8)) & 255;
                    naomibd->prot.s_bits -= 8;
                    naomibd_m1dec_storebyte(naomibd, t);
                    break;
                }
                naomibd_m1dec_storebyte(naomibd, naomibd->prot.s_dict [addr]);
                break;

                case 1:
                    if (naomibd->prot.s_bits < 5)
                        naomibd_m1dec_shiftin (naomibd);
                    t = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 3)) & 1;
                    if (t == 0)
                    {
                        // 010-aa
                        addr = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 5)) & 3;
                        addr += 4;
                        naomibd->prot.s_bits -= 5;
                    }
                    else
                    {
                        // 011-aaa
                        if (naomibd->prot.s_bits < 6)
                            naomibd_m1dec_shiftin (naomibd);
                        addr = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 6)) & 7;
                        addr += 8;
                        naomibd->prot.s_bits -= 6;
                    }
                    naomibd_m1dec_storebyte(naomibd, naomibd->prot.s_dict [addr]);
                    break;

                case 2:
                    if (naomibd->prot.s_bits < 7)
                        naomibd_m1dec_shiftin(naomibd);
                    // 10-aaaaa
                    addr = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 7)) & 31;
                    addr += 16;
                    naomibd->prot.s_bits -= 7;
                    naomibd_m1dec_storebyte(naomibd, naomibd->prot.s_dict [addr]);
                    break;

                case 3:
                    if (naomibd->prot.s_bits < 8)
                        naomibd_m1dec_shiftin(naomibd);
                    // 11-aaaaaa
                    addr = (naomibd->prot.s_shift >> (naomibd->prot.s_bits - 8)) & 63;
                    addr += 48;
                    naomibd->prot.s_bits -= 8;
                    if (addr == 111)
                    {
                        // end of stream
                        eos = 1;
                    }
                    else
                    {
                        naomibd_m1dec_storebyte(naomibd, naomibd->prot.s_dict [addr]);
                    }
                    break;
        }
    }
}

// Streaming M2/M3 protection and decompression

INLINE UINT16 naomi_bswap16(UINT16 in)
{
    return ((in>>8) | (in<<8));
}

static UINT16 naomibd_get_decrypted_stream(naomibd_state *naomibd)
{
	UINT16 wordn = naomi_bswap16(naomibd->prot.ptr[naomibd->prot.count++]);

	naomibd->prot.aux_word = block_decrypt(naomibd->dc_gamekey, naomibd->dc_seqkey, naomibd->prot.seed++, wordn);
	wordn = (naomibd->prot.last_word&~3) | (naomibd->prot.aux_word&3);
	naomibd->prot.last_word = naomibd->prot.aux_word;

	return wordn;
}

static void naomibd_init_stream(naomibd_state *naomibd)
{
	naomibd->prot.last_word = 0;

	naomibd->prot.control_bits = naomibd_get_decrypted_stream(naomibd);
	naomibd->prot.heading_word = naomibd_get_decrypted_stream(naomibd);

	if (naomibd->prot.control_bits & 2)
	{
	   naomibd->prot.pak_bit = 0;
	   naomibd->prot.pak_state = CMD_READY;
	   naomibd->prot.dec_count = 0;
	   naomibd->prot.pak_buf_size = 256 << (naomibd->prot.control_bits & 1);
	   naomibd->prot.pak_buf_pos = 0;
	}
}

static UINT16 naomibd_get_compressed_bit(naomibd_state *naomibd)
{
   if(naomibd->prot.pak_bit == 0)
   {
       naomibd->prot.pak_bit = 15;
       naomibd->prot.pak_word = naomibd_get_decrypted_stream(naomibd);
   }
   else
   {
       naomibd->prot.pak_bit--;
       naomibd->prot.pak_word<<=1;
   }
   return naomibd->prot.pak_word >> 15;
}

static UINT16 naomibd_get_decompressed_stream(naomibd_state *naomibd)
{
/* node format
0xxxxxxx - next node index
1a0bbccc - end node
           a - 0 = repeat
               1 = fetch
           b - if a = 1
               00 - fetch  0
               01 - fetch  1
               11 - fetch -1
               if a = 0
               000
           c - repeat/fetch counter
               count = ccc + 1
11111111 - empty node
*/
   static UINT8 trees[9][2][32] = {
      {
         {0x01,0x10,0x0f,0x05,0xc4,0x13,0x87,0x0a,0xcc,0x81,0xce,0x0c,0x86,0x0e,0x84,0xc2,
          0x11,0xc1,0xc3,0xcf,0x15,0xc8,0xcd,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc7,0x02,0x03,0x04,0x80,0x06,0x07,0x08,0x09,0xc9,0x0b,0x0d,0x82,0x83,0x85,0xc0,
          0x12,0xc6,0xc5,0x14,0x16,0xca,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x02,0x80,0x05,0x04,0x81,0x10,0x15,0x82,0x09,0x83,0x0b,0x0c,0x0d,0xdc,0x0f,0xde,
          0x1c,0xcf,0xc5,0xdd,0x86,0x16,0x87,0x18,0x19,0x1a,0xda,0xca,0xc9,0x1e,0xce,0xff,},
         {0x01,0x17,0x03,0x0a,0x08,0x06,0x07,0xc2,0xd9,0xc4,0xd8,0xc8,0x0e,0x84,0xcb,0x85,
          0x11,0x12,0x13,0x14,0xcd,0x1b,0xdb,0xc7,0xc0,0xc1,0x1d,0xdf,0xc3,0xc6,0xcc,0xff,},
      },
      {
         {0xc6,0x80,0x03,0x0b,0x05,0x07,0x82,0x08,0x15,0xdc,0xdd,0x0c,0xd9,0xc2,0x14,0x10,
          0x85,0x86,0x18,0x16,0xc5,0xc4,0xc8,0xc9,0xc0,0xcc,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0x01,0x02,0x12,0x04,0x81,0x06,0x83,0xc3,0x09,0x0a,0x84,0x11,0x0d,0x0e,0x0f,0x19,
          0xca,0xc1,0x13,0xd8,0xda,0xdb,0x17,0xde,0xcd,0xcb,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0x80,0x0d,0x04,0x05,0x15,0x83,0x08,0xd9,0x10,0x0b,0x0c,0x84,0x0e,0xc0,0x14,
          0x12,0xcb,0x13,0xca,0xc8,0xc2,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc5,0x02,0x03,0x07,0x81,0x06,0x82,0xcc,0x09,0x0a,0xc9,0x11,0xc4,0x0f,0x85,0xd8,
          0xda,0xdb,0xc3,0xdc,0xdd,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0x80,0x06,0x0c,0x05,0x81,0xd8,0x84,0x09,0xdc,0x0b,0x0f,0x0d,0x0e,0x10,0xdb,
          0x11,0xca,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc4,0x02,0x03,0x04,0xcb,0x0a,0x07,0x08,0xd9,0x82,0xc8,0x83,0xc0,0xc1,0xda,0xc2,
          0xc9,0xc3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0x02,0x06,0x0a,0x83,0x0b,0x07,0x08,0x09,0x82,0xd8,0x0c,0xd9,0xda,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc3,0x80,0x03,0x04,0x05,0x81,0xca,0xc8,0xdb,0xc9,0xc0,0xc1,0x0d,0xc2,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0x02,0x03,0x04,0x81,0x07,0x08,0xd8,0xda,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc2,0x80,0x05,0xc9,0xc8,0x06,0x82,0xc0,0x09,0xc1,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0x80,0x04,0xc8,0xc0,0xd9,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc1,0x02,0x03,0x81,0x05,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
      {
         {0x01,0xd8,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
         {0xc0,0x80,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,},
      },
   };

   UINT32 word_complete = 2;
   UINT16 wordn = 0;

   while(word_complete)
   {
      switch(naomibd->prot.pak_state)
      {
      case CMD_READY:
      {
         UINT32 tmp = 0;
         INT32 slot = (naomibd->prot.pak_buf_pos & (naomibd->prot.pak_buf_size-1));
         if(slot > 0)
         {
            if(slot < (naomibd->prot.pak_buf_size-7))
               slot = 1;
            else
               slot = (slot & 7) + 1;
         }
         while (!(tmp&0x80))
           if(naomibd_get_compressed_bit(naomibd))
              tmp = trees[slot][1][tmp];
           else
              tmp = trees[slot][0][tmp];
         if(tmp != 0xff)
         {
            naomibd->prot.pak_byte = (tmp&7)+1;
            if(tmp&0x40)
            {
               static INT32 cmds[4] = {0, 1, 0, -1};
               naomibd->prot.pak_fetch_ofs = cmds[(tmp&0x18)>>3];
               naomibd->prot.pak_state = CMD_FETCH;
            }
            else
            {
               UINT8 byten;
               naomibd->prot.pak_state = CMD_REPEAT;
               byten =          naomibd_get_compressed_bit(naomibd)  << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten = (byten | naomibd_get_compressed_bit(naomibd)) << 1;
               byten =  byten | naomibd_get_compressed_bit(naomibd);
               naomibd->prot.cmd_byte = byten;
            }
         }
         break;
      }
      case CMD_FETCH:
         naomibd->prot.cmd_byte = naomibd->prot.pak_buf[(naomibd->prot.pak_buf_pos-naomibd->prot.pak_buf_size+naomibd->prot.pak_fetch_ofs)&(PACK_BUF_SIZE-1)];

      case CMD_REPEAT:
         naomibd->prot.pak_buf[naomibd->prot.pak_buf_pos&(PACK_BUF_SIZE-1)]=naomibd->prot.cmd_byte;
         if(word_complete&2)
		 {
            wordn = naomibd->prot.cmd_byte << 8;
		 }
         else
		 {
            wordn = wordn | naomibd->prot.cmd_byte;
		 }
         word_complete--;
         naomibd->prot.pak_byte--;
         naomibd->prot.pak_buf_pos++;
         if(naomibd->prot.pak_byte == 0) naomibd->prot.pak_state = CMD_READY;
         break;
      }
   }
   return wordn;
}

// stream read protected PIO hook
//-----------------------------------------------------------
static UINT16 naomibd_get_data_stream(naomibd_state *naomibd)
{
	UINT16 wordn;

	if(naomibd->prot.control_bits&2)
	{
		wordn = naomibd_get_decompressed_stream(naomibd);
	}
	else
	{
		wordn = naomibd_get_decrypted_stream(naomibd);
	}

	return wordn;
}

void *naomibd_get_memory(device_t *device)
{
	naomibd_state *naomibd = get_safe_token(device);

    // for M1 decodes, return the buffer we'll DMA from
	if (!(naomibd->dma_offset_flags & NAOMIBD_FLAG_DMA_COMPRESSION) && (naomibd->type == ROM_BOARD) && (naomibd->dc_dmakey != 0))
    {
        // perform the M1 decode
        naomibd_m1_decode(naomibd);

        // return the pointer to our output buffer
        return naomibd->dc_cart_ram;
    }

    if (!(naomibd->dma_offset_flags & NAOMIBD_FLAG_DMA_COMPRESSION) && (naomibd->type == ROM_BOARD))
    {
        logerror("Unhandled M1 DMA with key %x, flags %x, offset %x\n", naomibd->dc_dmakey, naomibd->dma_offset_flags, naomibd->dma_offset);
    }

	return get_safe_token(device)->memory;
}

offs_t naomibd_get_dmaoffset(device_t *device)
{
	naomibd_state *naomibd = get_safe_token(device);
	offs_t result = 0;

	#if NAOMIBD_PRINTF_PROTECTION
        printf("DMA source %08x, flags %x\n", get_safe_token(device)->dma_offset, get_safe_token(device)->dma_offset_flags);
	#endif

	// if the flag is cleared that lets the protection chip go,
	// we need to handle this specially.  but not on DIMM boards or if there's no key.
	if (!(naomibd->dma_offset_flags & NAOMIBD_FLAG_DMA_COMPRESSION) && (naomibd->type == ROM_BOARD) && (naomibd->dc_dmakey != 0))
	{
        // no offset, start at the beginning of cart ram for M1 transfers
        result = 0;
	}
    else
    {
        result = get_safe_token(device)->dma_offset;
    }

	return result;
}


/*************************************
 *
 *  Common initialization
 *
 *************************************/



static void naomibd_postload(naomibd_state *v)
{
}


static void init_save_state(device_t *device)
{
	naomibd_state *v = get_safe_token(device);

	device->machine().save().register_postload(save_prepost_delegate(FUNC(naomibd_postload), v));

	/* register states */
	device->save_item(NAME(v->rom_offset));
	device->save_item(NAME(v->rom_offset_flags));
	device->save_item(NAME(v->dma_count));
	device->save_item(NAME(v->dma_offset));
	device->save_item(NAME(v->dma_offset_flags));
	device->save_item(NAME(v->prot_offset));
	device->save_item(NAME(v->prot_key));
	device->save_item(NAME(v->aw_offset));
	device->save_item(NAME(v->aw_file_base));
	device->save_item(NAME(v->aw_file_offset));
	device->save_item(NAME(v->dc_m3_ptr));
	device->save_item(NAME(v->dc_cart_ram));
    device->save_item(NAME(v->prot.last_word));
    device->save_item(NAME(v->prot.aux_word));
    device->save_item(NAME(v->prot.pak_word));
    device->save_item(NAME(v->prot.heading_word));
    device->save_item(NAME(v->prot.count));
    device->save_item(NAME(v->prot.pak_bit));
    device->save_item(NAME(v->prot.control_bits));
    device->save_item(NAME(v->prot.pak_state));
    device->save_item(NAME(v->prot.dec_count));
    device->save_item(NAME(v->prot.pak_buf_size));
    device->save_item(NAME(v->prot.pak_buf_pos));
    device->save_item(NAME(v->prot.pak_fetch_ofs));
    device->save_item(NAME(v->prot.pak_byte));
    device->save_item(NAME(v->prot.cmd_byte));
    device->save_item(NAME(v->prot.seed));
    device->save_item(NAME(v->prot.s_xor));
    device->save_item(NAME(v->prot.s_dict));
    device->save_item(NAME(v->prot.s_subst));
    device->save_item(NAME(v->prot.s_out_len));
    device->save_item(NAME(v->prot.s_out_cnt));
    device->save_item(NAME(v->prot.s_shift));
    device->save_item(NAME(v->prot.s_bits));
    device->save_item(NAME(v->prot.s_in_len));
    device->save_item(NAME(v->prot.s_in_pos));
}



/*************************************
 *
 *  Reset
 *
 *************************************/

static void soft_reset(naomibd_state *v)
{
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
				ret = (UINT64)naomibd_get_data_stream(v);
			}
			else
			{
                ret = U64(0);
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

		ret = device->machine().device<x76f100_device>("naomibd_eeprom")->sda_r() << 15;

		return ret << 32;
	}
	else
	{
		//mame_printf_verbose("%s:ROM: read mask %" I64FMT "x @ %x\n", machine.describe_context(), mem_mask, offset);
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
					v->dma_offset_flags = NAOMIBD_FLAG_DMA_COMPRESSION|NAOMIBD_FLAG_AUTO_ADVANCE;	// force normal DMA mode
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
				v->dma_offset_flags = (data>>(28+16));
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
						printf("Protection: set up read @ %x, key %x (PIO %x DMA %x) [%s]\n", v->prot_offset*2, v->prot_key, v->rom_offset, v->dma_offset, device->machine().describe_context());
						#endif

						// if dc_gamekey isn't -1, we can live-decrypt this one
						if (v->dc_gamekey != -1)
						{
							UINT8 *ROM = (UINT8 *)v->memory;

							v->dc_seqkey = v->prot_key;

							if (v->prot_offset != 0x2000000/2)
							{
								// M2: decrypt from ROM
								v->prot.ptr = (UINT16 *)&ROM[v->prot_offset*2];
								v->prot.seed = v->prot_offset&0xffff;
    							#if NAOMIBD_PRINTF_PROTECTION
    							printf("M2 decrypt: gamekey %x seqkey %x length %x\n", v->dc_gamekey, v->dc_seqkey, v->dc_m3_ptr);
    							#endif
							}
							else
							{
								// M3: decrypt from cart ram
								v->prot.ptr = (UINT16 *)v->dc_cart_ram;
								v->prot.seed = 0;
    							#if NAOMIBD_PRINTF_PROTECTION
    							printf("M3 decrypt: gamekey %x seqkey %x length %x\n", v->dc_gamekey, v->dc_seqkey, v->dc_m3_ptr);
    							#endif
							}

							v->prot.count = 0;
							naomibd_init_stream(v);

							v->dc_m3_ptr = 0;
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
				x76f100_device *x76f100 = device->machine().device<x76f100_device>("naomibd_eeprom");
				// NAOMI_BOARDID_WRITE
				x76f100->cs_w((data >> 2) & 1);
				x76f100->rst_w((data >> 3) & 1);
				x76f100->scl_w((data >> 1) & 1);
				x76f100->sda_w((data >> 0) & 1);
			}
		}
		break;
		default:
			mame_printf_verbose("%s: ROM: write %" I64FMT "x to %x, mask %" I64FMT "x\n", device->machine().describe_context(), data, offset, mem_mask);
			break;
	}
}



/*************************************
 *
 *  Load rom file from gdrom
 *
 *************************************/

#define FILENAME_LENGTH 24

static void load_rom_gdrom(running_machine& machine, naomibd_state *v)
{
//  UINT32 result;
	cdrom_file *gdromfile;
	UINT8 buffer[2048];
	UINT8 *ptr;
	UINT32 start,size,sectors,dir;
	int pos,len,a;
	char name[128];
	UINT64 key;
	UINT8* realpic; // todo, add to device

	memset(name,'\0',128);

	realpic = machine.region("pic")->base();

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
	cdrom_read_data(gdromfile, 0xb06e - 150, buffer, CD_TRACK_MODE1);
	start=((buffer[0x8c+0] << 0) |
		   (buffer[0x8c+1] << 8) |
		   (buffer[0x8c+2] << 16) |
		   (buffer[0x8c+3] << 24));
	// path table
	cdrom_read_data(gdromfile, start, buffer, CD_TRACK_MODE1);
	start=((buffer[0x2+0] << 0) |
		   (buffer[0x2+1] << 8) |
		   (buffer[0x2+2] << 16) |
		   (buffer[0x2+3] << 24));
	dir = start;
	// directory
	cdrom_read_data(gdromfile, dir, buffer, CD_TRACK_MODE1);
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
		cdrom_read_data(gdromfile, start, buffer, CD_TRACK_MODE1);
		// get "rom" file name
		memset(name,'\0', 128);
		memcpy(name, buffer+0xc0, FILENAME_LENGTH-1);


		// directory
		cdrom_read_data(gdromfile, dir, buffer, CD_TRACK_MODE1);
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
				cdrom_read_data(gdromfile, start, ptr, CD_TRACK_MODE1);
				ptr += 2048;
				start++;
				sectors--;
			}
		}
	}
	// get des key
	realpic = machine.region("pic")->base();

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
the game-key (>=25 bits) and the sequence-key (16 bits) and output a middle result (16 bits) which will act as another key
for the second one. The second FN will take the encrypted word (16 bits), the game-key, the sequence-key and the result
from the first FN and will output the decrypted word (16 bits).

Each round of the Feistel Networks use four substitution sboxes, each having 6 inputs and 2 outputs. The input can be the
XOR of at most two "sources bits", being source bits the bits from the previous round and the bits from the different keys.

The underlying block cipher has the same structure than the one used by the CPS-2 (Capcom Play System 2) and,
indeed, some of the used sboxes are exactly the same and appear in the same FN/round in both systems (this is not evident,
as you need to apply a bitswapping and some XORs to the input & output of the sboxes to get the same values due). However,
the key scheduling used by this implementation is much weaker than the CPS-2's one. Many s-boxes inputs are XORed with any
key bit.

Due to the small key-length, no sophisticated attacks are needed to recover the keys; a brute-force attack knowing just
some (encrypted word-decrypted word) pairs suffice. However, due to the weak key scheduling, it should be noted that some
related keys can produce the same output bytes for some (short) input sequences.

The only difference in the decryption process between M2 and M3 is the initialization of the counter. In M3, the counter is
always set to 0 at the beginning of the decryption while, in M2, the bits #1-#16 of the ciphertext's address are used
to initialize the counter.

Due to the nature of the cipher, there are some degrees of freedom when choosing the s-boxes and keys values; by example,
you could apply a fixed bitswapping and XOR to the keys and the decryption would remain the same as long as you change
accordingly the s-boxes' definitions. So the order of the bits in the keys is arbitrary, and the s-boxes values have been
chosen so as to make the key for CAPSNK equal to 0.

It can be observed that a couple of sboxes have incomplete tables (a 255 value indicate an unknown value). The recovered keys
as of december/2010 show small randomness and big correlations, making possible that some unseen bits could make the
decryption need those incomplete parts.

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
                2,3,1,1,2,2,1,1,1,0,2,3,3,0,2,1,1,1,1,1,3,0,3,2,1,0,1,2,0,3,1,3,
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
                3,0,2,0,0,2,2,1,0,2,3,3,1,3,1,0,1,3,3,0,0,1,3,1,0,2,0,3,2,1,0,1,
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
                // unused?
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
                0,1,0,2,0,1,1,3,2,0,3,2,1,1,1,3,2,3,0,2,3,0,2,2,1,3,0,1,1,2,2,2,
            },
            {0,2,3,4,7,-1},
            {3,4}
        },

        {
            {
                2,3,1,3,2,0,1,2,0,0,3,3,3,3,3,1,2,0,2,1,2,3,0,2,0,1,0,3,0,2,1,0,
                2,3,0,1,3,0,3,2,3,1,2,0,3,1,1,2,0,3,0,0,2,0,2,1,2,2,3,2,1,2,3,1,
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
                // unused?
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

static const int fn1_game_key_scheduling[38][2] =
{
    {1,29},  {1,71},  {2,4},   {2,54},  {3,8},   {4,56},  {4,73},  {5,11},
    {6,51},  {7,92},  {8,89},  {9,9},   {9,10},  {9,39},  {9,41},  {9,58},
    {9,59},  {9,86},  {10,90}, {11,6},  {12,64}, {13,49}, {14,44}, {15,40},
    {16,69}, {17,15}, {18,23}, {18,43}, {19,82}, {20,81}, {21,32}, {21,61},
    {22,5},  {23,66}, {24,13}, {24,45}, {25,12}, {25,35}
};

static const int fn2_game_key_scheduling[34][2] =
{
    {0,0},   {1,3},   {2,11},  {3,20},  {4,22},  {5,23},  {6,29},  {7,38},
    {8,39},  {9,47},  {9,55},  {9,86},  {9,87},  {9,90},  {10,50}, {10,53},
    {11,57}, {12,59}, {13,61}, {13,64}, {14,63}, {15,67}, {16,72}, {17,83},
    {18,88}, {19,94}, {20,35}, {21,17}, {21,92}, {22,6},  {22,11}, {23,85},
    {24,16}, {25,25}
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

    for (j=0; j<38; ++j)
    {
        if (BIT(game_key, fn1_game_key_scheduling[j][0])!=0)
        {
            aux = fn1_game_key_scheduling[j][1]%24;
            aux2 = fn1_game_key_scheduling[j][1]/24;
            fn1_subkeys[aux2] ^= (1<<aux);
        }
    }

    for (j=0; j<34; ++j)
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

/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( naomibd )
{
	const naomibd_config *config = (const naomibd_config *)downcast<const legacy_device_base *>(device)->inline_config();
	naomibd_state *v = get_safe_token(device);
	int i;

	/* validate some basic stuff */
	assert(device->static_config() == NULL);
	assert(downcast<const legacy_device_base *>(device)->inline_config() != NULL);

	/* validate configuration */
	assert(config->type >= ROM_BOARD && config->type < MAX_NAOMIBD_TYPES);

	/* store a pointer back to the device */
	v->device = device;

    v->dc_dmakey = 0;

	for (i=0; i<ARRAY_LENGTH(naomibd_translate_tbl); i++)
	{
		if (!strcmp(device->machine().system().name, naomibd_translate_tbl[i].name))
		{
            v->dc_gamekey = naomibd_translate_tbl[i].m2m3_key;
            v->dc_dmakey = naomibd_translate_tbl[i].m1_key;
			break;
		}
	}

	/* configure type-specific values */
	switch (config->type)
	{
		case ROM_BOARD:
			v->memory = (UINT8 *)device->machine().region(config->regiontag)->base();
			break;

		case AW_ROM_BOARD:
			v->memory = (UINT8 *)device->machine().region(config->regiontag)->base();
			break;

		case DIMM_BOARD:
			v->memory = (UINT8 *)auto_alloc_array_clear(device->machine(), UINT8, 0x40000000); // 0x40000000 is needed for some Chihiro sets, Naomi should be less, we should pass as device param
			v->gdromchd = get_disk_handle(device->machine(), config->gdromregiontag);
			v->picdata = (UINT8 *)device->machine().region(config->picregiontag)->base();
			if (v->memory != NULL && v->gdromchd != NULL && v->picdata != NULL)
				load_rom_gdrom(device->machine(), v);
			break;

		default:
			fatalerror("Unsupported plug-in board in naomibd_start!");
			break;
	}

	/* set the type */
	v->index = device->machine().devicelist().indexof(device->type(), device->tag());
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
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( naomibd )
{
	const naomibd_config *config = (device != NULL) ? (const naomibd_config *)downcast<const legacy_device_base *>(device)->inline_config() : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(naomibd_state);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(naomibd_config);				break;

		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = NULL;							break;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = NULL;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(naomibd);		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(naomibd);			break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(naomibd);		break;

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


DEFINE_LEGACY_DEVICE(NAOMI_BOARD, naomibd);
