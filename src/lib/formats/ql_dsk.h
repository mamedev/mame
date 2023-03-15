// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/ql_dsk.h

    Sinclair QL disk image formats

*********************************************************************/
#ifndef MAME_FORMATS_QL_DSK_H
#define MAME_FORMATS_QL_DSK_H

#pragma once

#include "wd177x_dsk.h"

class ql_format : public wd177x_format
{
public:
	ql_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const ql_format FLOPPY_QL_FORMAT;

#endif // MAME_FORMATS_QL_DSK_H
