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

#include "corestr.h"
#include "opresolv.h"
#include "path.h"
#include "zippath.h"

#include <algorithm>
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
	, m_file()
	, m_mame_file()
	, m_default_region(-1)
	, m_current_region(-1)
	, m_software_part_ptr(nullptr)
	, m_sequence_counter(0)
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
//  add_region - register a region that may
//  have a chd image
//-------------------------------------------------
void device_image_interface::add_region(std::string name, bool is_default)
{
	if (is_default)
		m_default_region = m_possible_preset_regions.size();
	m_possible_preset_regions.push_back(name);
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
//  check_preset_images - lookup the chds from the
//  region(s), if any
//-------------------------------------------------

void device_image_interface::check_preset_images()
{
	if (!m_possible_preset_regions.empty())
	{
		for(const auto &r : m_possible_preset_regions)
			m_preset_images.push_back(device().machine().rom_load().get_disk_handle(":" + r));
		if (m_default_region != -1 && m_preset_images[m_default_region])
			m_current_region = m_default_region;
		else
		{
			for (m_current_region = 0; m_current_region != int(m_preset_images.size()) && !m_preset_images[m_current_region]; m_current_region++);
			if (m_current_region == int(m_preset_images.size()))
				fatalerror("%s: No configured region has an image\n", device().tag());
		}
		set_image_tag();
		set_user_loadable(false);
	}
	else
	{
		std::string tag = device().owner()->tag();
		auto *chd = device().machine().rom_load().get_disk_handle(tag);
		if (chd)
		{
			m_possible_preset_regions.push_back(tag);
			m_preset_images.push_back(chd);
			m_current_region = 0;
			set_image_tag();
			set_user_loadable(false);
		}
	}
}

//-------------------------------------------------
//  has_preset_images - does the device have an
//  image to retrieve through current_image_*
//-------------------------------------------------

bool device_image_interface::has_preset_images() const
{
	return !m_possible_preset_regions.empty();
}

//-------------------------------------------------
//  has_preset_images - does the device have
//  multiple preset images with user selection
//-------------------------------------------------

bool device_image_interface::has_preset_images_selection() const
{
	int icount = 0;
	for (const auto *f : m_preset_images)
		if (f)
			icount ++;
	return icount > 1;
}


//-------------------------------------------------
//  preset_images_list -- generate the list of
//  available image names
//-------------------------------------------------

std::vector<std::string> device_image_interface::preset_images_list() const
{
	std::vector<std::string> result;
	for (unsigned int i = 0; i != m_preset_images.size(); i++)
		if (m_preset_images[i])
			result.push_back(m_possible_preset_regions[i]);
	return result;
}

//-------------------------------------------------
//  current_preset_image_id -- current image id,
//  recomputed to ignore non-present images.
//  returns -1 if not in preset mode
//-------------------------------------------------

int device_image_interface::current_preset_image_id() const
{
	if (m_current_region == -1)
		return -1;
	int id = 0;
	for (int i = 0; i != m_current_region; i++)
		if (m_preset_images[i])
			id++;
	return id;
}

//-------------------------------------------------
//  current_preset_image_chd -- return the chd of
//  the current active image, nullptr if non
//-------------------------------------------------

chd_file *device_image_interface::current_preset_image_chd() const
{
	if (m_current_region == -1)
		return nullptr;
	return m_preset_images[m_current_region];
}

//-------------------------------------------------
//  switch_preset_image -- change of preset image
//-------------------------------------------------

void device_image_interface::switch_preset_image(int id)
{
	for (unsigned int i = 0; i != m_preset_images.size(); i++)
		if (m_preset_images[i])
		{
			if(!id)
			{
				call_unload();
				m_current_region = i;
				call_load();
				break;
			}
			id--;
		}

	return;
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
//  set_image_tag - specifies the filename of
//  an image as the device tag
//-------------------------------------------------

void device_image_interface::set_image_tag()
{
	m_image_name = device().owner()->tag();
	m_working_directory = "";
	m_basename = "";
	m_basename_noext = "";
	m_filetype = "";
}


//-------------------------------------------------
//  is_filetype - check if the filetype matches
//-------------------------------------------------

bool device_image_interface::is_filetype(std::string_view candidate_filetype) const
{
	return util::streqlower(m_filetype, candidate_filetype);
}


//***************************************************************************
//  CREATION FORMATS
//***************************************************************************

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


//***************************************************************************
//  ERROR HANDLING
//***************************************************************************

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
					"Invalid image length"sv,
					"File already open"sv,
					"Unrecognized software item"sv,
					"Invalid software item"sv,
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

std::error_condition device_image_interface::load_software_region(std::string_view tag, std::unique_ptr<u8[]> &ptr)
{
	size_t size = get_software_region_length(tag);

	if (size)
	{
		ptr = std::make_unique<u8[]>(size);
		memcpy(ptr.get(), get_software_region(tag), size);
		return std::error_condition();
	}
	else
		return image_error::UNSUPPORTED;
}


// ****************************************************************************
// Hash info loading
//
// If the hash is not checked and the relevant info not loaded, force that info
// to be loaded
// ****************************************************************************

std::error_condition device_image_interface::run_hash(util::random_read &file, u32 skip_bytes, util::hash_collection &hashes, const char *types)
{
	// reset the hash; we want to override existing data
	hashes.reset();

	// figure out the size, and "cap" the skip bytes
	u64 size;
	std::error_condition filerr = file.length(size);
	if (filerr)
		return filerr;
	skip_bytes = u32(std::min<u64>(skip_bytes, size));

	// and compute the hashes
	size_t actual_count;
	filerr = hashes.compute(file, skip_bytes, size - skip_bytes, actual_count, types);
	if (filerr)
		return filerr;

	return std::error_condition();
}



std::error_condition device_image_interface::image_checkhash()
{
	// only calculate CRC if it hasn't been calculated, and the open_mode is read only
	u32 crcval;
	if (!m_hash.crc(crcval) && is_readonly() && !m_created)
	{
		// do not cause a linear read of 600 megs please
		// TODO: use SHA1 in the CHD header as the hash
		if (image_is_chd_type())
			return std::error_condition();

		// Skip calculating the hash when we have an image mounted through a software list
		if (loaded_through_softlist())
			return std::error_condition();

		// run the hash
		return run_hash(*m_file, unhashed_header_length(), m_hash, util::hash_collection::HASH_TYPES_ALL);
	}
	return std::error_condition();
}


util::hash_collection device_image_interface::calculate_hash_on_file(util::random_read &file) const
{
	// calculate the hash
	util::hash_collection hash;
	if (run_hash(file, unhashed_header_length(), hash, util::hash_collection::HASH_TYPES_ALL))
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

	if (!is_create)
	{
		if (is_writeable())
			open_plan.push_back(is_readable() ? (OPEN_FLAG_READ | OPEN_FLAG_WRITE) : OPEN_FLAG_WRITE);
		if (is_readable())
			open_plan.push_back(OPEN_FLAG_READ);
	}
	else if (is_writeable() && is_creatable())
	{
		if (is_readable())
			open_plan.push_back(OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
		else
			open_plan.push_back(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	}

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

std::error_condition device_image_interface::load_software(software_list_device &swlist, std::string_view swname, const rom_entry *start)
{
	std::error_condition retval;
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
					return image_error::NOSOFTWARE;

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
				if (filerr)
					retval = filerr;

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

std::pair<std::error_condition, std::string> device_image_interface::load_internal(std::string_view path, bool is_create, int create_format, util::option_resolution *create_args)
{
	std::pair<std::error_condition, std::string> err;

	// first unload the image
	unload();

	// we are now loading
	m_is_loading = true;
	m_sequence_counter ++;

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
			err.first = load_image_by_path(*iter, path);
			if (err.first && (err.first != std::errc::no_such_file_or_directory) && (err.first != std::errc::permission_denied))
				goto done;
		}

		// did we fail to find the file?
		if (!m_file)
		{
			err.first = std::errc::no_such_file_or_directory;
			goto done;
		}
	}

	// call device load or create
	m_create_format = create_format;
	m_create_args = create_args;

	if (!init_phase())
		err = finish_load();

done:
	if (err.first)
	{
		osd_printf_error(
				!err.second.empty()
					? (is_create ? "Unable to create image '%1$s': %2$s (%3$s:%4$d %5$s)\n" : "Unable to load image '%1$s': %2$s (%3$s:%4$d %5$s)\n")
					: (is_create ? "Unable to create image '%1$s': %5$s (%3$s:%4$d)\n" : "Unable to load image '%1$s': %5$s (%3$s:%4$d)\n"),
				path,
				err.second,
				err.first.category().name(),
				err.first.value(),
				err.first.message());
		clear();
	}
	return err;
}


//-------------------------------------------------
//  load - load an image into MAME
//-------------------------------------------------

std::pair<std::error_condition, std::string> device_image_interface::load(std::string_view path)
{
	// is this a reset on load item?
	if (is_reset_on_load() && !init_phase())
	{
		reset_and_load(path);
		return std::make_pair(std::error_condition(), std::string());
	}

	return load_internal(path, false, 0, nullptr);
}


//-------------------------------------------------
//  load_software - loads a softlist item by name
//-------------------------------------------------

std::pair<std::error_condition, std::string> device_image_interface::load_software(std::string_view software_identifier)
{
	// Is this a software part that forces a reset and we're at runtime?  If so, get this loaded through reset_and_load
	if (is_reset_on_load() && !init_phase())
	{
		reset_and_load(software_identifier);
		return std::make_pair(std::error_condition(), std::string());
	}

	// Prepare to load
	unload();
	m_is_loading = true;

	// Check if there's a software list defined for this device and use that if we're not creating an image
	std::error_condition err = load_software_part(software_identifier);
	if (err)
	{
		m_is_loading = false;
		return std::make_pair(err, std::string());
	}

	// set up softlist stuff
	m_full_software_name = m_software_part_ptr->info().shortname();

	// specify image name with softlist-derived names
	m_image_name = m_full_software_name;
	m_basename = m_full_software_name;
	m_basename_noext = m_full_software_name;
	if (use_software_list_file_extension_for_filetype() && m_mame_file)
		m_filetype = core_filename_extract_extension(m_mame_file->filename(), true);
	else
		m_filetype.clear();

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
	if (!init_phase())
		return finish_load();
	else
		return make_pair(std::error_condition(), std::string());
}


//-------------------------------------------------
//  image_finish_load - special call - only use
//  from core
//-------------------------------------------------

std::pair<std::error_condition, std::string> device_image_interface::finish_load()
{
	std::pair<std::error_condition, std::string> err;

	if (m_is_loading)
	{
		err.first = image_checkhash();

		if (!err.first)
		{
			if (m_created)
				err = call_create(m_create_format, m_create_args);
			else
				err = call_load(); // using device load
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

std::pair<std::error_condition, std::string> device_image_interface::create(std::string_view path)
{
	return create(path, nullptr, nullptr);
}


//-------------------------------------------------
//  create - create a image
//-------------------------------------------------

std::pair<std::error_condition, std::string> device_image_interface::create(std::string_view path, const image_device_format *create_format, util::option_resolution *create_args)
{
	int format_index = 0;
	int cnt = 0;
	for (auto &format : m_formatlist)
	{
		if (create_format == format.get())
		{
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

void device_image_interface::clear() noexcept
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

	m_sequence_counter ++;
}


//-------------------------------------------------
//  unload - main call to unload an image
//-------------------------------------------------

void device_image_interface::unload()
{
	if (is_loaded() || loaded_through_softlist())
	{
		m_sequence_counter ++;
		call_unload();
	}
	clear();
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

std::error_condition device_image_interface::load_software_part(std::string_view identifier)
{
	// if no match has been found, we suggest similar shortnames
	software_list_device *swlist;
	m_software_part_ptr = find_software_item(identifier, true, &swlist);
	if (m_software_part_ptr == nullptr)
	{
		software_list_device::display_matches(device().machine().config(), image_interface(), identifier);
		return image_error::NOSOFTWARE;
	}

	// Load the software part
	const std::string &swname = m_software_part_ptr->info().shortname();
	const rom_entry *start_entry = m_software_part_ptr->romdata().data();
	const software_list_loader &loader = get_software_list_loader();
	std::error_condition result = loader.load_software(*this, *swlist, swname, start_entry);

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
