/***************************************************************************

    validity.h

    Validity checks

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __VALIDITY_H__
#define __VALIDITY_H__

void validate_drivers(emu_options &options, const game_driver *driver = NULL);
bool validate_tag(const game_driver &driver, const char *object, const char *tag);

#endif
