// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devdelegate.c

    Delegates that are late-bound to MAME devices.

***************************************************************************/

#include "emu.h"


//-------------------------------------------------
//  bound_object - use the device name to locate
//  a device relative to the given search root;
//  fatal error if not found
//-------------------------------------------------

delegate_late_bind &device_delegate_helper::bound_object(device_t &search_root)
{
	device_t *device = search_root.subdevice(m_device_name);
	if (device == nullptr)
		throw emu_fatalerror("Unable to locate device '%s' relative to '%s'\n", m_device_name, search_root.tag());
	return *device;
}


//-------------------------------------------------
//  safe_tag - return a tag string or (unknown) if
//  the object is not valid
//-------------------------------------------------

const char *device_delegate_helper::safe_tag(device_t *object)
{
	return (object != nullptr) ? object->tag() : "(unknown)";
}
