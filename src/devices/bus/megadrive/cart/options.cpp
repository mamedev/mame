// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "options.h"

#include "action_replay.h"
#include "avartisan.h"
#include "eeprom.h"
#include "everdrive.h"
#include "gamtec.h"
#include "jcart.h"
#include "mcpirate.h"
#include "miky.h"
#include "multigame.h"
#include "rockworld.h"
#include "rom.h"
#include "segach.h"
#include "sfteam.h"
#include "sram.h"
#include "smb.h"
#include "smw64.h"
#include "ssf.h"
#include "t5740.h"
#include "tekkensp.h"
#include "xboy.h"


namespace bus::megadrive::slotoptions {

char const *const MD_STD                = "rom";
char const *const MD_SSF2               = "rom_ssf2";
char const *const HB_SSF                = "rom_titan"; // TODO: rename
char const *const HB_SSF_SRAM           = "ssf_sram";
char const *const HB_SSF_EX             = "ssf_ex";
char const *const HB_EVERDRIVE          = "everdrive";
char const *const MD_SRAM               = "rom_sram";
char const *const MD_SONIC3             = "rom_fram"; // TODO: change string
char const *const MD_TPLAY96            = "rom_tplay96";
char const *const MD_HARDBALL95         = "rom_hardbl95";
char const *const MD_BARKLEY2           = "barkley2";
char const *const MD_EEPROM             = "rom_eeprom_mode1";
char const *const MD_EEPROM_NBAJAM      = "rom_nbajam_alt";
char const *const MD_EEPROM_NBAJAMTE    = "rom_nbajamte";
char const *const MD_EEPROM_NFLQB96     = "rom_nflqb96";
char const *const MD_EEPROM_COLLSLAM    = "rom_cslam";
char const *const MD_EEPROM_NHLPA       = "rom_nhlpa";
char const *const MD_EEPROM_BLARA95     = "rom_blara95";
char const *const MD_EEPROM_BLARA96     = "rom_blara96";

char const *const MD_CM2IN1             = "rom_cm2in1";
char const *const MD_JCART_SAMPRAS      = "rom_jcart_sampras";
char const *const MD_JCART_SSKID        = "rom_jcart";
char const *const MD_JCART_MICROMAC2    = "rom_codemast";
char const *const MD_JCART_MICROMAC96   = "rom_mm96";
char const *const MD_SEGACH_JP          = "segach_jp";
char const *const MD_SEGACH_US          = "segach_us";
char const *const MD_TECTOY_SPORTS      = "tectoy_sports";
char const *const MD_3IN1_FWT           = "3in1fwt";
// MC: MultiCart
char const *const MC_PIRATE             = "rom_mcpir";
char const *const MC_18KIN1             = "18kin1";
char const *const MC_GOLDM250           = "goldm250";
char const *const UNL_XINQIG            = "rom_xinqig";
char const *const HB_BEGGARP            = "rom_sf001";
char const *const HB_BEGGARP1           = "rom_sf001_beggarp1";
char const *const HB_WUKONG             = "rom_sf002";
char const *const HB_STARODYS           = "rom_sf004";
char const *const UNL_TILESMJ2          = "rom_16mj2";
char const *const UNL_ELFWOR            = "rom_elfwor";
char const *const UNL_SMOUSE            = "rom_smouse";
char const *const UNL_YASECH            = "rom_yasech";
char const *const UNL_777CASINO         = "rom_777casino";
char const *const UNL_SOULBLADE         = "rom_soulb";
char const *const UNL_SUPRBUBL          = "rom_sbubl";
char const *const UNL_CJMJCLUB          = "rom_cjmjclub";
char const *const UNL_MJLOV             = "rom_mjlov";
char const *const UNL_REDCLIFF          = "rom_redcl";
char const *const UNL_SQUIRRELK         = "rom_squir";
char const *const UNL_LIONKING2         = "rom_lion2";
char const *const UNL_KOF98             = "rom_kof98";
char const *const UNL_BUGSLIFE          = "rom_bugs";
char const *const UNL_POKEMONA          = "rom_pokea";
char const *const UNL_KOF99             = "rom_kof99";
char const *const UNL_SMB               = "rom_smb";
char const *const UNL_SMB2              = "rom_smb2";
char const *const UNL_ROCKMANX3         = "rom_rx3";
char const *const UNL_SANGUO5           = "rom_sanguo5";
char const *const UNL_AVARTISAN         = "rom_realtec";
char const *const UNL_TEKKENSP          = "rom_tekkensp";
char const *const UNL_TC2000            = "rom_tc2000";
char const *const UNL_FUTBOL_ARG96      = "rom_sram_arg96";
char const *const UNL_TOPF              = "rom_lion3";
char const *const UNL_POKESTAD          = "rom_pokestad"; // TODO: alias of above, probably unneeded
char const *const UNL_CHINF3            = "rom_chinf3";
char const *const UNL_SMW64             = "rom_smw64";
char const *const UNL_ROCKWORLD         = "rockworld";
char const *const UNL_ROCKHEAVEN        = "rockheaven";
char const *const HB_PSOLAR             = "rom_stm95"; // TODO: rename string

char const *const ACTION_REPLAY         = "ar";

} // namespace bus::megadrive::slotoptions


void megadrive_cart_options(device_slot_interface &device)
{
	using namespace bus::megadrive;

	// normal
	device.option_add_internal(slotoptions::MD_STD,  MEGADRIVE_ROM);
	device.option_add_internal(slotoptions::MD_SSF2, MEGADRIVE_ROM_SSF2);

	// SRAM
	device.option_add_internal(slotoptions::MD_SRAM,       MEGADRIVE_ROM_SRAM);
	device.option_add_internal(slotoptions::MD_SONIC3,     MEGADRIVE_ROM_SONIC3);
	device.option_add_internal(slotoptions::MD_TPLAY96,    MEGADRIVE_ROM_TPLAY96);
	device.option_add_internal(slotoptions::MD_HARDBALL95, MEGADRIVE_ROM_HARDBALL95);
	device.option_add_internal(slotoptions::MD_BARKLEY2,   MEGADRIVE_ROM_BARKLEY2);
	device.option_add_internal(slotoptions::UNL_SANGUO5,   MEGADRIVE_UNL_SANGUO5);

	// EEPROM
	device.option_add_internal(slotoptions::MD_EEPROM,          MEGADRIVE_EEPROM);
	device.option_add_internal(slotoptions::MD_EEPROM_NBAJAM,   MEGADRIVE_EEPROM_NBAJAM);
	device.option_add_internal(slotoptions::MD_EEPROM_NBAJAMTE, MEGADRIVE_EEPROM_NBAJAMTE);
	device.option_add_internal(slotoptions::MD_EEPROM_NFLQB96,  MEGADRIVE_EEPROM_NFLQB96);
	device.option_add_internal(slotoptions::MD_EEPROM_COLLSLAM, MEGADRIVE_EEPROM_COLLSLAM);
	device.option_add_internal(slotoptions::MD_EEPROM_NHLPA,    MEGADRIVE_EEPROM_NHLPA);
	device.option_add_internal(slotoptions::MD_EEPROM_BLARA95,  MEGADRIVE_EEPROM_BLARA95);
	device.option_add_internal(slotoptions::MD_EEPROM_BLARA96,  MEGADRIVE_EEPROM_BLARA96);

	// J-Cart
	device.option_add_internal(slotoptions::MD_JCART_SAMPRAS,    MEGADRIVE_ROM_JCART_SAMPRAS);
	device.option_add_internal(slotoptions::MD_JCART_SSKID,      MEGADRIVE_ROM_JCART_SSKID);
	device.option_add_internal(slotoptions::MD_JCART_MICROMAC2,  MEGADRIVE_ROM_JCART_MICROMAC2);
	device.option_add_internal(slotoptions::MD_JCART_MICROMAC96, MEGADRIVE_ROM_JCART_MICROMAC96);

	// reset based multigames
	device.option_add_internal(slotoptions::MD_CM2IN1,        MEGADRIVE_CM2IN1);
	device.option_add_internal(slotoptions::MD_TECTOY_SPORTS, MEGADRIVE_TECTOY_SPORTS);
	device.option_add_internal(slotoptions::MD_3IN1_FWT,      MEGADRIVE_3IN1FWT);

	// Sega Channel
	device.option_add_internal(slotoptions::MD_SEGACH_JP,   MEGADRIVE_SEGACH_JP);
	device.option_add_internal(slotoptions::MD_SEGACH_US,   MEGADRIVE_SEGACH_US);

	// menu based multigames (pirate)
	device.option_add_internal(slotoptions::MC_PIRATE,    MEGADRIVE_MCPIRATE);
	device.option_add_internal(slotoptions::MC_18KIN1,    MEGADRIVE_18KIN1);
	device.option_add_internal(slotoptions::MC_GOLDM250,  MEGADRIVE_GOLDM250);

	// unlicensed
	// Gamtec
	device.option_add_internal(slotoptions::UNL_TILESMJ2,  MEGADRIVE_UNL_TILESMJ2);
	device.option_add_internal(slotoptions::UNL_ELFWOR,    MEGADRIVE_UNL_ELFWOR);
	device.option_add_internal(slotoptions::UNL_SMOUSE,    MEGADRIVE_UNL_SMOUSE);
	device.option_add_internal(slotoptions::UNL_YASECH,    MEGADRIVE_UNL_YASECH);
	device.option_add_internal(slotoptions::UNL_777CASINO, MEGADRIVE_UNL_777CASINO);
	device.option_add_internal(slotoptions::UNL_SOULBLADE, MEGADRIVE_UNL_SOULBLADE);
	device.option_add_internal(slotoptions::UNL_SUPRBUBL,  MEGADRIVE_UNL_SUPRBUBL);
	device.option_add_internal(slotoptions::UNL_CJMJCLUB,  MEGADRIVE_UNL_CJMJCLUB);
	device.option_add_internal(slotoptions::UNL_MJLOV,     MEGADRIVE_UNL_MJLOV);
	device.option_add_internal(slotoptions::UNL_REDCLIFF,  MEGADRIVE_UNL_REDCLIFF);
	device.option_add_internal(slotoptions::UNL_SQUIRRELK, MEGADRIVE_UNL_SQUIRRELK);
	device.option_add_internal(slotoptions::UNL_LIONKING2, MEGADRIVE_UNL_LIONKING2);

	// X Boy
	device.option_add_internal(slotoptions::UNL_KOF98,    MEGADRIVE_UNL_KOF98);
	device.option_add_internal(slotoptions::UNL_BUGSLIFE, MEGADRIVE_UNL_BUGSLIFE);
	device.option_add_internal(slotoptions::UNL_POKEMONA, MEGADRIVE_UNL_POKEMONA);
	device.option_add_internal(slotoptions::UNL_KOF99,    MEGADRIVE_UNL_KOF99);
	device.option_add_internal(slotoptions::UNL_POKESTAD, MEGADRIVE_UNL_TOPF);
	device.option_add_internal(slotoptions::UNL_TOPF,     MEGADRIVE_UNL_TOPF);
	device.option_add_internal(slotoptions::UNL_CHINF3,   MEGADRIVE_UNL_CHINF3);

	// Super Mario Bros
	device.option_add_internal(slotoptions::UNL_SMB,       MEGADRIVE_UNL_SMB);
	device.option_add_internal(slotoptions::UNL_SMB2,      MEGADRIVE_UNL_SMB2);
	device.option_add_internal(slotoptions::UNL_ROCKMANX3, MEGADRIVE_UNL_ROCKMANX3);

	// Super Mario World 64
	device.option_add_internal(slotoptions::UNL_SMW64, MEGADRIVE_UNL_SMW64);

	// AV Artisan
	device.option_add_internal(slotoptions::UNL_AVARTISAN, MEGADRIVE_UNL_AVARTISAN);

	// <unknown> Taiwanese carts
	device.option_add_internal(slotoptions::UNL_TEKKENSP, MEGADRIVE_UNL_TEKKENSP);

	// Miky
	device.option_add_internal(slotoptions::UNL_TC2000,       MEGADRIVE_UNL_TC2000);
	device.option_add_internal(slotoptions::UNL_FUTBOL_ARG96, MEGADRIVE_UNL_FUTBOL_ARG96);

	// Rock Heaven / Rock World
	device.option_add_internal(slotoptions::UNL_ROCKHEAVEN, MEGADRIVE_UNL_ROCKHEAVEN);
	device.option_add_internal(slotoptions::UNL_ROCKWORLD,  MEGADRIVE_UNL_ROCKWORLD);

	// Action Replay
	device.option_add_internal(slotoptions::ACTION_REPLAY, MEGADRIVE_ACTION_REPLAY);

	// Homebrew
	// Super Fighter Team
	device.option_add_internal(slotoptions::UNL_XINQIG,    MEGADRIVE_UNL_XINQIG);
	device.option_add_internal(slotoptions::HB_BEGGARP,    MEGADRIVE_HB_BEGGARP);
	device.option_add_internal(slotoptions::HB_BEGGARP1,   MEGADRIVE_HB_BEGGARP1);
	device.option_add_internal(slotoptions::HB_WUKONG,     MEGADRIVE_HB_WUKONG);
	device.option_add_internal(slotoptions::HB_STARODYS,   MEGADRIVE_HB_STARODYS);

	// krikzz "SEGA SSF"
	device.option_add_internal(slotoptions::HB_SSF,         MEGADRIVE_HB_SSF);
	device.option_add_internal(slotoptions::HB_SSF_SRAM,    MEGADRIVE_HB_SSF_SRAM);
	device.option_add_internal(slotoptions::HB_SSF_EX,      MEGADRIVE_HB_SSF_EX);

	// Everdrive based carts
	device.option_add(slotoptions::HB_EVERDRIVE, MEGADRIVE_HB_EVERDRIVE);

	// WaterMelon
	device.option_add_internal(slotoptions::HB_PSOLAR,      MEGADRIVE_HB_PSOLAR);

}
