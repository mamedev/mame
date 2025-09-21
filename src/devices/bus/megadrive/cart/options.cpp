// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "options.h"

#include "avartisan.h"
#include "gamtec.h"
#include "mcpirate.h"
#include "multigame.h"
#include "rom.h"
#include "seganet.h"
#include "sram.h"
#include "smb.h"
#include "xboy.h"


namespace bus::megadrive::slotoptions {

char const *const MD_STD           = "rom";
char const *const MD_SSF2          = "rom_ssf2";
char const *const MD_SRAM          = "rom_sram";
char const *const MD_SONIC3        = "rom_fram"; // TODO: change string
char const *const MD_TPLAY96       = "rom_tplay96";
char const *const MD_HARDBALL95    = "rom_hardbl95";
char const *const MD_CM2IN1        = "rom_cm2in1";
char const *const MD_SEGANET       = "ram_seganet";
char const *const MD_TECTOY_SPORTS = "tectoy_sports";
char const *const MC_PIRATE        = "rom_mcpir"; // TODO: rename, what even MC stands for?

char const *const UNL_XINQIG       = "rom_xinqig";
char const *const UNL_TILESMJ2     = "rom_16mj2";
char const *const UNL_ELFWOR       = "rom_elfwor";
char const *const UNL_SMOUSE       = "rom_smouse";
char const *const UNL_YASECH       = "rom_yasech";
char const *const UNL_777CASINO    = "rom_777casino";
char const *const UNL_SOULBLADE    = "rom_soulb";
char const *const UNL_SUPRBUBL     = "rom_sbubl";
char const *const UNL_CJMJCLUB     = "rom_cjmjclub";
char const *const UNL_MJLOV        = "rom_mjlov";
char const *const UNL_REDCLIFF     = "rom_redcl";
char const *const UNL_SQUIRRELK    = "rom_squir";
char const *const UNL_LIONKING2    = "rom_lion2";

char const *const UNL_KOF98        = "rom_kof98";
char const *const UNL_BUGSLIFE     = "rom_bugs";
char const *const UNL_POKEMONA     = "rom_pokea";
char const *const UNL_KOF99        = "rom_kof99";

char const *const UNL_SMB          = "rom_smb";
char const *const UNL_SMB2         = "rom_smb2";
char const *const UNL_ROCKMANX3    = "rom_rx3";

char const *const UNL_SANGUO5      = "rom_sanguo5";

char const *const UNL_AVARTISAN    = "rom_realtec";


} // namespace bus::megadrive::slotoptions


void megadrive_cart_options(device_slot_interface &device)
{
	using namespace bus::megadrive;

	// normal
	device.option_add_internal(slotoptions::MD_STD, MEGADRIVE_ROM);
	device.option_add_internal(slotoptions::MD_SSF2, MEGADRIVE_ROM_SSF2);

	// SRAM
	device.option_add_internal(slotoptions::MD_SRAM, MEGADRIVE_ROM_SRAM);
	device.option_add_internal(slotoptions::MD_SONIC3, MEGADRIVE_ROM_SONIC3);
	device.option_add_internal(slotoptions::MD_TPLAY96, MEGADRIVE_ROM_TPLAY96);
	device.option_add_internal(slotoptions::MD_HARDBALL95, MEGADRIVE_ROM_HARDBALL95);
	device.option_add_internal(slotoptions::UNL_XINQIG, MEGADRIVE_UNL_XINQIG);
	device.option_add_internal(slotoptions::UNL_SANGUO5, MEGADRIVE_UNL_SANGUO5);

	// reset based multigames
	device.option_add_internal(slotoptions::MD_CM2IN1, MEGADRIVE_CM2IN1);
	device.option_add_internal(slotoptions::MD_TECTOY_SPORTS, MEGADRIVE_TECTOY_SPORTS);

	// menu based multigames
	device.option_add_internal(slotoptions::MD_SEGANET, MEGADRIVE_SEGANET);
	device.option_add_internal(slotoptions::MC_PIRATE, MEGADRIVE_MCPIRATE);

	// unlicensed
	// Gamtec
	device.option_add_internal(slotoptions::UNL_TILESMJ2, MEGADRIVE_UNL_TILESMJ2);
	device.option_add_internal(slotoptions::UNL_ELFWOR, MEGADRIVE_UNL_ELFWOR);
	device.option_add_internal(slotoptions::UNL_SMOUSE, MEGADRIVE_UNL_SMOUSE);
	device.option_add_internal(slotoptions::UNL_YASECH, MEGADRIVE_UNL_YASECH);
	device.option_add_internal(slotoptions::UNL_777CASINO, MEGADRIVE_UNL_777CASINO);
	device.option_add_internal(slotoptions::UNL_SOULBLADE, MEGADRIVE_UNL_SOULBLADE);
	device.option_add_internal(slotoptions::UNL_SUPRBUBL, MEGADRIVE_UNL_SUPRBUBL);
	device.option_add_internal(slotoptions::UNL_CJMJCLUB, MEGADRIVE_UNL_CJMJCLUB);
	device.option_add_internal(slotoptions::UNL_MJLOV, MEGADRIVE_UNL_MJLOV);
	device.option_add_internal(slotoptions::UNL_REDCLIFF, MEGADRIVE_UNL_REDCLIFF);
	device.option_add_internal(slotoptions::UNL_SQUIRRELK, MEGADRIVE_UNL_SQUIRRELK);
	device.option_add_internal(slotoptions::UNL_LIONKING2, MEGADRIVE_UNL_LIONKING2);

	// X Boy
	device.option_add_internal(slotoptions::UNL_KOF98, MEGADRIVE_UNL_KOF98);
	device.option_add_internal(slotoptions::UNL_BUGSLIFE, MEGADRIVE_UNL_BUGSLIFE);
	device.option_add_internal(slotoptions::UNL_POKEMONA, MEGADRIVE_UNL_POKEMONA);
	device.option_add_internal(slotoptions::UNL_KOF99, MEGADRIVE_UNL_KOF99);

	// Super Mario Bros
	device.option_add_internal(slotoptions::UNL_SMB, MEGADRIVE_UNL_SMB);
	device.option_add_internal(slotoptions::UNL_SMB2, MEGADRIVE_UNL_SMB2);
	device.option_add_internal(slotoptions::UNL_ROCKMANX3, MEGADRIVE_UNL_ROCKMANX3);

	// AV Artisan
	device.option_add_internal(slotoptions::UNL_AVARTISAN, MEGADRIVE_UNL_AVARTISAN);

}
