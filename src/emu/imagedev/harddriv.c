/*********************************************************************

    Code to interface the image code with harddisk core.

    Raphael Nabet 2003

    Update: 23-Feb-2004 - Unlike floppy disks, for which we support
    myriad formats on many systems, it is my intention for MESS to
    standardize on the CHD file format for hard drives so I made a few
    changes to support this

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "harddisk.h"
#include "harddriv.h"


static OPTION_GUIDE_START(hd_option_guide)
	OPTION_INT('C', "cylinders",		"Cylinders")
	OPTION_INT('H', "heads",			"Heads")
	OPTION_INT('S', "sectors",			"Sectors")
	OPTION_INT('L', "sectorlength",		"Sector Bytes")
	OPTION_INT('K', "hunksize",			"Hunk Bytes")
OPTION_GUIDE_END

static const char *hd_option_spec =
	"C1-[512]-1024;H1/2/[4]/8;S1-[16]-64;L128/256/[512]/1024;K512/1024/2048/[4096]";


// device type definition
const device_type HARDDISK = &device_creator<harddisk_image_device>;

//-------------------------------------------------
//  harddisk_image_device - constructor
//-------------------------------------------------

harddisk_image_device::harddisk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, HARDDISK, "Harddisk", tag, owner, clock),
	  device_image_interface(mconfig, *this),
	  m_chd(NULL),
	  m_hard_disk_handle(NULL)
{

}

//-------------------------------------------------
//  harddisk_image_device - destructor
//-------------------------------------------------

harddisk_image_device::~harddisk_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void harddisk_image_device::device_config_complete()
{
	// inherit a copy of the static data
	const harddisk_interface *intf = reinterpret_cast<const harddisk_interface *>(static_config());
	if (intf != NULL)
		*static_cast<harddisk_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_device_image_load,   0, sizeof(m_device_image_load));
		memset(&m_device_image_unload, 0, sizeof(m_device_image_unload));
		memset(&m_interface, 0, sizeof(m_interface));
		memset(&m_device_displayinfo, 0, sizeof(m_device_displayinfo));
	}

	image_device_format *format = global_alloc_clear(image_device_format);;
	format->m_index 	  = 0;
	format->m_name        = "chd";
	format->m_description = "CHD Hard drive";
	format->m_extensions  = "chd,hd";
	format->m_optspec     = hd_option_spec;
	format->m_next		  = NULL;

	m_formatlist = format;

	// set brief and instance name
	update_names();
}

const option_guide *harddisk_image_device::create_option_guide() const
{
	return hd_option_guide;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void harddisk_image_device::device_start()
{
	m_chd = NULL;
	m_hard_disk_handle = NULL;
}

bool harddisk_image_device::call_load()
{
	int our_result;

	our_result = internal_load_hd();

	/* Check if there is an image_load callback defined */
	if ( m_device_image_load )
	{
		/* Let the override do some additional work/checks */
		our_result = m_device_image_load(*this);
	}
	return our_result;

}

bool harddisk_image_device::call_create(int create_format, option_resolution *create_args)
{
	int err;
	UINT32 sectorsize, hunksize;
	UINT32 cylinders, heads, sectors, totalsectors;
	astring metadata;

	cylinders	= option_resolution_lookup_int(create_args, 'C');
	heads		= option_resolution_lookup_int(create_args, 'H');
	sectors		= option_resolution_lookup_int(create_args, 'S');
	sectorsize	= option_resolution_lookup_int(create_args, 'L');
	hunksize	= option_resolution_lookup_int(create_args, 'K');

	totalsectors = cylinders * heads * sectors;

	/* create the CHD file */
	chd_codec_type compression[4] = { CHD_CODEC_NONE };
	err = m_origchd.create(*image_core_file(), (UINT64)totalsectors * (UINT64)sectorsize, hunksize, sectorsize, compression);
	if (err != CHDERR_NONE)
		goto error;

	/* if we created the image and hence, have metadata to set, set the metadata */
	metadata.format(HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	err = m_origchd.write_metadata(HARD_DISK_METADATA_TAG, 0, metadata);
	m_origchd.close();

	if (err != CHDERR_NONE)
		goto error;

	return internal_load_hd();

error:
	return IMAGE_INIT_FAIL;
}

void harddisk_image_device::call_unload()
{
	/* Check if there is an image_unload callback defined */
	if ( m_device_image_unload )
	{
		m_device_image_unload(*this);
	}

	if (m_hard_disk_handle != NULL)
	{
		hard_disk_close(m_hard_disk_handle);
		m_hard_disk_handle = NULL;
	}

	m_origchd.close();
	m_diffchd.close();
	m_chd = NULL;
}

/*-------------------------------------------------
    open_disk_diff - open a DISK diff file
-------------------------------------------------*/

static chd_error open_disk_diff(emu_options &options, const char *name, chd_file &source, chd_file &diff_chd)
{
	astring fname(name, ".dif");

	/* try to open the diff */
	//printf("Opening differencing image file: %s\n", fname.cstr());
	emu_file diff_file(options.diff_directory(), OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	file_error filerr = diff_file.open(fname);
	if (filerr == FILERR_NONE)
	{
		astring fullpath(diff_file.fullpath());
		diff_file.close();

		//printf("Opening differencing image file: %s\n", fullpath.cstr());
		return diff_chd.open(fullpath, true, &source);
	}

	/* didn't work; try creating it instead */
	//printf("Creating differencing image: %s\n", fname.cstr());
	diff_file.set_openflags(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	filerr = diff_file.open(fname);
	if (filerr == FILERR_NONE)
	{
		astring fullpath(diff_file.fullpath());
		diff_file.close();

		/* create the CHD */
		//printf("Creating differencing image file: %s\n", fullpath.cstr());
		chd_codec_type compression[4] = { CHD_CODEC_NONE };
		chd_error err = diff_chd.create(fullpath, source.logical_bytes(), source.hunk_bytes(), compression, source);
		if (err != CHDERR_NONE)
			return err;

		return diff_chd.clone_all_metadata(source);
	}

	return CHDERR_FILE_NOT_FOUND;
}

int harddisk_image_device::internal_load_hd()
{
	astring tempstring;
	chd_error err = CHDERR_NONE;

	m_chd = NULL;

	/* open the CHD file */
	if (software_entry() != NULL)
	{
		m_chd  = get_disk_handle(device().machine(), device().subtag(tempstring,"harddriv"));
	}
	else
	{
		err = m_origchd.open(*image_core_file(), true);
		if (err == CHDERR_NONE)
		{
			m_chd = &m_origchd;
		}
		else if (err == CHDERR_FILE_NOT_WRITEABLE)
		{
			err = m_origchd.open(*image_core_file(), false);
			if (err == CHDERR_NONE)
			{
				err = open_disk_diff(device().machine().options(), basename_noext(), m_origchd, m_diffchd);
				if (err == CHDERR_NONE)
				{
					m_chd = &m_diffchd;
				}
			}
		}
	}

	if (m_chd != NULL)
	{
		/* open the hard disk file */
		m_hard_disk_handle = hard_disk_open(m_chd);
		if (m_hard_disk_handle != NULL)
			return IMAGE_INIT_PASS;
	}

	/* if we had an error, close out the CHD */
	m_origchd.close();
	m_diffchd.close();
	m_chd = NULL;
	seterror(IMAGE_ERROR_UNSPECIFIED, chd_file::error_string(err));

	return IMAGE_INIT_FAIL;
}

/*************************************
 *
 *  Get the CHD file (from the src/chd.c core)
 *  after an image has been opened with the hd core
 *
 *************************************/

chd_file *harddisk_image_device::get_chd_file()
{
	chd_file *result = NULL;
	hard_disk_file *hd_file = get_hard_disk_file();
	if (hd_file)
		result = hard_disk_get_chd(hd_file);
	return result;
}
