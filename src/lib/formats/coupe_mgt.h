// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe MGT Disk Image

***************************************************************************/

#ifndef MAME_FORMATS_COUPE_MGT_H
#define MAME_FORMATS_COUPE_MGT_H

#pragma once

#include "wd177x_dsk.h"

class coupe_mgt_format : public wd177x_format
{
public:
	coupe_mgt_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

protected:

private:
	static const format formats[];
};

extern const coupe_mgt_format FLOPPY_COUPE_MGT_FORMAT;

#endif // MAME_FORMATS_COUPE_MGT_H
