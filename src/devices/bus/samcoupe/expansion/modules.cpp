// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Expansion Slot modules

***************************************************************************/

#include "emu.h"
#include "modules.h"

#include "blue_sampler.h"
#include "dallas.h"
#include "onemeg.h"
#include "sambus.h"
#include "sid.h"
#include "spi.h"
#include "voicebox.h"

void samcoupe_expansion_modules(device_slot_interface &device)
{
	device.option_add("blue_sampler", SAM_BLUE_SOUND_SAMPLER);
	device.option_add("dallas", SAM_DALLAS_CLOCK);
	device.option_add("onemeg", SAM_ONEMEG);
	device.option_add("sambus", SAM_SAMBUS);
	device.option_add("sid6581", SAM_SID6581);
	device.option_add("sid8580", SAM_SID8580);
	device.option_add("spi", SAM_SPI);
	device.option_add("voicebox", SAM_VOICEBOX);
}
