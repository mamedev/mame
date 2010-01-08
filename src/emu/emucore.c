/***************************************************************************

    mamecore.c

    Simple core functions that are defined in emucore.h and which may
    need to be accessed by other MAME-related tools.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#include "emucore.h"
#include <ctype.h>

/* a giant string buffer for temporary strings */
char giant_string_buffer[GIANT_STRING_BUFFER_SIZE] = { 0 };
