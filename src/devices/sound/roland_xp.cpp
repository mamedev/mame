// license:BSD-3-Clause
// copyright-holders:superctr
// thanks-to: giulioz
/*
 *  Roland XP (MBCS30109/RA01-005)
 *
 *  64-voice custom sound generator with built-in microcoded effect DSP.
 *
 *  Host window: 44 voice pages of 64 x 32 bits (page * 0x100 + voice * 4, high word first,
 *  committed by the low word), CRAM/IRAM/PRAM of the effect DSP, the control block at 0x3900
 *  (run mask, readback latch, interrupt status/acknowledge, ROM window page/bank), four
 *  banks of mixer sends at 0x3a00 and the wave ROM access window at 0x3c00.
 *
 *  Voice: DPCM reader with forward, alternate and reverse loops over 1 MB ROM regions,
 *  4-tap 128-phase interpolator, Chamberlin state-variable filter, two amplitude gains and
 *  four sends (10-bit level, 6-bit destination word). The five ramps per voice (pitch,
 *  cutoff, resonance, two amplitudes) update every 8 frames, the first amplitude every 2,
 *  with a rate, a hold divider and a law selected by their control word. Pitch and cutoff
 *  registers are logarithmic (16384 per octave, unity 0x38000 for pitch), decoded through
 *  a 257-entry exponential.
 *
 *  DSP: 256 instruction slots per frame over 256 24-bit words of internal RAM (the 64 mixer
 *  bus words land in words 0x40-0x7f, words 0xf0-0xff are the host-ramped gain registers)
 *  and a 64K-word external delay ring addressed relative to a cursor that steps back once
 *  per frame. Each slot reads a word into the operand register, forms one product (operand
 *  x coefficient, or x a gain register under the 42xx coefficient codes) and applies its
 *  ALU operation to the previous slot's product, so the pipeline is three slots deep; ERAM
 *  accesses span two slots and a read lands two slots after its start; a third access kind
 *  stores the sample on the chip's serial input. The mixer bus enters the DSP four bits down
 *  (the SC-88 program's output stage is x32 on its dry words; with less a loud GS song
 *  saturates the output words).
 *
 *  Outputs: the program's output stage stores into RAM words 00-07, and the chip's three serial
 *  data lines clock six of them out a frame, one per line per half. The stream carries all eight
 *  as the lines present them - the word one bit up, saturated to 24 bits - and the board's DACs
 *  take whichever pairs its program stores (SC-88 01/04; SC-88Pro 02/03 and 06/07 for its two
 *  DAC pairs; JV-1080 00/03, 01/04, 02/05 for its three), which is the driver's routing. How the
 *  chip's own configuration words pick the six is not known.
 *
 *  Serial link: once a frame the chip clocks a stereo pair out of two of its RAM words on the
 *  same terms (set_serial_output_words(), the SC-88Pro's effect-processor send) and takes a pair back.
 *  An ext strobe of 1 presents one channel of that pair - the first strobe of a frame the
 *  first channel, the next the other - and it lands on the external RAM bus two slots later,
 *  the same latency a read has, so a write in that slot stores it instead of the accumulator;
 *  the third kind of access stores it wherever it is issued. The program reads the cells back
 *  in its output stage, one per channel. Which words a board sends is a per-board fact, as the
 *  DAC pair is; the exchange is one callback each way, so the processor on the other end is
 *  the driver's business.
 *
 *  The DSP program is the chip's schedule - one pass of the 256 slots is one sample - so the
 *  device is a CPU as much as a sound chip: execute_run() steps slots, the voices run when the
 *  program wraps, and the output stream is synchronous, taking whatever the pass left in the
 *  output words.
 *
 *  TODO:
 *  - readback timing
 *  - wait states
 *  - master/slave configuration (used by SC-8850)
 *  - the rest of the DSP ext (serial I/O strobe) field: which pins a strobe drives
 *  - DSP branch/PRNG
 *  - fetch-overload or filter ramp interrupts (not used on SC-88 family)
 *  - figure out the meaning of the unnamed voice pages and of the configuration words
 */
#include "emu.h"
#include "roland_xp.h"

#include "roland_xpd.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#define LOG_VOICE   (1U << 1)
#define LOG_CRAM    (1U << 2)
#define LOG_IRAM    (1U << 3)
#define LOG_PRAM    (1U << 4)
#define LOG_RUNMASK (1U << 5)
#define LOG_CTRL    (1U << 6)
#define LOG_MIX     (1U << 7)
#define LOG_READ    (1U << 8)
#define LOG_UNKNOWN (1U << 9)

#define VERBOSE (LOG_GENERAL | LOG_UNKNOWN)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ROLAND_XP, roland_xp_device, "roland_xp", "Roland XP")

namespace {

const s16 interp_weights[3][128] = {
	{
		3385, 3401, 3417, 3432, 3448, 3463, 3478, 3492, 3506, 3521, 3534, 3548, 3562, 3575, 3588, 3601,
		3614, 3626, 3638, 3650, 3662, 3673, 3685, 3696, 3707, 3718, 3728, 3739, 3749, 3759, 3768, 3778,
		3787, 3796, 3805, 3814, 3823, 3831, 3839, 3847, 3855, 3863, 3870, 3878, 3885, 3892, 3899, 3905,
		3912, 3918, 3924, 3930, 3936, 3942, 3948, 3953, 3958, 3963, 3968, 3973, 3978, 3983, 3987, 3991,
		3995, 4000, 4004, 4007, 4011, 4015, 4018, 4022, 4025, 4028, 4031, 4034, 4037, 4040, 4042, 4045,
		4047, 4050, 4052, 4054, 4057, 4059, 4061, 4063, 4064, 4066, 4068, 4070, 4071, 4073, 4074, 4076,
		4077, 4078, 4079, 4081, 4082, 4083, 4084, 4085, 4086, 4086, 4087, 4088, 4089, 4089, 4090, 4091,
		4091, 4092, 4092, 4093, 4093, 4094, 4094, 4094, 4094, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
	},
	{
		 710,  726,  742,  758,  775,  792,  809,  826,  844,  861,  879,  897,  915,  933,  952,  971,
		 990, 1009, 1028, 1047, 1067, 1087, 1106, 1126, 1147, 1167, 1188, 1208, 1229, 1250, 1271, 1292,
		1314, 1335, 1357, 1379, 1400, 1423, 1445, 1467, 1489, 1512, 1534, 1557, 1580, 1602, 1625, 1648,
		1671, 1695, 1718, 1741, 1764, 1788, 1811, 1835, 1858, 1882, 1906, 1929, 1953, 1977, 2000, 2024,
		2048, 2069, 2095, 2119, 2143, 2166, 2190, 2214, 2237, 2261, 2284, 2308, 2331, 2355, 2378, 2401,
		2425, 2448, 2471, 2494, 2517, 2539, 2562, 2585, 2607, 2630, 2652, 2674, 2696, 2718, 2740, 2762,
		2783, 2805, 2826, 2847, 2868, 2889, 2910, 2931, 2951, 2971, 2991, 3011, 3031, 3051, 3070, 3089,
		3108, 3127, 3146, 3164, 3182, 3200, 3218, 3236, 3253, 3271, 3288, 3304, 3321, 3338, 3354, 3370,
	},
	{
		   0,    0,    0,    1,    1,    1,    2,    2,    3,    3,    3,    4,    4,    5,    5,    6,
		   6,    7,    8,    8,    9,   10,   10,   11,   12,   13,   14,   15,   16,   17,   18,   19,
		  20,   22,   23,   24,   26,   27,   29,   30,   32,   34,   36,   38,   40,   42,   44,   46,
		  49,   51,   53,   56,   59,   62,   65,   68,   71,   74,   77,   81,   84,   88,   92,   96,
		 100,  104,  109,  113,  118,  122,  127,  132,  137,  143,  148,  154,  160,  165,  171,  178,
		 184,  191,  197,  204,  211,  219,  226,  234,  241,  249,  257,  266,  274,  283,  292,  301,
		 310,  319,  329,  339,  349,  359,  369,  380,  391,  402,  413,  424,  436,  448,  460,  472,
		 484,  497,  510,  523,  536,  549,  563,  577,  591,  605,  619,  634,  648,  663,  679,  694,
	},
};

const u32 hold_masks[4] = { 0, 7, 31, 127 };

constexpr s32 wrap_add(s32 a, s32 b) { return s32(u32(a) + u32(b)); }

} // anonymous namespace


//-------------------------------------------------
//  ramps
//-------------------------------------------------

s32 roland_xp_device::exp_decode(s32 value) const
{
	if (value == 0)
		return 0;

	const int index = (value >> 6) & 0xff;
	const int fraction = value & 0x3f;
	s32 v = m_exp_table[index] * (64 - fraction) + m_exp_table[index + 1] * fraction;
	v = (v + (v < 0 ? 63 : 0)) >> 6;
	return v >> (15 - ((value >> 14) & 15));
}

u32 roland_xp_device::ramp::hold_mask() const
{
	return hold_masks[(control >> 12) & 3];
}

roland_xp_device::ramp_law roland_xp_device::ramp::control_law() const
{
	switch (control >> 14)
	{
	case 0: return LAW_EXPONENTIAL;
	case 1: return LAW_LINEAR;
	default: return LAW_S_CURVE;
	}
}

void roland_xp_device::ramp::seed(s32 value, ramp_law law)
{
	current = value;
	previous = value;
	counter = 0;
	accumulator = s32(u32(value) << 10);
	arm(law);
}

void roland_xp_device::ramp::retarget(s32 value, ramp_law law)
{
	target = value;
	accumulator = s32(u32(current) << 10);
	arm(law);
}

void roland_xp_device::ramp::configure(u16 value, ramp_law law)
{
	control = value;
	arm(law);
}

void roland_xp_device::ramp::configure(u16 value)
{
	control = value;
	arm(control_law());
}

void roland_xp_device::ramp::arm(ramp_law law)
{
	switch (law)
	{
	case LAW_LINEAR:
		step = s32((s64(target) - current) * rate() >> 13);
		if (step == 0 && target > current)
			step = 1;
		active = current != target;
		break;

	case LAW_EXPONENTIAL:
		active = current != target;
		break;

	case LAW_S_CURVE:
		target = 0;
		step = 0;
		midpoint = current / 2;
		active = current != 0;
		break;
	}
}

bool roland_xp_device::ramp::update(ramp_law law)
{
	previous = current;
	if (!active)
		return false;

	counter++;
	if (counter & hold_mask())
		return false;

	switch (law)
	{
	case LAW_LINEAR:
		current += step;
		current = (step > 0) ? std::min(current, target) : std::max(current, target);
		if (current == target)
		{
			step = 0;
			active = false;
		}
		break;

	case LAW_EXPONENTIAL:
	{
		const s32 error = s16((s32(u32(target) << 10) - accumulator) >> 13);
		s32 delta = error * rate();
		delta = (delta < 0) ? std::min(delta, -0x400) : std::max(delta, 0x400);
		accumulator += delta;
		current = accumulator >> 10;
		active = current != target;
		break;
	}

	case LAW_S_CURVE:
		if (current > midpoint)
		{
			step -= rate();
			current += step;
		}
		else
		{
			step += rate();
			current = (step > 0) ? 0 : current + step;
		}
		if (current == 0)
			active = false;
		break;
	}

	return !active;
}

s32 roland_xp_device::ramp::value_at(int phase, int period) const
{
	return previous + s32((s64(current - previous) * (phase + 1)) / period);
}

s16 roland_xp_device::ramp::coefficient() const
{
	return s16(std::clamp<s32>(current >> 3, -0x8000, 0x7fff));
}

s16 roland_xp_device::ramp::coefficient(int phase, int period) const
{
	return s16(std::clamp<s32>(value_at(phase, period) >> 3, -0x8000, 0x7fff));
}


//-------------------------------------------------
//  device
//-------------------------------------------------

roland_xp_device::roland_xp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, ROLAND_XP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_pram_config("pram", ENDIANNESS_BIG, 32, 10, -2, address_map_constructor(FUNC(roland_xp_device::pram_map), this))
	, m_wave_config("wave", ENDIANNESS_LITTLE, 8, 27)
	, m_iram_config("iram", ENDIANNESS_BIG, 32, 10, -2, address_map_constructor(FUNC(roland_xp_device::iram_map), this))
	, m_eram_config("eram", ENDIANNESS_BIG, 32, 18, -2, address_map_constructor(FUNC(roland_xp_device::eram_map), this))
	, m_int_callback(*this)
	, m_serial_out_cb(*this)
	, m_serial_in_cb(*this, 0)
	, m_stream(nullptr)
	, m_run_mask(0)
	, m_read_latch(0)
	, m_frame_counter(0)
	, m_noise(0)
	, m_irq_head(0)
	, m_irq_count(0)
	, m_int_state(false)
	, m_landing_valid(0)
	, m_program_dirty(true)
	, m_dsp_enabled(false)
	, m_icount(0)
	, m_pc(0)
	, m_serial_out_word{ 0, 0 }
	, m_serial_frame{ 0, 0 }
	, m_serial_in(0)
	, m_serial_phase(0)
{
}

void roland_xp_device::pram_map(address_map &map)
{
	map(0x000, 0x3ff).rw(FUNC(roland_xp_device::pram_r), FUNC(roland_xp_device::pram_w));
}

void roland_xp_device::iram_map(address_map &map)
{
	map(0x000, 0x3ff).rw(FUNC(roland_xp_device::iram_r), FUNC(roland_xp_device::iram_w));
}

void roland_xp_device::eram_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(FUNC(roland_xp_device::eram_r), FUNC(roland_xp_device::eram_w));
}

device_memory_interface::space_config_vector roland_xp_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_pram_config),
		std::make_pair(AS_WAVE, &m_wave_config),
		std::make_pair(AS_IRAM, &m_iram_config),
		std::make_pair(AS_ERAM, &m_eram_config)
	};
}

std::unique_ptr<util::disasm_interface> roland_xp_device::create_disassembler()
{
	return std::make_unique<roland_xp_disassembler>(this);
}

u32 roland_xp_device::pram_r(offs_t address)
{
	const int word = (PRAM_BASE >> 1) + (address & 0xff) * 2;
	return (u32(m_regs[word]) << 16) | m_regs[word | 1];
}

void roland_xp_device::pram_w(offs_t address, u32 data)
{
	const int word = (PRAM_BASE >> 1) + (address & 0xff) * 2;
	m_regs[word] = u16(data >> 16);
	m_regs[word | 1] = u16(data);
	m_program_dirty = true;
}

void roland_xp_device::device_start()
{
	m_regs = make_unique_clear<u16[]>(0x2000);
	m_eram = make_unique_clear<s32[]>(ERAM_SIZE);
	space(AS_WAVE).cache(m_wave_cache);
	m_stream = stream_alloc(0, OUTPUT_WORDS, clock() / 768, STREAM_SYNCHRONOUS);

	for (int i = 0; i < 256; i++)
		m_exp_table[i] = s32(std::floor(std::exp2(17.0 + i / 257.0)));
	m_exp_table[256] = 1 << 18;

	set_icountptr(m_icount);

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(0, "PC", m_pc);
	state_add(1, "ACC", m_dsp.acc);
	state_add(2, "R", m_dsp.r);
	state_add(3, "MEM", m_dsp.mem);
	state_add(4, "P", m_dsp.product);
	state_add(5, "LATCH", m_dsp.latch);
	state_add(6, "G", m_dsp.gain);
	state_add(7, "CURSOR", m_dsp.cursor);
	state_add(8, "F", m_dsp.fraction);

	save_pointer(NAME(m_regs), 0x2000);
	save_pointer(NAME(m_eram), ERAM_SIZE);
	save_item(NAME(m_iram));
	save_item(NAME(m_iram_ramping));
	save_item(NAME(m_dsp.acc));
	save_item(NAME(m_dsp.acc_before));
	save_item(NAME(m_dsp.acc_before_prev));
	save_item(NAME(m_dsp.product));
	save_item(NAME(m_dsp.product_prev));
	save_item(NAME(m_dsp.r));
	save_item(NAME(m_dsp.r_prev));
	save_item(NAME(m_dsp.mem));
	save_item(NAME(m_dsp.mem_prev));
	save_item(NAME(m_dsp.now));
	save_item(NAME(m_dsp.latch));
	save_item(NAME(m_dsp.gain));
	save_item(NAME(m_dsp.r_age));
	save_item(NAME(m_dsp.latch_age));
	save_item(NAME(m_dsp.now_valid));
	save_item(NAME(m_dsp.fraction));
	save_item(NAME(m_dsp.cursor));
	save_item(NAME(m_landing));
	save_item(NAME(m_landing_valid));
	save_item(NAME(m_dsp_enabled));
	save_item(NAME(m_pc));
	save_item(NAME(m_serial_out_word));
	save_item(NAME(m_serial_frame));
	save_item(NAME(m_serial_in));
	save_item(NAME(m_serial_phase));
	save_item(NAME(m_bus));
	save_item(NAME(m_run_mask));
	save_item(NAME(m_read_latch));
	save_item(NAME(m_frame_counter));
	save_item(NAME(m_noise));
	save_item(NAME(m_irq_queue));
	save_item(NAME(m_irq_head));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_int_state));

	save_item(STRUCT_MEMBER(m_voices, region));
	save_item(STRUCT_MEMBER(m_voices, start));
	save_item(STRUCT_MEMBER(m_voices, address));
	save_item(STRUCT_MEMBER(m_voices, loop));
	save_item(STRUCT_MEMBER(m_voices, end));
	save_item(STRUCT_MEMBER(m_voices, start_pending));
	save_item(STRUCT_MEMBER(m_voices, alternate));
	save_item(STRUCT_MEMBER(m_voices, reverse));
	save_item(STRUCT_MEMBER(m_voices, backward));
	save_item(STRUCT_MEMBER(m_voices, reading));
	save_item(STRUCT_MEMBER(m_voices, loop_reported));
	save_item(STRUCT_MEMBER(m_voices, done_reported));
	save_item(STRUCT_MEMBER(m_voices, sub_phase));
	save_item(STRUCT_MEMBER(m_voices, predictor));
	save_item(STRUCT_MEMBER(m_voices, filter_low));
	save_item(STRUCT_MEMBER(m_voices, filter_band));

	for (int n = 0; n < MAX_VOICES; n++)
	{
		ramp *const ramps[5] = { &m_voices[n].pitch, &m_voices[n].tvf, &m_voices[n].reso, &m_voices[n].tva1, &m_voices[n].tva2 };
		for (int k = 0; k < 5; k++)
		{
			ramp &r = *ramps[k];
			const int index = n * 5 + k;
			save_item(NAME(r.current), index);
			save_item(NAME(r.previous), index);
			save_item(NAME(r.target), index);
			save_item(NAME(r.step), index);
			save_item(NAME(r.accumulator), index);
			save_item(NAME(r.midpoint), index);
			save_item(NAME(r.control), index);
			save_item(NAME(r.counter), index);
			save_item(NAME(r.active), index);
		}
	}
}

void roland_xp_device::device_reset()
{
	std::fill_n(m_regs.get(), 0x2000, 0);
	std::fill_n(m_eram.get(), ERAM_SIZE, 0);
	std::fill(std::begin(m_bus), std::end(m_bus), 0);
	std::fill(std::begin(m_iram), std::end(m_iram), 0);
	std::fill(std::begin(m_iram_ramping), std::end(m_iram_ramping), 0);
	for (voice &v : m_voices)
		v = voice();
	m_dsp = dsp_state();
	m_dsp.latch_age = 0xff;
	m_dsp.r_age = 0xff;
	std::fill(std::begin(m_landing), std::end(m_landing), 0);
	m_landing_valid = 0;
	m_program_dirty = true;
	m_dsp_enabled = false;
	m_pc = 0;

	m_run_mask = 0;
	m_read_latch = 0;
	m_frame_counter = 0;
	m_noise = 0x2545f491;
	m_irq_head = 0;
	m_irq_count = 0;
	update_int();
}

void roland_xp_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 768);
}

void roland_xp_device::device_post_load()
{
	m_program_dirty = true;
}

void roland_xp_device::update_int()
{
	const bool state = m_irq_count != 0;
	if (state != m_int_state)
	{
		m_int_state = state;
		m_int_callback(state ? ASSERT_LINE : CLEAR_LINE);
	}
}

void roland_xp_device::raise_irq(int voice, int reason)
{
	if (m_irq_count == MAX_VOICES)
		return;

	m_irq_queue[(m_irq_head + m_irq_count) % MAX_VOICES] = u16((voice << 8) | reason);
	m_irq_count++;
	update_int();
}


//-------------------------------------------------
//  host interface
//-------------------------------------------------

u32 roland_xp_device::page(int voice, int index) const
{
	const int word = ((index & 0xff) << 7) | ((voice & 63) << 1);
	return (u32(m_regs[word]) << 16) | m_regs[word | 1];
}

bool roland_xp_device::ramp_current(const voice &v, int index, u32 &value) const
{
	switch (index)
	{
	case PITCH_SEED: value = u32(v.pitch.current); return true;
	case TVF_SEED:   value = u32(v.tvf.current); return true;
	case TVA2_SEED:  value = u32(v.tva2.current); return true;
	case TVA1_SEED:  value = u32(v.tva1.current); return true;
	case RESO_SEED:  value = u32(v.reso.current) << 2; return true;
	default:         return false;
	}
}

// offset is the word index; the byte address is offset << 1
void roland_xp_device::load_latch(offs_t address)
{
	const voice &v = m_voices[(address >> 2) & 63];

	if (address >= IRAM3_BASE + 8 && address < IRAM3_BASE + 12)
		m_read_latch = m_noise;
	else if (address < CRAM_BASE && ramp_current(v, address >> 8, m_read_latch))
		;
	else if (address >= CRAM_BASE && address < IRAM_BASE)
		m_read_latch = m_regs[address >> 1];
	else if (address >= IRAM_BASE && address < IRAM3_TARGET_BASE)
		m_read_latch = u32(m_iram[iram_word(address)]);
	else
		m_read_latch = (u32(m_regs[(address >> 1) & ~1]) << 16) | m_regs[(address >> 1) | 1];
}

u16 roland_xp_device::read(offs_t offset, u16 mem_mask)
{
	const offs_t address = (offset << 1) & 0x3ffe;
	u16 data = m_regs[address >> 1];

	if (address < RUN_MASK)
	{
		if (!machine().side_effects_disabled())
			load_latch(address);
	}
	else if (address < SEND_BASE)
	{
		switch (address)
		{
		case READBACK_LOW:
			data = m_read_latch & 0xffff;
			break;
		case READBACK_HIGH:
			data = m_read_latch >> 16;
			break;
		case IRQ_STATUS:
			data = m_irq_count ? m_irq_queue[m_irq_head] : 0;
			break;
		case IRQ_ACK:
			if (!machine().side_effects_disabled())
			{
				if (m_irq_count)
				{
					m_irq_head = (m_irq_head + 1) % MAX_VOICES;
					m_irq_count--;
				}
				update_int();
			}
			data = 0;
			break;
		}
	}
	else if (address >= ROM_WINDOW)
	{
		const u32 word = (u32(m_regs[ROM_BANK >> 1] & 0x7f) << 20) | (u32(m_regs[ROM_PAGE >> 1] & 0x7ff) << 9) | ((address - ROM_WINDOW) >> 1);
		data = m_wave_cache.read_byte(word * 2) | (u16(m_wave_cache.read_byte(word * 2 + 1)) << 8);
	}

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_READ, "%s: read %04x = %04x\n", machine().describe_context(), address, data);
	return data;
}

void roland_xp_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t address = (offset << 1) & 0x3ffe;
	COMBINE_DATA(&m_regs[address >> 1]);

	if (address < CRAM_BASE)
	{
		LOGMASKED(LOG_VOICE, "%s: voice %2d page %02x%s = %04x\n", machine().describe_context(), (address >> 2) & 0x3f, address >> 8, BIT(address, 1) ? "+2" : "  ", data);
		if (BIT(address, 1))
		{
			const int voice = (address >> 2) & 63;
			const int index = address >> 8;
			write_page(voice, index, page(voice, index));
		}
	}
	else if (address < IRAM_BASE)
	{
		LOGMASKED(LOG_CRAM, "%s: cram %3d = %04x\n", machine().describe_context(), (address - CRAM_BASE) >> 1, data);
		m_program_dirty = true;
	}
	else if (address < IRAM3_TARGET_BASE)
	{
		LOGMASKED(LOG_IRAM, "%s: iram%d %2d%s = %04x\n", machine().describe_context(), ((address - IRAM_BASE) >> 8) + 1, (address & 0xff) >> 2, BIT(address, 1) ? "+2" : "  ", data);
		if (BIT(address, 1))
			write_iram(iram_word(address), (u32(m_regs[(address >> 1) & ~1]) << 16) | m_regs[address >> 1]);
	}
	else if (address < PRAM_BASE)
	{
		LOGMASKED(LOG_IRAM, "%s: iram4 %2d   = %04x\n", machine().describe_context(), (address & 0xff) >> 1, data);
		write_iram_target(0xc0 + ((address & 0xff) >> 1), m_regs[address >> 1]);
	}
	else if (address < RUN_MASK)
	{
		LOGMASKED(LOG_PRAM, "%s: pram %3d%s = %04x\n", machine().describe_context(), (address - PRAM_BASE) >> 2, BIT(address, 1) ? "+2" : "  ", data);
		m_program_dirty = true;
	}
	else if (address < SEND_BASE)
	{
		if (address < 0x3908)
		{
			LOGMASKED(LOG_RUNMASK, "%s: runmask %04x = %04x\n", machine().describe_context(), address, data);
			write_run_mask((address - RUN_MASK) >> 1, m_regs[address >> 1]);
		}
		else if (address < 0x3910 || address == 0x3914 || address == 0x3924 || address == 0x3926 || address == 0x3932)
		{
			LOGMASKED(LOG_UNKNOWN, "%s: unknown %04x = %04x\n", machine().describe_context(), address, data);
		}
		else
		{
			LOGMASKED(LOG_CTRL, "%s: ctrl %04x = %04x\n", machine().describe_context(), address, data);
		}
	}
	else if (address < ROM_WINDOW)
		LOGMASKED(LOG_MIX, "%s: mix bank %d voice %2d = %04x\n", machine().describe_context(), (address - SEND_BASE) >> 7, (address >> 1) & 0x3f, data);
	else
		LOGMASKED(LOG_CTRL, "%s: window write %04x = %04x\n", machine().describe_context(), address, data);
}

void roland_xp_device::write_page(int n, int index, u32 value)
{
	voice &v = m_voices[n];

	switch (index)
	{
	case CONTROL:
		if (BIT(value, 15))
			v.start_pending = 1;
		break;

	case PITCH_SEED:    v.pitch.seed(s32(value), LAW_LINEAR); break;
	case PITCH_TARGET:  v.pitch.retarget(s32(value), LAW_LINEAR); break;
	case PITCH_CONTROL: v.pitch.configure(u16(value), LAW_LINEAR); break;

	case TVF_SEED:      v.tvf.seed(exp_decode(s32(value)), LAW_LINEAR); break;
	case TVF_TARGET:    v.tvf.retarget(exp_decode(s32(value)), LAW_LINEAR); break;
	case TVF_CONTROL:   v.tvf.configure(u16(value), LAW_LINEAR); break;

	case RESO_SEED:    v.reso.seed(s32(value) >> 2, LAW_EXPONENTIAL); break;
	case RESO_TARGET:  v.reso.retarget(s32(value), LAW_EXPONENTIAL); break;
	case RESO_CONTROL: v.reso.configure(u16(value), LAW_EXPONENTIAL); break;

	case TVA2_SEED:     v.tva2.seed(s32(value), LAW_EXPONENTIAL); break;
	case TVA2_TARGET:   v.tva2.retarget(s32(value), LAW_EXPONENTIAL); break;
	case TVA2_CONTROL:  v.tva2.configure(u16(value), LAW_EXPONENTIAL); break;

	case TVA1_SEED:     v.tva1.seed(s32(value), v.tva1.control_law()); break;
	case TVA1_TARGET:   v.tva1.retarget(s32(value), v.tva1.control_law()); break;
	case TVA1_CONTROL:
		v.tva1.configure(u16(value));
		if (v.tva1.control_law() == LAW_S_CURVE && !v.tva1.active && !v.done_reported)
		{
			v.done_reported = 1;
			raise_irq(n, IRQ_VOICE_DONE);
		}
		break;

	default:
		break;
	}
}

void roland_xp_device::write_run_mask(int word, u16 data)
{
	const u64 before = m_run_mask;
	m_run_mask = (m_run_mask & ~(u64(0xffff) << (word * 16))) | (u64(data) << (word * 16));

	for (int n = word * 16; n < word * 16 + 16; n++)
	{
		if (BIT(before, n) && !BIT(m_run_mask, n))
		{
			voice &v = m_voices[n];
			v.reading = 0;
			v.predictor = 0;
			v.filter_low = 0;
			v.filter_band = 0;
		}
	}
}


//-------------------------------------------------
//  address generator and DPCM
//-------------------------------------------------

s32 roland_xp_device::delta_at(const voice &v, u32 address)
{
	const s8 delta = s8(rom_byte(v, address));
	const u8 shifts = rom_byte(v, address >> 5);
	const int shift = BIT(address, 4) ? (shifts >> 4) : (shifts & 0x0f);
	return s32(u32(s32(delta)) << (shift + 10));
}

void roland_xp_device::start_reader(int n)
{
	voice &v = m_voices[n];
	const u32 control = page(n, CONTROL);

	v.start_pending = 0;
	v.region = control & 0x7f;
	v.alternate = BIT(control, 12);
	v.reverse = BIT(control, 11);
	v.start = page(n, ADDRESS) & 0xfffff;
	v.loop = page(n, LOOP) & 0xfffff;
	v.end = page(n, END) & 0xfffff;

	v.backward = v.reverse;
	v.address = v.reverse ? v.end : v.start;
	v.sub_phase = 0;
	v.reading = 1;
	v.loop_reported = 0;
	v.done_reported = 0;
	v.filter_low = 0;
	v.filter_band = 0;

	v.predictor = 0;
	for (u32 a = v.address & ~0x1f; a < v.address; a++)
		v.predictor = wrap_add(v.predictor, delta_at(v, a));
}

roland_xp_device::address_step roland_xp_device::advance(const voice &v, address_step s) const
{
	const bool looping = v.loop < v.end;

	if (!s.backward)
	{
		if (!looping)
		{
			if (s.address + 1 >= v.end)
				return { s.address, false, true };
			return { s.address + 1, false, false };
		}
		if (s.address >= v.end)
		{
			if (v.alternate)
				return { s.address, true, false };
			return { v.loop, false, false };
		}
		return { s.address + 1, false, false };
	}

	const u32 bound = (v.alternate && looping) ? v.loop : (v.reverse ? v.start : v.loop);
	if (s.address <= bound)
	{
		if (v.alternate && looping)
			return { s.address, false, false };
		return { s.address, true, true };
	}
	return { s.address - 1, true, false };
}


//-------------------------------------------------
//  the voice
//-------------------------------------------------

void roland_xp_device::run_voice(int n)
{
	voice &v = m_voices[n];
	if (!running(n))
		return;

	if (v.start_pending)
		start_reader(n);

	if ((m_frame_counter & 7) == 0)
	{
		if (v.pitch.update(LAW_LINEAR) && v.pitch.current == 0)
		{
			v.reading = 0;
			v.predictor = 0;
			v.done_reported = 1;
		}
		v.tvf.update(LAW_LINEAR);
		v.reso.update(LAW_EXPONENTIAL);
		v.tva2.update(LAW_EXPONENTIAL);
	}

	if ((m_frame_counter & 1) == 0)
	{
		const bool arrived = v.tva1.update(v.tva1.control_law());

		if (arrived && v.tva1.current == 0 && !v.done_reported)
		{
			v.done_reported = 1;
			raise_irq(n, IRQ_VOICE_DONE);
		}
	}

	s32 sample = 0;
	if (v.reading)
	{
		const int phase = v.sub_phase >> 9;
		address_step s{ v.address, bool(v.backward), false };
		s64 weighted = 0;
		for (int i = 0; i < 3 && !s.stopped; i++)
		{
			weighted += s64(interp_weights[i][phase]) * delta_at(v, s.address);
			s = advance(v, s);
		}
		sample = wrap_add(v.predictor, s32(weighted >> 12)) >> 4;

		const u32 phase_sum = u32(v.sub_phase) + u32(exp_decode(v.pitch.value_at(m_frame_counter & 7, 8)));
		v.sub_phase = phase_sum & 0xffff;
		for (u32 carry = phase_sum >> 16; carry && v.reading; carry--)
		{
			v.predictor = wrap_add(v.predictor, delta_at(v, v.address));
			const address_step next = advance(v, { v.address, bool(v.backward), false });
			if (next.stopped)
			{
				v.reading = 0;
				v.predictor = 0;
				if (!v.done_reported)
				{
					v.done_reported = 1;
					raise_irq(n, IRQ_VOICE_DONE);
				}
				break;
			}
			v.address = next.address;
			v.backward = next.backward;

			if (!v.loop_reported && (v.backward ? v.address <= v.loop : v.address >= v.loop))
			{
				v.loop_reported = 1;
				raise_irq(n, IRQ_LOOP_REACHED);
			}
		}
	}

	const u32 filter = page(n, FILTER);
	if (!(filter & 0xf000))
	{
		const s32 f = v.tvf.coefficient(m_frame_counter & 7, 8);
		const s32 q = v.reso.coefficient(m_frame_counter & 7, 8);
		v.filter_low += s32((s64(f) * v.filter_band) >> 14);
		const s32 high = sample - (s32((s64(q) * v.filter_band) >> 14) + v.filter_low);
		v.filter_band += s32((s64(f) * high) >> 14);
		switch ((filter >> 10) & 3)
		{
		case 0: sample = v.filter_low; break;
		case 1: sample = v.filter_band; break;
		case 2: sample = high; break;
		case 3: sample = v.filter_low - high; break;
		}
	}

	sample = s32((s64(sample) * v.tva1.coefficient(m_frame_counter & 1, 2)) >> 14);
	sample = s32((s64(sample) * v.tva2.coefficient(m_frame_counter & 7, 8)) >> 14);

	for (int bank = 0; bank < 4; bank++)
	{
		const u16 s = send(n, bank);
		m_bus[s & 63] += s32((s64(sample) * (s >> 6)) >> 10);
	}
}

//-------------------------------------------------
//  the DSP
//-------------------------------------------------

void roland_xp_device::write_iram(int word, u32 value)
{
	m_iram[word & 0xff] = s32(value << 8) >> 8;
	m_iram_ramping[word & 0xff] = 0;
}

void roland_xp_device::write_iram_target(int word, u16 value)
{
	m_iram_ramping[word & 0xff] = 1;
}

void roland_xp_device::update_iram_ramps()
{
	for (int word = 0xc0; word < IRAM_SIZE; word++)
	{
		if (!m_iram_ramping[word])
			continue;

		const s32 target = s32(m_regs[(IRAM3_TARGET_BASE >> 1) + (word - 0xc0)]) << 13;
		const s32 rate = m_regs[(IRAM3_RATE >> 1) + ((word - 0xc0) >> 4)];
		s32 &current = m_iram[word];
		if (current < target)
			current = std::min(current + rate, target);
		else
			current = std::max(current - rate, target);
		if (current == target)
			m_iram_ramping[word] = 0;
	}
}

void roland_xp_device::decode_program()
{
	static const u8 shifts[4] = { 0, 1, 2, 4 };

	for (int i = 0; i < DSP_SLOTS; i++)
	{
		const u32 w = (u32(m_regs[(PRAM_BASE >> 1) + i * 2]) << 16) | m_regs[(PRAM_BASE >> 1) + i * 2 + 1];
		const u16 c = m_regs[(CRAM_BASE >> 1) + i];
		const s32 mantissa = s32(s16(c << 2)) >> 2;
		dsp_slot &s = m_program[i];
		s.st = (w >> 14) & 3;
		s.word = (w >> 6) & 0xff;
		s.col = w & 0x3f;
		s.ext = (w >> 25) & 7;
		s.eram_op = 0;
		s.eram_offset = 0;
		s.read_bypass = s.col == 0x30 && (c & 0x3e3f) == 0x0221;
		s.cram = c;
		s.coefficient = mantissa << shifts[c >> 14];
		s.raw = BIT(c, 15) ? s32((c & 0x3fff) << 13) : mantissa;
	}

	for (int i = 0; i < DSP_SLOTS - 1; )
	{
		const u32 w = (u32(m_regs[(PRAM_BASE >> 1) + i * 2]) << 16) | m_regs[(PRAM_BASE >> 1) + i * 2 + 1];
		const u32 next = (u32(m_regs[(PRAM_BASE >> 1) + i * 2 + 2]) << 16) | m_regs[(PRAM_BASE >> 1) + i * 2 + 3];
		const int op = (w >> 23) & 3;
		if (op)
		{
			m_program[i].eram_op = op;
			m_program[i].eram_offset = (((w >> 16) & 0x7f) << 9) | ((next >> 16) & 0x1ff);
			i += 2;
		}
		else
			i++;
	}

	m_program_dirty = false;
}

s32 roland_xp_device::operand(const dsp_slot &s) const
{
	const dsp_state &d = m_dsp;

	switch (s.col >> 4)
	{
	case 1:
		return d.acc;

	case 3:
		if (d.latch_age < 4)
			return d.latch;
		if (s.st == 1)
			return d.now_valid ? d.now : 0;
		if (s.st == 2)
			return d.latch;
		break;
	}

	return (d.r_age <= 3) ? d.r_prev : 0;
}

void roland_xp_device::execute(const dsp_slot &s)
{
	dsp_state &d = m_dsp;
	const int nibble = s.col & 0xf;
	const s32 o = operand(s);

	if (s.col == 0x30 && s.cram <= 0x000f)
	{
		d.product = d.now_valid ? d.now : 0;
		switch (s.cram & 0xf)
		{
		case 0:
		case 2: d.acc += d.mem_prev; break;
		case 4: d.acc = d.mem_prev; break;
		case 5: d.acc = d.product_prev; break;
		case 9: d.acc = d.mem_prev + d.product_prev; break;
		default: d.acc += d.product_prev; break;
		}
		return;
	}

	if (s.col == 0x30 && (s.cram & 0x3e80) == 0x0280)
	{
		const int code = s.cram & 0xf;
		const int select = (s.cram >> 4) & 3;

		if (code == 5)
		{
			s32 value;
			if (select == 1)
				value = (s.st == 3) ? d.acc : d.acc_before_prev;
			else if (select == 2)
				value = d.mem_prev;
			else
				value = (d.latch_age < 4 || d.now_valid) ? o : d.mem_prev;
			d.product = multiply(value, d.gain);
			d.acc = d.product_prev;
			return;
		}

		if (code == 3 && select == 2)
			d.product = multiply(d.mem_prev, d.gain);
		else if (code == 1 && select == 2)
			d.product = multiply(d.now_valid ? d.now : (d.r_age <= 3) ? d.r_prev : 0, d.gain);
		else if ((code == 1 || code == 3) && select == 3)
			d.product = multiply(o, d.gain);
		else if (code == 1 || code == 2 || code == 3)
			d.product = multiply(d.acc, d.gain);
		else
			d.product = o;

		if (code == 2 || code == 3)
			d.acc += d.product_prev;
		else
			d.acc = multiply(d.acc, d.gain) + d.product_prev;
		return;
	}

	if (s.col == 0x30 && (s.cram == 0x1001 || s.cram == 0x2801 || s.cram == 0x0231 || s.cram == 0x0325))
	{
		d.product = 0;
		switch (s.cram)
		{
		case 0x1001: d.acc = wrap24(d.acc); break;
		case 0x2801: d.acc = std::abs(wrap24(d.acc)); break;
		case 0x0231:
		{
			const s32 first = d.now_valid ? d.now : 0;
			d.acc = first + s32((s64(d.latch - first) * d.fraction) >> 12);
			break;
		}
		default: break;
		}
		return;
	}

	if (s.col == 0x0c)
	{
		d.product = multiply(m_serial_in, s.coefficient);
		return;
	}

	d.product = (nibble == 0 && s.col != 0x30) ? o : multiply(o, s.coefficient);
	const s32 p = d.product_prev;

	switch (s.col)
	{
	case 0x11:
		if (d.now_valid)
			d.acc = d.now;
		return;
	case 0x14:
		d.acc = d.mem_prev;
		return;
	case 0x0f:
		d.acc += s.raw;
		return;
	case 0x1f:
		d.acc = d.mem_prev + s.raw;
		return;
	case 0x2f:
		d.acc = p + s.raw;
		return;
	case 0x20:
	case 0x21:
		return;
	case 0x32:
		d.acc += p;
		return;
	default:
		break;
	}

	switch (nibble)
	{
	case 0:
	case 3:
		d.acc += p;
		break;
	case 5:
		d.acc = p;
		break;
	case 9:
		d.acc = d.mem_prev + p;
		break;
	default:
		break;
	}
}

void roland_xp_device::exchange_serial()
{
	if (!m_serial_out_cb.isunset())
	{
		for (int channel = 0; channel < 2; channel++)
			m_serial_out_cb(channel, u32(output_word(m_serial_out_word[channel])));
	}

	if (!m_serial_in_cb.isunset())
	{
		for (int channel = 0; channel < 2; channel++)
			m_serial_frame[channel] = wrap24(s32(m_serial_in_cb(channel)));
	}
	m_serial_phase = 0;
}

void roland_xp_device::dsp_frame_start()
{
	m_dsp_enabled = BIT(m_regs[DSP_MODE >> 1], 1);
	if (!m_dsp_enabled)
		return;

	if (m_program_dirty)
		decode_program();

	update_iram_ramps();

	for (int n = 0; n < BUS_COUNT; n++)
		m_iram[0x40 + n] = clamp24(m_bus[n] >> DSP_INPUT_SHIFT);

	exchange_serial();

	m_dsp.acc_before_prev = m_dsp.acc;
	m_dsp.product_prev = 0;
}

void roland_xp_device::dsp_step()
{
	if (!m_pc)
		dsp_frame_start();

	if (m_dsp_enabled)
	{
		const dsp_slot &s = m_program[m_pc];
		dsp_state &d = m_dsp;

		if (BIT(m_landing_valid, 0))
		{
			d.latch = m_landing[0];
			d.latch_age = 0;
		}
		m_landing[0] = m_landing[1];
		m_landing_valid >>= 1;

		if (s.eram_op == 1)
		{
			m_landing[1] = m_eram[(s.eram_offset + d.cursor) & 0xffff];
			m_landing_valid |= 2;
		}
		if (s.col == 0x20)
		{
			m_landing[1] = m_eram[(d.cursor + (d.acc >> 12)) & 0xffff];
			m_landing_valid |= 2;
			d.fraction = d.acc & 0xfff;
		}

		if (s.ext == 1)
			m_serial_in = m_serial_frame[m_serial_phase++ & 1];

		d.r_prev = d.r;
		d.mem_prev = d.mem;
		if (d.r_age < 0xff)
			d.r_age++;
		d.now_valid = 0;

		s32 gain_pending = 0;
		bool gain_arrives = false;
		if (s.st == 1)
		{
			const s32 v = m_iram[s.word];
			if (s.word >= 0xf0)
			{
				gain_pending = v >> 8;
				gain_arrives = true;
			}
			else
			{
				d.now = v;
				d.now_valid = 1;
				d.r = v;
				d.r_age = 0;
				if (!s.read_bypass)
					d.mem = v;
			}
		}

		d.acc_before = d.acc;
		execute(s);

		if (gain_arrives)
			d.gain = gain_pending;
		if (d.latch_age < 0xff)
			d.latch_age++;

		if (s.st == 2)
			m_iram[s.word] = d.latch;
		else if (s.st == 3)
			m_iram[s.word] = clamp24(d.acc_before);

		if (s.eram_op == 3)
			m_eram[(s.eram_offset + d.cursor) & 0xffff] = clamp24(d.acc_before);
		else if (s.eram_op == 2)
			m_eram[(s.eram_offset + d.cursor) & 0xffff] = d.r_prev;

		d.acc_before_prev = d.acc_before;
		d.product_prev = d.product;
	}

	if (++m_pc >= DSP_SLOTS)
	{
		m_pc = 0;
		if (m_dsp_enabled)
			m_dsp.cursor--;
	}
}

void roland_xp_device::run_voices()
{
	std::fill(std::begin(m_bus), std::end(m_bus), 0);

	for (int n = 0; n < MAX_VOICES; n++)
		run_voice(n);

	m_frame_counter++;
	m_noise ^= m_noise << 13;
	m_noise ^= m_noise >> 17;
	m_noise ^= m_noise << 5;
}

void roland_xp_device::execute_run()
{
	while (m_icount > 0)
	{
		if (!m_pc)
			run_voices();

		debugger_instruction_hook(m_pc);
		dsp_step();
		m_icount--;
	}
}

void roland_xp_device::sound_stream_update(sound_stream &stream)
{
	for (int n = 0; n < OUTPUT_WORDS; n++)
		stream.put_int(n, 0, output_word(n), 1 << 23);
}
