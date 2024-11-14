// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy cartridge header constants

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_CARTHEADER_H
#define MAME_BUS_GAMEBOY_CARTHEADER_H

#pragma once

#include <iterator>


namespace bus::gameboy::cartheader {

enum : unsigned
{
	OFFSET_ENTRYPOINT           = 0x0100,   // 4-byte instructions
	OFFSET_LOGO                 = 0x0104,   // 48-byte bitmap
	OFFSET_TITLE                = 0x0134,   // originally 16-byte ASCII zero-padded, reduced to 11-byte
	OFFSET_MANUFACTURER         = 0x013f,   // 4-byte ASCII
	OFFSET_CGB                  = 0x0143,   // 1-byte - 0x80 = CGB enhanced, 0xc0 = CGB required
	OFFSET_LICENSEE_EXT         = 0x0144,   // 2-byte ASCII
	OFFSET_SGB                  = 0x0146,   // 1-byte - 0x00 = no SGB support, 0x03 = SGB enhanced
	OFFSET_TYPE                 = 0x0147,   // 1-byte - see values below
	OFFSET_ROM_SIZE             = 0x0148,   // 1-byte - see values below
	OFFSET_RAM_SIZE             = 0x0149,   // 1-byte - see values below
	OFFSET_REGION               = 0x014a,   // 1-byte - 0x00 = Japan, 0x01 = export
	OFFSET_LICENSEE             = 0x014b,   // 1-byte, 0x33 = ext
	OFFSET_REVISION             = 0x014c,   // 1-byte
	OFFSET_CHECKSUM_HEADER      = 0x014d,   // 1-byte - checksum of bytes 0x0134 to 0x014c
	OFFSET_CHECKSUM_ROM         = 0x014e,   // 2-byte - big-Endian checksum of ROM excluding checksum bytes
};


enum : u8
{
	TYPE_ROM                    = 0x00,
	TYPE_MBC1                   = 0x01,
	TYPE_MBC1_RAM               = 0x02,
	TYPE_MBC1_RAM_BATT          = 0x03,

	TYPE_MBC2                   = 0x05,
	TYPE_MBC2_BATT              = 0x06,

	TYPE_ROM_RAM                = 0x08,
	TYPE_ROM_RAM_BATT           = 0x09,

	TYPE_MMM01                  = 0x0b,
	TYPE_MMM01_RAM              = 0x0c,
	TYPE_MMM01_RAM_BATT         = 0x0d,

	TYPE_MBC3_RTC_BATT          = 0x0f,
	TYPE_MBC3_RTC_RAM_BATT      = 0x10,
	TYPE_MBC3                   = 0x11,
	TYPE_MBC3_RAM               = 0x12,
	TYPE_MBC3_RAM_BATT          = 0x13,

	TYPE_MBC5                   = 0x19,
	TYPE_MBC5_RAM               = 0x1a,
	TYPE_MBC5_RAM_BATT          = 0x1b,
	TYPE_MBC5_RUMBLE            = 0x1c,
	TYPE_MBC5_RUMBLE_RAM        = 0x1d,
	TYPE_MBC5_RUMBLE_RAM_BATT   = 0x1e,

	TYPE_MBC6                   = 0x20,

	TYPE_MBC7_ACCEL_EEPROM      = 0x22,

	TYPE_UNLICENSED_YONGYONG    = 0xea,

	TYPE_CAMERA                 = 0xfc,
	TYPE_TAMA5                  = 0xfd,
	TYPE_HUC3                   = 0xfe,
	TYPE_HUC1_RAM_BATT          = 0xff
};


enum : u8
{
	ROM_SIZE_32K                = 0x00,
	ROM_SIZE_64K                = 0x01,
	ROM_SIZE_128K               = 0x02,
	ROM_SIZE_256K               = 0x03,
	ROM_SIZE_512K               = 0x04,
	ROM_SIZE_1024K              = 0x05,
	ROM_SIZE_2048K              = 0x06,
	ROM_SIZE_4096K              = 0x07,
	ROM_SIZE_8192K              = 0x08,

	ROM_SIZE_1152K              = 0x52,
	ROM_SIZE_1280K              = 0x53,
	ROM_SIZE_1536K              = 0x54
};


enum : u8
{
	RAM_SIZE_0K                 = 0x00,
	RAM_SIZE_2K                 = 0x01,
	RAM_SIZE_8K                 = 0x02,
	RAM_SIZE_32K                = 0x03,
	RAM_SIZE_128K               = 0x04,
	RAM_SIZE_64K                = 0x05
};


template <typename It>
inline u8 checksum(It begin, It end)
{
	u8 result(0U);
	while (begin != end)
		result += u8(~*begin++ & 0xff);
	return result;
}


template <typename It>
inline bool verify_header_checksum(It begin)
{
	std::advance(begin, OFFSET_TITLE - 0x100);
	u8 checksum(0U);
	for (unsigned i = 0U; (OFFSET_CHECKSUM_HEADER - OFFSET_TITLE) > i; ++i, ++begin)
		checksum += u8(~*begin & 0xff);
	return *begin == checksum;
}

} // namespace bus::gameboy::cartheader

#endif // MAME_BUS_GAMEBOY_CARTHEADER_H
