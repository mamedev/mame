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


void a1000_expansion_cards(device_slot_interface &device);
void a500_expansion_cards(device_slot_interface &device);
void a2000_expansion_cards(device_slot_interface &device);

void zorro2_cards(device_slot_interface &device);
void zorro3_cards(device_slot_interface &device);

#endif // MAME_BUS_AMIGA_ZORRO_CARDS_H
