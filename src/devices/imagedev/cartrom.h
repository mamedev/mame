// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    cartrom.h

    Base classes for ROM and cartridge ROM image devices.

*********************************************************************/

#ifndef MAME_IMAGEDEV_CARTROM_H
#define MAME_IMAGEDEV_CARTROM_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_rom_image_interface

class device_rom_image_interface : public device_image_interface
{
public:
	// image-level overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual const char *image_type_name() const noexcept override { return "romimage"; }
	virtual const char *image_brief_type_name() const noexcept override { return "rom"; }

protected:
	// construction/destruction
	device_rom_image_interface(const machine_config &mconfig, device_t &device);

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;
};

// ======================> device_cartrom_image_interface

class device_cartrom_image_interface : public device_rom_image_interface
{
public:
	// image-level overrides
	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }

protected:
	// construction/destruction
	device_cartrom_image_interface(const machine_config &mconfig, device_t &device);
};

#endif // MAME_IMAGEDEV_CARTROM_H
