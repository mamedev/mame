#pragma once

#ifndef __NUBUS_IMAGE_H__
#define __NUBUS_IMAGE_H__

#include "emu.h"
#include "machine/nubus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct disk_data
{
	device_t *device;
	UINT32 size;
	UINT8 *data;
	bool ejected;

	device_image_interface *image;
};

// ======================> nubus_image_device

class nubus_image_device :
		public device_t,
		public device_nubus_card_interface
{
public:
		// construction/destruction
		nubus_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		nubus_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
		virtual const rom_entry *device_rom_region() const;

protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_reset();

		DECLARE_READ32_MEMBER(image_status_r);
		DECLARE_WRITE32_MEMBER(image_status_w);
		DECLARE_READ32_MEMBER(image_r);
		DECLARE_WRITE32_MEMBER(image_w);
		DECLARE_READ32_MEMBER(image_super_r);
		DECLARE_WRITE32_MEMBER(image_super_w);

public:
	disk_data *m_image;
};


// device type definition
extern const device_type NUBUS_IMAGE;

#endif  /* __NUBUS_IMAGE_H__ */
