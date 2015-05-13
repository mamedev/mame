// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    ISA bus cards for ex-USSR PC clones

**********************************************************************/

#include "isa_cards.h"
#include "xsu_cards.h"

SLOT_INTERFACE_START( p1_isa8_cards )
	SLOT_INTERFACE("rom", P1_ROM)       // B003
	SLOT_INTERFACE("fdc", P1_FDC)       // B504
	SLOT_INTERFACE("hdc", P1_HDC)       // B942
/*
    SLOT_INTERFACE("comlpt", P1_COMLPT) // B620
    SLOT_INTERFACE("joy", P1_JOY)       // B621
    SLOT_INTERFACE("mouse", P1_MOUSE)   // B943
    SLOT_INTERFACE("lan", P1_LAN)       // B944
*/
	SLOT_INTERFACE("pccom", ISA8_COM)
	SLOT_INTERFACE("pclpt", ISA8_LPT)
	SLOT_INTERFACE("xtide", ISA8_XTIDE)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( mc1502_isa8_cards )
	SLOT_INTERFACE("cga_mc1502", ISA8_CGA_MC1502)

	SLOT_INTERFACE("rom", MC1502_ROM)
	SLOT_INTERFACE("fdc", MC1502_FDC)
/*
    SLOT_INTERFACE("hdc", MC1502_HDC)
*/
	SLOT_INTERFACE("pccom", ISA8_COM)
	SLOT_INTERFACE("pclpt", ISA8_LPT)
	SLOT_INTERFACE("xtide", ISA8_XTIDE)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( ec184x_isa8_cards )
	SLOT_INTERFACE("ec1840.0002", ISA8_EC1840_0002) // MDA with downloadable font
	SLOT_INTERFACE("ec1841.0002", ISA8_EC1841_0002) // CGA with downloadable font
	SLOT_INTERFACE("ec1841.0003", ISA8_FDC_XT)
/*
    SLOT_INTERFACE("ec1841.0010", ISA8_EC1841_0010) // 8089-based HDC
    SLOT_INTERFACE("ec1841.0003", ISA8_EC1841_0003) // FDC + mouse port
    SLOT_INTERFACE("ec1841.0004", ISA8_EC1841_0004) // BSC-like serial ports + parallel port
*/
	SLOT_INTERFACE("mda", ISA8_MDA)
	SLOT_INTERFACE("hdc", ISA8_HDC_EC1841)
	SLOT_INTERFACE("pccom", ISA8_COM)
	SLOT_INTERFACE("pclpt", ISA8_LPT)
	SLOT_INTERFACE("xtide", ISA8_XTIDE)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( iskr103x_isa8_cards )
	SLOT_INTERFACE("cga_iskr1030m", ISA8_CGA_ISKR1030M)
	SLOT_INTERFACE("cga_iskr1031", ISA8_CGA_ISKR1031)
/**/
	SLOT_INTERFACE("fdc_xt", ISA8_FDC_XT)
	SLOT_INTERFACE("mda", ISA8_MDA)
	SLOT_INTERFACE("hdc", ISA8_HDC)
	SLOT_INTERFACE("pccom", ISA8_COM)
	SLOT_INTERFACE("pclpt", ISA8_LPT)
	SLOT_INTERFACE("xtide", ISA8_XTIDE)
SLOT_INTERFACE_END
