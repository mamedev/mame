// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy and Mega Duck cartridge slot options

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_CARTS_H
#define MAME_BUS_GAMEBOY_CARTS_H

#pragma once

void gameboy_cartridges(device_slot_interface &device);
void megaduck_cartridges(device_slot_interface &device);


namespace bus::gameboy::slotoptions {

extern char const *const GB_STD;
extern char const *const GB_M161;
extern char const *const GB_WISDOM;
extern char const *const GB_YONG;
extern char const *const GB_ROCKMAN8;
extern char const *const GB_SM3SP;
extern char const *const GB_SACHEN1;
extern char const *const GB_SACHEN2;
extern char const *const GB_ROCKET;
extern char const *const GB_LASAMA;
extern char const *const GB_MBC1;
extern char const *const GB_MBC2;
extern char const *const GB_MBC3;
extern char const *const GB_MBC3;
extern char const *const GB_MBC30;
extern char const *const GB_MBC5;
extern char const *const GB_MBC6;
extern char const *const GB_MBC7_2K;
extern char const *const GB_MBC7_4K;
extern char const *const GB_MMM01;
extern char const *const GB_CAMERA;
extern char const *const GB_HUC1;
extern char const *const GB_HUC3;
extern char const *const GB_TAMA5;
extern char const *const GB_TFANGBOOT;
extern char const *const GB_BBD;
extern char const *const GB_DSHGGB81;
extern char const *const GB_SINTAX;
extern char const *const GB_CHONGWU;
extern char const *const GB_LICHENG;
extern char const *const GB_NEWGBCHK;
extern char const *const GB_VF001;
extern char const *const GB_LIEBAO;
extern char const *const GB_NTNEW;
extern char const *const GB_SLMULTI;
extern char const *const GB_GBCK003;

extern char const *const MEGADUCK_STD;
extern char const *const MEGADUCK_BANKED;

} // namespace bus::gameboy::slotoptions

#endif // MAME_BUS_GAMEBOY_CARTS_H
