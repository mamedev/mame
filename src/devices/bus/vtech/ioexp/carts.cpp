// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ I/O Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "joystick.h"
#include "printer.h"


void vtech_ioexp_slot_carts(device_slot_interface &device)
{
	device.option_add("joystick", VTECH_JOYSTICK_INTERFACE);
	device.option_add("printer", VTECH_PRINTER_INTERFACE);
}
