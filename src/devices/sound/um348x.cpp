// license:BSD-3-Clause
// copyright-holders: Tomás García-Merás (ClawGrip)

/***************************************************************************

    UMC UM348x multi-instrument melody generator family

    Every constant here was measured against logic-level captures of real
    parts; the datasheet gives none of them. The evidence behind each one
    and the questions still open are in:

        https://github.com/clawgrip/UM348xDecoder

    Based on previous work from:
      - Sean Riddle: https://www.seanriddle.com/um348x/
      - ArcadeHacker: https://arcadehacker.blogspot.com/2020/07/um3481a-series-multi-instrument-melody.html

    Song pointers
    -------------
    Both parts have 16 pointer slots, all of them reachable with SL. A song
    runs from its pointer up to the next slot's.

    Note ROM layout
    ---------------
    448 bytes = 3584 bits = 64 rows of 56 columns, i.e. 7 column-groups of 8
    sub-columns. Row r contributes bit s of each group byte to the word of
    sub-column s. Melodies do not run through the sub-columns in the order
    0..7 but in the order 0,1,2,3,7,6,5,4, the second half of the array being
    traversed in reverse, so

        noteIndex = position_in_SUBCOLUMN_ORDER * 64 + row      (0..511)

    Each note is a 7-bit word: bits 6-4 a duration code, bits 3-0 a tone code.
    Tone code 3 is a rest. Tone code 1 is a silent control word carrying a
    timbre in its upper field. The remaining 14 codes select an oscillator
    divisor.

    Timing
    ------
    One base unit is 1024 oscillator cycles. A word lasts

        ticks(duration code) * multiplier   base units

    except a rest opening a song, which always lasts exactly 16 base units
    regardless of its duration code or the song's multiplier.

    The multiplier is not in any dumped ROM. The measured values take five
    distinct values across the two parts, matching the datasheet's count of
    five mask tempos.

    The 8 x 7 area
    --------------
    Identical on both parts, so fixed family logic rather than song data.
    Entries 1 to 7 are one-hot and cover the seven bit positions exactly once;
    entry 0 is the OR of entries 4 and 5. Its role is unknown and nothing
    reads it.

    What is NOT emulated
    --------------------
    - Songs whose multiplier could not be measured fall back to the most common
      value. Only three of the UM3482A's could be measured.
    - Mandolin articulation. The real part re-strikes the output once per tick,
      sounding for about 1024 oscillator cycles at the head of each tick. Here
      those songs play as sustained tones, right in pitch and total length.
    - The ROM dumps come from visual decapping. They are corroborated where the
      captures reach and unverified elsewhere.

***************************************************************************/

#include "emu.h"
#include "um348x.h"

#include "multibyte.h"


//**************************************************************************
//  CONSTANTS AND NOTE ROM DECODING
//**************************************************************************

namespace {

constexpr u16 TOTAL_NOTES = 512;
constexpr u8  ROM_ROWS    = 64;
constexpr u8  ROM_GROUPS  = 7;
constexpr u8  REST_TONE   = 3;
constexpr u8  CTRL_TONE   = 1;

/*  A control word carries a timbre in its upper field instead of a duration.
    The mandolin selector costs no time; the other two timbres take the
    duration their field would give a note. */
constexpr u8  TIMBRE_MANDOLIN = 4;

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

/*  The tone ROM holds one preload value per tone code, addressed by the code
    with its four bits reversed. The tone counter is a seven-bit shift register
    loaded with that value and clocked until it reaches 0x02; the number of
    clocks is the oscillator half-period N, and the tone is clock / (2N). */
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

} // anonymous namespace



DEFINE_DEVICE_TYPE(UM3481A, um3481a_device, "um3481a", "UMC UM3481A melody generator")
DEFINE_DEVICE_TYPE(UM3482A, um3482a_device, "um3482a", "UMC UM3482A melody generator")


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// All from visual decaps, hence the BAD_DUMP

ROM_START( um3481a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3481a_main.bin",    0x000, 0x1c0, BAD_DUMP CRC(8eef34d8) SHA1(b400e737ec8e7d694d629457d8909e8320715fe5) )

	ROM_REGION( 0x020, "offsets", 0 ) // 16 entries of 9 bits, padded to 16
	ROM_LOAD( "um3481a_offsets.bin", 0x000, 0x020, BAD_DUMP CRC(a1762be7) SHA1(356c4e0b96df3f8d6ebc78e073ba7f80f2fd4939) )

	ROM_REGION( 0x010, "tones", 0 ) // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3481a_tones.bin",   0x000, 0x010, BAD_DUMP CRC(646cdaef) SHA1(48d45db842e2dd588b58ba6aa656c6496e514d23) )

	ROM_REGION( 0x008, "unknown", 0 ) // 8 entries of 7 bits, padded to bytes; the 8 x 7 area described above
	ROM_LOAD( "unknown.bin",         0x000, 0x008, BAD_DUMP CRC(87a9efc4) SHA1(54ec7dea890dea8fd2aac85d7bee6db3c71d5db9) )
ROM_END

ROM_START( um3482a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3482a_main.bin",    0x000, 0x1c0, BAD_DUMP CRC(5871d564) SHA1(4203b6513ad08ece26177778e5defeb862d1a81d) )

	ROM_REGION( 0x020, "offsets", 0 ) // 16 entries of 9 bits, padded to 16
	ROM_LOAD( "um3482a_offsets.bin", 0x000, 0x020, BAD_DUMP CRC(f39aff3c) SHA1(255dcea154ed04c6d1968b09e188ca5fc8821721) )

	ROM_REGION( 0x010, "tones", 0 ) // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3482a_tones.bin",   0x000, 0x010, BAD_DUMP CRC(c3a37f74) SHA1(67eac8c6530c202760d492f3e52c44f9cd183b46) )

	ROM_REGION( 0x008, "unknown", 0 ) // 8 entries of 7 bits, padded to bytes, same as UM3481A
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

		m_divisors[code] = tone_divisor(rom[bitswap<4>(code, 0, 1, 2, 3)] & 0x7f);
	}
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void um348x_device::device_start()
{
	decode_tone_rom();

	// FIXME: unknown what the part does with a pointer into the trailing filler; songs are cut at the last sounding word
	m_data_end = 0;
	for (u16 i = 0; i < TOTAL_NOTES; i++)
	{
		const u8 tone = decode_word(m_notes->base(), i) & 0x0f;
		if (tone != REST_TONE && tone != CTRL_TONE)
			m_data_end = i;
	}

	// One sample per oscillator cycle, so the emulated waveform lines up
	// cycle for cycle with a logic capture of the real part.
	m_stream = stream_alloc(0, 1, clock());

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
	state = state ? 1 : 0;

	if (state != m_lp)
	{
		m_stream->update();
		m_lp = state;
	}
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
	state = state ? 1 : 0;

	if (state != m_as)
	{
		m_stream->update();
		m_as = state;
	}
}


u16 um348x_device::melody_start(u8 melody) const
{
	return get_u16be(&m_offsets->base()[melody * 2]);
}


// A song whose pointer addresses the trailing filler stays silent
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
	u8 tone;

	// The mandolin selector costs no time, so skip over it rather than
	// scheduling a word of zero length
	for ( ; ; index++)
	{
		if (index >= m_note_end || index >= TOTAL_NOTES)
		{
			m_playing = false;
			m_divisor = 0;
			return;
		}

		const u8 word = decode_word(m_notes->base(), index);
		const u8 duration = (word >> 4) & 0x07;
		tone = word & 0x0f;

		u32 units;
		if ((index == m_note_start) && (tone == REST_TONE))
		{
			units = FIRST_REST_BASE_UNITS;
		}
		else if (tone == CTRL_TONE)
		{
			units = (duration == TIMBRE_MANDOLIN) ? 0 : u32(DURATION_TICKS[duration]) * m_multiplier;
		}
		else
		{
			units = u32(DURATION_TICKS[duration]) * m_multiplier;
		}

		if (units)
		{
			m_note_index = index;
			m_word_cycles = units * BASE_UNIT_CYCLES;
			break;
		}
	}

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
