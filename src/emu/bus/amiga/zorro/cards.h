// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Amiga Zorro Cards

    There are different card types and layouts:

    * 86-pin expansion slot
      - A1000
      - A500 (rotated slot)
      - A2000/B2000 (internal slot)
    * Zorro-II
      - A2000
      - B2000
    * Zorro-III
      - A3000, A4000

    For details see zorro.h. Zorro-II cards can be inserted into
    Zorro-III slots.

***************************************************************************/

#pragma once

#ifndef __CARDS_H__
#define __CARDS_H__

#include "emu.h"

#include "a2052.h"
#include "a2232.h"
#include "a590.h"
#include "action_replay.h"
#include "buddha.h"

SLOT_INTERFACE_EXTERN( a1000_expansion_cards );
SLOT_INTERFACE_EXTERN( a500_expansion_cards );
SLOT_INTERFACE_EXTERN( a2000_expansion_cards );

SLOT_INTERFACE_EXTERN( zorro2_cards );
SLOT_INTERFACE_EXTERN( zorro3_cards );

#endif // __CARDS_H__
