// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansion Slot Devices

***************************************************************************/

#include "emu.h"
#include "carts.h"

#include "floppy.h"
#include "memory.h"
#include "rs232.h"
#include "rtty.h"
#include "sdloader.h"
#include "wordpro.h"


void vtech_memexp_carts(device_slot_interface &device)
{
	device.option_add("floppy",       VTECH_FLOPPY_CONTROLLER);
	device.option_add("laser110_16k", VTECH_LASER110_16K);
	device.option_add("laser210_16k", VTECH_LASER210_16K);
	device.option_add("laser310_16k", VTECH_LASER310_16K);
	device.option_add("laser_64k",    VTECH_LASER_64K);
	device.option_add("rs232",        VTECH_RS232_INTERFACE);
	device.option_add("rtty",         VTECH_RTTY_INTERFACE);
	device.option_add("sdloader",     VTECH_SDLOADER);
	device.option_add("wordpro",      VTECH_WORDPRO);
}
