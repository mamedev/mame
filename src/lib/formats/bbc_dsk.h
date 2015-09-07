// license:GPL-2.0+
// copyright-holders:Dirk Best, Nigel Barnes
/***************************************************************************

    BBC Micro

    Disk image formats

***************************************************************************/

#pragma once

#ifndef __BBC_DSK_H__
#define __BBC_DSK_H__

#include "flopimg.h"
#include "wd177x_dsk.h"

/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(bbc);

/**************************************************************************/

class bbc_ssd_525_format : public wd177x_format
{
public:
	bbc_ssd_525_format();

	virtual int find_size(io_generic *io, UINT32 form_factor);
	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

class bbc_dsd_525_format : public wd177x_format
{
public:
	bbc_dsd_525_format();

	virtual int find_size(io_generic *io, UINT32 form_factor);
	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

class bbc_adf_525_format : public wd177x_format
{
public:
	bbc_adf_525_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};

class bbc_adf_35_format : public wd177x_format
{
public:
	bbc_adf_35_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

private:
	static const format formats[];
};


extern const floppy_format_type FLOPPY_BBC_SSD_525_FORMAT;
extern const floppy_format_type FLOPPY_BBC_DSD_525_FORMAT;
extern const floppy_format_type FLOPPY_BBC_ADF_525_FORMAT;
extern const floppy_format_type FLOPPY_BBC_ADF_35_FORMAT;

#endif // __BBC_DSK_H__
