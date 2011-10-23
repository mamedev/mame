#ifndef IPF_DSK_H_
#define IPF_DSK_H_

#include "flopimg.h"

class ipf_format : public floppy_image_format_t
{
public:
	ipf_format();

	virtual int identify(io_generic *io);
	virtual bool load(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

private:
	struct track_info {
		UINT32 cylinder, head, type;
		UINT32 t12, t44, t56, t60, t64;
		UINT32 size_bytes, size_cells;
		UINT32 index_bytes, index_cells;
		UINT32 datasize_cells, gapsize_cells;
		UINT32 block_count, weak_bits;

		UINT32 d4;

		bool info_set;

		const UINT8 *data;
		UINT32 data_size;
	};

	track_info *tinfos;
	UINT32 tcount;

	UINT32 type, release, revision;
	UINT32 f4, f8, f14;
	UINT32 min_cylinder, max_cylinder, min_head, max_head;
	UINT32 credit_day, credit_time;
	UINT32 platform[4], extra[5];

	UINT32 crc32r(const UINT8 *data, UINT32 size);

	bool parse_info(const UINT8 *info);
	bool parse_imge(const UINT8 *imge);
	bool parse_data(const UINT8 *data, UINT32 &pos, UINT32 max_extra_size);

	bool scan_one_tag(UINT8 *data, UINT32 size, UINT32 &pos, UINT8 *&tag, UINT32 &tsize);
	bool scan_all_tags(UINT8 *data, UINT32 size);
	static UINT32 r32(const UINT8 *p);
	static UINT32 rb(const UINT8 *&p, int count);

	track_info *get_index(UINT32 idx);

	void track_write_raw(UINT32 *&track, const UINT8 *data, UINT32 cells, bool &context);
	void track_write_mfm(UINT32 *&track, const UINT8 *data, UINT32 start_offset, UINT32 patlen, UINT32 cells, bool &context);
	void track_write_weak(UINT32 *&track, UINT32 cells);
	bool generate_block_data(const UINT8 *data, const UINT8 *dlimit, UINT32 *track, UINT32 *tlimit, bool &context);

	bool gap_description_to_reserved_size(const UINT8 *&data, const UINT8 *dlimit, UINT32 &res_size);
	bool generate_gap_from_description(const UINT8 *&data, const UINT8 *dlimit, UINT32 *track, UINT32 size, bool pre, bool &context);
	bool generate_block_gap_0(UINT32 gap_cells, UINT8 pattern, UINT32 &spos, UINT32 ipos, UINT32 *track, bool &context);
	bool generate_block_gap_1(UINT32 gap_cells, UINT32 &spos, const UINT8 *data, const UINT8 *dlimit, UINT32 *track, bool &context);
	bool generate_block_gap_2(UINT32 gap_cells, UINT32 &spos, const UINT8 *data, const UINT8 *dlimit, UINT32 *track, bool &context);
	bool generate_block_gap_3(UINT32 gap_cells, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, UINT32 *track, bool &context);
	bool generate_block_gap(UINT32 gap_type, UINT32 gap_cells, UINT8 pattern, UINT32 &spos, UINT32 ipos, const UINT8 *data, const UINT8 *dlimit, UINT32 *track, bool &context);

	bool generate_block(track_info *t, UINT32 idx, UINT32 ipos, UINT32 *track, UINT32 &pos, UINT32 &dpos, UINT32 &gpos, UINT32 &spos, bool &context);
	UINT32 block_compute_real_size(track_info *t);

	void timing_set(UINT32 *track, UINT32 start, UINT32 end, UINT32 time);
	bool generate_timings(track_info *t, UINT32 *track, const UINT32 *data_pos, const UINT32 *gap_pos);

	void rotate(UINT32 *track, UINT32 offset, UINT32 size);
	void mark_track_splice(UINT32 *t);
	bool generate_track(track_info *t, floppy_image *image);
	bool generate_tracks(floppy_image *image);

	bool parse(UINT8 *data, UINT32 size, floppy_image *image);
};

extern const floppy_format_type FLOPPY_IPF_FORMAT;

#endif /*IPF_DSK_H_*/
