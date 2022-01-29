// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp85_tape.cpp

    HP-85 tape format

*********************************************************************/
#include "imgtool.h"

#include "formats/hti_tape.h"
#include "formats/imageutl.h"

#include "ioprocs.h"
#include "opresolv.h"

#include <cstdio>
#include <iostream>


// Constants
static constexpr unsigned CHARS_PER_FNAME = 6;  // Characters in a filename
static constexpr unsigned CHARS_PER_EXT = 4;    // Characters in (simulated) file extension
static constexpr unsigned CHARS_PER_FNAME_EXT = CHARS_PER_FNAME + 1 + CHARS_PER_EXT;    // Characters in filename + extension
static constexpr unsigned MAX_FILE_NO = 42;     // Maximum file #
static constexpr unsigned MAX_RECORD_SIZE = 256;    // Maximum size of record body
static constexpr unsigned MAX_N_RECORDS = 436 * 2 - 5;  // Total user-available capacity of tape (in physical records)
static constexpr unsigned DIR_RECORDS = 4;  // Records reserved to directory

// Words stored on tape
using tape_word_t = hti_format_t::tape_word_t;

// Tape position, 1 unit = 1 inch / (968 * 1024)
using tape_pos_t = hti_format_t::tape_pos_t;

// File number [0..42]
typedef uint8_t file_no_t;

// Minimum gap size to detect IFGs: 1.25"
static constexpr tape_pos_t MIN_IFG_SIZE = 1.25 * hti_format_t::ONE_INCH_POS;

// Minimum gap size to detect IRGs: 1/32 "
static constexpr tape_pos_t MIN_IRG_SIZE = hti_format_t::ONE_INCH_POS / 32;

// Formatted size of IFGs: 2.5"
static constexpr tape_pos_t FMT_IFG_SIZE = 2.5 * hti_format_t::ONE_INCH_POS;

// Formatted size of IRGs: 1"
static constexpr tape_pos_t FMT_IRG_SIZE = hti_format_t::ONE_INCH_POS;

// Formatted size of records: 2.67"
static constexpr tape_pos_t FMT_REC_SIZE = 2.67 * hti_format_t::ONE_INCH_POS;

// Starting position on tracks: 74" from beginning of tape
static constexpr tape_pos_t TRACK_START = 74 * hti_format_t::ONE_INCH_POS;

// Sync word: 0x0001
static constexpr tape_word_t SYNC_WORD = 1;

// Name of erased (NULL) files
static const char *const NULL_FILENAME = "==NULL==";

// Masks of bits in file type
static constexpr uint8_t FT_NEXT_AV_MASK = 0x80;    // Next available slot
static constexpr uint8_t FT_NULL_FILE_MASK = 0x40;  // Erased file
static constexpr uint8_t FT_PROG_MASK = 0x20;       // PROG (BASIC) file
static constexpr uint8_t FT_DATA_MASK = 0x10;       // DATA file
static constexpr uint8_t FT_BPGM_MASK = 0x08;       // BPGM file
static constexpr uint8_t FT_WP_MASK = 0x02;         // Write protection
static constexpr uint8_t FT_HIDDEN_MASK = 0x01;     // Hidden file

/********************************************************************************
 * Directory entries
 ********************************************************************************/
struct dir_entry_85 {
	uint8_t filename[ CHARS_PER_FNAME ];  // Filename (left justified, space-padded on the right)
	file_no_t file_no;  // File #
	uint8_t filetype;   // File type
	uint16_t n_recs;    // Physical records
	uint16_t record_len;    // Length of logical records
};

/********************************************************************************
 * Tape image
 ********************************************************************************/
class tape_image_85 {
public:
	tape_image_85(void);

	bool is_dirty(void) const { return dirty; }

	void format_img(void);

	imgtoolerr_t load_from_file(imgtool::stream *stream);
	typedef std::vector<uint8_t> sif_file_t;
	typedef std::unique_ptr<sif_file_t> sif_file_ptr_t;
	bool load_sif_file(file_no_t file_no , sif_file_t& out);
	imgtoolerr_t save_to_file(imgtool::stream *stream);

	bool get_dir_entry(unsigned idx , const dir_entry_85*& entry) const;
	bool find_file(const char *filename , bool ignore_ext , unsigned& idx) const;
	bool alloc_new_file(dir_entry_85*& entry , sif_file_ptr_t&& file_data);
	bool delete_dir_entry(unsigned idx);
	bool finalize_allocation();
	static void get_filename_and_ext(const dir_entry_85& ent , bool inc_ext , char *out);
	static void split_filename_and_ext(const char *filename , char *fname , char *ext);

private:
	// Tape image
	hti_format_t image;
	bool dirty;
	// Directory
	std::vector<dir_entry_85> dir;
	// Content
	std::vector<sif_file_ptr_t> content;
	// First file on track 1
	file_no_t file_track_1;
	// No. of first record on track 1
	uint16_t record_track_1;

	bool dec_rec_header(const tape_word_t *hdr , file_no_t& file_no , uint16_t& rec_no , bool& has_body , unsigned& body_len);
	bool load_whole_tape();
	static tape_word_t checksum(const tape_word_t *block , unsigned block_len);
	bool decode_dir(const sif_file_t& file_0);
	void encode_dir(sif_file_t& file_0) const;
	void save_words(unsigned track , tape_pos_t& pos , const tape_word_t *block , unsigned block_len);
	void save_sif_file(unsigned& track , tape_pos_t& pos , file_no_t file_no , const sif_file_t& in);
	static bool filename_char_check(uint8_t c);
	static bool filename_check(const uint8_t *filename);
};

/********************************************************************************
 * Image state
 ********************************************************************************/
typedef struct {
	imgtool::stream *stream;
	tape_image_85 *img;
} tape_state_t;

/********************************************************************************
 * Directory enumeration
 ********************************************************************************/
typedef struct {
	unsigned dir_idx;
} dir_state_t;

/********************************************************************************
 * Internal functions
 ********************************************************************************/
tape_image_85::tape_image_85(void)
	: dirty(false)
{
	image.set_image_format(hti_format_t::HTI_DELTA_MOD_16_BITS);
}

void tape_image_85::format_img(void)
{
	// Create an empty directory
	dir.clear();
	content.clear();

	// Allocate space
	finalize_allocation();
}

imgtoolerr_t tape_image_85::load_from_file(imgtool::stream *stream)
{
	auto io = imgtool::stream_read(*stream, 0);
	if (!io || !image.load_tape(*io)) {
		return IMGTOOLERR_READERROR;
	}
	io.reset();

	// Prevent track boundary crossing when reading directory
	file_track_1 = 0;

	// Get directory (file #0)
	sif_file_t file_0;
	if (!load_sif_file(0 , file_0)) {
		return IMGTOOLERR_CORRUPTDIR;
	}

	if (!decode_dir(file_0)) {
		return IMGTOOLERR_CORRUPTDIR;
	}

	dirty = false;

	return IMGTOOLERR_SUCCESS;
}

bool tape_image_85::load_sif_file(file_no_t file_no , sif_file_t& out)
{
	unsigned track;
	unsigned gaps_to_go;

	// What track is the file on?
	if (!file_track_1 || file_no < file_track_1 || (file_no == file_track_1 && record_track_1)) {
		track = 0;
		gaps_to_go = file_no + 1;
	} else {
		track = 1;
		gaps_to_go = file_no - file_track_1;
		if (!record_track_1) {
			gaps_to_go++;
		}
	}
	tape_pos_t pos = 0;

	hti_format_t::track_iterator_t it;
	while (gaps_to_go--) {
		// Search for IFG
		if (!image.next_data(track , pos , true , false , it)) {
			return false;
		}
		pos = it->first;
		if (!image.next_gap(track , pos , true , MIN_IFG_SIZE)) {
			return false;
		}
	}

	// Get back to the start of record before the gap
	image.next_data(track , pos , false , false , it);
	pos = it->first;
	bool direction = false;

	uint16_t expected_rec_no = 0;

	out.clear();

	while (true) {
		image.next_gap(track , pos , direction , MIN_IRG_SIZE);
		// Read record header
		if (!image.next_data(track , pos , true , false , it)) {
			break;
		}
		unsigned bit_idx = 15;
		if (!image.sync_with_record(track , it , bit_idx)) {
			// Couldn't align
			return false;
		}

		// 0    File word
		// 1    Record word
		// 2    Length word
		// 3    Checksum
		tape_word_t hdr[ 4 ];
		for (unsigned i = 0; i < 4; i++) {
			auto res = image.next_word(track , it , bit_idx , hdr[ i ]);
			if (res != hti_format_t::ADV_CONT_DATA) {
				return false;
			}
		}
		if (checksum(&hdr[ 0 ] , 3) != hdr[ 3 ]) {
			return false;
		}

		file_no_t hdr_file_no;
		uint16_t hdr_rec_no;
		bool hdr_has_body;
		unsigned hdr_body_len;

		if (!dec_rec_header(&hdr[ 0 ] , hdr_file_no , hdr_rec_no , hdr_has_body , hdr_body_len)) {
			return false;
		}

		if (hdr_file_no != file_no) {
			break;
		}
		if (!hdr_has_body || !hdr_body_len) {
			return true;
		}
		if (hdr_rec_no != expected_rec_no) {
			return false;
		}

		tape_word_t body[ MAX_RECORD_SIZE / 2 + 1 ];
		unsigned word_no = (hdr_body_len + 1) / 2 + 1;
		for (unsigned i = 0; i < word_no; i++) {
			auto res = image.next_word(track , it , bit_idx , body[ i ]);
			if (res != hti_format_t::ADV_CONT_DATA) {
				return false;
			}
		}
		if (checksum(&body[ 0 ] , word_no - 1) != body[ word_no - 1 ]) {
			return false;
		}
		for (unsigned i = 0; i < hdr_body_len; i++) {
			tape_word_t tmp = body[ i / 2 ];
			out.push_back((uint8_t)(tmp >> 8));
			i++;
			if (i < hdr_body_len) {
				out.push_back((uint8_t)(tmp & 0xff));
			}
		}

		// Move to next record (possibly crossing into track 1)
		expected_rec_no++;
		if (file_no == file_track_1 && expected_rec_no == record_track_1) {
			track = 1;
			pos = 0;
		} else {
			pos = it->first;
		}
		direction = true;

		if (hdr_body_len < MAX_RECORD_SIZE) {
			break;
		}
	}

	return expected_rec_no != 0;
}

bool tape_image_85::load_whole_tape()
{
	content.clear();

	for (const auto& i : dir) {
		sif_file_ptr_t file;
		if (!(i.filetype & FT_NULL_FILE_MASK)) {
			file = std::make_unique<sif_file_t>();
			if (!load_sif_file(i.file_no, *file)) {
				return false;
			}
		}
		content.push_back(std::move(file));
	}

	return true;
}

bool tape_image_85::dec_rec_header(const tape_word_t *hdr , file_no_t& file_no , uint16_t& rec_no , bool& has_body , unsigned& body_len)
{
	if ((hdr[ 0 ] & 0x6000) != 0x2000 ||
		(hdr[ 0 ] & 0x07ff) > MAX_FILE_NO ||
		(hdr[ 1 ] & 0xf000) != 0x1000) {
		return false;
	}
	file_no = (file_no_t)(hdr[ 0 ] & 0xff);
	rec_no = (uint16_t)(hdr[ 1 ] & 0xfff);
	has_body = (hdr[ 0 ] & 0x1800) != 0;

	bool has_file_id = (hdr[ 0 ] & 0x8000) != 0;
	if (has_file_id != (rec_no == 0)) {
		return false;
	}

	if (has_body) {
		if ((hdr[ 2 ] & 0xff00) != 0xff00) {
			return false;
		}
		body_len = hdr[ 2 ] & 0xff;
		if (body_len) {
			body_len++;
		}
	} else {
		body_len = 0;
	}

	return true;
}

tape_word_t tape_image_85::checksum(const tape_word_t *block , unsigned block_len)
{
	tape_word_t csum = 0;
	for (unsigned i = 0; i < block_len; i++) {
		csum += *block++;
	}
	return csum & 0xffff;
}

imgtoolerr_t tape_image_85::save_to_file(imgtool::stream *stream)
{
	sif_file_t file_0;

	encode_dir(file_0);

	unsigned track = 0;
	tape_pos_t pos = TRACK_START;

	image.clear_tape();

	save_sif_file(track , pos , 0 , file_0);

	for (auto i = dir.cbegin(); i != dir.cend(); i++) {
		file_no_t file_no = i - dir.cbegin();
		save_sif_file(track , pos , file_no + 1 , *content[ file_no ]);
	}

	// Empty file at the end
	file_0.clear();
	save_sif_file(track , pos , dir.size() + 1 , file_0);

	image.save_tape(*imgtool::stream_read_write(*stream, 0));

	return IMGTOOLERR_SUCCESS;
}

bool tape_image_85::get_dir_entry(unsigned idx , const dir_entry_85*& entry) const
{
	if (idx >= dir.size()) {
		return false;
	} else {
		entry = &dir[ idx ];
		return true;
	}
}

bool tape_image_85::find_file(const char *filename , bool ignore_ext , unsigned& idx) const
{
	if (strcmp(filename , NULL_FILENAME) == 0) {
		return false;
	}

	char fname[ CHARS_PER_FNAME_EXT + 1 ];
	char ext[ CHARS_PER_EXT + 1 ];

	split_filename_and_ext(filename, fname, ext);

	bool has_ext = !ignore_ext && *ext != '\0';

	if (has_ext) {
		strcat(fname , ".");
		strcat(fname , ext);
	}

	for (auto i = dir.cbegin(); i < dir.cend(); i++) {
		if (i->filetype & FT_NULL_FILE_MASK) {
			continue;
		}
		char full_fname[ CHARS_PER_FNAME_EXT + 1 ];

		get_filename_and_ext(*i, has_ext, full_fname);

		if (strcmp(fname , full_fname) == 0) {
			idx = i - dir.cbegin();
			return true;
		}
	}

	return false;
}

bool tape_image_85::alloc_new_file(dir_entry_85*& entry , sif_file_ptr_t&& file_data)
{
	if (file_data->size() > MAX_N_RECORDS * MAX_RECORD_SIZE) {
		// File bigger than tape capacity
		return false;
	}

	if (!load_whole_tape()) {
		return false;
	}

	dir_entry_85 new_entry;
	memset(&new_entry , 0 , sizeof(new_entry));

	unsigned idx = MAX_FILE_NO;
	for (auto i = dir.cbegin(); i != dir.cend(); i++) {
		if (i->filetype & FT_NULL_FILE_MASK) {
			idx = i - dir.cbegin();
			break;
		}
	}
	if (idx >= MAX_FILE_NO) {
		idx = dir.size();
		if (idx >= MAX_FILE_NO) {
			return false;
		}
		dir.push_back(new_entry);
		content.push_back(std::make_unique<sif_file_t>());
	} else {
		dir[ idx ] = new_entry;
	}
	entry = &dir[ idx ];

	content[ idx ] = std::move(file_data);

	return true;
}

bool tape_image_85::delete_dir_entry(unsigned idx)
{
	if (idx < dir.size()) {
		dir[ idx ].filetype = FT_NULL_FILE_MASK;
		return load_whole_tape();
	} else {
		return false;
	}
}

bool tape_image_85::finalize_allocation()
{
	tape_pos_t hole_pos = hti_format_t::next_hole(TRACK_START , true);

	for (unsigned i = 0; i < dir.size(); i++) {
		if (dir[ i ].filetype & FT_NULL_FILE_MASK) {
			dir.erase(dir.begin() + i);
			content.erase(content.begin() + i);
			i--;
		}
	}

	unsigned track = 0;
	// Position where file #1 starts (that is, after directory)
	tape_pos_t pos = TRACK_START + FMT_REC_SIZE * DIR_RECORDS + FMT_IRG_SIZE * (DIR_RECORDS - 1) + FMT_IFG_SIZE;

	file_track_1 = 0;
	record_track_1 = 0;

	for (auto i = dir.begin(); i != dir.end(); i++) {
		file_no_t file_no = i - dir.begin();
		unsigned recs = (content[ file_no ]->size() + MAX_RECORD_SIZE - 1) /  MAX_RECORD_SIZE;
		if (!recs) {
			// Always at least 1 record
			recs = 1;
		}
		i->file_no = file_no + 1;
		i->n_recs = recs;
		// Size of file on tape: "recs" records, 1 IFG between record 0 and 1, IRGs between all
		// other records
		tape_pos_t file_size = FMT_IFG_SIZE + recs * FMT_REC_SIZE + (recs - 1) * FMT_IRG_SIZE;
		tape_pos_t rec1_start = pos + FMT_REC_SIZE + FMT_IFG_SIZE;
		tape_pos_t next_track_pos = 0;
		if (pos <= hole_pos && hole_pos < rec1_start) {
			// Hole on record #0
			file_track_1 = file_no + 1;
			record_track_1 = 1;
			next_track_pos = rec1_start;
		} else if (rec1_start <= hole_pos && hole_pos < (pos + file_size)) {
			// Hole in this file, records from 1 on
			file_track_1 = file_no + 1;
			record_track_1 = (hole_pos - rec1_start) / (FMT_REC_SIZE + FMT_IRG_SIZE) + 2;
			next_track_pos = rec1_start + (record_track_1 - 1) * (FMT_REC_SIZE + FMT_IRG_SIZE);
		}
		if (next_track_pos) {
			// Move to next track
			if (++track >= 2) {
				// Out of space
				return false;
			}
			pos = pos + file_size - next_track_pos + TRACK_START;
		} else {
			pos += file_size;
		}
	}
	dirty = true;
	return true;
}

typedef struct {
	uint8_t filetype_mask;
	const char *ext;
} file_attr_t;

static const file_attr_t filetype_table[] = {
	{ FT_PROG_MASK , "PROG" },
	{ FT_DATA_MASK , "DATA" },
	{ FT_BPGM_MASK , "BPGM" }
};

void tape_image_85::get_filename_and_ext(const dir_entry_85& ent , bool inc_ext , char *out)
{
	if (ent.filetype & FT_NULL_FILE_MASK) {
		// Empty directory slot
		strcpy(out , NULL_FILENAME);
	} else {
		const uint8_t *s = &ent.filename[ 0 ];
		while (*s != '\0' && *s != ' ' && (s - &ent.filename[ 0 ]) < CHARS_PER_FNAME) {
			*out++ = *s++;
		}
		*out = '\0';

		// Decode filetype
		if (inc_ext) {
			const char *ext = nullptr;
			for (const auto& i : filetype_table) {
				if (ent.filetype & i.filetype_mask) {
					ext = i.ext;
					break;
				}
			}
			if (ext != nullptr) {
				strcat(out , ".");
				strcat(out , ext);
			}
		}
	}
}

void tape_image_85::split_filename_and_ext(const char *filename , char *fname , char *ext)
{
	char *fname_fence = fname + CHARS_PER_FNAME;

	while (fname < fname_fence && *filename != '\0' && *filename != '.') {
		*fname++ = *filename++;
	}

	*fname = '\0';

	while (*filename != '\0' && *filename != '.') {
		filename++;
	}

	if (*filename == '\0') {
		*ext = '\0';
	} else {
		filename++;
		strncpy(ext , filename , CHARS_PER_EXT);
		ext[ CHARS_PER_EXT ] = '\0';
	}
}

bool tape_image_85::decode_dir(const sif_file_t& file_0)
{
	if (file_0.size() != DIR_RECORDS * MAX_RECORD_SIZE) {
		return false;
	}
	dir.clear();

	// Check DIRSEG (directory segment), i.e. record #0 or #1 of directory
	// Check FL1TK1 (file & record no. where track 1 begins)
	if (file_0[ 0xfc ] != 0 ||
		file_0[ 0x1fc ] != 1 ||
		file_0[ 0xfd ] != file_0[ 0x1fd ] ||
		file_0[ 0xfe ] != file_0[ 0x1fe ] ||
		file_0[ 0xff ] != file_0[ 0x1ff ] ||
		file_0[ 0xfd ] > MAX_FILE_NO) {
		return false;
	}

	// Get FL1TK1
	file_track_1 = file_0[ 0xfd ];
	record_track_1 = pick_integer_le(file_0.data() , 0xfe , 2);

	file_no_t file_no = 1;

	// Iterate over all entries of directory
	for (unsigned i = 0; i < 0x1fc; i += 12) {
		if (i == 0xfc) {
			// Skip over 1st copy of DIRSEG/FL1TK1
			i = 0x100;
		}
		const uint8_t *p = file_0.data() + i;
		dir_entry_85 new_entry;

		// File type
		new_entry.filetype = p[ 7 ];
		if (new_entry.filetype & FT_NEXT_AV_MASK) {
			// Directory ends
			break;
		}

		// Filename
		memcpy(&new_entry.filename[ 0 ] , p , CHARS_PER_FNAME);
		if (!filename_check(&new_entry.filename[ 0 ])) {
			return false;
		}

		// File #
		// It is also stored at p[ 6 ] but HP85 firmware ignores it
		new_entry.file_no = file_no++;

		// Physical records
		new_entry.n_recs = pick_integer_le(p , 8 , 2);

		// Bytes per logical record
		new_entry.record_len = pick_integer_le(p , 10 , 2);
		if (new_entry.record_len < 4 || new_entry.record_len >= 32768) {
			return false;
		}

		dir.push_back(new_entry);
	}

	return true;
}

void tape_image_85::encode_dir(sif_file_t& file_0) const
{
	file_0.clear();
	file_0.resize(DIR_RECORDS * MAX_RECORD_SIZE , 0);

	// Set DIRSEG
	file_0[ 0xfc ] = 0;
	file_0[ 0x1fc ] = 1;
	// Set FL1TK1
	file_0[ 0xfd ] = file_0[ 0x1fd ] = file_track_1;
	place_integer_le(file_0.data() , 0xfe , 2 , record_track_1);
	place_integer_le(file_0.data() , 0x1fe , 2 , record_track_1);

	unsigned i = 0;
	file_no_t file_no = 1;
	for (auto entry = dir.cbegin(); entry != dir.cend(); entry++, i += 12, file_no++) {
		if (i == 0xfc) {
			// Skip over 1st copy of DIRSEG/FL1TK1
			i = 0x100;
		}
		uint8_t *p_entry = file_0.data() + i;
		memcpy(&p_entry[ 0 ] , &entry->filename[ 0 ] , CHARS_PER_FNAME);
		p_entry[ 6 ] = file_no;
		p_entry[ 7 ] = entry->filetype;
		place_integer_le(p_entry , 8 , 2 , entry->n_recs);
		place_integer_le(p_entry , 10 , 2 , entry->record_len);
	}

	if (file_no <= MAX_FILE_NO) {
		if (i == 0xfc) {
			// Skip over 1st copy of DIRSEG/FL1TK1
			i = 0x100;
		}
		file_0[ i + 7 ] = FT_NEXT_AV_MASK;
	}

	// Two identical copies of directory
	memcpy(file_0.data() + (DIR_RECORDS / 2) * MAX_RECORD_SIZE , file_0.data() , (DIR_RECORDS / 2) * MAX_RECORD_SIZE);
}

void tape_image_85::save_words(unsigned track , tape_pos_t& pos , const tape_word_t *block , unsigned block_len)
{
	tape_pos_t length;
	for (unsigned i = 0; i < block_len; i++) {
		image.write_word(track , pos , *block++ , length);
		pos += length;
	}
}

void tape_image_85::save_sif_file(unsigned& track , tape_pos_t& pos , file_no_t file_no , const sif_file_t& in)
{
	unsigned rec_no = 0;
	unsigned bytes_to_go = in.size();
	sif_file_t::const_iterator in_it = in.cbegin();

	do {
		if (file_track_1 != 0 && track == 0 &&
			((file_no == file_track_1 && rec_no >= record_track_1) || file_no > file_track_1)) {
			// Switch to track 1
			track = 1;
			pos = TRACK_START;
		}
		tape_pos_t start_pos = pos;

		unsigned rec_size = std::min(bytes_to_go , MAX_RECORD_SIZE);

		tape_word_t hdr[ 5 ];
		hdr[ 0 ] = SYNC_WORD;
		hdr[ 1 ] = (tape_word_t)file_no | 0x2000;
		if (rec_no == 0) {
			hdr[ 1 ] |= 0x8000;
		}
		if (rec_size) {
			hdr[ 1 ] |= 0x1800;
		}
		hdr[ 2 ] = 0x1000 | (tape_word_t)rec_no;
		hdr[ 3 ] = 0xff00;
		if (rec_size) {
			hdr[ 3 ] |= (rec_size - 1);
		}
		hdr[ 4 ] = checksum(&hdr[ 1 ], 3);

		save_words(track, pos, &hdr[ 0 ], 5);

		if (rec_size) {
			tape_word_t body[ MAX_RECORD_SIZE / 2 + 1 ];
			unsigned words = 0;
			while (words < MAX_RECORD_SIZE / 2 && in_it != in.cend()) {
				tape_word_t w = (tape_word_t)*in_it++ << 8;
				if (in_it != in.cend()) {
					w |= *in_it++;
				}
				body[ words++ ] = w;
			}
			body[ words ] = checksum(&body[ 0 ], words);
			save_words(track, pos, &body[ 0 ], words + 1);
		}

		// Pad record up to FMT_REC_SIZE
		tape_word_t filler = 0xffff;
		while ((pos - start_pos) < FMT_REC_SIZE) {
			save_words(track, pos, &filler, 1);
		}

		if (rec_no == 0) {
			// IFG
			pos = start_pos + FMT_REC_SIZE + FMT_IFG_SIZE;
		} else {
			// IRG
			pos = start_pos + FMT_REC_SIZE + FMT_IRG_SIZE;
		}
		bytes_to_go -= rec_size;
		rec_no++;
	} while (bytes_to_go > 0);
}

bool tape_image_85::filename_char_check(uint8_t c)
{
	// Quotation marks are forbidden in file names
	return 0x20 < c && c < 0x7f && c != '"';
}

bool tape_image_85::filename_check(const uint8_t *filename)
{
	bool ended = false;

	for (unsigned i = 0; i < CHARS_PER_FNAME; i++) {
		uint8_t c = *filename++;

		if (ended) {
			if (c != ' ') {
				return false;
			}
		} else if (c == ' ') {
			ended = true;
		} else if (!filename_char_check(c)) {
			return false;
		}
	}

	return true;
}

namespace {
	tape_state_t& get_tape_state(imgtool::image &img)
	{
		tape_state_t *ts = (tape_state_t*)img.extra_bytes();

		return *ts;
	}

	tape_image_85& get_tape_image(tape_state_t& ts)
	{
		if (ts.img == nullptr) {
			ts.img = new tape_image_85;
		}

		return *(ts.img);
	}
}
/********************************************************************************
 * Imgtool functions
 ********************************************************************************/
namespace {
	imgtoolerr_t hp85_tape_open(imgtool::image &image, imgtool::stream::ptr &&stream)
	{
		tape_state_t& state = get_tape_state(image);

		state.stream = stream.release();

		tape_image_85& tape_image = get_tape_image(state);

		imgtoolerr_t err = tape_image.load_from_file(state.stream);
		if (err)
			return err;

		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
	{
		tape_state_t& state = get_tape_state(image);

		state.stream = stream.release();

		tape_image_85& tape_image = get_tape_image(state);

		tape_image.format_img();

		return IMGTOOLERR_SUCCESS;
	}

	void hp85_tape_close(imgtool::image &image)
	{
		tape_state_t& state = get_tape_state(image);
		tape_image_85& tape_image = get_tape_image(state);

		if (tape_image.is_dirty()) {
			(void)tape_image.save_to_file(state.stream);
		}

		delete state.stream;

		// Free tape_image
		delete &tape_image;
	}

	imgtoolerr_t hp85_tape_begin_enum (imgtool::directory &enumeration, const char *path)
	{
		dir_state_t *ds = (dir_state_t*)enumeration.extra_bytes();

		ds->dir_idx = 0;

		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_next_enum (imgtool::directory &enumeration, imgtool_dirent &ent)
	{
		tape_state_t& state = get_tape_state(enumeration.image());
		tape_image_85& tape_image = get_tape_image(state);
		dir_state_t *ds = (dir_state_t*)enumeration.extra_bytes();

		const dir_entry_85 *entry = nullptr;

		if (!tape_image.get_dir_entry(ds->dir_idx, entry)) {
			ent.eof = 1;
		} else {
			ds->dir_idx++;

			unsigned filesize = entry->n_recs * MAX_RECORD_SIZE;

			if (entry->filetype & FT_NULL_FILE_MASK) {
				ent.filesize = 0;
			} else {
				ent.filesize = filesize;
			}

			tape_image_85::get_filename_and_ext(*entry, true, ent.filename);
			snprintf(ent.attr , sizeof(ent.attr) , "%u %u %u %c%c" , entry->record_len , filesize / entry->record_len , entry->file_no , (entry->filetype & FT_WP_MASK) ? 'W' : ' ' , (entry->filetype & FT_HIDDEN_MASK) ? 'H' : ' ');
		}


		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_free_space(imgtool::partition &partition, uint64_t *size)
	{
		tape_state_t& state = get_tape_state(partition.image());
		tape_image_85& tape_image = get_tape_image(state);
		unsigned used_recs = 0;

		const dir_entry_85 *entry = nullptr;

		for (unsigned i = 0; i < MAX_FILE_NO; i++) {
			if (!tape_image.get_dir_entry(i, entry)) {
				break;
			}
			// Ignore erased files
			if (entry->filetype & FT_NULL_FILE_MASK) {
				continue;
			}
			used_recs += entry->n_recs;
		}

		if (used_recs >= MAX_N_RECORDS) {
			*size = 0;
		} else {
			*size = (MAX_N_RECORDS - used_recs) * MAX_RECORD_SIZE;
		}

		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_read_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
	{
		tape_state_t& state = get_tape_state(partition.image());
		tape_image_85& tape_image = get_tape_image(state);

		unsigned idx;

		if (!tape_image.find_file(filename , false , idx)) {
			return IMGTOOLERR_FILENOTFOUND;
		}

		const dir_entry_85 *ent = nullptr;

		tape_image.get_dir_entry(idx, ent);

		tape_image_85::sif_file_t file_data;

		if (!tape_image.load_sif_file(ent->file_no , file_data)) {
			return IMGTOOLERR_READERROR;
		}

		destf.write(file_data.data() , file_data.size());

		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_write_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
	{
		tape_state_t& state = get_tape_state(partition.image());
		tape_image_85& tape_image = get_tape_image(state);

		unsigned idx;

		if (tape_image.find_file(filename , true , idx)) {
			// When overwriting a file, delete its old version first
			tape_image.delete_dir_entry(idx);
		}

		unsigned file_size = sourcef.size();

		if (!file_size) {
			return IMGTOOLERR_SUCCESS;
		}
		if (file_size > (MAX_N_RECORDS * MAX_RECORD_SIZE)) {
			return IMGTOOLERR_NOSPACE;
		}

		auto p_in_file = std::make_unique<tape_image_85::sif_file_t>();

		p_in_file->resize(file_size);

		if (sourcef.read(p_in_file->data() , file_size) != file_size) {
			return IMGTOOLERR_READERROR;
		}

		dir_entry_85 *ent = nullptr;

		if (!tape_image.alloc_new_file(ent , std::move(p_in_file))) {
			return IMGTOOLERR_NOSPACE;
		}

		char fname[ CHARS_PER_FNAME + 1 ];
		char ext[ CHARS_PER_EXT + 1 ];

		tape_image_85::split_filename_and_ext(filename, fname, ext);

		char *dest = (char *)&ent->filename[ 0 ];
		char *dest0 = dest;
		char *src = &fname[ 0 ];

		while ((dest - dest0) < CHARS_PER_FNAME && *src != '\0') {
			*dest++ = *src++;
		}
		while ((dest - dest0) < CHARS_PER_FNAME) {
			*dest++ = ' ';
		}

		int bpr = opts->lookup_int('B');
		if (bpr < 4) {
			bpr = MAX_RECORD_SIZE;
		} else if (bpr > file_size) {
			util::stream_format(std::wcerr, L"BPR value too large, using %u\n", MAX_RECORD_SIZE);
			bpr = MAX_RECORD_SIZE;
		}
		ent->record_len = (uint16_t)bpr;

		if (opts->lookup_int('T') == 0) {
			// File type defaults to DATA if no extension is given or extension is invalid
			ent->filetype = FT_DATA_MASK;
			for (const auto& i : filetype_table) {
				if (strcmp(i.ext , ext) == 0) {
					ent->filetype = i.filetype_mask;
					break;
				}
			}
		} else {
			ent->filetype = filetype_table[ opts->lookup_int('T') - 1 ].filetype_mask;
		}

		if (!tape_image.finalize_allocation()) {
			return IMGTOOLERR_NOSPACE;
		}

		return IMGTOOLERR_SUCCESS;
	}

	imgtoolerr_t hp85_tape_delete_file(imgtool::partition &partition, const char *filename)
	{
		tape_state_t& state = get_tape_state(partition.image());
		tape_image_85& tape_image = get_tape_image(state);

		unsigned idx;

		if (!tape_image.find_file(filename , true , idx)) {
			return IMGTOOLERR_FILENOTFOUND;
		}

		if (!tape_image.delete_dir_entry(idx)) {
			return IMGTOOLERR_READERROR;
		}
		if (!tape_image.finalize_allocation()) {
			return IMGTOOLERR_NOSPACE;
		}

		return IMGTOOLERR_SUCCESS;
	}
}
#define HP85_WRITEFILE_OPTSPEC    "B[0]-32767;T[0]-3"

OPTION_GUIDE_START(hp85_write_optguide)
	OPTION_INT('B' , "bpr" , "Bytes per record")
	OPTION_ENUM_START('T' , "ftype" , "File type")
	OPTION_ENUM(0 , "auto" , "Automatic (\"DATA\" or by extension)")
	OPTION_ENUM(1 , "P"    , "PROG")
	OPTION_ENUM(2 , "D"    , "DATA")
	OPTION_ENUM(3 , "B"    , "BPGM")
	OPTION_ENUM_END
OPTION_GUIDE_END

void hp85_tape_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch (state) {
	case IMGTOOLINFO_STR_NAME:
		strcpy(info->s = imgtool_temp_str(), "hp85_tape");
		break;

	case IMGTOOLINFO_STR_DESCRIPTION:
		strcpy(info->s = imgtool_temp_str(), "HP85 tape image");
		break;

	case IMGTOOLINFO_STR_FILE:
		strcpy(info->s = imgtool_temp_str(), __FILE__);
		break;

	case IMGTOOLINFO_STR_FILE_EXTENSIONS:
		strcpy(info->s = imgtool_temp_str(), "hti");
		break;

	case IMGTOOLINFO_INT_IMAGE_EXTRA_BYTES:
		info->i = sizeof(tape_state_t);
		break;

	case IMGTOOLINFO_INT_DIRECTORY_EXTRA_BYTES:
		info->i = sizeof(dir_state_t);
		break;

	case IMGTOOLINFO_PTR_OPEN:
		info->open = hp85_tape_open;
		break;

	case IMGTOOLINFO_PTR_CREATE:
		info->create = hp85_tape_create;
		break;

	case IMGTOOLINFO_PTR_CLOSE:
		info->close = hp85_tape_close;
		break;

	case IMGTOOLINFO_PTR_BEGIN_ENUM:
		info->begin_enum = hp85_tape_begin_enum;
		break;

	case IMGTOOLINFO_PTR_NEXT_ENUM:
		info->next_enum = hp85_tape_next_enum;
		break;

	case IMGTOOLINFO_PTR_FREE_SPACE:
		info->free_space = hp85_tape_free_space;
		break;

	case IMGTOOLINFO_PTR_READ_FILE:
		info->read_file = hp85_tape_read_file;
		break;

	case IMGTOOLINFO_PTR_WRITE_FILE:
		info->write_file = hp85_tape_write_file;
		break;

	case IMGTOOLINFO_PTR_DELETE_FILE:
		info->delete_file = hp85_tape_delete_file;
		break;

	case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:
		info->writefile_optguide = &hp85_write_optguide;
		break;

	case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:
		strcpy(info->s = imgtool_temp_str(), HP85_WRITEFILE_OPTSPEC);
		break;
	}
}
