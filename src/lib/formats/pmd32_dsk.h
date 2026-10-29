// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*********************************************************************

    formats/pmd32_dsk.h

    Tesla PMD 32 disk images

*********************************************************************/
#ifndef MAME_FORMATS_PMD32_DSK_H
#define MAME_FORMATS_PMD32_DSK_H

#pragma once

#include "upd765_dsk.h"

class pmd32_format : public upd765_format
{
public:
	pmd32_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

protected:
	static const format formats[];
};

// The Consul 2717's PMD 32 disk drivers use 26 of the 36 logical sectors of
// each track (the 8" disc layout); images hold just those, track by track.
// Up to 80 logical tracks fit a 40-cylinder drive; DZ8 images (154 tracks)
// need an 80-cylinder drive.
class pmd32_c2717_format : public upd765_format
{
public:
	pmd32_c2717_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

private:
	static constexpr unsigned LOGICAL_TRACK_SIZE = 26 * 128;
	static constexpr unsigned SIDE_8INCH_TRACKS = 77;
	static constexpr unsigned DZ8_TRACKS = 154;

	static const format c2717_formats[];
};

extern const pmd32_format FLOPPY_PMD32_FORMAT;
extern const pmd32_c2717_format FLOPPY_PMD32_C2717_FORMAT;

#endif // MAME_FORMATS_PMD32_DSK_H
