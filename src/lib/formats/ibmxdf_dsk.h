// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/xdf_dsk.h

    IBM Extended Density Format

*********************************************************************/
#ifndef MAME_FORMATS_IBMXDF_DSK_H
#define MAME_FORMATS_IBMXDF_DSK_H

#pragma once

#include "wd177x_dsk.h"

class ibmxdf_format : public wd177x_format
{
public:
	ibmxdf_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool supports_save() const override { return false; }

	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual int get_image_offset(const format &f, int head, int track) override;
	virtual const wd177x_format::format &get_track_format(const format &f, int head, int track) override;

private:
	static const format formats[];
	static const format formats_head1[];
	static const format formats_track0[];
	static const format formats_head1_track0[];

	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;
};

extern const floppy_format_type FLOPPY_IBMXDF_FORMAT;

#endif // MAME_FORMATS_IBMXDF_DSK_H
