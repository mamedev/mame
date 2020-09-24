// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "emu.h"
#include "ymopn.h"

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
// Test Bit Functions (YM2612)
// $21:0: Select which of two unknown signals is read as bit 14 of the test read output.
// $21:1: Some LFO control, unknown function.
// $21:2: Timers increment once every internal clock rather than once every sample. (Untested by me)
// $21:3: Freezes PG. Presumably disables writebacks to the phase register.
// $21:4: Ugly bit. Inverts MSB of operators.
// $21:5: Freezes EG. Presumably disables writebacks to the envelope counter register. Unknown whether this affects the other EG state bits.
// $21:6: Enable reading test data from OPN2 rather than status flags.
// $21:7: Select LSB (1) or MSB (0) of read test data. (Yes, it's backwards.)
// $2C:2 downto 0: Ignored by OPN2, confirmed by die shot.
// $2C:3: Bit 0 of Channel 6 DAC value
// $2C:4: Read 9-bit channel output (1) instead of 14-bit operator output (0)
// $2C:5: Play DAC output over all channels (possibly except for Channel 5--in my testing the DAC is the only thing you hear and it's much louder, you do not get any output from Channel 5; but someone else supposedly found // that the pan flags for Channel 5 don't affect the panning of this sound, which is only possible if it's not being output during that time slot for some reason. I don't have any other reason to believe this is true though).
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
//  lfo_pm_phase_adjustment - given the 7 most
//  significant frequency number bits, plus a 3-bit
//  PM depth value and a 5-bit phase counter index,
//  return a signed PM adjustment to the frequency;
//  algorithm written to match Nuked behavior
//-------------------------------------------------

inline s16 lfo_pm_phase_adjustment(u8 fnum_bits, u8 pm_depth, u8 pm_counter)
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

	// every 8 cycles the PM counter inverts phase
	if (BIT(pm_counter, 3))
		pm_counter ^= 7;

	// look up the relevant shifts
	u8 const shifts = s_lfo_pm_shifts[pm_depth][BIT(pm_counter, 0, 3)];

	// compute the adjustment
	s16 adjust = (fnum_bits >> BIT(shifts, 0, 4)) + (fnum_bits >> BIT(shifts, 4, 4));
	if (pm_depth > 5)
		adjust <<= pm_depth - 5;
	adjust >>= 2;

	// every 16 cycles it inverts sign
	return BIT(pm_counter, 4) ? -adjust : adjust;
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
//  block_fnum_to_keycode - given an 11-bit fnum
//  value concatenated with a 3-bit block (14 bits
//  total), compute the 5-bit keycode
//-------------------------------------------------

u8 block_fnum_to_keycode(u16 block_fnum)
{
	// upper 4 bits are 3 block bits and top FNUM bit
	u8 keycode = (block_fnum >> 9) & 0x1e;

	// lowest bit is determined by a mix of next lower FNUM bits
	// according to this equation from the YM2608 manual:
	//
	//   (F11 & (F10 | F9 | F8)) | (!F11 & F10 & F9 & F8)
	//
	// for speed, we just look it up in a 16-bit constant
	return keycode | BIT(0xfe80, BIT(block_fnum, 7, 4));
}



//*********************************************************
//  YMOPN_OPERATOR
//*********************************************************

//-------------------------------------------------
//  ymopn_operator - constructor
//-------------------------------------------------

template<class RegisterType>
ymopn_operator<RegisterType>::ymopn_operator(RegisterType regs) :
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

ALLOW_SAVE_TYPE(ymopn_operator<ymopn_registers>::envelope_state);
ALLOW_SAVE_TYPE(ymopn_operator<ymopna_registers>::envelope_state);

template<class RegisterType>
void ymopn_operator<RegisterType>::save(device_t &device, u8 index)
{
	// save our data
	device.save_item(YMOPN_NAME(m_phase), index);
	device.save_item(YMOPN_NAME(m_env_attenuation), index);
	device.save_item(YMOPN_NAME(m_env_state), index);
	device.save_item(YMOPN_NAME(m_ssg_inverted), index);
	device.save_item(YMOPN_NAME(m_key_state), index);
	device.save_item(YMOPN_NAME(m_keyon), index);
	device.save_item(YMOPN_NAME(m_csm_triggered), index);
}


//-------------------------------------------------
//  reset - reset the channel state
//-------------------------------------------------

template<class RegisterType>
void ymopn_operator<RegisterType>::reset()
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
void ymopn_operator<RegisterType>::clock(u32 env_counter, u8 lfo_counter, u8 pm_depth, u16 block_fnum)
{
	// clock the key state
	u8 keycode = block_fnum_to_keycode(block_fnum);
	clock_keystate(m_keyon | m_csm_triggered, keycode);
	m_csm_triggered = 0;

	// clock the SSG-EG state
	if (m_regs.ssg_eg_enabled())
		clock_ssg_eg_state(keycode);

	// clock the envelope if on an envelope cycle
	if (BIT(env_counter, 0, 2) == 0)
		clock_envelope(env_counter >> 2, keycode);

	// clock the phase
	clock_phase(lfo_counter, pm_depth, block_fnum);
}


//-------------------------------------------------
//  compute_volume - compute the 14-bit signed
//  volume of this operator, given a phase
//  modulation and an AM LFO offset
//-------------------------------------------------

template<class RegisterType>
s16 ymopn_operator<RegisterType>::compute_volume(u16 modulation, u8 am_offset) const
{
	// start with the upper 10 bits of the phase value plus modulation
	// the low 10 bits of this result represents a full 2*PI period over
	// the full sin wave
	u16 phase = (m_phase >> 10) + modulation;

	// get the absolute value of the sin, as attenuation, as a 4.8 fixed point value
	u16 sin_attenuation = abs_sin_attenuation(phase);

	// get the attenuation from the evelope egnerator as a 4.6 value, shifted up to 4.8
	u16 env_attenuation = envelope_attenuation(am_offset) << 2;

	// combine into a 5.8 value, then convert from attenuation to 13-bit linear volume
	s16 result = attenuation_to_volume(sin_attenuation + env_attenuation);

	// negate if in the negative part of the sin wave (sign bit gives 14 bits)
	return BIT(phase, 9) ? -result : result;
}


//-------------------------------------------------
//  effective_rate - return the effective 6-bit
//  rate value after adjusting for keycode
//-------------------------------------------------

template<class RegisterType>
u8 ymopn_operator<RegisterType>::effective_rate(u8 rawrate, u8 keycode)
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
void ymopn_operator<RegisterType>::start_attack(u8 keycode)
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

printf("KeyOn %d.%d: fnum=%04X fb=%d alg=%d dt=%d mul=%X tl=%02X ksr=%d ssg=%X adsr=%02X/%02X/%02X/%X sl=%X pan=%d%d am=%d pm=%d\n",
	BIT(m_regs.opbase(), 0, 2) + (BIT(m_regs.opbase(), 8) * 3), BIT(m_regs.opbase(), 3) + (BIT(m_regs.opbase(), 2) << 1),
	m_regs.block_fnum(),
	m_regs.feedback(),
	m_regs.algorithm(),
	m_regs.detune(),
	m_regs.multiple(),
	m_regs.total_level(),
	m_regs.ksr(),
	m_regs.ssg_eg_enabled() * 8 + m_regs.ssg_eg_mode(),
	m_regs.attack_rate(),
	m_regs.decay_rate(),
	m_regs.sustain_rate(),
	m_regs.release_rate(),
	m_regs.sustain_level(),
	m_regs.pan_left(),
	m_regs.pan_right(),
	(m_regs.lfo_am_enable() && m_regs.lfo_enable()) ? m_regs.am_shift() : 0,
	m_regs.lfo_enable() ? m_regs.pm_depth() : 0);
}


//-------------------------------------------------
//  start_release - start the release phase;
//  called when a keyoff happens
//-------------------------------------------------

template<class RegisterType>
void ymopn_operator<RegisterType>::start_release()
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
void ymopn_operator<RegisterType>::clock_keystate(u8 keystate, u8 keycode)
{
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
void ymopn_operator<RegisterType>::clock_ssg_eg_state(u8 keycode)
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
void ymopn_operator<RegisterType>::clock_envelope(u16 env_counter, u8 keycode)
{
	// if in attack state, see if we hit minimum attenuation
	if (m_env_state == ENV_ATTACK && m_env_attenuation == 0)
		m_env_state = ENV_DECAY;

	// if in decay state, see if we hit the sustain level
	else if (m_env_state == ENV_DECAY)
	{
		// 4-bit sustain level, but 15 means 31 so effectively 5 bits
		u8 target = m_regs.sustain_level();
		if (target == 15)
			target = 31;

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
	if ((env_counter & 0x7ff) != 0)
		return;

	// determine the increment based on the non-fractional part of env_counter
	u8 increment = attenuation_increment(rate, (env_counter >> 11) & 7);

	// attack is the only one that increases
	if (m_env_state == ENV_ATTACK)
	{
		// glitch means that attack rates of 62/63 don't increment if
		// changed after the initial key on (where they are handled
		// specially)

		// QUESTION: this check affects one of the operators on the gng credit sound
		//   is it correct? Removing it for now
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
//  clock_phase - clock the 10.10 phase value; this
//  logic has been verified against the Nuked phase
//  generator
//-------------------------------------------------

template<class RegisterType>
void ymopn_operator<RegisterType>::clock_phase(u8 lfo_counter, u8 pm_depth, u16 block_fnum)
{
	// extract frequency number and adjust by LFO
	u16 fnum = BIT(block_fnum, 0, 11) << 1;
	if (pm_depth != 0)
	{
		// apply the phase adjustment based on the upper 7 bits
		// of FNUM and the PM depth parameters
		fnum += lfo_pm_phase_adjustment(BIT(block_fnum, 4, 7), pm_depth, lfo_counter >> 2);

		// keep fnum to 12 bits
		fnum &= 0xfff;
	}

	// apply block shift to compute phase step
	u8 block = BIT(block_fnum, 11, 3);
	u32 phase_step = (fnum << block) >> 2;

	// apply detune based on the keycode
	phase_step += detune_adjustment(m_regs.detune(), block_fnum_to_keycode(block_fnum));

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
u16 ymopn_operator<RegisterType>::envelope_attenuation(u8 am_offset) const
{
	u16 result = m_env_attenuation;

	// invert if necessary due to SSG-EG
	if (m_ssg_inverted)
		result = (0x200 - result) & 0x3ff;

	// add in LFO AM modulation
	if (m_regs.lfo_am_enable())
		result += am_offset;

	// add in total level
	result += m_regs.total_level() << 3;

	// clamp to max and return
	return (result < 0x400) ? result : 0x3ff;
}



//*********************************************************
//  YMOPN_CHANNEL
//*********************************************************

//-------------------------------------------------
//  ymopn_channel - constructor
//-------------------------------------------------

template<class RegisterType>
ymopn_channel<RegisterType>::ymopn_channel(RegisterType regs) :
	m_feedback{ 0, 0 },
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
void ymopn_channel<RegisterType>::save(device_t &device, u8 index)
{
	// save our data
	device.save_item(YMOPN_NAME(m_feedback), index);

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
void ymopn_channel<RegisterType>::reset()
{
	// reset our data
	m_feedback[0] = m_feedback[1] = 0;

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
void ymopn_channel<RegisterType>::keyonoff(u8 states)
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
void ymopn_channel<RegisterType>::keyon_csm()
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
void ymopn_channel<RegisterType>::clock(u32 env_counter, u8 lfo_counter, bool multi_freq)
{
	// grab common PM depth and block/fnum values
	u16 block_fnum = m_regs.block_fnum();
	u8 pm_depth = m_regs.pm_depth();

	// clock the feedback through
	m_feedback[0] = m_feedback[1];
	m_feedback[1] = m_feedback[2];

	// in multi-frequency mode, the first 3 channels use independent block/fnum values
	if (multi_freq)
	{
		m_op1.clock(env_counter, lfo_counter, pm_depth, m_regs.multi_block_fnum(1));
		m_op2.clock(env_counter, lfo_counter, pm_depth, m_regs.multi_block_fnum(2));
		m_op3.clock(env_counter, lfo_counter, pm_depth, m_regs.multi_block_fnum(0));
	}

	// otherwise, all channels use the common block/fnum
	else
	{
		m_op1.clock(env_counter, lfo_counter, pm_depth, block_fnum);
		m_op2.clock(env_counter, lfo_counter, pm_depth, block_fnum);
		m_op3.clock(env_counter, lfo_counter, pm_depth, block_fnum);
	}

	// operator 3 uses the common values in all cases
	m_op4.clock(env_counter, lfo_counter, pm_depth, block_fnum);
}


//-------------------------------------------------
//  output - combine the operators according to the
//  specified algorithm, returning a sum according
//  to the rshift and clipmax parameters, which
//  vary between different OPN implementations
//-------------------------------------------------

template<class RegisterType>
void ymopn_channel<RegisterType>::output(u8 lfo_counter, s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax) const
{
	// skip if both pans are clear; no need to do all this work for nothing
	if (m_regs.pan_left() == 0 && m_regs.pan_right() == 0)
		return;

	// AM modulation amount is the same across all operators; compute it once
	u8 am_shift = (1 << (m_regs.am_shift() ^ 3)) - 1;
	u8 am_value = lfo_counter & 0x3f;
	if (BIT(lfo_counter, 6))
		am_value ^= 0x3f;
	u8 am_offset = (am_value << 1) >> am_shift;

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
	opout[1] = m_feedback[2] = m_op1.compute_volume(modulation, am_offset);

	// compute the 14-bit volume/value of operator 2
	opout[2] = m_op2.compute_volume(opout[BIT(algorithm_inputs, 0, 1)] >> 1, am_offset);
	opout[5] = opout[1] + opout[2];

	// compute the 14-bit volume/value of operator 3
	opout[3] = m_op3.compute_volume(opout[BIT(algorithm_inputs, 1, 3)] >> 1, am_offset);
	opout[6] = opout[1] + opout[3];
	opout[7] = opout[2] + opout[3];

	// compute the 14-bit volume/value of operator 4
	opout[4] = m_op4.compute_volume(opout[BIT(algorithm_inputs, 4, 3)] >> 1, am_offset);

	// all algorithms consume OP4 output
	s16 result = opout[4] >> rshift;

	// algorithms 4-7 add in OP2 output
	if (algorithm >= 4)
	{
		s16 clipmin = -clipmax - 1;
		result += opout[2] >> rshift;
		result = std::max(std::min(result, clipmax), clipmin);

		// agorithms 5-7 add in OP3 output
		if (algorithm >= 5)
		{
			result += opout[3] >> rshift;
			result = std::max(std::min(result, clipmax), clipmin);

			// algorithm 7 adds in OP1 output
			if (algorithm == 7)
			{
				result += opout[1] >> rshift;
				result = std::max(std::min(result, clipmax), clipmin);
			}
		}
	}

	// add to the output
	if (m_regs.pan_left())
		lsum += result;
	if (m_regs.pan_right())
		rsum += result;
}



//*********************************************************
//  YMOPN_ENGINE
//*********************************************************

//-------------------------------------------------
//  ymopn_engine - constructor
//-------------------------------------------------

template<class RegisterType>
ymopn_engine_base<RegisterType>::ymopn_engine_base(device_t &device) :
	m_device(device),
	m_env_counter(0),
	m_lfo_subcounter(0),
	m_lfo_counter(0),
	m_status(0),
	m_clock_prescale(6),
	m_irq_mask(STATUS_TIMERA | STATUS_TIMERB),
	m_irq_state(0),
	m_busy_end(attotime::zero),
	m_timer{ nullptr, nullptr },
	m_irq_handler(device),
	m_regdata(RegisterType::registers()),
	m_fnum_latches{ 0 },
	m_regs(m_regdata)
{
	// create the channels
	for (int chnum = 0; chnum < RegisterType::channels(); chnum++)
		m_channel.push_back(std::make_unique<ymopn_channel<RegisterType>>(m_regs.channel_registers(chnum)));
}


//-------------------------------------------------
//  save - register for save states
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::save(device_t &device)
{
	// allocate our timers
	for (int tnum = 0; tnum < 2; tnum++)
		m_timer[tnum] = device.machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ymopn_engine_base::timer_handler), this));

	// resolve the IRQ handler while we're here
	m_irq_handler.resolve();

	// save our data
	device.save_item(YMOPN_NAME(m_env_counter));
	device.save_item(YMOPN_NAME(m_lfo_subcounter));
	device.save_item(YMOPN_NAME(m_lfo_counter));
	device.save_item(YMOPN_NAME(m_status));
	device.save_item(YMOPN_NAME(m_clock_prescale));
	device.save_item(YMOPN_NAME(m_irq_mask));
	device.save_item(YMOPN_NAME(m_irq_state));
	device.save_item(YMOPN_NAME(m_busy_end));
	device.save_item(YMOPN_NAME(m_fnum_latches));
	device.save_item(YMOPN_NAME(m_regdata));

	// save channel data
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		m_channel[chnum]->save(device, chnum);
}


//-------------------------------------------------
//  reset - reset the overall state
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::reset()
{
	// reset all status bits
	set_reset_status(0, 0xff);

	// clear all registers
	std::fill_n(&m_regdata[0], m_regdata.size(), 0);

	// enable left/right on all channels by default
	m_regdata[0xb4] = m_regdata[0xb5] = m_regdata[0xb6] = 0xc0;
	if (m_regdata.size() >= 0x100)
		m_regdata[0x1b4] = m_regdata[0x1b5] = m_regdata[0x1b6] = 0xc0;

	// explicitly write to the mode register since it has side-effects
	// QUESTION: old cores initialize this to 0x30 -- who is right?
	write(0x27, 0);

	// reset the channels
	for (auto &chan : m_channel)
		chan->reset();
}


//-------------------------------------------------
//  clock - iterate over all channels, clocking
//  them forward one step
//-------------------------------------------------

template<class RegisterType>
u32 ymopn_engine_base<RegisterType>::clock(u8 chanmask)
{
	// increment the envelope count; low two bits are the subcount, which
	// only counts to 3, so if it reaches 3, count one more time
	m_env_counter++;
	if (BIT(m_env_counter, 0, 2) == 3)
		m_env_counter++;

	// now update the state of all the channels and operators
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->clock(m_env_counter, m_lfo_counter, chnum == 2 && m_regs.csm_multi_freq() != 0);

	// QUESTION: seems weird to update LFO AFTER the channel updates (where it is
	// consumed for PM), but BEFORE the output calculcations (where it is
	// consumed for AM), but this is what Nemesis does, so we'll do the
	// same for consistency
	if (m_regs.lfo_enable())
	{
		static u8 const lfo_max_count[8] = { 108, 77, 71, 67, 62, 44, 8, 5 };
		if (m_lfo_subcounter++ >= lfo_max_count[m_regs.lfo_rate()])
		{
			m_lfo_subcounter = 0;
			m_lfo_counter++;
		}
	}
	else
		m_lfo_counter = m_lfo_subcounter = 0;

	// return the envelope counter as it is used to clock ADPCM-A
	return m_env_counter;
}


//-------------------------------------------------
//  output - compute a sum over the relevant
//  channels
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::output(s32 &lsum, s32 &rsum, u8 rshift, s16 clipmax, u8 chanmask) const
{
	// sum over all the desired channels
	for (int chnum = 0; chnum < m_channel.size(); chnum++)
		if (BIT(chanmask, chnum))
			m_channel[chnum]->output(m_lfo_counter, lsum, rsum, rshift, clipmax);
}


//-------------------------------------------------
//  write - handle writes to the OPN registers
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::write(u16 regnum, u8 data)
{
	// writes in the 0xa0-af/0x1a0-af region are handled as latched pairs
	if ((regnum & 0xf0) == 0xa0)
	{
		u8 latchindex = (BIT(regnum, 8) << 3) | (BIT(regnum, 3) << 2) | BIT(regnum, 0, 2);

		// writes to the upper half just latch
		if (BIT(regnum, 2))
			m_fnum_latches[latchindex] = data | 0x80;

		// writes to the lower half only commit if the latch is there
		else if (BIT(m_fnum_latches[latchindex], 7))
		{
			m_regs.write(regnum, data);
			m_regs.write(regnum | 4, m_fnum_latches[latchindex] & 0x3f);
			m_fnum_latches[latchindex] = 0;
		}
		return;
	}

	// for remaining registers, store the raw value to the register array;
	// most writes are passive, consumed only when needed
	m_regs.write(regnum, data);

	// handle writes to the mode register
	if (regnum == 0x27)
	{
		// reset timer status
		if (m_regs.reset_timer_b())
			set_reset_status(0, STATUS_TIMERB);
		if (m_regs.reset_timer_a())
			set_reset_status(0, STATUS_TIMERA);

		// load timers
		update_timer(1, m_regs.load_timer_b());
		update_timer(0, m_regs.load_timer_a());
	}

	// handle writes to the keyon registers
	else if (regnum == 0x28)
	{
		u8 chnum = m_regs.keyon_channel();
		if (chnum == 3)
			return;
		if (m_channel.size() > 3)
			chnum += 3 * m_regs.keyon_channel2();
		m_channel[chnum]->keyonoff(m_regs.keyon_states());
	}
}


//-------------------------------------------------
//  status - return the current state of the
//  status flags
//-------------------------------------------------

template<class RegisterType>
u8 ymopn_engine_base<RegisterType>::status() const
{
	u8 result = m_status;

	// synthesize the busy flag if we're still busy
	if (m_device.machine().time() < m_busy_end)
		result |= STATUS_BUSY;
	return result;
}


//-------------------------------------------------
//  set_busy - set the busy flag in the status
//  register
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::set_busy()
{
	// according to hardware tests, BUSY is set for 32 * prescaled clock
	// regardless of actual data consumption
	m_busy_end = m_device.machine().time() + attotime::from_hz(m_device.clock()) * (32 * m_clock_prescale);
}


//-------------------------------------------------
//  update_timer - update the state of the given
//  timer
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::update_timer(u8 tnum, u8 enable)
{
	// if the timer is live, but not currently enabled, set the timer
	if (enable && !m_timer[tnum]->enabled())
	{
		// each timer clock is n channels * 4 operators * prescale factor (2/3/6)
		u32 clockscale = m_channel.size() * 4 * m_clock_prescale;

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
TIMER_CALLBACK_MEMBER(ymopn_engine_base<RegisterType>::timer_handler)
{
	// update status
	if (param == 0 && m_regs.enable_timer_a())
		set_reset_status(STATUS_TIMERA, 0);
	else if (param == 1 && m_regs.enable_timer_b())
	 	set_reset_status(STATUS_TIMERB, 0);

	// if timer A fired in CSM mode, signal it
	if (param == 0 && m_regs.csm_multi_freq() == 2)
		m_channel[2]->keyon_csm();

	// reset
	update_timer(param, 1);
}


//-------------------------------------------------
//  check_interrupts - check the interrupt sources
//  for interrupts
//-------------------------------------------------

template<class RegisterType>
void ymopn_engine_base<RegisterType>::check_interrupts()
{
	// update the state
	u8 old_state = m_irq_state;
	m_irq_state = ((m_status & m_irq_mask) != 0);

	// if changed, signal the new state
	if (old_state != m_irq_state && !m_irq_handler.isnull())
		m_irq_handler(m_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


// Explicit template instantiation
template class ymopn_engine_base<ymopn_registers>;
template class ymopn_engine_base<ymopna_registers>;

