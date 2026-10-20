// license:BSD-3-Clause
// copyright-holders:A. Lenard
/**********************************************************************

    ZBI expansion cards

**********************************************************************/

#ifndef MAME_BUS_ZBI_ZBI_CARDS_H
#define MAME_BUS_ZBI_ZBI_CARDS_H

#pragma once

// supported devices
void zbi_s8k_cpu_cards(device_slot_interface &device);
void zbi_s8k_ram_cards(device_slot_interface &device);
void zbi_s8k_disk_cards(device_slot_interface &device);
void zbi_s8k_tape_cards(device_slot_interface &device);
void zbi_s8k_option1_cards(device_slot_interface &device);
void zbi_s8k_option2_cards(device_slot_interface &device);

#endif // MAME_BUS_ZBI_ZBI_CARDS_H
