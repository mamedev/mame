// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Hector Minidisc

    Disk image format

***************************************************************************/

#pragma once

#ifndef __HMD_DSK_H__
#define __HMD_DSK_H__

#include "upd765_dsk.h"

class hmd_format : public upd765_format
{
public:
	hmd_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_HMD_FORMAT;

#endif // __HMD_DSK_H__
