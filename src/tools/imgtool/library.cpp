// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    library.cpp

    Code relevant to the Imgtool library; analogous to the MESS/MAME driver
    list.

****************************************************************************/

#include <string.h>
#include <algorithm>

#include "imgtool.h"
#include "library.h"
#include "pool.h"

namespace imgtool {

datetime::imgtool_clock::duration datetime::s_gmt_offset = datetime::calculate_gmt_offset();

//-------------------------------------------------
//  datetime ctor
//-------------------------------------------------

datetime::datetime(datetime_type type, std::chrono::time_point<std::chrono::system_clock> tp)
	: m_type(type)
	, m_time_point(imgtool_clock::from_system_clock(tp))
{
}


//-------------------------------------------------
//  datetime ctor
//-------------------------------------------------

datetime::datetime(datetime_type type, time_t t)
	: datetime(type, std::chrono::system_clock::from_time_t(t))
{
}


//-------------------------------------------------
//  datetime ctor
//-------------------------------------------------

datetime::datetime(datetime_type type, const util::arbitrary_datetime &dt, bool clamp)
	: m_type(type)
	, m_time_point(imgtool_clock::from_arbitrary_datetime(dt, clamp))
{
}


//-------------------------------------------------
//  datetime::now
//-------------------------------------------------

datetime datetime::now(datetime_type type)
{
	return imgtool::datetime(
		type,
		std::chrono::system_clock::now());
}


//-------------------------------------------------
//  datetime::localtime
//-------------------------------------------------

std::tm datetime::localtime() const
{
	imgtool_clock::time_point tp;

	switch (type())
	{
	case datetime_type::LOCAL:
		tp = time_point();
		break;
	case datetime_type::GMT:
		tp = time_point() + s_gmt_offset;
		break;
	default:
		tp = imgtool_clock::time_point();
		break;
	}
	return imgtool_clock::to_tm(tp);
}


//-------------------------------------------------
//  datetime::gmtime
//-------------------------------------------------

std::tm datetime::gmtime() const
{
	imgtool_clock::time_point tp;

	switch (type())
	{
	case datetime_type::GMT:
		tp = time_point();
		break;
	case datetime_type::LOCAL:
		tp = time_point() - s_gmt_offset;
		break;
	default:
		tp = imgtool_clock::time_point();
		break;
	}
	return imgtool_clock::to_tm(tp);
}


//-------------------------------------------------
//  datetime::calculate_gmt_offset
//-------------------------------------------------

datetime::imgtool_clock::duration datetime::calculate_gmt_offset()
{
	time_t t = time(nullptr);
	std::tm utc_tm = *std::gmtime(&t);
	time_t utc = mktime(&utc_tm);
	std::tm local_tm = *std::localtime(&t);
	time_t local = mktime(&local_tm);
	double d =  difftime(local, utc) * imgtool_clock::period::den / imgtool_clock::period::num;
	return imgtool_clock::duration((std::int64_t) d);
}


//-------------------------------------------------
//  datetime::to_time_t
//-------------------------------------------------

time_t datetime::to_time_t() const
{
	auto system_clock_tp = imgtool_clock::to_system_clock(time_point());
	return std::chrono::system_clock::to_time_t(system_clock_tp);
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

library::library()
{
	m_pool = pool_alloc_lib(nullptr);
	if (!m_pool)
		throw std::bad_alloc();
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

library::~library()
{
	pool_free_lib(m_pool);
}


//-------------------------------------------------
//  add_class
//-------------------------------------------------

void library::add_class(const imgtool_class *imgclass)
{
	char *s1, *s2;

	// allocate the module and place it in the chain
	m_modules.emplace_back(std::make_unique<imgtool_module>());
	imgtool_module *module = m_modules.back().get();
	memset(module, 0, sizeof(*module));

	// extensions have a weird format
	s1 = imgtool_get_info_string(imgclass, IMGTOOLINFO_STR_FILE_EXTENSIONS);
	s2 = (char*)imgtool_library_malloc(strlen(s1) + 1);
	strcpy(s2, s1);
	module->extensions = s2;

	module->imgclass                    = *imgclass;
	module->name                        = imgtool_library_strdup(imgtool_get_info_string(imgclass, IMGTOOLINFO_STR_NAME));
	module->description                 = imgtool_library_strdup(imgtool_get_info_string(imgclass, IMGTOOLINFO_STR_DESCRIPTION));
	module->eoln                        = imgtool_library_strdup_allow_null(imgtool_get_info_string(imgclass, IMGTOOLINFO_STR_EOLN));
	module->initial_path_separator      = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_INITIAL_PATH_SEPARATOR) ? 1 : 0;
	module->open_is_strict              = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_OPEN_IS_STRICT) ? 1 : 0;
	module->tracks_are_called_cylinders = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_TRACKS_ARE_CALLED_CYLINDERS) ? 1 : 0;
	module->writing_untested            = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_WRITING_UNTESTED) ? 1 : 0;
	module->creation_untested           = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_CREATION_UNTESTED) ? 1 : 0;
	module->open                        = (imgtoolerr_t (*)(imgtool::image &, imgtool::stream::ptr &&)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_OPEN);
	module->create                      = (imgtoolerr_t (*)(imgtool::image &, imgtool::stream::ptr &&, util::option_resolution *)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_CREATE);
	module->close                       = (void (*)(imgtool::image &)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_CLOSE);
	module->info                        = (void (*)(imgtool::image &, std::ostream &)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_INFO);
	module->read_sector                 = (imgtoolerr_t (*)(imgtool::image &, uint32_t, uint32_t, uint32_t, std::vector<uint8_t> &)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_READ_SECTOR);
	module->write_sector                = (imgtoolerr_t (*)(imgtool::image &, uint32_t, uint32_t, uint32_t, const void *, size_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_WRITE_SECTOR);
	module->get_geometry                = (imgtoolerr_t (*)(imgtool::image &, uint32_t *, uint32_t *, uint32_t *))imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_GET_GEOMETRY);
	module->read_block                  = (imgtoolerr_t (*)(imgtool::image &, void *, uint64_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_READ_BLOCK);
	module->write_block                 = (imgtoolerr_t (*)(imgtool::image &, const void *, uint64_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_WRITE_BLOCK);
	module->list_partitions             = (imgtoolerr_t (*)(imgtool::image &, std::vector<imgtool::partition_info> &)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_LIST_PARTITIONS);
	module->block_size                  = imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_BLOCK_SIZE);
	module->createimage_optguide        = (const util::option_guide *) imgtool_get_info_ptr(imgclass, IMGTOOLINFO_PTR_CREATEIMAGE_OPTGUIDE);
	module->createimage_optspec         = imgtool_library_strdup_allow_null((const char*)imgtool_get_info_ptr(imgclass, IMGTOOLINFO_STR_CREATEIMAGE_OPTSPEC));
	module->image_extra_bytes           += imgtool_get_info_int(imgclass, IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES);
}


//-------------------------------------------------
//  add
//-------------------------------------------------

void library::add(imgtool_get_info get_info)
{
	int (*make_class)(int index, imgtool_class *imgclass);
	imgtool_class imgclass;
	int i, result;

	// try this class
	memset(&imgclass, 0, sizeof(imgclass));
	imgclass.get_info = get_info;

	// do we have derived getinfo functions?
	make_class = (int (*)(int index, imgtool_class *imgclass))
		imgtool_get_info_fct(&imgclass, IMGTOOLINFO_PTR_MAKE_CLASS);

	if (make_class)
	{
		i = 0;
		do
		{
			// clear out the class
			memset(&imgclass, 0, sizeof(imgclass));
			imgclass.get_info = get_info;

			// make the class
			result = make_class(i++, &imgclass);
			if (result)
				add_class(&imgclass);
		}
		while(result);
	}
	else
	{
		add_class(&imgclass);
	}
}


//-------------------------------------------------
//  unlink
//-------------------------------------------------

void library::unlink(const std::string &module_name)
{
	const modulelist::iterator iter = find(module_name);
	if (iter != m_modules.end())
		m_modules.erase(iter);
}


//-------------------------------------------------
//  module_compare
//-------------------------------------------------

int library::module_compare(const imgtool_module *m1, const imgtool_module *m2, sort_type sort)
{
	int rc = 0;
	switch(sort)
	{
	case sort_type::NAME:
		rc = strcmp(m1->name, m2->name);
		break;
	case sort_type::DESCRIPTION:
		rc = core_stricmp(m1->description, m2->description);
		break;
	}
	return rc;
}


//-------------------------------------------------
//  sort
//-------------------------------------------------

void library::sort(sort_type sort)
{
	auto compare = [this, sort](const std::unique_ptr<imgtool_module> &a, const std::unique_ptr<imgtool_module> &b)
	{
		return module_compare(a.get(), b.get(), sort) < 0;
	};
	m_modules.sort(compare);
}


//-------------------------------------------------
//  find
//-------------------------------------------------

library::modulelist::iterator library::find(const std::string &module_name)
{
	return std::find_if(
		m_modules.begin(),
		m_modules.end(),
		[module_name](std::unique_ptr<imgtool_module> &module) { return !module_name.compare(module->name); });
}


//-------------------------------------------------
//  findmodule
//-------------------------------------------------

const imgtool_module *library::findmodule(const std::string &module_name)
{
	modulelist::iterator iter = find(module_name);
	return iter != m_modules.end()
		? iter->get()
		: nullptr;
}


//-------------------------------------------------
//  imgtool_library_malloc
//-------------------------------------------------

void *library::imgtool_library_malloc(size_t mem)
{
	return pool_malloc_lib(m_pool, mem);
}


//-------------------------------------------------
//  imgtool_library_malloc
//-------------------------------------------------

char *library::imgtool_library_strdup(const char *s)
{
	return pool_strdup_lib(m_pool, s);
}


//-------------------------------------------------
//  imgtool_library_strdup_allow_null
//-------------------------------------------------

char *library::imgtool_library_strdup_allow_null(const char *s)
{
	return s ? imgtool_library_strdup(s) : nullptr;
}


} // namespace imgtool
