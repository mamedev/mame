// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,tim lindner
/*********************************************************************

    formats/dmk_dsk.h

    DMK disk images

*********************************************************************/
#ifndef MAME_FORMATS_DMK_DSK_H
#define MAME_FORMATS_DMK_DSK_H

#pragma once

#include "flopimg.h"

/**************************************************************************/

class dmk_format : public floppy_image_format_t
{
public:
	dmk_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
	virtual bool supports_save() const noexcept override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

protected:
	// Derived containers translate their headers to DMK offsets and density flags.
	static constexpr int TRACK_HEADER_SIZE = 0x80;
	static constexpr int MAX_IDAMS = TRACK_HEADER_SIZE / 2;

	struct disk_header
	{
		int tracks;
		int heads;
		int track_size;
		uint8_t flags;

		int fm_stride() const { return (flags & 0xc0) ? 1 : 2; }
	};

	struct decoded_track
	{
		std::vector<uint8_t> data;
		std::vector<int> dams;
	};

	// The shared decoder produces DMK byte data plus exact data-mark offsets.
	// Container writers validate and serialize all tracks before returning.
	virtual bool write_image(util::random_read_write &io, int tracks, int heads, bool initial_mfm,
		std::vector<decoded_track> &data) const;

	virtual bool read_header(util::random_read &io, disk_header &disk) const;
	// An empty DAM table requests DMK's implicit search.  Otherwise it has
	// one explicit mark offset per IDAM, with zero denoting a missing field.
	virtual bool read_track(util::random_read &io, const disk_header &disk, int track, int head,
		std::vector<uint8_t> &data, std::vector<int> &dams) const;
};

extern const dmk_format FLOPPY_DMK_FORMAT;

#endif // MAME_FORMATS_DMK_DSK_H
