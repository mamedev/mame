// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/****************************************************************************

    macutil.c

    Imgtool Utility code for manipulating certain Apple/Mac data structures
    and conventions

*****************************************************************************/

#include "macutil.h"


time_t mac_crack_time(UINT32 t)
{
	/* not sure if this is correct... */
	return t - (((1970 - 1904) * 365) + 17) * 24 * 60 * 60;
}



UINT32 mac_setup_time(time_t t)
{
	/* not sure if this is correct... */
	return t + (((1970 - 1904) * 365) + 17) * 24 * 60 * 60;
}



UINT32 mac_time_now(void)
{
	time_t now;
	time(&now);
	return mac_setup_time(now);
}



imgtoolerr_t mac_identify_fork(const char *fork_string, mac_fork_t *fork_num)
{
	if (!strcmp(fork_string, ""))
		*fork_num = MAC_FORK_DATA;
	else if (!strcmp(fork_string, "RESOURCE_FORK"))
		*fork_num = MAC_FORK_RESOURCE;
	else
		return IMGTOOLERR_FORKNOTFOUND;
	return IMGTOOLERR_SUCCESS;
}



void mac_suggest_transfer(mac_filecategory_t file_category, imgtool_transfer_suggestion *suggestions, size_t suggestions_length)
{
	suggestions[0].viability = (file_category == MAC_FILECATEGORY_FORKED) ? SUGGESTION_RECOMMENDED : SUGGESTION_POSSIBLE;
	suggestions[0].filter = filter_macbinary_getinfo;
	suggestions[0].fork = NULL;
	suggestions[0].description = NULL;

	suggestions[1].viability = (file_category == MAC_FILECATEGORY_TEXT) ? SUGGESTION_RECOMMENDED : SUGGESTION_POSSIBLE;
	suggestions[1].filter = filter_eoln_getinfo;
	suggestions[1].fork = NULL;
	suggestions[1].description = NULL;

	suggestions[2].viability = (file_category == MAC_FILECATEGORY_DATA) ? SUGGESTION_RECOMMENDED : SUGGESTION_POSSIBLE;
	suggestions[2].filter = NULL;
	suggestions[2].fork = "";
	suggestions[2].description = "Raw (data fork)";

	suggestions[3].viability = SUGGESTION_POSSIBLE;
	suggestions[3].filter = NULL;
	suggestions[3].fork = "RESOURCE_FORK";
	suggestions[3].description = "Raw (resource fork)";
}



void pascal_from_c_string(unsigned char *pstring, size_t pstring_len, const char *cstring)
{
	size_t cstring_len, i;

	cstring_len = strlen(cstring);
	pstring[0] = MIN(cstring_len, pstring_len - 1);

	for (i = 0; i < pstring[0]; i++)
		pstring[1 + i] = cstring[i];
	while(i < pstring_len - 1)
		pstring[1 + i++] = '\0';
}
