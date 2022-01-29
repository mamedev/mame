// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq8_dsk.h

    Formats for 8-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 40 tracks, single (Mirage) or double (SQ-80) sided,
    with 6 sectors per track.
    Sectors 0-4 are 1024 bytes, sector 5 is 512 bytes

*********************************************************************/
#ifndef MAME_FORMATS_ESQ8_DSK_H
#define MAME_FORMATS_ESQ8_DSK_H

#pragma once

#include "flopimg.h"

class esq8img_format : public floppy_image_format_t
{
public:
	esq8img_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e esq_6_desc[];

private:
	void find_size(util::random_read &io, int &track_count, int &head_count, int &sector_count);
};

extern const floppy_format_type FLOPPY_ESQ8IMG_FORMAT;

#endif // MAME_FORMATS_ESQ8_DSK_H
