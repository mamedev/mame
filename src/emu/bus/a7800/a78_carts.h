#ifndef __A78_CARTS_H
#define __A78_CARTS_H


#include "emu.h"

#include "rom.h"
#include "xboard.h"
#include "hiscore.h"

static SLOT_INTERFACE_START(a7800_cart)
	SLOT_INTERFACE_INTERNAL("a78_rom",      A78_ROM)
	SLOT_INTERFACE_INTERNAL("a78_pokey",    A78_ROM_POKEY)
	SLOT_INTERFACE_INTERNAL("a78_sg",       A78_ROM_SG)
	SLOT_INTERFACE_INTERNAL("a78_sg_pokey", A78_ROM_SG_POKEY)
	SLOT_INTERFACE_INTERNAL("a78_sg_ram",   A78_ROM_SG_RAM)
	// not sure which dev cart support banked ram, nor whether there shall be a 9banks or a non-sg version of this...
	SLOT_INTERFACE_INTERNAL("a78_bankram",  A78_ROM_BANKRAM)	
	SLOT_INTERFACE_INTERNAL("a78_sg9",      A78_ROM_SG_9BANKS)
	SLOT_INTERFACE_INTERNAL("a78_xmc",      A78_ROM_XM)	// carts compatible with the expansions below (basically a 9Banks+POKEY)
	SLOT_INTERFACE_INTERNAL("a78_abs",      A78_ROM_ABSOLUTE)
	SLOT_INTERFACE_INTERNAL("a78_act",      A78_ROM_ACTIVISION)
	SLOT_INTERFACE_INTERNAL("a78_hsc",      A78_HISCORE)
	SLOT_INTERFACE_INTERNAL("a78_xboard",   A78_XBOARD)	// the actual XBoarD expansion (as passthru)
	SLOT_INTERFACE_INTERNAL("a78_xm",       A78_XM)	    // the actual XM expansion (as passthru)
SLOT_INTERFACE_END


// supported devices
SLOT_INTERFACE_EXTERN(a78_cart);

#endif
