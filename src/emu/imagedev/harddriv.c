/*********************************************************************

    Code to interface the image code with harddisk core.

    We do not support diff files as it will involve some changes in
    the image code.  Additionally, the need for diff files comes
    from MAME's need for "cannonical" hard drive images.

    Raphael Nabet 2003

    Update: 23-Feb-2004 - Unlike floppy disks, for which we support
    myriad formats on many systems, it is my intention for MESS to
    standardize on the CHD file format for hard drives so I made a few
    changes to support this

*********************************************************************/

#include "emu.h"
#include "harddisk.h"
#include "harddriv.h"


static const char *const error_strings[] =
{
	"no error",
	"no drive interface",
	"out of memory",
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
	"can't verify file"
	"operation not supported",
	"can't find metadata",
	"invalid metadata size",
	"unsupported CHD version"
};

static const char *chd_get_error_string(int chderr)
{
	if ((chderr < 0 ) || (chderr >= ARRAY_LENGTH(error_strings)))
		return NULL;
	return error_strings[chderr];
}



static OPTION_GUIDE_START(hd_option_guide)
	OPTION_INT('C', "cylinders",		"Cylinders")
	OPTION_INT('H', "heads",			"Heads")
	OPTION_INT('S', "sectors",			"Sectors")
	OPTION_INT('L', "sectorlength",		"Sector Bytes")
	OPTION_INT('K', "hunksize",			"Hunk Bytes")
OPTION_GUIDE_END

static const char *hd_option_spec =
	"C1-[512]-1024;H1/2/[4]/8;S1-[16]-64;L128/256/[512]/1024;K512/1024/2048/[4096]";


typedef struct _dev_harddisk_t	dev_harddisk_t;
struct _dev_harddisk_t
{
	const struct harddisk_callback_config	*config;
	chd_file		*chd;
	hard_disk_file	*hard_disk_handle;
};


INLINE dev_harddisk_t *get_safe_token(device_t *device)
{
	assert( device != NULL );
	assert( device->type() == HARDDISK);
	return (dev_harddisk_t *) downcast<legacy_device_base *>(device)->token();
}


/*************************************
 *
 *  DEVICE_IMAGE_LOAD(hd)
 *  DEVICE_IMAGE_CREATE(hd)
 *
 *  Device load and create
 *
 *************************************/

static int internal_load_hd(device_image_interface &image, const char *metadata)
{
	dev_harddisk_t	*harddisk = get_safe_token( &image.device() );
	chd_error		err = (chd_error)0;
	int				is_writeable;

	/* open the CHD file */
	do
	{
		is_writeable = !image.is_readonly();
		harddisk->chd = NULL;
		err = chd_open_file(image.image_core_file(), is_writeable ? CHD_OPEN_READWRITE : CHD_OPEN_READ, NULL, &harddisk->chd);

		/* special case; if we get CHDERR_FILE_NOT_WRITEABLE, make the
         * image read only and repeat */
		if (err == CHDERR_FILE_NOT_WRITEABLE)
			image.make_readonly();
	}
	while(!harddisk->chd && is_writeable && (err == CHDERR_FILE_NOT_WRITEABLE));
	if (!harddisk->chd)
		goto done;

	/* if we created the image and hence, have metadata to set, set the metadata */
	if (metadata)
	{
		err = chd_set_metadata(harddisk->chd, HARD_DISK_METADATA_TAG, 0, metadata, strlen(metadata) + 1, 0);
		if (err != CHDERR_NONE)
			goto done;
	}

	/* open the hard disk file */
	harddisk->hard_disk_handle = hard_disk_open(harddisk->chd);
	if (!harddisk->hard_disk_handle)
		goto done;

done:
	if (err)
	{
		/* if we had an error, close out the CHD */
		if (harddisk->chd != NULL)
		{
			chd_close(harddisk->chd);
			harddisk->chd = NULL;
		}

		image.seterror(IMAGE_ERROR_UNSPECIFIED, chd_get_error_string(err));
	}
	return err ? IMAGE_INIT_FAIL : IMAGE_INIT_PASS;
}



static DEVICE_IMAGE_LOAD( hd )
{
	dev_harddisk_t	*harddisk = get_safe_token( image );
	int our_result;

	our_result = internal_load_hd(image, NULL);

	/* Check if there is an image_load callback defined */
	if ( harddisk->config && harddisk->config->device_image_load )
	{
		/* Let the override do some additional work/checks */
		our_result = harddisk->config->device_image_load( image );
	}
	return our_result;
}


static DEVICE_IMAGE_CREATE( hd )
{
	int err;
	char metadata[256];
	UINT32 sectorsize, hunksize;
	UINT32 cylinders, heads, sectors, totalsectors;

	cylinders	= option_resolution_lookup_int(create_args, 'C');
	heads		= option_resolution_lookup_int(create_args, 'H');
	sectors		= option_resolution_lookup_int(create_args, 'S');
	sectorsize	= option_resolution_lookup_int(create_args, 'L');
	hunksize	= option_resolution_lookup_int(create_args, 'K');

	totalsectors = cylinders * heads * sectors;

	/* create the CHD file */
	err = chd_create_file(image.image_core_file(), (UINT64)totalsectors * (UINT64)sectorsize, hunksize, CHDCOMPRESSION_NONE, NULL);
	if (err != CHDERR_NONE)
		goto error;

	sprintf(metadata, HARD_DISK_METADATA_FORMAT, cylinders, heads, sectors, sectorsize);
	return internal_load_hd(image, metadata);

error:
	return IMAGE_INIT_FAIL;
}



/*************************************
 *
 *  DEVICE_IMAGE_UNLOAD(hd)
 *
 *  Device unload
 *
 *************************************/

static DEVICE_IMAGE_UNLOAD( hd )
{
	dev_harddisk_t	*harddisk = get_safe_token( image );

	/* Check if there is an image_unload callback defined */
	if ( harddisk->config && harddisk->config->device_image_unload )
	{
		harddisk->config->device_image_unload( image );
	}

	if (harddisk->hard_disk_handle != NULL)
	{
		hard_disk_close(harddisk->hard_disk_handle);
		harddisk->hard_disk_handle = NULL;
	}

	if (harddisk->chd != NULL)
	{
		chd_close(harddisk->chd);
		harddisk->chd = NULL;
	}
}


/*************************************
 *
 *  Get the hard disk handle (from the src/harddisk.c core)
 *  after an image has been opened with the hd core
 *
 *************************************/

hard_disk_file *hd_get_hard_disk_file(device_t *device)
{
	dev_harddisk_t	*harddisk = get_safe_token( device );

	return harddisk->hard_disk_handle;
}


/*************************************
 *
 *  Get the CHD file (from the src/chd.c core)
 *  after an image has been opened with the hd core
 *
 *************************************/

chd_file *hd_get_chd_file(device_t *device)
{
	chd_file *result = NULL;
	hard_disk_file *hd_file;

	if (device)
	{
		hd_file = hd_get_hard_disk_file(device);
		if (hd_file)
			result = hard_disk_get_chd(hd_file);
	}
	return result;
}


/*-------------------------------------------------
    DEVICE_START(hd)
-------------------------------------------------*/

static DEVICE_START(hd)
{
	dev_harddisk_t	*harddisk = get_safe_token( device );

	harddisk->config = (const harddisk_callback_config*)device->static_config();
	harddisk->chd = NULL;
	harddisk->hard_disk_handle = NULL;
}


/*-------------------------------------------------
    DEVICE_GET_INFO(hd)
-------------------------------------------------*/

DEVICE_GET_INFO(hd)
{
	switch( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(dev_harddisk_t); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0; break;
		case DEVINFO_INT_IMAGE_TYPE:				info->i = IO_HARDDISK; break;
		case DEVINFO_INT_IMAGE_READABLE:			info->i = 1; break;
		case DEVINFO_INT_IMAGE_WRITEABLE:			info->i = 1; break;
		case DEVINFO_INT_IMAGE_CREATABLE:			info->i = 0; break;
		case DEVINFO_INT_IMAGE_CREATE_OPTCOUNT:		info->i = 1; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(hd); break;
		case DEVINFO_FCT_IMAGE_LOAD:				info->f = (genf *) DEVICE_IMAGE_LOAD_NAME(hd); break;
		case DEVINFO_FCT_IMAGE_UNLOAD:				info->f = (genf *) DEVICE_IMAGE_UNLOAD_NAME(hd); break;
		case DEVINFO_FCT_IMAGE_CREATE:				info->f = (genf *) DEVICE_IMAGE_CREATE_NAME(hd); break;
		case DEVINFO_PTR_IMAGE_CREATE_OPTGUIDE:		info->p = (void *) hd_option_guide; break;
		case DEVINFO_PTR_IMAGE_CREATE_OPTSPEC+0:	info->p = (void *) hd_option_spec; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Harddisk"); break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Harddisk"); break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:		strcpy(info->s, "chd,hd"); break;
		case DEVINFO_STR_IMAGE_CREATE_OPTNAME+0:	strcpy(info->s, "chd"); break;
		case DEVINFO_STR_IMAGE_CREATE_OPTDESC+0:	strcpy(info->s, "CHD Hard drive"); break;
		case DEVINFO_STR_IMAGE_CREATE_OPTEXTS+0:	strcpy(info->s, "chd,hd"); break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(HARDDISK, hd);
