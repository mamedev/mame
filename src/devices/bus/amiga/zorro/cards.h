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

#ifndef MAME_BUS_AMIGA_ZORRO_CARDS_H
#define MAME_BUS_AMIGA_ZORRO_CARDS_H

#pragma once


SLOT_INTERFACE_EXTERN( a1000_expansion_cards );
SLOT_INTERFACE_EXTERN( a500_expansion_cards );
SLOT_INTERFACE_EXTERN( a2000_expansion_cards );

SLOT_INTERFACE_EXTERN( zorro2_cards );
SLOT_INTERFACE_EXTERN( zorro3_cards );

#endif // MAME_BUS_AMIGA_ZORRO_CARDS_H
