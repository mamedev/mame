/*********************************************************************

    Code to interface the image code with CHD-CD core.

    Based on harddriv.c by Raphael Nabet 2003

*********************************************************************/

#include "emu.h"
#include "cdrom.h"
#include "chd_cd.h"


static const char *const error_strings[] =
{
	"no error",
	"no drive interface",
	"out of memory",
	"file not open",
	"file already open",
	"invalid file",
	"invalid parameter",
	"invalid data",
	"file not found",
	"requires parent",
	"file not writeable",
	"read error",
	"write error",
	"codec error",
	"invalid parent",
	"hunk out of range",
	"decompression error",
	"compression error",
	"can't create file",
	"can't verify file",
	"operation not supported",
	"can't find metadata",
	"invalid metadata size",
	"unsupported CHD version",
	"verify incomplete",
	"invalid metadata",
	"invalid state",
	"operation pending",
	"unsupported format",
	"unknown compression",
	"error walking_parent",
	"error compressing"
};

static const char *chd_get_error_string(int chderr)
{
	if ((chderr < 0 ) || (chderr >= ARRAY_LENGTH(error_strings)))
		return NULL;
	return error_strings[chderr];
}


static OPTION_GUIDE_START(cd_option_guide)
	OPTION_INT('K', "hunksize",			"Hunk Bytes")
OPTION_GUIDE_END

static const char cd_option_spec[] = "K512/1024/2048/[4096]";

// device type definition
const device_type CDROM = &device_creator<cdrom_image_device>;

//-------------------------------------------------
//  cdrom_image_device - constructor
//-------------------------------------------------

cdrom_image_device::cdrom_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CDROM, "Cdrom", tag, owner, clock),
	  device_image_interface(mconfig, *this)
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
	// inherit a copy of the static data
	const cdrom_interface *intf = reinterpret_cast<const cdrom_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cdrom_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_interface, 0, sizeof(m_interface));
		memset(&m_device_displayinfo, 0, sizeof(m_device_displayinfo));
	}

	image_device_format *format = global_alloc_clear(image_device_format);;
	format->m_index 	  = 0;
	format->m_name        = "chdcd";
	format->m_description = "CD-ROM drive";
	format->m_extensions  = "chd,cue,toc,nrg,gdi";
	format->m_optspec     = cd_option_spec;
	format->m_next		  = NULL;

	m_formatlist = format;

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
	m_cdrom_handle = NULL;
}

bool cdrom_image_device::call_load()
{
	chd_error	err = (chd_error)0;
	chd_file	*chd = NULL;
	astring tempstring;

	if (software_entry() == NULL)
	{
		if (strstr(m_image_name,".chd") && is_loaded()) {
			err = m_self_chd.open( *image_core_file() );	/* CDs are never writeable */
			if ( err )
				goto error;
			chd = &m_self_chd;
		}
	} else {
		chd  = get_disk_handle(device().machine(), device().subtag(tempstring,"cdrom"));
	}

	/* open the CHD file */
	if (chd) {
		m_cdrom_handle = cdrom_open( chd );
	} else {
		m_cdrom_handle = cdrom_open( m_image_name );
	}
	if ( ! m_cdrom_handle )
		goto error;

	return IMAGE_INIT_PASS;

error:
	if ( chd && chd == &m_self_chd )
		m_self_chd.close( );
	if ( err )
		seterror( IMAGE_ERROR_UNSPECIFIED, chd_get_error_string( err ) );
	return IMAGE_INIT_FAIL;
}

void cdrom_image_device::call_unload()
{
	assert(m_cdrom_handle);
	cdrom_close(m_cdrom_handle);
	m_cdrom_handle = NULL;
}
