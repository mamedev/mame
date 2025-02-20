// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    86-pin expansion slot (A500, A1000)
    Coprocessor slot (A2000, B2000)

    Card options

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_CARDS_H
#define MAME_BUS_AMIGA_CPUSLOT_CARDS_H

#pragma once


void a1000_cpuslot_cards(device_slot_interface &device) ATTR_COLD;
void a500_cpuslot_cards(device_slot_interface &device) ATTR_COLD;
void a2000_cpuslot_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_AMIGA_CPUSLOT_CARDS_H
