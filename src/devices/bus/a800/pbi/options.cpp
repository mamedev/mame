// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "options.h"

#include "kmkide.h"

void atari_pbi_options(device_slot_interface &device)
{
	device.option_add("kmkide", KMKIDE_ATA);
}
