// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont, Miodrag Milanovic
/*********************************************************************

    CD/DVD reader

*********************************************************************/

#include "emu.h"
#include "cdromimg.h"

#include "romload.h"

// device type definition
DEFINE_DEVICE_TYPE(CDROM, cdrom_image_device, "cdrom_image", "CD-ROM Image")
DEFINE_DEVICE_TYPE(GDROM, gdrom_image_device, "gdrom_image", "CD/GD-ROM Image")
DEFINE_DEVICE_TYPE(DVDROM, dvdrom_image_device, "dvdrom_image", "CD/DVD-ROM Image")

//-------------------------------------------------
//  cdrom_image_device - constructor
//-------------------------------------------------

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cdrom_image_device(mconfig, CDROM, tag, owner, clock)
{
}

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool dvd_compat, bool gd_compat)
	: device_t(mconfig, type,  tag, owner, clock),
	  device_image_interface(mconfig, *this),
	  m_gd_compat(gd_compat),
	  m_dvd_compat(dvd_compat),
	  m_cdrom_handle(nullptr),
	  m_dvdrom_handle(nullptr),
	  m_extension_list(nullptr),
	  m_interface(nullptr)
{
}

//-------------------------------------------------
//  cdrom_image_device - destructor
//-------------------------------------------------

cdrom_image_device::~cdrom_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cdrom_image_device::device_config_complete()
{
	m_extension_list = "chd,cue,toc,nrg,gdi,iso,cdr";

	add_format("chdcd", m_dvd_compat ? "CD/DVD-ROM drive" : "CD-ROM drive", m_extension_list, "");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdrom_image_device::device_start()
{
	if (has_preset_images())
		setup_current_preset_image();
	else
	{
		m_cdrom_handle.reset();
		m_dvdrom_handle.reset();
	}
}

void cdrom_image_device::setup_current_preset_image()
{
	m_cdrom_handle.reset();
	m_dvdrom_handle.reset();

	chd_file *chd = current_preset_image_chd();
	if (chd->is_cd() || (m_gd_compat && chd->is_gd()))
		m_cdrom_handle = std::make_unique<cdrom_file>(chd);
	else if(m_dvd_compat && chd->is_dvd())
		m_dvdrom_handle = std::make_unique<dvdrom_file>(chd);
	else
		fatalerror("chd for region %s is not compatible with the cdrom image device\n", preset_images_list()[current_preset_image_id()]);
}

void cdrom_image_device::device_stop()
{
	m_cdrom_handle.reset();
	m_dvdrom_handle.reset();
	if (m_self_chd.opened())
		m_self_chd.close();
}

std::pair<std::error_condition, std::string> cdrom_image_device::call_load()
{
	if (has_preset_images())
	{
		setup_current_preset_image();
		return std::make_pair(image_error(0), std::string());
	}

	std::error_condition err;
	chd_file *chd = nullptr;

	m_cdrom_handle.reset();
	m_dvdrom_handle.reset();

	if (!loaded_through_softlist())
	{
		if (is_filetype("chd") && is_loaded())
		{
			util::core_file::ptr proxy;
			err = util::core_file::open_proxy(image_core_file(), proxy);
			if (!err)
				err = m_self_chd.open(std::move(proxy)); // CDs are never writeable
			if (err)
				goto error;
			chd = &m_self_chd;
		}
	}
	else
	{
		chd = device().machine().rom_load().get_disk_handle(device().subtag("cdrom"));
	}

	// open the CHD file
	if (chd)
	{
		if (chd->is_cd() || (m_gd_compat && chd->is_gd()))
			m_cdrom_handle.reset(new cdrom_file(chd));
		else if (m_dvd_compat && chd->is_dvd())
			m_dvdrom_handle.reset(new dvdrom_file(chd));
		else
		{
			err = image_error::INVALIDIMAGE;
			goto error;
		}
	}
	else
	{
		try
		{
			m_cdrom_handle.reset(new cdrom_file(filename()));
		}
		catch (...)
		{
			try
			{
				m_dvdrom_handle.reset(new dvdrom_file(filename()));
			}
			catch (...)
			{
				err = image_error::INVALIDIMAGE;
				goto error;
			}
		}
	}

	return std::make_pair(std::error_condition(), std::string());

error:
	if (chd && chd == &m_self_chd)
		m_self_chd.close();
	return std::make_pair(err ? err : image_error::UNSPECIFIED, std::string());
}

void cdrom_image_device::call_unload()
{
	assert(m_cdrom_handle || m_dvdrom_handle);
	m_cdrom_handle.reset();
	m_dvdrom_handle.reset();
	if (m_self_chd.opened())
		m_self_chd.close();
}

int cdrom_image_device::get_last_track() const
{
	if (m_cdrom_handle)
		return m_cdrom_handle->get_last_track();
	return 0;
}

uint32_t cdrom_image_device::get_track(uint32_t frame) const
{
	if (m_cdrom_handle)
		return m_cdrom_handle->get_track(frame);
	return 0;
}

uint32_t cdrom_image_device::get_track_start(uint32_t track) const
{
	if (m_cdrom_handle)
		return m_cdrom_handle->get_track_start(track);
	return 0;
}

bool cdrom_image_device::read_data(uint32_t lbasector, void *buffer, uint32_t datatype, bool phys)
{
	if (m_cdrom_handle)
		return m_cdrom_handle->read_data(lbasector, buffer, datatype, phys);
	if (m_dvdrom_handle)
		return !m_dvdrom_handle->read_data(lbasector, buffer);
	return 0;
}

bool cdrom_image_device::read_subcode(uint32_t lbasector, void *buffer, bool phys)
{
	if (m_cdrom_handle)
		return m_cdrom_handle->read_subcode(lbasector, buffer, phys);
	return 0;
}

int cdrom_image_device::get_adr_control(int track) const
{
	if (m_cdrom_handle)
		return m_cdrom_handle->get_adr_control(track);
	return 0;
}

const cdrom_file::toc &cdrom_image_device::get_toc() const
{
	static cdrom_file::toc notoc;
	if (m_cdrom_handle)
		return m_cdrom_handle->get_toc();
	return notoc;
}

int cdrom_image_device::get_track_type(int track) const
{
	if (m_cdrom_handle)
		return m_cdrom_handle->get_track_type(track);
	return 0;
}

bool cdrom_image_device::is_cd() const
{
	return m_cdrom_handle != nullptr;
}

bool cdrom_image_device::is_gd() const
{
	return m_cdrom_handle && m_cdrom_handle->is_gdrom();
}

bool cdrom_image_device::is_dvd() const
{
	return m_dvdrom_handle != nullptr;
}

