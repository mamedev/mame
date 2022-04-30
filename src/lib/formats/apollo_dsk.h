// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/apollo_dsk.h

    apollo format

*********************************************************************/
#ifndef MAME_FORMATS_APOLLO_DSK_H
#define MAME_FORMATS_APOLLO_DSK_H

#pragma once

#include "upd765_dsk.h"

class apollo_format : public upd765_format
{
public:
	apollo_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const apollo_format FLOPPY_APOLLO_FORMAT;

#endif // MAME_FORMATS_APOLLO_DSK_H
