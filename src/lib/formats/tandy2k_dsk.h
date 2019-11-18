// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/tandy2k_dsk.h

    Tandy 2000 disk format

*********************************************************************/
#ifndef MAME_FORMATS_TANDY2K_DSK_H
#define MAME_FORMATS_TANDY2K_DSK_H

#pragma once

#include "upd765_dsk.h"

class tandy2k_format : public upd765_format
{
public:
	tandy2k_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const format formats[];
};

extern const floppy_format_type FLOPPY_TANDY_2000_FORMAT;

#endif // MAME_FORMATS_TANDY2K_DSK_H
