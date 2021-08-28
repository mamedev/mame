// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_FORMATS_PASTI_DSK_H
#define MAME_FORMATS_PASTI_DSK_H

#pragma once

#include "flopimg.h"

class pasti_format : public floppy_image_format_t
{
public:
	pasti_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e xdesc[];

protected:
	struct wd_sect {
		const uint8_t *data;
		const uint8_t *fuzzy_mask;
		uint8_t id[6];
		int position;
		double time_ratio;
	};

	struct wd_obs {
		const uint8_t *track_data;
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
	void match_mfm_data(wd_obs &obs, int tpos, const uint8_t *data, int size, uint8_t context, int &bcount, int &ccount, bool &synced);
	void match_raw_data(wd_obs &obs, int tpos, const uint8_t *data, int size, uint8_t context, int &bcount, int &ccount);
	uint16_t byte_to_mfm(uint8_t data, bool context);
	uint16_t calc_crc(const uint8_t *data, int size, uint16_t crc);

	void wd_generate_unsynced_gap(std::vector<uint32_t> &track, const wd_obs &obs, int tstart, int tend, uint32_t cell_size);
	void wd_generate_synced_gap(std::vector<uint32_t> &track, const wd_obs &obs, int tstart, int tend, uint32_t cell_size);
	void wd_generate_gap(std::vector<uint32_t> &track, const wd_obs &obs, int tstart, int tend, bool synced, uint32_t cell_size_start, uint32_t cell_size_end);
	void wd_generate_sector_header(std::vector<uint32_t> &track, const wd_obs &obs, int sector, int tstart, uint32_t cell_size);
	void wd_generate_sector_data(std::vector<uint32_t> &track, const wd_obs &obs, int sector, int tstart, uint32_t cell_size);
	void wd_generate_track_from_sectors_and_track(int track, int head, floppy_image *image, wd_obs &obs);
	void wd_generate_track_from_sectors_only(int track, int head, floppy_image *image, wd_obs &obs);
};

extern const floppy_format_type FLOPPY_PASTI_FORMAT;

#endif // MAME_FORMATS_PASTI_DSK_H
