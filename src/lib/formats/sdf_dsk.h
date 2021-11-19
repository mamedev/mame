// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, tim lindner
/*********************************************************************

    formats/sdf_dsk.h

    SDF disk images. Format created by Darren Atkinson for use with
    his CoCoSDC floppy disk emulator.

*********************************************************************/
#ifndef MAME_FORMATS_SDF_DSK_H
#define MAME_FORMATS_SDF_DSK_H

#pragma once

#include "flopimg.h"

/**************************************************************************/

class sdf_format : public floppy_image_format_t
{
public:
	sdf_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

protected:
	static constexpr int HEADER_SIZE  = 512;
	static constexpr int TRACK_HEADER_SIZE  = 256;
	static constexpr int TRACK_SIZE  = 6250;
	static constexpr int TRACK_PADDING  = 150;
	static constexpr int TOTAL_TRACK_SIZE  = TRACK_HEADER_SIZE + TRACK_SIZE + TRACK_PADDING;

	static constexpr int SECTOR_SLOT_COUNT  = 31;
};

extern const floppy_format_type FLOPPY_SDF_FORMAT;

#endif // MAME_FORMATS_SDF_DSK_H
