// license:BSD-3-Clause
// copyright-holders:superctr
/*
    Yamaha YMF271-F "OPX" emulator

    Written from the datasheet and the application manual.  Where the
    documents leave the behaviour open, the model follows recordings of real
    hardware (no chip was available for direct probing).

    Architecture (from the datasheet / application manual):
      - 48 slots in 12 groups of 4; slot n = 12*bank + group, where bank is the
        register bank (S1..S4) and the slot number is also the order in which the
        chip evaluates operators inside one 44.1 kHz sample period.
      - Every slot runs the same operator pipeline (PG -> oscillator -> EG -> OP).
        The oscillator reads one of 7 internal waveforms (log-sin derived) or, for
        the 12 slots of groups 0/4/8 only (n % 4 == 0), external PCM data.
      - The per-group sync register selects how the 4 slots are connected
        (4op / 2x2op / 3op+PCM / 4xPCM), which slots receive broadcast writes,
        and which slot(s) act as key-on slot.

    Model:
      - Register model with sync broadcast, F-number latch, status / End flags,
        timers, external memory window.
      - OPM/OPZ-style log-sin/exp operator
      - Per-slot LFO: clock-divider rate table 2-6-2, saw / square / triangle,
        PMS depth fnum*k/1024 (table 2-6-3), AMS 63/126/252 units, phase reset
        at key-on; AM starts at full attenuation for every waveform (OPM convention).
      - AccOn = the slot output is accumulated in a saturating 14-bit sum, so
        any sustained tone rails into a full-level square that flips at the
        operator's zero crossings ("distorted" basses and drums).
      - External PCM: FM PG with the implicit fnum bit 11, Fs divider, 8-bit and
        packed 12-bit words, linear interpolation, looping, End flags, external
        key code (manual 2-9), envelope multiply, same pan as FM.

    Open points (need to be verified on real HW):
      - Waveforms 1-6 unverified on HW recordings and based on guesswork.
      - Key code: octave = Block for blocks 0..7, assumed to clamp at 0 for negative
        blocks (the manual's formula wraps to the top rows instead).
      - wave 7 on slots other than 0,4,..,44 assumed to be silent.

    Not implemented (decoded, unused by the available VGMs):
      - PFM (utility 0x0n bit 7)
      - PCM alternate loop (A/L), probably bidirectional loop but no games use it.
      - Timer A register split (0x10 high 8 / 0x11 low 2 bits) and the
        external-memory write address increment follow the previous core.
*/

#include "emu.h"
#include "ymf271.h"

#include <cmath>
#include <numbers>


// ------------------------------------------------------------------------
// constants and tables
// ------------------------------------------------------------------------

namespace {

// register address low nibble -> group (FM banks, utility) / slot (PCM bank).
// Nibbles 3/7/B/F are invalid.
const int8_t fm_tab[16]  = { 0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1 };
const int8_t pcm_tab[16] = { 0, 4, 8, -1, 12, 16, 20, -1, 24, 28, 32, -1, 36, 40, 44, -1 };

// sync modes (utility reg 0x0n bits 1:0)
enum
{
	SYNC_4OP = 0,       // S1-S2-S3-S4, key-on slot S1
	SYNC_2X2OP = 1,     // S1-S3 and S2-S4, key-on slots S1, S2
	SYNC_3OP_PCM = 2,   // S1-S2-S3 FM + S4 PCM, key-on slots S1,S4
	SYNC_PCM = 3        // 4 independent slots
};

enum
{
	EG_ATTACK = 0,
	EG_DECAY1,
	EG_DECAY2,
	EG_RELEASE,
	EG_OFF
};

// Wave 6: the MUL-scaled modulation input wraps at 2^W6_BITS (0 = continuous).
constexpr int W6_BITS = 9;

// EG increment patterns (ymfm/OPM): nibble k of entry r = increment for
// EG sub-step k at rate r.
const uint32_t eg_inc[64] =
{
	0x00000000, 0x00000000, 0x10101010, 0x10101010,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x10101010, 0x10111010, 0x11101110, 0x11111110,
	0x11111111, 0x21112111, 0x21212121, 0x22212221,
	0x22222222, 0x42224222, 0x42424242, 0x44424442,
	0x44444444, 0x84448444, 0x84848484, 0x88848884,
	0x88888888, 0x88888888, 0x88888888, 0x88888888
};

// Rate key scaling (manual table 2-6-7), [keycode][KS]
const uint8_t rks_tab[32][8] =
{
	{  0,  0,  0,  0,  0,  2,  4,  8 }, {  0,  0,  0,  0,  1,  3,  5,  9 },
	{  0,  0,  0,  1,  2,  4,  6, 10 }, {  0,  0,  0,  1,  3,  5,  7, 11 },
	{  0,  0,  1,  2,  4,  6,  8, 12 }, {  0,  0,  1,  2,  5,  7,  9, 13 },
	{  0,  0,  1,  3,  6,  8, 10, 14 }, {  0,  0,  1,  3,  7,  9, 11, 15 },
	{  0,  1,  2,  4,  8, 10, 12, 16 }, {  0,  1,  2,  4,  9, 11, 13, 17 },
	{  0,  1,  2,  5, 10, 12, 14, 18 }, {  0,  1,  2,  5, 11, 13, 15, 19 },
	{  0,  1,  3,  6, 12, 14, 16, 20 }, {  0,  1,  3,  6, 13, 15, 17, 21 },
	{  0,  1,  3,  7, 14, 16, 18, 22 }, {  0,  1,  3,  7, 15, 17, 19, 23 },
	{  0,  2,  4,  8, 16, 18, 20, 24 }, {  0,  2,  4,  8, 17, 19, 21, 25 },
	{  0,  2,  4,  9, 18, 20, 22, 26 }, {  0,  2,  4,  9, 19, 21, 23, 27 },
	{  0,  2,  5, 10, 20, 22, 24, 28 }, {  0,  2,  5, 10, 21, 23, 25, 29 },
	{  0,  2,  5, 11, 22, 24, 26, 30 }, {  0,  2,  5, 11, 23, 25, 27, 31 },
	{  0,  3,  6, 12, 24, 26, 28, 31 }, {  0,  3,  6, 12, 25, 27, 29, 31 },
	{  0,  3,  6, 13, 26, 28, 30, 31 }, {  0,  3,  6, 13, 27, 29, 31, 31 },
	{  0,  3,  7, 14, 28, 30, 31, 31 }, {  0,  3,  7, 14, 29, 31, 31, 31 },
	{  0,  3,  7, 15, 30, 31, 31, 31 }, {  0,  3,  7, 15, 31, 31, 31, 31 }
};

// Detune, [keycode][DT&3], in units of fs/2^20 Hz (= 1 LSB of a 20-bit phase
// increment).  This is the OPM DT1 table; the manual's table 2-6-5 (cents
// column) is exactly this table at fs = 44.1 kHz.
const uint8_t detune_tab[32][4] =
{
	{ 0, 0, 1, 2 }, { 0, 0, 1, 2 }, { 0, 0, 1, 2 }, { 0, 0, 1, 2 },
	{ 0, 1, 2, 2 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 }, { 0, 1, 2, 3 },
	{ 0, 1, 2, 4 }, { 0, 1, 3, 4 }, { 0, 1, 3, 4 }, { 0, 1, 3, 5 },
	{ 0, 2, 4, 5 }, { 0, 2, 4, 6 }, { 0, 2, 4, 6 }, { 0, 2, 5, 7 },
	{ 0, 2, 5, 8 }, { 0, 3, 6, 8 }, { 0, 3, 6, 9 }, { 0, 3, 7, 10 },
	{ 0, 4, 8, 11 }, { 0, 4, 8, 12 }, { 0, 4, 9, 13 }, { 0, 5, 10, 14 },
	{ 0, 5, 11, 16 }, { 0, 6, 12, 17 }, { 0, 6, 13, 19 }, { 0, 7, 14, 20 },
	{ 0, 8, 16, 22 }, { 0, 8, 16, 22 }, { 0, 8, 16, 22 }, { 0, 8, 16, 22 }
};

// Modulation level (FB register of a modulated slot), manual values:
// 0 = 16 pi, 1 = 8 pi, 2 = 4 pi, 3 = 2 pi, 4 = pi, 5 = 32 pi, 6 = 64 pi, 7 = 128 pi.
// Applied as (14-bit modulator sum * table) >> 8 on the 10-bit phase: level 0
// = 8192 * 128 / 256 = 4096 phase units = 4 cycles = +/-8 pi peak, i.e. half
// the manual's figure (read as the peak-to-peak swing) and exactly OPM's
// fixed depth.
const uint16_t modlevel[8] = { 128, 64, 32, 16, 8, 256, 512, 1024 };

// LFO PM depth (manual table 2-6-3): max deviation = fnum * k / 1024,
// k = 0, 2, 3, 4, 6, 12, 24, 48 (3.4 .. 79.3 cents).
const uint8_t pms_k[8] = { 0, 2, 3, 4, 6, 12, 24, 48 };

} // anonymous namespace

// Algorithm connection tables.  For each operator position of the voice:
// mods = bitmask of positions whose output modulates it; car = bitmask of
// carrier positions; fbsrc = position whose output is fed back into position
// 0 (0 = self, 2 = the S3 loop of algorithms 1/5/7/11).  Positions: 4op
// S1,S2,S3,S4 = 0..3; 2op head,tail = 0,1; 3op S1,S2,S3 = 0..2.
static const ymf271_device::opx_alg alg4[16] =
{
	{ { 0, 0x4, 0x1, 0x2 }, 0x8, 0 },   // 0: S1>S3>S2>S4
	{ { 0, 0x4, 0x1, 0x2 }, 0x8, 2 },   // 1: same, fb loop S1>S3
	{ { 0, 0x5, 0x0, 0x2 }, 0x8, 0 },   // 2: (S1+S3)>S2>S4
	{ { 0, 0x4, 0x0, 0x3 }, 0x8, 0 },   // 3: (S1 + S3>S2)>S4
	{ { 0, 0x0, 0x1, 0x6 }, 0x8, 0 },   // 4: (S1>S3 + S2)>S4
	{ { 0, 0x0, 0x1, 0x6 }, 0x8, 2 },   // 5: same, fb loop S1>S3
	{ { 0, 0x0, 0x1, 0x2 }, 0xC, 0 },   // 6: S1>S3, S2>S4
	{ { 0, 0x0, 0x1, 0x2 }, 0xC, 2 },   // 7: same, fb loop S1>S3
	{ { 0, 0x4, 0x0, 0x2 }, 0x9, 0 },   // 8: S1, S3>S2>S4
	{ { 0, 0x0, 0x0, 0x6 }, 0x9, 0 },   // 9: S1, (S3+S2)>S4
	{ { 0, 0x0, 0x1, 0x0 }, 0xE, 0 },   // 10: S1>S3, S2, S4
	{ { 0, 0x0, 0x1, 0x0 }, 0xE, 2 },   // 11: same, fb loop S1>S3
	{ { 0, 0x1, 0x1, 0x1 }, 0xE, 0 },   // 12: S1>(S2,S3,S4)
	{ { 0, 0x4, 0x0, 0x0 }, 0xB, 0 },   // 13: S1, S3>S2, S4
	{ { 0, 0x0, 0x1, 0x2 }, 0xD, 0 },   // 14: S1, S1>S3, S2>S4
	{ { 0, 0x0, 0x0, 0x0 }, 0xF, 0 }    // 15: all carriers
};

static const ymf271_device::opx_alg alg2[4] =
{
	{ { 0, 0x1, 0, 0 }, 0x2, 0 },   // 0: A>B
	{ { 0, 0x1, 0, 0 }, 0x2, 1 },   // 1: A>B, fb loop
	{ { 0, 0x0, 0, 0 }, 0x3, 0 },   // 2: A, B
	{ { 0, 0x1, 0, 0 }, 0x3, 0 }    // 3: A, A>B
};

static const ymf271_device::opx_alg alg3[8] =
{
	{ { 0, 0x4, 0x1, 0 }, 0x2, 0 }, // 0: S1>S3>S2
	{ { 0, 0x4, 0x1, 0 }, 0x2, 2 }, // 1: same, fb loop
	{ { 0, 0x5, 0x0, 0 }, 0x2, 0 }, // 2: (S1+S3)>S2
	{ { 0, 0x4, 0x0, 0 }, 0x3, 0 }, // 3: S1, S3>S2
	{ { 0, 0x0, 0x1, 0 }, 0x6, 0 }, // 4: S1>S3, S2
	{ { 0, 0x0, 0x1, 0 }, 0x6, 2 }, // 5: same, fb loop
	{ { 0, 0x0, 0x0, 0 }, 0x7, 0 }, // 6: S1, S2, S3
	{ { 0, 0x0, 0x1, 0 }, 0x7, 0 }  // 7: S1, S1>S3, S2
};

static const ymf271_device::opx_alg alg_single = { { 0, 0, 0, 0 }, 0x1, 0 };


// ------------------------------------------------------------------------
// helpers
// ------------------------------------------------------------------------

// Is slot (bank, group) a key-on slot under the group's sync mode?
bool ymf271_device::is_keyon_slot(int bank, int group) const
{
	switch (m_groups[group].sync)
	{
	case SYNC_4OP:      return bank == 0;
	case SYNC_2X2OP:    return bank == 0 || bank == 1;
	case SYNC_3OP_PCM:  return bank == 0 || bank == 3;
	default:            return true;
	}
}

// Fill 'slots' with the slot numbers that form the voice keyed by (bank, group).
// Returns the count (0 if not a key-on slot).
int ymf271_device::voice_slots(int bank, int group, int *slots) const
{
	switch (m_groups[group].sync)
	{
	case SYNC_4OP:
		if (bank != 0) return 0;
		slots[0] = group; slots[1] = group + 12; slots[2] = group + 24; slots[3] = group + 36;
		return 4;
	case SYNC_2X2OP:
		if (bank == 0) { slots[0] = group; slots[1] = group + 24; return 2; }
		if (bank == 1) { slots[0] = group + 12; slots[1] = group + 36; return 2; }
		return 0;
	case SYNC_3OP_PCM:
		if (bank == 0) { slots[0] = group; slots[1] = group + 12; slots[2] = group + 24; return 3; }
		if (bank == 3) { slots[0] = group + 36; return 1; }
		return 0;
	default:
		slots[0] = 12 * bank + group;
		return 1;
	}
}


// ------------------------------------------------------------------------
// key on / off
// ------------------------------------------------------------------------

// Key code (manual 2-9): 5 bits, octave = the *bottom 3 bits* of Block (so the
// negative blocks wrap to the top octaves) + N4N3 from the F-number.
// internal waveform: KC = 4 Block + N4N3, fnum thresholds 0x780/0x900/0xA80;
// external waveform: KC = (4 SrcB + SrcNote) + (4 Block + N4N3), 11-bit fnum
// thresholds 0x100/0x300/0x500 (table 2-9-3); the 5-bit sum wraps, which gives
// the arithmetically right result for negative blocks.
void ymf271_device::update_keycode(opx_slot &s, int slotnum)
{
	int n43, kc;

	if (s.wave == 7 && (slotnum & 3) == 0)
	{
		uint16_t fn = s.fnum & 0x7FF;
		if (fn < 0x100)      n43 = 0;
		else if (fn < 0x300) n43 = 1;
		else if (fn < 0x500) n43 = 2;
		else                 n43 = 3;
		kc = s.pcm_srcb * 4 + s.pcm_srcnote + (s.block & 7) * 4 + n43;
	}
	else
	{
		if (s.fnum < 0x780)      n43 = 0;
		else if (s.fnum < 0x900) n43 = 1;
		else if (s.fnum < 0xA80) n43 = 2;
		else                     n43 = 3;
		// negative blocks: the manual's 4*Block would wrap to octave 7, but
		// a recorded bass at block -1 (DT 7 / DT 2 on a 1:3 operator pair)
		// stays phase-locked within ~0.5 %, i.e. the detune of the lowest
		// key codes -- so the octave clamps at 0.
		kc = ((s.block_s < 0) ? 0 : (s.block & 7) * 4) + n43;
	}
	s.keycode = uint8_t(kc & 31);
}

int ymf271_device::eg_rate(int rate2, int rks)
{
	// rate2 = 2*AR/D1R/D2R or 4*RR; rate 0 stays 0 (infinite)
	if (rate2 == 0)
		return 0;
	rate2 += rks;
	return (rate2 > 63) ? 63 : rate2;
}

void ymf271_device::slot_keyon(int slotnum)
{
	opx_slot &s = m_slots[slotnum];

	s.eg_state = EG_ATTACK;
	s.phase = 0;
	// The attenuation continues from its current value (OPM); a maximum
	// attack rate jumps straight to 0 dB (table 2-6-8: rate 63 = 0.07 ms).
	if (eg_rate(s.ar * 2, rks_tab[s.keycode][s.ks]) >= 63)
		s.eg_att = 0;
	s.lfo_cnt = 0;
	s.lfo_pos = 0;
	s.pcm_pos = 0;
	s.pcm_frac = 0;
	s.pcm_ended = 0;
	s.acc = 0;
	if ((slotnum & 3) == 0)
		m_end_status &= ~(1 << (slotnum >> 2));
}

void ymf271_device::slot_keyoff(int slotnum)
{
	opx_slot &s = m_slots[slotnum];

	if (s.eg_state != EG_OFF)
		s.eg_state = EG_RELEASE;
}


// ------------------------------------------------------------------------
// register writes
// ------------------------------------------------------------------------

// write one decoded function register to one slot
void ymf271_device::write_slot_reg(int slotnum, int reg, uint8_t data)
{
	opx_slot &s = m_slots[slotnum];

	switch (reg)
	{
	case 0x0:
		s.ext_en = (data >> 7) & 1;
		s.ext_out = (data >> 3) & 0xF;
		s.kon = data & 1;
		// Every KON=1 write (re)triggers the slot: P-47 Aces writes KON=1 onto
		// already keyed slots for note repeats, so edge-triggering would drop notes.
		if (data & 1)
			slot_keyon(slotnum);
		else
			slot_keyoff(slotnum);
		break;
	case 0x1:
		s.lfo_freq = data;
		break;
	case 0x2:
		s.lfo_wave = data & 3;
		s.pms = (data >> 3) & 7;
		s.ams = (data >> 6) & 3;
		break;
	case 0x3:
		s.mul = data & 0xF;
		s.dt = (data >> 4) & 7;
		break;
	case 0x4:
		s.tl = data & 0x7F;
		break;
	case 0x5:
		s.ar = data & 0x1F;
		s.ks = (data >> 5) & 7;
		break;
	case 0x6:
		s.d1r = data & 0x1F;
		break;
	case 0x7:
		s.d2r = data & 0x1F;
		break;
	case 0x8:
		s.rr = data & 0xF;
		s.d1l = (data >> 4) & 0xF;
		break;
	case 0x9:
		// F-number low: commits the latched Block / F-number high nibble
		// (manual: Block and F-Number2 must be written before F-Number1)
		s.fnum = ((s.fnum_latch & 0x0F) << 8) | data;
		s.block = s.fnum_latch >> 4;
		s.block_s = int8_t((s.block ^ 8) - 8);
		update_keycode(s, slotnum);
		break;
	case 0xA:
		s.fnum_latch = data;
		break;
	case 0xB:
		s.wave = data & 7;
		s.fb = (data >> 4) & 7;
		s.accon = (data >> 7) & 1;
		update_keycode(s, slotnum);
		break;
	case 0xC:
		s.alg = data & 0xF;
		m_groups[slotnum % 12].dirty = 1;
		break;
	case 0xD:
		s.ch_level[0] = data >> 4;
		s.ch_level[1] = data & 0xF;
		break;
	case 0xE:
		s.ch_level[2] = data >> 4;
		s.ch_level[3] = data & 0xF;
		break;
	default:
		break;
	}
}

// FM function register write: bank 0..3 (S1..S4), address = reg<<4 | group nibble
void ymf271_device::write_fm(int bank, uint8_t address, uint8_t data)
{
	int group = fm_tab[address & 0xF];
	int reg = address >> 4;
	bool broadcast;

	if (group < 0)
	{
		logerror("write to invalid FM group nibble %02X (bank %d, data %02X)\n", address, bank, data);
		return;
	}

	// Registers managed by the key-on sync mode: EN/EXT out/KON, F-Number,
	// Block, Algorithm, CH0-CH3 level.  Written to the key-on slot they are
	// copied to all slots of the voice.
	switch (reg)
	{
	case 0x0: case 0x9: case 0xA: case 0xC: case 0xD: case 0xE:
		broadcast = true;
		break;
	default:
		broadcast = false;
		break;
	}

	if (broadcast && is_keyon_slot(bank, group) && m_groups[group].sync != SYNC_PCM)
	{
		int slots[4];
		int n = voice_slots(bank, group, slots);
		for (int i = 0; i < n; i++)
			write_slot_reg(slots[i], reg, data);
	}
	else
	{
		write_slot_reg(12 * bank + group, reg, data);
	}
}

// PCM attribute register write (bank 5)
void ymf271_device::write_pcm(uint8_t address, uint8_t data)
{
	int slotnum = pcm_tab[address & 0xF];

	if (slotnum < 0)
	{
		logerror("write to invalid PCM slot nibble %02X (data %02X)\n", address, data);
		return;
	}
	opx_slot &s = m_slots[slotnum];

	switch (address >> 4)
	{
	case 0x0: s.pcm_start = (s.pcm_start & 0xFFFF00) | data; break;
	case 0x1: s.pcm_start = (s.pcm_start & 0xFF00FF) | (data << 8); break;
	case 0x2:
		s.pcm_start = (s.pcm_start & 0x00FFFF) | ((data & 0x7F) << 16);
		s.pcm_altloop = data >> 7;
		break;
	case 0x3: s.pcm_end = (s.pcm_end & 0xFFFF00) | data; break;
	case 0x4: s.pcm_end = (s.pcm_end & 0xFF00FF) | (data << 8); break;
	case 0x5: s.pcm_end = (s.pcm_end & 0x00FFFF) | ((data & 0x7F) << 16); break;
	case 0x6: s.pcm_loop = (s.pcm_loop & 0xFFFF00) | data; break;
	case 0x7: s.pcm_loop = (s.pcm_loop & 0xFF00FF) | (data << 8); break;
	case 0x8: s.pcm_loop = (s.pcm_loop & 0x00FFFF) | ((data & 0x7F) << 16); break;
	case 0x9:
		s.pcm_fs = data & 3;
		s.pcm_12bit = (data >> 2) & 1;
		s.pcm_srcnote = (data >> 3) & 3;
		s.pcm_srcb = (data >> 5) & 7;
		update_keycode(s, slotnum);
		break;
	default:
		break;
	}
}

void ymf271_device::update_irq()
{
	m_irq_handler(m_irqstate ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(ymf271_device::timer_a_expired)
{
	m_status |= 1;

	// assert IRQ
	if (m_timer_ctrl & 4)
	{
		m_irqstate |= 1;
		update_irq();
	}

	// reload timer
	m_timA->adjust(clocks_to_attotime(384 * (1024 - m_timerA)), 0);
}

TIMER_CALLBACK_MEMBER(ymf271_device::timer_b_expired)
{
	m_status |= 2;

	// assert IRQ
	if (m_timer_ctrl & 8)
	{
		m_irqstate |= 2;
		update_irq();
	}

	// reload timer
	m_timB->adjust(clocks_to_attotime(384 * 16 * (256 - m_timerB)), 0);
}

// utility register write (bank 6): sync, timers, external memory access, test
void ymf271_device::write_util(uint8_t address, uint8_t data)
{
	if ((address & 0xF0) == 0x00)
	{
		int group = fm_tab[address & 0xF];
		if (group < 0)
		{
			logerror("write to invalid sync group nibble %02X (data %02X)\n", address, data);
			return;
		}
		m_groups[group].sync = data & 3;
		m_groups[group].pfm = data >> 7;
		m_groups[group].dirty = 1;
		return;
	}

	switch (address)
	{
	case 0x10:  // Timer A: the manual says 0x10 = low 8 bits, 0x11 = top 2 bits;
	            // seibuspi shows it behaves like other Yamaha chips:
	            // 0x10 = high 8 bits, 0x11 = low 2 bits.
		m_timerA = (m_timerA & 0x003) | (data << 2);
		break;
	case 0x11:
		m_timerA = (m_timerA & 0x3FC) | (data & 0x03);
		break;
	case 0x12:
		m_timerB = data;
		break;
	case 0x13:
		// bit0/1 load A/B, bit2/3 enable IRQ A/B, bit4/5 reset flag A/B.
		// A timer is (re)loaded on the rising edge of its load bit; clearing
		// the bit does not stop a running timer (only the IRQ is gated).
		if (~m_timer_ctrl & data & 1)
			m_timA->adjust(clocks_to_attotime(384 * (1024 - m_timerA)), 0);
		if (~m_timer_ctrl & data & 2)
			m_timB->adjust(clocks_to_attotime(384 * 16 * (256 - m_timerB)), 0);
		if (data & 0x10)
		{
			m_status &= ~0x01;
			m_irqstate &= ~0x01;
		}
		if (data & 0x20)
		{
			m_status &= ~0x02;
			m_irqstate &= ~0x02;
		}
		m_timer_ctrl = data;
		update_irq();
		break;
	case 0x14:
		m_ext_address = (m_ext_address & 0xFFFF00) | data;
		break;
	case 0x15:
		m_ext_address = (m_ext_address & 0xFF00FF) | (data << 8);
		break;
	case 0x16:
		m_ext_address = (m_ext_address & 0x00FFFF) | ((data & 0x7F) << 16);
		m_ext_rw = data >> 7;
		// prime the read latch for the read-direction window
		if (m_ext_rw)
			m_ext_readlatch = read_byte(m_ext_address);
		break;
	case 0x17:
		// write to external memory (SRAM); the address is incremented before
		// the write (previous core's reading; the documents do not say)
		m_ext_address = (m_ext_address + 1) & 0x7FFFFF;
		if (!m_ext_rw)
			space(0).write_byte(m_ext_address, data);
		break;
	case 0x20: case 0x21: case 0x22:
		// test registers
		break;
	default:
		break;
	}
}

void ymf271_device::write(offs_t offset, u8 data)
{
	m_stream->update();

	offset &= 0xF;
	m_regs_main[offset] = data;

	switch (offset)
	{
	case 0x1: write_fm(0, m_regs_main[0x0], data); break;
	case 0x3: write_fm(1, m_regs_main[0x2], data); break;
	case 0x5: write_fm(2, m_regs_main[0x4], data); break;
	case 0x7: write_fm(3, m_regs_main[0x6], data); break;
	case 0x9: write_pcm(m_regs_main[0x8], data); break;
	case 0xD: write_util(m_regs_main[0xC], data); break;
	default:
		// even offsets: address latches; 0xB/0xF: unused banks
		break;
	}
}

u8 ymf271_device::read(offs_t offset)
{
	offset &= 0xF;

	// update stream before reading status registers
	if (offset <= 1)
		m_stream->update();

	switch (offset)
	{
	case 0x0:
		// d0 TiA, d1 TiB, d2 0, d3 End0, d4 End12, d5 End24, d6 End36, d7 Busy
		// end_status bit i = PCM slot 4*i: slots 0,12,24,36 = bits 0,3,6,9.
		// The End flags are cleared by reading them (Brave Blade copies the
		// status registers to RAM every ~100 us and frees a PCM channel when
		// its End bit is seen; a sticky flag would kill the next note started
		// on that slot from the stale copy).
		{
			uint8_t ret = (m_status & 0x83) | (BIT(m_end_status, 0) << 3) | (BIT(m_end_status, 3) << 4)
			     | (BIT(m_end_status, 6) << 5) | (BIT(m_end_status, 9) << 6);
			if (!machine().side_effects_disabled())
				m_end_status &= ~0x0249;
			return ret;
		}
	case 0x1:
		// d0 End4, d1 End16, d2 End28, d3 End40, d4 End8, d5 End20, d6 End32, d7 End44
		// end_status bit i belongs to slot 4*i: slots 4,16,28,40 = bits 1,4,7,10;
		// slots 8,20,32,44 = bits 2,5,8,11
		{
			uint16_t e = m_end_status;
			uint8_t ret = ((e >> 1) & 1) | (((e >> 4) & 1) << 1) | (((e >> 7) & 1) << 2) | (((e >> 10) & 1) << 3)
			     | (((e >> 2) & 1) << 4) | (((e >> 5) & 1) << 5) | (((e >> 8) & 1) << 6) | (((e >> 11) & 1) << 7);
			if (!machine().side_effects_disabled())
				m_end_status &= ~0x0DB6;
			return ret;
		}
	case 0x2:
		{
			if (!m_ext_rw)
				return 0xFF;
			uint8_t ret = m_ext_readlatch;
			if (!machine().side_effects_disabled())
			{
				m_ext_address = (m_ext_address + 1) & 0x7FFFFF;
				m_ext_readlatch = read_byte(m_ext_address);
			}
			return ret;
		}
	default:
		break;
	}
	return 0xFF;
}


// ------------------------------------------------------------------------
// synthesis
// ------------------------------------------------------------------------

void ymf271_device::init_tables()
{
	for (int i = 0; i < 256; i++)
	{
		m_logsin[i] = uint16_t(floor(-log(sin((i + 0.5) * std::numbers::pi / 512.0)) / log(2.0) * 256.0 + 0.5));
		m_exp[i] = uint16_t(floor(pow(2.0, -(i + 1) / 256.0) * 2048.0 + 0.5));
	}
}

// apply one algorithm table to a list of slots
void ymf271_device::connect(const opx_alg &alg, const int *slots, int n)
{
	for (int p = 0; p < n; p++)
	{
		opx_slot &s = m_slots[slots[p]];
		s.c_nmod = 0;
		for (int q = 0; q < n; q++)
			if (alg.mods[p] & (1 << q))
				s.c_mod[s.c_nmod++] = uint8_t(slots[q]);
		s.c_carrier = (alg.car >> p) & 1;
		// the head slot takes the feedback input; its history is written by
		// position fbsrc (itself, or S3 for the loop algorithms)
		s.c_fbhead = (p == 0);
		s.c_fbtarget = (p == alg.fbsrc) ? int8_t(slots[0]) : -1;
	}
}

void ymf271_device::rebuild_group(int g)
{
	int slots[4];

	m_groups[g].dirty = 0;
	switch (m_groups[g].sync)
	{
	case SYNC_4OP:
		slots[0] = g; slots[1] = g + 12; slots[2] = g + 24; slots[3] = g + 36;
		connect(alg4[m_slots[g].alg & 15], slots, 4);
		break;
	case SYNC_2X2OP:
		slots[0] = g; slots[1] = g + 24;
		connect(alg2[m_slots[g].alg & 3], slots, 2);
		slots[0] = g + 12; slots[1] = g + 36;
		connect(alg2[m_slots[g + 12].alg & 3], slots, 2);
		break;
	case SYNC_3OP_PCM:
		slots[0] = g; slots[1] = g + 12; slots[2] = g + 24;
		connect(alg3[m_slots[g].alg & 7], slots, 3);
		slots[0] = g + 36;
		connect(alg_single, slots, 1);
		break;
	default:
		slots[0] = g;      connect(alg_single, slots, 1);
		slots[0] = g + 12; connect(alg_single, slots, 1);
		slots[0] = g + 24; connect(alg_single, slots, 1);
		slots[0] = g + 36; connect(alg_single, slots, 1);
		break;
	}
}

// one EG clock (fs/2) for one slot
void ymf271_device::eg_tick(opx_slot &s)
{
	int rks, rate, shift, idx, inc;

	if (s.eg_state == EG_OFF)
		return;

	// state transitions checked first (OPM)
	if (s.eg_state == EG_ATTACK && s.eg_att <= 0)
	{
		s.eg_att = 0;
		s.eg_state = EG_DECAY1;
	}
	if (s.eg_state == EG_DECAY1)
	{
		int d1l = (s.d1l == 15) ? (31 << 5) : (s.d1l << 5);
		if (s.eg_att >= d1l)
			s.eg_state = EG_DECAY2;
	}

	rks = rks_tab[s.keycode][s.ks];
	switch (s.eg_state)
	{
	case EG_ATTACK:  rate = eg_rate(s.ar * 2, rks); break;
	case EG_DECAY1:  rate = eg_rate(s.d1r * 2, rks); break;
	case EG_DECAY2:  rate = eg_rate(s.d2r * 2, rks); break;
	default:         rate = eg_rate(s.rr * 4, rks); break;
	}

	if (rate < 48)
	{
		shift = 11 - (rate >> 2);
		if (m_eg_cnt & ((1 << shift) - 1))
			return;
		idx = (m_eg_cnt >> shift) & 7;
	}
	else
	{
		idx = m_eg_cnt & 7;
	}
	inc = (eg_inc[rate] >> (idx * 4)) & 15;
	if (inc == 0)
		return;

	if (s.eg_state == EG_ATTACK)
	{
		s.eg_att += ((~s.eg_att) * inc) >> 4;
		if (s.eg_att <= 0)
		{
			s.eg_att = 0;
			s.eg_state = EG_DECAY1;
		}
	}
	else
	{
		s.eg_att += inc;
		if (s.eg_att >= 0x3FF)
		{
			s.eg_att = 0x3FF;
			if (s.eg_state == EG_RELEASE)
				s.eg_state = EG_OFF;
		}
	}
}

// LFO clock divider (manual table 2-6-2): the LFO advances one of 128 steps
// every K samples, K = (32 - (n & 15)) << (14 - (n >> 4)) for n < 240 and
// K = 16 - (n & 15) for n >= 240  ->  f = fs / 128 / K (0.00066 .. 344.5 Hz).
uint32_t ymf271_device::lfo_period(uint8_t n)
{
	if (n >= 240)
		return 16 - (n & 15);
	return (32 - (n & 15)) << (14 - (n >> 4));
}

// advance the LFO of one slot by one sample
void ymf271_device::lfo_tick(opx_slot &s)
{
	if (s.lfo_wave == 0)
		return;
	if (++s.lfo_cnt >= lfo_period(s.lfo_freq))
	{
		s.lfo_cnt = 0;
		s.lfo_pos = (s.lfo_pos + 1) & 127;
	}
}

// bipolar LFO value for PM, -128..127 (wave 1 saw, 2 square, 3 triangle),
// all starting at 0 at key-on and rising first
int32_t ymf271_device::lfo_pm(const opx_slot &s)
{
	int32_t p = s.lfo_pos;
	switch (s.lfo_wave)
	{
	case 1: return ((p + 64) & 127) * 2 - 128;          // saw: 0 -> +126, -128 -> -2
	case 2: return (p < 64) ? 127 : -128;               // square
	case 3: // triangle: 0 -> +124 -> 0 -> -128 -> -4
		if (p < 32) return p * 4;
		if (p < 96) return 128 - (p - 32) * 4;
		return (p - 96) * 4 - 128;
	default: return 0;
	}
}

// unipolar LFO value for AM (attenuation), 0..127.  Every waveform starts
// at full attenuation at key-on and the saw / triangle come down from there
// (the OPM/OPZ convention, ymfm's reading of the manual figures)
int32_t ymf271_device::lfo_am(const opx_slot &s)
{
	int32_t p = s.lfo_pos;
	switch (s.lfo_wave)
	{
	case 1: return 127 - p;                                         // saw: 127 -> 0
	case 2: return (p < 64) ? 127 : 0;                              // square: attenuated half first
	case 3: return (p < 64) ? (127 - p * 2) : (p - 64) * 2 + 1;     // triangle 127 -> 1 -> 127
	default: return 0;
	}
}

// phase increment per sample, 2^32 = one cycle:
// f = 2 * fnum * 2^(block-7) * MUL * fs / 2^15  ->  inc = fnum << (block + 11) * MUL,
// detune added in units of fs/2^20 (= 1 << 12 here) before the multiplier (OPM)
uint32_t ymf271_device::phase_inc(const opx_slot &s, int32_t lfo_pm)
{
	int64_t inc;
	int sh = s.block_s + 11;    // 3..18
	int dt;
	int64_t fnum = s.fnum << 7; // fnum with 7 fraction bits for the LFO

	// LFO PM: fnum * (1 + k * lfo / (1024 * 128))
	if (lfo_pm != 0)
		fnum += (int64_t(s.fnum) * pms_k[s.pms] * lfo_pm) >> 10;
	inc = (fnum << sh) >> 7;
	dt = detune_tab[s.keycode][s.dt & 3] << 12;
	if (s.dt & 4)
		inc -= dt;
	else
		inc += dt;
	if (inc < 0)
		inc = 0;
	if (s.mul == 0)
		inc >>= 1;
	else
		inc *= s.mul;
	// the accumulator wraps like OPM's: increments above fs/2 alias (the
	// hi-hat carriers of the Seibu titles are fed by such a modulator)
	return uint32_t(inc);
}

// ---- external PCM waveform (wave 7 on slots 0,4,..,44) ----
// Step in source samples per frame (16-bit fraction):
//   (fnum | 0x800) / 2048 * 2^block * MUL * {1, 1/2, 1/4, 1/8}[Fs]
// i.e. the same PG as FM with the implicit F-number bit 11 (block 0 /
// fnum 0 / MUL 1 plays a 44.1 kHz sample at its original rate), detune and
// LFO PM applied like FM.  Positions run over [0, End): when the position
// reaches End it wraps back by End-Loop (a looped sample's period is then
// exactly End-Loop words), so word End is only read as the interpolation
// partner of End-1 -- which is why the manual requires Words >= End+1.
// Samples are 8-bit or packed 12-bit (3 bytes per 2 words, see pcm_word) and
// linearly interpolated ("the waveform data is interpolated and the EG value
// multiplied").
uint32_t ymf271_device::pcm_step(const opx_slot &s, int32_t lfo_pm)
{
	int64_t inc;
	int64_t fnum = ((s.fnum & 0x7FF) | 0x800) << 7;  // 7 fraction bits for the LFO
	int dt;

	if (lfo_pm != 0)
		fnum += (int64_t((s.fnum & 0x7FF) | 0x800) * pms_k[s.pms] * lfo_pm) >> 10;
	// fnum(q7) * 2^(block-11) * 65536 = fnum(q7) << (block + 5) >> 7
	inc = (fnum << 16) >> (18 - s.block_s);
	dt = detune_tab[s.keycode][s.dt & 3] << 6;  // FM units (1 << 12) / 2^6
	if (s.dt & 4)
		inc -= dt;
	else
		inc += dt;
	if (inc < 0)
		inc = 0;
	if (s.mul == 0)
		inc >>= 1;
	else
		inc *= s.mul;
	inc >>= s.pcm_fs;
	return uint32_t(inc);
}

// 12-bit sample word at index pos of slot s (8-bit data = upper byte)
int32_t ymf271_device::pcm_word(const opx_slot &s, uint32_t pos)
{
	int32_t v;

	if (!s.pcm_12bit)
	{
		v = int8_t(read_byte((s.pcm_start + pos) & 0x7FFFFF)) << 4;
	}
	else
	{
		// 3 bytes per 2 words: b0 = word0 bits 11..4, b1 = word1 bits 3..0 (high
		// nibble) : word0 bits 3..0 (low nibble), b2 = word1 bits 11..4.
		// (Checked on the Bloody Roar 2 / Beastorizer sample data: this order
		// makes the smooth samples as much smoother than the 8-bit-only decode
		// as true LSBs should, the swapped order makes them rougher than
		// dropping the nibble.)
		uint32_t addr = (s.pcm_start + (pos >> 1) * 3) & 0x7FFFFF;
		if (pos & 1)
			v = (read_byte(addr + 2) << 4) | (read_byte(addr + 1) >> 4);
		else
			v = (read_byte(addr) << 4) | (read_byte(addr + 1) & 0x0F);
		v = int32_t((v & 0xFFF) ^ 0x800) - 0x800;
	}
	return v;
}

// interpolated 14-bit sample at the current position, then advance
int32_t ymf271_device::pcm_sample(opx_slot &s, int slotnum, int32_t lfo_pm)
{
	int32_t a = pcm_word(s, s.pcm_pos);
	int32_t b = pcm_word(s, s.pcm_pos + 1);
	int32_t frac = int32_t(s.pcm_frac >> 8);   // 8-bit interpolation weight
	int32_t out = (a * (256 - frac) + b * frac) >> 6;  // 12 bit -> 14 bit
	uint32_t step = pcm_step(s, lfo_pm);

	s.pcm_frac += step;
	s.pcm_pos += s.pcm_frac >> 16;
	s.pcm_frac &= 0xFFFF;
	if (s.pcm_pos >= s.pcm_end)
	{
		if (s.pcm_end > s.pcm_loop)
		{
			uint32_t len = s.pcm_end - s.pcm_loop;
			s.pcm_pos = s.pcm_loop + (s.pcm_pos - s.pcm_end) % len;
		}
		else
			s.pcm_pos = s.pcm_loop;   // degenerate loop: hold
		// The End flag is raised once per key-on, when the read address first
		// passes the end address.  Drivers play one-shot samples as a short
		// loop of silence at the end (loop = end - 2) and free the channel from
		// a copy of the status register: re-raising End on every pass of that
		// loop kills a note re-triggered on the same slot between the copy
		// and the free pass (Bloody Roar 2 / Brave Blade lose drum hits).
		if (!s.pcm_ended)
		{
			s.pcm_ended = 1;
			m_end_status |= 1 << (slotnum >> 2);
		}
	}
	return out;
}

// multiply a 14-bit sample by the envelope (10-bit attenuation, 64 = 6 dB)
int32_t ymf271_device::env_mul(int32_t v, uint32_t env) const
{
	return (v * int32_t(m_exp[(env & 63) << 2])) >> (11 + (env >> 6));
}

// operator: 10-bit phase, waveform, 10-bit total attenuation -> 14-bit output
int32_t ymf271_device::op(uint32_t phase, int wave, uint32_t env) const
{
	uint32_t p = phase & 1023;
	uint32_t idx, att, neg;
	int32_t out;

	idx = p & 255;
	if (p & 256)
		idx ^= 255;
	neg = 0;
	switch (wave)
	{
	case 0: // sine
		att = m_logsin[idx];
		neg = p & 512;
		break;
	case 1: // +/-sin^2 (manual plot): twice the log-sin attenuation, sign from
	        // phase bit 9 -- as ymfm's OPZ wave 1
		att = m_logsin[idx] << 1;
		neg = p & 512;
		break;
	case 2: // |sin|
		att = m_logsin[idx];
		break;
	case 3: // half sine
		if (p & 512)
			return 0;
		att = m_logsin[idx];
		break;
	case 4: // sin(2wt) on the first half
	case 5: // |sin(2wt)| on the first half
		if (p & 512)
			return 0;
		idx = (p << 1) & 255;
		if (p & 128)
			idx ^= 255;
		att = m_logsin[idx];
		if (wave == 4)
			neg = p & 256;
		break;
	default:    // 6: linear waveform, 7: external PCM -- both handled in sound_stream_update
		return 0;
	}

	att += env << 2;
	if (att >= 4096)
		return 0;
	out = (m_exp[att & 255] << 2) >> (att >> 8);
	return neg ? -out : out;
}

// channel level table: x(1 or 0.75) >> (L>>1), L >= 13 mute
int32_t ymf271_device::pan(int32_t v, uint8_t level)
{
	if (level >= 13)
		return 0;
	if (level & 1)
		v = (v * 3) >> 2;
	return v >> (level >> 1);
}

void ymf271_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		int32_t acc[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		for (int g = 0; g < NUM_GROUPS; g++)
			if (m_groups[g].dirty)
				rebuild_group(g);

		// EG runs at fs/2
		const bool eg_clock = m_eg_phase;
		m_eg_phase ^= 1;
		if (eg_clock)
			m_eg_cnt++;

		// slots in hardware order: modulators with a lower slot number are
		// taken from the current frame, higher-numbered ones from the previous
		for (int n = 0; n < NUM_SLOTS; n++)
		{
			opx_slot &s = m_slots[n];
			int32_t mod, out;
			uint32_t env;

			if (eg_clock)
				eg_tick(s);
			lfo_tick(s);

			if (s.eg_state == EG_OFF)
			{
				s.out = 0;
				s.acc = 0;
				continue;
			}

			// modulation input, in 10-bit phase units
			if (s.c_fbhead)
			{
				// OPM feedback law on the average of the last two outputs; the
				// S3->S1 loop of algorithms 1/5/7/11 follows the same law
				int fb = s.fb & 7;
				mod = fb ? ((s.fb_hist[0] + s.fb_hist[1]) >> (10 - fb)) : 0;
			}
			else
			{
				int32_t sum = 0;
				for (int k = 0; k < s.c_nmod; k++)
					sum += m_slots[s.c_mod[k]].out;
				mod = (sum * modlevel[s.fb]) >> 8;
			}

			env = uint32_t(s.eg_att) + (uint32_t(s.tl) << 3);
			if (s.ams && s.lfo_wave)
			{
				// AMS max 63 / 126 / 252 units of 0.09375 dB
				int32_t am = lfo_am(s);
				env += (s.ams == 1) ? (am >> 1) : (s.ams == 2) ? am : (am << 1);
			}
			if (env > 0x3FF)
				env = 0x3FF;

			const int32_t pm = (s.pms && s.lfo_wave) ? lfo_pm(s) : 0;

			if (s.wave == 7)
			{
				// external PCM data: only the slots of groups 0/4/8 can fetch it;
				// a wave-7 select elsewhere produces silence here
				out = ((n & 3) == 0) ? env_mul(pcm_sample(s, n, pm), env) : 0;
			}
			else if (s.wave == 6)
			{
				// "linear waveform table": assumed that output does not depend on the
				// phase; it is (1 + modulation input) * envelope, i.e. a DC level
				// when unmodulated (drum pitch sweeps use it as a modulator) and
				// the modulation input passed through when modulated (hi-hat
				// carriers emit their modulator directly).  The modulation input
				// is scaled by MUL (0 = 1/2) like a phase increment and wraps at
				// 2^W6_BITS; full scale = one wrap.  The wrap size sets the
				// passed-through modulator's noise floor and the level of the
				// PG-rate line in it.  The manual only states "1" (D.C.) for
				// this waveform; the phase does not enter.
				int32_t mm = s.mul ? mod * int32_t(s.mul) : (mod >> 1);
				int32_t lin;
				if (W6_BITS)
					lin = 8192 + ((1 + (mm & ((1 << W6_BITS) - 1))) << (13 - W6_BITS));
				else
					lin = 8168 + (mm << 4);
				out = env_mul(lin, env);
				s.phase += phase_inc(s, pm);   // the PG keeps running
			}
			else
			{
				out = op((s.phase >> 22) + mod, s.wave, env);
				s.phase += phase_inc(s, pm);
			}
			if (s.accon)
			{
				// "Acc On": the slot output is accumulated instead of output
				// directly -- modelled as a running sum that saturates at the
				// 14-bit operator range.  Any sustained tone rails the sum, so
				// the slot turns into a full-level square-like signal that
				// flips at the operator's zero crossings ("distorted" basses
				// and drums); the sum is cleared at key-on and when the EG
				// reaches off.  Applies to modulators as well.
				s.acc += out;
				if (s.acc > 8191) s.acc = 8191;
				else if (s.acc < -8192) s.acc = -8192;
				out = s.acc;
			}
			s.out = out;

			if (s.c_fbtarget >= 0)
			{
				opx_slot &t = m_slots[s.c_fbtarget];
				t.fb_hist[1] = t.fb_hist[0];
				t.fb_hist[0] = out;
			}

			if (s.c_carrier)
			{
				acc[0] += pan(out, s.ch_level[0]);
				acc[1] += pan(out, s.ch_level[1]);
				acc[2] += pan(out, s.ch_level[2]);
				acc[3] += pan(out, s.ch_level[3]);
				// extended channels CH4-CH7 (EXT1 = CH4/5, EXT2 = CH6/7):
				// EN enables them, EXT Out is a bitmask of the channels the
				// voice is sent to, at full level (no attenuation register)
				if (s.ext_en)
				{
					if (s.ext_out & 1) acc[4] += out;
					if (s.ext_out & 2) acc[5] += out;
					if (s.ext_out & 4) acc[6] += out;
					if (s.ext_out & 8) acc[7] += out;
				}
			}
		}

		// one carrier at full level = 14-bit +/-8192 = -12 dB FS
		for (int ch = 0; ch < 8; ch++)
			stream.put_int(ch, i, acc[ch], 32768);
	}
}


// ------------------------------------------------------------------------
// device lifecycle
// ------------------------------------------------------------------------

void ymf271_device::init_state()
{
	save_item(STRUCT_MEMBER(m_slots, kon));
	save_item(STRUCT_MEMBER(m_slots, ext_en));
	save_item(STRUCT_MEMBER(m_slots, ext_out));
	save_item(STRUCT_MEMBER(m_slots, lfo_freq));
	save_item(STRUCT_MEMBER(m_slots, ams));
	save_item(STRUCT_MEMBER(m_slots, pms));
	save_item(STRUCT_MEMBER(m_slots, lfo_wave));
	save_item(STRUCT_MEMBER(m_slots, dt));
	save_item(STRUCT_MEMBER(m_slots, mul));
	save_item(STRUCT_MEMBER(m_slots, tl));
	save_item(STRUCT_MEMBER(m_slots, ks));
	save_item(STRUCT_MEMBER(m_slots, ar));
	save_item(STRUCT_MEMBER(m_slots, d1r));
	save_item(STRUCT_MEMBER(m_slots, d2r));
	save_item(STRUCT_MEMBER(m_slots, d1l));
	save_item(STRUCT_MEMBER(m_slots, rr));
	save_item(STRUCT_MEMBER(m_slots, fnum));
	save_item(STRUCT_MEMBER(m_slots, block));
	save_item(STRUCT_MEMBER(m_slots, fnum_latch));
	save_item(STRUCT_MEMBER(m_slots, accon));
	save_item(STRUCT_MEMBER(m_slots, fb));
	save_item(STRUCT_MEMBER(m_slots, wave));
	save_item(STRUCT_MEMBER(m_slots, alg));
	save_item(STRUCT_MEMBER(m_slots, ch_level));
	save_item(STRUCT_MEMBER(m_slots, pcm_start));
	save_item(STRUCT_MEMBER(m_slots, pcm_end));
	save_item(STRUCT_MEMBER(m_slots, pcm_loop));
	save_item(STRUCT_MEMBER(m_slots, pcm_altloop));
	save_item(STRUCT_MEMBER(m_slots, pcm_fs));
	save_item(STRUCT_MEMBER(m_slots, pcm_12bit));
	save_item(STRUCT_MEMBER(m_slots, pcm_srcnote));
	save_item(STRUCT_MEMBER(m_slots, pcm_srcb));
	save_item(STRUCT_MEMBER(m_slots, block_s));
	save_item(STRUCT_MEMBER(m_slots, keycode));
	save_item(STRUCT_MEMBER(m_slots, eg_state));
	save_item(STRUCT_MEMBER(m_slots, eg_att));
	save_item(STRUCT_MEMBER(m_slots, phase));
	save_item(STRUCT_MEMBER(m_slots, out));
	save_item(STRUCT_MEMBER(m_slots, acc));
	save_item(STRUCT_MEMBER(m_slots, fb_hist));
	save_item(STRUCT_MEMBER(m_slots, lfo_cnt));
	save_item(STRUCT_MEMBER(m_slots, lfo_pos));
	save_item(STRUCT_MEMBER(m_slots, pcm_pos));
	save_item(STRUCT_MEMBER(m_slots, pcm_frac));
	save_item(STRUCT_MEMBER(m_slots, pcm_ended));
	save_item(STRUCT_MEMBER(m_slots, c_nmod));
	save_item(STRUCT_MEMBER(m_slots, c_mod));
	save_item(STRUCT_MEMBER(m_slots, c_fbhead));
	save_item(STRUCT_MEMBER(m_slots, c_fbtarget));
	save_item(STRUCT_MEMBER(m_slots, c_carrier));

	save_item(STRUCT_MEMBER(m_groups, sync));
	save_item(STRUCT_MEMBER(m_groups, pfm));
	save_item(STRUCT_MEMBER(m_groups, dirty));

	save_item(NAME(m_regs_main));
	save_item(NAME(m_timerA));
	save_item(NAME(m_timerB));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_status));
	save_item(NAME(m_end_status));
	save_item(NAME(m_irqstate));
	save_item(NAME(m_ext_address));
	save_item(NAME(m_ext_rw));
	save_item(NAME(m_ext_readlatch));
	save_item(NAME(m_eg_cnt));
	save_item(NAME(m_eg_phase));
	save_item(NAME(m_master_clock));
}

void ymf271_device::device_start()
{
	m_timA = timer_alloc(FUNC(ymf271_device::timer_a_expired), this);
	m_timB = timer_alloc(FUNC(ymf271_device::timer_b_expired), this);

	m_master_clock = clock();
	init_tables();
	init_state();

	// outputs 0-3 = CH0-CH3 (DO1 = CH0/1, DO2 = CH2/3), 4-7 = CH4-CH7 (EXT1 = CH4/5, EXT2 = CH6/7)
	m_stream = stream_alloc(0, 8, m_master_clock / 384);
}

void ymf271_device::device_reset()
{
	for (int i = 0; i < NUM_SLOTS; i++)
	{
		opx_slot &s = m_slots[i];
		s = opx_slot();
		s.eg_state = EG_OFF;
		s.eg_att = 0x3FF;
		s.c_fbtarget = -1;
	}
	for (int i = 0; i < NUM_GROUPS; i++)
	{
		m_groups[i].sync = 0;
		m_groups[i].pfm = 0;
		m_groups[i].dirty = 1;
	}
	m_eg_cnt = 0;
	m_eg_phase = 0;
	std::fill(std::begin(m_regs_main), std::end(m_regs_main), 0);

	// reset timers and IRQ
	m_timA->reset();
	m_timB->reset();

	m_timerA = 0;
	m_timerB = 0;
	m_timer_ctrl = 0;
	m_status = 0;
	m_end_status = 0;
	m_irqstate = 0;
	m_ext_address = 0;
	m_ext_rw = 0;
	m_ext_readlatch = 0;

	m_irq_handler(0);
}

void ymf271_device::device_clock_changed()
{
	uint32_t old_clock = m_master_clock;
	m_master_clock = clock();

	if (m_master_clock != old_clock)
		m_stream->set_sample_rate(m_master_clock / 384);
}

void ymf271_device::rom_bank_pre_change()
{
	m_stream->update();
}

DEFINE_DEVICE_TYPE(YMF271, ymf271_device, "ymf271", "Yamaha YMF271 OPX")

ymf271_device::ymf271_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, YMF271, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_timerA(0)
	, m_timerB(0)
	, m_timer_ctrl(0)
	, m_status(0)
	, m_end_status(0)
	, m_irqstate(0)
	, m_ext_address(0)
	, m_ext_rw(0)
	, m_ext_readlatch(0)
	, m_eg_cnt(0)
	, m_eg_phase(0)
	, m_master_clock(0)
	, m_timA(nullptr)
	, m_timB(nullptr)
	, m_stream(nullptr)
	, m_irq_handler(*this)
{
	std::fill(std::begin(m_slots), std::end(m_slots), opx_slot());
	std::fill(std::begin(m_groups), std::end(m_groups), opx_group());
	std::fill(std::begin(m_regs_main), std::end(m_regs_main), 0);
}
