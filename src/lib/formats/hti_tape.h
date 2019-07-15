// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    "HTI" format

    Format of images of DC-100 tape cassettes as used in HP 9845
    and HP 85 systems.

*********************************************************************/
#ifndef MAME_FORMATS_HTI_TAPE_H
#define MAME_FORMATS_HTI_TAPE_H

#pragma once

#include "ioprocs.h"

#include <map>

class hti_format_t
{
public:
	hti_format_t();

	// Tape position, 1 unit = 1 inch / (968 * 1024)
	typedef int32_t tape_pos_t;

	// 1 inch in tape_pos_t
	static constexpr tape_pos_t ONE_INCH_POS = 968 * 1024;

	// Special value for invalid/unknown tape position
	static constexpr tape_pos_t NULL_TAPE_POS = -1;

	// Tape length: 140 ft of usable tape + 72" of punched tape at either end
	static constexpr tape_pos_t TAPE_LENGTH = (140 * 12 + 72 * 2) * ONE_INCH_POS;

	// Length of 0 bits at slow tape speed: 1/(35200 Hz)
	static constexpr tape_pos_t ZERO_BIT_LEN = 619;

	// Length of 1 bits at slow tape speed: 1.75 times ZERO_BIT_LEN
	static constexpr tape_pos_t ONE_BIT_LEN = 1083;

	// Words stored on tape
	typedef uint16_t tape_word_t;

	// Storage of tracks: mapping from a tape position to word stored there
	typedef std::map<tape_pos_t, tape_word_t> tape_track_t;

	// Iterator to access words on tape
	typedef tape_track_t::iterator track_iterator_t;

	bool load_tape(io_generic *io);
	void save_tape(io_generic *io);
	void clear_tape();

	// Return physical length of a 16-bit word on tape
	static tape_pos_t word_length(tape_word_t w);

	static tape_pos_t farthest_end(const track_iterator_t& it , bool forward);

	static bool pos_offset(tape_pos_t& pos , bool forward , tape_pos_t offset);

	// Position of next hole tape will reach in a given direction
	static tape_pos_t next_hole(tape_pos_t pos , bool forward);

	// Write a data word on tape
	void write_word(unsigned track_no , tape_pos_t start , tape_word_t word , tape_pos_t& length , bool forward = true);
	// Write a gap on tape
	void write_gap(unsigned track_no , tape_pos_t a , tape_pos_t b);

	// Check that a section of tape has no data (it's just gap)
	bool just_gap(unsigned track_no , tape_pos_t a , tape_pos_t b);

	// Return position of next data word in a given direction
	bool next_data(unsigned track_no , tape_pos_t pos , bool forward , bool inclusive , track_iterator_t& it);

	enum adv_res_t
	{
		ADV_NO_MORE_DATA,
		ADV_CONT_DATA,
		ADV_DISCONT_DATA
	};

	// Advance an iterator to next word of data
	adv_res_t adv_it(unsigned track_no , bool forward , track_iterator_t& it);

	// Scan for beginning of next gap in a given direction
	bool next_gap(unsigned track_no , tape_pos_t& pos , bool forward , tape_pos_t min_gap);

private:
	// Content of tape tracks
	tape_track_t m_tracks[ 2 ];

	static bool load_track(io_generic *io , uint64_t& offset , tape_track_t& track);
	static void dump_sequence(io_generic *io , uint64_t& offset , tape_track_t::const_iterator it_start , unsigned n_words);

	static tape_pos_t word_end_pos(const track_iterator_t& it);
	static void adjust_it(tape_track_t& track , track_iterator_t& it , tape_pos_t pos);
	static void ensure_a_lt_b(tape_pos_t& a , tape_pos_t& b);
};

#endif // MAME_FORMATS_HTI_TAPE_H
