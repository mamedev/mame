// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    picture.cpp

    Image device for still pictures.

*********************************************************************/

#include "emu.h"
#include "picture.h"
#include "png.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// device type definition
DEFINE_DEVICE_TYPE(IMAGE_PICTURE, picture_image_device, "picture_image", "Still Image")

//-------------------------------------------------
//  picture_image_device - constructor
//-------------------------------------------------

picture_image_device::picture_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IMAGE_PICTURE, tag, owner, clock),
	device_image_interface(mconfig, *this),
	m_picture(nullptr)
{
}

//-------------------------------------------------
//  picture_image_device - destructor
//-------------------------------------------------

picture_image_device::~picture_image_device()
{
	if (m_picture)
	{
		delete m_picture;
		m_picture = nullptr;
	}
}


void picture_image_device::device_start()
{
}

image_init_result picture_image_device::call_load()
{
	m_picture = new bitmap_argb32;
	if (png_read_bitmap(image_core_file(), *m_picture) != PNGERR_NONE)
	{
		delete m_picture;
		m_picture = nullptr;

		// todo: try JPEG here.
		return image_init_result::FAIL;
	}

	return image_init_result::PASS;
}

void picture_image_device::call_unload()
{
	if (m_picture)
	{
		delete m_picture;
		m_picture = nullptr;
	}
}
