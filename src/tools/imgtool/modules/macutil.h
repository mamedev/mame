// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    macutil.h

    Imgtool Utility code for manipulating certain Apple/Mac data structures
    and conventions

*****************************************************************************/

#ifndef MACUTIL_H
#define MACUTIL_H

#include "imgtool.h"

enum mac_fork_t
{
	MAC_FORK_DATA,
	MAC_FORK_RESOURCE
};

enum mac_filecategory_t
{
	MAC_FILECATEGORY_DATA,
	MAC_FILECATEGORY_TEXT,
	MAC_FILECATEGORY_FORKED
};


/* converting Classic Mac OS time <==> Imgtool time */
time_t mac_crack_time(UINT32 t);
UINT32 mac_setup_time(time_t t);
UINT32 mac_time_now(void);

imgtoolerr_t mac_identify_fork(const char *fork_string, mac_fork_t *fork_num);

void mac_suggest_transfer(mac_filecategory_t file_category, imgtool_transfer_suggestion *suggestions, size_t suggestions_length);

void pascal_from_c_string(unsigned char *pstring, size_t pstring_len, const char *cstring);

#endif /* MACUTIL_H */
