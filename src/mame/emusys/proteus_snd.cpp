// license:BSD-3-Clause
// copyright-holders:Grant McNaught
/***************************************************************************

    E-mu Proteus/1 sound generator (custom chip, 32 voices)

    Reconstructed from the Proteus/1 XR firmware (v2.00 / v2.30). Labels:
    [O] read from firmware code, [I] inferred, [U] unknown.

    Output rate: 39062.5 Hz [O: frequency-response test pitches for
    10/18/19 kHz on the 64-sample test sine all give 39062 Hz]. Clock 20 MHz,
    from the 40 MHz crystal: 32 voices x 16 cycles per output sample [I].

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
    cycles per voice for volume and mixing [I]. Current position = address + 4 +
    fraction (the firmware programs start - 4); 8 words address+1 .. address+8
    are read and the tap 3.5 or more words away has weight 0.

    The patent's "family of six filters" of lower cutoff is selected by the tone
    value (KERNELS[tone][a], a = 32 x tap distance, 65536 = 1.0):
      tone 0: the patent's Table 1, normalised so that each of the 32 polyphase
              branches sums to 1.0. Two printed values look misprinted and are
              corrected: entry 79 2.30703e-03 -> 2.12703e-03 and entry 111
              -1.76482e-04 -> -1.76482e-05.
      tones 1..7 [I]: no source gives their response. Each is a weighted
              least-squares design with the patent's weights (passband 1,
              transition 1e-6, stopband 2000 at multiples of the sample rate and
              66 between them) and exact zeros at multiples of the sample rate.
              Fitted to Table 1, that method gives a stopband edge of 1.55 x the
              source Nyquist frequency and -6 dB at 1.06 x. The firmware raises
              the tone by one per 5 semitones of upward transposition, so tone t
              scales both by 2^(-5t/12). Tones 6 and 7 are not used by the
              firmware.

    Level [I]: 12-bit float, exponent = bits 11..9, mantissa = bits 8..0;
    gain = exp ? (0x200 | m) << (exp - 1) : m, full scale 0x3ff << 6 (Q16).
    Derived from the firmware's level tables: 0x200 level units per 6 dB.

    Pan byte (CPU 0x500009 + 2 * voice) [I]: bits 5..4 output pair (main, sub 1,
    sub 2), bits 3..0 position, 0xf = left, 0x0 = right; linear. All pairs are
    mixed to the stereo output here.

***************************************************************************/

#include "emu.h"
#include "proteus_snd.h"

#include <algorithm>
#include <cstdlib>

#define LOG_REGS (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"


namespace {

// interpolation kernels [tone][a], a = 0..128 (32 per sample period), 65536 = 1.0
constexpr s32 KERNELS[8][129] = {
	{ 67755, 67617, 67203, 66518, 65566, 64356, 62896, 61198, 59275, 57144, 54820, 52320, 49663, 46870, 43968, 40991, 37888, 34751, 31613, 28461, 25320, 22215, 19166, 16193, 13311, 10538, 7888, 5376, 3012, 809, -1227, -3088, -4769, -6267, -7580, -8708, -9654, -10419, -11010, -11431, -11690, -11796, -11759, -11586, -11290, -10881, -10381, -9849, -9102, -8365, -7590, -6775, -5928, -5067, -4199, -3338, -2489, -1663, -866, -106, 613, 1283, 1903, 2468, 2976, 3425, 3816, 4147, 4420, 4635, 4796, 4902, 4959, 4969, 4937, 4863, 4754, 4611, 4449, 4225, 3989, 3765, 3543, 3312, 3073, 2834, 2596, 2363, 2135, 1915, 1703, 1503, 1313, 1136, 972, 821, 684, 560, 449, 352, 266, 193, 131, 80, 38, 5, -20, -37, -49, -57, -56, -35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 52196, 52136, 51955, 51655, 51236, 50702, 50053, 49295, 48430, 47462, 46395, 45236, 43989, 42660, 41256, 39782, 38266, 36658, 35012, 33324, 31603, 29855, 28088, 26309, 24526, 22746, 20975, 19220, 17489, 15787, 14120, 12494, 10914, 9386, 7913, 6499, 5149, 3866, 2652, 1510, 442, -552, -1469, -2310, -3074, -3762, -4374, -4912, -5361, -5772, -6102, -6366, -6565, -6704, -6785, -6812, -6788, -6716, -6601, -6447, -6257, -6035, -5785, -5511, -5217, -4905, -4581, -4247, -3907, -3563, -3219, -2878, -2542, -2213, -1893, -1585, -1290, -1009, -744, -496, -138, 29, 197, 349, 484, 602, 704, 791, 862, 918, 961, 990, 1007, 1013, 1009, 995, 972, 942, 906, 864, 818, 768, 715, 661, 606, 550, 495, 441, 389, 339, 292, 247, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 40271, 40244, 40164, 40031, 39845, 39607, 39317, 38977, 38587, 38148, 37661, 37128, 36551, 35930, 35268, 34566, 33802, 33046, 32244, 31410, 30546, 29655, 28739, 27800, 26840, 25862, 24869, 23863, 22847, 21822, 20792, 19759, 18725, 17693, 16665, 15643, 14629, 13627, 12636, 11661, 10702, 9762, 8841, 7943, 7068, 6217, 5393, 4595, 3809, 3090, 2388, 1717, 1077, 470, -106, -649, -1158, -1635, -2079, -2490, -2869, -3216, -3530, -3814, -4066, -4289, -4483, -4648, -4786, -4897, -4983, -5045, -5083, -5100, -5095, -5071, -5028, -4969, -4893, -4803, -4843, -4668, -4507, -4340, -4167, -3990, -3810, -3628, -3444, -3260, -3077, -2895, -2715, -2537, -2363, -2192, -2026, -1865, -1710, -1560, -1416, -1279, -1148, -1024, -907, -797, -695, -599, -511, -430, -357, -290, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 28437, 28426, 28391, 28333, 28253, 28150, 28024, 27876, 27705, 27513, 27300, 27065, 26810, 26534, 26239, 25925, 25487, 25198, 24840, 24465, 24075, 23668, 23246, 22810, 22360, 21898, 21423, 20937, 20441, 19934, 19419, 18895, 18365, 17827, 17285, 16737, 16185, 15631, 15074, 14516, 13957, 13398, 12841, 12285, 11732, 11182, 10636, 10096, 9481, 9026, 8514, 8010, 7514, 7027, 6549, 6081, 5623, 5176, 4739, 4315, 3902, 3502, 3114, 2738, 2376, 2028, 1692, 1371, 1063, 769, 489, 222, -30, -268, -492, -703, -900, -1083, -1253, -1409, -2200, -2185, -2250, -2304, -2348, -2380, -2403, -2417, -2421, -2417, -2405, -2386, -2359, -2326, -2287, -2242, -2191, -2137, -2078, -2015, -1948, -1879, -1808, -1734, -1658, -1582, -1504, -1426, -1347, -1269, -1190, -1113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 21420, 21415, 21399, 21372, 21335, 21287, 21228, 21159, 21079, 20990, 20890, 20780, 20659, 20529, 20390, 20241, 19957, 19852, 19677, 19493, 19301, 19100, 18892, 18676, 18452, 18220, 17982, 17736, 17484, 17225, 16960, 16689, 16413, 16131, 15844, 15552, 15256, 14956, 14651, 14343, 14032, 13718, 13401, 13082, 12760, 12437, 12113, 11787, 11366, 11115, 10789, 10464, 10139, 9814, 9491, 9168, 8848, 8529, 8212, 7897, 7585, 7276, 6970, 6667, 6367, 6072, 5780, 5492, 5209, 4931, 4657, 4388, 4124, 3865, 3612, 3365, 3123, 2887, 2657, 2433, 1445, 1351, 1156, 969, 791, 621, 460, 306, 161, 24, -105, -227, -341, -447, -546, -638, -722, -800, -871, -935, -992, -1043, -1088, -1127, -1160, -1187, -1209, -1226, -1237, -1244, -1245, -1242, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 15571, 15569, 15563, 15553, 15539, 15521, 15499, 15473, 15444, 15410, 15373, 15331, 15286, 15237, 15184, 15128, 15166, 15040, 14959, 14874, 14786, 14695, 14600, 14503, 14402, 14298, 14191, 14082, 13969, 13854, 13735, 13615, 13491, 13365, 13237, 13106, 12972, 12837, 12699, 12560, 12418, 12274, 12128, 11981, 11832, 11681, 11529, 11375, 11295, 11067, 10896, 10725, 10553, 10380, 10206, 10031, 9856, 9680, 9504, 9328, 9151, 8974, 8797, 8620, 8444, 8267, 8091, 7915, 7740, 7565, 7391, 7217, 7045, 6873, 6702, 6532, 6363, 6196, 6029, 5864, 6307, 5992, 5768, 5547, 5330, 5117, 4908, 4703, 4502, 4306, 4113, 3925, 3741, 3561, 3386, 3214, 3048, 2885, 2727, 2573, 2424, 2279, 2138, 2002, 1870, 1742, 1619, 1501, 1386, 1276, 1170, 1069, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 12620, 12619, 12616, 12612, 12606, 12598, 12589, 12578, 12565, 12550, 12534, 12516, 12496, 12474, 12451, 12427, 12768, 12542, 12494, 12444, 12392, 12340, 12285, 12229, 12172, 12114, 12054, 11993, 11930, 11866, 11801, 11735, 11667, 11598, 11528, 11457, 11385, 11311, 11237, 11161, 11085, 11007, 10928, 10848, 10768, 10686, 10604, 10521, 10716, 10392, 10289, 10185, 10081, 9976, 9870, 9764, 9657, 9550, 9442, 9334, 9225, 9116, 9006, 8897, 8787, 8676, 8566, 8455, 8345, 8234, 8123, 8012, 7901, 7790, 7679, 7568, 7457, 7347, 7236, 7126, 9285, 8763, 8565, 8369, 8175, 7983, 7792, 7604, 7418, 7233, 7051, 6871, 6693, 6518, 6344, 6173, 6004, 5838, 5673, 5512, 5352, 5195, 5041, 4889, 4739, 4592, 4448, 4306, 4167, 4030, 3896, 3765, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 10670, 10669, 10668, 10666, 10664, 10660, 10656, 10651, 10645, 10639, 10632, 10624, 10615, 10605, 10595, 10584, 11212, 10871, 10839, 10807, 10775, 10741, 10707, 10672, 10637, 10601, 10565, 10527, 10490, 10451, 10412, 10373, 10333, 10292, 10251, 10209, 10167, 10124, 10081, 10037, 9992, 9948, 9902, 9857, 9810, 9763, 9716, 9669, 10106, 9657, 9591, 9525, 9458, 9392, 9324, 9257, 9189, 9121, 9053, 8984, 8915, 8846, 8777, 8707, 8637, 8567, 8497, 8426, 8356, 8285, 8214, 8143, 8072, 8001, 7930, 7859, 7787, 7716, 7644, 7573, 11450, 10730, 10573, 10416, 10260, 10105, 9951, 9797, 9645, 9494, 9343, 9194, 9046, 8898, 8752, 8607, 8463, 8320, 8179, 8038, 7899, 7761, 7624, 7489, 7355, 7222, 7090, 6960, 6831, 6703, 6577, 6453, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(PROTEUS_SND, proteus_snd_device, "proteus_snd", "E-mu Proteus sound generator")

proteus_snd_device::proteus_snd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PROTEUS_SND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
{
}

void proteus_snd_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / (VOICES * 16));

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

u32 proteus_snd_device::reg_long(u32 v, u32 word) const
{
	return (u32(m_regs[v * WORDS_PER_VOICE + word]) << 16) | m_regs[v * WORDS_PER_VOICE + word + 1];
}

u32 proteus_snd_device::level_to_gain(u16 level)
{
	const u32 e = (level >> 9) & 7;
	const u32 m = level & 0x1ff;
	return e ? ((0x200 | m) << (e - 1)) : m;
}

s32 proteus_snd_device::interpolate(const voice &vc)
{
	// 8 taps: words addr+1 .. addr+8 around the position addr + 4 + frac / 4096
	const s32 *const k = KERNELS[(vc.pitch >> 21) & 7];
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
	const s32 whole = util::sext(vc.pitch >> 12, 6);
	const u32 pos = (vc.frac & 0xfff) + (vc.pitch & 0xfff);
	vc.frac = pos & 0xfff;
	vc.addr = (vc.addr + u32(whole) + (pos >> 12)) & ADDR_MASK;
	const bool passed = reverse
			? (vc.addr < vc.loop_end && vc.loop_end < vc.loop_start)
			: (vc.addr > vc.loop_end && vc.loop_end >= vc.loop_start);
	if (passed)
		vc.addr = (vc.addr - (vc.loop_end - vc.loop_start)) & ADDR_MASK;
}

u16 proteus_snd_device::read(offs_t offset)
{
	const u32 v = offset >> 4;
	const u32 r = offset & 15;
	if (r == 0x18 / 2)
	{
		m_stream->update();
		voice &vc = m_voice[v];
		const u16 data = u16(rom_word(vc.addr + 4));
		if (!machine().side_effects_disabled() && BIT(vc.pitch, 18))
			step(vc);
		return data;
	}
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGS, "%s: read  v%02u +%02x -> %04x\n", machine().describe_context(), v, r << 1, m_regs[offset]);
	return m_regs[offset];
}

void proteus_snd_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();
	COMBINE_DATA(&m_regs[offset]);
	const u32 v = offset >> 4;
	const u32 r = offset & 15;
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
			const s32 rate = util::sext(vc.env, 9);
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
		// Each output pair has a 16-bit DAC (PCM56P); all pairs are mixed here.
		stream.put_int_clamp(0, i, s32(left >> 6), 32768);
		stream.put_int_clamp(1, i, s32(right >> 6), 32768);
	}
}
