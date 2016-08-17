// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    filter.h

    Imgtool filters

***************************************************************************/

#ifndef FILTER_H
#define FILTER_H

#include <stdlib.h>
#include <stdio.h>

#include "library.h"

struct imgtool_filter;

enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	FILTINFO_INT_FIRST = 0x00000,
	FILTINFO_INT_STATESIZE,

	/* --- the following bits of info are returned as pointers to data or functions --- */
	FILTINFO_PTR_FIRST = 0x10000,
	FILTINFO_PTR_READFILE,
	FILTINFO_PTR_WRITEFILE,
	FILTINFO_PTR_CHECKSTREAM,

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	FILTINFO_STR_FIRST = 0x20000,
	FILTINFO_STR_NAME,
	FILTINFO_STR_HUMANNAME,
	FILTINFO_STR_EXTENSION
};

extern const filter_getinfoproc filters[];

filter_getinfoproc filter_lookup(const char *name);

/* ----------------------------------------------------------------------- */

INT64 filter_get_info_int(filter_getinfoproc get_info, UINT32 state);
void *filter_get_info_ptr(filter_getinfoproc get_info, UINT32 state);
void *filter_get_info_fct(filter_getinfoproc get_info, UINT32 state);
const char *filter_get_info_string(filter_getinfoproc get_info, UINT32 state);

/* ----------------------------------------------------------------------- */

extern void filter_eoln_getinfo(UINT32 state, union filterinfo *info);
extern void filter_cocobas_getinfo(UINT32 state, union filterinfo *info);
extern void filter_dragonbas_getinfo(UINT32 state, union filterinfo *info);
extern void filter_macbinary_getinfo(UINT32 state, union filterinfo *info);
extern void filter_vzsnapshot_getinfo(UINT32 state, union filterinfo *info);
extern void filter_vzbas_getinfo(UINT32 state, union filterinfo *info);
extern void filter_thombas5_getinfo(UINT32 state, union filterinfo *info);
extern void filter_thombas7_getinfo(UINT32 state, union filterinfo *info);
extern void filter_thombas128_getinfo(UINT32 state, union filterinfo *info);
extern void filter_thomcrypt_getinfo(UINT32 state, union filterinfo *info);
extern void filter_bml3bas_getinfo(UINT32 state, union filterinfo *info);


#endif /* FILTER_H */
