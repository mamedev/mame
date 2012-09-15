/*********************************************************************

    ace_tap.h

    Format code for Jupiter Ace casette files

*********************************************************************/

#ifndef ACE_TAP_H
#define ACE_TAP_H

#include "cassimg.h"

struct ace_tape_t 
{
	UINT8 hdr_type;
	UINT8 hdr_name[10];
	UINT16 hdr_len;
	UINT16 hdr_addr;
	UINT8 hdr_vars[8];
	UINT8 hdr_3c4c;
	UINT8 hdr_3c4d;
	UINT16 dat_len;
};

CASSETTE_FORMATLIST_EXTERN(ace_cassette_formats);

#endif /* ACE_TAP_H */

