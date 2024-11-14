// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont, Miodrag Milanovic
/*********************************************************************

    cdromimg.h

    CD/DVD reader

*********************************************************************/

#ifndef MAME_DEVICES_IMAGEDEV_CHD_CD_H
#define MAME_DEVICES_IMAGEDEV_CHD_CD_H

#pragma once

#include "softlist_dev.h"

#include "cdrom.h"
#include "dvdrom.h"

#include <memory>
#include <string>
#include <system_error>
#include <utility>

// device type definition
DECLARE_DEVICE_TYPE(CDROM,  cdrom_image_device)
DECLARE_DEVICE_TYPE(GDROM,  gdrom_image_device)  // Includes CDROM compatibility (but not DVDROM)
DECLARE_DEVICE_TYPE(DVDROM, dvdrom_image_device) // Includes CDROM compatibility (but not GDROM)


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

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return m_interface; }
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }
	virtual const char *image_type_name() const noexcept override { return "cdrom"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cdrm"; }

	int get_last_track() const;
	int get_last_session() const;
	uint32_t get_track(uint32_t frame) const;
	uint32_t get_track_index(uint32_t frame) const;
	uint32_t get_track_start(uint32_t track) const;
	bool read_data(uint32_t lbasector, void *buffer, uint32_t datatype, bool phys=false);
	bool read_subcode(uint32_t lbasector, void *buffer, bool phys=false);
	int get_adr_control(int track) const;
	const cdrom_file::toc &get_toc() const;
	int get_track_type(int track) const;

	bool is_cd() const;
	bool is_gd() const;
	bool is_dvd() const;

protected:
	cdrom_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool dvd_compat = false, bool gd_compat = false);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	void setup_current_preset_image();

	bool        m_gd_compat;
	bool        m_dvd_compat;
	chd_file    m_self_chd;
	std::unique_ptr<cdrom_file> m_cdrom_handle;
	std::unique_ptr<dvdrom_file> m_dvdrom_handle;
	const char  *m_extension_list;
	const char  *m_interface;
};

class gdrom_image_device : public cdrom_image_device
{
public:
	// construction/destruction
	gdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		cdrom_image_device(mconfig, GDROM, tag, owner, clock, false, true) {}
	virtual ~gdrom_image_device() = default;
};

class dvdrom_image_device : public cdrom_image_device
{
public:
	// construction/destruction
	dvdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) :
		cdrom_image_device(mconfig, DVDROM, tag, owner, clock, true, false) {}
	virtual ~dvdrom_image_device() = default;
};


#endif // MAME_DEVICES_IMAGEDEV_CHD_CD_H
