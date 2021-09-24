// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    VDK

    Disk image format

    Used by Paul Burgin's PC-Dragon emulator

***************************************************************************/
#ifndef MAME_FORMATS_VDK_DSK_H
#define MAME_FORMATS_VDK_DSK_H

#pragma once

#include "flopimg.h"

class vdk_format : public floppy_image_format_t
{
public:
	vdk_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	static const int SECTOR_SIZE = 256;
	static const int SECTOR_COUNT = 18;
	static const int FIRST_SECTOR_ID = 1;
};

extern const floppy_format_type FLOPPY_VDK_FORMAT;

#endif // MAME_FORMATS_VDK_DSK_H
