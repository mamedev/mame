// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymfm.h"

#define VERBOSE 1
#define LOG_OUTPUT_FUNC osd_printf_verbose
#include "logmacro.h"

//
// This emulator is written from the ground-up based on analysis and deduction
// by Nemesis, particularly in this thread:
//
//    https://gendev.spritesmind.net/forum/viewtopic.php?f=24&t=386
//
// The core assumption is that these details apply to all OPN variants unless
// otherwise proven incorrect.
//
// The fine details of this implementation have also been cross-checked against
// Nemesis' implementation in his Exodus emulator, as well as Alexey Khokholov's
// "Nuked" implementation based off die shots.
//
// Operator and channel summing/mixing code is largely based off of research
// done by David Viens and Hubert Lamontagne.
//
// Search for QUESTION to find areas where I am unsure.
//
// ===================================================================================
//
// OPN pedigree:
//
//                    +--------++-----------------++------------------++--------------------------+
//    broad catgeory: |   OPN  ||      OPNA       ||      OPNB        ||            OPN2          |
//                    +--------++--------+--------++--------+---------++--------+--------+--------+
//           chip ID: | YM2203 || YM2608 | YMF288 || YM2610 | YM2610B || YM2612 | YM3438 | YMF276 |
//                    +--------++--------+--------++--------+---------++--------+--------+--------+
//               aka: |   OPN  ||  OPNA  |  OPN3  ||  OPNB  |  OPNB2  ||  OPN2  |  OPN2C |  OPN2L |
//       FM channels: |    3   ||    6   |    6   ||    4   |    6    ||    6   |    6   |    6   |
//AY-3-8910 channels: |    3   ||    3   |    3   ||    3   |    3    ||    -   |    -   |    -   |
//  ADPCM-A channels: |    -   ||  6 int |  6 int ||  6 ext |  6 ext  ||    -   |    -   |    -   |
//  ADPCM-B channels: |    -   ||  1 ext |    -   ||  1 ext |  1 ext  ||    -   |    -   |    -   |
//   Channel 6 "DAC": |   no   ||   no   |   no   ||   no   |   no    ||   yes  |   yes  |   yes  |
//     Clock divider: |  6/3/2 ||  6/3/2 |  6/3/2 ||    6   |    6    ||    6   |    6   |    6   |
//            Stereo: |   no   ||   yes  |   yes  ||   yes  |   yes   ||   yes  |   yes  |   yes  |
//               DAC: | 10.3fp || 16-bit | 16-bit || 16-bit |  16-bit ||  9-bit |  9-bit | 16-bit |
//           Summing: |  adder ||  adder |  adder ||  adder |  adder  ||  muxer |  muxer |  adder |
//               LFO: |   no   ||   yes  |   yes  ||   yes  |   yes   ||   yes  |   yes  |   yes  |
//                    +--------++--------+--------++--------+---------++--------+--------+--------+
//
// ===================================================================================
//
// From OPM to OPN:
//   - FM Channels reduced from 8 to 3
//   - Stereo removed, Hardware LFO removed, Channel 8 noise removed
//   - Hardware pitch table removed, coarse detune removed, pitch calculation is different
//   - 3 square wave channels added (GI AY-3–8910 compatible)
//   - SSG-EG envelope mode added (lets you do AY style looping envelopes on FM ops)
//   - Channel 3 can have different frequency for each op
//   - CSM only applies to channel 3
//   - Register map is different
//   - Operator timing is different. Channel 1 and 2 have very different timing.
//   - OPN’s hardware FM clock divider can be changed from /6 (default) to /2 or /3
//
// From OPN to OPNA:
//   - Channels doubled from 3 to 6
//   - Added hardware LFO (different from OPM)
//   - OPNA is stereo
//   - OPNA uses a full 16bit dac instead of a 10:3bit dac.
//   - 6 ADPCM-A drum channels added (play from built-in rom only)
//      and 1 variable rate ADPCM-B channel (streaming from a small RAM).
//   - Operator timing is different. All channels have the same timing on OPNA
//      (roughly the same timing as Channel 3 on OPN), except for Channel 6 when
//      set to algorithm 8.
//   - Frequency calculation is 1 bit less precise and can wrap.
//   - All carrier output values / 2 (this makes carrier output 13 bits instead
//      of 14 bits)
//
// OPNB/OPNB2 is a OPNA that uses external ROM for the 6 ADPCM-A channels and the
// ADPCM-B channel. ADPCM-A and ADPCM-B use different buses and different ADPCM
// encodings. OPNB(2) doesn’t have a changeable divider (always /6). OPNB has 4 FM
// channels only (ch. 1 and 4 removed), OPNB2 has 6 channels.
//
// From OPNA to OPN2:
//   - Removed GI AY-3–8910 channels and drums and streaming ADPCM
//   - Operator timing is different. All channels have the same timing on OPN2.
//   - Removed changeable divider (always /6)
//   - Carrier output values / 32 instead of / 2 (carriers output 9 bits, down
//      from 13 bits)
//   - Built-in 9bit dac, uses analog mixing (time division multiplexing). The
//      dac has a large gap between values 0 and -1 (resulting in the ladder effect).
//   - Ch6 “DAC” mode.
//
// From OPN2 to OPN2C:
//   - The DAC is more linear (no gap between 0 and -1).
//
// From OPN2C to OPN2L:
//   - Carrier output is different (full 14 bits instead of 9 bits, narrowed to
//      13 on ch. mix)
//   - Uses external DAC (16bit stereo), no analog mixing
//
// ===================================================================================
//
// OPN Test Bit Functions (YM2612)
// $21:0: Select which of two unknown signals is read as bit 14 of the test read output.
// $21:1: Some LFO control, unknown function.
// $21:2: Timers increment once every internal clock rather than once every sample. (Untested by me)
// $21:3: Freezes PG. Presumably disables writebacks to the phase register.
// $21:4: Ugly bit. Inverts MSB of operators.
// $21:5: Freezes EG. Presumably disables writebacks to the envelope counter register.
//        Unknown whether this affects the other EG state bits.
// $21:6: Enable reading test data from OPN2 rather than status flags.
// $21:7: Select LSB (1) or MSB (0) of read test data. (Yes, it's backwards.)
// $2C:2 downto 0: Ignored by OPN2, confirmed by die shot.
// $2C:3: Bit 0 of Channel 6 DAC value
// $2C:4: Read 9-bit channel output (1) instead of 14-bit operator output (0)
// $2C:5: Play DAC output over all channels (possibly except for Channel 5--in my testing
//        the DAC is the only thing you hear and it's much louder, you do not get any output
//        from Channel 5; but someone else supposedly found that the pan flags for Channel 5
//        don't affect the panning of this sound, which is only possible if it's not being
//        output during that time slot for some reason. I don't have any other reason to
//        believe this is true though).
// $2C:6: Select function of TEST pin input--both unknown functions.
// $2C:7: Set the TEST pin to be an output (1) instead of input (0).
//


//*********************************************************
//  GLOBAL TABLE LOOKUPS
//*********************************************************

//-------------------------------------------------
//  abs_sin_attenuation - given a sin (phase) input
//  where the range 0-2*PI is mapped onto 10 bits,
//  return the absolute value of sin(input),
//  logarithmically-adjusted and treated as an
//  attenuation value, in 4.8 fixed point format
//-------------------------------------------------

inline u16 abs_sin_attenuation(u16 input)
{
	// the values here are stored as 4.8 logarithmic values for 1/4 phase
	// this matches the internal format of the OPN chip, extracted from the die
	static u16 const s_sin_table[256] =
	{
		0x859,0x6c3,0x607,0x58b,0x52e,0x4e4,0x4a6,0x471,0x443,0x41a,0x3f5,0x3d3,0x3b5,0x398,0x37e,0x365,
		0x34e,0x339,0x324,0x311,0x2ff,0x2ed,0x2dc,0x2cd,0x2bd,0x2af,0x2a0,0x293,0x286,0x279,0x26d,0x261,
		0x256,0x24b,0x240,0x236,0x22c,0x222,0x218,0x20f,0x206,0x1fd,0x1f5,0x1ec,0x1e4,0x1dc,0x1d4,0x1cd,
		0x1c5,0x1be,0x1b7,0x1b0,0x1a9,0x1a2,0x19b,0x195,0x18f,0x188,0x182,0x17c,0x177,0x171,0x16b,0x166,
		0x160,0x15b,0x155,0x150,0x14b,0x146,0x141,0x13c,0x137,0x133,0x12e,0x129,0x125,0x121,0x11c,0x118,
		0x114,0x10f,0x10b,0x107,0x103,0x0ff,0x0fb,0x0f8,0x0f4,0x0f0,0x0ec,0x0e9,0x0e5,0x0e2,0x0de,0x0db,
		0x0d7,0x0d4,0x0d1,0x0cd,0x0ca,0x0c7,0x0c4,0x0c1,0x0be,0x0bb,0x0b8,0x0b5,0x0b2,0x0af,0x0ac,0x0a9,
		0x0a7,0x0a4,0x0a1,0x09f,0x09c,0x099,0x097,0x094,0x092,0x08f,0x08d,0x08a,0x088,0x086,0x083,0x081,
		0x07f,0x07d,0x07a,0x078,0x076,0x074,0x072,0x070,0x06e,0x06c,0x06a,0x068,0x066,0x064,0x062,0x060,
		0x05e,0x05c,0x05b,0x059,0x057,0x055,0x053,0x052,0x050,0x04e,0x04d,0x04b,0x04a,0x048,0x046,0x045,
		0x043,0x042,0x040,0x03f,0x03e,0x03c,0x03b,0x039,0x038,0x037,0x035,0x034,0x033,0x031,0x030,0x02f,
		0x02e,0x02d,0x02b,0x02a,0x029,0x028,0x027,0x026,0x025,0x024,0x023,0x022,0x021,0x020,0x01f,0x01e,
		0x01d,0x01c,0x01b,0x01a,0x019,0x018,0x017,0x017,0x016,0x015,0x014,0x014,0x013,0x012,0x011,0x011,
		0x010,0x00f,0x00f,0x00e,0x00d,0x00d,0x00c,0x00c,0x00b,0x00a,0x00a,0x009,0x009,0x008,0x008,0x007,
		0x007,0x007,0x006,0x006,0x005,0x005,0x005,0x004,0x004,0x004,0x003,0x003,0x003,0x002,0x002,0x002,
		0x002,0x001,0x001,0x001,0x001,0x001,0x001,0x001,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
	};

	// if the top bit is set, we're in the second half of the curve
	// which is a mirror image, so invert the index
	if (BIT(input, 8))
		input = ~input;

	// return the value from the table
	return s_sin_table[input & 0xff];
}


//-------------------------------------------------
//  attenuation_to_volume - given a 5.8 fixed point
//  logarithmic attenuation value, return a 13-bit
//  linear volume
//-------------------------------------------------

inline u16 attenuation_to_volume(u16 input)
{
	// the values here are 10-bit mantissas with an implied leading bit
	// this matches the internal format of the OPN chip, extracted from the die
	static u16 const s_power_table[256] =
	{
		0x000,0x003,0x006,0x008,0x00b,0x00e,0x011,0x014,0x016,0x019,0x01c,0x01f,0x022,0x025,0x028,0x02a,
		0x02d,0x030,0x033,0x036,0x039,0x03c,0x03f,0x042,0x045,0x048,0x04b,0x04e,0x051,0x054,0x057,0x05a,
		0x05d,0x060,0x063,0x066,0x069,0x06c,0x06f,0x072,0x075,0x078,0x07b,0x07e,0x082,0x085,0x088,0x08b,
		0x08e,0x091,0x094,0x098,0x09b,0x09e,0x0a1,0x0a4,0x0a8,0x0ab,0x0ae,0x0b1,0x0b5,0x0b8,0x0bb,0x0be,
		0x0c2,0x0c5,0x0c8,0x0cc,0x0cf,0x0d2,0x0d6,0x0d9,0x0dc,0x0e0,0x0e3,0x0e7,0x0ea,0x0ed,0x0f1,0x0f4,
		0x0f8,0x0fb,0x0ff,0x102,0x106,0x109,0x10c,0x110,0x114,0x117,0x11b,0x11e,0x122,0x125,0x129,0x12c,
		0x130,0x134,0x137,0x13b,0x13e,0x142,0x146,0x149,0x14d,0x151,0x154,0x158,0x15c,0x160,0x163,0x167,
		0x16b,0x16f,0x172,0x176,0x17a,0x17e,0x181,0x185,0x189,0x18d,0x191,0x195,0x199,0x19c,0x1a0,0x1a4,
		0x1a8,0x1ac,0x1b0,0x1b4,0x1b8,0x1bc,0x1c0,0x1c4,0x1c8,0x1cc,0x1d0,0x1d4,0x1d8,0x1dc,0x1e0,0x1e4,
		0x1e8,0x1ec,0x1f0,0x1f5,0x1f9,0x1fd,0x201,0x205,0x209,0x20e,0x212,0x216,0x21a,0x21e,0x223,0x227,
		0x22b,0x230,0x234,0x238,0x23c,0x241,0x245,0x249,0x24e,0x252,0x257,0x25b,0x25f,0x264,0x268,0x26d,
		0x271,0x276,0x27a,0x27f,0x283,0x288,0x28c,0x291,0x295,0x29a,0x29e,0x2a3,0x2a8,0x2ac,0x2b1,0x2b5,
		0x2ba,0x2bf,0x2c4,0x2c8,0x2cd,0x2d2,0x2d6,0x2db,0x2e0,0x2e5,0x2e9,0x2ee,0x2f3,0x2f8,0x2fd,0x302,
		0x306,0x30b,0x310,0x315,0x31a,0x31f,0x324,0x329,0x32e,0x333,0x338,0x33d,0x342,0x347,0x34c,0x351,
		0x356,0x35b,0x360,0x365,0x36a,0x370,0x375,0x37a,0x37f,0x384,0x38a,0x38f,0x394,0x399,0x39f,0x3a4,
		0x3a9,0x3ae,0x3b4,0x3b9,0x3bf,0x3c4,0x3c9,0x3cf,0x3d4,0x3da,0x3df,0x3e4,0x3ea,0x3ef,0x3f5,0x3fa
	};

	// look up the fractional part, then shift by the whole
	return ((s_power_table[~input & 0xff] | 0x400) << 2) >> (input >> 8);
}


//-------------------------------------------------
//  attenuation_increment - given a 6-bit ADSR
//  rate value and a 3-bit stepping index,
//  return a 4-bit increment to the attenutaion
//  for this step (or for the attack case, the
//  fractional scale factor to decrease by)
//-------------------------------------------------

inline u8 attenuation_increment(u8 rate, u8 index)
{
	static u32 const s_increment_table[64] =
	{
		0x00000000, 0x00000000, 0x10101010, 0x10101010,  // 0-3    (0x00-0x03)
		0x10101010, 0x10101010, 0x11101110, 0x11101110,  // 4-7    (0x04-0x07)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 8-11   (0x08-0x0B)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 12-15  (0x0C-0x0F)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 16-19  (0x10-0x13)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 20-23  (0x14-0x17)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 24-27  (0x18-0x1B)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 28-31  (0x1C-0x1F)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 32-35  (0x20-0x23)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 36-39  (0x24-0x27)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 40-43  (0x28-0x2B)
		0x10101010, 0x10111010, 0x11101110, 0x11111110,  // 44-47  (0x2C-0x2F)
		0x11111111, 0x21112111, 0x21212121, 0x22212221,  // 48-51  (0x30-0x33)
		0x22222222, 0x42224222, 0x42424242, 0x44424442,  // 52-55  (0x34-0x37)
		0x44444444, 0x84448444, 0x84848484, 0x88848884,  // 56-59  (0x38-0x3B)
		0x88888888, 0x88888888, 0x88888888, 0x88888888   // 60-63  (0x3C-0x3F)
	};
	return BIT(s_increment_table[rate], 4*index, 4);
}


//-------------------------------------------------
//  detune_adjustment - given a 5-bit key code
//  value and a 3-bit detune parameter, return a
//  6-bit signed phase displacement; this table
//  has been verified against Nuked's equations,
//  but the equations are rather complicated, so
//  we'll keep the simplicity of the table
//-------------------------------------------------

inline s8 detune_adjustment(u8 detune, u8 keycode)
{
	static u8 const s_detune_adjustment[32][4] =
	{
		{ 0,  0,  1,  2 },	{ 0,  0,  1,  2 },	{ 0,  0,  1,  2 },	{ 0,  0,  1,  2 },
		{ 0,  1,  2,  2 },	{ 0,  1,  2,  3 },	{ 0,  1,  2,  3 },	{ 0,  1,  2,  3 },
		{ 0,  1,  2,  4 },	{ 0,  1,  3,  4 },	{ 0,  1,  3,  4 },	{ 0,  1,  3,  5 },
		{ 0,  2,  4,  5 },	{ 0,  2,  4,  6 },	{ 0,  2,  4,  6 },	{ 0,  2,  5,  7 },
		{ 0,  2,  5,  8 },	{ 0,  3,  6,  8 },	{ 0,  3,  6,  9 },	{ 0,  3,  7, 10 },
		{ 0,  4,  8, 11 },	{ 0,  4,  8, 12 },	{ 0,  4,  9, 13 },	{ 0,  5, 10, 14 },
		{ 0,  5, 11, 16 },	{ 0,  6, 12, 17 },	{ 0,  6, 13, 19 },	{ 0,  7, 14, 20 },
		{ 0,  8, 16, 22 },	{ 0,  8, 16, 22 },	{ 0,  8, 16, 22 },	{ 0,  8, 16, 22 }
	};
	s8 result = s_detune_adjustment[keycode][detune & 3];
	return BIT(detune, 2) ? -result : result;
}


//-------------------------------------------------
//  opn_lfo_pm_phase_adjustment - given the 7 most
//  significant frequency number bits, plus a 3-bit
//  PM depth value and a signed 5-bit raw PM value,
//  return a signed PM adjustment to the frequency;
//  algorithm written to match Nuked behavior
//-------------------------------------------------

inline s16 opn_lfo_pm_phase_adjustment(u8 fnum_bits, u8 pm_sensitivity, s8 lfo_raw_pm)
{
	// this table encodes 2 shift values to apply to the top 7 bits
	// of fnum; it is effectively a cheap multiply by a constant
	// value containing 0-2 bits
	static u8 const s_lfo_pm_shifts[8][8] =
	{
		{ 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77 },
		{ 0x77, 0x77, 0x77, 0x77, 0x72, 0x72, 0x72, 0x72 },
		{ 0x77, 0x77, 0x77, 0x72, 0x72, 0x72, 0x17, 0x17 },
		{ 0x77, 0x77, 0x72, 0x72, 0x17, 0x17, 0x12, 0x12 },
		{ 0x77, 0x77, 0x72, 0x17, 0x17, 0x17, 0x12, 0x07 },
		{ 0x77, 0x77, 0x17, 0x12, 0x07, 0x07, 0x02, 0x01 },
		{ 0x77, 0x77, 0x17, 0x12, 0x07, 0x07, 0x02, 0x01 },
		{ 0x77, 0x77, 0x17, 0x12, 0x07, 0x07, 0x02, 0x01 }
	};

	// look up the relevant shifts
	s8 abs_pm = (lfo_raw_pm < 0) ? -lfo_raw_pm : lfo_raw_pm;
	u8 const shifts = s_lfo_pm_shifts[pm_sensitivity][BIT(abs_pm, 0, 3)];

	// compute the adjustment
	s16 adjust = (fnum_bits >> BIT(shifts, 0, 4)) + (fnum_bits >> BIT(shifts, 4, 4));
	if (pm_sensitivity > 5)
		adjust <<= pm_sensitivity - 5;
	adjust >>= 2;

	// every 16 cycles it inverts sign
	return (lfo_raw_pm < 0) ? -adjust : adjust;
}


//-------------------------------------------------
//  opm_keycode_to_phase_step - converts an
//  OPM concatenated block (3 bits), keycode
//  (4 bits) and key fraction (6 bits) to a 0.10
//  phase step, after applying the given delta
//-------------------------------------------------

inline u32 opm_keycode_to_phase_step(u16 block_freq, s16 delta)
{
	// The phase step is essentially the fnum in OPN-speak. To compute this table,
	// we used the standard formula for computing the frequency of a note, and
	// then converted that frequency to fnum using the formula documented in the
	// YM2608 manual.
	//
	// However, the YM2608 manual describes everything in terms of a nominal 8MHz
	// clock, which produces an FM clock of:
	//
	//    8000000 / 6(channels) / 4(operators) / 6(prescale) = 55555Hz FM clock
	//
	// Whereas the descriptions for the YM2151 use a nominal 3.579545MHz clock:
	//
	//    3579545 / 8(channels) / 4(operators) / 2(prescale) = 55930Hz FM clock
	//
	// To correct for this, the YM2608 formula was adjusted to use a clock of
	// 8053920Hz, giving this equation for the fnum:
	//
	//    fnum = (double(144) * freq * (1 << 20)) / double(8053920) / 4;
	//
	// Unfortunately, the computed table differs in a few spots from the data
	// verified from an actual chip. The table below comes from David Viens'
	// analysis, used with his permission.
	static const u32 s_phase_step[12*64] =
	{
		41568,41600,41632,41664,41696,41728,41760,41792,41856,41888,41920,41952,42016,42048,42080,42112,
		42176,42208,42240,42272,42304,42336,42368,42400,42464,42496,42528,42560,42624,42656,42688,42720,
		42784,42816,42848,42880,42912,42944,42976,43008,43072,43104,43136,43168,43232,43264,43296,43328,
		43392,43424,43456,43488,43552,43584,43616,43648,43712,43744,43776,43808,43872,43904,43936,43968,
		44032,44064,44096,44128,44192,44224,44256,44288,44352,44384,44416,44448,44512,44544,44576,44608,
		44672,44704,44736,44768,44832,44864,44896,44928,44992,45024,45056,45088,45152,45184,45216,45248,
		45312,45344,45376,45408,45472,45504,45536,45568,45632,45664,45728,45760,45792,45824,45888,45920,
		45984,46016,46048,46080,46144,46176,46208,46240,46304,46336,46368,46400,46464,46496,46528,46560,
		46656,46688,46720,46752,46816,46848,46880,46912,46976,47008,47072,47104,47136,47168,47232,47264,
		47328,47360,47392,47424,47488,47520,47552,47584,47648,47680,47744,47776,47808,47840,47904,47936,
		48032,48064,48096,48128,48192,48224,48288,48320,48384,48416,48448,48480,48544,48576,48640,48672,
		48736,48768,48800,48832,48896,48928,48992,49024,49088,49120,49152,49184,49248,49280,49344,49376,
		49440,49472,49504,49536,49600,49632,49696,49728,49792,49824,49856,49888,49952,49984,50048,50080,
		50144,50176,50208,50240,50304,50336,50400,50432,50496,50528,50560,50592,50656,50688,50752,50784,
		50880,50912,50944,50976,51040,51072,51136,51168,51232,51264,51328,51360,51424,51456,51488,51520,
		51616,51648,51680,51712,51776,51808,51872,51904,51968,52000,52064,52096,52160,52192,52224,52256,
		52384,52416,52448,52480,52544,52576,52640,52672,52736,52768,52832,52864,52928,52960,52992,53024,
		53120,53152,53216,53248,53312,53344,53408,53440,53504,53536,53600,53632,53696,53728,53792,53824,
		53920,53952,54016,54048,54112,54144,54208,54240,54304,54336,54400,54432,54496,54528,54592,54624,
		54688,54720,54784,54816,54880,54912,54976,55008,55072,55104,55168,55200,55264,55296,55360,55392,
		55488,55520,55584,55616,55680,55712,55776,55808,55872,55936,55968,56032,56064,56128,56160,56224,
		56288,56320,56384,56416,56480,56512,56576,56608,56672,56736,56768,56832,56864,56928,56960,57024,
		57120,57152,57216,57248,57312,57376,57408,57472,57536,57568,57632,57664,57728,57792,57824,57888,
		57952,57984,58048,58080,58144,58208,58240,58304,58368,58400,58464,58496,58560,58624,58656,58720,
		58784,58816,58880,58912,58976,59040,59072,59136,59200,59232,59296,59328,59392,59456,59488,59552,
		59648,59680,59744,59776,59840,59904,59936,60000,60064,60128,60160,60224,60288,60320,60384,60416,
		60512,60544,60608,60640,60704,60768,60800,60864,60928,60992,61024,61088,61152,61184,61248,61280,
		61376,61408,61472,61536,61600,61632,61696,61760,61824,61856,61920,61984,62048,62080,62144,62208,
		62272,62304,62368,62432,62496,62528,62592,62656,62720,62752,62816,62880,62944,62976,63040,63104,
		63200,63232,63296,63360,63424,63456,63520,63584,63648,63680,63744,63808,63872,63904,63968,64032,
		64096,64128,64192,64256,64320,64352,64416,64480,64544,64608,64672,64704,64768,64832,64896,64928,
		65024,65056,65120,65184,65248,65312,65376,65408,65504,65536,65600,65664,65728,65792,65856,65888,
		65984,66016,66080,66144,66208,66272,66336,66368,66464,66496,66560,66624,66688,66752,66816,66848,
		66944,66976,67040,67104,67168,67232,67296,67328,67424,67456,67520,67584,67648,67712,67776,67808,
		67904,67936,68000,68064,68128,68192,68256,68288,68384,68448,68512,68544,68640,68672,68736,68800,
		68896,68928,68992,69056,69120,69184,69248,69280,69376,69440,69504,69536,69632,69664,69728,69792,
		69920,69952,70016,70080,70144,70208,70272,70304,70400,70464,70528,70560,70656,70688,70752,70816,
		70912,70976,71040,71104,71136,71232,71264,71360,71424,71488,71552,71616,71648,71744,71776,71872,
		71968,72032,72096,72160,72192,72288,72320,72416,72480,72544,72608,72672,72704,72800,72832,72928,
		72992,73056,73120,73184,73216,73312,73344,73440,73504,73568,73632,73696,73728,73824,73856,73952,
		74080,74144,74208,74272,74304,74400,74432,74528,74592,74656,74720,74784,74816,74912,74944,75040,
		75136,75200,75264,75328,75360,75456,75488,75584,75648,75712,75776,75840,75872,75968,76000,76096,
		76224,76288,76352,76416,76448,76544,76576,76672,76736,76800,76864,76928,77024,77120,77152,77248,
		77344,77408,77472,77536,77568,77664,77696,77792,77856,77920,77984,78048,78144,78240,78272,78368,
		78464,78528,78592,78656,78688,78784,78816,78912,78976,79040,79104,79168,79264,79360,79392,79488,
		79616,79680,79744,79808,79840,79936,79968,80064,80128,80192,80256,80320,80416,80512,80544,80640,
		80768,80832,80896,80960,80992,81088,81120,81216,81280,81344,81408,81472,81568,81664,81696,81792,
		81952,82016,82080,82144,82176,82272,82304,82400,82464,82528,82592,82656,82752,82848,82880,82976
	};

	// extract the block (octave) first
	u8 block = BIT(block_freq, 10, 3);

	// the keycode (bits 6-9) is "gappy", mapping 12 values over 16 in each
	// octave; to correct for this, we multiply the 4-bit value by 3/4 (or
	// rather subtract 1/4); note that a (invalid) value of 15 will bleed into
	// the next octave -- this is confirmed
	u8 adjusted_code = BIT(block_freq, 6, 4) - BIT(block_freq, 8, 2);

	// now re-insert the 6-bit fraction
	s16 eff_freq = (adjusted_code << 6) | BIT(block_freq, 0, 6);

	// now that the gaps are removed, add the delta
	eff_freq += delta;

	// handle over/underflow by adjusting the block:
	if (u16(eff_freq) >= 768)
	{
		// minimum delta is -512 (PM), so we can only underflow by 1 octave
		if (eff_freq < 0)
		{
			eff_freq += 768;
			if (block-- == 0)
				return s_phase_step[0] >> 7;
		}

		// maximum delta is +512+608 (PM+detune), so we can overflow by up to 2 octaves
		else
		{
			eff_freq -= 768;
			if (eff_freq >= 768)
				block++, eff_freq -= 768;
			if (block++ >= 7)
				return s_phase_step[767];
		}
	}

	// look up the phase shift for the key code, then shift by octave
	return s_phase_step[eff_freq] >> (block ^ 7);
}



//*********************************************************
//  YMFM OPERATOR
//*********************************************************

//-------------------------------------------------
//  ymfm_operator - constructor
//-------------------------------------------------

template<class RegisterType>
ymfm_operator<RegisterType>::ymfm_operator(RegisterType regs) :
	m_phase(0),
	m_env_attenuation(0x3ff),
	m_env_state(ENV_RELEASE),
	m_ssg_inverted(false),
	m_key_state(0),
	m_keyon(0),
	m_csm_triggered(0),
	m_regs(regs)
{
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

ALLOW_SAVE_TYPE(ymfm_operator<ymopm_registers>::envelope_state);
ALLOW_SAVE_TYPE(ymfm_operator<ymopn_registers>::envelope_state);
ALLOW_SAVE_TYPE(ymfm_operator<ymopna_registers>::envelope_state);

template<class RegisterType>
void ymfm_operator<RegisterType>::save(device_t &device, u8 index)
{
	// save our data
	device.save_item(YMFM_NAME(m_phase), index);
	device.save_item(YMFM_NAME(m_env_attenuation), index);
	device.save_item(YMFM_NAME(m_env_state), index);
	device.save_item(YMFM_NAME(m_ssg_inverted), index);
	device.save_item(YMFM_NAME(m_key_state), index);
	device.save_item(YMFM_NAME(m_keyon), index);
	device.save_item(YMFM_NAME(m_csm_triggered), index);
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::reset()
{
	// reset our data
	m_phase = 0;
	m_env_attenuation = 0x3ff;
	m_env_state = ENV_RELEASE;
	m_ssg_inverted = 0;
	m_key_state = 0;
	m_keyon = 0;
	m_csm_triggered = 0;
}


//-------------------------------------------------
//  clock - master clocking function
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::clock(u32 env_counter, s8 lfo_raw_pm, u16 block_freq)
{
	// clock the key state
	u8 keycode = block_freq_to_keycode(block_freq);
	clock_keystate(m_keyon | m_csm_triggered, keycode);
	m_csm_triggered = 0;

	// clock the SSG-EG state (OPN/OPNA)
	if (m_regs.ssg_eg_enabled())
		clock_ssg_eg_state(keycode);

	// clock the envelope if on an envelope cycle
	if (BIT(env_counter, 0, 2) == 0)
		clock_envelope(env_counter >> 2, keycode);

	// clock the phase
	clock_phase(lfo_raw_pm, block_freq);
}


//-------------------------------------------------
//  compute_volume - compute the 14-bit signed
//  volume of this operator, given a phase
//  modulation and an AM LFO offset
//-------------------------------------------------

template<class RegisterType>
s16 ymfm_operator<RegisterType>::compute_volume(u16 modulation, u16 am_offset) const
{
	// start with the upper 10 bits of the phase value plus modulation
	// the low 10 bits of this result represents a full 2*PI period over
	// the full sin wave
	u16 phase = (m_phase >> 10) + modulation;

	// get the absolute value of the sin, as attenuation, as a 4.8 fixed point value
	u16 sin_attenuation = abs_sin_attenuation(phase);

	// get the attenuation from the evelope generator as a 4.6 value, shifted up to 4.8
	u16 env_attenuation = envelope_attenuation(am_offset) << 2;

	// combine into a 5.8 value, then convert from attenuation to 13-bit linear volume
	s16 result = attenuation_to_volume(sin_attenuation + env_attenuation);

	// negate if in the negative part of the sin wave (sign bit gives 14 bits)
	return BIT(phase, 9) ? -result : result;
}


//-------------------------------------------------
//  compute_noise_volume - compute the 14-bit
//  signed noise volume of this operator, given a
//  noise input value and an AM offset
//-------------------------------------------------

template<class RegisterType>
s16 ymfm_operator<RegisterType>::compute_noise_volume(u8 noise_state, u16 am_offset) const
{
	// application manual says the logarithmic transform is not applied here, so we
	// just use the raw envelope attenuation, inverted (since 0 attenuation should be
	// maximum), and shift it up from a 10-bit value to an 11-bit value
	u16 result = (envelope_attenuation(am_offset) ^ 0x3ff) << 1;

	// QUESTION: is AM applied still?

	// negate based on the noise state
	return BIT(noise_state, 0) ? -result : result;
}


//-------------------------------------------------
//  block_freq_to_keycode - given a concatenated
//  block+frequency value, return the 5-bit keycode
//-------------------------------------------------

// OPM version
template<>
u8 ymfm_operator<ymopm_registers>::block_freq_to_keycode(u16 block_freq)
{
	// block_freq is block(3b):keycode(4b):keyfrac(6b); the 5-bit keycode
	// we want is just the top 5 bits here
	return BIT(block_freq, 8, 5);
}

// OPN/OPNA version
template<class RegisterType>
u8 ymfm_operator<RegisterType>::block_freq_to_keycode(u16 block_freq)
{
	// block_freq is block(3b):fnum(11b); the 5-bit keycode uses the top
	// 4 bits plus a magic formula for the final bit
	u8 keycode = BIT(block_freq, 10, 4) << 1;

	// lowest bit is determined by a mix of next lower FNUM bits
	// according to this equation from the YM2608 manual:
	//
	//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
	//
	// for speed, we just look it up in a 16-bit constant
	return keycode | BIT(0xfe80, BIT(block_freq, 7, 4));
}


//-------------------------------------------------
//  effective_rate - return the effective 6-bit
//  ADSR rate value after adjusting for keycode
//-------------------------------------------------

template<class RegisterType>
u8 ymfm_operator<RegisterType>::effective_rate(u8 rawrate, u8 keycode)
{
	if (rawrate == 0)
		return 0;
	u8 rate = rawrate * 2 + (keycode >> (m_regs.ksr() ^ 3));
	return (rate < 64) ? rate : 63;
}


//-------------------------------------------------
//  start_attack - start the attack phase; called
//  when a keyon happens or when an SSG-EG cycle
//  is complete and restarts
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::start_attack(u8 keycode)
{
	// don't change anything if already in attack state
	if (m_env_state == ENV_ATTACK)
		return;
	m_env_state = ENV_ATTACK;

	// generally not inverted at start, except if SSG-EG is
	// enabled and one of the inverted modes is specified
	m_ssg_inverted = m_regs.ssg_eg_enabled() & BIT(m_regs.ssg_eg_mode(), 2);

	// reset the phase when we start an attack
	m_phase = 0;

	// if the attack rate >= 62 then immediately go to max attenuation
	if (effective_rate(m_regs.attack_rate(), keycode) >= 62)
		m_env_attenuation = 0;

	// log key on events under certain conditions
	if (m_regs.lfo_waveform() == 3 && m_regs.lfo_enabled() && ((m_regs.lfo_am_enabled() && m_regs.lfo_am_sensitivity() != 0) || m_regs.lfo_pm_sensitivity() != 0))
	{
		LOG("KeyOn %d.%d: freq=%04X dt2=%d fb=%d alg=%d dt=%d mul=%X tl=%02X ksr=%d adsr=%02X/%02X/%02X/%X sl=%X pan=%c%c",
			m_regs.chnum(), m_regs.opnum(),
			m_regs.block_freq(),
			m_regs.detune2(),
			m_regs.feedback(),
			m_regs.algorithm(),
			m_regs.detune(),
			m_regs.multiple(),
			m_regs.total_level(),
			m_regs.ksr(),
			m_regs.attack_rate(),
			m_regs.decay_rate(),
			m_regs.sustain_rate(),
			m_regs.release_rate(),
			m_regs.sustain_level(),
			m_regs.pan_left() ? 'L' : '-',
			m_regs.pan_right() ? 'R' : '-');
		if (m_regs.ssg_eg_enabled())
			LOG(" ssg=%X", m_regs.ssg_eg_mode());
		if (m_regs.lfo_enabled() && ((m_regs.lfo_am_enabled() && m_regs.lfo_am_sensitivity() != 0) || m_regs.lfo_pm_sensitivity() != 0))
			LOG(" am=%d pm=%d w=%d", m_regs.lfo_am_enabled() ? m_regs.lfo_am_sensitivity() : 0, m_regs.lfo_pm_sensitivity(), m_regs.lfo_waveform());
		if (m_regs.noise_enabled() && m_regs.opnum() == 3 && m_regs.chnum() == 7)
			LOG(" noise=1");
		LOG("\n");
	}
}


//-------------------------------------------------
//  start_release - start the release phase;
//  called when a keyoff happens
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::start_release()
{
	// don't change anything if already in release state
	if (m_env_state == ENV_RELEASE)
		return;
	m_env_state = ENV_RELEASE;

	// adjust attenuation if inverted due to SSG-EG
	if (m_ssg_inverted)
		m_env_attenuation = 0x200 - m_env_attenuation;
}


//-------------------------------------------------
//  clock_keystate - clock the keystate to match
//  the incoming keystate
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::clock_keystate(u8 keystate, u8 keycode)
{
	assert(keystate == 0 || keystate == 1);

	// has the key changed?
	if ((keystate ^ m_key_state) != 0)
	{
		m_key_state = keystate;

		// if the key has turned on, start the attack
		if (keystate != 0)
			start_attack(keycode);

		// otherwise, start the release
		else
			start_release();
	}
}


//-------------------------------------------------
//  clock_ssg_eg_state - clock the SSG-EG state;
//  should only be called if SSG-EG is enabled
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::clock_ssg_eg_state(u8 keycode)
{
	// work only happens once the attenuation crosses above 0x200
	if (!BIT(m_env_attenuation, 9))
		return;

	// 8 SSG-EG modes:
	//    000: repeat normally
	//    001: run once, hold low
	//    010: repeat, alternating between inverted/non-inverted
	//    011: run once, hold high
	//    100: inverted repeat normally
	//    101: inverted run once, hold low
	//    110: inverted repeat, alternating between inverted/non-inverted
	//    111: inverted run once, hold high
	u8 mode = m_regs.ssg_eg_mode();

	// hold modes (1/3/5/7)
	if (BIT(mode, 0))
	{
		// set the inverted flag to the end state (0 for modes 1/7, 1 for modes 3/5)
		m_ssg_inverted = BIT(mode, 2) ^ BIT(mode, 1);

		// if holding low (modes 1/5), force the attenuation to maximum
		// once we're past the attack phase
		if (m_env_state != ENV_ATTACK && BIT(mode, 1) == 0)
			m_env_attenuation = 0x3ff;
	}

	// continuous modes (0/2/4/6)
	else
	{
		// toggle invert in alternating mode (even in attack state)
		m_ssg_inverted ^= BIT(mode, 1);

		// restart attack if in decay/sustain states
		if (m_env_state == ENV_DECAY || m_env_state == ENV_SUSTAIN)
			start_attack(keycode);

		// phase is reset to 0 regardless in modes 0/4
		if (BIT(mode, 1) == 0)
			m_phase = 0;
	}

	// in all modes, once we hit release state, attenuation is forced to maximum
	if (m_env_state == ENV_RELEASE)
		m_env_attenuation = 0x3ff;
}


//-------------------------------------------------
//  clock_envelope - clock the envelope state
//  according to the given count
//-------------------------------------------------

template<class RegisterType>
void ymfm_operator<RegisterType>::clock_envelope(u16 env_counter, u8 keycode)
{
	// if in attack state, see if we hit minimum attenuation
	if (m_env_state == ENV_ATTACK && m_env_attenuation == 0)
		m_env_state = ENV_DECAY;

	// if in decay state, see if we hit the sustain level
	else if (m_env_state == ENV_DECAY)
	{
		// 4-bit sustain level, but 15 means 31 so effectively 5 bits
		u8 target = m_regs.sustain_level();
		target |= (target + 1) & 0x10;

		// bring current attenuation down to 5 bits and compare
		if ((m_env_attenuation >> 5) >= target)
			m_env_state = ENV_SUSTAIN;
	}

	// determine our raw 5-bit rate value
	u8 rate = effective_rate(m_regs.adsr_rate(m_env_state), keycode);

	// compute the rate shift value; this is the shift needed to
	// apply to the env_counter such that it becomes a 5.11 fixed
	// point number
	u8 rate_shift = rate >> 2;
	env_counter <<= rate_shift;

	// see if the fractional part is 0; if not, it's not time to clock
	if (BIT(env_counter, 0, 11) != 0)
		return;

	// determine the increment based on the non-fractional part of env_counter
	u8 increment = attenuation_increment(rate, BIT(env_counter, 11, 3));

	// attack is the only one that increases
	if (m_env_state == ENV_ATTACK)
	{
		// glitch means that attack rates of 62/63 don't increment if
		// changed after the initial key on (where they are handled
		// specially)

		// QUESTION: this check affects one of the operators on the gng credit sound
		//   is it correct?
		// QUESTION: does this apply only to YM2612?
		if (rate < 62)
			m_env_attenuation += (~m_env_attenuation * increment) >> 4;
	}

	// all other cases are similar
	else
	{
		// non-SSG-EG cases just apply the increment
		if (!m_regs.ssg_eg_enabled())
			m_env_attenuation += increment;

		// SSG-EG only applies if less than mid-point, and then at 4x
		else if (m_env_attenuation < 0x200)
			m_env_attenuation += 4 * increment;

		// clamp the final attenuation
		if (m_env_attenuation >= 0x400)
			m_env_attenuation = 0x3ff;
	}
}


//-------------------------------------------------
//  clock_phase - clock the 10.10 phase value; the
//  OPN version of the logic has been verified
//  against the Nuked phase generator
//-------------------------------------------------

// OPM version
template<>
void ymfm_operator<ymopm_registers>::clock_phase(s8 lfo_raw_pm, u16 block_freq)
{
	// start with coarse detune delta; table uses cents value from
	// manual, converted into 1/64ths
	static const s16 s_detune2_delta[4] = { 0, (600*64+50)/100, (781*64+50)/100, (950*64+50)/100 };
	s16 delta = s_detune2_delta[m_regs.detune2()];

	// add in the PM delta
	u8 pm_sensitivity = m_regs.lfo_pm_sensitivity();
	if (pm_sensitivity != 0)
	{
		// raw PM value is -127..128 which is +/- 200 cents
		// manual gives these magnitudes in cents:
		//    0, +/-5, +/-10, +/-20, +/-50, +/-100, +/-400, +/-700
		// this roughly corresponds to shifting the 200-cent value:
		//    0  >> 5,  >> 4,  >> 3,  >> 2,  >> 1,   << 1,   << 2
		if (pm_sensitivity < 6)
			delta += lfo_raw_pm >> (6 - pm_sensitivity);
		else
			delta += lfo_raw_pm << (pm_sensitivity - 5);
	}

	// apply delta and convert to a frequency number
	u32 phase_step = opm_keycode_to_phase_step(block_freq, delta);

	// apply detune based on the keycode
	phase_step += detune_adjustment(m_regs.detune(), block_freq_to_keycode(block_freq));

	// QUESTION: do we clamp to 17 bits like YM2612?

	// apply frequency multiplier (0 means 0.5, other values are as-is)
	u8 multiple = m_regs.multiple();
	if (multiple == 0)
		phase_step >>= 1;
	else
		phase_step *= multiple;

	// finally apply the step to the current phase value
	m_phase += phase_step;
}

template<class RegisterType>
void ymfm_operator<RegisterType>::clock_phase(s8 lfo_raw_pm, u16 block_freq)
{
	// extract frequency number (low 11 bits of block_freq)
	u16 fnum = BIT(block_freq, 0, 11) << 1;

	// if there's a non-zero PM sensitivity, compute the adjustment
	u8 pm_sensitivity = m_regs.lfo_pm_sensitivity();
	if (pm_sensitivity != 0)
	{
		// apply the phase adjustment based on the upper 7 bits
		// of FNUM and the PM depth parameters
		fnum += opn_lfo_pm_phase_adjustment(BIT(block_freq, 4, 7), pm_sensitivity, lfo_raw_pm);

		// keep fnum to 12 bits
		fnum &= 0xfff;
	}

	// apply block shift to compute phase step
	u8 block = BIT(block_freq, 11, 3);
	u32 phase_step = (fnum << block) >> 2;

	// apply detune based on the keycode
	phase_step += detune_adjustment(m_regs.detune(), block_freq_to_keycode(block_freq));

	// clamp to 17 bits in case detune overflows
	// QUESTION: is this specific to the YM2612/3438?
	phase_step &= 0x1ffff;

	// apply frequency multiplier (0 means 0.5, other values are as-is)
	u8 multiple = m_regs.multiple();
	if (multiple == 0)
		phase_step >>= 1;
	else
		phase_step *= multiple;

	// finally apply the step to the current phase value
	m_phase += phase_step;
}


//-------------------------------------------------
//  envelope_attenuation - return the effective
//  attenuation of the envelope
//-------------------------------------------------

template<class RegisterType>
u16 ymfm_operator<RegisterType>::envelope_attenuation(u8 am_offset) const
{
	u16 result = m_env_attenuation;

	// invert if necessary due to SSG-EG
	if (m_ssg_inverted)
		result = (0x200 - result) & 0x3ff;

	// add in LFO AM modulation
	if (m_regs.lfo_am_enabled())
		result += am_offset;

	// add in total level
	result += m_regs.total_level() << 3;

	// clamp to max and return
	return (result < 0x400) ? result : 0x3ff;
}



//*********************************************************
//  YMFM_CHANNEL
//*********************************************************

//-------------------------------------------------
//  ymfm_channel - constructor
//-------------------------------------------------

template<class RegisterType>
ymfm_channel<RegisterType>::ymfm_channel(RegisterType regs) :
	m_feedback{ 0, 0 },
	m_feedback_in(0),
	m_op1(regs.operator_registers(0)),
	m_op2(regs.operator_registers(1)),
	m_op3(regs.operator_registers(2)),
	m_op4(regs.operator_registers(3)),
	m_regs(regs)
{
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::save(device_t &device, u8 index)
{
	// save our data
	device.save_item(YMFM_NAME(m_feedback), index);
	device.save_item(YMFM_NAME(m_feedback_in), index);

	// save operator data
	m_op1.save(device, index * 4 + 0);
	m_op2.save(device, index * 4 + 1);
	m_op3.save(device, index * 4 + 2);
	m_op4.save(device, index * 4 + 3);
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::reset()
{
	// reset our data
	m_feedback[0] = m_feedback[1] = 0;
	m_feedback_in = 0;

	// reset the operators
	m_op1.reset();
	m_op2.reset();
	m_op3.reset();
	m_op4.reset();
}


//-------------------------------------------------
//  keyonoff - signal key on/off to our operators
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::keyonoff(u8 states)
{
	m_op1.keyonoff(BIT(states, 0));
	m_op2.keyonoff(BIT(states, 1));
	m_op3.keyonoff(BIT(states, 2));
	m_op4.keyonoff(BIT(states, 3));
}


//-------------------------------------------------
//  keyon_csm - signal CSM key on to our operators
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::keyon_csm()
{
	m_op1.keyon_csm();
	m_op2.keyon_csm();
	m_op3.keyon_csm();
	m_op4.keyon_csm();
}


//-------------------------------------------------
//  clock - master clock of all operators
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::clock(u32 env_counter, s8 lfo_raw_pm, bool is_multi_freq)
{
	// grab common block/fnum values
	u16 block_freq = m_regs.block_freq();

	// clock the feedback through
	m_feedback[0] = m_feedback[1];
	m_feedback[1] = m_feedback_in;

	// in multi-frequency mode, the first 3 channels use independent block/fnum values
	if (is_multi_freq)
	{
		m_op1.clock(env_counter, lfo_raw_pm, m_regs.multi_block_freq1());
		m_op2.clock(env_counter, lfo_raw_pm, m_regs.multi_block_freq2());
		m_op3.clock(env_counter, lfo_raw_pm, m_regs.multi_block_freq0());
	}

	// otherwise, all channels use the common block/fnum
	else
	{
		m_op1.clock(env_counter, lfo_raw_pm, block_freq);
		m_op2.clock(env_counter, lfo_raw_pm, block_freq);
		m_op3.clock(env_counter, lfo_raw_pm, block_freq);
	}

	// operator 3 uses the common values in all cases
	m_op4.clock(env_counter, lfo_raw_pm, block_freq);
}


//-------------------------------------------------
//  output - combine the operators according to the
//  specified algorithm, returning a sum according
//  to the rshift and clipmax parameters, which
//  vary between different OPN implementations
//-------------------------------------------------

template<class RegisterType>
void ymfm_channel<RegisterType>::output(u8 lfo_raw_am, u8 noise_state, s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax) const
{
	// AM amount is the same across all operators; compute it once
	u16 am_offset = lfo_am_offset(lfo_raw_am);

	// Algorithms:
	//    0: O1 -> O2 -> O3 -> O4 -> out
	//    1: (O1 + O2) -> O3 -> O4 -> out
	//    2: (O1 + (O2 -> O3)) -> O4 -> out
	//    3: ((O1 -> O2) + O3) -> O4 -> out
	//    4: ((O1 -> O2) + (O3 -> O4)) -> out
	//    5: ((O1 -> O2) + (O1 -> O3) + (O1 -> O4)) -> out
	//    6: ((O1 -> O2) + O3 + O4) -> out
	//    7: (O1 + O2 + O3 + O4) -> out
	//
	// The operators are computed in order, with the inputs pulled from
	// an array of values that is populated as we go:
	//    0 = 0
	//    1 = O1
	//    2 = O2
	//    3 = O3
	//    4 = O4
	//    5 = O1+O2
	//    6 = O1+O3
	//    7 = O2+O3
	//
	// This table encodes for operators 2-4 which of the 8 input values
	// above is used: 1 bit for O2 and 3 bits for O3 and O4
	static u8 const s_algorithm_inputs[8] =
	{
	// OP2  OP3        OP4
		1 | (2 << 1) | (3 << 4),
		0 | (5 << 1) | (3 << 4),
		0 | (2 << 1) | (6 << 4),
		1 | (0 << 1) | (7 << 4),
		1 | (0 << 1) | (3 << 4),
		1 | (1 << 1) | (1 << 4),
		1 | (0 << 1) | (0 << 4),
		0 | (0 << 1) | (0 << 4)
	};
	u8 algorithm = m_regs.algorithm();
	u8 algorithm_inputs = s_algorithm_inputs[algorithm];
	s16 opout[8];
	opout[0] = 0;

	// operator 1 has optional self-feedback
	s16 modulation = 0;
	u8 feedback = m_regs.feedback();
	if (feedback != 0)
		modulation = (m_feedback[0] + m_feedback[1]) >> (10 - feedback);

	// compute the 14-bit volume/value of operator 1 and update the feedback
	opout[1] = m_feedback_in = m_op1.compute_volume(modulation, am_offset);

	// no that the feedback has been computed, skip the rest if both pans are clear;
	// no need to do all this work for nothing
	if (m_regs.pan_left() == 0 && m_regs.pan_right() == 0)
		return;

	// compute the 14-bit volume/value of operator 2
	opout[2] = m_op2.compute_volume(opout[BIT(algorithm_inputs, 0, 1)] >> 1, am_offset);
	opout[5] = opout[1] + opout[2];

	// compute the 14-bit volume/value of operator 3
	opout[3] = m_op3.compute_volume(opout[BIT(algorithm_inputs, 1, 3)] >> 1, am_offset);
	opout[6] = opout[1] + opout[3];
	opout[7] = opout[2] + opout[3];

	// compute the 14-bit volume/value of operator 4; this could be a noise
	// value on the OPM
	if (noise_state != 0)
		opout[4] = m_op4.compute_noise_volume(noise_state, am_offset);
	else
		opout[4] = m_op4.compute_volume(opout[BIT(algorithm_inputs, 4, 3)] >> 1, am_offset);

	// all algorithms consume OP4 output
	s16 result = opout[4] >> rshift;

	// algorithms 4-7 add in OP2 output
	if (algorithm >= 4)
	{
		s16 clipmin = -clipmax - 1;
		result += opout[2] >> rshift;
		result = std::clamp(result, clipmin, clipmax);

		// agorithms 5-7 add in OP3 output
		if (algorithm >= 5)
		{
			result += opout[3] >> rshift;
			result = std::clamp(result, clipmin, clipmax);

			// algorithm 7 adds in OP1 output
			if (algorithm == 7)
			{
				result += opout[1] >> rshift;
				result = std::clamp(result, clipmin, clipmax);
			}
		}
	}

	// add to the output
	if (m_regs.pan_left())
		lsum += result;
	if (m_regs.pan_right())
		rsum += result;
}


//-------------------------------------------------
//  lfo_am_offset - convert a 6/8-bit raw AM value
//  into an amplitude offset based on sensitivity
//-------------------------------------------------

// OPM version
template<>
u16 ymfm_channel<ymopm_registers>::lfo_am_offset(u8 lfo_raw_am) const
{
	// shift value for AM sensitivity is [*, 0, 1, 2],
	// mapping to values of [0, 23.9, 47.8, and 95.6dB]
	u8 am_sensitivity = m_regs.lfo_am_sensitivity();
	if (am_sensitivity == 0)
		return 0;

	// QUESTION: see OPN note below for the dB range mapping; it applies
	// here as well

	// raw LFO AM value on OPM is 0-FF, which is already a factor of 2
	// larger than the OPN below, putting our staring point at 2x theirs;
	// this works out since our minimum is 2x their maximum
	return lfo_raw_am << (am_sensitivity - 1);
}

// OPN/OPNA version
template<class RegisterType>
u16 ymfm_channel<RegisterType>::lfo_am_offset(u8 lfo_raw_am) const
{
	// shift value for AM sensitivity is [7, 3, 1, 0],
	// mapping to values of [0, 1.4, 5.9, and 11.8dB]
	u8 am_shift = (1 << (m_regs.lfo_am_sensitivity() ^ 3)) - 1;

	// QUESTION: max sensitivity should give 11.8dB range, but this value
	// is directly added to an x.8 attenuation value, which will only give
	// 126/256 or ~4.9dB range -- what am I missing? The calculation below
	// matches several other emulators, including the Nuked implemenation.

	// raw LFO AM value on OPN is 0-3F, scale that up by a factor of 2
	// (giving 7 bits) before applying the final shift
	return (lfo_raw_am << 1) >> am_shift;
}



//*********************************************************
//  YMFM ENGINE BASE
//*********************************************************

//-------------------------------------------------
//  ymfm_engine_base - constructor
//-------------------------------------------------

template<class RegisterType>
ymfm_engine_base<RegisterType>::ymfm_engine_base(device_t &device) :
	m_device(device),
	m_env_counter(0),
	m_lfo_counter(0),
	m_noise_lfsr(0),
	m_noise_counter(0),
	m_noise_state(0),
	m_noise_lfo(0),
	m_lfo_am(0),
	m_status(0),
	m_clock_prescale(RegisterType::DEFAULT_PRESCALE),
	m_irq_mask(STATUS_TIMERA | STATUS_TIMERB),
	m_irq_state(0),
	m_busy_end(attotime::zero),
	m_timer{ nullptr, nullptr },
	m_irq_handler(device),
	m_regdata(RegisterType::REGISTERS),
	m_regs(m_regdata)
{
	// create the channels
	for (int chnum = 0; chnum < RegisterType::CHANNELS; chnum++)
		m_channel[chnum] = std::make_unique<ymfm_channel<RegisterType>>(m_regs.channel_registers(chnum));
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::save(device_t &device)
{
	// allocate our timers
	for (int tnum = 0; tnum < 2; tnum++)
		m_timer[tnum] = device.machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ymfm_engine_base::timer_handler), this));

	// resolve the IRQ handler while we're here
	m_irq_handler.resolve();

	// save our data
	device.save_item(YMFM_NAME(m_env_counter));
	device.save_item(YMFM_NAME(m_lfo_counter));
	device.save_item(YMFM_NAME(m_noise_lfsr));
	device.save_item(YMFM_NAME(m_noise_counter));
	device.save_item(YMFM_NAME(m_noise_state));
	device.save_item(YMFM_NAME(m_noise_lfo));
	device.save_item(YMFM_NAME(m_lfo_am));
	device.save_item(YMFM_NAME(m_status));
	device.save_item(YMFM_NAME(m_clock_prescale));
	device.save_item(YMFM_NAME(m_irq_mask));
	device.save_item(YMFM_NAME(m_irq_state));
	device.save_item(YMFM_NAME(m_busy_end));
	device.save_item(YMFM_NAME(m_regdata));

	// save channel data
	for (int chnum = 0; chnum < RegisterType::CHANNELS; chnum++)
		m_channel[chnum]->save(device, chnum);
}


//-------------------------------------------------
//  reset - reset the overall state
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::reset()
{
	// reset all status bits
	set_reset_status(0, 0xff);

	// clear all registers
	std::fill_n(&m_regdata[0], m_regdata.size(), 0);

	// explicitly write to the mode register since it has side-effects
	// QUESTION: old cores initialize this to 0x30 -- who is right?
	write(RegisterType::REG_MODE, 0);

	// register type-specific initialization
	m_regs.reset();

	// reset the channels
	for (auto &chan : m_channel)
		chan->reset();
}


//-------------------------------------------------
//  clock - iterate over all channels, clocking
//  them forward one step
//-------------------------------------------------

template<class RegisterType>
u32 ymfm_engine_base<RegisterType>::clock(u8 chanmask)
{
	// increment the envelope count; low two bits are the subcount, which
	// only counts to 3, so if it reaches 3, count one more time
	m_env_counter++;
	if (BIT(m_env_counter, 0, 2) == 3)
		m_env_counter++;

	// clock the noise generator
	clock_noise();

	// clock the LFO
	s8 lfo_raw_pm = clock_lfo();

	// now update the state of all the channels and operators
	for (int chnum = 0; chnum < RegisterType::CHANNELS; chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->clock(m_env_counter, lfo_raw_pm, chnum == 2 && m_regs.multi_freq());

	// return the envelope counter as it is used to clock ADPCM-A
	return m_env_counter;
}


//-------------------------------------------------
//  output - compute a sum over the relevant
//  channels
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::output(s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax, u8 chanmask) const
{
	// sum over all the desired channels
	for (int chnum = 0; chnum < RegisterType::CHANNELS; chnum++)
		if (BIT(chanmask, chnum))
		{
			// noise must be non-zero to use noise on OP4, so if it is enabled,
			// OR with 2 (since only the LSB is actually checked for the noise state)
			u8 noise = (chnum == 7 && m_regs.noise_enabled()) ? (m_noise_state | 2) : 0;
			m_channel[chnum]->output(m_lfo_am, noise, lsum, rsum, rshift, clipmax);
		}
}


//-------------------------------------------------
//  write - handle writes to the OPN registers
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::write(u16 regnum, u8 data)
{
	// special case: writes to the mode register can impact IRQs;
	// schedule these writes to ensure ordering with timers
	if (regnum == RegisterType::REG_MODE)
	{
		m_device.machine().scheduler().synchronize(timer_expired_delegate(FUNC(ymfm_engine_base<RegisterType>::synced_mode_w), this), data);
		return;
	}

	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// handle writes to the keyon registers
	if (regnum == RegisterType::REG_KEYON)
	{
		u8 chnum = m_regs.keyon_channel();
		if (chnum < RegisterType::CHANNELS)
			m_channel[chnum]->keyonoff(m_regs.keyon_states());
	}
}


//-------------------------------------------------
//  status - return the current state of the
//  status flags
//-------------------------------------------------

template<class RegisterType>
u8 ymfm_engine_base<RegisterType>::status() const
{
	u8 result = m_status & ~STATUS_BUSY;

	// synthesize the busy flag if we're still busy
	if (m_device.machine().time() < m_busy_end)
		result |= STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  clock_lfo - clock the LFO, handling clock
//  division, depth, and waveform computations
//-------------------------------------------------

// OPM implementation
template<>
s8 ymfm_engine_base<ymopm_registers>::clock_lfo()
{
	// treat the rate as a 4.4 floating-point step value with implied
	// leading 1; this matches exactly the frequencies in the application
	// manual, though it might not be implemented exactly this way on chip
	u8 rate = m_regs.lfo_rate();
	u32 prev_counter = m_lfo_counter;
	m_lfo_counter += (0x10 | BIT(rate, 0, 4)) << BIT(rate, 4, 4);
	u8 lfo = BIT(m_lfo_counter, 22, 8);

	// compute the AM and PM values based on the waveform
	// AM is 8-bit unsigned; PM is 8-bit signed; waveforms are adjusted
	// to match the pictures in the application manual
	u8 am;
	s8 pm;
	switch (m_regs.lfo_waveform())
	{
		// sawtooth
		default:
		case 0:
			am = lfo ^ 0xff;
			pm = lfo;
			break;

		// square wave
		case 1:
			am = BIT(lfo, 7) ? 0 : 0xff;
			pm = am ^ 0x80;
			break;

		// triangle wave
		case 2:
			am = BIT(lfo, 7) ? (lfo << 1) : (~lfo << 1);
			pm = BIT(lfo, 6) ? am : ~am;
			break;

		// noise:
		case 3:
			// QUESTION: this behavior is surmised but not yet verified:
			// LFO noise value is accumulated over 8 bits of LFSR and
			// clocked as the LFO value transitions
			if (BIT(m_lfo_counter ^ prev_counter, 22, 8) != 0)
				m_noise_lfo = m_noise_lfsr & 0xff;
			am = m_noise_lfo;
			pm = am ^ 0x80;
			break;
	}

	// apply depth to the AM value and store for later
	m_lfo_am = (am * m_regs.lfo_am_depth()) >> 7;

	// apply depth to the PM value and return it
	return (pm * m_regs.lfo_pm_depth()) >> 7;
}

// OPN/OPNA implementation
template<class RegisterType>
s8 ymfm_engine_base<RegisterType>::clock_lfo()
{
	// if not enabled, quick exit with 0s
	if (!m_regs.lfo_enabled())
	{
		m_lfo_counter = 0;
		m_lfo_am = 0;
		return 0;
	}

	// this table is based on converting the frequencies in the applications
	// manual to clock dividers, based on the assumption of a 7-bit LFO value
	static u8 const lfo_max_count[8] = { 109, 78, 72, 68, 63, 45, 9, 6 };
	u8 subcount = u8(m_lfo_counter++);

	// when we cross the divider count, add enough to zero it and cause an
	// increment at bit 8; the 7-bit value lives from bits 8-14
	if (subcount >= lfo_max_count[m_regs.lfo_rate()])
		m_lfo_counter += subcount ^ 0xff;

	// AM value is 7 bits, staring at bit 8; grab the low 6 directly
	m_lfo_am = BIT(m_lfo_counter, 8, 6);

	// first half of the AM period (bit 6 == 0) is inverted
	if (BIT(m_lfo_counter, 8+6) == 0)
		m_lfo_am ^= 0x3f;

	// PM value is 5 bits, starting at bit 10; grab the low 3 directly
	s8 pm = BIT(m_lfo_counter, 10, 3);

	// PM is reflected based on bit 3
	if (BIT(m_lfo_counter, 10+3))
		pm ^= 7;

	// PM is negated based on bit 4
	return BIT(m_lfo_counter, 10+4) ? -pm : pm;
}


//-------------------------------------------------
//  clock_noise - clock the noise generator
//-------------------------------------------------

// OPM implementation
template<>
void ymfm_engine_base<ymopm_registers>::clock_noise()
{
	// base noise frequency is measured at 2x 1/2 FM frequency; this means
	// each tick counts as two steps against the noise counter
	u8 freq = m_regs.noise_frequency();
	for (int rep = 0; rep < 2; rep++)
	{
		// evidence seems to suggest the LFSR is clocked continually and just
		// sampled at the noise frequency for output purposes; clock it here
		// twice; note that the low 8 bits are the most recent 8 bits of history
		// while bits 8-24 contain the 17 bit LFSR state
		m_noise_lfsr >>= 1;
		m_noise_lfsr |= (BIT(m_noise_lfsr, 7) ^ BIT(m_noise_lfsr, 10) ^ 1) << 24;

		// compare against the frequency and latch when we exceed it
		if (m_noise_counter++ >= freq)
		{
			m_noise_counter = 0;
			m_noise_state = BIT(m_noise_lfsr, 7);
		}
	}
}

// OPN/OPNA implementation
template<class RegisterType>
void ymfm_engine_base<RegisterType>::clock_noise()
{
	// OPN does not have a noise generator
}


//-------------------------------------------------
//  update_timer - update the state of the given
//  timer
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::update_timer(u8 tnum, u8 enable)
{
	// if the timer is live, but not currently enabled, set the timer
	if (enable && !m_timer[tnum]->enabled())
	{
		// each timer clock is n channels * 4 operators * prescale factor (2/3/6)
		u32 clockscale = RegisterType::CHANNELS * 4 * m_clock_prescale;

		// period comes from the registers, and is different for each
		u32 period = (tnum == 0) ? (1024 - m_regs.timer_a_value()) : 16 * (256 - m_regs.timer_b_value());

		// reset it
		m_timer[tnum]->adjust(attotime::from_hz(m_device.clock()) * (period * clockscale), tnum);
	}

	// if the timer is not live, ensure it is not enabled
	else if (!enable)
		m_timer[tnum]->enable(false);
}


//-------------------------------------------------
//  timer_handler - timer has expired - signal
//  status and possibly IRQs
//-------------------------------------------------

template<class RegisterType>
TIMER_CALLBACK_MEMBER(ymfm_engine_base<RegisterType>::timer_handler)
{
	// update status
	if (param == 0 && m_regs.enable_timer_a())
		set_reset_status(STATUS_TIMERA, 0);
	else if (param == 1 && m_regs.enable_timer_b())
	 	set_reset_status(STATUS_TIMERB, 0);

	// if timer A fired in CSM mode, trigger CSM on all relevant channels
	if (param == 0 && m_regs.csm())
		for (int chnum = 0; chnum < RegisterType::CHANNELS; chnum++)
			if (BIT(RegisterType::CSM_TRIGGER_MASK, chnum))
				m_channel[chnum]->keyon_csm();

	// reset
	update_timer(param, 1);
}


//-------------------------------------------------
//  schedule_check_interrupts - schedule an
//  interrupt check via timer
//-------------------------------------------------

template<class RegisterType>
void ymfm_engine_base<RegisterType>::schedule_check_interrupts()
{
	// if we're currently executing a CPU, schedule the interrupt check;
	// otherwise, do it directly
	auto &scheduler = m_device.machine().scheduler();
	if (scheduler.currently_executing())
		scheduler.synchronize(timer_expired_delegate(FUNC(ymfm_engine_base<RegisterType>::check_interrupts), this), 0);
	else
		check_interrupts(nullptr, 0);
}


//-------------------------------------------------
//  check_interrupts - check the interrupt sources
//  for interrupts
//-------------------------------------------------

template<class RegisterType>
TIMER_CALLBACK_MEMBER(ymfm_engine_base<RegisterType>::check_interrupts)
{
	// update the state
	u8 old_state = m_irq_state;
	m_irq_state = ((m_status & m_irq_mask) != 0);

	// if changed, signal the new state
	if (old_state != m_irq_state && !m_irq_handler.isnull())
		m_irq_handler(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  synced_mode_w - handle a mode register write
//  via timer callback
//-------------------------------------------------

template<class RegisterType>
TIMER_CALLBACK_MEMBER(ymfm_engine_base<RegisterType>::synced_mode_w)
{
	// actually write the mode register now
	m_regs.write(RegisterType::REG_MODE, param);

	// reset timer status
	if (m_regs.reset_timer_b())
		set_reset_status(0, STATUS_TIMERB);
	if (m_regs.reset_timer_a())
		set_reset_status(0, STATUS_TIMERA);

	// load timers
	update_timer(1, m_regs.load_timer_b());
	update_timer(0, m_regs.load_timer_a());
}


// Explicit template instantiation
template class ymfm_engine_base<ymopm_registers>;
template class ymfm_engine_base<ymopn_registers>;
template class ymfm_engine_base<ymopna_registers>;
