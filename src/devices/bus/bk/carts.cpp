// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    BK parallel slot carts

***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "ay.h"
#include "covox.h"
#include "joystick.h"
#include "loopback.h"
#include "printer.h"


void bk_parallel_devices(device_slot_interface &device)
{
	device.option_add("ay", BK_AY);
	device.option_add("covox", BK_COVOX);
	device.option_add("joystick", BK_JOYSTICK);
	device.option_add("loopback", BK_LOOPBACK);
	device.option_add("printer", BK_PRINTER);
}
