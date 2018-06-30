// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    ISA bus cards for ex-USSR PC clones

**********************************************************************/

#include "emu.h"
#include "xsu_cards.h"

#include "cga.h"

// storage
#include "mc1502_fdc.h"
#include "p1_fdc.h"
#include "p1_hdc.h"

// misc
#include "mc1502_rom.h"
#include "p1_rom.h"
#include "p1_sound.h"

// non-native
#include "com.h"
#include "fdc.h"
#include "hdc.h"
#include "lpt.h"
#include "mda.h"
#include "xtide.h"


void p1_isa8_cards(device_slot_interface &device)
{
	device.option_add("rom", P1_ROM);       // B003
	device.option_add("fdc", P1_FDC);       // B504
	device.option_add("hdc", P1_HDC);       // B942
	device.option_add("p1sound", P1_SOUND); // B623
/*
    device.option_add("comlpt", P1_COMLPT); // B620
    device.option_add("joy", P1_JOY);       // B621
    device.option_add("mouse", P1_MOUSE);   // B943
    device.option_add("lan", P1_LAN);       // B944
*/
	device.option_add("pccom", ISA8_COM);
	device.option_add("pclpt", ISA8_LPT);
	device.option_add("xtide", ISA8_XTIDE);
}

void mc1502_isa8_cards(device_slot_interface &device)
{
	device.option_add("cga_mc1502", ISA8_CGA_MC1502);

	device.option_add("rom", MC1502_ROM);
	device.option_add("fdc", MC1502_FDC);
/*
    device.option_add("hdc", MC1502_HDC);
*/
	device.option_add("pccom", ISA8_COM);
	device.option_add("pclpt", ISA8_LPT);
	device.option_add("xtide", ISA8_XTIDE);
}

void ec184x_isa8_cards(device_slot_interface &device)
{
	device.option_add("ec1840.0002", ISA8_EC1840_0002); // MDA with downloadable font
	device.option_add("ec1840.0003", ISA8_FDC_XT);
	device.option_add("ec1841.0002", ISA8_EC1841_0002); // CGA with downloadable font
	device.option_add("ec1841.0003", ISA8_EC1841_0003); // FDC + mouse port
	device.option_add("ec1841.0004", ISA8_LPT);
/*
    device.option_add("ec1841.0004", ISA8_EC1841_0004); // BSC-like serial ports + parallel port
    device.option_add("ec1841.0010", ISA8_EC1841_0010); // 8089-based HDC
*/
	device.option_add("mda", ISA8_MDA);
	device.option_add("hdc", ISA8_HDC_EC1841);
	device.option_add("pccom", ISA8_COM);
	device.option_add("xtide", ISA8_XTIDE);
}

void iskr103x_isa8_cards(device_slot_interface &device)
{
	device.option_add("cga_iskr1030m", ISA8_CGA_ISKR1030M);
	device.option_add("cga_iskr1031", ISA8_CGA_ISKR1031);
/**/
	device.option_add("fdc_xt", ISA8_FDC_XT);
	device.option_add("mda", ISA8_MDA);
	device.option_add("hdc", ISA8_HDC);
	device.option_add("pccom", ISA8_COM);
	device.option_add("pclpt", ISA8_LPT);
	device.option_add("xtide", ISA8_XTIDE);
}
