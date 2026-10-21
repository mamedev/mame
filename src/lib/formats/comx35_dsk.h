// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/comx35_dsk.h

    COMX-35 disk image format

*********************************************************************/
#ifndef MAME_FORMATS_COMX35_DSK_H
#define MAME_FORMATS_COMX35_DSK_H

#pragma once

#include "wd177x_dsk.h"

class comx35_format : public wd177x_format
{
public:
	comx35_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

protected:
	virtual int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual floppy_image_format_t::desc_e *get_desc_fm(const format &f, int &current_size, int &end_gap_index) const override;

private:
	static const format formats[];
};

extern const comx35_format FLOPPY_COMX35_FORMAT;

#endif // MAME_FORMATS_COMX35_DSK_H
