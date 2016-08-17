// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef PASTI_DSK_H_
#define PASTI_DSK_H_

#include "flopimg.h"

class pasti_format : public floppy_image_format_t
{
public:
	pasti_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e xdesc[];

protected:
	struct wd_sect {
		const UINT8 *data;
		const UINT8 *fuzzy_mask;
		UINT8 id[6];
		int position;
		double time_ratio;
	};

	struct wd_obs {
		const UINT8 *track_data;
		wd_sect sectors[256];
		int track_size, sector_count;
	};

	void wd_generate_track_from_observations(int track, int head, floppy_image *image, wd_obs &obs);

private:
	struct wd_sect_info {
		int hstart, hend;
		int dstart, dend;
		bool hsynced, dsynced;
	};

	void map_sectors_in_track(wd_obs &obs, wd_sect_info *sect_infos);
	void match_mfm_data(wd_obs &obs, int tpos, const UINT8 *data, int size, UINT8 context, int &bcount, int &ccount, bool &synced);
	void match_raw_data(wd_obs &obs, int tpos, const UINT8 *data, int size, UINT8 context, int &bcount, int &ccount);
	UINT16 byte_to_mfm(UINT8 data, bool context);
	UINT16 calc_crc(const UINT8 *data, int size, UINT16 crc);

	void wd_generate_unsynced_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, UINT32 cell_size);
	void wd_generate_synced_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, UINT32 cell_size);
	void wd_generate_gap(std::vector<UINT32> &track, const wd_obs &obs, int tstart, int tend, bool synced, UINT32 cell_size_start, UINT32 cell_size_end);
	void wd_generate_sector_header(std::vector<UINT32> &track, const wd_obs &obs, int sector, int tstart, UINT32 cell_size);
	void wd_generate_sector_data(std::vector<UINT32> &track, const wd_obs &obs, int sector, int tstart, UINT32 cell_size);
	void wd_generate_track_from_sectors_and_track(int track, int head, floppy_image *image, wd_obs &obs);
	void wd_generate_track_from_sectors_only(int track, int head, floppy_image *image, wd_obs &obs);
};

extern const floppy_format_type FLOPPY_PASTI_FORMAT;

#endif /*PASTI_DSK_H_*/
