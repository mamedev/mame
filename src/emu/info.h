/***************************************************************************

    info.h

    Dumps the MAME internal data as an XML file.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __INFO_H__
#define __INFO_H__

#include "mamecore.h"

/* Print the MAME database in XML format */
void print_mame_xml(FILE* out, const game_driver* const games[], const char *gamename);

#endif	/* __INFO_H__ */
