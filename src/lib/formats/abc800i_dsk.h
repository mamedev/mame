// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800i_dsk.h

    Luxor ABC 830 interleaved disk image formats

*********************************************************************/
#ifndef MAME_FORMATS_ABC800I_DSK_H
#define MAME_FORMATS_ABC800I_DSK_H

#pragma once

#include "wd177x_dsk.h"

class abc800i_format : public wd177x_format
{
public:
	abc800i_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;

protected:
	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;

private:
	static const format formats[];
};

extern const abc800i_format FLOPPY_ABC800I_FORMAT;

#endif // MAME_FORMATS_ABC800I_DSK_H
