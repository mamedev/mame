// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#include "emu.h"
#include "hil_devices.h"

#include "hlekbd.h"

SLOT_INTERFACE_START(hp_hil_devices)
	SLOT_INTERFACE(STR_KBD_HP_INTEGRAL, HP_IPC_HLE_KEYBOARD)
SLOT_INTERFACE_END
