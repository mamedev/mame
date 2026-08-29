// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Apple Desktop Bus devices

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "adbmonitor.h"
#include "adbhlekeyboard.h"
#include "adbhlemouse.h"
#include "adbhlepippin.h"
#include "a9m0115.h"
#include "a9m0330.h"
#include "a9m0331.h"
#include "kb305.h"
#include "tk3000.h"

void adb_devices(device_slot_interface &device)
{
	device.option_add("hle_keyboard", ADB_HLE_KEYBOARD);
	device.option_add("hle_mouse", ADB_HLE_MOUSE);
	device.option_add("pippin_controller", ADB_HLE_PIPPIN_CONTROLLER);
	device.option_add("ext_kbd", ADB_A9M0115);
	device.option_add("ext_kbdii", ADB_A9M3501);
	device.option_add("iigs_kbd", ADB_A9M0330);
	device.option_add("iigs_kbd_r1", ADB_A9M0330_01);
	device.option_add("kbd_ii", ADB_A9M0487);
	device.option_add("apple_mouse", ADB_A9M0331);
	device.option_add("monitor", ADB_MONITOR);
	device.option_add("kb305", ADB_KB305);
	device.option_add("tk3000", ADB_TK3000);
}
