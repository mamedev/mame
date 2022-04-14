// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    diimage.cpp

    Device image interfaces.

***************************************************************************/

#include "emu.h"

#include "emuopts.h"
#include "fileio.h"
#include "romload.h"
#include "softlist.h"
#include "softlist_dev.h"

#include "ui/uimain.h"

#include "corestr.h"
#include "opresolv.h"
#include "zippath.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <regex>
#include <sstream>


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
	, m_readonly(false)
	, m_created(false)
	, m_create_format(0)
	, m_create_args(nullptr)
	, m_user_loadable(true)
	, m_must_be_loaded(false)
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
//  set_image_filename - specifies the filename of
//  an image
//-------------------------------------------------

void device_image_interface::set_image_filename(std::string_view filename)
{
	m_image_name = filename;
	m_working_directory = util::zippath_parent(m_image_name);
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


//-------------------------------------------------
//  is_filetype - check if the filetype matches
//-------------------------------------------------

bool device_image_interface::is_filetype(std::string_view candidate_filetype) const
{
	return std::equal(m_filetype.begin(), m_filetype.end(), candidate_filetype.begin(), candidate_filetype.end(),
						[] (unsigned char c1, unsigned char c2) { return std::tolower(c1) == c2; });
}


/****************************************************************************
    CREATION FORMATS
****************************************************************************/

//-------------------------------------------------
//  device_get_named_creatable_format -
//  accesses a specific image format available for
//  image creation by name
//-------------------------------------------------

const image_device_format *device_image_interface::device_get_named_creatable_format(std::string_view format_name) const noexcept
{
	for (const auto &format : m_formatlist)
		if (std::string_view(format->name()) == format_name)
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
//  clear_error - clear out any specified error
//-------------------------------------------------

void device_image_interface::clear_error() noexcept
{
	m_err.clear();
	m_err_message.clear();
}



//-------------------------------------------------
//  error - returns the error text for an image
//  error
//-------------------------------------------------

std::error_category const &image_category() noexcept
{
	class image_category_impl : public std::error_category
	{
	public:
		virtual char const *name() const noexcept override { return "image"; }

		virtual std::string message(int condition) const override
		{
			using namespace std::literals;
			static std::string_view const s_messages[] = {
					"No error"sv,
					"Internal error"sv,
					"Unsupported operation"sv,
					"Invalid image"sv,
					"File already open"sv,
					"Unspecified error"sv };
			if ((0 <= condition) && (std::size(s_messages) > condition))
				return std::string(s_messages[condition]);
			else
				return "Unknown error"s;
		}
	};
	static image_category_impl const s_image_category_instance;
	return s_image_category_instance;
}

std::string_view device_image_interface::error()
{
	if (m_err && m_err_message.empty())
		m_err_message = m_err.message();
	return m_err_message;
}



//-------------------------------------------------
//  seterror - specifies an error on an image
//-------------------------------------------------

void device_image_interface::seterror(std::error_condition err, const char *message)
{
	clear_error();
	m_err = err;
	if (message)
		m_err_message = message;
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
	vsnprintf(buffer, std::size(buffer), format, args);
	va_end(args);

	/* display the popup for a standard amount of time */
	device().machine().ui().popup_time(5, "%s: %s",
		basename(),
		buffer);
}


//-------------------------------------------------
//  software_entry - return a pointer to the
//  software_info structure from the softlist
//-------------------------------------------------

const software_info *device_image_interface::software_entry() const noexcept
{
	return !m_software_part_ptr ? nullptr : &m_software_part_ptr->info();
}


//-------------------------------------------------
//  get_software_region
//-------------------------------------------------

u8 *device_image_interface::get_software_region(std::string_view tag)
{
	if (!loaded_through_softlist())
		return nullptr;

	std::string full_tag = util::string_format("%s:%s", device().tag(), tag);
	memory_region *region = device().machine().root_device().memregion(full_tag);
	return region != nullptr ? region->base() : nullptr;
}


//-------------------------------------------------
//  image_get_software_region_length
//-------------------------------------------------

u32 device_image_interface::get_software_region_length(std::string_view tag)
{
	std::string full_tag = util::string_format("%s:%s", device().tag(), tag);
	memory_region *region = device().machine().root_device().memregion(full_tag);
	return region != nullptr ? region->bytes() : 0;
}


//-------------------------------------------------
//  image_get_feature
//-------------------------------------------------

const char *device_image_interface::get_feature(std::string_view feature_name) const
{
	return !m_software_part_ptr ? nullptr : m_software_part_ptr->feature(feature_name);
}


//-------------------------------------------------
//  load_software_region -
//-------------------------------------------------

bool device_image_interface::load_software_region(std::string_view tag, std::unique_ptr<u8[]> &ptr)
{
	size_t size = get_software_region_length(tag);

	if (size)
	{
		ptr = std::make_unique<u8[]>(size);
		memcpy(ptr.get(), get_software_region(tag), size);
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
	u64 size;
	if (file.length(size))
		return false;
	skip_bytes = u32(std::min<u64>(skip_bytes, size));

	// seek to the beginning
	file.seek(skip_bytes, SEEK_SET); // TODO: check error return
	u64 position = skip_bytes;

	// keep on reading hashes
	hashes.begin(types);
	while (position < size)
	{
		uint8_t buffer[8192];

		// read bytes
		const size_t count = size_t(std::min<u64>(size - position, sizeof(buffer)));
		size_t actual_count;
		const std::error_condition filerr = file.read(buffer, count, actual_count);
		if (filerr || !actual_count)
			return false;
		position += actual_count;

		// and compute the hashes
		hashes.buffer(buffer, actual_count);
	}
	hashes.end();

	// cleanup
	file.seek(0, SEEK_SET); // TODO: check error return
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
		if (image_is_chd_type())
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
	if (!m_hash.crc(crc))
		crc = 0;

	return crc;
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
	if (!buffer || (length <= 0))
		throw emu_fatalerror("device_image_interface::battery_load: Must specify sensical buffer/length");

	std::string const fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext).append(".nv");

	/* try to open the battery file and read it in, if possible */
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_READ);
	std::error_condition const filerr = file.open(fname);
	int bytes_read = 0;
	if (!filerr)
		bytes_read = file.read(buffer, length);

	// fill remaining bytes (if necessary)
	memset(((char *)buffer) + bytes_read, fill, length - bytes_read);
}

void device_image_interface::battery_load(void *buffer, int length, const void *def_buffer)
{
	if (!buffer || (length <= 0))
		throw emu_fatalerror("device_image_interface::battery_load: Must specify sensical buffer/length");

	std::string const fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext).append(".nv");

	// try to open the battery file and read it in, if possible
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_READ);
	std::error_condition const filerr = file.open(fname);
	int bytes_read = 0;
	if (!filerr)
		bytes_read = file.read(buffer, length);

	// if no file was present, copy the default contents
	if (!bytes_read && def_buffer)
		std::memcpy(buffer, def_buffer, length);
}


//-------------------------------------------------
//  battery_save - stores the battery
//  backed RAM for an image. The file name is
//  created from the machine driver name and the
//  image name.
//-------------------------------------------------

void device_image_interface::battery_save(const void *buffer, int length)
{
	if (!buffer || (length <= 0))
		throw emu_fatalerror("device_image_interface::battery_save: Must specify sensical buffer/length");

	if (!device().machine().options().nvram_save())
		return;

	std::string fname = std::string(device().machine().system().name).append(PATH_SEPARATOR).append(m_basename_noext).append(".nv");

	// try to open the battery file and write it out, if possible
	emu_file file(device().machine().options().nvram_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	std::error_condition const filerr = file.open(fname);
	if (!filerr)
		file.write(buffer, length);
}


// ***************************************************************************
// IMAGE LOADING
// ***************************************************************************

//-------------------------------------------------
//  load_image_by_path - loads an image with a
//  specific path
//-------------------------------------------------

std::error_condition device_image_interface::load_image_by_path(u32 open_flags, std::string_view path)
{
	std::string revised_path;

	// attempt to read the file
	auto filerr = util::zippath_fopen(path, open_flags, m_file, revised_path);
	if (filerr)
	{
		osd_printf_verbose("%s: error opening image file %s with flags=%08X (%s:%d %s)\n", device().tag(), path, open_flags, filerr.category().name(), filerr.value(), filerr.message());
		return filerr;
	}
	else
	{
		osd_printf_verbose("%s: opened image file %s with flags=%08X\n", device().tag(), path, open_flags);
	}

	m_readonly = (open_flags & OPEN_FLAG_WRITE) ? 0 : 1;
	m_created = (open_flags & OPEN_FLAG_CREATE) ? 1 : 0;
	set_image_filename(revised_path);
	return std::error_condition();
}


//-------------------------------------------------
//  reopen_for_write
//-------------------------------------------------

std::error_condition device_image_interface::reopen_for_write(std::string_view path)
{
	m_file.reset();

	std::string revised_path;

	// attempt to open the file for writing
	auto const filerr = util::zippath_fopen(path, OPEN_FLAG_READ|OPEN_FLAG_WRITE|OPEN_FLAG_CREATE, m_file, revised_path);
	if (filerr)
		return filerr;

	// success!
	m_readonly = 0;
	m_created = 1;
	set_image_filename(revised_path);

	return std::error_condition();
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
//  verify_length_and_hash - verify the length
//  and hash signatures of a file
//-------------------------------------------------

static int verify_length_and_hash(emu_file *file, std::string_view name, u32 explength, const util::hash_collection &hashes)
{
	int retval = 0;
	if (!file)
		return 0;

	// verify length
	u32 actlength = file->size();
	if (explength != actlength)
	{
		osd_printf_error("%s WRONG LENGTH (expected: %d found: %d)\n", name, explength, actlength);
		retval++;
	}

	util::hash_collection &acthashes = file->hashes(hashes.hash_types());
	if (hashes.flag(util::hash_collection::FLAG_NO_DUMP))
	{
		// If there is no good dump known, write it
		osd_printf_error("%s NO GOOD DUMP KNOWN\n", name);
	}
	else if (hashes != acthashes)
	{
		// otherwise, it's just bad
		osd_printf_error("%s WRONG CHECKSUMS:\n", name);
		osd_printf_error("    EXPECTED: %s\n", hashes.macro_string());
		osd_printf_error("       FOUND: %s\n", acthashes.macro_string());
		retval++;
	}
	else if (hashes.flag(util::hash_collection::FLAG_BAD_DUMP))
	{
		// If it matches, but it is actually a bad dump, write it
		osd_printf_error("%s NEEDS REDUMP\n",name);
	}
	return retval;
}


//-------------------------------------------------
//  load_software - software image loading
//-------------------------------------------------

bool device_image_interface::load_software(software_list_device &swlist, std::string_view swname, const rom_entry *start)
{
	bool retval = false;
	int warningcount = 0;
	for (const rom_entry *region = start; region; region = rom_next_region(region))
	{
		// loop until we hit the end of this region
		for (const rom_entry *romp = region + 1; !ROMENTRY_ISREGIONEND(romp); romp++)
		{
			// handle files
			if (ROMENTRY_ISFILE(romp))
			{
				const software_info *const swinfo = swlist.find(std::string(swname));
				if (!swinfo)
					return false;

				if (swinfo->supported() == software_support::PARTIALLY_SUPPORTED)
					osd_printf_error("WARNING: support for software %s (in list %s) is only partial\n", swname, swlist.list_name());
				else if (swinfo->supported() == software_support::UNSUPPORTED)
					osd_printf_error("WARNING: support for software %s (in list %s) is only preliminary\n", swname, swlist.list_name());

				u32 crc = 0;
				const bool has_crc = util::hash_collection(romp->hashdata()).crc(crc);
				std::vector<const software_info *> parents;
				std::vector<std::string> searchpath = rom_load_manager::get_software_searchpath(swlist, *swinfo);

				// for historical reasons, add the search path for the software list device's owner
				const device_t *const listowner = swlist.owner();
				if (listowner)
				{
					std::vector<std::string> devsearch = listowner->searchpath();
					for (std::string &path : devsearch)
						searchpath.emplace_back(std::move(path));
				}

				// try to load the file
				m_mame_file.reset(new emu_file(device().machine().options().media_path(), searchpath, OPEN_FLAG_READ));
				m_mame_file->set_restrict_to_mediapath(1);
				std::error_condition filerr;
				if (has_crc)
					filerr = m_mame_file->open(romp->name(), crc);
				else
					filerr = m_mame_file->open(romp->name());
				if (filerr)
				{
					m_mame_file.reset();
					std::ostringstream msg;
					util::stream_format(msg,
							"%s: error opening image file %s: %s (%s:%d)",
							device().tag(), romp->name(),
							filerr.message(),
							filerr.category().name(),
							filerr.value());
					if (!searchpath.empty())
					{
						msg << " (tried in";
						for (auto const &path : searchpath)
							msg << ' ' << path;
						msg << ')';
					}
					osd_printf_error("%s\n", std::move(msg).str());
				}

				warningcount += verify_length_and_hash(m_mame_file.get(), romp->name(), romp->get_length(), util::hash_collection(romp->hashdata()));

				if (!filerr)
					filerr = util::core_file::open_proxy(*m_mame_file, m_file);
				if (!filerr)
					retval = true;

				break; // load first item for start
			}
		}
	}

	if (warningcount > 0)
		osd_printf_error("WARNING: the software item might not run correctly.\n");

	return retval;
}


//-------------------------------------------------
//  load_internal - core image loading
//-------------------------------------------------

image_init_result device_image_interface::load_internal(std::string_view path, bool is_create, int create_format, util::option_resolution *create_args)
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
			if (m_err && (m_err != std::errc::no_such_file_or_directory) && (m_err != std::errc::permission_denied))
				goto done;
		}

		// did we fail to find the file?
		if (!m_file)
		{
			m_err = std::errc::no_such_file_or_directory;
			goto done;
		}
	}

	// call device load or create
	m_create_format = create_format;
	m_create_args = create_args;

	if (!init_phase())
	{
		m_err = (finish_load() == image_init_result::PASS) ? std::error_condition() : image_error::INTERNAL;
		if (m_err)
			goto done;
	}
	// success!

done:
	if (m_err)
	{
		if (!init_phase())
		{
			if (device().machine().phase() == machine_phase::RUNNING)
				device().popmessage("Error: Unable to %s image '%s': %s", is_create ? "create" : "load", path, error());
			else
				osd_printf_error("Error: Unable to %s image '%s': %s\n", is_create ? "create" : "load", path, error());
		}
		clear();
	}
	return m_err ? image_init_result::FAIL : image_init_result::PASS;
}


//-------------------------------------------------
//  load - load an image into MAME
//-------------------------------------------------

image_init_result device_image_interface::load(std::string_view path)
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

image_init_result device_image_interface::load_software(std::string_view software_identifier)
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
		? std::string(core_filename_extract_extension(m_mame_file->filename(), true))
		: "";

	// Copy some image information when we have been loaded through a software list
	software_info &swinfo = m_software_part_ptr->info();

	// sanitize
	if (swinfo.longname().empty() || swinfo.publisher().empty() || swinfo.year().empty())
		fatalerror("Each entry in an XML list must have all of the following fields: description, publisher, year!\n");

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
			m_err = image_error::INVALIDIMAGE;
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
						m_err = image_error::UNSPECIFIED;
				}
			}
			else
			{
				// using device load
				err = call_load();
				if (err != image_init_result::PASS)
				{
					if (!m_err)
						m_err = image_error::UNSPECIFIED;
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

image_init_result device_image_interface::create(std::string_view path)
{
	return create(path, nullptr, nullptr);
}


//-------------------------------------------------
//  create - create a image
//-------------------------------------------------

image_init_result device_image_interface::create(std::string_view path, const image_device_format *create_format, util::option_resolution *create_args)
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

void device_image_interface::reset_and_load(std::string_view path)
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

	m_basename.clear();
	m_basename_noext.clear();
	m_filetype.clear();

	m_full_software_name.clear();
	m_software_part_ptr = nullptr;
	m_software_list_name.clear();

	m_hash.reset();
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
	const char *inst_name = image_type_name();
	const char *brief_name = image_brief_type_name();
	assert(inst_name != nullptr);
	assert(brief_name != nullptr);

	// count instances of the general image type, or device type if custom
	int count = 0;
	int index = -1;
	for (const device_image_interface &image : image_interface_enumerator(device().mconfig().root_device()))
	{
		if (this == &image)
			index = count;
		const char *other_name = image.image_type_name();
		const char *other_brief_name = image.image_brief_type_name();
		assert(other_name != nullptr);
		assert(other_brief_name != nullptr);

		if (other_name == inst_name || !strcmp(other_name, inst_name) ||
			other_brief_name == brief_name || !strcmp(other_brief_name, brief_name))
			count++;
	}

	m_canonical_instance_name = util::string_format("%s%d", inst_name, index + 1);
	if (count > 1)
	{
		m_instance_name = m_canonical_instance_name;
		m_brief_instance_name = util::string_format("%s%d", brief_name, index + 1);
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

const software_part *device_image_interface::find_software_item(std::string_view identifier, bool restrict_to_interface, software_list_device **dev) const
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
	for (software_list_device &swlistdev : software_list_device_enumerator(device().mconfig().root_device()))
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

bool device_image_interface::load_software_part(std::string_view identifier)
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
	const std::string &swname = m_software_part_ptr->info().shortname();
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
			osd_printf_warning("WARNING: the set %s might not work on this system due to incompatible filter(s) '%s'\n", m_software_part_ptr->info().shortname(), swlist->filter());
			break;

		case SOFTWARE_NOT_COMPATIBLE:
			swlist->popmessage("WARNING! the set %s might not work on this system due to missing filter(s) '%s'\n", m_software_part_ptr->info().shortname(), swlist->filter());
			osd_printf_warning("WARNING! the set %s might not work on this system due to missing filter(s) '%s'\n", m_software_part_ptr->info().shortname(), swlist->filter());
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

std::string device_image_interface::software_get_default_slot(std::string_view default_card_slot) const
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
