// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    papertape.h

    Base classes for paper tape reader and punch devices.

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_PAPERTAPE_H
#define MAME_DEVICES_IMAGEDEV_PAPERTAPE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> paper_tape_image_device

class paper_tape_image_device : public device_t, public device_image_interface
{
public:
	// image-level overrides
	virtual const char *image_type_name() const noexcept override { return "punchtape"; }
	virtual const char *image_brief_type_name() const noexcept override { return "ptap"; }

protected:
	// construction/destruction
	paper_tape_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

// ======================> paper_tape_reader_device

class paper_tape_reader_device : public paper_tape_image_device
{
public:
	// image-level overrides
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }

protected:
	// construction/destruction
	paper_tape_reader_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;
};

// ======================> paper_tape_punch_device

class paper_tape_punch_device : public paper_tape_image_device
{
public:
	// image-level overrides
	virtual bool is_readable() const noexcept override { return false; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return true; }

protected:
	// construction/destruction
	paper_tape_punch_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
};

#endif // MAME_DEVICES_IMAGEDEV_PAPERTAPE_H
