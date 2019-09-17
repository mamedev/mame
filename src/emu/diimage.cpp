// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    diimage.c

    Device image interfaces.

***************************************************************************/

#include "emu.h"

#include "emuopts.h"
#include "drivenum.h"
#include "romload.h"
#include "ui/uimain.h"
#include "zippath.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "formats/ioprocs.h"

#include <regex>


//**************************************************************************
//  DEVICE CONFIG IMAGE INTERFACE
//**************************************************************************
const image_device_type_info device_image_interface::m_device_info_array[] =
	{
		{ IO_UNKNOWN,   "unknown",      "unkn" },
		{ IO_CARTSLOT,  "cartridge",    "cart" }, /*  0 */
		{ IO_FLOPPY,    "floppydisk",   "flop" }, /*  1 */
		{ IO_HARDDISK,  "harddisk",     "hard" }, /*  2 */
		{ IO_CYLINDER,  "cylinder",     "cyln" }, /*  3 */
		{ IO_CASSETTE,  "cassette",     "cass" }, /*  4 */
		{ IO_PUNCHCARD, "punchcard",    "pcrd" }, /*  5 */
		{ IO_PUNCHTAPE, "punchtape",    "ptap" }, /*  6 */
		{ IO_PRINTER,   "printout",     "prin" }, /*  7 */
		{ IO_SERIAL,    "serial",       "serl" }, /*  8 */
		{ IO_PARALLEL,  "parallel",     "parl" }, /*  9 */
		{ IO_SNAPSHOT,  "snapshot",     "dump" }, /* 10 */
		{ IO_QUICKLOAD, "quickload",    "quik" }, /* 11 */
		{ IO_MEMCARD,   "memcard",      "memc" }, /* 12 */
		{ IO_CDROM,     "cdrom",        "cdrm" }, /* 13 */
		{ IO_MAGTAPE,   "magtape",      "magt" }, /* 14 */
		{ IO_ROM,       "romimage",     "rom"  }, /* 15 */
		{ IO_MIDIIN,    "midiin",       "min"  }, /* 16 */
		{ IO_MIDIOUT,   "midiout",      "mout" }, /* 17 */
		{ IO_PICTURE,   "picture",      "pic"  }  /* 18 */
	};


//**************************************************************************
//  IMAGE DEVICE FORMAT
//**************************************************************************

//-------------------------------------------------
//  ctor
//-------------------------------------------------

image_device_format::image_device_format(const std::string &name, const std::string &description, const std::string &extensions, const std::string &optspec)
	: m_name(name), m_description(description), m_optspec(optspec)
{
	std::regex comma_regex("\\,");
	std::copy(
		std::sregex_token_iterator(extensions.begin(), extensions.end(), comma_regex, -1),
		std::sregex_token_iterator(),
		std::back_inserter(m_extensions));
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

image_device_format::~image_device_format()
{
}


//**************************************************************************
//  DEVICE IMAGE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_image_interface - constructor
//-------------------------------------------------

device_image_interface::device_image_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "image")
	, m_err()
	, m_file()
	, m_mame_file()
	, m_software_part_ptr(nullptr)
	, m_supported(0)
	, m_readonly(false)
	, m_created(false)
	, m_create_format(0)
	, m_create_args(nullptr)
	, m_user_loadable(true)
	, m_is_loading(false)
	, m_is_reset_and_loading(false)
{
}


//-------------------------------------------------
//  ~device_image_interface - destructor
//-------------------------------------------------

device_image_interface::~device_image_interface()
{
}


//-------------------------------------------------
//  interface_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void device_image_interface::interface_config_complete()
{
	// set brief and instance name
	update_names();
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
	return nullptr;
}

//-------------------------------------------------
//  device_typename - retrieves device type name
//-------------------------------------------------

const char *device_image_interface::device_typename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != nullptr) ? info->m_name : "unknown";
}

//-------------------------------------------------
//  device_brieftypename - retrieves device
//  brief type name
//-------------------------------------------------

const char *device_image_interface::device_brieftypename(iodevice_t type)
{
	const image_device_type_info *info = find_device_type(type);
	return (info != nullptr) ? info->m_shortname : "unk";
}

//-------------------------------------------------
//  device_typeid - retrieves device type id
//-------------------------------------------------

iodevice_t device_image_interface::device_typeid(const char *name)
{
	int i;
	for (i = 0; i < ARRAY_LENGTH(device_image_interface::m_device_info_array); i++)
	{
		if (!core_stricmp(name, m_device_info_array[i].m_name) || !core_stricmp(name, m_device_info_array[i].m_shortname))
			return m_device_info_array[i].m_type;
	}
	return (iodevice_t)-1;
}

//-------------------------------------------------
//  set_image_filename - specifies the filename of
//  an image
//-------------------------------------------------

void device_image_interface::set_image_filename(const std::string &filename)
{
	m_image_name = filename;
	util::zippath_parent(m_working_directory, filename);
	m_basename.assign(m_image_name);

	// find the last "path separator"
	auto iter = std::find_if(
		m_image_name.rbegin(),
		m_image_name.rend(),
		[](char c) { return (c == '\\') || (c == '/') || (c == ':'); });

	if (iter != m_image_name.rend())
		m_basename.assign(iter.base(), m_image_name.end());

	m_basename_noext = m_basename;
	auto loc = m_basename_noext.find_last_of('.');
	if (loc != std::string::npos)
		m_basename_noext = m_basename_noext.substr(0, loc);

	m_filetype = core_filename_extract_extension(m_basename, true);
}


/****************************************************************************
    CREATION FORMATS
****************************************************************************/

//-------------------------------------------------
//  device_get_named_creatable_format -
//  accesses a specific image format available for
//  image creation by name
//-------------------------------------------------

const image_device_format *device_image_interface::device_get_named_creatable_format(const std::string &format_name)
{
	for (auto &format : m_formatlist)
		if (format->name() == format_name)
			return format.get();
	return nullptr;
}


//-------------------------------------------------
//  add_format
//-------------------------------------------------

void device_image_interface::add_format(std::unique_ptr<image_device_format> &&format)
{
	m_formatlist.push_back(std::move(format));
}


//-------------------------------------------------
//  add_format
//-------------------------------------------------

void device_image_interface::add_format(std::string &&name, std::string &&description, std::string &&extensions, std::string &&optspec)
{
	auto format = std::make_unique<image_device_format>(std::move(name), std::move(description), std::move(extensions), std::move(optspec));
	add_format(std::move(format));
}


/****************************************************************************
    ERROR HANDLING
****************************************************************************/

//-------------------------------------------------
//  image_clear_error - clear out any specified
//  error
//-------------------------------------------------

void device_image_interface::clear_error()
{
	m_err = IMAGE_ERROR_SUCCESS;
	if (!m_err_message.empty())
	{
		m_err_message.clear();
	}
}



//-------------------------------------------------
//  error - returns the error text for an image
//  error
//-------------------------------------------------

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
	return (!m_err_message.empty()) ? m_err_message.c_str() : messages[m_err];
}



//-------------------------------------------------
//  seterror - specifies an error on an image
//-------------------------------------------------

void device_image_interface::seterror(image_error_t err, const char *message)
{
	clear_error();
	m_err = err;
	if (message != nullptr)
	{
		m_err_message = message;
	}
}



//-------------------------------------------------
//  message - used to display a message while
//  loading
//-------------------------------------------------

void device_image_interface::message(const char *format, ...)
{
	va_list args;
	char buffer[256];

	/* format the message */
	va_start(args, format);
	vsnprintf(buffer, ARRAY_LENGTH(buffer), format, args);
	va_end(args);

	/* display the popup for a standard amount of time */
	device().machine().ui().popup_time(5, "%s: %s",
		basename(),
		buffer);
}


/***************************************************************************
    WORKING DIRECTORIES
***************************************************************************/

//-------------------------------------------------
//  try_change_working_directory - tries to change
//  the working directory, but only if the directory
//  actually exists
//-------------------------------------------------

bool device_image_interface::try_change_working_directory(const std::string &subdir)
{
	const osd::directory::entry *entry;
	bool success = false;
	bool done = false;

	auto directory = osd::directory::open(m_working_directory);
	if (directory)
	{
		while (!done && (entry = directory->read()) != nullptr)
		{
			if (!core_stricmp(subdir.c_str(), entry->name))
			{
				done = true;
				success = entry->type == osd::directory::entry::entry_type::DIR;
			}
		}

		directory.reset();
	}

	// did we successfully identify the directory?
	if (success)
		m_working_directory = util::zippath_combine(m_working_directory, subdir);

	return success;
}


//-------------------------------------------------
//  setup_working_directory - sets up the working
//  directory according to a few defaults
//-------------------------------------------------

void device_image_interface::setup_working_directory()
{
	bool success = false;
	// get user-specified directory and make sure it exists
	m_working_directory = device().mconfig().options().sw_path();
	// if multipath, get first
	size_t i = m_working_directory.find_first_of(";");
	if (i != std::string::npos)
		m_working_directory.resize(i);
	// validate directory
	if (!m_working_directory.empty())
		if (osd::directory::open(m_working_directory))
			success = true;

	// if not exist, use previous method
	if (!success)
	{
		// first set up the working directory to be the starting directory
		osd_get_full_path(m_working_directory, ".");
		// now try browsing down to "software"
		if (try_change_working_directory("software"))
			success = true;
	}

	if (success)
	{
		// now down to a directory for this computer
		int gamedrv = driver_list::find(device().machine().system());
		while(gamedrv != -1 && !try_change_working_directory(driver_list::driver(gamedrv).name))
		{
			gamedrv = driver_list::compatible_with(gamedrv);
		}
	}
}


//-------------------------------------------------
//  working_directory - returns the working
//  directory to use for this image; this is
//  valid even if not mounted
//-------------------------------------------------

const std::string &device_image_interface::working_directory()
{
	// check to see if we've never initialized the working directory
	if (m_working_directory.empty())
		setup_working_directory();

	return m_working_directory;
}


//-------------------------------------------------
//  software_entry - return a pointer to the
//  software_info structure from the softlist
//-------------------------------------------------

const software_info *device_image_interface::software_entry() const
{
	return !m_software_part_ptr ? nullptr : &m_software_part_ptr->info();
}


//-------------------------------------------------
//  get_software_region
//-------------------------------------------------

u8 *device_image_interface::get_software_region(const char *tag)
{
	if (!loaded_through_softlist())
		return nullptr;

	std::string full_tag = util::string_format("%s:%s", device().tag(), tag);
	memory_region *region = device().machine().root_device().memregion(full_tag.c_str());
	return region != nullptr ? region->base() : nullptr;
}


//-------------------------------------------------
//  image_get_software_region_length
//-------------------------------------------------

u32 device_image_interface::get_software_region_length(const char *tag)
{
	std::string full_tag = util::string_format("%s:%s", device().tag(), tag);
	memory_region *region = device().machine().root_device().memregion(full_tag.c_str());
	return region != nullptr ? region->bytes() : 0;
}


//-------------------------------------------------
//  image_get_feature
//-------------------------------------------------

const char *device_image_interface::get_feature(const char *feature_name) const
{
	return !m_software_part_ptr ? nullptr : m_software_part_ptr->feature(feature_name);
}


//-------------------------------------------------
//  load_software_region -
//-------------------------------------------------

bool device_image_interface::load_software_region(const char *tag, optional_shared_ptr<u8> &ptr)
{
	size_t size = get_software_region_length(tag);

	if (size)
	{
		ptr.allocate(size);

		memcpy(ptr, get_software_region(tag), size);
	}

	return size > 0;
}


// ****************************************************************************
// Hash info loading
//
// If the hash is not checked and the relevant info not loaded, force that info
// to be loaded
// ****************************************************************************

bool device_image_interface::run_hash(util::core_file &file, u32 skip_bytes, util::hash_collection &hashes, const char *types)
{
	// reset the hash; we want to override existing data
	hashes.reset();

	// figure out the size, and "cap" the skip bytes
	u64 size = file.size();
	skip_bytes = (u32) std::min((u64) skip_bytes, size);

	// seek to the beginning
	file.seek(skip_bytes, SEEK_SET);
	u64 position = skip_bytes;

	// keep on reading hashes
	hashes.begin(types);
	while (position < size)
	{
		uint8_t buffer[8192];

		// read bytes
		const u32 count = (u32) std::min(size - position, (u64) sizeof(buffer));
		const u32 actual_count = file.read(buffer, count);
		if (actual_count == 0)
			return false;
		position += actual_count;

		// and compute the hashes
		hashes.buffer(buffer, actual_count);
	}
	hashes.end();

	// cleanup
	file.seek(0, SEEK_SET);
	return true;
}



bool device_image_interface::image_checkhash()
{
	// only calculate CRC if it hasn't been calculated, and the open_mode is read only
	u32 crcval;
	if (!m_hash.crc(crcval) && is_readonly() && !m_created)
	{
		// do not cause a linear read of 600 megs please
		// TODO: use SHA1 in the CHD header as the hash
		if (image_type() == IO_CDROM)
			return true;

		// Skip calculating the hash when we have an image mounted through a software list
		if (loaded_through_softlist())
			return true;

		// run the hash
		if (!run_hash(*m_file, unhashed_header_length(), m_hash, util::hash_collection::HASH_TYPES_ALL))
			return false;
	}
	return true;
}


util::hash_collection device_image_interface::calculate_hash_on_file(util::core_file &file) const
{
	// calculate the hash
	util::hash_collection hash;
	if (!run_hash(file, unhashed_header_length(), hash, util::hash_collection::HASH_TYPES_ALL))
		hash.reset();
	return hash;
}


u32 device_image_interface::crc()
{
	u32 crc = 0;

	image_checkhash();
	m_hash.crc(crc);

	return crc;
}


//-------------------------------------------------
//  support_command_line_image_creation - do we
//  want to support image creation from the front
//  end command line?
//-------------------------------------------------

bool device_image_interface::support_command_line_image_creation() const
{
	bool result;
	switch (image_type())
	{
	case IO_PRINTER:
	case IO_SERIAL:
	case IO_PARALLEL:
		// going by the assumption that these device image types should support this
		// behavior; ideally we'd get rid of IO_* and just push this to the specific
		// devices
		result = true;
		break;
	default:
		result = false;
		break;
	}
	return result;
}


// ****************************************************************************
// Battery functions
//
// These functions provide transparent access to battery-backed RAM on an
// image; typically for cartridges.
// ****************************************************************************


//-------------------------------------------------
//  battery_load - retrieves the battery
//  backed RAM for an image. The file name is
//  created from the machine driver name and the
//  image name.
//-------------------------------------------------

void device_image_interface::battery_load(void *buffer, int length, int fill)
{
	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	osd_file::error filerr;
	int bytes_read = 0;
	std::string fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext.c_str()).append(".nv");

	/* try to open the battery file and read it in, if possible */
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_READ);
	filerr = file.open(fname);
	if (filerr == osd_file::error::NONE)
		bytes_read = file.read(buffer, length);

	// fill remaining bytes (if necessary)
	memset(((char *)buffer) + bytes_read, fill, length - bytes_read);
}

void device_image_interface::battery_load(void *buffer, int length, void *def_buffer)
{
	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	osd_file::error filerr;
	int bytes_read = 0;
	std::string fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext.c_str()).append(".nv");

	// try to open the battery file and read it in, if possible
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_READ);
	filerr = file.open(fname);
	if (filerr == osd_file::error::NONE)
		bytes_read = file.read(buffer, length);

	// if no file was present, copy the default battery
	if (bytes_read == 0 && def_buffer)
		memcpy((char *)buffer, (char *)def_buffer, length);
}


//-------------------------------------------------
//  battery_save - stores the battery
//  backed RAM for an image. The file name is
//  created from the machine driver name and the
//  image name.
//-------------------------------------------------

void device_image_interface::battery_save(const void *buffer, int length)
{
	assert_always(buffer && (length > 0), "Must specify sensical buffer/length");

	if (!device().machine().options().nvram_save())
		return;

	std::string fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext.c_str()).append(".nv");

	// try to open the battery file and write it out, if possible
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	osd_file::error filerr = file.open(fname);
	if (filerr == osd_file::error::NONE)
		file.write(buffer, length);
}


//-------------------------------------------------
//  uses_file_extension - update configuration
//  based on completed device setup
//-------------------------------------------------

bool device_image_interface::uses_file_extension(const char *file_extension) const
{
	bool result = false;

	if (file_extension[0] == '.')
		file_extension++;

	/* find the extensions */
	std::string extensions(file_extensions());
	char *ext = strtok((char*)extensions.c_str(),",");
	while (ext != nullptr)
	{
		if (!core_stricmp(ext, file_extension))
		{
			result = true;
			break;
		}
		ext = strtok (nullptr, ",");
	}
	return result;
}


// ***************************************************************************
// IMAGE LOADING
// ***************************************************************************

//-------------------------------------------------
//  is_loaded - quick check to determine whether an
//  image is loaded
//-------------------------------------------------

bool device_image_interface::is_loaded()
{
	return (m_file != nullptr);
}

//-------------------------------------------------
//  image_error_from_file_error - converts an image
//  error to a file error
//-------------------------------------------------

image_error_t device_image_interface::image_error_from_file_error(osd_file::error filerr)
{
	switch (filerr)
	{
	case osd_file::error::NONE:
		return IMAGE_ERROR_SUCCESS;

	case osd_file::error::NOT_FOUND:
	case osd_file::error::ACCESS_DENIED:
		// file not found (or otherwise cannot open)
		return IMAGE_ERROR_FILENOTFOUND;

	case osd_file::error::OUT_OF_MEMORY:
		// out of memory
		return IMAGE_ERROR_OUTOFMEMORY;

	case osd_file::error::ALREADY_OPEN:
		// this shouldn't happen
		return IMAGE_ERROR_ALREADYOPEN;

	case osd_file::error::FAILURE:
	case osd_file::error::TOO_MANY_FILES:
	case osd_file::error::INVALID_DATA:
	default:
		// other errors
		return IMAGE_ERROR_INTERNAL;
	}
}


//-------------------------------------------------
//  load_image_by_path - loads an image with a
//  specific path
//-------------------------------------------------

image_error_t device_image_interface::load_image_by_path(u32 open_flags, const std::string &path)
{
	std::string revised_path;

	// attempt to read the file
	auto const filerr = util::zippath_fopen(path, open_flags, m_file, revised_path);
	if (filerr != osd_file::error::NONE)
		return image_error_from_file_error(filerr);

	m_readonly = (open_flags & OPEN_FLAG_WRITE) ? 0 : 1;
	m_created = (open_flags & OPEN_FLAG_CREATE) ? 1 : 0;
	set_image_filename(revised_path);
	return IMAGE_ERROR_SUCCESS;
}


//-------------------------------------------------
//  reopen_for_write
//-------------------------------------------------

int device_image_interface::reopen_for_write(const std::string &path)
{
	m_file.reset();

	std::string revised_path;

	// attempt to open the file for writing
	auto const filerr = util::zippath_fopen(path, OPEN_FLAG_READ|OPEN_FLAG_WRITE|OPEN_FLAG_CREATE, m_file, revised_path);
	if (filerr != osd_file::error::NONE)
		return image_error_from_file_error(filerr);

	// success!
	m_readonly = 0;
	m_created = 1;
	set_image_filename(revised_path);

	return IMAGE_ERROR_SUCCESS;
}


//-------------------------------------------------
//  determine_open_plan - determines which open
//  flags to use, and in what order
//-------------------------------------------------

std::vector<u32> device_image_interface::determine_open_plan(bool is_create)
{
	std::vector<u32> open_plan;

	// emit flags into a vector
	if (!is_create && is_readable() && is_writeable())
		open_plan.push_back(OPEN_FLAG_READ | OPEN_FLAG_WRITE);
	if (!is_create && !is_readable() && is_writeable())
		open_plan.push_back(OPEN_FLAG_WRITE);
	if (!is_create && is_readable())
		open_plan.push_back(OPEN_FLAG_READ);
	if (is_create && is_writeable() && is_creatable())
		open_plan.push_back(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);

	return open_plan;
}


//-------------------------------------------------
//  dump_wrong_and_correct_checksums - dump an
//  error message containing the wrong and the
//  correct checksums for a given software item
//-------------------------------------------------

static void dump_wrong_and_correct_checksums(const util::hash_collection &hashes, const util::hash_collection &acthashes)
{
	osd_printf_error("    EXPECTED: %s\n", hashes.macro_string().c_str());
	osd_printf_error("       FOUND: %s\n", acthashes.macro_string().c_str());
}


//-------------------------------------------------
//  verify_length_and_hash - verify the length
//  and hash signatures of a file
//-------------------------------------------------

static int verify_length_and_hash(emu_file *file, const char *name, u32 explength, const util::hash_collection &hashes)
{
	int retVal = 0;
	if (file==nullptr) return 0;

	// verify length
	u32 actlength = file->size();
	if (explength != actlength)
	{
		osd_printf_error("%s WRONG LENGTH (expected: %d found: %d)\n", name, explength, actlength);
		retVal++;
	}

	// If there is no good dump known, write it
	util::hash_collection &acthashes = file->hashes(hashes.hash_types().c_str());
	if (hashes.flag(util::hash_collection::FLAG_NO_DUMP))
	{
		osd_printf_error("%s NO GOOD DUMP KNOWN\n", name);
	}
	// verify checksums
	else if (hashes != acthashes)
	{
		// otherwise, it's just bad
		osd_printf_error("%s WRONG CHECKSUMS:\n", name);
		dump_wrong_and_correct_checksums(hashes, acthashes);
		retVal++;
	}
	// If it matches, but it is actually a bad dump, write it
	else if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
	{
		osd_printf_error("%s NEEDS REDUMP\n",name);
	}
	return retVal;
}


//-------------------------------------------------
//  load_software - software image loading
//-------------------------------------------------

bool device_image_interface::load_software(software_list_device &swlist, const char *swname, const rom_entry *start)
{
	std::string locationtag, breakstr("%");
	const rom_entry *region;
	bool retVal = false;
	int warningcount = 0;
	for (region = start; region != nullptr; region = rom_next_region(region))
	{
		// loop until we hit the end of this region
		const rom_entry *romp = region + 1;
		while (!ROMENTRY_ISREGIONEND(romp))
		{
			// handle files
			if (ROMENTRY_ISFILE(romp))
			{
				osd_file::error filerr = osd_file::error::NOT_FOUND;

				u32 crc = 0;
				bool has_crc = util::hash_collection(ROM_GETHASHDATA(romp)).crc(crc);

				const software_info *swinfo = swlist.find(swname);
				if (swinfo == nullptr)
					return false;

				u32 supported = swinfo->supported();
				if (supported == SOFTWARE_SUPPORTED_PARTIAL)
					osd_printf_error("WARNING: support for software %s (in list %s) is only partial\n", swname, swlist.list_name().c_str());
				if (supported == SOFTWARE_SUPPORTED_NO)
					osd_printf_error("WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name().c_str());

				// attempt reading up the chain through the parents and create a locationtag std::string in the format
				// " swlist % clonename % parentname "
				// below, we have the code to split the elements and to create paths to load from

				while (swinfo != nullptr)
				{
					locationtag.append(swinfo->shortname()).append(breakstr);
					swinfo = !swinfo->parentname().empty() ? swlist.find(swinfo->parentname().c_str()) : nullptr;
				}
				// strip the final '%'
				locationtag.erase(locationtag.length() - 1, 1);


				// check if locationtag actually contains two locations separated by '%'
				// (i.e. check if we are dealing with a clone in softwarelist)
				std::string tag2, tag3, tag4(locationtag), tag5;
				int separator = tag4.find_first_of('%');
				if (separator != -1)
				{
					// we are loading a clone through softlists, split the setname from the parentname
					tag5.assign(tag4.substr(separator + 1, tag4.length() - separator + 1));
					tag4.erase(separator, tag4.length() - separator);
				}

				// prepare locations where we have to load from: list/parentname & list/clonename
				std::string tag1(swlist.list_name());
				tag1.append(PATH_SEPARATOR);
				tag2.assign(tag1.append(tag4));
				tag1.assign(swlist.list_name());
				tag1.append(PATH_SEPARATOR);
				tag3.assign(tag1.append(tag5));

				if (tag5.find_first_of('%') != -1)
					fatalerror("We do not support clones of clones!\n");

				// try to load from the available location(s):
				// - if we are not using lists, we have regiontag only;
				// - if we are using lists, we have: list/clonename, list/parentname, clonename, parentname
				// try to load from list/setname
				if ((m_mame_file == nullptr) && (tag2.c_str() != nullptr))
					m_mame_file = common_process_file(device().machine().options(), tag2.c_str(), has_crc, crc, romp, filerr);
				// try to load from list/parentname
				if ((m_mame_file == nullptr) && (tag3.c_str() != nullptr))
					m_mame_file = common_process_file(device().machine().options(), tag3.c_str(), has_crc, crc, romp, filerr);
				// try to load from setname
				if ((m_mame_file == nullptr) && (tag4.c_str() != nullptr))
					m_mame_file = common_process_file(device().machine().options(), tag4.c_str(), has_crc, crc, romp, filerr);
				// try to load from parentname
				if ((m_mame_file == nullptr) && (tag5.c_str() != nullptr))
					m_mame_file = common_process_file(device().machine().options(), tag5.c_str(), has_crc, crc, romp, filerr);

				warningcount += verify_length_and_hash(m_mame_file.get(),ROM_GETNAME(romp),ROM_GETLENGTH(romp), util::hash_collection(ROM_GETHASHDATA(romp)));

				if (filerr == osd_file::error::NONE)
					filerr = util::core_file::open_proxy(*m_mame_file, m_file);
				if (filerr == osd_file::error::NONE)
					retVal = true;

				break; // load first item for start
			}
			romp++; /* something else; skip */
		}
	}
	if (warningcount > 0)
	{
		osd_printf_error("WARNING: the software item might not run correctly.\n");
	}
	return retVal;
}


//-------------------------------------------------
//  load_internal - core image loading
//-------------------------------------------------

image_init_result device_image_interface::load_internal(const std::string &path, bool is_create, int create_format, util::option_resolution *create_args)
{
	// first unload the image
	unload();

	// clear any possible error messages
	clear_error();

	// we are now loading
	m_is_loading = true;

	// record the filename
	set_image_filename(path);

	if (core_opens_image_file())
	{
		// determine open plan
		std::vector<u32> open_plan = determine_open_plan(is_create);

		// attempt to open the file in various ways
		for (auto iter = open_plan.cbegin(); !m_file && iter != open_plan.cend(); iter++)
		{
			// open the file
			m_err = load_image_by_path(*iter, path);
			if (m_err && (m_err != IMAGE_ERROR_FILENOTFOUND))
				goto done;
		}

		// did we fail to find the file?
		if (!is_loaded())
		{
			m_err = IMAGE_ERROR_FILENOTFOUND;
			goto done;
		}
	}

	// call device load or create
	m_create_format = create_format;
	m_create_args = create_args;

	if (init_phase()==false) {
		m_err = (finish_load() == image_init_result::PASS) ? IMAGE_ERROR_SUCCESS : IMAGE_ERROR_INTERNAL;
		if (m_err)
			goto done;
	}
	// success!

done:
	if (m_err!=0) {
		if (!init_phase())
		{
			if (device().machine().phase() == machine_phase::RUNNING)
				device().popmessage("Error: Unable to %s image '%s': %s", is_create ? "create" : "load", path, error());
			else
				osd_printf_error("Error: Unable to %s image '%s': %s\n", is_create ? "create" : "load", path.c_str(), error());
		}
		clear();
	}
	return m_err ? image_init_result::FAIL : image_init_result::PASS;
}


//-------------------------------------------------
//  load - load an image into MAME
//-------------------------------------------------

image_init_result device_image_interface::load(const std::string &path)
{
	// is this a reset on load item?
	if (is_reset_on_load() && !init_phase())
	{
		reset_and_load(path);
		return image_init_result::PASS;
	}

	return load_internal(path, false, 0, nullptr);
}


//-------------------------------------------------
//  load_software - loads a softlist item by name
//-------------------------------------------------

image_init_result device_image_interface::load_software(const std::string &software_identifier)
{
	// Is this a software part that forces a reset and we're at runtime?  If so, get this loaded through reset_and_load
	if (is_reset_on_load() && !init_phase())
	{
		reset_and_load(software_identifier);
		return image_init_result::PASS;
	}

	// Prepare to load
	unload();
	clear_error();
	m_is_loading = true;

	// Check if there's a software list defined for this device and use that if we're not creating an image
	bool softload = load_software_part(software_identifier);
	if (!softload)
	{
		m_is_loading = false;
		return image_init_result::FAIL;
	}

	// set up softlist stuff
	m_full_software_name = m_software_part_ptr->info().shortname();

	// specify image name with softlist-derived names
	m_image_name = m_full_software_name;
	m_basename = m_full_software_name;
	m_basename_noext = m_full_software_name;
	m_filetype = use_software_list_file_extension_for_filetype() && m_mame_file != nullptr
		? core_filename_extract_extension(m_mame_file->filename(), true)
		: "";

	// Copy some image information when we have been loaded through a software list
	software_info &swinfo = m_software_part_ptr->info();

	// sanitize
	if (swinfo.longname().empty() || swinfo.publisher().empty() || swinfo.year().empty())
		fatalerror("Each entry in an XML list must have all of the following fields: description, publisher, year!\n");

	// store
	m_longname = swinfo.longname();
	m_manufacturer = swinfo.publisher();
	m_year = swinfo.year();

	// set file type
	std::string filename = (m_mame_file != nullptr) && (m_mame_file->filename() != nullptr)
			? m_mame_file->filename()
			: "";
	m_filetype = core_filename_extract_extension(filename, true);

	// call finish_load if necessary
	if (init_phase() == false && (finish_load() != image_init_result::PASS))
		return image_init_result::FAIL;

	return image_init_result::PASS;
}


//-------------------------------------------------
//  image_finish_load - special call - only use
//  from core
//-------------------------------------------------

image_init_result device_image_interface::finish_load()
{
	image_init_result err = image_init_result::PASS;

	if (m_is_loading)
	{
		if (!image_checkhash())
		{
			m_err = IMAGE_ERROR_INVALIDIMAGE;
			err = image_init_result::FAIL;
		}

		if (err == image_init_result::PASS)
		{
			if (m_created)
			{
				err = call_create(m_create_format, m_create_args);
				if (err != image_init_result::PASS)
				{
					if (!m_err)
						m_err = IMAGE_ERROR_UNSPECIFIED;
				}
			}
			else
			{
				// using device load
				err = call_load();
				if (err != image_init_result::PASS)
				{
					if (!m_err)
						m_err = IMAGE_ERROR_UNSPECIFIED;
				}
			}
		}
	}
	m_is_loading = false;
	m_create_format = 0;
	m_create_args = nullptr;
	return err;
}


//-------------------------------------------------
//  create - create a image
//-------------------------------------------------

image_init_result device_image_interface::create(const std::string &path)
{
	return create(path, nullptr, nullptr);
}


//-------------------------------------------------
//  create - create a image
//-------------------------------------------------

image_init_result device_image_interface::create(const std::string &path, const image_device_format *create_format, util::option_resolution *create_args)
{
	int format_index = 0;
	int cnt = 0;
	for (auto &format : m_formatlist)
	{
		if (create_format == format.get()) {
			format_index = cnt;
			break;
		}
		cnt++;
	}
	return load_internal(path, true, format_index, create_args);
}


//-------------------------------------------------
//  reset_and_load - called internally when we try
//  to load an is_reset_on_load() item; will reset
//  the emulation and record this image to be loaded
//-------------------------------------------------

void device_image_interface::reset_and_load(const std::string &path)
{
	// first make sure the reset is scheduled
	device().machine().schedule_hard_reset();

	// and record the new load
	device().machine().options().image_option(instance_name()).specify(path);

	// record that we're reset and loading
	m_is_reset_and_loading = true;
}


//-------------------------------------------------
//  clear - clear all internal data pertaining
//  to an image
//-------------------------------------------------

void device_image_interface::clear()
{
	m_mame_file.reset();
	m_file.reset();

	m_image_name.clear();
	m_readonly = false;
	m_created = false;
	m_create_format = 0;
	m_create_args = nullptr;

	m_longname.clear();
	m_manufacturer.clear();
	m_year.clear();
	m_basename.clear();
	m_basename_noext.clear();
	m_filetype.clear();

	m_full_software_name.clear();
	m_software_part_ptr = nullptr;
	m_software_list_name.clear();
}


//-------------------------------------------------
//  unload - main call to unload an image
//-------------------------------------------------

void device_image_interface::unload()
{
	if (is_loaded() || loaded_through_softlist())
	{
		call_unload();
	}
	clear();
	clear_error();
}


//-------------------------------------------------
//  create_option_guide
//-------------------------------------------------

OPTION_GUIDE_START(null_option_guide)
OPTION_GUIDE_END

const util::option_guide &device_image_interface::create_option_guide() const
{
	return null_option_guide;
}

//-------------------------------------------------
//  update_names - update brief and instance names
//-------------------------------------------------

void device_image_interface::update_names()
{
	const char *inst_name = custom_instance_name();
	const char *brief_name = custom_brief_instance_name();
	if (inst_name == nullptr)
		inst_name = device_typename(image_type());
	if (brief_name == nullptr)
		brief_name = device_brieftypename(image_type());

	// count instances of the general image type, or device type if custom
	int count = 0;
	int index = -1;
	for (const device_image_interface &image : image_interface_iterator(device().mconfig().root_device()))
	{
		if (this == &image)
			index = count;
		const char *other_name = image.custom_instance_name();
		if (!other_name)
			other_name = device_typename(image.image_type());

		if (other_name == inst_name || !strcmp(other_name, inst_name))
			count++;
	}

	m_canonical_instance_name = string_format("%s%d", inst_name, index + 1);
	if (count > 1)
	{
		m_instance_name = m_canonical_instance_name;
		m_brief_instance_name = string_format("%s%d", brief_name, index + 1);
	}
	else
	{
		m_instance_name = inst_name;
		m_brief_instance_name = brief_name;
	}
}

//-------------------------------------------------
//  find_software_item
//-------------------------------------------------

const software_part *device_image_interface::find_software_item(const std::string &identifier, bool restrict_to_interface, software_list_device **dev) const
{
	// split full software name into software list name and short software name
	std::string list_name, software_name, part_name;
	if (!software_name_parse(identifier, &list_name, &software_name, &part_name))
		return nullptr;

	// determine interface
	const char *interface = restrict_to_interface
		? image_interface()
		: nullptr;

	// find the software list if explicitly specified
	for (software_list_device &swlistdev : software_list_device_iterator(device().mconfig().root_device()))
	{
		if (list_name.empty() || (list_name == swlistdev.list_name()))
		{
			const software_info *info = swlistdev.find(software_name);
			if (info != nullptr)
			{
				const software_part *part = info->find_part(part_name, interface);
				if (part != nullptr)
				{
					if (dev != nullptr)
						*dev = &swlistdev;
					return part;
				}
			}
		}

		if (software_name == swlistdev.list_name())
		{
			// ad hoc handling for the case path = swlist_name:swinfo_name (e.g.
			// gameboy:sml) which is not handled properly by software_name_split
			// since the function cannot distinguish between this and the case
			// path = swinfo_name:swpart_name
			const software_info *info = swlistdev.find(part_name);
			if (info != nullptr)
			{
				const software_part *part = info->find_part("", interface);
				if (part != nullptr)
				{
					if (dev != nullptr)
						*dev = &swlistdev;
					return part;
				}
			}
		}
	}

	// if explicitly specified and not found, just error here
	if (dev != nullptr)
		*dev = nullptr;
	return nullptr;
}


//-------------------------------------------------
//  get_software_list_loader
//-------------------------------------------------

const software_list_loader &device_image_interface::get_software_list_loader() const
{
	return false_software_list_loader::instance();
}


//-------------------------------------------------
//  load_software_part
//
//  Load a software part for a device. The part to
//  load is determined by the "path", software lists
//  configured for a driver, and the interface
//  supported by the device.
//
//  returns true if the software could be loaded,
//  false otherwise. If the software could be loaded
//  sw_info and sw_part are also set.
//-------------------------------------------------

bool device_image_interface::load_software_part(const std::string &identifier)
{
	// if no match has been found, we suggest similar shortnames
	software_list_device *swlist;
	m_software_part_ptr = find_software_item(identifier, true, &swlist);
	if (m_software_part_ptr == nullptr)
	{
		software_list_device::display_matches(device().machine().config(), image_interface(), identifier);
		return false;
	}

	// Load the software part
	const char *swname = m_software_part_ptr->info().shortname().c_str();
	const rom_entry *start_entry = m_software_part_ptr->romdata().data();
	const software_list_loader &loader = get_software_list_loader();
	bool result = loader.load_software(*this, *swlist, swname, start_entry);

	// check compatibility
	switch (swlist->is_compatible(*m_software_part_ptr))
	{
		case SOFTWARE_IS_COMPATIBLE:
			break;

		case SOFTWARE_IS_INCOMPATIBLE:
			swlist->popmessage("WARNING! the set %s might not work on this system due to incompatible filter(s) '%s'\n", m_software_part_ptr->info().shortname(), swlist->filter());
			break;

		case SOFTWARE_NOT_COMPATIBLE:
			swlist->popmessage("WARNING! the set %s might not work on this system due to missing filter(s) '%s'\n", m_software_part_ptr->info().shortname(), swlist->filter());
			break;
	}

	// check requirements and load those images
	const char *requirement = m_software_part_ptr->feature("requirement");
	if (requirement != nullptr)
	{
		const software_part *req_swpart = find_software_item(requirement, false);
		if (req_swpart != nullptr)
		{
			device_image_interface *req_image = software_list_device::find_mountable_image(device().mconfig(), *req_swpart);
			if (req_image != nullptr)
				req_image->load_software(requirement);
		}
	}

	m_software_list_name = swlist->list_name();
	return result;
}

//-------------------------------------------------
//  software_get_default_slot
//-------------------------------------------------

std::string device_image_interface::software_get_default_slot(const char *default_card_slot) const
{
	std::string result;

	const std::string &image_name(device().mconfig().options().image_option(instance_name()).value());
	if (!image_name.empty())
	{
		result.assign(default_card_slot);
		const software_part *swpart = find_software_item(image_name, true);
		if (swpart != nullptr)
		{
			const char *slot = swpart->feature("slot");
			if (slot != nullptr)
				result.assign(slot);
		}
	}
	return result;
}


//-------------------------------------------------
//  init_phase
//-------------------------------------------------

bool device_image_interface::init_phase() const
{
	// diimage.cpp has quite a bit of logic that randomly decides to behave
	// differently at startup; this is an enc[r]apsulation of the "logic"
	// that switches these behaviors
	return !device().has_running_machine()
		|| device().machine().phase() == machine_phase::INIT;
}


//----------------------------------------------------------------------------

static int image_fseek_thunk(void *file, s64 offset, int whence)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fseek(offset, whence);
}

static size_t image_fread_thunk(void *file, void *buffer, size_t length)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fread(buffer, length);
}

static size_t image_fwrite_thunk(void *file, const void *buffer, size_t length)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->fwrite(buffer, length);
}

static u64 image_fsize_thunk(void *file)
{
	device_image_interface *image = (device_image_interface *) file;
	return image->length();
}

//----------------------------------------------------------------------------

struct io_procs image_ioprocs =
{
	nullptr,
	image_fseek_thunk,
	image_fread_thunk,
	image_fwrite_thunk,
	image_fsize_thunk
};
