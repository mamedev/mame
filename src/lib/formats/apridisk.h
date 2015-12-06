// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    APRIDISK

    Disk image format for the ACT Apricot

***************************************************************************/

#pragma once

#ifndef __APRIDISK_H__
#define __APRIDISK_H__

#include "flopimg.h"

class apridisk_format : public floppy_image_format_t
{
public:
	apridisk_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	static const int APR_HEADER_SIZE = 128;

	// sector types
	enum
	{
		APR_DELETED = 0xe31d0000,
		APR_SECTOR  = 0xe31d0001,
		APR_COMMENT = 0xe31d0002,
		APR_CREATOR = 0xe31d0003
	};

	// compression types
	enum
	{
		APR_UNCOMPRESSED = 0x9e90,
		APR_COMPRESSED   = 0x3e5a
	};

	static const int SECTOR_SIZE = 512;
	static const int MAX_SECTORS = 2880;  // enough for a hd disk image
};

extern const floppy_format_type FLOPPY_APRIDISK_FORMAT;

#endif // __APRIDISK_H__
