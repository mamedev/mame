// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

#include "emu.h"
#include "hil_devices.h"

#include "hlekbd.h"
#include "hlemouse.h"

void hp_hil_devices(device_slot_interface &device)
{
	device.option_add(STR_KBD_HP_INTEGRAL, HP_IPC_HLE_KEYBOARD);
	device.option_add(STR_KBD_HP_46021A, HP_ITF_HLE_KEYBOARD);
	device.option_add(STR_MOUSE_HP_46060B, HP_46060B_MOUSE);
}
