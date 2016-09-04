// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/****************************************************************************

    library.cpp

    Code relevant to the Imgtool library; analgous to the MESS/MAME driver
    list.

****************************************************************************/

#include <string.h>

#include "imgtool.h"
#include "library.h"
#include "pool.h"

namespace imgtool {


//-------------------------------------------------
//  ctor
//-------------------------------------------------

library::library()
{
	m_pool = pool_alloc_lib(nullptr);
	if (!m_pool)
		throw std::bad_alloc();
	m_first = nullptr;
	m_last = nullptr;
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
	imgtool_module *module;
	char *s1, *s2;

	// allocate the module and place it in the chain
	module = (imgtool_module *)imgtool_library_malloc(sizeof(*module));
	memset(module, 0, sizeof(*module));
	module->previous = m_last;
	if (m_last)
		m_last->next = module;
	else
		m_first = module;
	m_last = module;

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
	module->open                        = (imgtoolerr_t (*)(imgtool_image *, imgtool_stream *)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_OPEN);
	module->create                      = (imgtoolerr_t (*)(imgtool_image *, imgtool_stream *, util::option_resolution *)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_CREATE);
	module->close                       = (void (*)(imgtool_image *)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_CLOSE);
	module->info                        = (void (*)(imgtool_image *, char *, size_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_INFO);
	module->read_sector                 = (imgtoolerr_t (*)(imgtool_image *, UINT32, UINT32, UINT32, void *, size_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_READ_SECTOR);
	module->write_sector                = (imgtoolerr_t (*)(imgtool_image *, UINT32, UINT32, UINT32, const void *, size_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_WRITE_SECTOR);
	module->get_geometry                = (imgtoolerr_t (*)(imgtool_image *, UINT32 *, UINT32 *, UINT32 *))imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_GET_GEOMETRY);
	module->get_sector_size             = (imgtoolerr_t (*)(imgtool_image *, UINT32, UINT32, UINT32, UINT32 *))imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_GET_SECTOR_SIZE);
	module->read_block                  = (imgtoolerr_t (*)(imgtool_image *, void *, UINT64)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_READ_BLOCK);
	module->write_block                 = (imgtoolerr_t (*)(imgtool_image *, const void *, UINT64)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_WRITE_BLOCK);
	module->list_partitions             = (imgtoolerr_t (*)(imgtool_image *, imgtool_partition_info *, size_t)) imgtool_get_info_fct(imgclass, IMGTOOLINFO_PTR_LIST_PARTITIONS);
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

const imgtool_module *library::unlink(const char *module)
{
	imgtool_module *m;
	imgtool_module **previous;
	imgtool_module **next;

	for (m = m_first; m; m = m->next)
	{
		if (!core_stricmp(m->name, module))
		{
			previous = m->previous ? &m->previous->next : &m_first;
			next = m->next ? &m->next->previous : &m_last;
			*previous = m->next;
			*next = m->previous;
			m->previous = nullptr;
			m->next = nullptr;
			return m;
		}
	}
	return nullptr;
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
	imgtool_module *m1;
	imgtool_module *m2;
	imgtool_module *target;
	imgtool_module **before;
	imgtool_module **after;

	for (m1 = m_first; m1; m1 = m1->next)
	{
		target = m1;
		for (m2 = m1->next; m2; m2 = m2->next)
		{
			while(module_compare(target, m2, sort) > 0)
				target = m2;
		}

		if (target != m1)
		{
			// unlink the target
			before = target->previous ? &target->previous->next : &m_first;
			after = target->next ? &target->next->previous : &m_last;
			*before = target->next;
			*after = target->previous;

			// now place the target before m1
			target->previous = m1->previous;
			target->next = m1;
			before = m1->previous ? &m1->previous->next : &m_first;
			*before = target;
			m1->previous = target;

			// since we changed the order, we have to replace ourselves
			m1 = target;
		}
	}
}


//-------------------------------------------------
//  findmodule
//-------------------------------------------------

const imgtool_module *library::findmodule(const char *module_name)
{
	const imgtool_module *module;

	module = m_first;
	while(module && module_name && strcmp(module->name, module_name))
		module = module->next;
	return module;
}


//-------------------------------------------------
//  iterate
//-------------------------------------------------

imgtool_module *library::iterate(const imgtool_module *module)
{
	return module ? module->next : m_first;
}


//-------------------------------------------------
//  index
//-------------------------------------------------

imgtool_module *library::index(int i)
{
	imgtool_module *module;
	module = m_first;
	while(module && i--)
		module = module->next;
	return module;
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
