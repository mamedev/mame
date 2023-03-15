// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/ppg_dsk.h

    ppg format

*********************************************************************/
#ifndef MAME_FORMATS_PPG_DSK_H
#define MAME_FORMATS_PPG_DSK_H

#pragma once

#include "wd177x_dsk.h"

class ppg_format : public wd177x_format
{
public:
	ppg_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];

	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;
};

extern const ppg_format FLOPPY_PPG_FORMAT;

#endif // MAME_FORMATS_PPG_DSK_H
