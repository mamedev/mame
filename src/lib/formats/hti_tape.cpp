// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    "HTI" format

*********************************************************************/

#include "hti_tape.h"

#include "imageutl.h"


static constexpr uint32_t FILE_MAGIC =  0x5441434f;      // Magic value at start of image file: "TACO"

// *** Position of tape holes ***
// At beginning of tape:
// *START*
// |<-----24"----->|<---12"--->|<---12"--->|<-----24"----->|
//               O   O       O   O       O   O             O
//               |<->|       |<->|       |<->|
//               0.218"      0.218"      0.218"
// At end of tape:
//                                                     *END*
// |<-----24"----->|<---12"--->|<---12"--->|<-----24"----->|
// O               O           O           O
//
static const hti_format_t::tape_pos_t tape_holes[] = {
	(hti_format_t::tape_pos_t)(23.891 * hti_format_t::ONE_INCH_POS),    // 24 - 0.218 / 2
	(hti_format_t::tape_pos_t)(24.109 * hti_format_t::ONE_INCH_POS),    // 24 + 0.218 / 2
	(hti_format_t::tape_pos_t)(35.891 * hti_format_t::ONE_INCH_POS),    // 36 - 0.218 / 2
	(hti_format_t::tape_pos_t)(36.109 * hti_format_t::ONE_INCH_POS),    // 36 + 0.218 / 2
	(hti_format_t::tape_pos_t)(47.891 * hti_format_t::ONE_INCH_POS),    // 48 - 0.218 / 2
	(hti_format_t::tape_pos_t)(48.109 * hti_format_t::ONE_INCH_POS),    // 48 + 0.218 / 2
	72 * hti_format_t::ONE_INCH_POS,      // 72
	1752 * hti_format_t::ONE_INCH_POS,    // 1752
	1776 * hti_format_t::ONE_INCH_POS,    // 1776
	1788 * hti_format_t::ONE_INCH_POS,    // 1788
	1800 * hti_format_t::ONE_INCH_POS     // 1800
};

hti_format_t::hti_format_t()
{
	clear_tape();
}

bool hti_format_t::load_tape(io_generic *io)
{
	uint8_t tmp[ 4 ];

	io_generic_read(io, tmp, 0, 4);
	if (pick_integer_be(tmp , 0 , 4) != FILE_MAGIC) {
		return false;
	}

	uint64_t offset = 4;

	for (tape_track_t& track : m_tracks) {
		if (!load_track(io , offset , track)) {
			clear_tape();
			return false;
		}
	}

	return true;
}

void hti_format_t::save_tape(io_generic *io)
{
	uint8_t tmp[ 4 ];

	place_integer_be(tmp, 0, 4, FILE_MAGIC);
	io_generic_write(io, tmp, 0, 4);

	uint64_t offset = 4;

	for (const tape_track_t& track : m_tracks) {
		tape_pos_t next_pos = (tape_pos_t)-1;
		unsigned n_words = 0;
		tape_track_t::const_iterator it_start;
		for (tape_track_t::const_iterator it = track.cbegin(); it != track.cend(); ++it) {
			if (it->first != next_pos) {
				dump_sequence(io , offset , it_start , n_words);
				it_start = it;
				n_words = 0;
			}
			next_pos = it->first + word_length(it->second);
			n_words++;
		}
		dump_sequence(io , offset , it_start , n_words);
		// End of track
		place_integer_le(tmp, 0, 4, (uint32_t)-1);
		io_generic_write(io, tmp, offset, 4);
		offset += 4;
	}
}

void hti_format_t::clear_tape()
{
	for (tape_track_t& track : m_tracks) {
		track.clear();
	}
}

hti_format_t::tape_pos_t hti_format_t::word_length(tape_word_t w)
{
	unsigned zeros , ones;

	// pop count of w
	ones = (w & 0x5555) + ((w >> 1) & 0x5555);
	ones = (ones & 0x3333) + ((ones >> 2) & 0x3333);
	ones = (ones & 0x0f0f) + ((ones >> 4) & 0x0f0f);
	ones = (ones & 0x00ff) + ((ones >> 8) & 0x00ff);

	zeros = 16 - ones;

	return zeros * ZERO_BIT_LEN + (ones + 1) * ONE_BIT_LEN;
}

hti_format_t::tape_pos_t hti_format_t::farthest_end(const track_iterator_t& it , bool forward)
{
	if (forward) {
		return word_end_pos(it);
	} else {
		return it->first;
	}
}

bool hti_format_t::pos_offset(tape_pos_t& pos , bool forward , tape_pos_t offset)
{
	if (offset == 0) {
		return true;
	}

	if (!forward) {
		offset = -offset;
	}

	pos += offset;

	// In real life tape would unspool..
	if (pos > TAPE_LENGTH) {
		pos = TAPE_LENGTH;
		return false;
	} else if (pos < 0) {
		pos = 0;
		return false;
	} else {
		return true;
	}
}

hti_format_t::tape_pos_t hti_format_t::next_hole(tape_pos_t pos , bool forward)
{
	if (forward) {
		for (tape_pos_t hole : tape_holes) {
			if (hole > pos) {
				return hole;
			}
		}
		// No more holes: will hit end of tape
		return NULL_TAPE_POS;
	} else {
		for (int i = (sizeof(tape_holes) / sizeof(tape_holes[ 0 ])) - 1; i >= 0; i--) {
			if (tape_holes[ i ] < pos) {
				return tape_holes[ i ];
			}
		}
		// No more holes: will hit start of tape
		return NULL_TAPE_POS;
	}
}

void hti_format_t::write_word(unsigned track_no , tape_pos_t start , tape_word_t word , tape_pos_t& length , bool forward)
{
	tape_track_t& track = m_tracks[ track_no ];
	track_iterator_t it_low = track.lower_bound(start);
	adjust_it(track , it_low , start);
	length = word_length(word);
	tape_pos_t end_pos = start + length;
	track_iterator_t it_high = track.lower_bound(end_pos);

	track.erase(it_low , it_high);

	// A 0 word is inserted after the word being written, if space allows.
	// This is meant to avoid fragmentation of the slack space at the end of a record
	// as the record expands & contracts when re-written with different content.
	// Without this fix, a gap could form in the slack big enough to cause
	// false gap detections.
	if (forward && it_high != track.end() && (it_high->first - end_pos) >= (ZERO_BIT_LEN * 16 + ONE_BIT_LEN)) {
		track.insert(it_high, std::make_pair(end_pos, 0));
		it_high--;
	}

	track.insert(it_high , std::make_pair(start, word));
}

void hti_format_t::write_gap(unsigned track_no , tape_pos_t a , tape_pos_t b)
{
	ensure_a_lt_b(a , b);
	tape_track_t& track = m_tracks[ track_no ];
	track_iterator_t it_low = track.lower_bound(a);
	adjust_it(track , it_low , a);
	track_iterator_t it_high = track.lower_bound(b);

	track.erase(it_low, it_high);
}

bool hti_format_t::just_gap(unsigned track_no , tape_pos_t a , tape_pos_t b)
{
	ensure_a_lt_b(a , b);
	tape_track_t& track = m_tracks[ track_no ];
	track_iterator_t it_low = track.lower_bound(a);
	track_iterator_t it_high = track.lower_bound(b);

	adjust_it(track, it_low, a);

	return it_low == it_high;
}

bool hti_format_t::next_data(unsigned track_no , tape_pos_t pos , bool forward , bool inclusive , track_iterator_t& it)
{
	tape_track_t& track = m_tracks[ track_no ];
	it = track.lower_bound(pos);
	if (forward) {
		if (inclusive) {
			adjust_it(track, it, pos);
		}
		return it != track.end();
	} else {
		// Never more than 2 iterations
		do {
			if (it == track.begin()) {
				it = track.end();
				return false;
			}
			--it;
		} while (!inclusive && word_end_pos(it) > pos);
		return true;
	}
}

hti_format_t::adv_res_t hti_format_t::adv_it(unsigned track_no , bool forward , track_iterator_t& it)
{
	tape_track_t& track = m_tracks[ track_no ];
	if (forward) {
		tape_pos_t prev_pos = word_end_pos(it);
		++it;
		if (it == track.end()) {
			return ADV_NO_MORE_DATA;
		} else {
			adv_res_t res = prev_pos == it->first ? ADV_CONT_DATA : ADV_DISCONT_DATA;
			return res;
		}
	} else {
		if (it == track.begin()) {
			it = track.end();
			return ADV_NO_MORE_DATA;
		} else {
			tape_pos_t prev_pos = it->first;
			--it;
			return prev_pos == word_end_pos(it) ? ADV_CONT_DATA : ADV_DISCONT_DATA;
		}
	}
}

bool hti_format_t::next_gap(unsigned track_no , tape_pos_t& pos , bool forward , tape_pos_t min_gap)
{
	tape_track_t::iterator it;
	// First align with next data
	next_data(track_no , pos , forward , true , it);
	// Then scan for 1st gap
	tape_track_t& track = m_tracks[ track_no ];
	bool done = false;
	track_iterator_t prev_it;
	unsigned n_gaps = 1;

	if (forward) {
		tape_pos_t next_pos;

		while (1) {
			if (it == track.end()) {
				next_pos = TAPE_LENGTH;
				done = true;
			} else {
				next_pos = it->first;
			}
			if (((next_pos - pos) >= min_gap && --n_gaps == 0) || done) {
				break;
			}
			adv_res_t adv_res;
			do {
				prev_it = it;
				adv_res = adv_it(track_no , forward , it);
			} while (adv_res == ADV_CONT_DATA);
			pos = word_end_pos(prev_it);
		}
	} else {
		tape_pos_t next_pos;

		while (1) {
			if (it == track.end()) {
				next_pos = 0;
				done = true;
			} else {
				next_pos = word_end_pos(it);
			}
			if (((pos - next_pos) >= min_gap && --n_gaps == 0) || done) {
				break;
			}
			adv_res_t adv_res;
			do {
				prev_it = it;
				adv_res = adv_it(track_no , forward , it);
			} while (adv_res == ADV_CONT_DATA);
			pos = prev_it->first;
		}
	}

	// Set "pos" where minimum gap size is met
	pos_offset(pos , forward , min_gap);

	return n_gaps == 0;
}

bool hti_format_t::load_track(io_generic *io , uint64_t& offset , tape_track_t& track)
{
	uint8_t tmp[ 4 ];
	uint32_t tmp32;

	track.clear();

	while (1) {
		// Read no. of words to follow
		io_generic_read(io, tmp, offset, 4);
		offset += 4;

		tmp32 = pick_integer_le(tmp, 0, 4);

		// Track ends
		if (tmp32 == (uint32_t)-1) {
			return true;
		}

		unsigned n_words = tmp32;

		// Read tape position of block
		io_generic_read(io, tmp, offset, 4);
		offset += 4;

		tmp32 = pick_integer_le(tmp, 0, 4);

		tape_pos_t pos = (tape_pos_t)tmp32;

		for (unsigned i = 0; i < n_words; i++) {
			uint16_t tmp16;

			io_generic_read(io, tmp, offset, 2);
			offset += 2;
			tmp16 = pick_integer_le(tmp, 0, 2);

			track.insert(std::make_pair(pos , tmp16));
			pos += word_length(tmp16);
		}
	}
}

void hti_format_t::dump_sequence(io_generic *io , uint64_t& offset , tape_track_t::const_iterator it_start , unsigned n_words)
{
	if (n_words) {
		uint8_t tmp[ 8 ];
		place_integer_le(tmp, 0, 4, n_words);
		place_integer_le(tmp, 4, 4, it_start->first);
		io_generic_write(io, tmp, offset, 8);
		offset += 8;

		for (unsigned i = 0; i < n_words; i++) {
			place_integer_le(tmp, 0, 2, it_start->second);
			io_generic_write(io, tmp, offset, 2);
			offset += 2;
			++it_start;
		}
	}
}

hti_format_t::tape_pos_t hti_format_t::word_end_pos(const track_iterator_t& it)
{
	return it->first + word_length(it->second);
}

void hti_format_t::adjust_it(tape_track_t& track , track_iterator_t& it , tape_pos_t pos)
{
	if (it != track.begin()) {
		--it;
		if (word_end_pos(it) <= pos) {
			++it;
		}
	}
}

void hti_format_t::ensure_a_lt_b(tape_pos_t& a , tape_pos_t& b)
{
	if (a > b) {
		// Ensure A always comes before B
		tape_pos_t tmp;
		tmp = a;
		a = b;
		b = tmp;
	}
}
