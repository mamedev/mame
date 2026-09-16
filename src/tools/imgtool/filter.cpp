// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    filter.c

    Imgtool filters

***************************************************************************/

#include "filter.h"

#include <cstring>

/* ----------------------------------------------------------------------- */

int64_t filter_get_info_int(imgtool::filter_getinfoproc get_info, uint32_t state)
{
	imgtool::filterinfo info;
	info.i = 0;
	get_info(state, &info);
	return info.i;
}

void *filter_get_info_ptr(imgtool::filter_getinfoproc get_info, uint32_t state)
{
	imgtool::filterinfo info;
	info.p = nullptr;
	get_info(state, &info);
	return info.p;
}

void *filter_get_info_fct(imgtool::filter_getinfoproc get_info, uint32_t state)
{
	imgtool::filterinfo info;
	info.f = nullptr;
	get_info(state, &info);
	return info.f;
}

const char *filter_get_info_string(imgtool::filter_getinfoproc get_info, uint32_t state)
{
	imgtool::filterinfo info;
	info.s = nullptr;
	get_info(state, &info);
	return info.s;
}

/* ----------------------------------------------------------------------- */

const imgtool::filter_getinfoproc filters[] =
{
	filter_eoln_getinfo,
	filter_cocobas_getinfo,
	filter_dragonbas_getinfo,
	filter_vzsnapshot_getinfo,
	filter_vzbas_getinfo,
	filter_thombas5_getinfo,
	filter_thombas7_getinfo,
	filter_thombas128_getinfo,
	filter_thomcrypt_getinfo,
	filter_bml3bas_getinfo,
		filter_hp9845data_getinfo,
	nullptr
};



imgtool::filter_getinfoproc filter_lookup(const char *name)
{
	for (int i = 0; filters[i]; i++)
	{
		char const *const filter_name = filter_get_info_string(filters[i], FILTINFO_STR_NAME);
		if (!strcmp(name, filter_name))
			return filters[i];
	}

	return nullptr;
}
