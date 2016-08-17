// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/naslite_dsk.h

    NASLite 1.72MB with funky interleaving format

*********************************************************************/

#ifndef NASLITE_DSK_H_
#define NASLITE_DSK_H_

#include "upd765_dsk.h"

class naslite_format : public upd765_format {
public:
	naslite_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

protected:
	static const format formats[];

	virtual void build_sector_description(const format &d, UINT8 *sectdata, desc_s *sectors, int track, int head) const override;
};

extern const floppy_format_type FLOPPY_NASLITE_FORMAT;

#endif
