// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __A78_CARTS_H
#define __A78_CARTS_H

#include "emu.h"

#include "rom.h"
#include "xboard.h"
#include "hiscore.h"
#include "cpuwiz.h"

static SLOT_INTERFACE_START(a7800_cart)
	SLOT_INTERFACE_INTERNAL("a78_rom",      A78_ROM)
	SLOT_INTERFACE_INTERNAL("a78_pokey",    A78_ROM_POKEY)
	SLOT_INTERFACE_INTERNAL("a78_sg",       A78_ROM_SG)
	SLOT_INTERFACE_INTERNAL("a78_sg_pokey", A78_ROM_SG_POKEY)
	SLOT_INTERFACE_INTERNAL("a78_sg_ram",   A78_ROM_SG_RAM)
	SLOT_INTERFACE_INTERNAL("a78_sg9",      A78_ROM_SG9)
	SLOT_INTERFACE_INTERNAL("a78_abs",      A78_ROM_ABSOLUTE)
	SLOT_INTERFACE_INTERNAL("a78_act",      A78_ROM_ACTIVISION)
	SLOT_INTERFACE_INTERNAL("a78_hsc",      A78_HISCORE)
	SLOT_INTERFACE_INTERNAL("a78_xboard",   A78_XBOARD) // the actual XBoarD expansion (as passthru)
	SLOT_INTERFACE_INTERNAL("a78_xm",       A78_XM)     // the actual XM expansion (as passthru)
	SLOT_INTERFACE_INTERNAL("a78_megacart", A78_ROM_MEGACART)
	SLOT_INTERFACE_INTERNAL("a78_versa",    A78_ROM_VERSABOARD)
	// cart variants with a POKEY at 0x0450 (typically a VersaBoard variant, or an homebrew pcb)
	SLOT_INTERFACE_INTERNAL("a78_p450_t0",  A78_ROM_P450)
	SLOT_INTERFACE_INTERNAL("a78_p450_t1",  A78_ROM_P450_POKEY)
	SLOT_INTERFACE_INTERNAL("a78_p450_t6",  A78_ROM_P450_SG_RAM)
	SLOT_INTERFACE_INTERNAL("a78_p450_ta",  A78_ROM_P450_SG9)
	SLOT_INTERFACE_INTERNAL("a78_p450_vb",  A78_ROM_P450_VB)
SLOT_INTERFACE_END


// supported devices
SLOT_INTERFACE_EXTERN(a78_cart);

#endif
