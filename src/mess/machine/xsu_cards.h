/**********************************************************************

    ISA bus cards for ex-USSR PC clones

    license: MAME, GPL-2.0+
    copyright-holders: XXX

**********************************************************************/

#pragma once

#ifndef __XSU_CARDS_H__
#define __XSU_CARDS_H__

#include "emu.h"

// storage
#include "machine/mc1502_fdc.h"
#include "machine/p1_fdc.h"
#include "machine/p1_hdc.h"

// misc
#include "machine/mc1502_rom.h"
#include "machine/p1_rom.h"

// non-native
#include "machine/isa_com.h"
#include "machine/isa_xtide.h"
#include "machine/pc_lpt.h"

// supported devices
SLOT_INTERFACE_EXTERN( p1_isa8_cards );
SLOT_INTERFACE_EXTERN( mc1502_isa8_cards );

#endif // __XSU_CARDS_H__
