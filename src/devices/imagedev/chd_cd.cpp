// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont, Miodrag Milanovic
/*********************************************************************

    Code to interface the image code with CHD-CD core.

    Based on harddriv.c by Raphael Nabet 2003

*********************************************************************/

#include "emu.h"
#include "chd_cd.h"

#include "cdrom.h"
#include "romload.h"

// device type definition
DEFINE_DEVICE_TYPE(CDROM, cdrom_image_device, "cdrom_image", "CD-ROM Image")

//-------------------------------------------------
//  cdrom_image_device - constructor
//-------------------------------------------------

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cdrom_image_device(mconfig, CDROM, tag, owner, clock)
{
}

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type,  tag, owner, clock),
		device_image_interface(mconfig, *this),
		m_cdrom_handle(nullptr),
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

	add_format("chdcd", "CD-ROM drive", m_extension_list, "");
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdrom_image_device::device_start()
{
	// try to locate the CHD from a DISK_REGION
	chd_file *chd = machine().rom_load().get_disk_handle(owner()->tag() );
	if( chd != nullptr )
	{
		m_cdrom_handle = cdrom_open( chd );
	}
	else
	{
		m_cdrom_handle = nullptr;
	}
}

void cdrom_image_device::device_stop()
{
	if (m_cdrom_handle)
		cdrom_close(m_cdrom_handle);
	if( m_self_chd.opened() )
		m_self_chd.close();
}

image_init_result cdrom_image_device::call_load()
{
	std::error_condition err;
	chd_file *chd = nullptr;

	if (m_cdrom_handle)
		cdrom_close(m_cdrom_handle);

	if (!loaded_through_softlist()) {
		if (is_filetype("chd") && is_loaded()) {
			err = m_self_chd.open(util::core_file_read_write(image_core_file()));    // CDs are never writeable
			if (err)
				goto error;
			chd = &m_self_chd;
		}
	} else {
		chd = device().machine().rom_load().get_disk_handle(device().subtag("cdrom").c_str());
	}

	/* open the CHD file */
	if (chd) {
		m_cdrom_handle = cdrom_open(chd);
	} else {
		m_cdrom_handle = cdrom_open(filename());
	}
	if (!m_cdrom_handle)
		goto error;

	return image_init_result::PASS;

error:
	if (chd && chd == &m_self_chd)
		m_self_chd.close();
	if (err)
		seterror(err, nullptr);
	return image_init_result::FAIL;
}

void cdrom_image_device::call_unload()
{
	assert(m_cdrom_handle);
	cdrom_close(m_cdrom_handle);
	m_cdrom_handle = nullptr;
	if( m_self_chd.opened() )
		m_self_chd.close();
}
