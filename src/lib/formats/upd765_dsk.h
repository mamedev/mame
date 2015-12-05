// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/upd765_dsk.h

    helper for simple upd765-formatted disk images

*********************************************************************/

#ifndef UPD765_DSK_H
#define UPD765_DSK_H

#include "flopimg.h"

class upd765_format : public floppy_image_format_t
{
public:
	struct format {
		UINT32 form_factor;      // See floppy_image for possible values
		UINT32 variant;          // See floppy_image for possible values
		UINT32 encoding;         // See floppy_image for possible values

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

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;
	virtual bool supports_save() const override;

protected:
	floppy_image_format_t::desc_e* get_desc_fm(const format &f, int &current_size, int &end_gap_index);
	floppy_image_format_t::desc_e* get_desc_mfm(const format &f, int &current_size, int &end_gap_index);
	int find_size(io_generic *io, UINT32 form_factor);
	int compute_track_size(const format &f) const;
	virtual void build_sector_description(const format &d, UINT8 *sectdata, desc_s *sectors, int track, int head) const;
	void check_compatibility(floppy_image *image, std::vector<int> &candidates);
	void extract_sectors(floppy_image *image, const format &f, desc_s *sdesc, int track, int head);

private:
	const format *formats;
};

#endif /* UPD765_DSK_H */
