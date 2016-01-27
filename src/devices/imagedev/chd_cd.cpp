// license:BSD-3-Clause
// copyright-holders:Nathan Woods, R. Belmont, Miodrag Milanovic
/*********************************************************************

    Code to interface the image code with CHD-CD core.

    Based on harddriv.c by Raphael Nabet 2003

*********************************************************************/

#include "emu.h"
#include "cdrom.h"
#include "chd_cd.h"


static OPTION_GUIDE_START(cd_option_guide)
	OPTION_INT('K', "hunksize",         "Hunk Bytes")
OPTION_GUIDE_END

static const char cd_option_spec[] = "K512/1024/2048/[4096]";

// device type definition
const device_type CDROM = &device_creator<cdrom_image_device>;

//-------------------------------------------------
//  cdrom_image_device - constructor
//-------------------------------------------------

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDROM, "CD-ROM Image", tag, owner, clock, "cdrom_image", __FILE__),
		device_image_interface(mconfig, *this),
		m_cdrom_handle(nullptr),
		m_extension_list(nullptr),
		m_interface(nullptr)
{
}

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name,  tag, owner, clock, shortname, source),
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

	m_formatlist.append(*global_alloc(image_device_format("chdcd", "CD-ROM drive", m_extension_list, cd_option_spec)));

	// set brief and instance name
	update_names();
}

const option_guide *cdrom_image_device::create_option_guide() const
{
	return cd_option_guide;
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

bool cdrom_image_device::call_load()
{
	chd_error   err = (chd_error)0;
	chd_file    *chd = nullptr;

	if (m_cdrom_handle)
		cdrom_close(m_cdrom_handle);

	if (software_entry() == nullptr)
	{
		if (strstr(m_image_name.c_str(), ".chd") && is_loaded()) {
			err = m_self_chd.open( *image_core_file() );    /* CDs are never writeable */
			if ( err )
				goto error;
			chd = &m_self_chd;
		}
	} else {
		chd = device().machine().rom_load().get_disk_handle(device().subtag("cdrom").c_str());
	}

	/* open the CHD file */
	if (chd) {
		m_cdrom_handle = cdrom_open( chd );
	} else {
		m_cdrom_handle = cdrom_open(m_image_name.c_str());
	}
	if ( ! m_cdrom_handle )
		goto error;

	return IMAGE_INIT_PASS;

error:
	if ( chd && chd == &m_self_chd )
		m_self_chd.close( );
	if ( err )
		seterror( IMAGE_ERROR_UNSPECIFIED, chd_file::error_string( err ) );
	return IMAGE_INIT_FAIL;
}

void cdrom_image_device::call_unload()
{
	assert(m_cdrom_handle);
	cdrom_close(m_cdrom_handle);
	m_cdrom_handle = nullptr;
	if( m_self_chd.opened() )
		m_self_chd.close();
}
