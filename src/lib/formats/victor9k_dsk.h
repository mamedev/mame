// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/victor9k_dsk.h

    Victor 9000 sector disk image format

*********************************************************************/
#ifndef MAME_FORMATS_VICTOR9K_DSK_H
#define MAME_FORMATS_VICTOR9K_DSK_H

#pragma once

#include "flopimg.h"

//#define USE_SCP 1

class victor9k_format : public floppy_image_format_t {
public:
	struct format {
		uint32_t form_factor;      // See floppy_image for possible values
		uint32_t variant;          // See floppy_image for possible values

		uint16_t sector_count;
		uint8_t track_count;
		uint8_t head_count;
		uint16_t sector_base_size;
	};

	victor9k_format();

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool supports_save() const override { return true; }

	static int get_rpm(int head, int track);

protected:
	static const format formats[];

	static const uint32_t cell_size[9];
	static const int sectors_per_track[2][80];
	static const int speed_zone[2][80];
	static const int rpm[9];

	static int find_size(util::random_read &io, uint32_t form_factor);
	static void log_boot_sector(uint8_t *data);
	static floppy_image_format_t::desc_e* get_sector_desc(const format &f, int &current_size, int sector_count);
	static void build_sector_description(const format &f, uint8_t *sectdata, uint32_t sect_offs, desc_s *sectors, int sector_count);
	static int get_image_offset(const format &f, int head, int track);
	static int compute_track_size(const format &f, int head, int track);
	static void extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head, int sector_count);
};

extern const victor9k_format FLOPPY_VICTOR_9000_FORMAT;


#endif // MAME_FORMATS_VICTOR9K_DSK_H
