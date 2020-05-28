// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont, Miodrag Milanovic
/*********************************************************************

    chd_cd.h

    Interface to the CHD CDROM code

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_CHD_CD_H
#define MAME_DEVICES_IMAGEDEV_CHD_CD_H

#pragma once

#include "cdrom.h"
#include "softlist_dev.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cdrom_image_device

class cdrom_image_device :  public device_t,
							public device_image_interface
{
public:
	// construction/destruction
	cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~cdrom_image_device();

	void set_interface(const char *interface) { m_interface = interface; }

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual iodevice_t image_type() const noexcept override { return IO_CDROM; }

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }

	// specific implementation
	cdrom_file *get_cdrom_file() { return m_cdrom_handle; }

protected:
	cdrom_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_stop() override;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	chd_file    m_self_chd;
	cdrom_file  *m_cdrom_handle;
	const char  *m_extension_list;
	const char  *m_interface;
};

// device type definition
DECLARE_DEVICE_TYPE(CDROM, cdrom_image_device)

#endif // MAME_DEVICES_IMAGEDEV_CHD_CD_H
