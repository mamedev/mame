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

#include "dmk_dsk.h"

/**************************************************************************/

class sdf_format : public dmk_format
{
public:
	sdf_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

protected:
	virtual bool write_image(util::random_read_write &io, int tracks, int heads, bool initial_mfm,
		std::vector<decoded_track> &data) const override;
	virtual bool read_header(util::random_read &io, disk_header &disk) const override;
	virtual bool read_track(util::random_read &io, const disk_header &disk, int track, int head,
		std::vector<uint8_t> &data, std::vector<int> &dams) const override;

	static constexpr int HEADER_SIZE  = 512;
	static constexpr int TRACK_HEADER_SIZE  = 256;
	static constexpr int TRACK_SIZE  = 6250;
	static constexpr int TRACK_PADDING  = 150;
	static constexpr int TOTAL_TRACK_SIZE  = TRACK_HEADER_SIZE + TRACK_SIZE + TRACK_PADDING;

	static constexpr int SECTOR_SLOT_COUNT  = 31;
};

extern const sdf_format FLOPPY_SDF_FORMAT;

#endif // MAME_FORMATS_SDF_DSK_H
