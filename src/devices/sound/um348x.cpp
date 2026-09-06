// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************

    UMC UM348x multi-instrument melody generator family

    UM3481A  8 melodies
    UM3482A 12 melodies

    Both parts have 16 pointer slots and both mask ROMs fill all 16, but the
    recorded devices play only the counts above; the remaining pointers
    address filler. Which pointers a part actually exposes is not visible in
    the dumps, so the melody count is a property of the device type here.

    Mask-programmed melody generators used in doorbells, toys and low-end
    arcade bootlegs. A single on-chip RC oscillator, nominally around
    100 kHz, drives everything: tones are produced by toggling the output
    every N oscillator cycles, and note lengths are counted in units of 2048
    cycles off the same divider chain.

    Everything below was derived by reverse engineering the mask ROM dumps
    against logic-level captures of real parts. Full notes, including the
    evidence behind every constant and the questions still open, are in:

        https://github.com/clawgrip/UM348xDecoder

    Based on previous work from:
      - Sean Riddle: https://www.seanriddle.com/um348x/
      - ArcadeHacker: https://arcadehacker.blogspot.com/2020/07/um3481a-series-multi-instrument-melody.html

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
    One "base unit" is 2048 oscillator cycles. A word lasts

        ticks(duration code) * multiplier   base units

    except the first word of a melody, which always lasts exactly 8 base
    units regardless of its duration code or the melody's multiplier.

    What is NOT emulated
    --------------------
    - The tempo multiplier cannot be derived from any dumped ROM, so a
      per-melody table measured from real playback is used, falling back to
      the most common value for melodies that could not be measured.
    - One melody per part is rendered by the real chip in a staccato
      articulation: the output is re-struck once per tick, sounding for a
      fixed 1024 oscillator cycles at the head of each tick. Nothing in the
      note words marks which melody uses it, so it is not reproduced; the
      melody plays as sustained tones, correct in pitch and total length but
      not in texture.
    - Five UM3482A tone codes never occur in a passage that could be matched
      against real playback and are estimated within the range that part is
      observed to produce.

***************************************************************************/

#include "emu.h"
#include "um348x.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace {

constexpr u16 TOTAL_NOTES = 512;
constexpr u8  ROM_ROWS    = 64;
constexpr u8  ROM_GROUPS  = 7;
constexpr u8  REST_TONE   = 3;
constexpr u8  CTRL_TONE   = 1;

// Melodies visit the physical sub-columns in this order.
constexpr u8 SUBCOLUMN_ORDER[8] = { 0, 1, 2, 3, 7, 6, 5, 4 };

// One base unit, in oscillator cycles.
constexpr u16 BASE_UNIT_CYCLES = 2048;

// The first word of a melody always lasts this many base units.
constexpr u8 FIRST_REST_BASE_UNITS = 8;

// Duration code -> ticks. Codes 0, 2, 3 and 6 are counted directly from the
// staccato melody, where the part re-articulates once per tick; 5 and 7 come
// from duration ratios. Codes 1 and 4 never appear on a sounding note in
// either dump, so they cannot be counted; they are measured instead on silent
// words at melody boundaries, which is weaker evidence.
constexpr u8 DURATION_TICKS[8] = { 2, 3, 15, 4, 1, 8, 12, 6 };

constexpr u8 DEFAULT_MULTIPLIER = 5;

// Tempo multipliers measured from real playback, indexed by melody. 0 means
// "not measured", in which case DEFAULT_MULTIPLIER is used.
constexpr u8 UM3481A_MULTIPLIERS[8]  = { 5, 4, 6, 3, 4, 5, 4, 6 };
constexpr u8 UM3482A_MULTIPLIERS[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0 };

// Tone code -> oscillator divisor N; the output toggles every N cycles, so the
// tone is f_osc / (2N). 0 = silent. Both parts draw 14 divisors from a common
// family pool of 17, overlapping in 11; neither set contains the other. They
// also assign different divisors to the same code: code 11 is 38 on the
// UM3481A and 127 on the UM3482A, both from exact matches against playback.
constexpr u8 UM3481A_DIVISORS[16] =
{
	101,   0,  71,   0,  85,  48,  57,  36,
	 95,  53,  63,  38,  75,  42,  50,  45
};

// UM3482A codes 7, 9, 13, 14 and 15 are estimates: each is a divisor the part
// is observed to use, ordered as those codes are ordered on the UM3481A.
constexpr u8 UM3482A_DIVISORS[16] =
{
	  0,   0,  71,   0,  85,   0,  57,  42,
	 95,  67,  63, 127,  75,  48,  53,  50
};

} // anonymous namespace


//**************************************************************************
//  NOTE ROM DECODING
//**************************************************************************

namespace {

// Decode one note word out of the 448-byte array.
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

ROM_START( um3481a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3481araw.bin",     0x000, 0x1c0, BAD_DUMP CRC(8eef34d8) SHA1(b400e737ec8e7d694d629457d8909e8320715fe5) ) // from visual decap

	ROM_REGION( 0x018, "offsets", 0 ) // 16 entries of 12 packed bits
	ROM_LOAD( "um3481a_offsets.bin", 0x000, 0x018, BAD_DUMP CRC(66b16105) SHA1(c74b6da95318909408ddfab42cf21d3493d2b821) )

	ROM_REGION( 0x010, "tempos", 0 )  // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3481a_tempos.bin",  0x000, 0x010, BAD_DUMP CRC(136bea79) SHA1(5a4a7fa124110b54368deea4cf0fdb96f3f25c05) )
ROM_END

ROM_START( um3482a )
	ROM_REGION( 0x1c0, "notes", 0 )
	ROM_LOAD( "um3482araw.bin",      0x000, 0x1c0, BAD_DUMP CRC(5871d564) SHA1(4203b6513ad08ece26177778e5defeb862d1a81d) ) // from visual decap

	ROM_REGION( 0x020, "offsets", 0 ) // 16 entries of 9 bits, padded to 16
	ROM_LOAD( "um3482a_offsets.bin", 0x000, 0x020, BAD_DUMP CRC(f39aff3c) SHA1(255dcea154ed04c6d1968b09e188ca5fc8821721) )

	ROM_REGION( 0x010, "tempos", 0 )  // 16 entries of 7 bits, padded to bytes
	ROM_LOAD( "um3482a_tempos.bin",  0x000, 0x010, BAD_DUMP CRC(c3a37f74) SHA1(67eac8c6530c202760d492f3e52c44f9cd183b46) )
ROM_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

um348x_device::um348x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 melodies) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_notes(*this, "notes"),
	m_offsets(*this, "offsets"),
	m_tempos(*this, "tempos"),
	m_stream(nullptr),
	m_melodies(melodies),
	m_data_end(0),
	m_melody(0),
	m_trigger(0),
	m_reset(0),
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
	um348x_device(mconfig, UM3481A, tag, owner, clock, 8)
{
}

um3482a_device::um3482a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	um348x_device(mconfig, UM3482A, tag, owner, clock, 12)
{
}

const tiny_rom_entry *um3481a_device::device_rom_region() const { return ROM_NAME(um3481a); }
const tiny_rom_entry *um3482a_device::device_rom_region() const { return ROM_NAME(um3482a); }

const u8 *um3481a_device::tone_divisors() const { return UM3481A_DIVISORS; }
const u8 *um3482a_device::tone_divisors() const { return UM3482A_DIVISORS; }


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

	// Everything after the last sounding word is filler; melodies are clamped
	// to it so a stray pointer cannot play minutes of rests.
	m_data_end = 0;
	for (u16 i = 0; i < TOTAL_NOTES; i++)
	{
		const u8 tone = decode_word(m_notes->base(), i) & 0x0f;
		if (tone != REST_TONE && tone != CTRL_TONE)
			m_data_end = i;
	}

	// Tone codes with no divisor would silently drop notes; say so rather than
	// letting a future dump lose them without a trace.
	u16 unknown = 0;
	for (u16 i = 0; i <= m_data_end; i++)
	{
		const u8 tone = decode_word(m_notes->base(), i) & 0x0f;
		if (tone != REST_TONE && tone != CTRL_TONE && !tone_divisors()[tone])
			unknown |= 1 << tone;
	}
	if (unknown)
		logerror("tone codes with no divisor in the table: %04x; those notes will be silent\n", unknown);

	// One sample per oscillator cycle, so the emulated waveform lines up
	// cycle for cycle with a logic capture of the real part.
	m_stream = stream_alloc(0, 1, clock());

	save_item(NAME(m_melody));
	save_item(NAME(m_trigger));
	save_item(NAME(m_reset));
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
	m_playing = false;
	m_divisor = 0;
	m_div_count = 0;
	m_word_cycles = 0;
	m_out = 1;
}


//**************************************************************************
//  PLAYBACK
//**************************************************************************

void um348x_device::melody_w(u8 data)
{
	m_melody = data;
}

void um348x_device::trigger_w(int state)
{
	if (state && !m_trigger)
	{
		m_stream->update();

		if (m_melody < m_melodies)
		{
			const u16 start = melody_start(m_melody);
			u16 end = (m_melody + 1 < 16) ? melody_start(m_melody + 1) : TOTAL_NOTES;

			// Unused trailing pointers repeat a filler value, and the tail of
			// the ROM is filler too; never run past the last sounding word.
			if (end <= start || end > TOTAL_NOTES)
				end = TOTAL_NOTES;
			if (end > u16(m_data_end + 1))
				end = m_data_end + 1;

			if (start <= m_data_end)
			{
				const u8 *mult = (m_melodies == 8) ? UM3481A_MULTIPLIERS : UM3482A_MULTIPLIERS;
				m_multiplier = mult[m_melody] ? mult[m_melody] : DEFAULT_MULTIPLIER;

				m_note_start = start;
				m_note_end = end;
				m_playing = true;
				m_out = 1;
				start_word(start);
			}
			else
			{
				logerror("melody %d points at filler (word %d); not playing\n", m_melody, start);
			}
		}
	}

	m_trigger = state ? 1 : 0;
}

void um348x_device::reset_w(int state)
{
	if (state && !m_reset)
	{
		m_stream->update();
		device_reset();
	}

	m_reset = state ? 1 : 0;
}

void um348x_device::start_word(u16 index)
{
	const u8 word = decode_word(m_notes->base(), index);
	const u8 duration = (word >> 4) & 0x07;
	const u8 tone     = word & 0x0f;

	m_note_index = index;

	// A melody's opening rest always lasts 8 base units, whatever its duration
	// code says and whatever the melody's multiplier is. The rule is about that
	// rest specifically, so a melody that opens on a note is not affected.
	const bool openingRest = (index == m_note_start) && (tone == REST_TONE);
	const u32 units = openingRest
			? FIRST_REST_BASE_UNITS : u32(DURATION_TICKS[duration]) * m_multiplier;

	m_word_cycles = units * BASE_UNIT_CYCLES;

	if (tone == REST_TONE || tone == CTRL_TONE)
	{
		m_divisor = 0; // silent, but the word still takes its time
	}
	else
	{
		const u8 div = tone_divisors()[tone];
		m_divisor = div;
		if (div)
			m_div_count = div;
	}
}

u16 um348x_device::melody_start(u8 melody) const
{
	return melody_offset(m_offsets->base(), m_offsets->bytes(), melody);
}

void um348x_device::advance_word()
{
	const u16 next = m_note_index + 1;

	if (next >= m_note_end || next >= TOTAL_NOTES)
	{
		m_playing = false;
		m_divisor = 0;
		return;
	}

	start_word(next);
}


//-------------------------------------------------
//  sound_stream_update
//-------------------------------------------------

void um348x_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		if (m_playing)
		{
			if (m_divisor)
			{
				if (--m_div_count == 0)
				{
					m_div_count = m_divisor;
					m_out = -m_out;
				}
			}

			if (--m_word_cycles == 0)
				advance_word();
		}

		stream.put(0, i, (m_playing && m_divisor) ? sound_stream::sample_t(m_out) * 0.5 : 0.0);
	}
}
