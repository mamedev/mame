// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy and Mega Duck cartridge slot options

 ***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "camera.h"
#include "gbck003.h"
#include "huc1.h"
#include "huc3.h"
#include "liebao.h"
#include "mbc.h"
#include "mbc2.h"
#include "mbc3.h"
#include "mbc6.h"
#include "mbc7.h"
#include "mmm01.h"
#include "ntnew.h"
#include "rom.h"
#include "slmulti.h"
#include "tama5.h"


namespace bus::gameboy::slotoptions {

char const *const GB_STD            = "rom";
char const *const GB_M161           = "rom_m161";
char const *const GB_WISDOM         = "rom_wisdom";
char const *const GB_YONG           = "rom_yong";
char const *const GB_ROCKMAN8       = "rom_rock8";
char const *const GB_SM3SP          = "rom_sm3sp";
char const *const GB_SACHEN1        = "rom_sachen1";
char const *const GB_SACHEN2        = "rom_sachen2";
char const *const GB_ROCKET         = "rom_rocket";
char const *const GB_LASAMA         = "rom_lasama";
char const *const GB_MBC1           = "rom_mbc1";
char const *const GB_MBC2           = "rom_mbc2";
char const *const GB_MBC3           = "rom_mbc3";
char const *const GB_MBC30          = "rom_mbc30";
char const *const GB_MBC5           = "rom_mbc5";
char const *const GB_MBC6           = "rom_mbc6";
char const *const GB_MBC7_2K        = "rom_mbc7_2k";
char const *const GB_MBC7_4K        = "rom_mbc7_4k";
char const *const GB_MMM01          = "rom_mmm01";
char const *const GB_HUC1           = "rom_huc1";
char const *const GB_HUC3           = "rom_huc3";
char const *const GB_TAMA5          = "rom_tama5";
char const *const GB_CAMERA         = "rom_camera";
char const *const GB_TFANGBOOT      = "rom_tfboot";
char const *const GB_BBD            = "rom_bbd";
char const *const GB_DSHGGB81       = "rom_dshggb81";
char const *const GB_SINTAX         = "rom_sintax";
char const *const GB_CHONGWU        = "rom_chong";
char const *const GB_LICHENG        = "rom_licheng";
char const *const GB_NEWGBCHK       = "rom_newgbchk";
char const *const GB_VF001          = "rom_vf001";
char const *const GB_LIEBAO         = "rom_liebao";
char const *const GB_NTNEW          = "rom_ntnew";
char const *const GB_SLMULTI        = "rom_slmulti";
char const *const GB_GBCK003        = "rom_gbck003";

char const *const MEGADUCK_STD      = "rom";
char const *const MEGADUCK_BANKED   = "rom_banked";

} // namespace bus::gameboy::slotoptions



void gameboy_cartridges(device_slot_interface &device)
{
	using namespace bus::gameboy;

	device.option_add_internal(slotoptions::GB_STD,             GB_ROM_STD);
	device.option_add_internal(slotoptions::GB_WISDOM,          GB_ROM_WISDOM);
	device.option_add_internal(slotoptions::GB_YONG,            GB_ROM_YONG);
	device.option_add_internal(slotoptions::GB_ROCKMAN8,        GB_ROM_ROCKMAN8);
	device.option_add_internal(slotoptions::GB_SM3SP,           GB_ROM_SM3SP);
	device.option_add_internal(slotoptions::GB_SACHEN1,         GB_ROM_SACHEN1);
	device.option_add_internal(slotoptions::GB_SACHEN2,         GB_ROM_SACHEN2);
	device.option_add_internal(slotoptions::GB_ROCKET,          GB_ROM_ROCKET);
	device.option_add_internal(slotoptions::GB_LASAMA,          GB_ROM_LASAMA);
	device.option_add_internal(slotoptions::GB_MBC1,            GB_ROM_MBC1);
	device.option_add_internal(slotoptions::GB_MBC2,            GB_ROM_MBC2);
	device.option_add_internal(slotoptions::GB_MBC3,            GB_ROM_MBC3);
	device.option_add_internal(slotoptions::GB_MBC30,           GB_ROM_MBC30);
	device.option_add_internal(slotoptions::GB_MBC5,            GB_ROM_MBC5);
	device.option_add_internal(slotoptions::GB_MBC6,            GB_ROM_MBC6);
	device.option_add_internal(slotoptions::GB_MBC7_2K,         GB_ROM_MBC7_2K);
	device.option_add_internal(slotoptions::GB_MBC7_4K,         GB_ROM_MBC7_4K);
	device.option_add_internal(slotoptions::GB_M161,            GB_ROM_M161);
	device.option_add_internal(slotoptions::GB_MMM01,           GB_ROM_MMM01);
	device.option_add_internal(slotoptions::GB_CAMERA,          GB_ROM_CAMERA);
	device.option_add_internal(slotoptions::GB_HUC1,            GB_ROM_HUC1);
	device.option_add_internal(slotoptions::GB_HUC3,            GB_ROM_HUC3);
	device.option_add_internal(slotoptions::GB_TAMA5,           GB_ROM_TAMA5);
	device.option_add_internal(slotoptions::GB_TFANGBOOT,       GB_ROM_TFANGBOOT);
	device.option_add_internal(slotoptions::GB_BBD,             GB_ROM_BBD);
	device.option_add_internal(slotoptions::GB_DSHGGB81,        GB_ROM_DSHGGB81);
	device.option_add_internal(slotoptions::GB_SINTAX,          GB_ROM_SINTAX);
	device.option_add_internal(slotoptions::GB_CHONGWU,         GB_ROM_CHONGWU);
	device.option_add_internal(slotoptions::GB_LICHENG,         GB_ROM_LICHENG);
	device.option_add_internal(slotoptions::GB_NEWGBCHK,        GB_ROM_NEWGBCHK);
	device.option_add_internal(slotoptions::GB_VF001,           GB_ROM_VF001);
	device.option_add_internal(slotoptions::GB_LIEBAO,          GB_ROM_LIEBAO);
	device.option_add_internal(slotoptions::GB_NTNEW,           GB_ROM_NTNEW);
	device.option_add_internal(slotoptions::GB_SLMULTI,         GB_ROM_SLMULTI);
	device.option_add_internal(slotoptions::GB_GBCK003,         GB_ROM_GBCK003);
}


void megaduck_cartridges(device_slot_interface &device)
{
	using namespace bus::gameboy;

	device.option_add_internal(slotoptions::MEGADUCK_STD,       MEGADUCK_ROM_STD);
	device.option_add_internal(slotoptions::MEGADUCK_BANKED,    MEGADUCK_ROM_BANKED);
}
