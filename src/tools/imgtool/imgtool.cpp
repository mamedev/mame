// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    imgtool.cpp

    Core code for Imgtool

***************************************************************************/

#include "imgtool.h"
#include "charconv.h"
#include "filter.h"
#include "library.h"
#include "modules.h"

#include "formats/imageutl.h"

#include "corefile.h"
#include "corestr.h"
#include "opresolv.h"

#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>



/***************************************************************************
    GLOBALS
***************************************************************************/

static std::unique_ptr<imgtool::library> global_imgtool_library;

static int global_omit_untested;
static void (*global_warn)(const char *message);


void CLIB_DECL ATTR_PRINTF(1,2) logerror(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}

/***************************************************************************

    Imgtool initialization and basics

***************************************************************************/

//-------------------------------------------------
//  rtrim
//-------------------------------------------------

void rtrim(char *buf)
{
	size_t buflen;
	char *s;

	buflen = strlen(buf);
	if (buflen)
	{
		for (s = &buf[buflen-1]; s >= buf && (*s >= '\0') && isspace(*s); s--)
			*s = '\0';
	}
}


//-------------------------------------------------
//  strncpyz
//-------------------------------------------------

char *strncpyz(char *dest, const char *source, size_t len)
{
	char *s;
	if (len) {
		s = strncpy(dest, source, len - 1);
		dest[len-1] = '\0';
	}
	else {
		s = dest;
	}
	return s;
}


//-------------------------------------------------
//  extract_padded_string
//-------------------------------------------------

static std::string extract_padded_string(const char *source, size_t len, char pad)
{
	while ((len > 0) && (source[len - 1] == pad))
		len--;

	return std::string(source, len);
}


//-------------------------------------------------
//  extract_padded_filename - this is a common
//  enough scenario that it is justified to have
//  this in common code
//-------------------------------------------------

std::string extract_padded_filename(const char *source, size_t filename_length, size_t extension_length, char pad)
{
	std::string filename = extract_padded_string(source, filename_length, pad);
	std::string extension = extract_padded_string(source + filename_length, extension_length, pad);
	return extension.empty() ? filename : filename + "." + extension;
}


//-------------------------------------------------
//  markerrorsource - marks where an error source
//-------------------------------------------------

static imgtoolerr_t markerrorsource(imgtoolerr_t err)
{
	switch(err)
	{
		case IMGTOOLERR_OUTOFMEMORY:
		case IMGTOOLERR_UNEXPECTED:
		case IMGTOOLERR_BUFFERTOOSMALL:
			/* Do nothing */
			break;

		case IMGTOOLERR_FILENOTFOUND:
		case IMGTOOLERR_BADFILENAME:
			err = imgtoolerr_t(err | IMGTOOLERR_SRC_FILEONIMAGE);
			break;

		default:
			err = imgtoolerr_t(err | IMGTOOLERR_SRC_IMAGEFILE);
			break;
	}
	return err;
}

//-------------------------------------------------
//  internal_error - debug function for raising
//  internal errors
//-------------------------------------------------

static void internal_error(const imgtool_module *module, const char *message)
{
#ifdef MAME_DEBUG
	printf("%s: %s\n", module->name.c_str(), message);
#endif
}


//-------------------------------------------------
//  imgtool_init - initializes the imgtool core
//-------------------------------------------------

void imgtool_init(bool omit_untested, void (*warn)(const char *message))
{
	imgtoolerr_t err;
	err = imgtool_create_canonical_library(omit_untested, global_imgtool_library);
	assert(err == IMGTOOLERR_SUCCESS);
	if (err == IMGTOOLERR_SUCCESS)
	{
		global_imgtool_library->sort(imgtool::library::sort_type::DESCRIPTION);
	}
	global_omit_untested = omit_untested;
	global_warn = warn;
}



//-------------------------------------------------
//  imgtool_exit - closes out the imgtool core
//-------------------------------------------------

void imgtool_exit(void)
{
	if (global_imgtool_library)
		global_imgtool_library.reset();

	global_warn = nullptr;
}



//-------------------------------------------------
//  imgtool_find_module - looks up a module
//-------------------------------------------------

const imgtool_module *imgtool_find_module(const std::string &modulename)
{
	return global_imgtool_library->findmodule(modulename);
}


//-------------------------------------------------
//  imgtool_find_module - looks up a module
//-------------------------------------------------

const imgtool::library::modulelist &imgtool_get_modules()
{
	return global_imgtool_library->modules();
}


//-------------------------------------------------
//  imgtool_get_module_features - retrieves a
//  structure identifying this module's features
//  associated with an image
//-------------------------------------------------

imgtool_module_features imgtool_get_module_features(const imgtool_module *module)
{
	imgtool_module_features features;
	memset(&features, 0, sizeof(features));

	if (module->create)
		features.supports_create = 1;
	if (module->open)
		features.supports_open = 1;
	if (module->read_sector)
		features.supports_readsector = 1;
	if (module->write_sector)
		features.supports_writesector = 1;
	return features;
}



//-------------------------------------------------
//  imgtool_warn - issues a warning
//-------------------------------------------------

void imgtool_warn(const char *format, ...)
{
	va_list va;
	char buffer[2000];

	if (global_warn)
	{
		va_start(va, format);
		vsprintf(buffer, format, va);
		va_end(va);
		global_warn(buffer);
	}
}



//-------------------------------------------------
//  evaluate_module - evaluates a single file to
//  determine what module can best handle a file
//-------------------------------------------------

static imgtoolerr_t evaluate_module(const char *fname, const imgtool_module *module, float &result)
{
	imgtoolerr_t err;
	imgtool::image::ptr image;
	imgtool::partition::ptr partition;
	imgtool::directory::ptr imageenum;
	imgtool_dirent ent;
	float current_result;

	result = 0.0;

	err = imgtool::image::open(module, fname, OSD_FOPEN_READ, image);
	if (err)
		goto done;

	if (image)
	{
		current_result = module->open_is_strict ? 0.9 : 0.5;

		err = imgtool::partition::open(*image, 0, partition);
		if (err)
			goto done;

		err = imgtool::directory::open(*partition, "", imageenum);
		if (err)
			goto done;

		memset(&ent, 0, sizeof(ent));
		do
		{
			err = imageenum->get_next(ent);
			if (err)
				goto done;

			if (ent.corrupt)
				current_result = (current_result * 99 + 1.00f) / 100;
			else
				current_result = (current_result + 1.00f) / 2;
		}
		while(!ent.eof);

		result = current_result;
	}

done:
	if (ERRORCODE(err) == IMGTOOLERR_CORRUPTIMAGE)
		err = IMGTOOLERR_SUCCESS;
	return err;
}


//-------------------------------------------------
//  identify_file - attempts to determine the module
//  for any given image
//-------------------------------------------------

imgtoolerr_t imgtool::image::identify_file(const char *fname, imgtool_module **modules, size_t count)
{
	imgtoolerr_t err = IMGTOOLERR_SUCCESS;
	imgtool::library &library = *global_imgtool_library.get();
	imgtool_module *insert_module;
	imgtool_module *temp_module;
	size_t i = 0;
	const char *extension;
	float val, temp_val;
	std::unique_ptr<float[]> values;

	if (count <= 0)
		return IMGTOOLERR_UNEXPECTED;

	for (i = 0; i < count; i++)
		modules[i] = nullptr;
	if (count > 1)
		count--;        /* null terminate */

	try { values = std::make_unique<float[]>(count); }
	catch (std::bad_alloc const &) { return IMGTOOLERR_OUTOFMEMORY; }
	for (i = 0; i < count; i++)
		values[i] = 0.0;

	/* figure out the file extension, if any */
	extension = strrchr(fname, '.');
	if (extension)
		extension++;

	/* iterate through all modules */
	for (const auto &module : library.modules())
	{
		if (!extension || image_find_extension(module->extensions.c_str(), extension))
		{
			err = evaluate_module(fname, module.get(), val);
			if (err)
				return err;

			insert_module = module.get();
			for (i = 0; (val > 0.0f) && (i < count); i++)
			{
				if (val > values[i])
				{
					temp_val = values[i];
					temp_module = modules[i];
					values[i] = val;
					modules[i] = insert_module;
					val = temp_val;
					insert_module = temp_module;
				}
			}
		}
	}

	if (!modules[0])
		return imgtoolerr_t(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_IMAGEFILE);

	return IMGTOOLERR_SUCCESS;
}



//-------------------------------------------------
//  get_geometry - gets the geometry
//  of an image; note that this may disagree with
//  particular sectors; this is a common copy
//  protection scheme
//-------------------------------------------------

imgtoolerr_t imgtool::image::get_geometry(uint32_t *tracks, uint32_t *heads, uint32_t *sectors)
{
	uint32_t dummy;

	/* some sanitization, to make the callbacks easier to implement */
	if (!tracks)
		tracks = &dummy;
	if (!heads)
		heads = &dummy;
	if (!sectors)
		sectors = &dummy;
	*tracks = 0;
	*heads = 0;
	*sectors = 0;

	/* implemented? */
	if (!module().get_geometry)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return module().get_geometry(*this, tracks, heads, sectors);
}



//-------------------------------------------------
//  read_sector - reads a sector on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::read_sector(uint32_t track, uint32_t head,
	uint32_t sector, std::vector<uint8_t> &buffer)
{
	// implemented?
	if (!module().read_sector)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return module().read_sector(*this, track, head, sector, buffer);
}



//-------------------------------------------------
//  write_sector - writes a sector on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::write_sector(uint32_t track, uint32_t head,
	uint32_t sector, const void *buffer, size_t len)
{
	// implemented?
	if (!module().write_sector)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return module().write_sector(*this, track, head, sector, buffer, len);
}



//-------------------------------------------------
//  get_block_size - gets the size of a standard
//  block on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::get_block_size(uint32_t &length)
{
	// implemented?
	if (module().block_size == 0)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	length = module().block_size;
	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  read_block - reads a standard block on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::read_block(uint64_t block, void *buffer)
{
	// implemented?
	if (!module().read_block)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return module().read_block(*this, buffer, block);
}


//-------------------------------------------------
//  write_block - writes a standard block on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::write_block(uint64_t block, const void *buffer)
{
	// implemented?
	if (!module().write_block)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	return module().write_block(*this, buffer, block);
}


//-------------------------------------------------
//  clear_block - clears a standard block on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::clear_block(uint64_t block, uint8_t data)
{
	imgtoolerr_t err;
	uint8_t *block_data = nullptr;
	uint32_t length;

	err = get_block_size(length);
	if (err)
		goto done;

	block_data = (uint8_t*)malloc(length);
	if (!block_data)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}
	memset(block_data, data, length);

	err = write_block(block, block_data);
	if (err)
		goto done;

done:
	if (block_data)
		free(block_data);
	return err;
}


//-------------------------------------------------
//  list_partitions - lists the partitions on an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::list_partitions(std::vector<imgtool::partition_info> &partitions)
{
	imgtoolerr_t err;

	// clear out partitions first
	partitions.clear();

	// implemented?
	if (module().list_partitions)
	{
		// if so, call the module's callback
		err = module().list_partitions(*this, partitions);
		if (err)
			return err;
	}
	else
	{
		// default implementation
		partitions.emplace_back(module().imgclass, 0, ~0);
	}
	return IMGTOOLERR_SUCCESS;
}


/***************************************************************************

    Imgtool partition management

***************************************************************************/

//-------------------------------------------------
//  imgtool::partition ctor
//-------------------------------------------------

imgtool::partition::partition(imgtool::image &image, const imgtool_class &imgclass, int partition_index, uint64_t base_block, uint64_t block_count)
	: m_image(image)
	, m_base_block(base_block)
	, m_block_count(block_count)
	, m_imgclass(imgclass)
{
	// does this partition type have extra bytes?
	size_t extra_bytes_size = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES);
	if (extra_bytes_size > 0)
	{
		m_extra_bytes = std::make_unique<uint8_t[]>(extra_bytes_size);
		memset(m_extra_bytes.get(), 0, sizeof(m_extra_bytes.get()[0]) * extra_bytes_size);
	}

	m_directory_extra_bytes = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES);
	m_path_separator = (char)imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PATH_SEPARATOR);
	m_alternate_path_separator = (char)imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_ALTERNATE_PATH_SEPARATOR);
	m_prefer_ucase = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_PREFER_UCASE) ? 1 : 0;
	m_supports_creation_time = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME) ? 1 : 0;
	m_supports_lastmodified_time = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME) ? 1 : 0;
	m_supports_bootblock = imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK) ? 1 : 0;
	m_begin_enum = (imgtoolerr_t(*)(imgtool::directory &, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_BEGIN_ENUM);
	m_next_enum = (imgtoolerr_t(*)(imgtool::directory &, imgtool_dirent &)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_NEXT_ENUM);
	m_close_enum = (void(*)(imgtool::directory &)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_CLOSE_ENUM);
	m_free_space = (imgtoolerr_t(*)(imgtool::partition &, uint64_t *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_FREE_SPACE);
	m_read_file = (imgtoolerr_t(*)(imgtool::partition &, const char *, const char *, imgtool::stream &)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_READ_FILE);
	m_write_file = (imgtoolerr_t(*)(imgtool::partition &, const char *, const char *, imgtool::stream &, util::option_resolution *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_WRITE_FILE);
	m_delete_file = (imgtoolerr_t(*)(imgtool::partition &, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_DELETE_FILE);
	m_list_forks = (imgtoolerr_t(*)(imgtool::partition &, const char *, std::vector<imgtool::fork_entry> &forks)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_LIST_FORKS);
	m_create_dir = (imgtoolerr_t(*)(imgtool::partition &, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_CREATE_DIR);
	m_delete_dir = (imgtoolerr_t(*)(imgtool::partition &, const char *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_DELETE_DIR);
	m_list_attrs = (imgtoolerr_t(*)(imgtool::partition &, const char *, uint32_t *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_LIST_ATTRS);
	m_get_attrs = (imgtoolerr_t(*)(imgtool::partition &, const char *, const uint32_t *, imgtool_attribute *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_ATTRS);
	m_set_attrs = (imgtoolerr_t(*)(imgtool::partition &, const char *, const uint32_t *, const imgtool_attribute *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_SET_ATTRS);
	m_attr_name = (imgtoolerr_t(*)(uint32_t, const imgtool_attribute *, char *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_ATTR_NAME);
	m_get_iconinfo = (imgtoolerr_t(*)(imgtool::partition &, const char *, imgtool_iconinfo *)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_ICON_INFO);
	m_suggest_transfer = (imgtoolerr_t(*)(imgtool::partition &, const char *, imgtool_transfer_suggestion *, size_t))  imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_SUGGEST_TRANSFER);
	m_get_chain = (imgtoolerr_t(*)(imgtool::partition &, const char *, imgtool_chainent *, size_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_GET_CHAIN);
	m_writefile_optguide = (const util::option_guide *) imgtool_get_info_ptr(&imgclass, IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE);

	const char *writefile_optspec = (const char *)imgtool_get_info_ptr(&imgclass, IMGTOOLINFO_STR_WRITEFILE_OPTSPEC);
	if (writefile_optspec)
		m_writefile_optspec.assign(writefile_optspec);

	// mask out if writing is untested
	if (global_omit_untested && imgtool_get_info_int(&imgclass, IMGTOOLINFO_INT_WRITING_UNTESTED))
	{
		m_write_file = nullptr;
		m_delete_file = nullptr;
		m_create_dir = nullptr;
		m_delete_dir = nullptr;
		m_writefile_optguide = nullptr;
		m_writefile_optspec = nullptr;
	}
}


//-------------------------------------------------
//  imgtool::partition dtor
//-------------------------------------------------

imgtool::partition::~partition()
{
}


//-------------------------------------------------
//  open - opens a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::open(imgtool::image &image, int partition_index, imgtool::partition::ptr &partition)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	imgtool::partition::ptr p;
	std::vector<imgtool::partition_info> partitions;
	imgtoolerr_t (*open_partition)(imgtool::partition &partition, uint64_t first_block, uint64_t block_count);

	// list the partitions
	err = image.list_partitions(partitions);
	if (err)
		return err;

	// is this an invalid index?
	if ((partition_index < 0) || (partition_index >= partitions.size()) || (!partitions[partition_index].imgclass().get_info && !partitions[partition_index].imgclass().derived_get_info))
		return IMGTOOLERR_INVALIDPARTITION;

	// use this partition
	const imgtool_class &imgclass(partitions[partition_index].imgclass());
	uint64_t base_block = partitions[partition_index].base_block();
	uint64_t block_count = partitions[partition_index].block_count();

	// allocate the new partition object
	try { p = std::make_unique<imgtool::partition>(image, imgclass, partition_index, base_block, block_count); }
	catch (std::bad_alloc const &)
	{
		err = (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY;
		goto done;
	}

	// call the partition open function, if present
	open_partition = (imgtoolerr_t (*)(imgtool::partition &, uint64_t, uint64_t)) imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_OPEN_PARTITION);
	if (open_partition)
	{
		/* we have an open partition function */
		err = (*open_partition)(*p, base_block, block_count);
		if (err)
			goto done;
	}

done:
	if (!err)
		partition = std::move(p);
	else
		partition.reset();
	return err;
}



/***************************************************************************

    Imgtool partition operations

***************************************************************************/

//-------------------------------------------------
//  get_attribute_name - retrieves the human readable
//  name for an attribute
//-------------------------------------------------

void imgtool::partition::get_attribute_name(uint32_t attribute, const imgtool_attribute *attr_value,
	char *buffer, size_t buffer_len)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_UNIMPLEMENTED;

	buffer[0] = '\0';

	if (attr_value)
	{
		if (m_attr_name)
			err = m_attr_name(attribute, attr_value, buffer, buffer_len);

		if (err == (imgtoolerr_t)IMGTOOLERR_UNIMPLEMENTED)
		{
			switch(attribute & 0xF0000)
			{
				case IMGTOOLATTR_INT_FIRST:
					snprintf(buffer, buffer_len, "%d", (int) attr_value->i);
					break;
			}
		}
	}
	else
	{
		switch(attribute)
		{
			case IMGTOOLATTR_INT_MAC_TYPE:
				snprintf(buffer, buffer_len, "File type");
				break;
			case IMGTOOLATTR_INT_MAC_CREATOR:
				snprintf(buffer, buffer_len, "File creator");
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFLAGS:
				snprintf(buffer, buffer_len, "Finder flags");
				break;
			case IMGTOOLATTR_INT_MAC_COORDX:
				snprintf(buffer, buffer_len, "X coordinate");
				break;
			case IMGTOOLATTR_INT_MAC_COORDY:
				snprintf(buffer, buffer_len, "Y coordinate");
				break;
			case IMGTOOLATTR_INT_MAC_FINDERFOLDER:
				snprintf(buffer, buffer_len, "Finder folder");
				break;
			case IMGTOOLATTR_INT_MAC_ICONID:
				snprintf(buffer, buffer_len, "Icon ID");
				break;
			case IMGTOOLATTR_INT_MAC_SCRIPTCODE:
				snprintf(buffer, buffer_len, "Script code");
				break;
			case IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS:
				snprintf(buffer, buffer_len, "Extended flags");
				break;
			case IMGTOOLATTR_INT_MAC_COMMENTID:
				snprintf(buffer, buffer_len, "Comment ID");
				break;
			case IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY:
				snprintf(buffer, buffer_len, "Putaway directory");
				break;
			case IMGTOOLATTR_TIME_CREATED:
				snprintf(buffer, buffer_len, "Creation time");
				break;
			case IMGTOOLATTR_TIME_LASTMODIFIED:
				snprintf(buffer, buffer_len, "Last modified time");
				break;
		}
	}
}


//-------------------------------------------------
//  test_imgtool_datetime - unit test for imgtool::datetime
//-------------------------------------------------

static bool test_imgtool_datetime(int second, int minute, int hour, int day_of_month, int month, int year)
{
	bool error = false;

	util::arbitrary_datetime t;
	t.second = second;
	t.minute = minute;
	t.hour = hour;
	t.day_of_month = day_of_month;
	t.month = month;
	t.year = year;

	imgtool::datetime dt(imgtool::datetime::datetime_type::GMT, t);
	std::tm t2 = dt.gmtime();

	if (t2.tm_sec != second)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_sec to be %d, instead got %d\n", second, t2.tm_sec);
		error = true;
	}
	if (t2.tm_min != minute)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_min to be %d, instead got %d\n", minute, t2.tm_min);
		error = true;
	}
	if (t2.tm_hour != hour)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_hour to be %d, instead got %d\n", hour, t2.tm_hour);
		error = true;
	}
	if (t2.tm_mday != day_of_month)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_mday to be %d, instead got %d\n", day_of_month, t2.tm_mday);
		error = true;
	}
	if (t2.tm_mon != month - 1)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_mon to be %d, instead got %d\n", month - 1, t2.tm_mon);
		error = true;
	}
	if (t2.tm_year != year - 1900)
	{
		util::stream_format(std::wcerr, L"test_imgtool_datetime():  Expected t2.tm_mon to be %d, instead got %d\n", year - 1900, t2.tm_year);
		error = true;
	}
	return error;
}


//-------------------------------------------------
//  test_imgtool_datetime - unit tests for imgtool::datetime
//-------------------------------------------------

static bool test_imgtool_datetime()
{
	bool error = false;

	// various test cases for imgtool::datetime
	if (test_imgtool_datetime(34, 23, 12, 18, 3, 1993))     // March 18th, 1993 12:23:34
		error = true;
	if (test_imgtool_datetime(0, 20, 16, 25, 12, 1976))     // December 25th, 1976 16:20:00
		error = true;

	return error;
}


//-------------------------------------------------
//  imgtool_validitychecks - checks the validity
//  of the imgtool modules
//-------------------------------------------------

bool imgtool_validitychecks(void)
{
	bool error = false;
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;
	imgtool_module_features features;
	int created_library = false;

	// various test cases for imgtool::datetime
	if (test_imgtool_datetime())
		error = true;

	if (!global_imgtool_library)
	{
		imgtool_init(false, nullptr);
		created_library = true;
	}

	for (const auto &module : global_imgtool_library->modules())
	{
		features = imgtool_get_module_features(module.get());

		if (module->name.empty())
		{
			util::stream_format(std::wcerr, L"imgtool module has null 'name'\n");
			error = true;
		}
		if (module->description.empty())
		{
			util::stream_format(std::wcerr, L"imgtool module %s has null 'description'\n", wstring_from_utf8(module->name));
			error = true;
		}
		if (module->extensions.empty())
		{
			util::stream_format(std::wcerr, L"imgtool module %s has null 'extensions'\n", wstring_from_utf8(module->extensions));
			error = true;
		}

#if 0
		/* sanity checks on modules that do not support directories */
		if (!module->path_separator)
		{
			if (module->alternate_path_separator)
			{
				util::stream_format(std::wcerr, L"imgtool module %s specified alternate_path_separator but not path_separator\n", wstring_from_utf8(module->name));
				error = true;
			}
			if (module->initial_path_separator)
			{
				util::stream_format(std::wcerr, L"imgtool module %s specified initial_path_separator without directory support\n", wstring_from_utf8(module->name));
				error = true;
			}
			if (module->create_dir)
			{
				util::stream_format(std::wcerr, L"imgtool module %s implements create_dir without directory support\n", wstring_from_utf8(module->name));
				error = true;
			}
			if (module->delete_dir)
			{
				util::stream_format(std::wcerr, L"imgtool module %s implements delete_dir without directory support\n", wstring_from_utf8(module->name));
				error = true;
			}
		}
#endif

		/* sanity checks on creation options */
		if (module->createimage_optguide || !module->createimage_optspec.empty())
		{
			if (!module->create)
			{
				util::stream_format(std::wcerr, L"imgtool module %s has creation options without supporting create\n", wstring_from_utf8(module->name));
				error = true;
			}
			if ((!module->createimage_optguide && !module->createimage_optspec.empty())
				|| (module->createimage_optguide && module->createimage_optspec.empty()))
			{
				util::stream_format(std::wcerr, L"imgtool module %s does has partially incomplete creation options\n", wstring_from_utf8(module->name));
				error = true;
			}

			if (module->createimage_optguide && !module->createimage_optspec.empty())
			{
				auto resolution = std::make_unique<util::option_resolution>(*module->createimage_optguide);
				resolution->set_specification(module->createimage_optspec.c_str());
			}
		}
	}

	if (created_library)
		imgtool_exit();
	if (err)
	{
		util::stream_format(std::wcerr, L"imgtool: %s\n", wstring_from_utf8(imgtool_error(err)));
		error = true;
	}
	return error;
}



/*-------------------------------------------------
    imgtool_temp_str - provides a temporary string
    buffer used for string passing
-------------------------------------------------*/

char *imgtool_temp_str(void)
{
	static int index;
	static char temp_string_pool[32][256];
	return temp_string_pool[index++ % std::size(temp_string_pool)];
}



/***************************************************************************

    Image handling functions

***************************************************************************/

imgtoolerr_t imgtool::image::internal_open(const imgtool_module *module, const std::string &filename,
	int read_or_write, util::option_resolution *createopts, imgtool::image::ptr &outimg)
{
	imgtoolerr_t err;
	imgtool::stream::ptr stream;
	imgtool::image::ptr image;

	outimg.reset();

	// is the requested functionality implemented?
	if ((read_or_write == OSD_FOPEN_RW_CREATE) ? !module->create : !module->open)
	{
		err = imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);
		goto done;
	}

	// open the stream
	stream = imgtool::stream::open(filename, read_or_write);
	if (!stream)
	{
		err = imgtoolerr_t(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_IMAGEFILE);
		goto done;
	}

	// setup the image structure
	try { image = std::make_unique<imgtool::image>(*module); }
	catch (std::bad_alloc const &)
	{
		err = imgtoolerr_t(IMGTOOLERR_OUTOFMEMORY);
		goto done;
	}

	// actually call create or open
	err = (read_or_write == OSD_FOPEN_RW_CREATE)
		? module->create(*image, std::move(stream), createopts)
		: module->open(*image, std::move(stream));
	if (err)
	{
		err = markerrorsource(err);
		goto done;
	}

	// we've succeeded - set the output image, and record that
	// we are "okay to close"
	image->m_okay_to_close = true;
	outimg = std::move(image);

done:
	return err;
}


//-------------------------------------------------
//  open - open an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::open(const imgtool_module *module, const std::string &filename, int read_or_write, ptr &outimg)
{
	read_or_write = read_or_write ? OSD_FOPEN_RW : OSD_FOPEN_READ;
	return internal_open(module, filename, read_or_write, nullptr, outimg);
}


//-------------------------------------------------
//  imgtool::image::open_byname - open an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::open(const std::string &modulename, const std::string &filename, int read_or_write, ptr &outimg)
{
	const imgtool_module *module;

	module = imgtool_find_module(modulename);
	if (!module)
		return imgtoolerr_t(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);

	return open(module, filename, read_or_write, outimg);
}


//-------------------------------------------------
//  imgtool::image::image
//-------------------------------------------------

imgtool::image::image(const imgtool_module &module)
	: m_module(module)
	, m_okay_to_close(false)
{
	if (module.image_extra_bytes > 0)
	{
		m_extra_bytes = std::make_unique<uint8_t[]>(module.image_extra_bytes);
		std::fill_n(&m_extra_bytes[0], module.image_extra_bytes, 0);
	}

}


//-------------------------------------------------
//  imgtool::image::~image
//-------------------------------------------------

imgtool::image::~image()
{
	if (m_okay_to_close && module().close)
		module().close(*this);
}


//-------------------------------------------------
//  create - creates an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::create(const imgtool_module *module, const std::string &filename,
	util::option_resolution *opts, ptr &image)
{
	std::unique_ptr<util::option_resolution> alloc_resolution;

	/* allocate dummy options if necessary */
	if (!opts && module->createimage_optguide)
	{
		try { alloc_resolution.reset(new util::option_resolution(*module->createimage_optguide)); }
		catch (...) { return (imgtoolerr_t)IMGTOOLERR_OUTOFMEMORY; }

		if (!module->createimage_optspec.empty())
			alloc_resolution->set_specification(module->createimage_optspec.c_str());

		opts = alloc_resolution.get();
	}

	return internal_open(module, filename, OSD_FOPEN_RW_CREATE, opts, image);
}


//-------------------------------------------------
//  create - creates an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::create(const std::string &modulename, const std::string &filename, util::option_resolution *opts, ptr &image)
{
	const imgtool_module *module;

	module = imgtool_find_module(modulename);
	if (!module)
		return imgtoolerr_t(IMGTOOLERR_MODULENOTFOUND | IMGTOOLERR_SRC_MODULE);

	return create(module, filename, opts, image);
}


//-------------------------------------------------
//  create - creates an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::create(const imgtool_module *module, const std::string &filename,
	util::option_resolution *opts)
{
	std::unique_ptr<image> image;
	return create(module, filename, opts, image);
}


//-------------------------------------------------
//  create - creates an image
//-------------------------------------------------

imgtoolerr_t imgtool::image::create(const std::string &modulename, const std::string &filename, util::option_resolution *opts)
{
	std::unique_ptr<image> image;
	return create(modulename, filename, opts, image);
}


//-------------------------------------------------
//  info - returns format specific information about an image
//-------------------------------------------------

std::string imgtool::image::info()
{
	std::string string;
	if (module().info)
	{
		std::stringstream stream;
		module().info(*this, stream);
		string = stream.str();
	}
	return string;
}



#define PATH_MUSTBEDIR      0x00000001
#define PATH_LEAVENULLALONE 0x00000002
#define PATH_CANBEBOOTBLOCK 0x00000004

//-------------------------------------------------
//  partition::canonicalize_path - normalizes a path string
//  into a NUL delimited list
//-------------------------------------------------

imgtoolerr_t imgtool::partition::canonicalize_path(uint32_t flags, const char *path, std::string &result)
{
	imgtoolerr_t err = (imgtoolerr_t)IMGTOOLERR_SUCCESS;

	// is this path NULL?  if so, is that ignored?
	if (!path && (flags & PATH_LEAVENULLALONE))
		return IMGTOOLERR_SUCCESS;

	// is this the special filename for bootblocks?
	if (path == FILENAME_BOOTBLOCK)
	{
		if (!(flags & PATH_CANBEBOOTBLOCK))
			err = (imgtoolerr_t)IMGTOOLERR_UNEXPECTED;
		else if (!m_supports_bootblock)
			err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;
		return err;
	}

	// normalize the path into the native character set
	std::string converted_path;
	imgtool::charconverter * const converter = reinterpret_cast<imgtool::charconverter * const>(get_info_ptr(IMGTOOLINFO_PTR_CHARCONVERTER));
	if (converter)
	{
		converted_path = converter->from_utf8(path);
		path = converted_path.c_str();
	}

	if (m_path_separator == '\0')
	{
		if (flags & PATH_MUSTBEDIR)
		{
			// do we specify a path when paths are not supported? */
			if (path && *path)
				return imgtoolerr_t(IMGTOOLERR_CANNOTUSEPATH | IMGTOOLERR_SRC_FUNCTIONALITY);

			result = "";   /* normalize empty path */
		}
		else
		{
			// simple passthrough
			result = path;
		}
	}
	else
	{
		const char *s = path ? path : "";

		// allocate space for a new canonical path
		std::ostringstream stream;

		// copy the path
		bool in_path_separator = true;
		size_t i = 0;
		do
		{
			if ((s[i] != '\0') && (s[i] != m_path_separator) && (s[i] != m_alternate_path_separator))
			{
				stream << s[i];
				in_path_separator = false;
			}
			else if (!in_path_separator)
			{
				stream << '\0';
				in_path_separator = true;
			}
		}
		while(s[i++] != '\0');
		stream << '\0' << '\0';
		result = stream.str();
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::canonicalize_fork
//-------------------------------------------------

imgtoolerr_t imgtool::partition::canonicalize_fork(const char **fork)
{
	// does this module support forks?
	if (m_list_forks)
	{
		// this module supports forks; make sure that fork is non-NULL
		if (!*fork)
			*fork = "";
	}
	else
	{
		// this module does not support forks; make sure that fork is NULL
		if (*fork)
			return IMGTOOLERR_NOFORKS;
	}
	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_directory_entry - retrieves
//  the nth directory entry within a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_directory_entry(const char *path, int index, imgtool_dirent &ent)
{
	imgtoolerr_t err;
	imgtool::directory::ptr imgenum;

	if (index < 0)
	{
		err = (imgtoolerr_t)IMGTOOLERR_PARAMTOOSMALL;
		goto done;
	}

	err = imgtool::directory::open(*this, path, imgenum);
	if (err)
		goto done;

	do
	{
		err = imgenum->get_next(ent);
		if (err)
			goto done;

		if (ent.eof)
		{
			err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;
			goto done;
		}
	}
	while(index--);

done:
	if (err)
		memset(ent.filename, 0, sizeof(ent.filename));
	return err;
}


//-------------------------------------------------
//  partition::get_file_size - returns free
//  space on a partition, in bytes
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_file_size(const char *fname, uint64_t &filesize)
{
	imgtoolerr_t err;
	imgtool::directory::ptr imgenum;
	imgtool_dirent ent;
	const char *path;

	path = nullptr;    /* TODO: Need to parse off the path */

	filesize = ~((uint64_t) 0);
	memset(&ent, 0, sizeof(ent));

	err = imgtool::directory::open(*this, path, imgenum);
	if (err)
		goto done;

	do
	{
		err = imgenum->get_next(ent);
		if (err)
			goto done;

		if (!core_stricmp(fname, ent.filename))
		{
			filesize = ent.filesize;
			goto done;
		}
	}
	while(ent.filename[0]);

	err = (imgtoolerr_t)IMGTOOLERR_FILENOTFOUND;

done:
	return err;
}


//-------------------------------------------------
//  partition::list_file_attributes - identifies
//  all attributes on a file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::list_file_attributes(const char *path, uint32_t *attrs, size_t len)
{
	imgtoolerr_t err;

	memset(attrs, 0, sizeof(*attrs) * len);

	if (!m_list_attrs)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_LEAVENULLALONE, path, canonical_path);
	if (err)
		return err;

	err = m_list_attrs(*this, canonical_path.c_str(), attrs, len);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_file_attributes - retrieves
//  attributes on a file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_file_attributes(const char *path, const uint32_t *attrs, imgtool_attribute *values)
{
	imgtoolerr_t err;
	if (!m_get_attrs)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_LEAVENULLALONE, path, canonical_path);
	if (err)
		return err;

	err = m_get_attrs(*this, canonical_path.c_str(), attrs, values);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  imgtool::partition::put_file_attributes - sets
//  attributes on a file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::put_file_attributes(const char *path, const uint32_t *attrs, const imgtool_attribute *values)
{
	imgtoolerr_t err;
	if (!m_set_attrs)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_LEAVENULLALONE, path, canonical_path);
	if (err)
		return err;

	err = m_set_attrs(*this, canonical_path.c_str(), attrs, values);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_file_attribute - retrieves
//  an attribute on a single file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_file_attribute(const char *path, uint32_t attr, imgtool_attribute &value)
{
	uint32_t attrs[2];
	attrs[0] = attr;
	attrs[1] = 0;
	return get_file_attributes(path, attrs, &value);
}


//-------------------------------------------------
//  partition::put_file_attribute - sets
//  attributes on a single file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::put_file_attribute(const char *path, uint32_t attr, const imgtool_attribute &value)
{
	uint32_t attrs[2];
	attrs[0] = attr;
	attrs[1] = 0;
	return put_file_attributes(path, attrs, &value);
}


//-------------------------------------------------
//  partition::get_icon_info - retrieves the
//  icon for a file stored on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_icon_info(const char *path, imgtool_iconinfo *iconinfo)
{
	imgtoolerr_t err;
	if (!m_get_iconinfo)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(0, path, canonical_path);
	if (err)
		return err;

	memset(iconinfo, 0, sizeof(*iconinfo));
	err = m_get_iconinfo(*this, canonical_path.c_str(), iconinfo);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::suggest_file_filters - suggests a
//  list of filters appropriate for a file on a
//  partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::suggest_file_filters(const char *path,
	imgtool::stream *stream, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	imgtoolerr_t err;
	int i, j;
	imgtoolerr_t (*check_stream)(imgtool::stream &stream, imgtool_suggestion_viability_t *viability);
	size_t position;

	// clear out buffer
	memset(suggestions, 0, sizeof(*suggestions) * suggestions_length);

	if (!m_suggest_transfer)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_LEAVENULLALONE, path, canonical_path);
	if (err)
		return err;

	// invoke the module's suggest call
	err = m_suggest_transfer(*this, canonical_path.c_str(), suggestions, suggestions_length);
	if (err)
		return err;

	// loop on resulting suggestions, and do the following:
	//  1.  Call check_stream if present, and remove disqualified streams
	//  2.  Fill in missing descriptions
	i = j = 0;
	while(suggestions[i].viability)
	{
		if (stream && suggestions[i].filter)
		{
			check_stream = (imgtoolerr_t (*)(imgtool::stream &, imgtool_suggestion_viability_t *)) filter_get_info_fct(suggestions[i].filter, FILTINFO_PTR_CHECKSTREAM);
			if (check_stream)
			{
				position = stream->tell();
				err = check_stream(*stream, &suggestions[i].viability);
				stream->seek(position, SEEK_SET);
				if (err)
					return err;
			}
		}

		/* the check_stream proc can remove the option by clearing out the viability */
		if (suggestions[i].viability)
		{
			/* we may have to move this suggestion, if one was removed */
			if (i != j)
				memcpy(&suggestions[j], &suggestions[i], sizeof(*suggestions));

			/* if the description is missing, fill it in */
			if (!suggestions[j].description)
			{
				if (suggestions[j].filter)
					suggestions[j].description = filter_get_info_string(suggestions[i].filter, FILTINFO_STR_HUMANNAME);
				else
					suggestions[j].description = "Raw";
			}

			j++;
		}
		i++;
	}
	suggestions[j].viability = (imgtool_suggestion_viability_t)0;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_chain - retrieves the block
//  chain for a file or directory on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_chain(const char *path, imgtool_chainent *chain, size_t chain_size)
{
	size_t i;

	assert(chain_size > 0);

	if (!m_get_chain)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// initialize the chain array, so the module's get_chain function can be lazy
	for (i = 0; i < chain_size; i++)
	{
		chain[i].level = 0;
		chain[i].block = ~0;
	}

	return m_get_chain(*this, path, chain, chain_size - 1);
}


//-------------------------------------------------
//  partition::get_chain_string - retrieves
//  the block chain for a file or directory on a
//  partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_chain_string(const char *path, char *buffer, size_t buffer_len)
{
	imgtoolerr_t err;
	imgtool_chainent chain[512];
	uint64_t last_block;
	uint8_t cur_level = 0;
	int len, i;
	int comma_needed = false;

	// determine the last block identifier
	chain[0].block = ~0;
	last_block = chain[0].block;

	err = get_chain(path, chain, std::size(chain));
	if (err)
		return err;

	len = snprintf(buffer, buffer_len, "[");
	buffer += len;
	buffer_len -= len;

	for (i = 0; chain[i].block != last_block; i++)
	{
		while(cur_level < chain[i].level)
		{
			len = snprintf(buffer, buffer_len, " [");
			buffer += len;
			buffer_len -= len;
			cur_level++;
			comma_needed = false;
		}
		while(cur_level > chain[i].level)
		{
			len = snprintf(buffer, buffer_len, "]");
			buffer += len;
			buffer_len -= len;
			cur_level--;
		}

		if (comma_needed)
		{
			len = snprintf(buffer, buffer_len, ", ");
			buffer += len;
			buffer_len -= len;
		}

		len = snprintf(buffer, buffer_len, "%u", (unsigned) chain[i].block);
		buffer += len;
		buffer_len -= len;
		comma_needed = true;
	}

	do
	{
		len = snprintf(buffer, buffer_len, "]");
		buffer += len;
		buffer_len -= len;
	}
	while(cur_level-- > 0);

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_free_space - returns the
//  amount of free space on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_free_space(uint64_t &sz)
{
	imgtoolerr_t err;

	if (!m_free_space)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	err = m_free_space(*this, &sz);
	if (err)
		return (imgtoolerr_t)(err | IMGTOOLERR_SRC_IMAGEFILE);

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::read_file - starts reading
//  from a file on a partition with a stream
//-------------------------------------------------

imgtoolerr_t imgtool::partition::read_file(const char *filename, const char *fork, imgtool::stream &destf, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	union filterinfo u;

	if (!m_read_file)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	if (filter)
	{
		// use a filter
		filter(FILTINFO_PTR_READFILE, &u);
		if (!u.read_file)
			return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

		err = u.read_file(*this, filename, fork, destf);
		if (err)
			return markerrorsource(err);
	}
	else
	{
		// canonicalize path
		std::string canonical_path;
		err = canonicalize_path(PATH_CANBEBOOTBLOCK, filename, canonical_path);
		if (err)
			return err;

		err = canonicalize_fork(&fork);
		if (err)
			return err;

		// invoke the actual module
		err = m_read_file(*this, canonical_path.c_str(), fork, destf);
		if (err)
			return markerrorsource(err);
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::write_file - starts writing
//  to a new file on an image with a stream
//-------------------------------------------------

imgtoolerr_t imgtool::partition::write_file(const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	std::unique_ptr<util::option_resolution> alloc_resolution;
	uint64_t free_space;
	uint64_t file_size;
	union filterinfo u;

	if (!m_write_file)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	if (filter)
	{
		// use a filter
		filter(FILTINFO_PTR_WRITEFILE, &u);
		if (!u.write_file)
			return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

		err = u.write_file(*this, filename, fork, sourcef, opts);
		if (err)
			return markerrorsource(err);
	}
	else
	{
		// does this partition prefer upper case file names?
		std::string ucase_str;
		if (m_prefer_ucase)
		{
			std::ostringstream s;
			size_t i = 0;
			while (filename[i])
			{
				char32_t ch;
				int count = uchar_from_utf8(&ch, &filename[i], UTF8_CHAR_MAX);
				if (count > 0)
				{
					char32_t upper_ch = uchar_toupper(ch);
					s << utf8_from_uchar(upper_ch);
				}
				i += count > 0 ? count : 1;
			}
			ucase_str = s.str();
			filename = ucase_str.c_str();
		}

		// canonicalize path
		std::string canonical_path;
		err = canonicalize_path(PATH_CANBEBOOTBLOCK, filename, canonical_path);
		if (err)
			return err;

		err = canonicalize_fork(&fork);
		if (err)
			return err;

		// allocate dummy options if necessary
		if (!opts && m_writefile_optguide)
		{
			try { alloc_resolution.reset(new util::option_resolution(*m_writefile_optguide)); }
			catch (...)
			{
				return IMGTOOLERR_OUTOFMEMORY;
			}
			if (!m_writefile_optspec.empty())
				alloc_resolution->set_specification(m_writefile_optspec);
			opts = alloc_resolution.get();
		}

		// if free_space is implemented; do a quick check to see if space is available
		if (m_free_space)
		{
			err = m_free_space(*this, &free_space);
			if (err)
				return markerrorsource(err);

			file_size = sourcef.size();

			if (file_size > free_space)
				return markerrorsource(IMGTOOLERR_NOSPACE);
		}

		// actually invoke the write file handler
		err = m_write_file(*this, canonical_path.c_str(), fork, sourcef, opts);
		if (err)
			return markerrorsource(err);
	}

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_file - read a file from
//  an image, storing it into a native file
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_file(const char *filename, const char *fork,
	const char *dest, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	imgtool::stream::ptr f;
	char *alloc_dest = nullptr;
	const char *filter_extension = nullptr;

	if (!dest)
	{
		// determine the filter extension, if appropriate
		if (filter != nullptr)
			filter_extension = filter_get_info_string(filter, FILTINFO_STR_EXTENSION);

		if (filter_extension != nullptr)
		{
			alloc_dest = (char*)malloc(strlen(filename) + 1 + strlen(filter_extension) + 1);
			if (!alloc_dest)
				return IMGTOOLERR_OUTOFMEMORY;

			sprintf(alloc_dest, "%s.%s", filename, filter_extension);
			dest = alloc_dest;
		}
		else
		{
			if (filename == FILENAME_BOOTBLOCK)
				dest = "boot.bin";
			else
				dest = filename;
		}
	}

	f = imgtool::stream::open(dest, OSD_FOPEN_WRITE);
	if (!f)
	{
		err = imgtoolerr_t(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);
		goto done;
	}

	err = read_file(filename, fork, *f, filter);

done:
	if (alloc_dest != nullptr)
		free(alloc_dest);
	return err;
}


//-------------------------------------------------
//  partition::put_file - read a native file
//  and store it on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::put_file(const char *newfname, const char *fork,
	const char *source, util::option_resolution *opts, filter_getinfoproc filter)
{
	imgtoolerr_t err;
	imgtool::stream::ptr f;
	std::string alloc_newfname;
	std::string basename;

	if (!newfname)
	{
		basename = std::string(core_filename_extract_base(source));
		newfname = basename.c_str();
	}

	imgtool::charconverter *charconverter = (imgtool::charconverter *) get_info_ptr(IMGTOOLINFO_PTR_CHARCONVERTER);
	if (charconverter)
	{
		// convert to native format
		try
		{
			alloc_newfname = charconverter->from_utf8(newfname);
		}
		catch (charconverter_exception)
		{
			return imgtoolerr_t(IMGTOOLERR_BADFILENAME | IMGTOOLERR_SRC_NATIVEFILE);
		}
		newfname = alloc_newfname.c_str();
	}

	f = imgtool::stream::open(source, OSD_FOPEN_READ);
	if (f)
		err = write_file(newfname, fork, *f, opts, filter);
	else
		err = imgtoolerr_t(IMGTOOLERR_FILENOTFOUND | IMGTOOLERR_SRC_NATIVEFILE);

	return err;
}



/*-------------------------------------------------
    imgtool::partition::delete_file - delete a file
    on a partition
-------------------------------------------------*/

imgtoolerr_t imgtool::partition::delete_file(const char *fname)
{
	imgtoolerr_t err;
	if (!m_delete_file)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(0, fname, canonical_path);
	if (err)
		return err;

	err = m_delete_file(*this, canonical_path.c_str());
	if (err)
		return markerrorsource(err);

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::list_file_forks - lists all
//  forks on an image
//-------------------------------------------------

imgtoolerr_t imgtool::partition::list_file_forks(const char *path, std::vector<imgtool::fork_entry> &forks)
{
	imgtoolerr_t err;
	if (!m_list_forks)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(0, path, canonical_path);
	if (err)
		return err;

	// call the callback
	forks.clear();
	err = m_list_forks(*this, canonical_path.c_str(), forks);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::create_directory - creates a
//  directory on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::create_directory(const char *path)
{
	imgtoolerr_t err;

	// implemented?
	if (!m_create_dir)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_MUSTBEDIR, path, canonical_path);
	if (err)
		return err;

	err = m_create_dir(*this, path);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::delete_directory - deletes a
//  directory on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::delete_directory(const char *path)
{
	imgtoolerr_t err;

	// implemented?
	if (!m_delete_dir)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	// canonicalize path
	std::string canonical_path;
	err = canonicalize_path(PATH_MUSTBEDIR, path, canonical_path);
	if (err)
		return err;

	err = m_delete_dir(*this, path);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::get_block_size - gets the
//  size of a standard block on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::get_block_size(uint32_t &length)
{
	return m_image.get_block_size(length);
}


//-------------------------------------------------
//  partition::is_block_in_range
//-------------------------------------------------

imgtoolerr_t imgtool::partition::map_block_to_image_block(uint64_t partition_block, uint64_t &image_block) const
{
	if (partition_block >= m_block_count)
		return IMGTOOLERR_SEEKERROR;

	image_block = m_base_block + partition_block;
	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  partition::read_block - reads a standard
//  block on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::read_block(uint64_t block, void *buffer)
{
	uint64_t image_block;
	imgtoolerr_t err = map_block_to_image_block(block, image_block);
	if (err)
		return err;

	return m_image.read_block(image_block, buffer);
}


//-------------------------------------------------
//  partition::write_block - writes a
//  standard block on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::partition::write_block(uint64_t block, const void *buffer)
{
	uint64_t image_block;
	imgtoolerr_t err = map_block_to_image_block(block, image_block);
	if (err)
		return err;

	return m_image.write_block(image_block, buffer);
}


//-------------------------------------------------
//  partition::get_features - retrieves a
//  structure identifying this partition's features
//  associated with an image
//-------------------------------------------------

imgtool_partition_features imgtool::partition::get_features() const
{
	imgtool_partition_features features;
	memset(&features, 0, sizeof(features));

	if (m_read_file)
		features.supports_reading = 1;
	if (m_write_file)
		features.supports_writing = 1;
	if (m_delete_file)
		features.supports_deletefile = 1;
	if (m_path_separator)
		features.supports_directories = 1;
	if (m_free_space)
		features.supports_freespace = 1;
	if (m_create_dir)
		features.supports_createdir = 1;
	if (m_delete_dir)
		features.supports_deletedir = 1;
	if (m_supports_creation_time)
		features.supports_creation_time = 1;
	if (m_supports_lastmodified_time)
		features.supports_lastmodified_time = 1;
	if (m_get_iconinfo)
		features.supports_geticoninfo = 1;
	if (!features.supports_writing && !features.supports_createdir && !features.supports_deletefile && !features.supports_deletedir)
		features.is_read_only = 1;
	return features;
}


//-------------------------------------------------
//  partition::get_info_ptr - retrieves a
//  pointer associated with a partition's format
//-------------------------------------------------

void *imgtool::partition::get_info_ptr(uint32_t state)
{
	return imgtool_get_info_ptr(&m_imgclass, state);
}


//-------------------------------------------------
//  partition::get_info_string - retrieves a
//  string associated with a partition's format
//-------------------------------------------------

const char *imgtool::partition::get_info_string(uint32_t state)
{
	return imgtool_get_info_string(&m_imgclass, state);
}


//-------------------------------------------------
//  partition::get_info_int - retrieves a
//  pointer associated with a partition's format
//-------------------------------------------------

uint64_t imgtool::partition::get_info_int(uint32_t state)
{
	return imgtool_get_info_int(&m_imgclass, state);
}


//-------------------------------------------------
//  partition::extra_bytes - returns extra
//  bytes on a partition
//-------------------------------------------------

void *imgtool::partition::extra_bytes()
{
	if (!m_extra_bytes)
		throw false;
	return m_extra_bytes.get();
}


/***************************************************************************

    Path handling functions

***************************************************************************/

//-------------------------------------------------
//  partition::get_root_path - retrieves
//  the path root of this partition
//-------------------------------------------------

const char *imgtool::partition::get_root_path()
{
	int initial_path_separator;
	char path_separator;
	char *buf;
	int pos = 0;

	initial_path_separator = get_info_int(IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR) ? 1 : 0;
	path_separator = (char) get_info_int(IMGTOOLINFO_INT_PATH_SEPARATOR);
	buf = imgtool_temp_str();

	if (initial_path_separator)
		buf[pos++] = path_separator;
	buf[pos] = '\0';
	return buf;
}


//-------------------------------------------------
//  partition::path_concatenate - retrieves
//  a pointer associated with a partition's format
//-------------------------------------------------

const char *imgtool::partition::path_concatenate(const char *path1, const char *path2)
{
	char path_separator;
	size_t len;
	char *buffer;
	size_t buffer_len;

	path_separator = (char) get_info_int(IMGTOOLINFO_INT_PATH_SEPARATOR);
	len = strlen(path1);

	/* prepare buffer */
	buffer = imgtool_temp_str();
	buffer_len = 256;

	if (!strcmp(path2, "."))
	{
		/* keep the same directory */
		snprintf(buffer, buffer_len, "%s", path1);
	}
	else if (!strcmp(path2, ".."))
	{
		/* up one directory */
		while((len > 0) && (path1[len - 1] == path_separator))
			len--;
		while((len > 0) && (path1[len - 1] != path_separator))
			len--;
		while((len > 1) && (path1[len - 1] == path_separator))
			len--;
		snprintf(buffer, buffer_len, "%s", path1);
		buffer[len] = '\0';
	}
	else
	{
		/* append a path */
		if ((*path1 != 0) && (path1[strlen(path1) - 1] != path_separator))
			snprintf(buffer, buffer_len, "%s%c%s", path1, path_separator, path2);
		else
			snprintf(buffer, buffer_len, "%s%s", path1, path2);
	}
	return buffer;
}


//-------------------------------------------------
//  partition::get_base_name - retrieves
//  a base name for a partition specific path
//-------------------------------------------------

const char *imgtool::partition::get_base_name(const char *path)
{
	char path_separator;
	const char *new_path = path;
	int i;

	path_separator = (char) get_info_int(IMGTOOLINFO_INT_PATH_SEPARATOR);

	for (i = 0; path[i]; i++)
	{
		if (path[i] == path_separator)
			new_path = &path[i + 1];
	}
	return new_path;
}



/***************************************************************************

    Directory handling functions

***************************************************************************/

//-------------------------------------------------
//  directory ctor
//-------------------------------------------------

imgtool::directory::directory(imgtool::partition &partition)
	: m_partition(partition)
	, m_okay_to_close(false)
{
	if (partition.m_directory_extra_bytes > 0)
	{
		m_extra_bytes = std::make_unique<uint8_t[]>(partition.m_directory_extra_bytes);
		memset(m_extra_bytes.get(), 0, sizeof(m_extra_bytes.get()[0] * partition.m_directory_extra_bytes));
	}
}


//-------------------------------------------------
//  directory::open - begins
//  enumerating files on a partition
//-------------------------------------------------

imgtoolerr_t imgtool::directory::open(imgtool::partition &partition, const std::string &path, imgtool::directory::ptr &outenum)
{
	imgtoolerr_t err = imgtoolerr_t(IMGTOOLERR_SUCCESS);
	imgtool::directory::ptr enumeration;

	outenum.reset();

	if (!partition.m_next_enum)
		return imgtoolerr_t(IMGTOOLERR_UNIMPLEMENTED | IMGTOOLERR_SRC_FUNCTIONALITY);

	std::string canonical_path;
	err = partition.canonicalize_path(PATH_MUSTBEDIR, path.c_str(), canonical_path);
	if (err)
		return err;

	try { enumeration = std::make_unique<directory>(partition); }
	catch (std::bad_alloc const &)
	{
		return imgtoolerr_t(IMGTOOLERR_OUTOFMEMORY);
	}

	if (partition.m_begin_enum)
	{
		err = partition.m_begin_enum(*enumeration, canonical_path.c_str());
		if (err)
			return markerrorsource(err);
	}

	enumeration->m_okay_to_close = true;
	outenum = std::move(enumeration);

	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  directory dtor
//-------------------------------------------------

imgtool::directory::~directory()
{
	if (m_okay_to_close && m_partition.m_close_enum)
		m_partition.m_close_enum(*this);
}


//-------------------------------------------------
//  directory::get_next - continues
//  enumerating files within a partition
//-------------------------------------------------

imgtoolerr_t imgtool::directory::get_next(imgtool_dirent &ent)
{
	imgtoolerr_t err;

	// This makes it so that drivers don't have to take care of clearing
	// the attributes if they don't apply
	memset(&ent, 0, sizeof(ent));

	err = m_partition.m_next_enum(*this, ent);
	if (err)
		return markerrorsource(err);

	imgtool::charconverter * const converter  = reinterpret_cast<imgtool::charconverter * const>(m_partition.get_info_ptr(IMGTOOLINFO_PTR_CHARCONVERTER));
	if (converter)
	{
		std::string new_fname;
		try
		{
			new_fname = converter->to_utf8(ent.filename);
		}
		catch (charconverter_exception)
		{
			return imgtoolerr_t(IMGTOOLERR_BADFILENAME);
		}
		snprintf(ent.filename, std::size(ent.filename), "%s", new_fname.c_str());
	}

	// don't trust the module!
	if (!m_partition.m_supports_creation_time && (ent.creation_time.type() != imgtool::datetime::datetime_type::NONE))
	{
		internal_error(nullptr, "next_enum() specified creation_time, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	if (!m_partition.m_supports_lastmodified_time && (ent.lastmodified_time.type() != imgtool::datetime::datetime_type::NONE))
	{
		internal_error(nullptr, "next_enum() specified lastmodified_time, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	if (!m_partition.m_path_separator && ent.directory)
	{
		internal_error(nullptr, "next_enum() returned a directory, which is marked as unsupported by this module");
		return IMGTOOLERR_UNEXPECTED;
	}
	return IMGTOOLERR_SUCCESS;
}


//-------------------------------------------------
//  unknown_partition_get_info - represents an
//  unknown partition
//-------------------------------------------------

void unknown_partition_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch(state)
	{
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case IMGTOOLINFO_STR_NAME:                          strcpy(info->s = imgtool_temp_str(), "unknown"); break;
		case IMGTOOLINFO_STR_DESCRIPTION:                   strcpy(info->s = imgtool_temp_str(), "Unknown partition type"); break;
	}
}
