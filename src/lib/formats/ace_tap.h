// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*********************************************************************

    ace_tap.h

    Format code for Jupiter Ace cassette files

*********************************************************************/
#ifndef MAME_FORMATS_ACE_TAP_H
#define MAME_FORMATS_ACE_TAP_H

#include "cassimg.h"

#if 0
struct ace_tape_t
{
	uint8_t hdr_type;
	uint8_t hdr_name[10];
	uint16_t hdr_len;
	uint16_t hdr_addr;
	uint8_t hdr_vars[8];
	uint8_t hdr_3c4c;
	uint8_t hdr_3c4d;
	uint16_t dat_len;
};
#endif

CASSETTE_FORMATLIST_EXTERN(ace_cassette_formats);

#endif // MAME_FORMATS_ACE_TAP_H
