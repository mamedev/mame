// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * flex_dsk.h
 *
 *  Created on: 24/06/2014
 */
#ifndef MAME_FORMATS_FLEX_DSK_H
#define MAME_FORMATS_FLEX_DSK_H

#pragma once

#include "flopimg.h"

class flex_format : public floppy_image_format_t
{
public:
	flex_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	struct sysinfo_sector
	{
		uint8_t unused1[16];
		uint8_t disk_name[8];
		uint8_t disk_ext[3];
		uint8_t disk_number[2];
		uint8_t fc_start_trk;
		uint8_t fc_start_sec;
		uint8_t fc_end_trk;
		uint8_t fc_end_sec;
		uint8_t free[2];
		uint8_t month;
		uint8_t day;
		uint8_t year;
		uint8_t last_trk;
		uint8_t last_sec;
		uint8_t unused2[216];
	} info;
};

extern const floppy_format_type FLOPPY_FLEX_FORMAT;

#endif // MAME_FORMATS_FLEX_DSK_H
