// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/abc800_dsk.h

    Luxor ABC 830/832/834/838 disk image formats

*********************************************************************/
#ifndef MAME_FORMATS_ABC800_DSK_H
#define MAME_FORMATS_ABC800_DSK_H

#pragma once

#include "wd177x_dsk.h"

class abc800_format : public wd177x_format
{
public:
	abc800_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;

private:
	static const format formats[];
};

extern const abc800_format FLOPPY_ABC800_FORMAT;

#endif // MAME_FORMATS_ABC800_DSK_H
