/***************************************************************************

    mamecore.c

    Simple core functions that are defined in emucore.h and which may
    need to be accessed by other MAME-related tools.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************/

#include "emu.h"

void bitch_bad_cast(const std::type_info &src_type, const std::type_info &dst_type)
{
	fprintf(stderr, "Error: bad downcast<> or device<>.  Tried to convert a %s to a %s, which are incompatible.\n",
			src_type.name(), dst_type.name());
	abort();
}

void bitch_bad_device_cast(const device_t *dev, const std::type_info &dst_type)
{
	fprintf(stderr, "Error: bad downcast<> or device<>.  Tried to convert the device %s of type %s to a %s, which are incompatible.\n",
			dev->tag(), dev->name(), dst_type.name());
	abort();
}
