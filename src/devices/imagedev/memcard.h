// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    memcard.h

    Base class for memory card image devices.

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_MEMCARD_H
#define MAME_DEVICES_IMAGEDEV_MEMCARD_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_memcard_image_interface

class device_memcard_image_interface : public device_image_interface
{
public:
	// image-level overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual const char *image_type_name() const noexcept override { return "memcard"; }
	virtual const char *image_brief_type_name() const noexcept override { return "memc"; }

protected:
	// construction/destruction
	device_memcard_image_interface(const machine_config &mconfig, device_t &device);
};

#endif // MAME_DEVICES_IMAGEDEV_MEMCARD_H
