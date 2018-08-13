// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Generic ROM/RAM socket slots

**********************************************************************/

#ifndef MAME_BUS_GENERIC_CARTS_H
#define MAME_BUS_GENERIC_CARTS_H

#pragma once


device_slot_interface &generic_plain_slot(device_slot_interface &device);
device_slot_interface &generic_linear_slot(device_slot_interface &device);
device_slot_interface &generic_romram_plain_slot(device_slot_interface &device);

#endif // MAME_BUS_GENERIC_CARTS_H
