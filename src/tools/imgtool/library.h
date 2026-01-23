// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    library.h

    Code relevant to the Imgtool library; analogous to the MAME driver list.

    Unlike MAME which has a static driver lists, Imgtool has a concept of a
    library and this library is built at startup time.
    dynamic for which modules are added to.  This makes "dynamic" modules
    much easier

****************************************************************************/
#ifndef MAME_TOOLS_IMGTOOL_LIBRARY_H
#define MAME_TOOLS_IMGTOOL_LIBRARY_H

#pragma once

#include "imgterrs.h"

#include "timeconv.h"
#include "utilfwd.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iosfwd>
#include <list>
#include <memory>
#include <string>
#include <vector>


namespace imgtool {

class image;
class partition;
class directory;
class charconverter;
class stream;

enum suggestion_viability_t
{
	SUGGESTION_END,
	SUGGESTION_POSSIBLE,
	SUGGESTION_RECOMMENDED
};

union filterinfo
{
	int64_t   i;                                          /* generic integers */
	void *  p;                                          /* generic pointers */
	void *  f;                                          /* generic function pointers */
	const char *s;                                      /* generic strings */

	imgtoolerr_t (*read_file)(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf);
	imgtoolerr_t (*write_file)(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts);
	imgtoolerr_t (*check_stream)(imgtool::stream &stream, suggestion_viability_t *viability);
};

typedef void (*filter_getinfoproc)(uint32_t state, union filterinfo *info);

class datetime
{
public:
	typedef util::arbitrary_clock<std::int64_t, 1600, 1, 1, 0, 0, 0, std::ratio<1, 10000000> > imgtool_clock;

	enum datetime_type
	{
		NONE,
		LOCAL,
		GMT
	};

	datetime()
		: m_type(datetime_type::NONE)
	{
	}


	template<typename Rep, int Y, int M, int D, int H, int N, int S, typename Ratio>
	datetime(datetime_type type, std::chrono::time_point<util::arbitrary_clock<Rep, Y, M, D, H, N, S, Ratio> > tp)
		: m_type(type)
		, m_time_point(imgtool_clock::from_arbitrary_time_point(tp))
	{
	}

	datetime(datetime_type type, std::chrono::time_point<std::chrono::system_clock> tp);
	datetime(datetime_type type, time_t t);
	datetime(datetime_type type, const util::arbitrary_datetime &dt, bool clamp = true);
	datetime(const datetime &that) = default;
	datetime(datetime &&that) = default;

	// accessors
	datetime_type type() const { return m_type; }
	bool empty() const { return type() == datetime_type::NONE; }
	std::chrono::time_point<imgtool_clock> time_point() const { return m_time_point; }

	// operators
	datetime &operator =(const datetime &that)
	{
		m_type = that.m_type;
		m_time_point = that.m_time_point;
		return *this;
	}

	// returns the current time
	static datetime now(datetime_type type);

	// returns time structures
	std::tm localtime() const;
	std::tm gmtime() const;
	time_t to_time_t() const;

private:
	static imgtool_clock::duration          s_gmt_offset;
	datetime_type                           m_type;
	std::chrono::time_point<imgtool_clock>  m_time_point;

	static imgtool_clock::duration calculate_gmt_offset();
};


class fork_entry
{
public:
	enum class type_t
	{
		DATA,
		RESOURCE,
		ALT
	};

	fork_entry(uint64_t size, type_t type = type_t::DATA)
		: m_size(size)
		, m_type(type)
		, m_name(default_name(type))
	{

	}

	fork_entry(uint64_t size, std::string &&name)
		: m_size(size)
		, m_type(fork_entry::type_t::ALT)
		, m_name(std::move(name))
	{
	}

	fork_entry(const fork_entry &that) = default;
	fork_entry(fork_entry &&that) = default;

	uint64_t size() const { return m_size; }
	type_t type() const { return m_type; }
	const std::string &name() const { return m_name; }

private:
	static std::string default_name(type_t type)
	{
		switch (type)
		{
		case type_t::DATA:
			return std::string("");
		case type_t::RESOURCE:
			return std::string("RESOURCE_FORK");
		default:
			throw false;
		}
	}

	uint64_t    m_size;
	type_t      m_type;
	std::string m_name;
};

struct transfer_suggestion
{
	suggestion_viability_t viability;
	filter_getinfoproc filter;
	const char *fork;
	const char *description;
};

} // namespace imgtool

struct imgtool_dirent
{
	imgtool_dirent()
	{
		std::fill(std::begin(filename), std::end(filename), 0);
		std::fill(std::begin(attr), std::end(attr), 0);
		filesize = 0;

		std::fill(std::begin(softlink), std::end(softlink), 0);
		std::fill(std::begin(comment), std::end(comment), 0);

		eof = corrupt = directory = hardlink = 0;
	}

	char filename[1024];
	char attr[64];
	uint64_t filesize;

	imgtool::datetime creation_time;
	imgtool::datetime lastmodified_time;
	imgtool::datetime lastaccess_time;

	char softlink[1024];
	char comment[256];

	/* flags */
	unsigned int eof : 1;
	unsigned int corrupt : 1;
	unsigned int directory : 1;
	unsigned int hardlink : 1;
};

enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	IMGTOOLATTR_INT_FIRST = 0x00000,
	IMGTOOLATTR_INT_MAC_TYPE,
	IMGTOOLATTR_INT_MAC_CREATOR,
	IMGTOOLATTR_INT_MAC_FINDERFLAGS,
	IMGTOOLATTR_INT_MAC_COORDX,
	IMGTOOLATTR_INT_MAC_COORDY,
	IMGTOOLATTR_INT_MAC_FINDERFOLDER,
	IMGTOOLATTR_INT_MAC_ICONID,
	IMGTOOLATTR_INT_MAC_SCRIPTCODE,
	IMGTOOLATTR_INT_MAC_EXTENDEDFLAGS,
	IMGTOOLATTR_INT_MAC_COMMENTID,
	IMGTOOLATTR_INT_MAC_PUTAWAYDIRECTORY,

	/* --- the following bits of info are returned as pointers to data or functions --- */
	IMGTOOLATTR_PTR_FIRST = 0x10000,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	IMGTOOLATTR_STR_FIRST = 0x20000,

	/* --- the following bits of info are returned as time_t values --- */
	IMGTOOLATTR_TIME_FIRST = 0x30000,
	IMGTOOLATTR_TIME_CREATED,
	IMGTOOLATTR_TIME_LASTMODIFIED
};

union imgtool_attribute
{
	int64_t   i;
	time_t  t;
};

struct imgtool_iconinfo
{
	unsigned icon16x16_specified : 1;
	unsigned icon32x32_specified : 1;
	uint32_t icon16x16[16][16];
	uint32_t icon32x32[32][32];
};

enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	IMGTOOLINFO_INT_FIRST = 0x00000,
	IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES,
	IMGTOOLINFO_INT_PARTITION_EXTRA_BYTES,
	IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES,
	IMGTOOLINFO_INT_PATH_SEPARATOR,
	IMGTOOLINFO_INT_ALTERNATE_PATH_SEPARATOR,
	IMGTOOLINFO_INT_PREFER_UCASE,
	IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR,
	IMGTOOLINFO_INT_OPEN_IS_STRICT,
	IMGTOOLINFO_INT_SUPPORTS_CREATION_TIME,
	IMGTOOLINFO_INT_SUPPORTS_LASTMODIFIED_TIME,
	IMGTOOLINFO_INT_TRACKS_ARE_CALLED_CYLINDERS,
	IMGTOOLINFO_INT_WRITING_UNTESTED,
	IMGTOOLINFO_INT_CREATION_UNTESTED,
	IMGTOOLINFO_INT_SUPPORTS_BOOTBLOCK,
	IMGTOOLINFO_INT_BLOCK_SIZE,

	IMGTOOLINFO_INT_CLASS_SPECIFIC = 0x08000,

	/* --- the following bits of info are returned as pointers to data or functions --- */
	IMGTOOLINFO_PTR_FIRST = 0x10000,

	IMGTOOLINFO_PTR_OPEN,
	IMGTOOLINFO_PTR_CREATE,
	IMGTOOLINFO_PTR_CLOSE,
	IMGTOOLINFO_PTR_OPEN_PARTITION,
	IMGTOOLINFO_PTR_CREATE_PARTITION,
	IMGTOOLINFO_PTR_INFO,
	IMGTOOLINFO_PTR_BEGIN_ENUM,
	IMGTOOLINFO_PTR_NEXT_ENUM,
	IMGTOOLINFO_PTR_CLOSE_ENUM,
	IMGTOOLINFO_PTR_FREE_SPACE,
	IMGTOOLINFO_PTR_READ_FILE,
	IMGTOOLINFO_PTR_WRITE_FILE,
	IMGTOOLINFO_PTR_DELETE_FILE,
	IMGTOOLINFO_PTR_LIST_FORKS,
	IMGTOOLINFO_PTR_CREATE_DIR,
	IMGTOOLINFO_PTR_DELETE_DIR,
	IMGTOOLINFO_PTR_LIST_ATTRS,
	IMGTOOLINFO_PTR_GET_ATTRS,
	IMGTOOLINFO_PTR_SET_ATTRS,
	IMGTOOLINFO_PTR_ATTR_NAME,
	IMGTOOLINFO_PTR_GET_ICON_INFO,
	IMGTOOLINFO_PTR_SUGGEST_TRANSFER,
	IMGTOOLINFO_PTR_GET_GEOMETRY,
	IMGTOOLINFO_PTR_READ_SECTOR,
	IMGTOOLINFO_PTR_WRITE_SECTOR,
	IMGTOOLINFO_PTR_READ_BLOCK,
	IMGTOOLINFO_PTR_WRITE_BLOCK,
	IMGTOOLINFO_PTR_APPROVE_FILENAME_CHAR,
	IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE,
	IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE,
	IMGTOOLINFO_PTR_MAKE_CLASS,
	IMGTOOLINFO_PTR_LIST_PARTITIONS,
	IMGTOOLINFO_PTR_CHARCONVERTER,

	IMGTOOLINFO_PTR_CLASS_SPECIFIC = 0x18000,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	IMGTOOLINFO_STR_FIRST = 0x20000,

	IMGTOOLINFO_STR_NAME,
	IMGTOOLINFO_STR_DESCRIPTION,
	IMGTOOLINFO_STR_FILE,
	IMGTOOLINFO_STR_FILE_EXTENSIONS,
	IMGTOOLINFO_STR_EOLN,
	IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC,
	IMGTOOLINFO_STR_WRITEFILE_OPTSPEC,

	IMGTOOLINFO_STR_CLASS_SPECIFIC = 0x28000
};



union imgtoolinfo;

struct imgtool_class;
typedef void (*imgtool_get_info)(const imgtool_class *, uint32_t, union imgtoolinfo *);

struct imgtool_class
{
	imgtool_get_info get_info;
	imgtool_get_info derived_get_info;
	void *derived_param;
};



namespace imgtool
{
	class partition_info
	{
	public:
		partition_info(imgtool_get_info get_info, uint64_t base_block, uint64_t block_count)
			: m_base_block(base_block)
			, m_block_count(block_count)
		{
			memset(&m_imgclass, 0, sizeof(m_imgclass));
			m_imgclass.get_info = get_info;
		}

		partition_info(imgtool_class imgclass, uint64_t base_block, uint64_t block_count)
			: m_imgclass(imgclass)
			, m_base_block(base_block)
			, m_block_count(block_count)
		{
		}

		const imgtool_class &imgclass() const { return m_imgclass; }
		uint64_t base_block() const { return m_base_block; }
		uint64_t block_count() const { return m_block_count; }

	private:
		imgtool_class           m_imgclass;
		uint64_t                m_base_block;
		uint64_t                m_block_count;
	};
};


union imgtoolinfo
{
	int64_t   i;                                          /* generic integers */
	void *  p;                                          /* generic pointers */
	void *  f;                                          /* generic function pointers */
	char *  s;                                          /* generic strings */

	imgtoolerr_t    (*open)             (imgtool::image &image, std::unique_ptr<imgtool::stream> &&stream);
	void            (*close)            (imgtool::image &image);
	imgtoolerr_t    (*create)           (imgtool::image &image, std::unique_ptr<imgtool::stream> &&stream, util::option_resolution *opts);
	imgtoolerr_t    (*create_partition) (imgtool::image &image, uint64_t first_block, uint64_t block_count);
	void            (*info)             (imgtool::image &image, std::ostream &stream);
	imgtoolerr_t    (*begin_enum)       (imgtool::directory &enumeration, const char *path);
	imgtoolerr_t    (*next_enum)        (imgtool::directory &enumeration, imgtool_dirent &ent);
	void            (*close_enum)       (imgtool::directory &enumeration);
	imgtoolerr_t    (*open_partition)   (imgtool::partition &partition, uint64_t first_block, uint64_t block_count);
	imgtoolerr_t    (*free_space)       (imgtool::partition &partition, uint64_t *size);
	imgtoolerr_t    (*read_file)        (imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf);
	imgtoolerr_t    (*write_file)       (imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts);
	imgtoolerr_t    (*delete_file)      (imgtool::partition &partition, const char *filename);
	imgtoolerr_t    (*list_forks)       (imgtool::partition &partition, const char *path, std::vector<imgtool::fork_entry> &forks);
	imgtoolerr_t    (*create_dir)       (imgtool::partition &partition, const char *path);
	imgtoolerr_t    (*delete_dir)       (imgtool::partition &partition, const char *path);
	imgtoolerr_t    (*list_attrs)       (imgtool::partition &partition, const char *path, uint32_t *attrs, size_t len);
	imgtoolerr_t    (*get_attrs)        (imgtool::partition &partition, const char *path, const uint32_t *attrs, imgtool_attribute *values);
	imgtoolerr_t    (*set_attrs)        (imgtool::partition &partition, const char *path, const uint32_t *attrs, const imgtool_attribute *values);
	imgtoolerr_t    (*attr_name)        (uint32_t attribute, const imgtool_attribute *attr, char *buffer, size_t buffer_len);
	imgtoolerr_t    (*get_iconinfo)     (imgtool::partition &partition, const char *path, imgtool_iconinfo *iconinfo);
	imgtoolerr_t    (*suggest_transfer) (imgtool::partition &partition, const char *path, imgtool::transfer_suggestion *suggestions, size_t suggestions_length);
	imgtoolerr_t    (*get_geometry)     (imgtool::image &image, uint32_t *tracks, uint32_t *heads, uint32_t *sectors);
	imgtoolerr_t    (*read_sector)      (imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, std::vector<uint8_t> &buffer);
	imgtoolerr_t    (*write_sector)     (imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, const void *buffer, size_t len, int ddam);
	imgtoolerr_t    (*read_block)       (imgtool::image &image, void *buffer, uint64_t block);
	imgtoolerr_t    (*write_block)      (imgtool::image &image, const void *buffer, uint64_t block);
	imgtoolerr_t    (*list_partitions)  (imgtool::image &image, std::vector<imgtool::partition_info> &partitions);
	int             (*approve_filename_char)(char32_t ch);
	int             (*make_class)(int index, imgtool_class *imgclass);

	const util::option_guide *createimage_optguide;
	const util::option_guide *writefile_optguide;
	const imgtool::charconverter *charconverter;
};



static inline int64_t imgtool_get_info_int(const imgtool_class *imgclass, uint32_t state)
{
	union imgtoolinfo info;
	info.i = 0;
	imgclass->get_info(imgclass, state, &info);
	return info.i;
}

static inline void *imgtool_get_info_ptr(const imgtool_class *imgclass, uint32_t state)
{
	union imgtoolinfo info;
	info.p = nullptr;
	imgclass->get_info(imgclass, state, &info);
	return info.p;
}

static inline void *imgtool_get_info_fct(const imgtool_class *imgclass, uint32_t state)
{
	union imgtoolinfo info;
	info.f = nullptr;
	imgclass->get_info(imgclass, state, &info);
	return info.f;
}

static inline char *imgtool_get_info_string(const imgtool_class *imgclass, uint32_t state)
{
	union imgtoolinfo info;
	info.s = nullptr;
	imgclass->get_info(imgclass, state, &info);
	return info.s;
}

/* circular string buffer */
char *imgtool_temp_str(void);



struct imgtool_module
{
	imgtool_class imgclass = { 0 };

	std::string name;
	std::string description;
	std::string extensions;
	std::string eoln;

	size_t image_extra_bytes = 0;

	/* flags */
	bool initial_path_separator = false;
	bool open_is_strict = false;
	bool tracks_are_called_cylinders = false;    /* used for hard drivers */
	bool writing_untested = false;               /* used when we support writing, but not in main build */
	bool creation_untested = false;              /* used when we support creation, but not in main build */

	imgtoolerr_t    (*open)         (imgtool::image &image, std::unique_ptr<imgtool::stream> &&stream) = nullptr;
	void            (*close)        (imgtool::image &image) = nullptr;
	void            (*info)         (imgtool::image &image, std::ostream &stream) = nullptr;
	imgtoolerr_t    (*create)       (imgtool::image &image, std::unique_ptr<imgtool::stream> &&stream, util::option_resolution *opts) = nullptr;
	imgtoolerr_t    (*get_geometry) (imgtool::image &image, uint32_t *track, uint32_t *heads, uint32_t *sectors) = nullptr;
	imgtoolerr_t    (*read_sector)  (imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, std::vector<uint8_t> &buffer) = nullptr;
	imgtoolerr_t    (*write_sector) (imgtool::image &image, uint32_t track, uint32_t head, uint32_t sector, const void *buffer, size_t len) = nullptr;
	imgtoolerr_t    (*read_block)   (imgtool::image &image, void *buffer, uint64_t block) = nullptr;
	imgtoolerr_t    (*write_block)  (imgtool::image &image, const void *buffer, uint64_t block) = nullptr;
	imgtoolerr_t    (*list_partitions)(imgtool::image &image, std::vector<imgtool::partition_info> &partitions) = nullptr;

	uint32_t block_size = 0;

	const util::option_guide *createimage_optguide = nullptr;
	std::string createimage_optspec;

	const void *extra = nullptr;
};

namespace imgtool {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// imgtool "library" - equivalent to the MAME driver list
class library
{
public:
	typedef std::list<std::unique_ptr<imgtool_module> > modulelist;

	enum class sort_type
	{
		NAME,
		DESCRIPTION
	};

	library();
	~library();

	// adds a module to an imgtool library
	void add(imgtool_get_info get_info);

	// seeks out and removes a module from an imgtool library
	void unlink(const std::string &module_name);

	// sorts an imgtool library
	void sort(sort_type sort);

	// finds a module
	const imgtool_module *findmodule(const std::string &module_name);

	// module iteration
	const modulelist &modules() { return m_modules; }

private:
	modulelist      m_modules;

	// internal lookup and iteration
	modulelist::iterator find(const std::string &module_name);

	// helpers
	void add_class(const imgtool_class *imgclass);
	int module_compare(const imgtool_module *m1, const imgtool_module *m2, sort_type sort);
};

} // namespace imgtool

#endif // MAME_TOOLS_IMGTOOL_LIBRARY_H
