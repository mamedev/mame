// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Megadrive carts

**********************************************************************/

#include "emu.h"
#include "md_carts.h"

#include "rom.h"
#include "svp.h"
#include "sk.h"
#include "ggenie.h"
#include "eeprom.h"
#include "jcart.h"
#include "stm95.h"


void md_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",  MD_STD_ROM);
	device.option_add_internal("rom_svp",  MD_ROM_SVP);
	device.option_add_internal("rom_sk",  MD_ROM_SK);
// NVRAM handling
	device.option_add_internal("rom_sram",  MD_ROM_SRAM);
	device.option_add_internal("rom_sramsafe",  MD_ROM_SRAM);
	device.option_add_internal("rom_fram",  MD_ROM_FRAM);
	device.option_add_internal("rom_hardbl95", MD_ROM_SRAM);
	device.option_add_internal("rom_xinqig",  MD_ROM_SRAM);
	device.option_add_internal("rom_sf001",  MD_ROM_BEGGARP);
	device.option_add_internal("rom_sf002",  MD_ROM_WUKONG);
	device.option_add_internal("rom_sf004",  MD_ROM_STARODYS);
// EEPROM handling (most not supported fully yet)
	device.option_add_internal("rom_eeprom",  MD_STD_EEPROM);
	device.option_add_internal("rom_nbajam",  MD_EEPROM_NBAJAM);
	device.option_add_internal("rom_nbajamte", MD_EEPROM_NBAJAMTE);
	device.option_add_internal("rom_nflqb96",  MD_EEPROM_NFLQB96);
	device.option_add_internal("rom_cslam",  MD_EEPROM_CSLAM);
	device.option_add_internal("rom_nhlpa",  MD_EEPROM_NHLPA);
	device.option_add_internal("rom_blara",  MD_EEPROM_BLARA);
	device.option_add_internal("rom_eeprom_mode1",  MD_EEPROM_MODE1);
// J-Cart controller (Sampras Tennis)
	device.option_add_internal("rom_jcart",  MD_JCART);
// J-Cart controller + EEPROM handling (not supported fully yet)
	device.option_add_internal("rom_codemast",  MD_SEPROM_CODEMAST);
	device.option_add_internal("rom_mm96",  MD_SEPROM_MM96);
// STM95 EEPROM
	device.option_add_internal("rom_stm95",  MD_EEPROM_STM95);
// CodeMasters 2-in-1 (reset based)
	device.option_add_internal("rom_cm2in1",  MD_ROM_CM2IN1);
// Game Genie
	device.option_add_internal("rom_ggenie",  MD_ROM_GAMEGENIE);
// unique bankswitch
	device.option_add_internal("rom_ssf2",  MD_ROM_SSF2);
	device.option_add_internal("rom_radica",  MD_ROM_RADICA);
// pirate mappers (protection and/or bankswitch)
	device.option_add_internal("rom_16mj2",  MD_ROM_16MJ2);
	device.option_add_internal("rom_bugs",  MD_ROM_BUGSLIFE);
	device.option_add_internal("rom_chinf3",  MD_ROM_CHINF3);
	device.option_add_internal("rom_elfwor",  MD_ROM_ELFWOR);
	device.option_add_internal("rom_yasech",  MD_ROM_YASECH);
	device.option_add_internal("rom_kof98",  MD_ROM_KOF98);
	device.option_add_internal("rom_kof99",  MD_ROM_KOF99);
	device.option_add_internal("rom_lion2",  MD_ROM_LION2);
	device.option_add_internal("rom_lion3",  MD_ROM_LION3);
	device.option_add_internal("rom_mcpir",  MD_ROM_MCPIR);
	device.option_add_internal("rom_mjlov",  MD_ROM_MJLOV);
	device.option_add_internal("rom_cjmjclub",  MD_ROM_CJMJCLUB);
	device.option_add_internal("rom_pokea",  MD_ROM_POKEA);
	device.option_add_internal("rom_pokestad",  MD_ROM_POKESTAD);
	device.option_add_internal("rom_realtec",  MD_ROM_REALTEC);
	device.option_add_internal("rom_redcl",  MD_ROM_REDCL);
	device.option_add_internal("rom_rx3",  MD_ROM_RX3);
	device.option_add_internal("rom_sbubl",  MD_ROM_SBUBL);
	device.option_add_internal("rom_smb",  MD_ROM_SMB);
	device.option_add_internal("rom_smb2",  MD_ROM_SMB2);
	device.option_add_internal("rom_smw64",  MD_ROM_SMW64);
	device.option_add_internal("rom_smouse",  MD_ROM_SMOUSE);
	device.option_add_internal("rom_soulb",  MD_ROM_SOULB);
	device.option_add_internal("rom_squir",  MD_ROM_SQUIR);
	device.option_add_internal("rom_sram_arg96",  MD_ROM_SRAM_ARG96);
	device.option_add_internal("rom_tc2000",  MD_ROM_TC2000);
	device.option_add_internal("rom_tekkensp",  MD_ROM_TEKKENSP);
	device.option_add_internal("rom_topf",  MD_ROM_TOPF);


	device.option_add_internal("rom_nbajam_alt",  MD_EEPROM_NBAJAM_ALT);
}
