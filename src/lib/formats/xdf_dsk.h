// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/xdf_dsk.h

    x68k bare-bones formats

*********************************************************************/
#ifndef MAME_FORMATS_XDF_DSK_H
#define MAME_FORMATS_XDF_DSK_H

#pragma once

#include "upd765_dsk.h"

class xdf_format : public upd765_format
{
public:
	xdf_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

class _2hc_format : public upd765_format
{
public:
	_2hc_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const format formats[];
};

extern const xdf_format FLOPPY_XDF_FORMAT;
extern const _2hc_format FLOPPY_2HC_FORMAT;

#endif // MAME_FORMATS_XDF_DSK_H
