/***************************************************************************

    diimage.c

    Device image interfaces.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "drivenum.h"
#include "ui.h"
#include "zippath.h"
#include "uiimage.h"
#include "uiswlist.h"

//**************************************************************************
//  DEVICE CONFIG IMAGE INTERFACE
//**************************************************************************
const image_device_type_info device_image_interface::m_device_info_array[] =
	{
		{ IO_UNKNOWN,	"unknown",		"unkn" },
		{ IO_CARTSLOT,	"cartridge",	"cart" }, /*  0 */
		{ IO_FLOPPY,	"floppydisk",	"flop" }, /*  1 */
		{ IO_HARDDISK,	"harddisk",		"hard" }, /*  2 */
		{ IO_CYLINDER,	"cylinder",		"cyln" }, /*  3 */
		{ IO_CASSETTE,	"cassette",		"cass" }, /*  4 */
		{ IO_PUNCHCARD,	"punchcard",	"pcrd" }, /*  5 */
		{ IO_PUNCHTAPE,	"punchtape",	"ptap" }, /*  6 */
		{ IO_PRINTER,	"printer",		"prin" }, /*  7 */
		{ IO_SERIAL,	"serial",		"serl" }, /*  8 */
		{ IO_PARALLEL,	"parallel",		"parl" }, /*  9 */
		{ IO_SNAPSHOT,	"snapshot",		"dump" }, /* 10 */
		{ IO_QUICKLOAD,	"quickload",	"quik" }, /* 11 */
		{ IO_MEMCARD,	"memcard",		"memc" }, /* 12 */
		{ IO_CDROM,     "cdrom",        "cdrm" }, /* 13 */
		{ IO_MAGTAPE,	"magtape",		"magt" }, /* 14 */
		{ IO_ROM,		"romimage",		"rom"  }, /* 15 */
	};


//**************************************************************************
//  DEVICE IMAGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_image_interface - constructor
//-------------------------------------------------

device_image_interface::device_image_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device),
	  m_file(NULL),
	  m_mame_file(NULL),
	  m_full_software_name(NULL),
	  m_software_info_ptr(NULL),
	  m_software_part_ptr(NULL),
	  m_software_list_name(NULL),
      m_readonly(false),
      m_created(false),
	  m_formatlist(NULL),
	  m_is_loading(FALSE)
{
}


//-------------------------------------------------
//  ~device_image_interface - destructor
//-------------------------------------------------

device_image_interface::~device_image_interface()
{
   image_device_format **formatptr = &m_formatlist;

	/* free all entries */
	while (*formatptr != NULL)
	{
		image_device_format *entry = *formatptr;
		*formatptr = entry->m_next;
		global_free(entry);
	}
}

//-------------------------------------------------
//  find_device_type - search trough list of
//  device types to extract data
//-------------------------------------------------

const image_device_type_info *device_image_interface::find_device_type(iodevice_t type)
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(device_image_interface::m_device_info_array); i++)
	{
		if (m_device_info_array[i].m_type == type)
			return &m_device_info_array[i];
	}
	return NULL;
}

//-------------------------------------------------
//  device_typename - retrieves device type name
//-------------------------------------------------

const char *device_image_interface::device_typename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_name : NULL;
}

//-------------------------------------------------
//  device_brieftypename - retrieves device
//  brief type name
//-------------------------------------------------

const char *device_image_interface::device_brieftypename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != NULL) ? info->m_shortname : NULL;
}

//-------------------------------------------------
//  device_typeid - retrieves device type id
//-------------------------------------------------

iodevice_t device_image_interface::device_typeid(const char *name)
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(device_image_interface::m_device_info_array); i++)
	{
		if (!mame_stricmp(name, m_device_info_array[i].m_name) || !mame_stricmp(name, m_device_info_array[i].m_shortname))
			return m_device_info_array[i].m_type;
	}
	return (iodevice_t)-1;
}

/*-------------------------------------------------
    device_compute_hash - compute a hash,
    using this device's partial hash if appropriate
-------------------------------------------------*/

void device_image_interface::device_compute_hash(hash_collection &hashes, const void *data, size_t length, const char *types) const
{
	/* retrieve the partial hash func */
	device_image_partialhash_func partialhash = get_partial_hash();

	/* compute the hash */
	if (partialhash)
		partialhash(hashes, (const unsigned char*)data, length, types);
	else
		hashes.compute(reinterpret_cast<const UINT8 *>(data), length, types);
}

/*-------------------------------------------------
    set_image_filename - specifies the filename of
    an image
-------------------------------------------------*/

image_error_t device_image_interface::set_image_filename(const char *filename)
{
    m_image_name = filename;
    zippath_parent(m_working_directory, filename);
	m_basename.cpy(m_image_name);

	int loc1 = m_image_name.rchr(0,'\\');
	int loc2 = m_image_name.rchr(0,'/');
	int loc3 = m_image_name.rchr(0,':');
	int loc = MAX(loc1,MAX(loc2,loc3));
	if (loc!=-1) {
		if (loc == loc3)
		{
			// temp workaround for softlists now that m_image_name contains the part name too (e.g. list:gamename:cart)
			astring tmpstr = astring(m_basename.substr(0,loc));
			int tmploc = tmpstr.rchr(0,':');
			m_basename = m_basename.substr(tmploc + 1,loc-tmploc);
		}
		else
			m_basename = m_basename.substr(loc + 1,m_basename.len()-loc);
	}
	m_basename_noext = m_basename.cpy(m_basename);
	m_filetype = "";
	loc = m_basename_noext.rchr(0,'.');
	if (loc!=-1) {
		m_basename_noext = m_basename_noext.substr(0,loc);
		m_filetype = m_basename.cpy(m_basename);
		m_filetype = m_filetype.substr(loc + 1,m_filetype.len()-loc);
	}

    return IMAGE_ERROR_SUCCESS;
}

/****************************************************************************
    CREATION FORMATS
****************************************************************************/

/*-------------------------------------------------
    device_get_indexed_creatable_format -
    accesses a specific image format available for
    image creation by index
-------------------------------------------------*/

const image_device_format *device_image_interface::device_get_indexed_creatable_format(int index)
{
    const image_device_format *format = device_get_creatable_formats();
    while(index-- && (format != NULL))
        format = format->m_next;
    return format;
}



/*-------------------------------------------------
    device_get_named_creatable_format -
    accesses a specific image format available for
    image creation by name
-------------------------------------------------*/

const image_device_format *device_image_interface::device_get_named_creatable_format(const char *format_name)
{
    const image_device_format *format = device_get_creatable_formats();
    while((format != NULL) && strcmp(format->m_name, format_name))
        format = format->m_next;
    return format;
}

/****************************************************************************
    ERROR HANDLING
****************************************************************************/

/*-------------------------------------------------
    image_clear_error - clear out any specified
    error
-------------------------------------------------*/

void device_image_interface::clear_error()
{
    m_err = IMAGE_ERROR_SUCCESS;
    if (m_err_message)
    {
        m_err_message.reset();
    }
}



/*-------------------------------------------------
    error - returns the error text for an image
    error
-------------------------------------------------*/
static const char *const messages[] =
{
	"",
	"Internal error",
	"Unsupported operation",
	"Out of memory",
	"File not found",
	"Invalid image",
	"File already open",
	"Unspecified error"
};

const char *device_image_interface::error()
{
    return (m_err_message) ? m_err_message.cstr() : messages[m_err];
}



/*-------------------------------------------------
    seterror - specifies an error on an image
-------------------------------------------------*/

void device_image_interface::seterror(image_error_t err, const char *message)
{
    clear_error();
    m_err = err;
    if (message != NULL)
    {
        m_err_message = message;
    }
}



/*-------------------------------------------------
    message - used to display a message while
    loading
-------------------------------------------------*/

void device_image_interface::message(const char *format, ...)
{
    va_list args;
    char buffer[256];

    /* format the message */
    va_start(args, format);
    vsnprintf(buffer, ARRAY_LENGTH(buffer), format, args);
    va_end(args);

    /* display the popup for a standard amount of time */
    ui_popup_time(5, "%s: %s",
        basename(),
        buffer);
}


/***************************************************************************
    WORKING DIRECTORIES
***************************************************************************/

/*-------------------------------------------------
    try_change_working_directory - tries to change
    the working directory, but only if the directory
    actually exists
-------------------------------------------------*/
bool device_image_interface::try_change_working_directory(const char *subdir)
{
    osd_directory *directory;
    const osd_directory_entry *entry;
    bool success = FALSE;
    bool done = FALSE;

    directory = osd_opendir(m_working_directory.cstr());
    if (directory != NULL)
    {
        while(!done && (entry = osd_readdir(directory)) != NULL)
        {
            if (!mame_stricmp(subdir, entry->name))
            {
                done = TRUE;
                success = entry->type == ENTTYPE_DIR;
            }
        }

        osd_closedir(directory);
    }

    /* did we successfully identify the directory? */
    if (success)
        zippath_combine(m_working_directory, m_working_directory, subdir);

    return success;
}
/*-------------------------------------------------
    setup_working_directory - sets up the working
    directory according to a few defaults
-------------------------------------------------*/

void device_image_interface::setup_working_directory()
{
	char *dst = NULL;

	osd_get_full_path(&dst,".");
    /* first set up the working directory to be the starting directory */
    m_working_directory = dst;

    /* now try browsing down to "software" */
    if (try_change_working_directory("software"))
    {
        /* now down to a directory for this computer */
        int gamedrv = driver_list::find(device().machine().system());
        while(gamedrv != -1 && !try_change_working_directory(driver_list::driver(gamedrv).name))
        {
            gamedrv = driver_list::compatible_with(gamedrv);
        }
    }
	osd_free(dst);
}

//-------------------------------------------------
//  working_directory - returns the working
//  directory to use for this image; this is
//  valid even if not mounted
//-------------------------------------------------

const char * device_image_interface::working_directory()
{
   /* check to see if we've never initialized the working directory */
    if (!m_working_directory)
        setup_working_directory();

    return m_working_directory;
}


/*-------------------------------------------------
    get_software_region
-------------------------------------------------*/

UINT8 *device_image_interface::get_software_region(const char *tag)
{
	char full_tag[256];

	if ( m_software_info_ptr == NULL || m_software_part_ptr == NULL )
		return NULL;

	sprintf( full_tag, "%s:%s", device().tag(), tag );
	return device().machine().root_device().memregion( full_tag )->base();
}


/*-------------------------------------------------
    image_get_software_region_length
-------------------------------------------------*/

UINT32 device_image_interface::get_software_region_length(const char *tag)
{
    char full_tag[256];

    sprintf( full_tag, "%s:%s", device().tag(), tag );
    return device().machine().root_device().memregion( full_tag )->bytes();
}


/*-------------------------------------------------
 image_get_feature
 -------------------------------------------------*/

const char *device_image_interface::get_feature(const char *feature_name)
{
	feature_list *feature;

	if ( ! m_software_part_ptr->featurelist )
		return NULL;

	for ( feature = m_software_part_ptr->featurelist; feature; feature = feature->next )
	{
		if ( ! strcmp( feature->name, feature_name ) )
			return feature->value;
	}

	return NULL;
}

/****************************************************************************
  Hash info loading

  If the hash is not checked and the relevant info not loaded, force that info
  to be loaded
****************************************************************************/

void device_image_interface::run_hash(void (*partialhash)(hash_collection &, const unsigned char *, unsigned long, const char *),
    hash_collection &hashes, const char *types)
{
    UINT32 size;
    UINT8 *buf = NULL;

    hashes.reset();
    size = (UINT32) length();

    buf = (UINT8*)malloc(size);
	memset(buf,0,size);

    /* read the file */
    fseek(0, SEEK_SET);
    fread(buf, size);

    if (partialhash)
        partialhash(hashes, buf, size, types);
    else
        hashes.compute(buf, size, types);

    /* cleanup */
    free(buf);
    fseek(0, SEEK_SET);
}



void device_image_interface::image_checkhash()
{
    device_image_partialhash_func partialhash;

    /* only calculate CRC if it hasn't been calculated, and the open_mode is read only */
    UINT32 crcval;
    if (!m_hash.crc(crcval) && m_readonly && !m_created)
    {
        /* do not cause a linear read of 600 megs please */
        /* TODO: use SHA1 in the CHD header as the hash */
        if (image_type() == IO_CDROM)
            return;

		/* Skip calculating the hash when we have an image mounted through a software list */
		if ( m_software_info_ptr )
			return;

        /* retrieve the partial hash func */
        partialhash = get_partial_hash();

        run_hash(partialhash, m_hash, hash_collection::HASH_TYPES_ALL);
    }
    return;
}

UINT32 device_image_interface::crc()
{
    UINT32 crc = 0;

	image_checkhash();
    m_hash.crc(crc);

    return crc;
}

/****************************************************************************
  Battery functions

  These functions provide transparent access to battery-backed RAM on an
  image; typically for cartridges.
****************************************************************************/


/*-------------------------------------------------
    battery_load - retrieves the battery
    backed RAM for an image. The file name is
    created from the machine driver name and the
    image name.
-------------------------------------------------*/
void device_image_interface::battery_load(void *buffer, int length, int fill)
{
    astring fname(device().machine().system().name, PATH_SEPARATOR, m_basename_noext, ".nv");

    image_battery_load_by_name(device().machine().options(), fname, buffer, length, fill);
}

/*-------------------------------------------------
    battery_save - stores the battery
    backed RAM for an image. The file name is
    created from the machine driver name and the
    image name.
-------------------------------------------------*/
void device_image_interface::battery_save(const void *buffer, int length)
{
    astring fname(device().machine().system().name, PATH_SEPARATOR, m_basename_noext, ".nv");

    image_battery_save_by_name(device().machine().options(), fname, buffer, length);
}

//-------------------------------------------------
//  uses_file_extension - update configuration
//  based on completed device setup
//-------------------------------------------------

bool device_image_interface::uses_file_extension(const char *file_extension) const
{
    bool result = FALSE;

	if (file_extension[0] == '.')
        file_extension++;

	/* find the extensions */
	astring extensions(file_extensions());
	char *ext = strtok((char*)extensions.cstr(),",");
	while (ext != NULL)
	{
		if (!mame_stricmp(ext, file_extension))
        {
            result = TRUE;
            break;
        }
		ext = strtok (NULL, ",");
	}
    return result;
}

/****************************************************************************
    IMAGE LOADING
****************************************************************************/

/*-------------------------------------------------
    is_loaded - quick check to determine whether an
    image is loaded
-------------------------------------------------*/

bool device_image_interface::is_loaded()
{
    return (m_file != NULL);
}

/*-------------------------------------------------
    load_image_by_path - loads an image with a
    specific path
-------------------------------------------------*/

image_error_t device_image_interface::load_image_by_path(UINT32 open_flags, const char *path)
{
    file_error filerr = FILERR_NOT_FOUND;
    image_error_t err = IMAGE_ERROR_FILENOTFOUND;
    astring revised_path;

    /* attempt to read the file */
    filerr = zippath_fopen(path, open_flags, m_file, revised_path);

    /* did the open succeed? */
    switch(filerr)
    {
        case FILERR_NONE:
            /* success! */
            m_readonly = (open_flags & OPEN_FLAG_WRITE) ? 0 : 1;
            m_created = (open_flags & OPEN_FLAG_CREATE) ? 1 : 0;
            err = IMAGE_ERROR_SUCCESS;
            break;

        case FILERR_NOT_FOUND:
        case FILERR_ACCESS_DENIED:
            /* file not found (or otherwise cannot open); continue */
            err = IMAGE_ERROR_FILENOTFOUND;
            break;

        case FILERR_OUT_OF_MEMORY:
            /* out of memory */
            err = IMAGE_ERROR_OUTOFMEMORY;
            break;

        case FILERR_ALREADY_OPEN:
            /* this shouldn't happen */
            err = IMAGE_ERROR_ALREADYOPEN;
            break;

        case FILERR_FAILURE:
        case FILERR_TOO_MANY_FILES:
        case FILERR_INVALID_DATA:
        default:
            /* other errors */
            err = IMAGE_ERROR_INTERNAL;
            break;
    }

    /* if successful, set the file name */
    if (filerr == FILERR_NONE)
        set_image_filename(revised_path);

    return err;
}

int device_image_interface::reopen_for_write(const char *path)
{
	if(m_file)
		core_fclose(m_file);

    file_error filerr = FILERR_NOT_FOUND;
    image_error_t err = IMAGE_ERROR_FILENOTFOUND;
    astring revised_path;

    /* attempt to open the file for writing*/
    filerr = zippath_fopen(path, OPEN_FLAG_READ|OPEN_FLAG_WRITE|OPEN_FLAG_CREATE, m_file, revised_path);

    /* did the open succeed? */
    switch(filerr)
    {
        case FILERR_NONE:
            /* success! */
            m_readonly = 0;
            m_created = 1;
            err = IMAGE_ERROR_SUCCESS;
            break;

        case FILERR_NOT_FOUND:
        case FILERR_ACCESS_DENIED:
            /* file not found (or otherwise cannot open); continue */
            err = IMAGE_ERROR_FILENOTFOUND;
            break;

        case FILERR_OUT_OF_MEMORY:
            /* out of memory */
            err = IMAGE_ERROR_OUTOFMEMORY;
            break;

        case FILERR_ALREADY_OPEN:
            /* this shouldn't happen */
            err = IMAGE_ERROR_ALREADYOPEN;
            break;

        case FILERR_FAILURE:
        case FILERR_TOO_MANY_FILES:
        case FILERR_INVALID_DATA:
        default:
            /* other errors */
            err = IMAGE_ERROR_INTERNAL;
            break;
    }

    /* if successful, set the file name */
    if (filerr == FILERR_NONE)
        set_image_filename(revised_path);

    return err;
}

/*-------------------------------------------------
    determine_open_plan - determines which open
    flags to use, and in what order
-------------------------------------------------*/

void device_image_interface::determine_open_plan(int is_create, UINT32 *open_plan)
{
    int i = 0;

    /* emit flags */
    if (!is_create && is_readable() && is_writeable())
        open_plan[i++] = OPEN_FLAG_READ | OPEN_FLAG_WRITE;
    if (!is_create && !is_readable() && is_writeable())
        open_plan[i++] = OPEN_FLAG_WRITE;
    if (!is_create && is_readable())
        open_plan[i++] = OPEN_FLAG_READ;
    if (is_writeable() && is_creatable())
        open_plan[i++] = OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE;
    open_plan[i] = 0;
}

/*-------------------------------------------------
    load_software - software image loading
-------------------------------------------------*/
bool device_image_interface::load_software(char *swlist, char *swname, rom_entry *start)
{
	astring locationtag, breakstr("%");
	const rom_entry *region;
	astring regiontag;
	bool retVal = FALSE;
	for (region = start; region != NULL; region = rom_next_region(region))
	{
		/* loop until we hit the end of this region */
		const rom_entry *romp = region + 1;
		while (!ROMENTRY_ISREGIONEND(romp))
		{
			/* handle files */
			if (ROMENTRY_ISFILE(romp))
			{
				file_error filerr = FILERR_NOT_FOUND;

				UINT32 crc = 0;
				bool has_crc = hash_collection(ROM_GETHASHDATA(romp)).crc(crc);

				// attempt reading up the chain through the parents and create a locationtag astring in the format
				// " swlist % clonename % parentname "
				// below, we have the code to split the elements and to create paths to load from

				software_list *software_list_ptr = software_list_open(device().machine().options(), swlist, FALSE, NULL);
				if (software_list_ptr)
				{
					for (software_info *swinfo = software_list_find(software_list_ptr, swname, NULL); swinfo != NULL; )
					{
						{
							astring tmp(swinfo->shortname);
							locationtag.cat(tmp);
							locationtag.cat(breakstr);
							//printf("%s\n", locationtag.cstr());
						}

						const char *parentname = software_get_clone(device().machine().options(), swlist, swinfo->shortname);
						if (parentname != NULL)
							swinfo = software_list_find(software_list_ptr, parentname, NULL);
						else
							swinfo = NULL;
					}
					// strip the final '%'
					locationtag.del(locationtag.len() - 1, 1);
					software_list_close(software_list_ptr);
				}

				if (software_get_support(device().machine().options(), swlist, swname) == SOFTWARE_SUPPORTED_PARTIAL)
					mame_printf_error("WARNING: support for software %s (in list %s) is only partial\n", swname, swlist);

				if (software_get_support(device().machine().options(), swlist, swname) == SOFTWARE_SUPPORTED_NO)
					mame_printf_error("WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist);

				// check if locationtag actually contains two locations separated by '%'
				// (i.e. check if we are dealing with a clone in softwarelist)
				astring tag2, tag3, tag4(locationtag), tag5;
				int separator = tag4.chr(0, '%');
				if (separator != -1)
				{
					// we are loading a clone through softlists, split the setname from the parentname
					tag5.cpysubstr(tag4, separator + 1, tag4.len() - separator + 1);
					tag4.del(separator, tag4.len() - separator);
				}

				// prepare locations where we have to load from: list/parentname & list/clonename
				astring tag1(swlist);
				tag1.cat(PATH_SEPARATOR);
				tag2.cpy(tag1.cat(tag4));
				tag1.cpy(swlist);
				tag1.cat(PATH_SEPARATOR);
				tag3.cpy(tag1.cat(tag5));

				if (tag5.chr(0, '%') != -1)
					fatalerror("We do not support clones of clones!\n");

				// try to load from the available location(s):
				// - if we are not using lists, we have regiontag only;
				// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
				// try to load from list/setname
				if ((m_mame_file == NULL) && (tag2.cstr() != NULL))
					filerr = common_process_file(device().machine().options(), tag2.cstr(), has_crc, crc, romp, &m_mame_file);
				// try to load from list/parentname
				if ((m_mame_file == NULL) && (tag3.cstr() != NULL))
					filerr = common_process_file(device().machine().options(), tag3.cstr(), has_crc, crc, romp, &m_mame_file);
				// try to load from setname
				if ((m_mame_file == NULL) && (tag4.cstr() != NULL))
					filerr = common_process_file(device().machine().options(), tag4.cstr(), has_crc, crc, romp, &m_mame_file);
				// try to load from parentname
				if ((m_mame_file == NULL) && (tag5.cstr() != NULL))
					filerr = common_process_file(device().machine().options(), tag5.cstr(), has_crc, crc, romp, &m_mame_file);

				if (filerr == FILERR_NONE)
				{
					m_file = *m_mame_file;
					retVal = TRUE;
				}

				break; // load first item for start
			}
			romp++;	/* something else; skip */
		}
	}
	return retVal;
}

/*-------------------------------------------------
    load_internal - core image loading
-------------------------------------------------*/

bool device_image_interface::load_internal(const char *path, bool is_create, int create_format, option_resolution *create_args, bool just_load)
{
    UINT32 open_plan[4];
    int i;
	bool softload = FALSE;
	m_from_swlist = FALSE;

	// if the path contains no period, we are using softlists, so we won't create an image
	astring pathstr(path);
	bool filename_has_period = (pathstr.rchr(0, '.') != -1) ? TRUE : FALSE;

    /* first unload the image */
    unload();

    /* clear any possible error messages */
    clear_error();

    /* we are now loading */
    m_is_loading = TRUE;

    /* record the filename */
    m_err = set_image_filename(path);

    if (m_err)
        goto done;

	/* Check if there's a software list defined for this device and use that if we're not creating an image */
	if (!filename_has_period && !just_load)
	{
		softload = load_software_part( device().machine().options(), this, path, &m_software_info_ptr, &m_software_part_ptr, &m_full_software_name, &m_software_list_name );
		// if we had launched from softlist with a specified part, e.g. "shortname:part"
		// we would have recorded the wrong name, so record it again based on software_info
		if (m_software_info_ptr && m_full_software_name)
			m_err = set_image_filename(m_full_software_name);

		m_from_swlist = TRUE;
	}

	if (is_create || filename_has_period)
	{
		/* determine open plan */
		determine_open_plan(is_create, open_plan);

		/* attempt to open the file in various ways */
		for (i = 0; !m_file && open_plan[i]; i++)
		{
			/* open the file */
			m_err = load_image_by_path(open_plan[i], path);
			if (m_err && (m_err != IMAGE_ERROR_FILENOTFOUND))
				goto done;
		}
	}

	/* Copy some image information when we have been loaded through a software list */
	if ( m_software_info_ptr )
	{
		m_longname = m_software_info_ptr->longname;
		m_manufacturer = m_software_info_ptr->publisher;
		m_year = m_software_info_ptr->year;
		//m_playable = m_software_info_ptr->supported;
	}

	/* did we fail to find the file? */
	if (!is_loaded() && !softload)
	{
		m_err = IMAGE_ERROR_FILENOTFOUND;
		goto done;
	}

	/* call device load or create */
	m_create_format = create_format;
	m_create_args = create_args;

	if (m_init_phase==FALSE) {
		m_err = (image_error_t)finish_load();
		if (m_err)
			goto done;
	}
    /* success! */

done:
	if (just_load) {
		if(m_err) clear();
		return m_err ? IMAGE_INIT_FAIL : IMAGE_INIT_PASS;
	}
    if (m_err!=0) {
		if (!m_init_phase)
		{
			if (device().machine().phase() == MACHINE_PHASE_RUNNING)
				popmessage("Error: Unable to %s image '%s': %s\n", is_create ? "create" : "load", path, error());
			else
				mame_printf_error("Error: Unable to %s image '%s': %s", is_create ? "create" : "load", path, error());
		}
		clear();
	}
	else {
		/* do we need to reset the CPU? only schedule it if load/create is successful */
		if (device().machine().time() > attotime::zero && is_reset_on_load())
			device().machine().schedule_hard_reset();
		else
		{
			if (!m_init_phase)
			{
				if (device().machine().phase() == MACHINE_PHASE_RUNNING)
					popmessage("Image '%s' was successfully %s.", path, is_create ? "created" : "loaded");
				else
					mame_printf_info("Image '%s' was successfully %s.\n", path, is_create ? "created" : "loaded");
			}
		}
	}
	return m_err ? IMAGE_INIT_FAIL : IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    load - load an image into MESS
-------------------------------------------------*/

bool device_image_interface::load(const char *path)
{
    return load_internal(path, FALSE, 0, NULL, FALSE);
}

/*-------------------------------------------------
    open_image_file - opening plain image file
-------------------------------------------------*/

bool device_image_interface::open_image_file(emu_options &options)
{
	const char* path = options.value(instance_name());
	if (*path != 0)
	{
		set_init_phase();
		if (load_internal(path, FALSE, 0, NULL, TRUE)==IMAGE_INIT_PASS)
		{
			if (software_entry()==NULL) return true;
		}
	}
	return false;
}

/*-------------------------------------------------
    image_finish_load - special call - only use
    from core
-------------------------------------------------*/

bool device_image_interface::finish_load()
{
    bool err = IMAGE_INIT_PASS;

    if (m_is_loading)
    {
		image_checkhash();

		if (m_from_swlist)
			call_display_info();

		if (has_been_created())
        {
            err = call_create(m_create_format, m_create_args);
            if (err)
            {
                if (!m_err)
                    m_err = IMAGE_ERROR_UNSPECIFIED;
            }
        }
        else
        {
            /* using device load */
            err = call_load();
            if (err)
            {
                if (!m_err)
                    m_err = IMAGE_ERROR_UNSPECIFIED;
            }
        }
    }
    m_is_loading = FALSE;
    m_create_format = 0;
    m_create_args = NULL;
	m_init_phase = FALSE;
    return err;
}

/*-------------------------------------------------
    create - create a image
-------------------------------------------------*/

bool device_image_interface::create(const char *path, const image_device_format *create_format, option_resolution *create_args)
{
    int format_index = (create_format != NULL) ? create_format->m_index : 0;
    return load_internal(path, TRUE, format_index, create_args, FALSE);
}


/*-------------------------------------------------
    clear - clear all internal data pertaining
    to an image
-------------------------------------------------*/

void device_image_interface::clear()
{
	if (m_mame_file)
    {
		global_free(m_mame_file);
		m_mame_file = NULL;
		m_file = NULL;
	} else {
		if (m_file)
		{
			core_fclose(m_file);
			m_file = NULL;
		}
	}

    m_image_name.reset();
    m_readonly = false;
    m_created = false;

    m_longname.reset();
    m_manufacturer.reset();
    m_year.reset();
	m_basename.reset();
    m_basename_noext.reset();
	m_filetype.reset();

	m_full_software_name = NULL;
	m_software_info_ptr = NULL;
	m_software_part_ptr = NULL;
	m_software_list_name = NULL;
}

/*-------------------------------------------------
    unload - main call to unload an image
-------------------------------------------------*/

void device_image_interface::unload()
{
	if (is_loaded() || m_software_info_ptr)
	{
		call_unload();
	}
    clear();
	clear_error();
}

/*-------------------------------------------------
    update_names - update brief and instance names
-------------------------------------------------*/

void device_image_interface::update_names(const device_type device_type, const char *inst, const char *brief)
{
	image_interface_iterator iter(device().mconfig().root_device());
	int count = 0;
	int index = -1;
	for (const device_image_interface *image = iter.first(); image != NULL; image = iter.next())
	{
		if (this == image)
			index = count;
		if ((image->image_type() == image_type() && device_type==NULL) || (device_type==image->device().type()))
			count++;
	}
	const char *inst_name = (device_type!=NULL) ? inst : device_typename(image_type());
	const char *brief_name = (device_type!=NULL) ? brief : device_brieftypename(image_type());
	if (count > 1)
	{
		m_instance_name.printf("%s%d", inst_name , index + 1);
		m_brief_instance_name.printf("%s%d", brief_name, index + 1);
	}
	else
	{
		m_instance_name = inst_name;
		m_brief_instance_name = brief_name;
	}
}

/*-------------------------------------------------
    get_selection_menu - create the menu stack
    for ui-level image selection
-------------------------------------------------*/

ui_menu *device_image_interface::get_selection_menu(running_machine &machine, render_container *container)
{
	return auto_alloc_clear(machine, ui_menu_control_device_image(machine, container, this));
}

ui_menu_control_device_image::ui_menu_control_device_image(running_machine &machine, render_container *container, device_image_interface *_image) : ui_menu(machine, container)
{
	image = _image;

	sld = 0;
	if (image->software_list_name()) {
		software_list_device_iterator iter(machine.config().root_device());
		for (const software_list_device *swlist = iter.first(); swlist != NULL; swlist = iter.next())
		{
			if (strcmp(swlist->list_name(),image->software_list_name())==0) sld = swlist;
		}
	}
	swi = image->software_entry();
	swp = image->part_entry();

	if(swi)
	{
		state = START_OTHER_PART;
		current_directory.cpy(image->working_directory());
	}
	else
	{
		state = START_FILE;

		/* if the image exists, set the working directory to the parent directory */
		if (image->exists())
		{
			current_file.cpy(image->filename());
			zippath_parent(current_directory, current_file);
		} else
			current_directory.cpy(image->working_directory());

		/* check to see if the path exists; if not clear it */
		if (zippath_opendir(current_directory, NULL) != FILERR_NONE)
			current_directory.reset();
	}
}

ui_menu_control_device_image::~ui_menu_control_device_image()
{
}




/*-------------------------------------------------
    create_new_image - creates a new disk image
-------------------------------------------------*/

void ui_menu_control_device_image::test_create(bool &can_create, bool &need_confirm)
{
	astring path;
	osd_directory_entry *entry;
	osd_dir_entry_type file_type;

	/* assemble the full path */
	zippath_combine(path, current_directory, current_file);

	/* does a file or a directory exist at the path */
	entry = osd_stat(path);
	file_type = (entry != NULL) ? entry->type : ENTTYPE_NONE;

	switch(file_type)
	{
		case ENTTYPE_NONE:
			/* no file/dir here - always create */
			can_create = true;
			need_confirm = false;
			break;

		case ENTTYPE_FILE:
			/* a file exists here - ask for permission from the user */
			can_create = true;
			need_confirm = true;
			break;

		case ENTTYPE_DIR:
			/* a directory exists here - we can't save over it */
			ui_popup_time(5, "Cannot save over directory");
			can_create = false;
			need_confirm = false;
			break;

		default:
			fatalerror("Unexpected");
			can_create = false;
			need_confirm = false;
			break;
	}
}

void ui_menu_control_device_image::load_software_part()
{
	astring temp_name(sld->list_name());
	temp_name.cat(":");
	temp_name.cat(swi->shortname);
	temp_name.cat(":");
	temp_name.cat(swp->name);
	hook_load(temp_name, true);
}

void ui_menu_control_device_image::hook_load(astring name, bool softlist)
{
	image->load(name);
	ui_menu::stack_pop(machine());
}

void ui_menu_control_device_image::populate()
{
}

void ui_menu_control_device_image::handle()
{
	switch(state) {
	case START_FILE: {
		bool can_create = false;
		if(image->is_creatable()) {
			zippath_directory *directory = NULL;
			file_error err = zippath_opendir(current_directory, &directory);
			can_create = err == FILERR_NONE && !zippath_is_zip(directory);
			if(directory)
				zippath_closedir(directory);
		}
		submenu_result = -1;
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_file_selector(machine(), container, image, current_directory, current_file, true, image->image_interface()!=NULL, can_create, &submenu_result)));
		state = SELECT_FILE;
		break;
	}

	case START_SOFTLIST:
		sld = 0;
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software(machine(), container, image->image_interface(), &sld)));
		state = SELECT_SOFTLIST;
		break;

	case START_OTHER_PART: {
		submenu_result = -1;
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_parts(machine(), container, swi, swp->interface_, &swp, true, &submenu_result)));
		state = SELECT_OTHER_PART;
		break;
	}

	case SELECT_SOFTLIST:
		if(!sld) {
			ui_menu::stack_pop(machine());
			break;
		}
		software_info_name = "";
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_list(machine(), container, sld, image->image_interface(), software_info_name)));
		state = SELECT_PARTLIST;
		break;

	case SELECT_PARTLIST:
		swl = software_list_open(machine().options(), sld->list_name(), false, NULL);
		swi = software_list_find(swl, software_info_name, NULL);
		if(swinfo_has_multiple_parts(swi, image->image_interface())) {
			submenu_result = -1;
			swp = 0;
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_parts(machine(), container, swi, image->image_interface(), &swp, false, &submenu_result)));
			state = SELECT_ONE_PART;
		} else {
			swp = software_find_part(swi, NULL, NULL);
			load_software_part();
			software_list_close(swl);
			ui_menu::stack_pop(machine());
		}
		break;

	case SELECT_ONE_PART:
		switch(submenu_result) {
		case ui_menu_software_parts::T_ENTRY: {
			load_software_part();
			software_list_close(swl);
			ui_menu::stack_pop(machine());
			break;
		}

		case -1: // return to list
			software_list_close(swl);
			state = SELECT_SOFTLIST;
			break;

		}
		break;

	case SELECT_OTHER_PART:
		switch(submenu_result) {
		case ui_menu_software_parts::T_ENTRY: {
			load_software_part();
			break;
		}

		case ui_menu_software_parts::T_FMGR:
			state = START_FILE;
			handle();
			break;

		case -1: // return to system
			ui_menu::stack_pop(machine());
			break;

		}
		break;

	case SELECT_FILE:
		switch(submenu_result) {
		case ui_menu_file_selector::R_EMPTY:
			image->unload();
			ui_menu::stack_pop(machine());
			break;

		case ui_menu_file_selector::R_FILE:
			hook_load(current_file, false);
			break;

		case ui_menu_file_selector::R_CREATE:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_file_create(machine(), container, image, current_directory, current_file)));
			state = CREATE_FILE;
			break;

		case ui_menu_file_selector::R_SOFTLIST:
			state = START_SOFTLIST;
			handle();
			break;

		case -1: // return to system
			ui_menu::stack_pop(machine());
			break;
		}
		break;

	case CREATE_FILE: {
		bool can_create, need_confirm;
		test_create(can_create, need_confirm);
		if(can_create) {
			if(need_confirm) {
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_confirm_save_as(machine(), container, &create_confirmed)));
				state = CREATE_CONFIRM;
			} else {
				state = DO_CREATE;
				handle();
			}
		} else {
			state = START_FILE;
			handle();
		}
		break;
	}

	case CREATE_CONFIRM: {
		state = create_confirmed ? DO_CREATE : START_FILE;
		handle();
		break;
	}

	case DO_CREATE: {
		astring path;
		zippath_combine(path, current_directory, current_file);
		int err = image->create(path, 0, NULL);
		if (err != 0)
			popmessage("Error: %s", image->error());
		ui_menu::stack_pop(machine());
		break;
	}
	}
}
