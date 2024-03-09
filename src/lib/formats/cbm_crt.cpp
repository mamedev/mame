// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    cbm_crt.c

    Commodore C64 cartridge images in .CRT format

    This format was introduced in the CCS64 emulator by Per Hakan
    Sundell.

    Header info based on the VICE manual chapter 15.11, which in turn
    is based on CRT.txt rev 1.14 compiled by Peter Schepers, with
    additional contributions by Per Hakan Sundell, Markus Brenner,
    and Marco Van Den Heuvel.
    Relevant links:
        http://vice-emu.sourceforge.net/vice_15.html#SEC300
        http://ist.uwaterloo.ca/~schepers/formats/CRT.TXT (version 1.13, outdated)

    Header Contents (bytes $0000-003F):
    Bytes $0000-000F - 16-byte cartridge signature "C64 CARTRIDGE" (padded with spaces)
          $0010-0013 - File header length
          $0014-0015 - Cartridge version (high/low, presently 01.00)
          $0016-0017 - Cartridge hardware type ($0000, high/low), see below
          $0018      - Cartridge port EXROM line status (0 = inactive, 1 = active)
          $0019      - Cartridge port GAME line status (0 = inactive, 1 = active)
          $001A-001F - Reserved for future use
          $0020-003F - 32-byte cartridge name (uppercase, padded with null characters)

    CHIP Packet Contents (starting from $0040; there can be multiple CHIP packets
    in a single CRT file):
    Bytes $0040-0043 - Contained ROM signature "CHIP"
          $0044-0047 - Total packet length (ROM image size and header combined) (high/low format)
          $0048-0049 - Chip type (0 = ROM, 1 = RAM (no ROM data), 2 = Flash ROM)
          $004A-004B - Bank number
          $004C-004D - Starting load address (high/low format)
          $004E-004F - ROM image size in bytes (high/low format, typically $2000 or $4000)
          $0050-xxxx - ROM data

*********************************************************************/

#include "cbm_crt.h"

#include "corefile.h"
#include "multibyte.h"

#include "osdcore.h" // osd_printf_*

#include <tuple>


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0


#define CRT_SIGNATURE       "C64 CARTRIDGE   "

#define CRT_HEADER_LENGTH   0x40
#define CRT_CHIP_LENGTH     0x10

#define UNSUPPORTED         "standard"


// slot names for the C64 cartridge types
static char const *const CRT_C64_SLOT_NAMES[_CRT_C64_COUNT] =
{
	"standard",         //  0 - Normal cartridge
	UNSUPPORTED,        //  1 - Action Replay
	UNSUPPORTED,        //  2 - KCS Power Cartridge
	"final3",           //  3 - Final Cartridge III
	"simons_basic",     //  4 - Simons' BASIC
	"ocean",            //  5 - Ocean type 1
	UNSUPPORTED,        //  6 - Expert Cartridge
	"fun_play",         //  7 - Fun Play, Power Play
	"super_games",      //  8 - Super Games
	UNSUPPORTED,        //  9 - Atomic Power
	"epyxfastload",     // 10 - Epyx Fastload
	"westermann",       // 11 - Westermann Learning
	"rex",              // 12 - Rex Utility
	"final",            // 13 - Final Cartridge I
	"magic_formel",     // 14 - Magic Formel
	"system3",          // 15 - C64 Game System, System 3
	"warp_speed",       // 16 - Warp Speed
	"dinamic",          // 17 - Dinamic
	"zaxxon",           // 18 - Zaxxon, Super Zaxxon (SEGA)
	"magic_desk",       // 19 - Magic Desk, Domark, HES Australia
	UNSUPPORTED,        // 20 - Super Snapshot V5
	"comal80",          // 21 - Comal-80
	"struct_basic",     // 22 - Structured BASIC
	"ross",             // 23 - Ross
	"ep64",             // 24 - Dela EP64
	"ep7x8",            // 25 - Dela EP7x8
	"dela_ep256",       // 26 - Dela EP256
	"rex_ep256",        // 27 - Rex EP256
	"mikroasm",         // 28 - Mikro Assembler
	UNSUPPORTED,        // 29 - Final Cartridge Plus
	UNSUPPORTED,        // 30 - Action Replay 4
	"stardos",          // 31 - Stardos
	"easyflash",        // 32 - EasyFlash
	UNSUPPORTED,        // 33 - EasyFlash Xbank
	UNSUPPORTED,        // 34 - Capture
	UNSUPPORTED,        // 35 - Action Replay 3
	UNSUPPORTED,        // 36 - Retro Replay
	UNSUPPORTED,        // 37 - MMC64
	UNSUPPORTED,        // 38 - MMC Replay
	"ide64",            // 39 - IDE64
	UNSUPPORTED,        // 40 - Super Snapshot V4
	"ieee488",          // 41 - IEEE-488
	UNSUPPORTED,        // 42 - Game Killer
	"prophet64",        // 43 - Prophet64
	"exos",             // 44 - EXOS
	UNSUPPORTED,        // 45 - Freeze Frame
	UNSUPPORTED,        // 46 - Freeze Machine
	UNSUPPORTED,        // 47 - Snapshot64
	"super_explode",    // 48 - Super Explode V5.0
	"magic_voice",      // 49 - Magic Voice
	UNSUPPORTED,        // 50 - Action Replay 2
	"mach5",            // 51 - MACH 5
	UNSUPPORTED,        // 52 - Diashow-Maker
	"pagefox",          // 53 - Pagefox
	UNSUPPORTED,        // 54 - ?
	"silverrock"        // 55 - Silverrock
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  cbm_crt_get_card - get slot interface card
//-------------------------------------------------

std::string cbm_crt_get_card(util::core_file &file)
{
	// read the header
	cbm_crt_header header;
	auto const [err, actual] = read(file, &header, CRT_HEADER_LENGTH);

	if (!err && (CRT_HEADER_LENGTH == actual) && !memcmp(header.signature, CRT_SIGNATURE, 16))
	{
		uint16_t hardware = get_u16be(header.hardware);

		return std::string(CRT_C64_SLOT_NAMES[hardware]);
	}

	return std::string();
}


//-------------------------------------------------
//  cbm_crt_read_header - read cartridge header
//-------------------------------------------------

bool cbm_crt_read_header(util::core_file &file, size_t *roml_size, size_t *romh_size, int *exrom, int *game)
{
	std::error_condition err;
	size_t actual;

	// read the header
	cbm_crt_header header;
	std::tie(err, actual) = read(file, &header, CRT_HEADER_LENGTH);
	if (err)
		return false;

	if ((CRT_HEADER_LENGTH != actual) || (memcmp(header.signature, CRT_SIGNATURE, 16) != 0))
		return false;

	uint16_t hardware = get_u16be(header.hardware);
	*exrom = header.exrom;
	*game = header.game;

	if (LOG)
	{
		osd_printf_verbose("Name: %s\n", header.name);
		osd_printf_verbose("Hardware: %04x\n", hardware);
		osd_printf_verbose("Slot device: %s\n", CRT_C64_SLOT_NAMES[hardware]);
		osd_printf_verbose("EXROM: %u\n", header.exrom);
		osd_printf_verbose("GAME: %u\n", header.game);
	}

	// determine ROM region lengths
	while (!file.eof())
	{
		cbm_crt_chip chip;
		std::tie(err, actual) = read(file, &chip, CRT_CHIP_LENGTH);
		if (err || (CRT_CHIP_LENGTH != actual))
			return false;

		const uint16_t address = get_u16be(chip.start_address);
		const uint16_t size = get_u16be(chip.image_size);
		const uint16_t type = get_u16be(chip.chip_type);

		if (LOG)
		{
			osd_printf_verbose("CHIP Address: %04x\n", address);
			osd_printf_verbose("CHIP Size: %04x\n", size);
			osd_printf_verbose("CHIP Type: %04x\n", type);
		}

		switch (address)
		{
		case 0x8000: *roml_size += size; break;
		case 0xa000: *romh_size += size; break;
		case 0xe000: *romh_size += size; break;
		default: osd_printf_verbose("Invalid CHIP loading address!\n"); break;
		}

		if (file.seek(size, SEEK_CUR))
			return false;
	}

	return true;
}


//-------------------------------------------------
//  cbm_crt_read_data - read cartridge data
//-------------------------------------------------

bool cbm_crt_read_data(util::core_file &file, uint8_t *roml, uint8_t *romh)
{
	if (file.seek(CRT_HEADER_LENGTH, SEEK_SET))
		return false;

	uint32_t roml_offset = 0;
	uint32_t romh_offset = 0;

	while (!file.eof())
	{
		std::error_condition err;
		size_t actual;

		cbm_crt_chip chip;
		std::tie(err, actual) = read(file, &chip, CRT_CHIP_LENGTH);
		if (err || (CRT_CHIP_LENGTH != actual))
			return false;

		const uint16_t address = get_u16be(chip.start_address);
		const uint16_t size = get_u16be(chip.image_size);

		switch (address)
		{
		case 0x8000: std::tie(err, actual) = read(file, roml + roml_offset, size); roml_offset += size; break;
		case 0xa000: std::tie(err, actual) = read(file, romh + romh_offset, size); romh_offset += size; break;
		case 0xe000: std::tie(err, actual) = read(file, romh + romh_offset, size); romh_offset += size; break;
		// FIXME: surely one needs to report an error or skip over the data if the load address is not recognised?
		}
		if (err) // TODO: check size - all bets are off if the address isn't recognised anyway
			return false;
	}

	return true;
}
