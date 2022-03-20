// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    TRS-80

    JV1 disk image format

***************************************************************************/
#ifndef MAME_FORMATS_TRS80_DSK_H
#define MAME_FORMATS_TRS80_DSK_H

#pragma once

#include "wd177x_dsk.h"

class jv1_format : public wd177x_format
{
public:
	jv1_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	virtual int get_track_dam_fm(const format &f, int head, int track) override;

private:
	static const format formats[];
};

class jv3_format : public floppy_image_format_t
{
public:
	jv3_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_JV1_FORMAT;
extern const floppy_format_type FLOPPY_JV3_FORMAT;

#endif // MAME_FORMATS_TRS80_DSK_H
