// license:BSD-3-Clause
// copyright-holders:Olivier Galibert,Andreas Naive

#include "emu.h"
#include "awboard.h"
#include <algorithm>

/*

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
  reading header and program code data, cannot be used for reading MPR-ROMs data.

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

 AW_PIO_DATA                                             Register addres: 0x5f7080
 +-------------------------------------------------------------------------------+
 |                                  bit15-0                                      |
 +-------------------------------------------------------------------------------+
 |                Read/Write word from/to ROM board address space                |
 +-------------------------------------------------------------------------------+

  Using this register data can be read or written to ROM BD at AW_EPR_OFFSET directly,
  decryption is not used, flash ROMs (re)programming via CFI commands possible.

  Type 2 ROM BD have MPR_BANK register at AW_EPR_OFFSET 007fffff, which selects
  1 of 4 mask ROM banks.

ROM board internal layouts:

 Type 1:

 00000000 - 00800000 IC18 flash ROM
 00800000 - 01000000 unk, probably mirror of above
 01000000 - 02000000 IC10 \
        .....               mask ROMs
 07000000 - 08000000 IC17 /

 Type 2:

 00000000 - 00800000 FMEM1 flash ROM
 00800000 - 01000000 FMEM2 flash ROM
 01000000 - 02000000 unk, probably mirror of above
 02000000 - 04000000 MROM1 MROM4 MROM7 MROM10 \
 04000000 - 06000000 MROM2 MROM5 MROM8 MROM11   banked mask ROMs
 06000000 - 08000000 MROM3 MROM6 MROM9 MROM12 /

 Type 3:

 00000000 - 01000000 U3  flash ROM
 01000000 - 02000000 U1  flash ROM
 02000000 - 03000000 U4  flash ROM
 03000000 - 04000000 U2  flash ROM
 04000000 - 05000000 U15 flash ROM
 05000000 - 06000000 U17 flash ROM
 06000000 - 07000000 U14 flash ROM
 07000000 - 08000000 U16 flash ROM

 Development:

 00000000 - 00800000 IC12 \
        .....               flash ROMs
 07800000 - 08000000 IC27 /


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

const device_type AW_ROM_BOARD = &device_creator<aw_rom_board>;

DEVICE_ADDRESS_MAP_START(submap, 16, aw_rom_board)
	AM_RANGE(0x00, 0x01) AM_WRITE(epr_offsetl_w)
	AM_RANGE(0x02, 0x03) AM_WRITE(epr_offseth_w)
	AM_RANGE(0x06, 0x07) AM_WRITE(mpr_record_index_w)
	AM_RANGE(0x08, 0x09) AM_WRITE(mpr_first_file_index_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(mpr_file_offsetl_w)
	AM_RANGE(0x0c, 0x0d) AM_WRITE(mpr_file_offseth_w)
	AM_RANGE(0x40, 0x41) AM_READWRITE(pio_r, pio_w)
ADDRESS_MAP_END

aw_rom_board::aw_rom_board(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: naomi_g1_device(mconfig, AW_ROM_BOARD, "Sammy Atomiswave ROM Board", tag, owner, clock, "aw_rom_board", __FILE__)
{
	keyregion = nullptr;
}

void aw_rom_board::static_set_keyregion(device_t &device, const char *_keyregion)
{
	aw_rom_board &dev = downcast<aw_rom_board &>(device);
	dev.keyregion = _keyregion;
}


/*
We are using 20 bits keys with the following subfields' structure:
bits 0-15 is a 16-bits XOR
bits 17-16 is a index to the sboxes table
bits 19-18 is a index to the permutation table

These subfields could be differing from the "real" ones in the following ways:
- Every one of the index subfields could be suffering an arbitrary bitswap and XOR
- The 16-bits-XOR subfield could suffer an arbitrary XOR which could depend on the 4 index bits (that is: a different XOR per every index combination)
- Of course, the way in which we are mixing 3 subfields in one only key is arbitrary too.

The keys are stored as 32-bits big-endian values in a file.
*/


const int aw_rom_board::permutation_table[4][16] =
{
	{8,10,1,3,7,4,11,2,5,15,6,0,12,13,9,14},
	{4,5,9,6,1,13,7,11,10,0,14,12,8,15,2,3},
	{12,7,11,2,0,5,15,6,1,8,14,4,9,13,3,10},
	{14,1,11,15,7,3,8,13,0,4,2,12,6,10,5,9}
};

const aw_rom_board::sbox_set aw_rom_board::sboxes_table[4] =
{
	{
		{4,12,8,14,16,30,31,0,23,29,24,21,11,22,27,5,3,20,18,26,10,7,17,1,28,6,15,13,2,9,25,19},
		{13,1,0,9,5,12,4,14,3,15,2,10,11,6,8,7},
		{7,13,4,6,5,9,3,2,0,15,12,10,8,11,1,14},
		{4,0,1,2,5,7,3,6}
	},
	{
		{3,0,14,17,10,15,31,20,13,2,29,28,9,18,25,27,6,19,30,22,7,12,1,16,23,11,24,4,8,26,21,5},
		{2,10,6,9,11,13,4,5,3,15,7,14,12,1,0,8},
		{1,13,11,3,8,7,9,10,12,15,4,14,0,5,6,2},
		{6,5,0,3,7,1,4,2}
	},
	{
		{9,15,28,7,13,24,2,23,21,1,22,16,18,8,17,31,27,6,30,12,4,20,5,19,0,25,3,29,10,14,11,26},
		{5,2,13,11,8,6,12,1,4,3,0,10,14,15,7,9},
		{11,6,10,0,12,1,8,14,2,9,13,3,7,4,15,5},
		{1,5,6,2,4,7,3,0}
	},
	{
		{17,3,31,2,28,10,9,29,6,25,24,8,13,1,19,15,22,0,14,20,16,7,21,4,18,26,27,5,12,23,11,30},
		{4,8,11,15,3,14,7,12,1,0,9,5,6,13,2,10},
		{14,0,9,11,4,1,7,5,13,6,8,12,2,3,10,15},
		{2,1,0,5,4,6,7,3}
	},
};

UINT16 aw_rom_board::decrypt(UINT16 cipherText, UINT32 address, const UINT32 key)
{
	UINT8 b0,b1,b2,b3;
	UINT16 aux;
	const int* pbox = permutation_table[key>>18];
	const sbox_set* ss = &sboxes_table[(key>>16)&3];

	aux = BITSWAP16(cipherText,
					pbox[15],pbox[14],pbox[13],pbox[12],pbox[11],pbox[10],pbox[9],pbox[8],
					pbox[7],pbox[6],pbox[5],pbox[4],pbox[3],pbox[2],pbox[1],pbox[0]);
	aux = aux ^ BITSWAP16(address, 13,5,2, 14,10,9,4, 15,11,6,1, 12,8,7,3,0);

	b0 = aux&0x1f;
	b1 = (aux>>5)&0xf;
	b2 = (aux>>9)&0xf;
	b3 = aux>>13;

	b0 = ss->S0[b0];
	b1 = ss->S1[b1];
	b2 = ss->S2[b2];
	b3 = ss->S3[b3];

	return ((b3<<13)|(b2<<9)|(b1<<5)|b0)^(key&0xffff);
}

void aw_rom_board::set_key()
{
	if(!m_region)
		throw emu_fatalerror("AW-ROM-BOARD: region %s is missing\n", tag().c_str());

	if(!keyregion)
		return;

	memory_region *kr = memregion(keyregion);
	if(!kr)
		return;

	if(kr->bytes() != 4)
		throw emu_fatalerror("AW-ROM-BOARD: key region %s has incorrect size (%d, expected 4)\n", keyregion, kr->bytes());

	const UINT8 *krp = kr->base();
	rombd_key = (krp[0] << 24) | (krp[1] << 16) | (krp[2] << 8) | krp[3];
}

void aw_rom_board::device_start()
{
	naomi_g1_device::device_start();
	set_key();

	mpr_offset = decrypt16(0x58/2) | (decrypt16(0x5a/2) << 16);

	save_item(NAME(epr_offset));
	save_item(NAME(mpr_record_index));
	save_item(NAME(mpr_first_file_index));
	save_item(NAME(mpr_file_offset));
	save_item(NAME(dma_offset));
	save_item(NAME(dma_limit));
	save_item(NAME(mpr_bank));
}

void aw_rom_board::device_reset()
{
	naomi_g1_device::device_reset();
	epr_offset = 0;
	mpr_record_index = 0;
	mpr_first_file_index = 0;
	mpr_file_offset = 0;
	mpr_bank = 0;

	dma_offset = 0;
	dma_limit  = 0;
}

READ16_MEMBER(aw_rom_board::pio_r)
{
	UINT32 roffset = epr_offset & 0x3ffffff;
	if (roffset >= (mpr_offset / 2))
		roffset += mpr_bank * 0x4000000;
	UINT16 retval = (m_region->bytes() > roffset) ? m_region->u16(roffset) : 0;
	epr_offset++;
	return retval;
}

WRITE16_MEMBER(aw_rom_board::pio_w)
{
	// write to ROM board address space, including FlashROM programming using CFI (TODO)
	if (epr_offset == 0x7fffff)
		mpr_bank = data & 3;
	epr_offset++;
}

WRITE16_MEMBER(aw_rom_board::epr_offsetl_w)
{
	epr_offset = (epr_offset & 0xffff0000) | data;
	recalc_dma_offset(EPR);
}

WRITE16_MEMBER(aw_rom_board::epr_offseth_w)
{
	epr_offset = (epr_offset & 0x0000ffff) | (data << 16);
	recalc_dma_offset(EPR);
}

WRITE16_MEMBER(aw_rom_board::mpr_record_index_w)
{
	mpr_record_index = data;
	recalc_dma_offset(MPR_RECORD);
}

WRITE16_MEMBER(aw_rom_board::mpr_first_file_index_w)
{
	mpr_first_file_index = data;
	recalc_dma_offset(MPR_FILE);
}

WRITE16_MEMBER(aw_rom_board::mpr_file_offsetl_w)
{
	mpr_file_offset = (mpr_file_offset & 0xffff0000) | data;
	recalc_dma_offset(MPR_FILE);
}

WRITE16_MEMBER(aw_rom_board::mpr_file_offseth_w)
{
	mpr_file_offset = (mpr_file_offset & 0x0000ffff) | (data << 16);

	recalc_dma_offset(MPR_FILE);
}

void aw_rom_board::recalc_dma_offset(int mode)
{
	switch(mode) {
	case EPR:
		dma_offset = epr_offset * 2;
		dma_limit  = mpr_offset;
		break;

	case MPR_RECORD:
		dma_offset = mpr_offset + mpr_record_index * 0x40;
		dma_limit = std::min((UINT32)0x8000000, m_region->bytes());
		break;

	case MPR_FILE: {
		UINT32 filedata_offs = (mpr_bank * 0x8000000 + mpr_offset + mpr_first_file_index * 0x40 + 8) / 2;
		dma_offset = decrypt16(filedata_offs) | (decrypt16(filedata_offs + 1) << 16);
		dma_offset = (mpr_offset + dma_offset + mpr_file_offset * 2) & 0x7ffffff;
		dma_limit  = std::min((UINT32)0x8000000, m_region->bytes());
		break;
	}
	}

	if (dma_offset >= mpr_offset) {
		UINT32 bank_base = mpr_bank * 0x8000000;
		dma_offset += bank_base;
		dma_limit = std::min(dma_limit + bank_base, m_region->bytes());
	}
}

void aw_rom_board::dma_get_position(UINT8 *&base, UINT32 &limit, bool to_mainram)
{
	if(!to_mainram) {
		limit = 0;
		base = nullptr;
		return;
	}

	UINT32 offset = dma_offset / 2;
	for (int i = 0; i < 16; i++)
		decrypted_buf[i] = decrypt16(offset + i);
	base = (UINT8*)decrypted_buf;
	limit = 32;
}

void aw_rom_board::dma_advance(UINT32 size)
{
	dma_offset += size;
}
