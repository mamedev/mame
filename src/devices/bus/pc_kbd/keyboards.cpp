// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "keyboards.h"
#include "ec1841.h"
#include "iskr1030.h"
#include "keytro.h"
#include "msnat.h"
#include "pc83.h"
#include "pcxt83.h"
#include "pcat84.h"
#include "pcat101.h"

void pc_xt_keyboards(device_slot_interface &device)
{
	device.option_add(STR_KBD_KEYTRONIC_PC3270, PC_KBD_KEYTRONIC_PC3270);
	device.option_add(STR_KBD_IBM_PC_83, PC_KBD_IBM_PC_83);
	device.option_add(STR_KBD_IBM_PC_XT_83, PC_KBD_IBM_PC_XT_83);
	device.option_add(STR_KBD_EC_1841, PC_KBD_EC_1841);
	device.option_add(STR_KBD_ISKR_1030, PC_KBD_ISKR_1030);
}


void pc_at_keyboards(device_slot_interface &device)
{
	device.option_add(STR_KBD_KEYTRONIC_PC3270, PC_KBD_KEYTRONIC_PC3270_AT);
	device.option_add(STR_KBD_MICROSOFT_NATURAL, PC_KBD_MICROSOFT_NATURAL);
	device.option_add(STR_KBD_IBM_PC_AT_84, PC_KBD_IBM_PC_AT_84);
	device.option_add(STR_KBD_IBM_3270PC_122, PC_KBD_IBM_3270PC_122);
	device.option_add(STR_KBD_IBM_PC_AT_101, PC_KBD_IBM_PC_AT_101);
}
