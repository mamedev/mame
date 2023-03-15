// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/tvc_dsk.h

    Videoton TVC HBF format

*********************************************************************/
#ifndef MAME_FORMATS_TVC_DSK_H
#define MAME_FORMATS_TVC_DSK_H

#pragma once

#include "wd177x_dsk.h"

class tvc_format : public wd177x_format
{
public:
	tvc_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const tvc_format FLOPPY_TVC_FORMAT;

#endif // MAME_FORMATS_TVC_DSK_H
