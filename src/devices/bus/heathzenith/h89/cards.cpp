// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    H-89 and Z-90 slot cards

***************************************************************************/

#include "emu.h"
#include "cards.h"

#include "cdr_fdc_880h.h"
#include "h_88_3.h"
#include "h_88_5.h"
#include "mms77316_fdc.h"
#include "sigmasoft_sound.h"
#include "we_pullup.h"
#include "z_89_11.h"
#include "z37_fdc.h"


void h89_right_cards(device_slot_interface &device)
{
	device.option_add("cdr_fdc", H89BUS_CDR_FDC_880H);
	device.option_add("h_88_3", H89BUS_H_88_3);
	device.option_add("ha_88_3", H89BUS_HA_88_3);
	device.option_add("h_88_5", H89BUS_H_88_5);
	device.option_add("ss_snd", H89BUS_SIGMASOFT_SND);
	device.option_add("z_89_11", H89BUS_Z_89_11);

	device.option_add("z37fdc", H89BUS_Z37);
}

void h89_right_cards_mms(device_slot_interface &device)
{
	h89_right_cards(device);
	device.option_add("mms77316", H89BUS_MMS77316);
}

void h89_right_p506_cards(device_slot_interface &device)
{
	device.option_add("h_88_3", H89BUS_H_88_3);
	device.option_add("ha_88_3", H89BUS_HA_88_3);
	device.option_add("ss_snd", H89BUS_SIGMASOFT_SND);
	device.option_add("we_pullup", H89BUS_WE_PULLUP);
}
