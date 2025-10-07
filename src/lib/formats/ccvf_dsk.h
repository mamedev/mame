// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ccvf_dsk.h

    Compucolor Virtual Floppy Disk Image format

*********************************************************************/
#ifndef MAME_FORMATS_CCVF_DSK_H
#define MAME_FORMATS_CCVF_DSK_H

#pragma once

#include "flopimg.h"

class ccvf_format : public floppy_image_format_t
{
public:
	struct format
	{
		uint32_t form_factor;    // See floppy_image for possible values
		uint32_t variant;        // See floppy_image for possible values

		int cell_size;           // See floppy_image_format_t for details
		int sector_count;
		int track_count;
		int head_count;
		int sector_base_size;
		int per_sector_size[40]; // if sector_base_size is 0
		int sector_base_id;      // 0 or 1 usually, -1 if there's interleave
		int per_sector_id[40];   // if sector_base_id is -1.  If both per are used, then sector per_sector_id[i] has size per_sector_size[i]
	};

	ccvf_format();
	ccvf_format(const format *formats);

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

protected:
	const format *formats;

	static floppy_image_format_t::desc_e* get_desc_8n1(const format &f, int &current_size);

	static const format file_formats[];
};

extern const ccvf_format FLOPPY_CCVF_FORMAT;

#endif // MAME_FORMATS_CCVF_DSK_H
