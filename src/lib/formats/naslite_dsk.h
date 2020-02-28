// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/naslite_dsk.h

    NASLite 1.72MB with funky interleaving format

*********************************************************************/
#ifndef MAME_FORMATS_NASLITE_DSK_H
#define MAME_FORMATS_NASLITE_DSK_H

#pragma once

#include "upd765_dsk.h"

class naslite_format : public upd765_format
{
public:
	naslite_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format formats[];

	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;
};

extern const floppy_format_type FLOPPY_NASLITE_FORMAT;

#endif // MAME_FORMATS_NASLITE_DSK_H
