// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe SDF Disk Image

***************************************************************************/

#ifndef MAME_FORMATS_COUPE_SDF_H
#define MAME_FORMATS_COUPE_SDF_H

#pragma once

#include "flopimg.h"

class coupe_sdf_format : public floppy_image_format_t
{
public:
	coupe_sdf_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

private:
	static constexpr uint8_t SDF_CRC_ERROR = 0x08;
	static constexpr uint8_t SDF_RECORD_NOT_FOUND = 0x10;
	static constexpr uint8_t SDF_DELETED_DATA = 0x20;

	static constexpr unsigned SDF_TRACK_SIZE = 6144;
	static constexpr unsigned SDF_SIDES = 2;
};

extern const coupe_sdf_format FLOPPY_COUPE_SDF_FORMAT;

#endif // MAME_FORMATS_COUPE_SDF_H
