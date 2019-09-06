// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    JVC

    Disk image format

    Named after its creator, Jeff Vavasour

***************************************************************************/
#ifndef MAME_FORMATS_JVC_DSK_H
#define MAME_FORMATS_JVC_DSK_H

#pragma once

#include "flopimg.h"

class jvc_format : public floppy_image_format_t
{
public:
	jvc_format();

	struct jvc_header
	{
		uint8_t sectors_per_track;
		uint8_t side_count;
		uint8_t sector_size;
		uint8_t first_sector_id;
		uint8_t sector_attribute_flag;
		int header_size;
	};

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;
	virtual bool supports_save() const override;

private:
	bool parse_header(io_generic *io, int &header_size, int &tracks, int &heads, int &sectors, int &sector_size, int &base_sector_id);
};

extern const floppy_format_type FLOPPY_JVC_FORMAT;

#endif // MAME_FORMATS_JVC_DSK_H
