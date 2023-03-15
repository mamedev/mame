// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    hp9845_tape.cpp

    HP-9845 tape format

    This imgtool module manipulates HTI files. These are image files
    of the DC-100 tape cartridges that are simulated for the HP9845B
    driver.
    HP9845 filesystem for tapes has the following features:
    * File names are 1 to 6 characters long.
    * Case is significant in file names.
    * There is no file "extension", file type is encoded separately
      in file metadata.
    * There are 8 file types. File type is encoded in 5 bits.
      Only 8 out of the 32 possible values are valid.
    * This module handles the file type as a fake file extension.
      For example, a file named "TEST" having DATA type is get/put/shown
      as "TEST.DATA".
    * File type is deduced from host file extension when putting files
      into image. File type can be overridden by the "ftype" option.
      This table summarizes the file types.

      ftype     Fake        Type of file    BASIC commands
      switch    extension                   for this file type
      ========================================================
      U         BKUP        "Database backup"
                                            No idea
      D         DATA        Generic record-based data file
                                            SAVE/GET/PRINT#/READ#
      P         PROG        Program file (tokenized BASIC & other data)
                                            STORE/LOAD
      K         KEYS        KEY file (definition of soft keys)
                                            STORE KEY/LOAD KEY
      T         BDAT        Binary data file
                                            ?
      A         ALL         Full dump of system state
                                            STORE ALL/LOAD ALL
      B         BPRG        Binary program file
                                            STORE BIN/LOAD BIN
      O         OPRM        Option ROM specific file
                                            ?

    * Files are always stored in units of 256-byte physical records.
    * An important metadata of files is WPR: Words Per Record. This
      is a numeric value that sets the length of each logical record of
      the file (in units of 16-bit words). It defaults to 128 (i.e.
      logical and physical records are the same thing). It can be
      set by the "wpr" option when putting files into the image.
    * There is no fragmentation map in the filesystem: each file
      always occupy a contiguous set of physical records. This fact
      could prevent the putting of a file into an image when there
      is no single block of free records big enough to hold the file
      even though the total amount of free space would be sufficient.

    Notes on commands
    =================

    **** dir command ****
    The format of the "attr" part of file listing is as follows:
    %c      '*' if file has the protection bit set, else ' '
    %02x    Hexadecimal value of file type (00-1f)
    %c      '?' if file type is not valid, else ' '
    %4u     Number of logical records
    %4u     WPR * 2 (i.e. bytes per logical record)
    %3u     First physical record of file

    **** get command ****
    A file can be extracted from an image with or without an explicit
    extension. If an extension is given, it must match the one corresponding
    to file type.
    The "9845data" filter can be used on DATA files (see below).

    **** getall command ****
    Files are extracted with their "fake" extension.

    **** put command ****
    File type can be specified explicitly through the "ftype" option.
    If this option is "auto" (the default), type is deduced from file
    extension, if present. When extension is not given or it doesn't
    match any known type, file type is set to "DATA".
    WPR can be set through the "wpr" option. If it's 0 (the default),
    WPR is set to 128.
    The "9845data" filter can be used on DATA files (see below).

    **** del command ****
    File extension is ignored, if present.

    "9845data" filter
    =================

    This filter can be applied to DATA files whose content is made
    of strings only. BASIC programs that are saved with "SAVE" command
    have this format.
    This filter translates a DATA file into a standard ASCII text file
    and viceversa.
    Keep in mind that this translation is NOT lossless because all
    non-ASCII & non printable characters are substituted with spaces.
    This kind of characters must be removed because they may confuse
    the line-by-line reading of file when translating in the opposite
    direction.
    The 9845 system has the capability to insert formatting characters
    directly in the text strings to be displayed on screen. These
    characters set things like inverse video or underline.
    Turning a DATA file into a text file through this filter removes
    these special characters.

*********************************************************************/
#include "imgtool.h"
#include "filter.h"

#include "formats/hti_tape.h"
#include "formats/imageutl.h"

#include "corefile.h"
#include "ioprocs.h"
#include "opresolv.h"

#include <bitset>
#include <cstdio>


// Constants
#define SECTOR_LEN          256 // Bytes in a sector
#define WORDS_PER_SECTOR    (SECTOR_LEN / 2)    // 16-bit words in a sector payload
#define SECTORS_PER_TRACK   426 // Sectors in a track
#define TRACKS_NO           2   // Number of tracks
#define TOT_SECTORS         (SECTORS_PER_TRACK * TRACKS_NO) // Total number of sectors
#define DIR_WORD_0          0x0500  // First word of directories
#define DIR_WORD_1          0xffff  // Second word of directories
#define DIR_LAST_WORD       0xffff  // Last word of directories
#define FIRST_DIR_SECTOR    1   // First directory sector
#define SECTORS_PER_DIR     2   // Sectors per copy of directory
#define MAX_DIR_ENTRIES     42  // And the answer is.... the maximum number of entries in the directory!
#define DIR_COPIES          2   // Count of directory copies
#define CHARS_PER_FNAME     6   // Maximum characters in a filename
#define CHARS_PER_EXT       4   // Characters in file extension. Extension is encoded as file type, it's not actually stored in directory as characters.
#define CHARS_PER_FNAME_EXT (CHARS_PER_FNAME + 1 + CHARS_PER_EXT)   // Characters in filename + extension
#define PAD_WORD            0xffff  // Word value for padding
#define FIRST_FILE_SECTOR   (FIRST_DIR_SECTOR + SECTORS_PER_DIR * DIR_COPIES)   // First file sector
#define START_POS           ((tape_pos_t)(72.25 * hti_format_t::ONE_INCH_POS))    // Start position on each track
#define DZ_WORDS            350 // Words in deadzone
#define IRG_SIZE            hti_format_t::ONE_INCH_POS    // Size of inter-record-gap: 1"
#define IFG_SIZE            ((tape_pos_t)(2.5 * hti_format_t::ONE_INCH_POS))  // Size of inter-file-gap: 2.5"
#define HDR_W0_ZERO_MASK    0x4000  // Mask of zero bits in word 0 of header
#define RES_FREE_FIELD      0x2000  // Mask of "reserved free field" bit
#define FILE_ID_BIT         0x8000  // Mask of "file identifier" bit
#define SECTOR_IN_USE       0x1800  // Mask of "empty record indicator" (== !sector in use indicator)
#define SIF_FILE_NO         1   // SIF file #
#define SIF_FILE_NO_MASK    0x07ff  // Mask of SIF file #
#define SIF_FREE_FIELD      0   // SIF free field
#define SIF_FREE_FIELD_MASK 0xf000  // Mask of SIF free field
#define BYTES_AVAILABLE     0xff00  // "bytes available" field = 256
#define BYTES_AVAILABLE_MASK    0xff00  // Mask of "bytes available" field
#define BYTES_USED          0x00ff  // "bytes used" field = 256
#define BYTES_USED_MASK     0x00ff  // Mask of "bytes used" field
#define FORMAT_SECT_SIZE    ((tape_pos_t)(2.67 * hti_format_t::ONE_INCH_POS)) // Size of sectors including padding: 2.67"
#define PREAMBLE_WORD       0x0001  // Value of preamble word
#define WORDS_PER_HEADER_N_SECTOR   (WORDS_PER_SECTOR + 5)
#define MIN_IRG_SIZE        ((tape_pos_t)(16 * 1024))   // Minimum size of IRG gaps: 0.017"

// File types
#define BKUP_FILETYPE       0
#define BKUP_ATTR_STR       "BKUP"
#define DATA_FILETYPE       1
#define DATA_ATTR_STR       "DATA"
#define PROG_FILETYPE       2
#define PROG_ATTR_STR       "PROG"
#define KEYS_FILETYPE       3
#define KEYS_ATTR_STR       "KEYS"
#define BDAT_FILETYPE       4
#define BDAT_ATTR_STR       "BDAT"
#define ALL_FILETYPE        5
#define ALL_ATTR_STR        "ALL"
#define BPRG_FILETYPE       6
#define BPRG_ATTR_STR       "BPRG"
#define OPRM_FILETYPE       7
#define OPRM_ATTR_STR       "OPRM"

// Record type identifiers
#define REC_TYPE_EOR        0x1e    // End-of-record
#define REC_TYPE_FULLSTR    0x3c    // A whole (un-split) string
#define REC_TYPE_EOF        0x3e    // End-of-file
#define REC_TYPE_1STSTR     0x1c    // First part of a string
#define REC_TYPE_MIDSTR     0x0c    // Middle part(s) of a string
#define REC_TYPE_ENDSTR     0x2c    // Last part of a string

// End-of-lines
#define EOLN (CRLF == 1 ? "\r" : (CRLF == 2 ? "\n" : (CRLF == 3 ? "\r\n" : NULL)))

// Words stored on tape
using tape_word_t = hti_format_t::tape_word_t;

// Tape position, 1 unit = 1 inch / (968 * 1024)
using tape_pos_t = hti_format_t::tape_pos_t;

/********************************************************************************
 * Directory entries
 ********************************************************************************/
typedef struct {
	uint8_t filename[ CHARS_PER_FNAME ];  // Filename (left justified, 0 padded on the right)
	bool protection;    // File protection
	uint8_t filetype;     // File type (00-1f)
	uint16_t filepos;     // File position (# of 1st sector)
	uint16_t n_recs;      // Number of records
	uint16_t wpr;         // Word-per-record
	unsigned n_sects;   // Count of sectors
} dir_entry_t;

/********************************************************************************
 * Tape image
 ********************************************************************************/
class tape_image_t {
public:
	tape_image_t(void);

	bool is_dirty(void) const { return dirty; }

	void format_img(void);

	imgtoolerr_t load_from_file(imgtool::stream *stream);
	imgtoolerr_t save_to_file(imgtool::stream *stream);

	unsigned free_sectors(void) const;

	void set_sector(unsigned s_no , const tape_word_t *s_data);
	void unset_sector(unsigned s_no);
	bool get_sector(unsigned s_no , tape_word_t *s_data);

	bool get_dir_entry(unsigned idx , const dir_entry_t*& entry) const;
	bool find_file(const char *filename , bool ignore_ext , unsigned& idx) const;

	void delete_dir_entry(unsigned idx);

	bool find_free_block(unsigned blocks , unsigned& first_s) const;

	bool alloc_new_file(unsigned blocks , dir_entry_t*& entry);

	static void tape_word_to_bytes(tape_word_t w , uint8_t& bh , uint8_t& bl);
	static void bytes_to_tape_word(uint8_t bh , uint8_t bl , tape_word_t& w);

	static void get_filename_and_ext(const dir_entry_t& ent , bool inc_ext , char *out , bool& qmark);
	static void split_filename_and_ext(const char *filename , char *fname , char *ext);

private:
	bool dirty;
	// Tape image
	tape_word_t img[ TOT_SECTORS ][ WORDS_PER_SECTOR ];
	// Map of sectors in use
	std::bitset<TOT_SECTORS> alloc_map;
	// Directory
	std::vector<dir_entry_t> dir;

	static void wipe_sector(tape_word_t *s);
	void dump_dir_sect(const tape_word_t *dir_sect , unsigned dir_sect_idx);
	void fill_and_dump_dir_sect(tape_word_t *dir_sect , unsigned& idx , unsigned& dir_sect_idx , tape_word_t w) ;
	void encode_dir(void);
	bool read_sector_words(unsigned& sect_no , unsigned& sect_idx , size_t word_no , tape_word_t *out) const;
	static bool filename_char_check(uint8_t c);
	static bool filename_check(const uint8_t *filename);
	bool decode_dir(void);
	void save_words(hti_format_t& img , unsigned track , tape_pos_t& pos , const tape_word_t *block , unsigned block_len);
	static tape_word_t checksum(const tape_word_t *block , unsigned block_len);
};

/********************************************************************************
 * Image state
 ********************************************************************************/
typedef struct {
	imgtool::stream *stream;
	tape_image_t *img;
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
tape_image_t::tape_image_t(void)
	: dirty(false)
{
}

void tape_image_t::format_img(void)
{
	// Deallocate all sectors
	alloc_map.reset();

	// Create an empty directory
	dir.clear();

	dirty = true;
}

imgtoolerr_t tape_image_t::load_from_file(imgtool::stream *stream)
{
	hti_format_t inp_image;
	inp_image.set_image_format(hti_format_t::HTI_DELTA_MOD_16_BITS);

	auto io = imgtool::stream_read(*stream, 0);
	if (!io || !inp_image.load_tape(*io)) {
		return IMGTOOLERR_READERROR;
	}
	io.reset();

	unsigned exp_sector = 0;
	unsigned last_sector_on_track = SECTORS_PER_TRACK;
	for (unsigned track = 0; track < TRACKS_NO; track++ , last_sector_on_track += SECTORS_PER_TRACK) {
		tape_pos_t pos = 0;
		// Loader state:
		// 0    Wait for DZ
		// 1    Wait for sector data
		// 2    Wait for gap
		unsigned state = 0;

		while (exp_sector != last_sector_on_track) {
			switch (state) {
			case 0:
			case 1:
				{
					hti_format_t::track_iterator_t it;

					if (!inp_image.next_data(track , pos , true , false , it)) {
						// No more data on tape
						return IMGTOOLERR_CORRUPTIMAGE;
					}
					if (state == 1) {
						// Extract record data

						// The top 8 bits are ignored by TACO when aligning with preamble
						unsigned bit_idx = 7;
						if (!inp_image.sync_with_record(track , it , bit_idx)) {
							// Couldn't align
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						tape_word_t buffer[ WORDS_PER_HEADER_N_SECTOR ];
						for (unsigned i = 0; i < WORDS_PER_HEADER_N_SECTOR; i++) {
							auto res = inp_image.next_word(track , it , bit_idx , buffer[ i ]);
							if (res != hti_format_t::ADV_CONT_DATA) {
								return IMGTOOLERR_CORRUPTIMAGE;
							}
						}
						if (buffer[ 3 ] != checksum(&buffer[ 0 ], 3) ||
							buffer[ 4 + WORDS_PER_SECTOR ] != checksum(&buffer[ 4 ], WORDS_PER_SECTOR)) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						// Check record content
						if (exp_sector != (buffer[ 1 ] & 0xfff)) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						if (((buffer[ 0 ] & FILE_ID_BIT) != 0) != (exp_sector == 0)) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						if ((buffer[ 0 ] & (HDR_W0_ZERO_MASK | RES_FREE_FIELD | SIF_FILE_NO_MASK)) != (RES_FREE_FIELD | SIF_FILE_NO)) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						if ((buffer[ 1 ] & SIF_FREE_FIELD_MASK) != SIF_FREE_FIELD) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						bool in_use = (buffer[ 0 ] & SECTOR_IN_USE) != 0;
						if ((buffer[ 2 ] & BYTES_AVAILABLE_MASK) != BYTES_AVAILABLE ||
							(in_use && (buffer[ 2 ] & BYTES_USED_MASK) != BYTES_USED) ||
							(!in_use && (buffer[ 2 ] & BYTES_USED_MASK) != 0)) {
							return IMGTOOLERR_CORRUPTIMAGE;
						}
						if (in_use) {
							set_sector(exp_sector, &buffer[ 4 ]);
						} else {
							unset_sector(exp_sector);
						}
						exp_sector++;
					}
					pos = it->first;
					state = 2;
				}
				break;

			case 2:
				// Find next gap
				if (!inp_image.next_gap(track , pos , true , MIN_IRG_SIZE)) {
					return IMGTOOLERR_CORRUPTIMAGE;
				}
				state = 1;
				break;

			}
		}
	}

	if (!decode_dir()) {
		return IMGTOOLERR_CORRUPTDIR;
	}

	dirty = false;

	return IMGTOOLERR_SUCCESS;
}

void tape_image_t::save_words(hti_format_t& img , unsigned track , tape_pos_t& pos , const tape_word_t *block , unsigned block_len)
{
	// Preamble
	tape_pos_t length;
	img.write_word(track , pos , PREAMBLE_WORD , length);
	pos += length;
	// Words
	for (unsigned i = 0; i < block_len; i++) {
		img.write_word(track , pos , *block++ , length);
		pos += length;
	}
}

tape_word_t tape_image_t::checksum(const tape_word_t *block , unsigned block_len)
{
	tape_word_t csum = 0;
	for (unsigned i = 0; i < block_len; i++) {
		csum += *block++;
	}
	return csum & 0xffff;
}

imgtoolerr_t tape_image_t::save_to_file(imgtool::stream *stream)
{
	// Encode copies of directory into sectors
	encode_dir();

	// Store sectors into image
	hti_format_t out_image;

	unsigned rec_no = 0;
	for (unsigned track = 0; track < TRACKS_NO; track++) {
		tape_pos_t pos = START_POS;

		// Start of either track
		// Deadzone + 1" of gap
		tape_word_t deadzone[ DZ_WORDS ];
		for (auto& dz : deadzone) {
			dz = PAD_WORD;
		}
		save_words(out_image, track, pos, deadzone, DZ_WORDS);
		pos += IRG_SIZE;

		for (unsigned i = 0; i < SECTORS_PER_TRACK; i++ , rec_no++) {
			bool in_use = alloc_map[ rec_no ];
			// Sector header
			tape_word_t sector[ WORDS_PER_HEADER_N_SECTOR ];

			// Header word 0: file identifier bit, reserved free-field bit, empty record indicator & file #
			sector[ 0 ] = RES_FREE_FIELD | SIF_FILE_NO;
			if (rec_no == 0) {
				sector[ 0 ] |= FILE_ID_BIT;
			}
			if (in_use) {
				sector[ 0 ] |= SECTOR_IN_USE;
			}
			// Header word 1: free-field & sector #
			sector[ 1 ] = SIF_FREE_FIELD | rec_no;
			// Header word 2: bytes available & bytes used
			sector[ 2 ] = BYTES_AVAILABLE;
			if (in_use) {
				sector[ 2 ] |= BYTES_USED;
			}
			// Checksum of header
			sector[ 3 ] = checksum(&sector[ 0 ] , 3);
			// Sector payload
			if (in_use) {
				memcpy(&sector[ 4 ] , &img[ rec_no ][ 0 ] , SECTOR_LEN);
			} else {
				for (unsigned j = 4; j < (4 + WORDS_PER_SECTOR); j++) {
					sector[ j ] = PAD_WORD;
				}
			}
			// Checksum of payload
			sector[ 4 + WORDS_PER_SECTOR ] = checksum(&sector[ 4 ] , WORDS_PER_SECTOR);

			tape_pos_t start_pos = pos;
			save_words(out_image, track, pos, sector, WORDS_PER_HEADER_N_SECTOR);

			// Pad sector up to FORMAT_SECT_SIZE
			while ((pos - start_pos) < FORMAT_SECT_SIZE) {
				tape_pos_t length;
				out_image.write_word(track , pos , PAD_WORD , length);
				pos += length;
			}

			// Gap between sectors
			if (rec_no == 0) {
				pos += IFG_SIZE;
			} else {
				pos += IRG_SIZE;
			}
		}
	}

	out_image.save_tape(*imgtool::stream_read_write(*stream, 0));

	return IMGTOOLERR_SUCCESS;
}

unsigned tape_image_t::free_sectors(void) const
{
	std::bitset<TOT_SECTORS> tmp(alloc_map);

	// Reserve sectors that cannot be allocated to files
	for (unsigned i = 0; i < FIRST_FILE_SECTOR; i++) {
		tmp[ i ] = true;
	}

	return TOT_SECTORS - tmp.count();
}

void tape_image_t::set_sector(unsigned s_no , const tape_word_t *s_data)
{
	if (s_no < TOT_SECTORS) {
		memcpy(&img[ s_no ][ 0 ] , s_data , SECTOR_LEN);
		alloc_map.set(s_no);
		dirty = true;
	}
}

void tape_image_t::unset_sector(unsigned s_no)
{
	if (s_no < TOT_SECTORS) {
		alloc_map.reset(s_no);
		dirty = true;
	}
}

bool tape_image_t::get_sector(unsigned s_no , tape_word_t *s_data)
{
	if (s_no < TOT_SECTORS && alloc_map[ s_no ]) {
		memcpy(s_data , &img[ s_no ][ 0 ] , SECTOR_LEN);
		return true;
	} else {
		return false;
	}
}

bool tape_image_t::get_dir_entry(unsigned idx , const dir_entry_t*& entry) const
{
	if (idx >= dir.size()) {
		return false;
	} else {
		entry = &dir[ idx ];
		return true;
	}
}

bool tape_image_t::find_file(const char *filename , bool ignore_ext , unsigned& idx) const
{
	char fname[ CHARS_PER_FNAME_EXT + 1 ];
	char ext[ CHARS_PER_EXT + 1 ];

	split_filename_and_ext(filename, fname, ext);

	bool has_ext = !ignore_ext && *ext != '\0';

	if (has_ext) {
		strcat(fname , ".");
		strcat(fname , ext);
	}

	for (auto i = dir.cbegin(); i < dir.cend(); i++) {
		char full_fname[ CHARS_PER_FNAME_EXT + 1 ];
		bool qmark;

		get_filename_and_ext(*i, has_ext, full_fname, qmark);

		if (strcmp(fname , full_fname) == 0) {
			idx = i - dir.cbegin();
			return true;
		}
	}

	return false;
}

void tape_image_t::delete_dir_entry(unsigned idx)
{
	const dir_entry_t& ent = dir[ idx ];

	// Release all sectors of file
	for (unsigned i = ent.filepos; i < ent.filepos + ent.n_sects; i++) {
		unset_sector(i);
	}

	dir.erase(dir.begin() + idx);
	dirty = true;
}

bool tape_image_t::find_free_block(unsigned blocks , unsigned& first_s) const
{
	if (blocks >= (TOT_SECTORS - FIRST_FILE_SECTOR)) {
		return false;
	}

	std::bitset<TOT_SECTORS> scanner;

	for (unsigned i = FIRST_FILE_SECTOR; i < (FIRST_FILE_SECTOR + blocks); i++) {
		scanner[ i ] = true;
	}

	for (unsigned i = FIRST_FILE_SECTOR; i <= (TOT_SECTORS - blocks); i++) {
		std::bitset<TOT_SECTORS> tmp_map(alloc_map & scanner);
		if (tmp_map.none()) {
			first_s = i;
			return true;
		}
		scanner <<= 1;
	}

	return false;
}

bool tape_image_t::alloc_new_file(unsigned blocks , dir_entry_t*& entry)
{
	if (dir.size() >= MAX_DIR_ENTRIES) {
		return false;
	}

	dir_entry_t new_entry;

	memset(&new_entry.filename[ 0 ] , 0 , sizeof(new_entry.filename));
	new_entry.protection = 0;
	new_entry.filetype = 0;
	new_entry.n_recs = 0;
	new_entry.wpr = 0;

	unsigned first_s;

	if (!find_free_block(blocks, first_s)) {
		return false;
	}

	new_entry.filepos = (uint16_t)first_s;
	new_entry.n_sects = blocks;

	dir.push_back(new_entry);
	entry = &dir.back();
	dirty = true;

	return true;
}

void tape_image_t::split_filename_and_ext(const char *filename , char *fname , char *ext)
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

void tape_image_t::wipe_sector(tape_word_t *s)
{
	for (unsigned i = 0; i < WORDS_PER_SECTOR; i++) {
		s[ i ] = PAD_WORD;
	}
}

void tape_image_t::tape_word_to_bytes(tape_word_t w , uint8_t& bh , uint8_t& bl)
{
	bh = (uint8_t)(w >> 8);
	bl = (uint8_t)(w & 0xff);
}

void tape_image_t::bytes_to_tape_word(uint8_t bh , uint8_t bl , tape_word_t& w)
{
	w = ((tape_word_t)bh << 8) | ((tape_word_t)bl);
}

void tape_image_t::dump_dir_sect(const tape_word_t *dir_sect , unsigned dir_sect_idx)
{
	for (unsigned i = 0; i < DIR_COPIES; i++) {
		set_sector(FIRST_DIR_SECTOR + i * SECTORS_PER_DIR + dir_sect_idx, dir_sect);
	}
}

void tape_image_t::fill_and_dump_dir_sect(tape_word_t *dir_sect , unsigned& idx , unsigned& dir_sect_idx , tape_word_t w)
{
	// Dump sector once it's full
	if (idx >= WORDS_PER_SECTOR) {
		dump_dir_sect(dir_sect, dir_sect_idx);
		wipe_sector(dir_sect);
		idx = 0;
		dir_sect_idx++;
	}
	dir_sect[ idx++ ] = w;
}

void tape_image_t::encode_dir(void)
{
	tape_word_t dir_sect[ WORDS_PER_SECTOR ];

	wipe_sector(dir_sect);

	unsigned idx = 0;
	unsigned dir_sect_idx = 0;

	fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, DIR_WORD_0);
	fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, DIR_WORD_1);

	for (const dir_entry_t& ent : dir) {
		tape_word_t tmp;

		// Filename
		bytes_to_tape_word(ent.filename[ 0 ], ent.filename[ 1 ], tmp);
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, tmp);
		bytes_to_tape_word(ent.filename[ 2 ], ent.filename[ 3 ], tmp);
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, tmp);
		bytes_to_tape_word(ent.filename[ 4 ], ent.filename[ 5 ], tmp);
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, tmp);
		// Protection, file type & file position
		tmp = ((tape_word_t)ent.filetype << 10) | (tape_word_t)ent.filepos;
		if (ent.protection) {
			tmp |= 0x8000;
		}
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, tmp);
		// File size (# of records)
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, ent.n_recs);
		// Words per record
		fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, ent.wpr);
	}

	// Terminator
	fill_and_dump_dir_sect(dir_sect, idx, dir_sect_idx, DIR_LAST_WORD);

	// Dump last partial sector
	dump_dir_sect(dir_sect, dir_sect_idx);

	// Unset unused sectors
	for (unsigned i = dir_sect_idx + 1; i < SECTORS_PER_DIR; i++) {
		for (unsigned j = 0; j < DIR_COPIES; j++) {
			unset_sector(FIRST_DIR_SECTOR + i + j * SECTORS_PER_DIR);
		}
	}
}

bool tape_image_t::read_sector_words(unsigned& sect_no , unsigned& sect_idx , size_t word_no , tape_word_t *out) const
{
	while (word_no > 0) {
		if (sect_idx >= WORDS_PER_SECTOR) {
			sect_idx = 0;
			sect_no++;
			if (sect_no >= TOT_SECTORS || !alloc_map[ sect_no ]) {
				return false;
			}
		}
		*out++ = img[ sect_no ][ sect_idx ];
		sect_idx++;
		word_no--;
	}

	return true;
}

bool tape_image_t::filename_char_check(uint8_t c)
{
	// Colons and quotation marks are forbidden in file names
	return 0x20 < c && c < 0x7f && c != ':' && c != '"';
}

bool tape_image_t::filename_check(const uint8_t *filename)
{
	bool ended = false;

	for (unsigned i = 0; i < 6; i++) {
		uint8_t c = *filename++;

		if (ended) {
			if (c != 0) {
				return false;
			}
		} else if (c == 0) {
			ended = true;
		} else if (!filename_char_check(c)) {
			return false;
		}
	}

	return true;
}

static const char *const filetype_attrs[] = {
	BKUP_ATTR_STR,  // 0
	DATA_ATTR_STR,  // 1
	PROG_ATTR_STR,  // 2
	KEYS_ATTR_STR,  // 3
	BDAT_ATTR_STR,  // 4
	ALL_ATTR_STR,   // 5
	BPRG_ATTR_STR,  // 6
	OPRM_ATTR_STR   // 7
};

void tape_image_t::get_filename_and_ext(const dir_entry_t& ent , bool inc_ext , char *out , bool& qmark)
{
	strncpy(&out[ 0 ] , (const char*)&ent.filename[ 0 ] , CHARS_PER_FNAME);
	out[ CHARS_PER_FNAME ] = '\0';

	// Decode filetype
	uint8_t type_low = ent.filetype & 7;
	uint8_t type_hi = (ent.filetype >> 3) & 3;

	const char *filetype_str = filetype_attrs[ type_low ];

	// Same logic used by hp9845b to add a question mark next to filetype
	qmark = (type_low == DATA_FILETYPE && type_hi == 3) ||
				  (type_low != DATA_FILETYPE && type_hi != 2);

	if (inc_ext) {
		strcat(out , ".");
		strcat(out , filetype_str);
	}
}

bool tape_image_t::decode_dir(void)
{
	unsigned sect_no = FIRST_DIR_SECTOR - 1;
	unsigned sect_idx = SECTOR_LEN;

	dir.clear();

	tape_word_t tmp;

	if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
		return false;
	}
	if (tmp != DIR_WORD_0) {
		return false;
	}
	if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
		return false;
	}
	if (tmp != DIR_WORD_1) {
		return false;
	}

	// This is to check for overlapping files
	std::bitset<TOT_SECTORS> sect_in_use;

	while (1) {
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		if (tmp == DIR_LAST_WORD) {
			// End of directory
			break;
		}

		if (dir.size() >= MAX_DIR_ENTRIES) {
			// Too many entries
			return false;
		}

		dir_entry_t new_entry;

		// Filename
		tape_word_to_bytes(tmp, new_entry.filename[ 0 ], new_entry.filename[ 1 ]);
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		tape_word_to_bytes(tmp, new_entry.filename[ 2 ], new_entry.filename[ 3 ]);
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		tape_word_to_bytes(tmp, new_entry.filename[ 4 ], new_entry.filename[ 5 ]);
		if (!filename_check(new_entry.filename)) {
			return false;
		}

		// Protection, file type & file position
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		new_entry.protection = (tmp & 0x8000) != 0;
		new_entry.filetype = ((tmp >> 10) & 0x1f);
		new_entry.filepos = tmp & 0x3ff;
		if (new_entry.filepos < FIRST_FILE_SECTOR || new_entry.filepos >= TOT_SECTORS) {
			return false;
		}

		// File size (# of records)
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		new_entry.n_recs = tmp;

		// Words per record
		if (!read_sector_words(sect_no, sect_idx, 1, &tmp)) {
			return false;
		}
		new_entry.wpr = tmp;
		if (new_entry.wpr < 1) {
			return false;
		}

		new_entry.n_sects = ((unsigned)new_entry.wpr * new_entry.n_recs * 2 + SECTOR_LEN - 1) / SECTOR_LEN;
		if (new_entry.n_sects < 1 || (new_entry.n_sects + new_entry.filepos) > TOT_SECTORS) {
			return false;
		}

		for (unsigned i = new_entry.filepos; i < new_entry.n_sects + new_entry.filepos; i++) {
			if (sect_in_use[ i ]) {
				return false;
			}
			sect_in_use[ i ] = true;
		}

		dir.push_back(new_entry);
	}

	// Check for inconsistency between alloc_map & sect_in_use
	for (unsigned i = 0; i < FIRST_FILE_SECTOR; i++) {
		sect_in_use[ i ] = alloc_map[ i ];
	}

	std::bitset<TOT_SECTORS> tmp_map(~alloc_map & sect_in_use);
	if (tmp_map.any()) {
		// There is at least 1 sector that is in use by a file but it's empty/unallocated
		return false;
	}

	alloc_map = sect_in_use;

	return true;
}

static tape_state_t& get_tape_state(imgtool::image &img)
{
	tape_state_t *ts = (tape_state_t*)img.extra_bytes();

	return *ts;
}

static tape_image_t& get_tape_image(tape_state_t& ts)
{
	if (ts.img == nullptr) {
		ts.img = new tape_image_t;
	}

	return *(ts.img);
}

/********************************************************************************
 * Imgtool functions
 ********************************************************************************/
static imgtoolerr_t hp9845_tape_open(imgtool::image &image, imgtool::stream::ptr &&stream)
{
	tape_state_t& state = get_tape_state(image);

	state.stream = stream.release();

	tape_image_t& tape_image = get_tape_image(state);

	imgtoolerr_t err = tape_image.load_from_file(state.stream);
	if (err)
		return err;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_create(imgtool::image &image, imgtool::stream::ptr &&stream, util::option_resolution *opts)
{
	tape_state_t& state = get_tape_state(image);

	state.stream = stream.release();

	tape_image_t& tape_image = get_tape_image(state);

	tape_image.format_img();

	return IMGTOOLERR_SUCCESS;
}

static void hp9845_tape_close(imgtool::image &image)
{
	tape_state_t& state = get_tape_state(image);
	tape_image_t& tape_image = get_tape_image(state);

	if (tape_image.is_dirty()) {
		(void)tape_image.save_to_file(state.stream);
	}

	delete state.stream;

	// Free tape_image
	delete &tape_image;
}

static imgtoolerr_t hp9845_tape_begin_enum (imgtool::directory &enumeration, const char *path)
{
	dir_state_t *ds = (dir_state_t*)enumeration.extra_bytes();

	ds->dir_idx = 0;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_next_enum (imgtool::directory &enumeration, imgtool_dirent &ent)
{
	tape_state_t& state = get_tape_state(enumeration.image());
	tape_image_t& tape_image = get_tape_image(state);
	dir_state_t *ds = (dir_state_t*)enumeration.extra_bytes();

	const dir_entry_t *entry = nullptr;

	if (!tape_image.get_dir_entry(ds->dir_idx, entry)) {
		ent.eof = 1;
	} else {
		ds->dir_idx++;

		bool qmark;

		tape_image_t::get_filename_and_ext(*entry, true, ent.filename, qmark);

		// "filename" and "attr" fields try to look like the output of the "CAT" command
		snprintf(ent.attr , sizeof(ent.attr) , "%c %02x%c %4u %4u %3u" , entry->protection ? '*' : ' ' , entry->filetype , qmark ? '?' : ' ' , entry->n_recs , entry->wpr * 2 , entry->filepos);

		ent.filesize = entry->n_sects * SECTOR_LEN;
	}
	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_free_space(imgtool::partition &partition, uint64_t *size)
{
	tape_state_t& state = get_tape_state(partition.image());
	tape_image_t& tape_image = get_tape_image(state);

	*size = tape_image.free_sectors() * SECTOR_LEN;

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_read_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	tape_state_t& state = get_tape_state(partition.image());
	tape_image_t& tape_image = get_tape_image(state);

	unsigned idx;

	if (!tape_image.find_file(filename , false , idx)) {
		return IMGTOOLERR_FILENOTFOUND;
	}

	const dir_entry_t *ent = nullptr;

	tape_image.get_dir_entry(idx, ent);

	unsigned sect_no = ent->filepos;
	unsigned n_sects = ent->n_sects;
	tape_word_t buff_w[ WORDS_PER_SECTOR ];
	uint8_t buff_b[ SECTOR_LEN ];

	while (n_sects--) {
		if (!tape_image.get_sector(sect_no++, &buff_w[ 0 ])) {
			return IMGTOOLERR_READERROR;
		}
		for (unsigned i = 0; i < WORDS_PER_SECTOR; i++) {
			tape_image_t::tape_word_to_bytes(buff_w[ i ], buff_b[ i * 2 ], buff_b[ i * 2 + 1 ]);
		}

		destf.write(buff_b , SECTOR_LEN);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_write_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	tape_state_t& state = get_tape_state(partition.image());
	tape_image_t& tape_image = get_tape_image(state);

	unsigned idx;

	if (tape_image.find_file(filename , true , idx)) {
		// When overwriting a file, delete its old version first
		tape_image.delete_dir_entry(idx);
	}

	unsigned blocks = (unsigned)((sourcef.size() + SECTOR_LEN - 1) / SECTOR_LEN);

	if (!blocks) {
		fprintf(stderr , "Null file, not writing..\n");
		return IMGTOOLERR_SUCCESS;
	}

	dir_entry_t *ent = nullptr;

	if (!tape_image.alloc_new_file(blocks, ent)) {
		return IMGTOOLERR_NOSPACE;
	}

	unsigned s_no = ent->filepos;

	char fname[ CHARS_PER_FNAME + 1 ];
	char ext[ CHARS_PER_EXT + 1 ];

	tape_image_t::split_filename_and_ext(filename, fname, ext);

	strncpy((char*)&ent->filename[ 0 ] , fname , CHARS_PER_FNAME);

	for (unsigned i = 0; i < blocks; i++) {
		tape_word_t buff_w[ WORDS_PER_SECTOR ];
		uint8_t buff_b[ SECTOR_LEN ];

		memset(&buff_b[ 0 ] , 0 , sizeof(buff_b));

		if (sourcef.read(buff_b , SECTOR_LEN) != SECTOR_LEN && i != (blocks - 1)) {
			return IMGTOOLERR_READERROR;
		}
		for (unsigned j = 0; j < WORDS_PER_SECTOR; j++) {
			tape_image_t::bytes_to_tape_word(buff_b[ 2 * j ], buff_b[ 2 * j + 1 ], buff_w[ j ]);
		}
		tape_image.set_sector(s_no, buff_w);
		s_no++;
	}

	int wpr = opts->lookup_int('W');
	if (wpr == 0) {
		wpr = WORDS_PER_SECTOR;
	} else if (wpr > (blocks * WORDS_PER_SECTOR)) {
		fprintf(stderr , "WPR value too large, using %u\n" , WORDS_PER_SECTOR);
		wpr = WORDS_PER_SECTOR;
	}
	ent->wpr = (uint16_t)wpr;

	ent->n_recs = (uint16_t)((blocks * WORDS_PER_SECTOR) / wpr);

	unsigned type_low;

	if (opts->lookup_int('T') == 0) {
		// File type defaults to DATA if no extension is given or extension is invalid
		type_low = DATA_FILETYPE;
		for (unsigned i = 0; i < 8; i++) {
			if (strcmp(filetype_attrs[ i ] , ext) == 0) {
				type_low = i;
				break;
			}
		}
	} else {
		type_low = opts->lookup_int('T') - 1;
	}

	// See tape_image_t::get_filename_and_ext for the logic behind file type
	if (type_low == DATA_FILETYPE) {
		ent->filetype = (uint8_t)type_low + (1U << 3);
	} else {
		ent->filetype = (uint8_t)type_low + (2U << 3);
	}

	return IMGTOOLERR_SUCCESS;
}

static imgtoolerr_t hp9845_tape_delete_file(imgtool::partition &partition, const char *filename)
{
	tape_state_t& state = get_tape_state(partition.image());
	tape_image_t& tape_image = get_tape_image(state);

	unsigned idx;

	if (!tape_image.find_file(filename , true , idx)) {
		return IMGTOOLERR_FILENOTFOUND;
	}

	tape_image.delete_dir_entry(idx);

	return IMGTOOLERR_SUCCESS;
}

#define HP9845_WRITEFILE_OPTSPEC    "W[0]-65535;T[0]-8"

OPTION_GUIDE_START(hp9845_write_optguide)
	OPTION_INT('W' , "wpr" , "Words per record")
	OPTION_ENUM_START('T' , "ftype" , "File type")
	OPTION_ENUM(0 , "auto" , "Automatic (\"DATA\" or by extension)")
	OPTION_ENUM(1 , "U"    , "BKUP")
	OPTION_ENUM(2 , "D"    , "DATA")
	OPTION_ENUM(3 , "P"    , "PROG")
	OPTION_ENUM(4 , "K"    , "KEYS")
	OPTION_ENUM(5 , "T"    , "BDAT")
	OPTION_ENUM(6 , "A"    , "ALL")
	OPTION_ENUM(7 , "B"    , "BPRG")
	OPTION_ENUM(8 , "O"    , "OPRM")
	OPTION_ENUM_END
OPTION_GUIDE_END

void hp9845_tape_get_info(const imgtool_class *imgclass, uint32_t state, union imgtoolinfo *info)
{
	switch (state) {
	case IMGTOOLINFO_STR_NAME:
		strcpy(info->s = imgtool_temp_str(), "hp9845_tape");
		break;

	case IMGTOOLINFO_STR_DESCRIPTION:
		strcpy(info->s = imgtool_temp_str(), "HP9845 tape image");
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
		info->open = hp9845_tape_open;
		break;

	case IMGTOOLINFO_PTR_CREATE:
		info->create = hp9845_tape_create;
		break;

	case IMGTOOLINFO_PTR_CLOSE:
		info->close = hp9845_tape_close;
		break;

	case IMGTOOLINFO_PTR_BEGIN_ENUM:
		info->begin_enum = hp9845_tape_begin_enum;
		break;

	case IMGTOOLINFO_PTR_NEXT_ENUM:
		info->next_enum = hp9845_tape_next_enum;
		break;

	case IMGTOOLINFO_PTR_FREE_SPACE:
		info->free_space = hp9845_tape_free_space;
		break;

	case IMGTOOLINFO_PTR_READ_FILE:
		info->read_file = hp9845_tape_read_file;
		break;

	case IMGTOOLINFO_PTR_WRITE_FILE:
		info->write_file = hp9845_tape_write_file;
		break;

	case IMGTOOLINFO_PTR_DELETE_FILE:
		info->delete_file = hp9845_tape_delete_file;
		break;

	case IMGTOOLINFO_PTR_WRITEFILE_OPTGUIDE:
		info->writefile_optguide = &hp9845_write_optguide;
		break;

	case IMGTOOLINFO_STR_WRITEFILE_OPTSPEC:
		strcpy(info->s = imgtool_temp_str(), HP9845_WRITEFILE_OPTSPEC);
		break;
	}
}

/********************************************************************************
 * Filter functions
 ********************************************************************************/
static unsigned len_to_eor(imgtool::stream &inp)
{
	return SECTOR_LEN - (unsigned)(inp.tell() % SECTOR_LEN);
}

static bool get_record_part(imgtool::stream &inp , void *buf , unsigned len)
{
	// Reading must never cross sector boundary
	if (len > len_to_eor(inp)) {
		return false;
	}

	return inp.read(buf, len) == len;
}

static bool dump_string(imgtool::stream &inp, imgtool::stream &out , unsigned len , bool add_eoln)
{
	uint8_t tmp[ SECTOR_LEN ];

	if (!get_record_part(inp , tmp , len)) {
		return false;
	}

	// Sanitize string
	for (unsigned i = 0; i < len; i++) {
		if (!isascii(tmp[ i ]) || !isprint(tmp[ i ])) {
			tmp[ i ] = ' ';
		}
	}

	out.write(tmp , len);
	if (add_eoln) {
		out.puts(EOLN);
	}

	return true;
}

static imgtoolerr_t hp9845data_read_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &destf)
{
	imgtool::stream::ptr inp_data;
	imgtoolerr_t res;
	uint8_t tmp[ 2 ];

	inp_data = imgtool::stream::open_mem(NULL , 0);
	if (!inp_data)
		return IMGTOOLERR_OUTOFMEMORY;

	res = hp9845_tape_read_file(partition , filename , fork , *inp_data);
	if (res != IMGTOOLERR_SUCCESS)
		return res;

	inp_data->seek(0, SEEK_SET);

	uint16_t rec_type;
	unsigned rec_len;
	unsigned tmp_len;
	unsigned accum_len = 0;

	do {
		// Get record type
		if (!get_record_part(*inp_data , tmp , 2)) {
			return IMGTOOLERR_READERROR;
		}
		rec_type = (uint16_t)pick_integer_be(tmp , 0 , 2);
		switch (rec_type) {
		case REC_TYPE_EOR:
			// End of record: just skip it
			break;

		case REC_TYPE_FULLSTR:
			// A string in a single piece
		case REC_TYPE_1STSTR:
			// First piece of a split string
		case REC_TYPE_MIDSTR:
			// Mid piece(s) of a split string
		case REC_TYPE_ENDSTR:
			// Closing piece of a split string
			if (((rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_1STSTR) && accum_len > 0) ||
				((rec_type == REC_TYPE_MIDSTR || rec_type == REC_TYPE_ENDSTR) && accum_len == 0)) {
				fputs("Wrong sequence of string pieces\n" , stderr);
				return IMGTOOLERR_CORRUPTFILE;
			}

			if (!get_record_part(*inp_data , tmp , 2)) {
				return IMGTOOLERR_READERROR;
			}
			tmp_len = (unsigned)pick_integer_be(tmp , 0 , 2);

			if (rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_1STSTR) {
				accum_len = tmp_len;
			} else if (tmp_len != accum_len) {
				fputs("Wrong length of string piece\n" , stderr);
				return IMGTOOLERR_CORRUPTFILE;
			}

			if (rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_ENDSTR) {
				rec_len = accum_len;
			} else {
				rec_len = std::min(accum_len , len_to_eor(*inp_data));
			}
			if (!dump_string(*inp_data , destf , rec_len , rec_type == REC_TYPE_FULLSTR || rec_type == REC_TYPE_ENDSTR)) {
				return IMGTOOLERR_READERROR;
			}
			if (rec_len & 1) {
				// Keep length of string pieces even
				get_record_part(*inp_data , tmp , 1);
			}
			accum_len -= rec_len;
			break;

		case REC_TYPE_EOF:
			// End of file
			break;

		default:
			fprintf(stderr , "Unknown record type (%04x)\n" , rec_type);
			return IMGTOOLERR_CORRUPTFILE;
		}
	} while (rec_type != REC_TYPE_EOF);

	return IMGTOOLERR_SUCCESS;
}

static bool split_string_n_dump(const char *s , imgtool::stream &dest)
{
	unsigned s_len = strlen(s);
	uint16_t rec_type = REC_TYPE_1STSTR;
	uint8_t tmp[ 4 ];
	bool at_least_one = false;

	while (1) {
		unsigned free_len = len_to_eor(dest);
		if (free_len <= 4) {
			// Not enough free space at end of current record: fill with EORs
			place_integer_be(tmp , 0 , 2 , REC_TYPE_EOR);
			while (free_len) {
				if (dest.write(tmp , 2) != 2) {
					return false;
				}
				free_len -= 2;
			}
		} else {
			unsigned s_part_len = std::min(free_len - 4 , s_len);
			if (s_part_len == s_len) {
				// Free space to EOR enough for what's left of string
				break;
			}
			place_integer_be(tmp , 0 , 2 , rec_type);
			place_integer_be(tmp , 2 , 2 , s_len);
			if (dest.write(tmp , 4) != 4 ||
				dest.write(s, s_part_len) != s_part_len) {
				return false;
			}
			rec_type = REC_TYPE_MIDSTR;
			s_len -= s_part_len;
			s += s_part_len;
			at_least_one = true;
		}
	}

	place_integer_be(tmp , 0 , 2 , at_least_one ? REC_TYPE_ENDSTR : REC_TYPE_FULLSTR);
	place_integer_be(tmp , 2 , 2 , s_len);
	if (dest.write(tmp , 4) != 4 ||
		dest.write(s , s_len) != s_len) {
		return false;
	}
	if (s_len & 1) {
		tmp[ 0 ] = 0;
		if (dest.write(tmp , 1) != 1) {
			return false;
		}
	}
	return true;
}

static imgtoolerr_t hp9845data_write_file(imgtool::partition &partition, const char *filename, const char *fork, imgtool::stream &sourcef, util::option_resolution *opts)
{
	imgtool::stream::ptr out_data;

	out_data = imgtool::stream::open_mem(NULL , 0);
	if (!out_data)
		return IMGTOOLERR_OUTOFMEMORY;

	while (1) {
		char line[ 256 ];

		// Read input file one line at time
		if (sourcef.core_file()->gets(line , sizeof(line)) == nullptr) {
			// EOF
			break;
		}
		line[ sizeof(line) - 1 ] = '\0';

		// Strip space and non-ASCII characters from the end of the line
		size_t line_len = strlen(line);
		char *p = &line[ line_len ];
		while (p != line) {
			char c = *(--p);
			if (isascii(c) && !isspace(c)) {
				break;
			}
			*p = '\0';
		}

		// Ignore empty lines
		if (p == line) {
			continue;
		}

		if (!split_string_n_dump(line, *out_data)) {
			return IMGTOOLERR_WRITEERROR;
		}
	}

	// Fill free space of last record with EOFs
	unsigned free_len = len_to_eor(*out_data);
	uint8_t tmp[ 2 ];
	place_integer_be(tmp , 0 , 2 , REC_TYPE_EOF);

	while (free_len) {
		if (out_data->write(tmp , 2 ) != 2) {
			return IMGTOOLERR_WRITEERROR;
		}
		free_len -= 2;
	}

	out_data->seek(0 , SEEK_SET);

	imgtoolerr_t res = hp9845_tape_write_file(partition, filename, fork, *out_data, opts);

	return res;
}

void filter_hp9845data_getinfo(uint32_t state, union filterinfo *info)
{
	switch (state) {
	case FILTINFO_PTR_READFILE:
		info->read_file = hp9845data_read_file;
		break;

	case FILTINFO_PTR_WRITEFILE:
		info->write_file = hp9845data_write_file;
		break;

	case FILTINFO_STR_NAME:
		info->s = "9845data";
		break;

	case FILTINFO_STR_HUMANNAME:
		info->s = "HP9845 text-only DATA files";
		break;

	case FILTINFO_STR_EXTENSION:
		info->s = "txt";
		break;
	}
}
