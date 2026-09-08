// license:BSD-3-Clause
// copyright-holders: Tomás García-Merás (ClawGrip)

/***************************************************************************

	UMC UM348x multi-instrument melody generator family

	Every constant here was measured against logic-level captures of real
	parts; the datasheet gives none of them. Full notes, including the evidence
	behind each one and the questions still open, are in:

		https://github.com/clawgrip/UM348xDecoder

	Based on previous work from:
	  - Sean Riddle: https://www.seanriddle.com/um348x/
	  - ArcadeHacker: https://arcadehacker.blogspot.com/2020/07/um3481a-series-multi-instrument-melody.html

	Song pointers
	-------------
	Both parts have 16 pointer slots. All 16 are reachable here, and a song is
	silent only when its pointer addresses the ROM's trailing filler, which is
	a test on the data rather than on the datasheet's song count.

	Note ROM layout
	---------------
	448 bytes = 3584 bits = 64 rows of 56 columns, i.e. 7 column-groups of 8
	sub-columns. Row r contributes bit s of each group byte to the word of
	sub-column s. Melodies do not run through the sub-columns in the order
	0..7 but in the order 0,1,2,3,7,6,5,4, the second half of the array being
	traversed in reverse, so

		noteIndex = position_in_SUBCOLUMN_ORDER * 64 + row      (0..511)

	Each note is a 7-bit word: bits 6-4 a duration code, bits 3-0 a tone code.
	Tone code 3 is a rest. Tone code 1 is a silent control word which still
	consumes its word's time. The remaining 14 codes select an oscillator
	divisor.

	Timing
	------
	One base unit is 1024 oscillator cycles. A word lasts

		ticks(duration code) * multiplier   base units

	except the first word of a song, which always lasts exactly 16 base units
	regardless of its duration code or the song's multiplier.

	The multiplier is not in any dumped ROM. The measured values take five
	distinct values across the two parts, matching the datasheet's count of
	five mask tempos.

	What is NOT emulated
	--------------------
	- Songs whose multiplier could not be measured fall back to the most common
	  value. Only three of the UM3482A's could be measured.
	- One melody per part is rendered by the real chip in a staccato
	  articulation: the output is re-struck once per tick, sounding for a
	  fixed 1024 oscillator cycles at the head of each tick. Nothing in the
	  note words marks which melody uses it, so it is not reproduced; the
	  melody plays as sustained tones, correct in pitch and total length but
	  not in texture.
	- The ROM dumps come from visual decapping. They are corroborated where the
	  captures reach and unverified elsewhere.

***************************************************************************/

#include "emu.h"
#include "um348x.h"


//**************************************************************************
//  CONSTANTS AND NOTE ROM DECODING
//**************************************************************************

namespace {

constexpr u16 TOTAL_NOTES = 512;
constexpr u8  ROM_ROWS    = 64;
constexpr u8  ROM_GROUPS  = 7;
constexpr u8  REST_TONE   = 3;
constexpr u8  CTRL_TONE   = 1;

constexpr u8 SUBCOLUMN_ORDER[8] = { 0, 1, 2, 3, 7, 6, 5, 4 };

constexpr u8  MELODY_SLOTS = 16;

constexpr u16 BASE_UNIT_CYCLES = 1024;

constexpr u8 FIRST_REST_BASE_UNITS = 16;

// Duration code -> ticks. All but codes 5 and 7 are counted directly, either
// in the staccato song, where the part re-articulates once per tick, or in the
// UM3482A song 9 alignment. Codes 5 and 7 come from duration ratios.
constexpr u8 DURATION_TICKS[8] = { 2, 3, 15, 4, 1, 8, 12, 6 };

constexpr u8 DEFAULT_MULTIPLIER = 10;

// Measured from real playback; 0 where no measurement was possible.
constexpr u8 UM3481A_MULTIPLIERS[16] = { 10, 8, 12, 6, 8, 10, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0 };
constexpr u8 UM3482A_MULTIPLIERS[16] = { 0, 0, 0, 0, 0, 0, 5, 0, 10, 0, 6, 0, 0, 0, 0, 0 };

/*  The tone ROM holds one preload value per tone code. Each code, with its four
	bits reversed, addresses a seven-bit value; loading the tone counter with it
	and clocking the shift register below until it reaches 0x02 gives the
	oscillator half-period N, and the tone is clock / (2N). The stored value is
	not the divisor: the counter is a shift register, so the divisor is the
	number of clocks it takes, not the preload.

	This reproduces every measured divisor on both parts. */
u8 tone_divisor(u8 seed)
{
	u8 state = seed;
	for (u8 n = 1; n < 128; n++)
	{
		state = ((state << 1) & 0x7f) | (BIT(state, 6) ^ BIT(state, 5));
		if (state == 0x02)
			return n;
	}
	return 0;
}


u8 decode_word(const u8 *notes, u16 index)
{
	const u8 subcol = SUBCOLUMN_ORDER[(index / ROM_ROWS) & 7];
	const u8 row    = index % ROM_ROWS;

	u8 word = 0;
	for (int g = 0; g < ROM_GROUPS; g++)
		word = (word << 1) | BIT(notes[row * ROM_GROUPS + g], subcol);

	return word & 0x7f;
}


// The two dumps pad their offset tables differently: the UM3481A's is 16
// entries of 12 packed bits, the UM3482A's 16 big-endian 16-bit words.
u16 melody_offset(const u8 *offsets, size_t length, u8 index)
{
	if (length == 24)
	{
		const u16 bit = index * 12;
		u16 v = 0;
		for (int b = 0; b < 12; b++)
		{
			const u16 p = bit + b;
			v = (v << 1) | BIT(offsets[p >> 3], 7 - (p & 7));
		}
		return v;
	}

	return (offsets[index * 2] << 8) | offsets[index * 2 + 1];
}

} // anonymous namespace



DEFINE_DEVICE_TYPE(UM3481A, um3481a_device, "um3481a", "UM3481A Melody Generator")
DEFINE_DEVICE_TYPE(UM3482A, um3482a_device, "um3482a", "UM3482A Melody Generator")


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// All from visual decaps, hence the BAD_DUMP

ROM_START( um3481a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3481a_main.bin",    0x000, 0x1c0, BAD_DUMP CRC(8eef34d8) SHA1(b400e737ec8e7d694d629457d8909e8320715fe5) )

	ROM_REGION( 0x018, "offsets", 0 ) // 16 entries of 12 packed bits
	ROM_LOAD( "um3481a_offsets.bin", 0x000, 0x018, BAD_DUMP CRC(66b16105) SHA1(c74b6da95318909408ddfab42cf21d3493d2b821) )

	ROM_REGION( 0x010, "tones", 0 ) // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3481a_tones.bin",   0x000, 0x010, BAD_DUMP CRC(646cdaef) SHA1(48d45db842e2dd588b58ba6aa656c6496e514d23) )

	ROM_REGION( 0x010, "unknown", 0 ) // 8 entries of 7 bits, padded to bytes, same as UM3482A
	ROM_LOAD( "unknown.bin",         0x000, 0x008, BAD_DUMP CRC(87a9efc4) SHA1(54ec7dea890dea8fd2aac85d7bee6db3c71d5db9) )
ROM_END

ROM_START( um3482a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3482a_main.bin",    0x000, 0x1c0, BAD_DUMP CRC(5871d564) SHA1(4203b6513ad08ece26177778e5defeb862d1a81d) )

	ROM_REGION( 0x020, "offsets", 0 ) // 16 entries of 9 bits, padded to 16
	ROM_LOAD( "um3482a_offsets.bin", 0x000, 0x020, BAD_DUMP CRC(f39aff3c) SHA1(255dcea154ed04c6d1968b09e188ca5fc8821721) )

	ROM_REGION( 0x010, "tones", 0 ) // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3482a_tones.bin",   0x000, 0x010, BAD_DUMP CRC(c3a37f74) SHA1(67eac8c6530c202760d492f3e52c44f9cd183b46) )

	ROM_REGION( 0x010, "unknown", 0 ) // 8 entries of 7 bits, padded to bytes, same as UM3482A
	ROM_LOAD( "unknown.bin",         0x000, 0x008, BAD_DUMP CRC(87a9efc4) SHA1(54ec7dea890dea8fd2aac85d7bee6db3c71d5db9) )
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

um348x_device::um348x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const u8 *multipliers) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_notes(*this, "notes"),
	m_offsets(*this, "offsets"),
	m_tones(*this, "tones"),
	m_stream(nullptr),
	m_multipliers(multipliers),
	m_divisors{ 0 },
	m_data_end(0),
	m_ce(0),
	m_lp(0),
	m_sl(0),
	m_as(0),
	m_song(0),
	m_playing(false),
	m_note_index(0),
	m_note_start(0),
	m_note_end(0),
	m_multiplier(DEFAULT_MULTIPLIER),
	m_word_cycles(0),
	m_divisor(0),
	m_div_count(0),
	m_out(1)
{
}

um3481a_device::um3481a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	um348x_device(mconfig, UM3481A, tag, owner, clock, UM3481A_MULTIPLIERS)
{
}

um3482a_device::um3482a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	um348x_device(mconfig, UM3482A, tag, owner, clock, UM3482A_MULTIPLIERS)
{
}

const tiny_rom_entry *um3481a_device::device_rom_region() const { return ROM_NAME(um3481a); }
const tiny_rom_entry *um3482a_device::device_rom_region() const { return ROM_NAME(um3482a); }


void um348x_device::decode_tone_rom()
{
	const u8 *const rom = m_tones->base();

	for (u8 code = 0; code < 16; code++)
	{
		if (code == REST_TONE || code == CTRL_TONE)
		{
			m_divisors[code] = 0;
			continue;
		}

		u8 address = 0;
		for (int b = 0; b < 4; b++)
			address = (address << 1) | BIT(code, b);

		m_divisors[code] = tone_divisor(rom[address] & 0x7f);
	}
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void um348x_device::device_start()
{
	if (m_notes->bytes() != ROM_ROWS * ROM_GROUPS)
		fatalerror("%s: note ROM must be %d bytes, got %d\n", tag(), ROM_ROWS * ROM_GROUPS, int(m_notes->bytes()));

	const size_t olen = m_offsets->bytes();
	if (olen != 24 && olen != 32)
		fatalerror("%s: offsets ROM must be 24 bytes (12-bit packed) or 32 bytes (16-bit), got %d\n", tag(), int(olen));

	if (m_tones->bytes() != MELODY_SLOTS)
		fatalerror("%s: tone ROM must be %d bytes, got %d\n", tag(), MELODY_SLOTS, int(m_tones->bytes()));

	decode_tone_rom();

	// Everything after the last sounding word is filler; songs are clamped to
	// it so a stray pointer cannot play minutes of rests.
	m_data_end = 0;
	for (u16 i = 0; i < TOTAL_NOTES; i++)
	{
		const u8 tone = decode_word(m_notes->base(), i) & 0x0f;
		if (tone != REST_TONE && tone != CTRL_TONE)
			m_data_end = i;
	}

	u16 unknown = 0;
	for (u16 i = 0; i <= m_data_end; i++)
	{
		const u8 tone = decode_word(m_notes->base(), i) & 0x0f;
		if (tone != REST_TONE && tone != CTRL_TONE && !m_divisors[tone])
			unknown |= 1 << tone;
	}
	if (unknown)
		logerror("tone codes with no divisor in the table: %04x; those notes will be silent\n", unknown);

	// One sample per oscillator cycle, so the emulated waveform lines up
	// cycle for cycle with a logic capture of the real part.
	m_stream = stream_alloc(0, 1, clock());

	save_item(NAME(m_divisors));
	save_item(NAME(m_ce));
	save_item(NAME(m_lp));
	save_item(NAME(m_sl));
	save_item(NAME(m_as));
	save_item(NAME(m_song));
	save_item(NAME(m_playing));
	save_item(NAME(m_note_index));
	save_item(NAME(m_note_start));
	save_item(NAME(m_note_end));
	save_item(NAME(m_multiplier));
	save_item(NAME(m_word_cycles));
	save_item(NAME(m_divisor));
	save_item(NAME(m_div_count));
	save_item(NAME(m_out));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void um348x_device::device_reset()
{
	stop();
}


//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void um348x_device::device_clock_changed()
{
	if (clock() == 0)
		return;

	m_stream->update();
	m_stream->set_sample_rate(clock());
}


void um348x_device::stop()
{
	m_playing = false;
	m_divisor = 0;
	m_div_count = 0;
	m_word_cycles = 0;
	m_out = 1;
}


//**************************************************************************
//  PLAYBACK
//**************************************************************************

void um348x_device::ce_w(int state)
{
	state = state ? 1 : 0;

	if (state != m_ce)
	{
		m_stream->update();

		if (state)
			start_song();
		else
			stop();
	}

	m_ce = state;
}


void um348x_device::lp_w(int state)
{
	m_lp = state ? 1 : 0;
}


void um348x_device::sl_w(int state)
{
	state = state ? 1 : 0;

	if (state && !m_sl)
	{
		m_stream->update();
		next_song();

		if (m_ce)
			start_song();
	}

	m_sl = state;
}


void um348x_device::as_w(int state)
{
	m_as = state ? 1 : 0;
}


u16 um348x_device::melody_start(u8 melody) const
{
	return melody_offset(m_offsets->base(), m_offsets->bytes(), melody);
}


// A song whose pointer addresses the trailing filler does not exist on this
// part, so nothing sounds.
void um348x_device::start_song()
{
	const u16 start = melody_start(m_song);
	u16 end = (m_song + 1 < MELODY_SLOTS) ? melody_start(m_song + 1) : TOTAL_NOTES;

	if (end <= start || end > TOTAL_NOTES)
		end = TOTAL_NOTES;
	if (end > u16(m_data_end + 1))
		end = m_data_end + 1;

	if (start > m_data_end)
	{
		m_playing = false;
		m_divisor = 0;
		return;
	}

	m_multiplier = m_multipliers[m_song] ? m_multipliers[m_song] : DEFAULT_MULTIPLIER;

	m_note_start = start;
	m_note_end = end;
	m_playing = true;
	m_out = 1;
	start_word(start);
}


void um348x_device::next_song()
{
	m_song = (m_song + 1) & (MELODY_SLOTS - 1);
}


void um348x_device::start_word(u16 index)
{
	const u8 word = decode_word(m_notes->base(), index);
	const u8 duration = (word >> 4) & 0x07;
	const u8 tone     = word & 0x0f;

	m_note_index = index;

	const bool opening_rest = (index == m_note_start) && (tone == REST_TONE);
	const u32 units = opening_rest
			? FIRST_REST_BASE_UNITS : u32(DURATION_TICKS[duration]) * m_multiplier;

	m_word_cycles = units * BASE_UNIT_CYCLES;

	if (tone == REST_TONE || tone == CTRL_TONE)
	{
		m_divisor = 0; // silent, but the word still takes its time
	}
	else
	{
		/*  The divider free-runs across word boundaries: a new value only takes
			effect when the counter next expires. A note spread over several
			words therefore sounds continuous, and a change of tone completes
			the half cycle already in progress before adopting the new divisor,
			which is what the logic captures show. The counter is only loaded
			when starting a note out of silence. */
		const u8 div = m_divisors[tone];
		if (div && (!m_divisor || !m_div_count))
			m_div_count = div;
		m_divisor = div;
	}
}


void um348x_device::advance_word()
{
	const u16 next = m_note_index + 1;

	if (next < m_note_end && next < TOTAL_NOTES)
	{
		start_word(next);
		return;
	}

	if (!m_lp)
	{
		next_song();

		if (m_song != 0 || m_as)
		{
			start_song();
			return;
		}
	}
	else if (m_as)
	{
		start_song();
		return;
	}

	m_playing = false;
	m_divisor = 0;
}


//-------------------------------------------------
//  sound_stream_update
//-------------------------------------------------

void um348x_device::sound_stream_update(sound_stream &stream)
{
	if (!m_playing)
		return;

	for (int sampindex = 0; sampindex < stream.samples() && m_playing; sampindex++)
	{
		if (m_divisor)
		{
			if (--m_div_count == 0)
			{
				m_div_count = m_divisor;
				m_out = -m_out;
			}

			stream.put(0, sampindex, sound_stream::sample_t(m_out) * 0.5);
		}

		if (--m_word_cycles == 0)
			advance_word();
	}
}
