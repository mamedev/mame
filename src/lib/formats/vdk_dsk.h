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

#include "flopimg.h"

class vdk_format : public floppy_image_format_t
{
public:
	vdk_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	static const int SECTOR_SIZE = 256;
	static const int SECTOR_COUNT = 18;
	static const int FIRST_SECTOR_ID = 1;
};

extern const floppy_format_type FLOPPY_VDK_FORMAT;

#endif // __VDK_DSK_H__
