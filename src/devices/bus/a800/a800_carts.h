// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_A800_A800_CARTS_H
#define MAME_BUS_A800_A800_CARTS_H

#pragma once


#include "rom.h"
#include "oss.h"
#include "sparta.h"

static void a800_left(device_slot_interface &device)
{
	device.option_add_internal("a800_8k",       A800_ROM);
	device.option_add_internal("a800_8k_right", A800_ROM);
	device.option_add_internal("a800_16k",      A800_ROM);
	device.option_add_internal("a800_phoenix",  A800_ROM);  // not really emulated at this stage
	device.option_add_internal("a800_bbsb",     A800_ROM_BBSB);
	device.option_add_internal("a800_oss8k",    A800_ROM_OSS8K);
	device.option_add_internal("a800_oss034m",  A800_ROM_OSS34);
	device.option_add_internal("a800_oss043m",  A800_ROM_OSS43);
	device.option_add_internal("a800_ossm091",  A800_ROM_OSS91);
	device.option_add_internal("a800_williams", A800_ROM_WILLIAMS);
	device.option_add_internal("a800_diamond",  A800_ROM_EXPRESS);
	device.option_add_internal("a800_express",  A800_ROM_EXPRESS);
	device.option_add_internal("a800_sparta",   A800_ROM_SPARTADOS);    // this is a passthru cart with unemulated (atm) subslot
	device.option_add_internal("a800_blizzard", A800_ROM);
	device.option_add_internal("a800_turbo64",  A800_ROM_TURBO);
	device.option_add_internal("a800_turbo128", A800_ROM_TURBO);
	device.option_add_internal("a800_tlink2",   A800_ROM_TELELINK2);
	device.option_add_internal("a800_sitsa",    A800_ROM_MICROCALC);
	device.option_add_internal("a800_corina",   A800_ROM);  // NOT SUPPORTED YET!
	device.option_add_internal("xegs",          XEGS_ROM);
}

static void a800_right(device_slot_interface &device)
{
	device.option_add_internal("a800_8k_right", A800_ROM);
}

static void xegs_carts(device_slot_interface &device)
{
	device.option_add_internal("xegs",          XEGS_ROM);
}

static void a5200_carts(device_slot_interface &device)
{
	device.option_add_internal("a5200",         A800_ROM);
	device.option_add_internal("a5200_2chips",  A5200_ROM_2CHIPS);
	device.option_add_internal("a5200_bbsb",    A5200_ROM_BBSB);
}

#endif // MAME_BUS_A800_A800_CARTS_H
