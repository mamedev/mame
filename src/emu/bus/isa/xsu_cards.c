/**********************************************************************

    ISA bus cards for ex-USSR PC clones

    license: MAME, GPL-2.0+
    copyright-holders: XXX

**********************************************************************/

#include "xsu_cards.h"

SLOT_INTERFACE_START( p1_isa8_cards )
	SLOT_INTERFACE("rom", P1_ROM)
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
