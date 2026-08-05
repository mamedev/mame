// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot PC/Xi

    Disk image format

***************************************************************************/

#ifndef MAME_FORMATS_APRICOTPC_DSK_H
#define MAME_FORMATS_APRICOTPC_DSK_H

#pragma once

#include "upd765_dsk.h"

class apricotpc_format : public upd765_format
{
public:
	apricotpc_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const apricotpc_format FLOPPY_APRICOTPC_FORMAT;

#endif // MAME_FORMATS_APRICOTPC_DSK_H
