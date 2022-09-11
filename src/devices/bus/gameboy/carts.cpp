// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "carts.h"

#include "mbc.h"
#include "rom.h"


void gameboy_cartridges(device_slot_interface &device)
{
	device.option_add_internal("rom",         GB_STD_ROM);
	device.option_add_internal("rom_mbc1",    GB_ROM_MBC1);
	device.option_add_internal("rom_mbc1col", GB_ROM_MBC1);
	device.option_add_internal("rom_mbc2",    GB_ROM_MBC2);
	device.option_add_internal("rom_mbc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc1",    GB_ROM_MBC3);
	device.option_add_internal("rom_huc3",    GB_ROM_MBC3);
	device.option_add_internal("rom_mbc5",    GB_ROM_MBC5);
	device.option_add_internal("rom_mbc6",    GB_ROM_MBC6);
	device.option_add_internal("rom_mbc7",    GB_ROM_MBC7);
	device.option_add_internal("rom_tama5",   GB_ROM_TAMA5);
	device.option_add_internal("rom_mmm01",   GB_ROM_MMM01);
	device.option_add_internal("rom_m161",    GB_ROM_M161);
	device.option_add_internal("rom_sachen1", GB_ROM_SACHEN1);
	device.option_add_internal("rom_sachen2", GB_ROM_SACHEN2);
	device.option_add_internal("rom_wisdom",  GB_ROM_WISDOM);
	device.option_add_internal("rom_yong",    GB_ROM_YONG);
	device.option_add_internal("rom_lasama",  GB_ROM_LASAMA);
	device.option_add_internal("rom_atvrac",  GB_ROM_ATVRAC);
	device.option_add_internal("rom_camera",  GB_ROM_CAMERA);
	device.option_add_internal("rom_188in1",  GB_ROM_188IN1);
	device.option_add_internal("rom_sintax",  GB_ROM_SINTAX);
	device.option_add_internal("rom_chong",   GB_ROM_CHONGWU);
	device.option_add_internal("rom_licheng", GB_ROM_LICHENG);
	device.option_add_internal("rom_digimon", GB_ROM_DIGIMON);
	device.option_add_internal("rom_rock8",   GB_ROM_ROCKMAN8);
	device.option_add_internal("rom_sm3sp",   GB_ROM_SM3SP);
//  device.option_add_internal("rom_dkong5",  GB_ROM_DKONG5);
//  device.option_add_internal("rom_unk01",   GB_ROM_UNK01);
}


void megaduck_cartridges(device_slot_interface &device)
{
	device.option_add_internal("rom",  MEGADUCK_ROM);
}
