// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe SAD Disk Image

***************************************************************************/

#ifndef MAME_FORMATS_COUPE_SAD_H
#define MAME_FORMATS_COUPE_SAD_H

#pragma once

#include "wd177x_dsk.h"

class coupe_sad_format : public wd177x_format
{
public:
	coupe_sad_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

protected:
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual int get_image_offset(const format &f, int head, int track) const override;

private:
	static constexpr unsigned HEADER_SIZE = 22;

	static const format formats[];
};

extern const coupe_sad_format FLOPPY_COUPE_SAD_FORMAT;

#endif // MAME_FORMATS_COUPE_SAD_H
