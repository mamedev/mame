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
#include "wd177x_dsk.h"

class flex_format : public wd177x_format
{
public:
	flex_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual int find_size(io_generic *io, uint32_t form_factor) override;
	virtual void build_sector_description(const format &f, uint8_t *sectdata, desc_s *sectors, int track, int head) const override;
	virtual void check_compatibility(floppy_image *image, std::vector<int> &candidates) override;

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
	static const format formats[];

	uint8_t boot0_sector_id;
	uint8_t boot1_sector_id;
};

extern const floppy_format_type FLOPPY_FLEX_FORMAT;

#endif // MAME_FORMATS_FLEX_DSK_H
