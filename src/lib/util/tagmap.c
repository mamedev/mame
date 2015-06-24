// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    tagmap.c

    tagmap counter

***************************************************************************/

#include <assert.h>

#include "tagmap.h"

#ifdef MAME_DEBUG
/** @brief  The tagmap finds. */
INT32 g_tagmap_finds = 0;
/** @brief  true to enable, false to disable the tagmap counter. */
bool g_tagmap_counter_enabled = false;
#endif
