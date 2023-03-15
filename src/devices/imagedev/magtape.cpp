// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    magtape.cpp

    Base classes for magnetic tape image devices.

    TODO: figure out how to best differentiate actual devices.
    Perhaps multiple microtape types should be completely split from
    both each other and 7-track/9-track magnetic tape?

*********************************************************************/

#include "emu.h"
#include "magtape.h"

#include "softlist_dev.h"


magtape_image_device::magtape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

microtape_image_device::microtape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: magtape_image_device(mconfig, type, tag, owner, clock)
{
}

const software_list_loader &magtape_image_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}
