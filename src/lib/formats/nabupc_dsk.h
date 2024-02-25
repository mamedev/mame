// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*********************************************************************

    formats/nabupc_dsk.cpp

    NABU PC floppy-disk images

*********************************************************************/
#ifndef MAME_FORMATS_NABUPC_DSK_H
#define MAME_FORMATS_NABUPC_DSK_H

#pragma once

#include "flopimg.h"

class nabupc_format : public floppy_image_format_t {
public:
	struct format {
		uint32_t form_factor;      // See floppy_image for possible values
		uint32_t variant;          // See floppy_image for possible values
		int track_count;
		int head_count;
		int gap_1;
		int gap_2;
		int gap_3;
		uint8_t dpb[18];
	};

	nabupc_format();

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual bool supports_save() const noexcept override;

protected:
	static void build_nabu_track_mfm(int track, int head, floppy_image &image, int cell_count, int sector_count, const desc_pc_sector *sects, int gap_3, int gap_1, int gap_2, const uint8_t *dpb);
	int find_format(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const;
private:
	static constexpr int sector_count = 5;
	static constexpr int sector_size = 1024;
	static const format formats[];
};

extern const nabupc_format FLOPPY_NABUPC_FORMAT;

#endif // MAME_FORMATS_NABUPC_DSK_H
