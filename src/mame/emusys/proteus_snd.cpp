// license:BSD-3-Clause
// copyright-holders:Grant McNaught
/***************************************************************************

    E-mu Proteus/1 sound generator (custom chip, 32 voices)

    Reconstructed from the Proteus/1 XR firmware (v2.00 / v2.30). Labels:
    [O] read from firmware code, [I] inferred, [U] unknown.

    Output rate: 39062.5 Hz = 10 MHz / 256 [O: frequency-response test pitches
    for 10/18/19 kHz on the 64-sample test sine all give 39062 Hz].

    Per-voice registers (CPU 0x800000 + voice * 0x20, written as 32-bit longs,
    high word first):
      +0x00  current ROM word address, 24 bits. Firmware writes sample start - 4 [O].
      +0x04  pitch: bits 17..0 = signed (two's complement) increment in words per
             output sample, 12 fraction bits [O]. Reverse Sound negates it
             (neg.l; andi.l #$3ffff at 0x49f2). Bit 18 set only by the ROM tests:
             [I] "step on CPU read" (the voice advances once per read of +0x18
             instead of per sample). Bits 23..21: tone; the firmware writes 0..5 [O: 0x3b8a..0x3bc0,
             0x33e8], per instrument plus one step per 5 semitones of upward
             transposition. It selects the interpolation kernel (see below).
      +0x08  envelope: bits 23..12 = target level (12-bit float), bits 11..9 mode,
             bits 8..0 = rate, a signed 9-bit value [O: 0x4882..0x4896]. Mode bit 9
             set: [I] jump to target immediately (written at note-on with target 0).
             Otherwise [I] level moves toward target by |rate| (at least 1) per
             output sample; the direction comes from the target, not the sign.
      +0x0c  [U] (0xfff000 in ROM tests); stored only.
      +0x10  loop start (= sample loop start - 4) [O]
      +0x14  NOT(loop end) (loop end = sample loop end - 3) [O]
      +0x18  read: ROM word at the current position (used by the Sound ROM test) [O]
      +0x1c  [U] (voice 2 +0x1c = 0x27 at init: global control?); stored only.

    Looping [I]: when the address passes loop end in the direction of travel,
    subtract (loop end - loop start). Non-looping samples have their loop placed
    in the zero padding after the sample. Reversed voices [O: 0x39b0..0x39da]
    start at sample end - 0x64 and get loop end = previous sample end + 9,
    loop start = loop end + 0x18, i.e. a backward loop in the zero gap before
    the sample.

    Interpolation: 7-point, as in E-mu's patent US 5,111,727 (Rossum, filed
    1990-01-05): the output is the sum of the 7 words around the current position
    (window centred on the nearest word) times coefficients from a stored single-
    sided impulse response (112 points, 32 per sample period, mirrored, linearly
    interpolated by the low fraction bits). 7 points leave 2 of the patent's 16
    cycles per voice for volume and mixing: 32 voices x 16 x 39062.5 Hz = 20 MHz,
    the chip clock [I]. The patent's "family of six filters" of lower cutoff is
    selected by the tone value: tone 0 = the patent's Table 1 kernel [O], tones 1..7
    designed to match it, scaled by 5 semitones per step [I]
    (proteus_snd_kernels.ipp). Current position = address + 4 + fraction (the
    firmware programs start - 4); 8 words address+1 .. address+8 are read and the
    tap 3.5 or more words away has weight 0.

    Level [I]: 12-bit float, exponent = bits 11..9, mantissa = bits 8..0;
    gain = exp ? (0x200 | m) << (exp - 1) : m, full scale 0x3ff << 6 (Q16).
    Derived from the firmware's level tables: 0x200 level units per 6 dB.

    Pan byte (CPU 0x500009 + 2 * voice) [I]: bits 5..4 output pair (main, sub 1,
    sub 2), bits 3..0 position, 0xF = left, 0x0 = right; linear. All pairs are
    mixed to the stereo output here.

***************************************************************************/

#include "emu.h"
#include "proteus_snd.h"

#include <algorithm>
#include <cstdlib>

#include "proteus_snd_kernels.ipp"

#define LOG_REGS (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(PROTEUS_SND, proteus_snd_device, "proteus_snd", "E-mu Proteus sound generator")

proteus_snd_device::proteus_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PROTEUS_SND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_waves(*this, DEVICE_SELF)
	, m_stream(nullptr)
{
}

void proteus_snd_device::device_start()
{
	// rom_word() wraps addresses with a mask
	if (m_waves.length() & (m_waves.length() - 1))
		throw emu_fatalerror("%s: sound region size must be a power of two", tag());

	m_stream = stream_alloc(0, 2, clock() / 256);

	save_item(NAME(m_regs));
	save_item(STRUCT_MEMBER(m_voice, addr));
	save_item(STRUCT_MEMBER(m_voice, frac));
	save_item(STRUCT_MEMBER(m_voice, pitch));
	save_item(STRUCT_MEMBER(m_voice, env));
	save_item(STRUCT_MEMBER(m_voice, loop_start));
	save_item(STRUCT_MEMBER(m_voice, loop_end));
	save_item(STRUCT_MEMBER(m_voice, level));
	save_item(STRUCT_MEMBER(m_voice, pan));
}

void proteus_snd_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	for (voice &vc : m_voice)
		vc = voice();
}

u32 proteus_snd_device::reg_long(unsigned v, unsigned word) const
{
	return (u32(m_regs[v * WORDS_PER_VOICE + word]) << 16) | m_regs[v * WORDS_PER_VOICE + word + 1];
}

s16 proteus_snd_device::rom_word(u32 addr) const
{
	return s16(m_waves[addr & (m_waves.length() - 1)]);
}

u32 proteus_snd_device::level_to_gain(u16 level)
{
	const u32 e = (level >> 9) & 7;
	const u32 m = level & 0x1ff;
	return e ? ((0x200 | m) << (e - 1)) : m;
}

s32 proteus_snd_device::interpolate(const voice &vc) const
{
	// 8 taps: words addr+1 .. addr+8 around the position addr + 4 + frac / 4096
	const s32 *const k = proteus_kernels[(vc.pitch >> 21) & 7];
	const s32 f = vc.frac & 0xfff;
	s64 acc = 0;
	for (int j = 0; j < 8; j++)
	{
		const s32 u = (j <= 3) ? (((3 - j) << 12) + f) : (((j - 3) << 12) - f);
		const s32 a = u >> 7, w = u & 127;
		const s32 k1 = (a < 128) ? k[a + 1] : 0;
		const s32 c = k[a] + (((k1 - k[a]) * w) >> 7);
		acc += s64(rom_word(vc.addr + 1 + j)) * c;
	}
	return std::clamp<s32>(s32(acc >> 16), -65536, 65535);
}

void proteus_snd_device::step(voice &vc)
{
	// 18-bit signed increment: integer part bits 17..12 (signed), fraction bits 11..0
	const bool reverse = BIT(vc.pitch, 17);
	const s32 whole = util::sext((vc.pitch >> 12) & 0x3f, 6);
	const u32 pos = (vc.frac & 0xfff) + (vc.pitch & 0xfff);
	vc.frac = pos & 0xfff;
	vc.addr = (vc.addr + u32(whole) + (pos >> 12)) & ADDR_MASK;
	const bool passed = reverse
			? (vc.addr < vc.loop_end && vc.loop_end < vc.loop_start)
			: (vc.addr > vc.loop_end && vc.loop_end >= vc.loop_start);
	if (passed)
		vc.addr = (vc.addr - (vc.loop_end - vc.loop_start)) & ADDR_MASK;
}

u16 proteus_snd_device::read(offs_t offset, u16 mem_mask)
{
	const unsigned v = offset >> 4;
	const unsigned r = offset & 15;
	if (r == 0x18 / 2)
	{
		m_stream->update();
		voice &vc = m_voice[v];
		const u16 data = u16(rom_word(vc.addr + 4));
		if (!machine().side_effects_disabled() && BIT(vc.pitch, 18))
			step(vc);
		return data;
	}
	LOGMASKED(LOG_REGS, "%s: read  v%02u +%02x -> %04x\n", machine().describe_context(), v, r << 1, m_regs[offset]);
	return m_regs[offset];
}

void proteus_snd_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();
	COMBINE_DATA(&m_regs[offset]);
	const unsigned v = offset >> 4;
	const unsigned r = offset & 15;
	voice &vc = m_voice[v];
	LOGMASKED(LOG_REGS, "%s: write v%02u +%02x <- %04x\n", machine().describe_context(), v, r << 1, data);

	switch (r)
	{
	case 0x00 / 2: case 0x02 / 2:
		vc.addr = reg_long(v, 0) & ADDR_MASK;
		vc.frac = 0;
		break;
	case 0x04 / 2: case 0x06 / 2:
		vc.pitch = reg_long(v, 2);
		break;
	case 0x08 / 2: case 0x0a / 2:
		vc.env = reg_long(v, 4);
		if (r == 0x0a / 2 && BIT(vc.env, 9))
			vc.level = (vc.env >> 12) & 0xfff;
		break;
	case 0x10 / 2: case 0x12 / 2:
		vc.loop_start = reg_long(v, 8) & ADDR_MASK;
		break;
	case 0x14 / 2: case 0x16 / 2:
		vc.loop_end = ~reg_long(v, 10) & ADDR_MASK;
		break;
	default:
		break;
	}
}

void proteus_snd_device::pan_w(offs_t offset, u8 data)
{
	m_stream->update();
	m_voice[offset & (VOICES - 1)].pan = data;
}

void proteus_snd_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		s64 left = 0, right = 0;
		for (voice &vc : m_voice)
		{
			// envelope: move the 12-bit float level toward the target by the signed rate
			const u16 target = (vc.env >> 12) & 0xfff;
			const s32 rate = util::sext(vc.env & 0x1ff, 9);
			if (vc.level != target)
			{
				s32 lvl = vc.level;
				const s32 delta = rate ? std::abs(rate) : 1;
				if (lvl < target)
					lvl = std::min<s32>(lvl + delta, target);
				else
					lvl = std::max<s32>(lvl - delta, target);
				vc.level = lvl;
			}

			const s32 smp = interpolate(vc);
			const s32 out = s32((s64(smp) * level_to_gain(vc.level)) >> 16);

			const s32 p = vc.pan & 0x0f;
			left += out * p;
			right += out * (15 - p);

			if (!BIT(vc.pitch, 18))
				step(vc);
		}
		// pan weights sum to 15, so one voice at full level panned hard to one side
		// peaks near 65472 * 15 >> 6 = 15345: about two such voices reach full scale.
		// Power-of-two scaling keeps the model exact in integer arithmetic.
		stream.put_int_clamp(0, i, s32(left >> 6), 32768);
		stream.put_int_clamp(1, i, s32(right >> 6), 32768);
	}
}
