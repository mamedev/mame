/*********************************************************************

    formats/cbm_crt.c

    Commodore VIC-20/C64 cartridge images

*********************************************************************/

#include "formats/cbm_crt.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0


// slot names for the C64 cartridge types
static const char * CRT_C64_SLOT_NAMES[_CRT_C64_COUNT] =
{
	"standard",
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	"simons_basic",
	"ocean",
	UNSUPPORTED,
	"fun_play",
	"super_games",
	UNSUPPORTED,
	"epyxfastload",
	"westermann",
	"rex",
	UNSUPPORTED,
	"magic_formel",
	"system3",
	"warp_speed",
	"dinamic",
	"zaxxon",
	"magic_desk",
	UNSUPPORTED,
	"comal80",
	"struct_basic",
	"ross",
	"ep64",
	"ep7x8",
	"dela_ep256",
	"rex_ep256",
	"mikroasm",
	UNSUPPORTED,
	UNSUPPORTED,
	"stardos",
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	"ieee488",
	UNSUPPORTED,
	UNSUPPORTED,
	"exos",
	UNSUPPORTED,
	UNSUPPORTED,
	UNSUPPORTED,
	"super_explode",
	UNSUPPORTED,
	UNSUPPORTED,
	"mach5",
	UNSUPPORTED,
	"pagefox",
	UNSUPPORTED,
	"silverrock"
};



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  cbm_crt_get_card - get slot interface card
//-------------------------------------------------

const char * cbm_crt_get_card(core_file *file)
{
	// read the header
	cbm_crt_header header;
	core_fread(file, &header, CRT_HEADER_LENGTH);

	if (memcmp(header.signature, CRT_SIGNATURE, 16) == 0)
	{
		UINT16 hardware = pick_integer_be(header.hardware, 0, 2);

		return CRT_C64_SLOT_NAMES[hardware];
	}

	return NULL;
}


//-------------------------------------------------
//  cbm_crt_read_header - read cartridge header
//-------------------------------------------------

bool cbm_crt_read_header(core_file* file, size_t *roml_size, size_t *romh_size, int *exrom, int *game)
{
	// read the header
	cbm_crt_header header;
	core_fread(file, &header, CRT_HEADER_LENGTH);

	if (memcmp(header.signature, CRT_SIGNATURE, 16) != 0)
		return false;

	UINT16 hardware = pick_integer_be(header.hardware, 0, 2);
	*exrom = header.exrom;
	*game = header.game;

	if (LOG)
	{
		logerror("Name: %s\n", header.name);
		logerror("Hardware: %04x\n", hardware);
		logerror("Slot device: %s\n", CRT_C64_SLOT_NAMES[hardware]);
		logerror("EXROM: %u\n", header.exrom);
		logerror("GAME: %u\n", header.game);
	}

	// determine ROM region lengths
	while (!core_feof(file))
	{
		cbm_crt_chip chip;
		core_fread(file, &chip, CRT_CHIP_LENGTH);

		UINT16 address = pick_integer_be(chip.start_address, 0, 2);
		UINT16 size = pick_integer_be(chip.image_size, 0, 2);
		UINT16 type = pick_integer_be(chip.chip_type, 0, 2);

		if (LOG)
		{
			logerror("CHIP Address: %04x\n", address);
			logerror("CHIP Size: %04x\n", size);
			logerror("CHIP Type: %04x\n", type);
		}

		switch (address)
		{
		case 0x8000: *roml_size += size; break;
		case 0xa000: *romh_size += size; break;
		case 0xe000: *romh_size += size; break;
		default: logerror("Invalid CHIP loading address!\n"); break;
		}

		core_fseek(file, size, SEEK_CUR);
	}

	return true;
}


//-------------------------------------------------
//  cbm_crt_read_data - read cartridge data
//-------------------------------------------------

bool cbm_crt_read_data(core_file* file, UINT8 *roml, UINT8 *romh)
{
	offs_t roml_offset = 0;
	offs_t romh_offset = 0;

	core_fseek(file, CRT_HEADER_LENGTH, SEEK_SET);

	while (!core_feof(file))
	{
		cbm_crt_chip chip;
		core_fread(file, &chip, CRT_CHIP_LENGTH);

		UINT16 address = pick_integer_be(chip.start_address, 0, 2);
		UINT16 size = pick_integer_be(chip.image_size, 0, 2);

		switch (address)
		{
		case 0x8000: core_fread(file, roml + roml_offset, size); roml_offset += size; break;
		case 0xa000: core_fread(file, romh + romh_offset, size); romh_offset += size; break;
		case 0xe000: core_fread(file, romh + romh_offset, size); romh_offset += size; break;
		}
	}

	return true;
}
