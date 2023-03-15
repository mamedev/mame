// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/upd765_dsk.h

    helper for simple upd765-formatted disk images

*********************************************************************/
#ifndef MAME_FORMATS_UPD765_DSK_H
#define MAME_FORMATS_UPD765_DSK_H

#pragma once

#include "flopimg.h"

class upd765_format : public floppy_image_format_t
{
public:
	struct format {
		uint32_t form_factor = 0U; // See floppy_image for possible values
		uint32_t variant = 0U;   // See floppy_image for possible values
		uint32_t encoding = 0U;  // See floppy_image for possible values

		int cell_size;           // See floppy_image_format_t for details
		int sector_count;
		int track_count;
		int head_count;
		int sector_base_size;
		int per_sector_size[40]; // if sector_base_size is 0
		int sector_base_id;      // 0 or 1 usually, -1 if there's interleave
		int per_sector_id[40];   // if sector_base_id is -1.  If both per are used, then sector per_sector_id[i] has size per_sector_size[i]
		int gap_4a;              // Usually 80 - number of 4e between index and IAM sync
		int gap_1;               // Usually 50 - number of 4e between IAM and first IDAM sync
		int gap_2;               // 22 for <=1.44Mb, 41 for 2.88Mb - number of 4e between sector header and data sync
		int gap_3;               // Usually 84 - number of 4e between sector crc and next IDAM
	};

	// End the array with {}
	upd765_format(const format *formats);

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool supports_save() const override;

protected:
	floppy_image_format_t::desc_e* get_desc_fm(const format &f, int &current_size, int &end_gap_index) const;
	floppy_image_format_t::desc_e* get_desc_mfm(const format &f, int &current_size, int &end_gap_index) const;
	int find_size(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const;
	int compute_track_size(const format &f) const;
	virtual void build_sector_description(const format &d, uint8_t *sectdata, desc_s *sectors, int track, int head) const;
	void check_compatibility(floppy_image *image, std::vector<int> &candidates) const;
	void extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head) const;

private:
	format const *const formats;
};

#endif // MAME_FORMATS_UPD765_DSK_H
