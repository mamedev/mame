// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    memcard.cpp

    Base class for memory card image devices.

*********************************************************************/

#include "emu.h"
#include "memcard.h"


device_memcard_image_interface::device_memcard_image_interface(const machine_config &mconfig, device_t &device)
	: device_image_interface(mconfig, device)
{
}
