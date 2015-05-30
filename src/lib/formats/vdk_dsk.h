// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VDK

    Disk image format

    Used by Paul Burgin's PC-Dragon emulator

***************************************************************************/

#pragma once

#ifndef __VDK_DSK_H__
#define __VDK_DSK_H__

#include "wd177x_dsk.h"

class vdk_format : public wd177x_format
{
public:
	vdk_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

private:
	format m_format;
};

extern const floppy_format_type FLOPPY_VDK_FORMAT;

#endif // __VDK_DSK_H__
