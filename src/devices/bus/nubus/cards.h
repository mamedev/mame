// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    NuBus, SE/30 PDS, and LC PDS slot cards

***************************************************************************/

#ifndef MAME_BUS_NUBUS_CARDS_H
#define MAME_BUS_NUBUS_CARDS_H

#pragma once

void mac_nubus_cards(device_slot_interface &device) ATTR_COLD;
void powermac_nubus_cards(device_slot_interface &device) ATTR_COLD;
void mac_pds030_cards(device_slot_interface &device) ATTR_COLD;
void mac_pdslc_cards(device_slot_interface &device) ATTR_COLD;
void mac_pdslc_orig_cards(device_slot_interface &device) ATTR_COLD;
void mac_iisi_cards(device_slot_interface &device) ATTR_COLD;

#endif // MAME_BUS_NUBUS_CARDS_H
