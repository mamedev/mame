// license:BSD-3-Clause
// copyright-holders:F. Ulivi
/*********************************************************************

    "HTI" format

*********************************************************************/

#include "hti_tape.h"

#include "ioprocs.h"
#include "multibyte.h"

#include <tuple>


static constexpr uint32_t OLD_FILE_MAGIC = 0x5441434f;  // Magic value at start of old-format image file: "TACO"
static constexpr uint32_t FILE_MAGIC_DELTA = 0x48544930;    // Magic value at start of delta-modulation image file: "HTI0"
static constexpr uint32_t FILE_MAGIC_MANCHESTER = 0x48544931;   // Magic value at start of manchester-modulation image file: "HTI1"

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
	: m_img_format(HTI_DELTA_MOD_16_BITS)
{
	clear_tape();
}

bool hti_format_t::load_tape(util::random_read &io)
{
	if (io.seek(0, SEEK_SET)) {
		return false;
	}

	uint8_t tmp[4];
	auto const [err, actual] = read(io, tmp, 4);
	if (err || (4 != actual)) {
		return false;
	}

	auto magic = get_u32be(tmp);
	if (((m_img_format == HTI_DELTA_MOD_16_BITS || m_img_format == HTI_DELTA_MOD_17_BITS) && magic != FILE_MAGIC_DELTA && magic != OLD_FILE_MAGIC) ||
		(m_img_format == HTI_MANCHESTER_MOD && magic != FILE_MAGIC_MANCHESTER)) {
		return false;
	}

	for (unsigned i = 0; i < no_of_tracks(); i++) {
		tape_track_t &track = m_tracks[i];
		if (!load_track(io, track, magic == OLD_FILE_MAGIC)) {
			clear_tape();
			return false;
		}
	}

	return true;
}

void hti_format_t::save_tape(util::random_read_write &io)
{
	io.seek(0, SEEK_SET); // FIXME: check for errors

	uint8_t tmp[4];

	put_u32be(tmp, m_img_format == HTI_MANCHESTER_MOD ? FILE_MAGIC_MANCHESTER : FILE_MAGIC_DELTA);
	write(io, tmp, 4); // FIXME: check for errors

	for (unsigned i = 0; i < no_of_tracks(); i++) {
		const tape_track_t &track = m_tracks[i];
		tape_pos_t next_pos = tape_pos_t(-1);
		unsigned n_words = 0;
		tape_track_t::const_iterator it_start;
		for (tape_track_t::const_iterator it = track.cbegin(); it != track.cend(); ++it) {
			if (it->first != next_pos) {
				dump_sequence(io, it_start, n_words);
				it_start = it;
				n_words = 0;
			}
			next_pos = it->first + word_length(it->second);
			n_words++;
		}
		dump_sequence(io, it_start, n_words);
		// End of track
		put_u32le(tmp, (uint32_t)-1);
		write(io, tmp, 4); // FIXME: check for errors
	}
}

void hti_format_t::clear_tape()
{
	for (tape_track_t &track : m_tracks) {
		track.clear();
	}
}

hti_format_t::tape_pos_t hti_format_t::word_length(tape_word_t w) const
{
	unsigned zeros, ones;

	// pop count of w
	ones = (w & 0x5555) + ((w >> 1) & 0x5555);
	ones = (ones & 0x3333) + ((ones >> 2) & 0x3333);
	ones = (ones & 0x0f0f) + ((ones >> 4) & 0x0f0f);
	ones = (ones & 0x00ff) + ((ones >> 8) & 0x00ff);

	zeros = 16 - ones;

	return zeros * bit_length(false) + ones * bit_length(true);
}

hti_format_t::tape_pos_t hti_format_t::farthest_end(const track_iterator_t &it, bool forward) const
{
	if (forward) {
		return word_end_pos(it);
	} else {
		return it->first;
	}
}

bool hti_format_t::pos_offset(tape_pos_t &pos, bool forward, tape_pos_t offset)
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

hti_format_t::tape_pos_t hti_format_t::next_hole(tape_pos_t pos, bool forward)
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
		for (int i = (sizeof(tape_holes) / sizeof(tape_holes[0])) - 1; i >= 0; i--) {
			if (tape_holes[i] < pos) {
				return tape_holes[i];
			}
		}
		// No more holes: will hit start of tape
		return NULL_TAPE_POS;
	}
}

void hti_format_t::write_word(unsigned track_no, tape_pos_t start, tape_word_t word, tape_pos_t &length, bool forward)
{
	tape_track_t &track = m_tracks[track_no];
	track_iterator_t it_low = track.lower_bound(start);
	adjust_it(track, it_low, start);
	length = word_length(word);
	tape_pos_t end_pos = start + length;
	track_iterator_t it_high = track.lower_bound(end_pos);

	track.erase(it_low, it_high);

	// A 0 word is inserted after the word being written, if space allows.
	// This is meant to avoid fragmentation of the slack space at the end of a record
	// as the record expands & contracts when re-written with different content.
	// Without this fix, a gap could form in the slack big enough to cause
	// false gap detections.
	if (forward && it_high != track.end() && (it_high->first - end_pos) >= (bit_length(false) * 16)) {
		track.insert(it_high, std::make_pair(end_pos, 0));
		it_high--;
	}

	track.insert(it_high, std::make_pair(start, word));
}

void hti_format_t::write_gap(unsigned track_no, tape_pos_t a, tape_pos_t b)
{
	ensure_a_lt_b(a, b);
	tape_track_t &track = m_tracks[track_no];
	track_iterator_t it_low = track.lower_bound(a);
	adjust_it(track, it_low, a);
	track_iterator_t it_high = track.lower_bound(b);

	track.erase(it_low, it_high);
}

bool hti_format_t::just_gap(unsigned track_no, tape_pos_t a, tape_pos_t b)
{
	ensure_a_lt_b(a, b);
	tape_track_t &track = m_tracks[track_no];
	track_iterator_t it_low = track.lower_bound(a);
	track_iterator_t it_high = track.lower_bound(b);

	adjust_it(track, it_low, a);

	return it_low == it_high;
}

bool hti_format_t::next_data(unsigned track_no, tape_pos_t pos, bool forward, bool inclusive, track_iterator_t &it)
{
	tape_track_t &track = m_tracks[track_no];
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

hti_format_t::adv_res_t hti_format_t::adv_it(unsigned track_no, bool forward, track_iterator_t &it)
{
	tape_track_t &track = m_tracks[track_no];
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

bool hti_format_t::sync_with_record(unsigned track_no, track_iterator_t &it, unsigned &bit_idx)
{
	while ((it->second & (1U << bit_idx)) == 0) {
		if (bit_idx) {
			bit_idx--;
		} else {
			bit_idx = 15;
			auto res = adv_it(track_no, true, it);
			if (res != ADV_CONT_DATA) {
				return false;
			}
		}
	}
	if (bit_idx) {
		bit_idx--;
	} else {
		bit_idx = 15;
	}
	return true;
}

hti_format_t::adv_res_t hti_format_t::next_word(unsigned track_no, track_iterator_t &it, unsigned &bit_idx, tape_word_t &word)
{
	if (bit_idx == 15) {
		auto res = adv_it(track_no, true, it);
		if (res == ADV_NO_MORE_DATA) {
			return res;
		}
		word = it->second;
		return res;
	} else {
		word = it->second << (15 - bit_idx);
		auto res = adv_it(track_no, true, it);
		if (res == ADV_DISCONT_DATA) {
			bit_idx = 15;
			it--;
		} else if (res == ADV_CONT_DATA) {
			word |= (it->second >> (bit_idx + 1));
		}

		return res;
	}
}

bool hti_format_t::next_gap(unsigned track_no, tape_pos_t &pos, bool forward, tape_pos_t min_gap)
{
	tape_track_t::iterator it;
	// First align with next data
	next_data(track_no, pos, forward, true, it);
	// Then scan for 1st gap
	tape_track_t &track = m_tracks[track_no];
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
				adv_res = adv_it(track_no, forward, it);
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
				adv_res = adv_it(track_no, forward, it);
			} while (adv_res == ADV_CONT_DATA);
			pos = prev_it->first;
		}
	}

	// Set "pos" where minimum gap size is met
	pos_offset(pos, forward, min_gap);

	return n_gaps == 0;
}

bool hti_format_t::load_track(util::random_read &io, tape_track_t &track, bool old_format)
{
	tape_pos_t delta_pos = 0;
	tape_pos_t last_word_end = 0;

	track.clear();

	while (1) {
		std::error_condition err;
		size_t actual;
		uint8_t tmp[4];
		uint32_t tmp32;

		// Read no. of words to follow
		std::tie(err, actual) = read(io, tmp, 4);
		if (err || (4 != actual)) {
			return false;
		}
		tmp32 = get_u32le(tmp);

		// Track ends
		if (tmp32 == (uint32_t)-1) {
			return true;
		}

		unsigned n_words = tmp32;

		// Read tape position of block
		std::tie(err, actual) = read(io, tmp, 4);
		if (err || (4 != actual)) {
			return false;
		}
		tmp32 = get_u32le(tmp);

		tape_pos_t pos = (tape_pos_t)tmp32 + delta_pos;

		tape_word_t word_accum = 0;
		unsigned bits_in_accum = 0;

		for (unsigned i = 0; i < n_words; i++) {
			uint16_t tmp16;

			std::tie(err, actual) = read(io, tmp, 2);
			if (err || (2 != actual)) {
				return false;
			}
			tmp16 = get_u16le(tmp);

			if (!old_format) {
				track.insert(std::make_pair(pos, tmp16));
				pos += word_length(tmp16);
			} else if (m_img_format == HTI_DELTA_MOD_16_BITS) {
				// Convert HP9845 & HP85 old format
				// Basically, in old format each word had 17 bits (an implicit 1
				// was added at the end). In new format we just keep the 16 bits
				// and don't add the 17th bit.
				if (i == 0 && tmp16 == 0 && (pos - last_word_end) > 16384) {
					// This mysterious heuristic is meant to turn the first
					// word of a record into a proper preamble word (from 0 to 1)
					// provided this is actually at the beginning of a new record
					// (enough distance from end of last record)
					tmp16 = 1;
				}
				track.insert(std::make_pair(pos, tmp16));
				pos += word_length(tmp16);
				last_word_end = pos;
				delta_pos -= DELTA_ONE_BIT_LEN;
			} else {
				// Convert HP9825 old format
				// In moving from old to new format we make the 17th bit at the
				// end of each word explicit
				word_accum |= (tmp16 >> bits_in_accum);
				// Avoid storing overlapping words
				if (pos >= last_word_end) {
					track.insert(std::make_pair(pos, word_accum));
				}
				pos += word_length(word_accum);
				last_word_end = pos;
				if (bits_in_accum == 0) {
					word_accum = 0;
				} else {
					word_accum = tmp16 << (16 - bits_in_accum);
				}
				word_accum |= (1U << (15 - bits_in_accum));
				if (++bits_in_accum >= 16) {
					track.insert(std::make_pair(pos, word_accum));
					pos += word_length(word_accum);
					last_word_end = pos;
					word_accum = 0;
					bits_in_accum = 0;
				}
			}
		}
		if (bits_in_accum) {
			track.insert(std::make_pair(pos, word_accum));
			tape_pos_t shift = (tape_pos_t)(16 - bits_in_accum) * DELTA_ZERO_BIT_LEN;
			delta_pos += shift;
			last_word_end = pos + word_length(word_accum);
		}
	}
}

void hti_format_t::dump_sequence(util::random_read_write &io, tape_track_t::const_iterator it_start, unsigned n_words)
{
	if (n_words) {
		uint8_t tmp[8];
		put_u32le(&tmp[0], n_words);
		put_u32le(&tmp[4], it_start->first);
		write(io, tmp, 8); // FIXME: check for errors

		for (unsigned i = 0; i < n_words; i++) {
			put_u16le(tmp, it_start->second);
			write(io, tmp, 2); // FIXME: check for errors
			++it_start;
		}
	}
}

hti_format_t::tape_pos_t hti_format_t::word_end_pos(const track_iterator_t &it) const
{
	return it->first + word_length(it->second);
}

void hti_format_t::adjust_it(tape_track_t &track, track_iterator_t &it, tape_pos_t pos) const
{
	if (it != track.begin()) {
		--it;
		if (word_end_pos(it) <= pos) {
			++it;
		}
	}
}

void hti_format_t::ensure_a_lt_b(tape_pos_t &a, tape_pos_t &b)
{
	if (a > b) {
		// Ensure A always comes before B
		tape_pos_t tmp;
		tmp = a;
		a = b;
		b = tmp;
	}
}
