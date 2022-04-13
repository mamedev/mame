// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    picture.cpp

    Image device for still pictures.

*********************************************************************/

#include "emu.h"
#include "picture.h"
#include "rendutil.h"

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
	device_image_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  picture_image_device - destructor
//-------------------------------------------------

picture_image_device::~picture_image_device()
{
}


void picture_image_device::device_start()
{
}

image_init_result picture_image_device::call_load()
{
	switch (render_detect_image(image_core_file()))
	{
	case RENDUTIL_IMGFORMAT_PNG:
		render_load_png(m_picture, image_core_file());
		break;

	case RENDUTIL_IMGFORMAT_JPEG:
		render_load_jpeg(m_picture, image_core_file());
		break;

	case RENDUTIL_IMGFORMAT_MSDIB:
		render_load_msdib(m_picture, image_core_file());
		break;

	default:
		m_picture.reset();
	}

	return m_picture.valid() ? image_init_result::PASS : image_init_result::FAIL;
}

void picture_image_device::call_unload()
{
	if (m_picture.valid())
	{
		m_picture.reset();
	}
}
