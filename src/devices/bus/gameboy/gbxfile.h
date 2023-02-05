// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 GBX file format helpers

 GBX files start with the program ROM contents, and end with a footer.
 Multi-byte values in the footer are stored in big-Endian order.  Fields are
 arranged for natural alignment.  Version 1.0 seems to be the only
 widespread version.

 The footer ends with a 16-byte trailer:
 * Offset from end of file to start of footer (4 bytes).
 * Major version of footer format (4 bytes).
 * Minor version of footer format (4 bytes).
 * Magic number 0x47425821 or FourCC 'GBX!' (4 bytes).

 The footer starts with a leader.  Version 1.0 uses a 16-byte leader:
 * A FourCC indicating the cartridge type (4 bytes).
 * Flag indicating whether cartridge RAM contents is preserved (1 byte).
 * Flag indicating whether a vibration motor is present (1 byte).
 * Flag indicating whether a real-time clock oscillator is present (1 byte).
 * Always seems to be zero - possibly unused padding (1 byte).
 * Program ROM size in bytes (4 bytes).
 * Cartridge RAM size in bytes (4 bytes).

 There is additional data between the leader and trailer that can hold
 additional wiring or configuration details for the cartridge if necessary.
 In general, it's 32 bytes long, and unused portions are filled with 0x00.
 It seems to be entirely unused for most cartridge types.

 Vast Fame VF001 (FourCC 'VF01') additional data:
 * Command preload value (1 byte).

 Kong Feng DSH-GGB81 (FourCC 'GB81') additional data:
 * Cartridge/PCB type (1 byte):
  - 0x00: DSH-GGB81
  - 0x01: BC-R1616T3P

 The list of cartridge type FourCC values here is incomplete.

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_GBXFILE_H
#define MAME_BUS_GAMEBOY_GBXFILE_H

#pragma once


namespace bus::gameboy::gbxfile {

enum : u32
{
	MAGIC_GBX       = 0x47425821    // 'GBX!'
};

enum : u32
{
	TYPE_BBD        = 0x42424400,   // 'BBD\0'
	TYPE_CAMERA     = 0x43414d52,   // 'CAMR'
	TYPE_DSHGGB81   = 0x47423831,   // 'GB81'
	TYPE_HUC1       = 0x48554331,   // 'HUC1'
	TYPE_HUC3       = 0x48554333,   // 'HUC3'
	TYPE_SLMULTI    = 0x4c424d43,   // 'LBMC'
	TYPE_LICHENG    = 0x4c494348,   // 'LICH'
	TYPE_LIEBAO     = 0x4c494241,   // 'LIBA'
	TYPE_M161       = 0x4d313631,   // 'M161'
	TYPE_MBC1_COLL  = 0x4d42314d,   // 'MB1M'
	TYPE_MBC1       = 0x4d424331,   // 'MBC1'
	TYPE_MBC2       = 0x4d424332,   // 'MBC2'
	TYPE_MBC3       = 0x4d424333,   // 'MBC3'
	TYPE_MBC5       = 0x4d424335,   // 'MBC5'
	TYPE_MBC6       = 0x4d424336,   // 'MBC6'
	TYPE_MBC7       = 0x4d424337,   // 'MBC7'
	TYPE_MMM01      = 0x4d4d4d31,   // 'MMM1'
	TYPE_NEWGBCHK   = 0x4e47484b,   // 'NGHK'
	TYPE_NTNEW      = 0x4e544e00,   // 'NTN\0'
	TYPE_TFANGBOOT  = 0x504b4a44,   // 'PKJD'
	TYPE_ROCKET     = 0x524f434b,   // 'ROCK'
	TYPE_PLAIN      = 0x524f4d00,   // 'ROM\0'
	TYPE_SACHEN1    = 0x53414d31,   // 'SAM1'
	TYPE_SACHEN2    = 0x53414d32,   // 'SAM2'
	TYPE_SINTAX     = 0x534e5458,   // 'SNTX'
	TYPE_TAMA5      = 0x54414d35,   // 'TAM5'
	TYPE_VF001      = 0x56463031,   // 'VF01'
	TYPE_WISDOM     = 0x57495344    // 'WISD'
};

struct leader_1_0
{
	u32 cart_type;
	u8 batt;
	u8 rumble;
	u8 rtc;
	u8 unknown; // maybe padding?
	u32 rom_bytes;
	u32 ram_bytes;

	void swap()
	{
		cart_type = big_endianize_int32(cart_type);
		rom_bytes = big_endianize_int32(rom_bytes);
		ram_bytes = big_endianize_int32(ram_bytes);
	}
};

struct trailer
{
	u32 size;
	u32 ver_maj;
	u32 ver_min;
	u32 magic;

	void swap()
	{
		size = big_endianize_int32(size);
		ver_maj = big_endianize_int32(ver_maj);
		ver_min = big_endianize_int32(ver_min);
		magic = big_endianize_int32(magic);
	}
};

bool get_data(
		memory_region *region,
		leader_1_0 &leader,
		u8 const *&extra,
		u32 &extralen);

} // namespace bus::gameboy::gbxfile

#endif // MAME_BUS_GAMEBOY_GBXFILE_H
