// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************

    wafadrive.cpp

    Emulation of an individual drive within the Wafadrive unit
    (preliminary, no actual emulation yet)

*********************************************************************/

#include "emu.h"
#include "wafadrive.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(WAFADRIVE_IMAGE, wafadrive_image_device, "wafadrive_image", "Sinclair Wafadrive Image")

//-------------------------------------------------
//  microdrive_image_device - constructor
//-------------------------------------------------

wafadrive_image_device::wafadrive_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WAFADRIVE_IMAGE, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  microdrive_image_device - destructor
//-------------------------------------------------

wafadrive_image_device::~wafadrive_image_device()
{
}


void wafadrive_image_device::device_start()
{
}

image_init_result wafadrive_image_device::call_load()
{
	return image_init_result::PASS;
}

void wafadrive_image_device::call_unload()
{
}
