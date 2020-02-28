// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_FORMATS_IPF_DSK_H
#define MAME_FORMATS_IPF_DSK_H

#pragma once

#include "flopimg.h"

#include <vector>

class ipf_format : public floppy_image_format_t
{
public:
	ipf_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	struct track_info {
		uint32_t cylinder, head, type;
		uint32_t sigtype, process, reserved[3];
		uint32_t size_bytes, size_cells;
		uint32_t index_bytes, index_cells;
		uint32_t datasize_cells, gapsize_cells;
		uint32_t block_count, weak_bits;

		uint32_t data_size_bits;

		bool info_set;

		const uint8_t *data;
		uint32_t data_size;
	};

	track_info *tinfos;
	uint32_t tcount;

	uint32_t type, release, revision;
	uint32_t encoder_type, encoder_revision, origin;
	uint32_t min_cylinder, max_cylinder, min_head, max_head;
	uint32_t credit_day, credit_time;
	uint32_t platform[4], extra[5];

	uint32_t crc32r(const uint8_t *data, uint32_t size);

	bool parse_info(const uint8_t *info);
	bool parse_imge(const uint8_t *imge);
	bool parse_data(const uint8_t *data, uint32_t &pos, uint32_t max_extra_size);

	bool scan_one_tag(std::vector<uint8_t> &data, uint32_t &pos, uint8_t *&tag, uint32_t &tsize);
	bool scan_all_tags(std::vector<uint8_t> &data);
	static uint32_t r32(const uint8_t *p);
	static uint32_t rb(const uint8_t *&p, int count);

	track_info *get_index(uint32_t idx);

	void track_write_raw(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t cells, bool &context);
	void track_write_mfm(std::vector<uint32_t>::iterator &tpos, const uint8_t *data, uint32_t start_offset, uint32_t patlen, uint32_t cells, bool &context);
	void track_write_weak(std::vector<uint32_t>::iterator &tpos, uint32_t cells);
	bool generate_block_data(const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, std::vector<uint32_t>::iterator tlimit, bool &context);

	bool gap_description_to_reserved_size(const uint8_t *&data, const uint8_t *dlimit, uint32_t &res_size);
	bool generate_gap_from_description(const uint8_t *&data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, uint32_t size, bool pre, bool &context);
	bool generate_block_gap_0(uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_1(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_2(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap_3(uint32_t gap_cells, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator &tpos, bool &context);
	bool generate_block_gap(uint32_t gap_type, uint32_t gap_cells, uint8_t pattern, uint32_t &spos, uint32_t ipos, const uint8_t *data, const uint8_t *dlimit, std::vector<uint32_t>::iterator tpos, bool &context);

	bool generate_block(track_info *t, uint32_t idx, uint32_t ipos, std::vector<uint32_t> &track, uint32_t &pos, uint32_t &dpos, uint32_t &gpos, uint32_t &spos, bool &context);
	uint32_t block_compute_real_size(track_info *t);

	void timing_set(std::vector<uint32_t> &track, uint32_t start, uint32_t end, uint32_t time);
	bool generate_timings(track_info *t, std::vector<uint32_t> &track, const std::vector<uint32_t> &data_pos, const std::vector<uint32_t> &gap_pos);

	void rotate(std::vector<uint32_t> &track, uint32_t offset, uint32_t size);
	void mark_track_splice(std::vector<uint32_t> &track, uint32_t offset, uint32_t size);
	bool generate_track(track_info *t, floppy_image *image);
	bool generate_tracks(floppy_image *image);

	bool parse(std::vector<uint8_t> &data, floppy_image *image);
};

extern const floppy_format_type FLOPPY_IPF_FORMAT;

#endif // MAME_FORMATS_IPF_DSK_H
