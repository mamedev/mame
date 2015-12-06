// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Thompson CD90-640 FDC

    Disk image format

***************************************************************************/

#pragma once

#ifndef __CD90_640_DSK_H__
#define __CD90_640_DSK_H__

#include "wd177x_dsk.h"

class cd90_640_format : public wd177x_format
{
public:
	cd90_640_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_CD90_640_FORMAT;

#endif // __CD90_640_DSK_H__
