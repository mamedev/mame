// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick

#include "emu.h"
#include "ymp706.h"
#include "yss236.h"

#include <algorithm>
#include <cmath>

#define LOG_REG (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(YMP706, ymp706_device, "ymp706", "Yamaha YMP706-F FS1-AB tone generator")

namespace
{
	// Per channel:
	// VL/VRATE/VOUT/VFREQ are the voiced envelope and oscillator; 
	// UL/URATE/UOUT/UFREQ are the unvoiced copy.
	constexpr unsigned STRIPE = 8;

	enum
	{
		R_VL1 = 0x000, R_VL2 = 0x008, R_VL3 = 0x010, R_VL4 = 0x018, R_VRATE = 0x020, R_VOUT = 0x048, R_VFREQ = 0x090,
		R_UL1 = 0x100, R_UL2 = 0x108, R_UL3 = 0x110, R_UL4 = 0x118, R_URATE = 0x120, R_UOUT = 0x148, R_UFREQ = 0x190,

		R_ALG0   = 0x200, R_ALG1 = 0x208,
		R_FORM   = 0x210,
		R_BW     = 0x218,
		R_VATT   = 0x228, R_UATT = 0x229,
		R_PANL   = 0x22a, R_PANR = 0x22b,
		R_VARL   = 0x22c, R_VARR = 0x22d,
		R_REVL   = 0x22e, R_REVR = 0x22f,
		R_RESO   = 0x238,
		R_PITCH  = 0x240,
		R_FB     = 0x260,
		R_ROUTE  = 0x268,

		R_USKIRT = 0x300,
		R_UBW    = 0x308,

		// Not banked by the channel select.
		R_GATEH  = 0x0f8, R_GATEL  = 0x0f9,
		R_OFFH   = 0x0fa, R_OFFL   = 0x0fb,
		R_ONH    = 0x0fc, R_ONL    = 0x0fd,
		R_LOOP   = 0x270,
		R_SEL    = 0x3ff
	};

	// 0 is idle. 1..3 run the level segments, 3 holds, 4 is the release.
	constexpr int SEG_IDLE    = 0;
	constexpr int SEG_START   = 1;
	constexpr int SEG_HOLD    = 3;
	constexpr int SEG_RELEASE = 4;

	// Six-bit level and rate. 63 is off, each step is -1.5 dB.
	constexpr int   EG_MASK  = 63;
	constexpr float EG_DB    = -1.5f;
	constexpr float EG_FLOOR = -200.0f;
	// Attack aims at +4 dB and is linear below the knee. Release falls 96 dB.
	constexpr float EG_KNEE  = -54.0f;
	constexpr float EG_AIM   = 4.0f;
	constexpr float EG_DROP  = 96.0f;
	constexpr float EG_K     = 16.0f;
	constexpr float EG_DIV   = float(1 << 26);

	constexpr double A440      = 440.0;
	constexpr int    A440_WORD = 26861;
	constexpr double STEPS_OCT = 1024.0;

	constexpr float  PI          = 3.14159265f;
	constexpr float  TAU         = PI * 2.0f;
	constexpr double PHASE_FULL  = double(1ull << 32);
	constexpr float  PHASE_SCALE = 1.0f / float(PHASE_FULL);

	// Output level is 0.375 dB per step. 250 and above is silent.
	constexpr float ATT_DB     = 0.375f;
	constexpr int   ATT_MUTE   = 250;
	constexpr int   BYTE_MAX   = 255;
	constexpr float BYTE_SCALE = 1.0f / float(BYTE_MAX);
	constexpr u8    SEND_OFF   = 0xff;

	constexpr float HZ_MIN  = 1.0f;
	constexpr float HZ_CEIL = 0.45f;

	// Form is bits 0..2. Skirt is bits 3..5. Bit 6 fixes the frequency.
	constexpr int FORM_MASK   = 7;
	constexpr int SKIRT_SHIFT = 3;
	constexpr int SKIRT_MASK  = 7;
	constexpr int FORM_FIXED  = 0x40;
	constexpr int SKIRT_POW   = 2;

	// sine, all1, all2, odd1, odd2, res1, res2, frmt.
	constexpr int FORM_SINE = 0;
	constexpr int FORM_ALL1 = 1;
	constexpr int FORM_ODD1 = 3;
	constexpr int FORM_ODD2 = 4;
	constexpr int FORM_RES1 = 5;
	constexpr int FORM_RES2 = 6;
	constexpr int FORM_FRMT = 7;

	// t0 bits 3..5 select the input. Bit 2 writes feedback, bit 1 the held bus.
	constexpr int SRC_SHIFT = 3;
	constexpr int SRC_MASK  = 7;
	constexpr int SRC_FB    = 3;
	constexpr int SRC_CHAIN = 4;
	constexpr int SRC_HELD  = 5;
	constexpr int SRC_SUM   = 6;
	constexpr int BUS_HOLD  = 1 << 1;
	constexpr int BUS_FB    = 1 << 2;
	constexpr int BUS_MIX   = 1 << 0;
	constexpr int BUS_SUM   = 1 << 2;
	constexpr int FB_MASK = 7;
	constexpr float FB_STEP = 1.0f / 16.0f;

	// Formant window. Bandwidth 0 is 1/PI of a period; it halves every 8 steps.
	constexpr int   BW_MAX   = 99;
	constexpr int   BW_MASK  = 0x7f;
	constexpr float WIN_BASE = 3.1f;
	constexpr float WIN_STEP = 8.0f;
	constexpr int   FRMT_F0  = 0x1243;

	// Unvoiced lowpass: 17 Hz times the bandwidth, widened by 1.35 per skirt step.
	constexpr float NOISE_HZ    = 17.0f;
	constexpr float NOISE_WIDEN = 1.35f;
	constexpr float NOISE_SCALE = 1.0f / 32768.0f;
	constexpr u32   NOISE_A     = 1664525;
	constexpr u32   NOISE_C     = 1013904223;

	// R_LOOP value that inserts the external filter.
	constexpr int FILTER_INSERT = 11;
}

ymp706_device::ymp706_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) : 
  device_t(mconfig, YMP706, tag, owner, clock),
  device_sound_interface(mconfig, *this),
  m_stream(nullptr),
  m_filter(nullptr),
  m_voice_base(0),
  m_mix_input(false),
  m_sel(0),
  m_noise(1)
{
}

void ymp706_device::set_filter(yss236_device &filter, int voice_base)
{
	m_filter = &filter;
	m_voice_base = voice_base;
	filter.add_client(*this);
}

void ymp706_device::device_start()
{
	m_stream = stream_alloc(m_mix_input ? BUSES : 0, BUSES, sample_rate());

	save_item(NAME(m_bus));
	save_item(NAME(m_sel));
	save_item(NAME(m_noise));
	save_item(STRUCT_MEMBER(m_note, mem));
	save_item(STRUCT_MEMBER(m_note, phase));
	save_item(STRUCT_MEMBER(m_note, nphase));
	save_item(STRUCT_MEMBER(m_note, cphase));
	save_item(STRUCT_MEMBER(m_note, pphase));
	save_item(STRUCT_MEMBER(m_note, grain));
	save_item(STRUCT_MEMBER(m_note, eg));
	save_item(STRUCT_MEMBER(m_note, ueg));
	save_item(STRUCT_MEMBER(m_note, nz));
	save_item(STRUCT_MEMBER(m_note, prev));
	save_item(STRUCT_MEMBER(m_note, stage));
	save_item(STRUCT_MEMBER(m_note, ustage));
}

void ymp706_device::device_reset()
{
	std::fill(std::begin(m_bus), std::end(m_bus), 0);
	m_sel = 0;
	m_noise = 1;
	for (note &n : m_note)
	{
		n = note{};
		for (int op = 0; op < OPS; op++)
		{
			n.eg[op] = EG_FLOOR;
			n.ueg[op] = EG_FLOOR;
		}
	}
}

void ymp706_device::device_clock_changed()
{
	m_stream->set_sample_rate(sample_rate());
}

void ymp706_device::map(address_map &map)
{
	map(0x000, MEM - 1).rw(FUNC(ymp706_device::reg_r), FUNC(ymp706_device::reg_w));
}

bool ymp706_device::chip_wide(offs_t offset)
{
	switch (offset)
	{
	// Shared bytes on the voiced and unvoiced pages.
	case 0x0c8:
	case 0x1c8:
	case R_GATEH:
	case R_GATEL:
	case R_OFFH:
	case R_OFFL:
	case R_ONH:
	case R_ONL:
	case R_LOOP:
	case R_SEL:
		return true;
	default:
		return false;
	}
}

u8 ymp706_device::reg_r(offs_t offset)
{
	offset &= MEM - 1;
	return chip_wide(offset) ? m_bus[offset] : m_note[m_sel].mem[offset];
}

void ymp706_device::release_mask(u16 mask)
{
	for (int ch = 0; ch < NOTES; ch++)
	{
		if ((mask & (1u << ch)) == 0)
			continue;
		for (int op = 0; op < OPS; op++)
		{
			if (m_note[ch].stage[op] ) m_note[ch].stage[op]  = SEG_RELEASE;
			if (m_note[ch].ustage[op]) m_note[ch].ustage[op] = SEG_RELEASE;
		}
	}
}

void ymp706_device::begin_attack(note &n, int op, bool unvoiced)
{
	u8 &seg = unvoiced ? n.ustage[op] : n.stage[op];
	if (seg >= SEG_START && seg <= SEG_HOLD)
		return;
	if (seg == SEG_IDLE)
	{
		(unvoiced ? n.ueg[op] : n.eg[op]) = EG_FLOOR;
		if (!unvoiced)
		{
			n.phase [op] = 0;
			n.cphase[op] = 0;
			n.pphase[op] = 0;
			n.grain [op] = 0;
		}
		else
			n.nphase[op] = 0;
	}
	seg = SEG_START;
}

void ymp706_device::reg_w(offs_t offset, u8 data)
{
	m_stream->update();
	offset &= MEM - 1;
	m_bus[offset] = data;

	if (offset == R_SEL)
		m_sel = data & (NOTES - 1);
	else if (!chip_wide(offset))
	{
		note &n = m_note[m_sel];
		n.mem[offset] = data;
		offs_t const row = offset & ~(STRIPE - 1);
		bool const voiced_level   = row == R_VL1 || row == R_VL2 || row == R_VL3 || row == R_VL4;
		bool const unvoiced_level = row == R_UL1 || row == R_UL2 || row == R_UL3 || row == R_UL4;
		if (voiced_level || unvoiced_level)
			begin_attack(n, int(offset & (STRIPE - 1)), unvoiced_level);
	}
	else if (offset == R_ONL || offset == R_OFFL)
		release_mask(u16(m_bus[offset]) | (u16(m_bus[offset & ~1]) << 8));

	LOGMASKED(LOG_REG, "%s: w %03x = %02x (ch %x)\n", machine().describe_context(), offset, data, m_sel);
}

namespace
{

	int pair(u8 const *mem, int hi)
	{
		return (mem[hi] << 8) | mem[hi + STRIPE];
	}

	float word_hz(int word)
	{
		return float(A440 * std::exp2((word - A440_WORD) / STEPS_OCT));
	}

	float eg_level_db(u8 raw)
	{
		int const a = raw & EG_MASK;
		return a >= EG_MASK ? EG_FLOOR : EG_DB * float(a);
	}

	void eg_tick(float &db, u8 &seg, u8 const *mem, int level, int rate)
	{
		if (seg < SEG_START || seg > SEG_RELEASE)
			return;
		int const row = seg - 1;
		float const target = eg_level_db(mem[level + row * STRIPE]);
		int const q = mem[rate + row * STRIPE] & EG_MASK;
		int const ladder = (4 + (q & 3)) << (q >> 2);
		if (db < target)
		{
			if (db < EG_KNEE)
				db = std::min(target, EG_KNEE);
			float const k = 1.f - std::exp(-EG_K * float(ladder) / EG_DIV);
			db += (EG_AIM - db) * k;
			if (db > target)
				db = target;
		}
		else if (db > target)
		{
			db -= EG_DROP * float(ladder) / EG_DIV;
			if (db < target)
				db = target;
		}
		if (db <= EG_FLOOR)
		{
			db = EG_FLOOR;
			seg = SEG_IDLE;
			return;
		}
		if (db == target)
		{
			if (seg < SEG_HOLD)
				seg++;
			else if (seg == SEG_RELEASE)
				seg = SEG_IDLE;
		}
	}

	// One cycle of sin(pi*x)^p, and sin^p on the rise with sin^2 on the fall.
	// p = 2, 4, 8, ... 256.
	float const WIN_MEAN[2][8] = {
		{0.500000f, 0.375000f, 0.273437f, 0.196381f, 0.139950f, 0.099347f, 0.070386f, 0.049819f},
		{0.500000f, 0.437500f, 0.386719f, 0.348190f, 0.319975f, 0.299673f, 0.285193f, 0.274910f},
	};

	float pulse(float ph, int skirt, bool asym)
	{
		ph -= std::floor(ph);
		float e = float(SKIRT_POW << skirt);
		if (asym && ph >= 0.5f)
			e = float(SKIRT_POW);
		float u = std::sin(PI * ph);
		if (u < 0.f)
			u = 0.f;
		return std::pow(u, e);
	}

	float clamp_hz(float hz, float sr)
	{
		if (hz < HZ_MIN)
			return HZ_MIN;
		if (hz > sr * HZ_CEIL)
			return sr * HZ_CEIL;
		return hz;
	}

}

float ymp706_device::render_note(note &n)
{
	bool sounding = false;
	for (int op = 0; op < OPS && !sounding; op++)
		sounding = n.stage[op] || n.ustage[op];
	if (!sounding)
		return 0;

	const float sr = float(sample_rate());
	const int pitch = pair(n.mem, R_PITCH);
	const float fb_scale = float(n.mem[R_FB] & FB_MASK) * FB_STEP;
	float mix = 0;
	// Algorithm bytes pack t0 and t1. Feedback holds across samples.
	// The other buses are cleared with each sample.
	float feedback = n.prev;
	float chain = 0;
	float held = 0;
	float sum = 0;
	for (int op = 0; op < OPS; op++)
	{
		const u8 lo = n.mem[R_ALG1 + op];
		const int t0 = (int(n.mem[R_ALG0 + op]) << 1) | (lo & 1);
		const int t1 = lo >> 1;
		const int sel = (t0 >> SRC_SHIFT) & SRC_MASK;
		float in = 0;
		if (sel == SRC_FB)
			in = feedback * fb_scale;
		else if (sel == SRC_CHAIN)
			in = chain;
		else if (sel == SRC_HELD)
			in = held;
		else if (sel == SRC_SUM)
		{
			in = sum;
			sum = 0;
		}

		const u8 shape = n.mem[R_FORM + op];
		const int form = shape & FORM_MASK;
		const bool fixed = (shape & FORM_FIXED) != 0;
		int word = pair(n.mem, R_VFREQ + op);
		if (!fixed && form != FORM_FRMT)
			word += pitch;

		eg_tick(n.eg[op], n.stage[op], n.mem + op, R_VL1, R_VRATE);
		const int att = std::min(BYTE_MAX, int(n.mem[R_VOUT + op]) + int(n.mem[R_VATT]));
		float s = 0;
		if (n.stage[op] && word > 0 && att < ATT_MUTE)
		{
			const int skirt = (shape >> SKIRT_SHIFT) & SKIRT_MASK;
			if (form == FORM_FRMT)
			{
				// Formant
				const int bw = std::min(BW_MAX, int(n.mem[R_BW + op] & BW_MASK));
				float f0 = clamp_hz(word_hz(pitch + FRMT_F0), sr);
				float fc = clamp_hz(word_hz(word), sr);
				float win = 1.0f / (WIN_BASE * std::exp2(float(bw) / WIN_STEP));
				float const period = 1.0f / f0;
				if (win > period * 2.0f)
					win = period * 2.0f;
				u32 const was = n.phase[op];
				n.phase[op] += u32(f0 / sr * PHASE_FULL);
				if (n.phase[op] < was)
				{
					n.pphase[op] = n.cphase[op];
					n.cphase[op] = 0;
					n.grain[op] = 1;
				}
				u32 const cstep = u32(fc / sr * PHASE_FULL);
				n.cphase[op] += cstep;
				n.pphase[op] += cstep;
				float const g = float(n.phase[op]) * PHASE_SCALE;
				float const pexp = 2.0f * std::exp2(float(skirt) * 0.5f);
				auto env = [&](float t)
				{
					if (t >= win)
						return 0.0f;
					float u = std::sin(PI * t / win);
					if (u < 0.0f)
						u = 0.0f;
					return std::pow(u, pexp);
				};
				auto carrier = [&](u32 ph)
				{
					float c = float(ph) * PHASE_SCALE + in;
					return std::sin(c * TAU);
				};
				s = env(g * period) * carrier(n.cphase[op]);
				if (n.grain[op])
					s += env(g * period + period) * carrier(n.pphase[op]);
			}
			else
			{
				float hz = clamp_hz(word_hz(word), sr);
				n.phase[op] += u32(hz / sr * PHASE_FULL);
				float ph = float(n.phase[op]) * PHASE_SCALE + in;
				ph -= std::floor(ph);
				if (form == FORM_SINE)
					s = std::sin(ph * TAU);
				else if (form == FORM_RES1 || form == FORM_RES2)
				{
					// The peak is the resonance byte plus one.
					int const partial = int(n.mem[R_RESO + op]) + 1;
					s = pulse(ph, skirt, form == FORM_RES1) * std::sin(ph * float(partial) * TAU);
				}
				else
				{
					const bool odd = form == FORM_ODD1 || form == FORM_ODD2;
					const bool asym = form == FORM_ALL1 || form == FORM_ODD1;
					float x = ph;
					float sign = 1.0f;
					if (odd)
					{
						x *= 2.0f;
						if (ph >= 0.5f)
							sign = -1.0f;
					}
					s = sign * (pulse(x, skirt, asym) - WIN_MEAN[asym ? 1 : 0][skirt]);
				}
			}
			s *= std::pow(10.0f, (n.eg[op] - ATT_DB * float(att)) / 20.0f);
		}
		chain = s;

		if (t0 & BUS_FB  ) feedback = s;
		if (t0 & BUS_HOLD) held = s;
		if (t1 & BUS_SUM ) sum += s;
		if (t1 & BUS_MIX ) mix += s;

		eg_tick(n.ueg[op], n.ustage[op], n.mem + op, R_UL1, R_URATE);
		const int uatt = std::min(BYTE_MAX, int(n.mem[R_UOUT + op]) + int(n.mem[R_UATT]));
		const int ubw = n.mem[R_UBW + op];
		const int uword = pair(n.mem, R_UFREQ + op);
		if (n.ustage[op] && ubw > 0 && uword > 0 && uatt < ATT_MUTE)
		{
			const int skirt = n.mem[R_USKIRT + op] & SKIRT_MASK;
			float fc = NOISE_HZ * float(ubw) * std::pow(NOISE_WIDEN, float(skirt));
			if (fc > sr * HZ_CEIL)
				fc = sr * HZ_CEIL;
			const float g = 1.0f - std::exp(-TAU * fc / sr);
			m_noise = m_noise * NOISE_A + NOISE_C;
			float u = float(s16(m_noise >> 16)) * NOISE_SCALE;
			n.nz[op][0] += g * (u - n.nz[op][0]);
			n.nz[op][1] += g * (n.nz[op][0] - n.nz[op][1]);
			n.nphase[op] += u32(clamp_hz(word_hz(uword), sr) / sr * PHASE_FULL);
			float ph = float(n.nphase[op]) * PHASE_SCALE;
			float ns = std::sin(ph * TAU) * n.nz[op][1];
			ns *= std::pow(10.0f, (n.ueg[op] - ATT_DB * float(uatt)) / 20.0f);
			mix += ns;
		}
	}
	n.prev = feedback;
	return mix;
}

void ymp706_device::sound_stream_update(sound_stream &stream)
{
	// Each channel is mono, panned into three stereo pairs.
	// Dry is linear:
	//   L = 255 - pan L
	//   R = pan R.
	// Route bit 0: is the dry bus
	// Route bit 1: insertion bus.
	// Variation and reverb are 0.375 dB steps, and 0xFF is off.
	auto const send = [](u8 att) {
		return att == SEND_OFF ? 0.0f : std::pow(10.0f, -ATT_DB * float(att) / 20.0f);
	};

	for (int i = 0; i < stream.samples(); i++)
	{
		float voice[NOTES];
		for (int ch = 0; ch < NOTES; ch++)
			voice[ch] = render_note(m_note[ch]);

		// YMP706 CHOUT -> filter -> CHIN.
		if (m_bus[R_LOOP] == FILTER_INSERT && m_filter)
			m_filter->filter_frame(m_voice_base, voice);

		float bus[BUSES];
		for (int b = 0; b < BUSES; b++)
			bus[b] = m_mix_input ? stream.get(b, i) : 0.0f;
		for (int ch = 0; ch < NOTES; ch++)
		{
			u8 const *mem = m_note[ch].mem;
			float const v = voice[ch];

			if (v == 0.0f)
				continue;

			float const l = v * float(BYTE_MAX - mem[R_PANL]) * BYTE_SCALE, r = v * float(mem[R_PANR]) * BYTE_SCALE;
			for (int b = 0; b < 2; b++)
				if (BIT(mem[R_ROUTE], b))
				{
					bus[BUS_DRY + 2 * b] += l;
					bus[BUS_DRY + 2 * b + 1] += r;
				}

			bus[BUS_VAR + 0] += v * send(mem[R_VARL]);
			bus[BUS_VAR + 1] += v * send(mem[R_VARR]);
			bus[BUS_REV + 0] += v * send(mem[R_REVL]);
			bus[BUS_REV + 1] += v * send(mem[R_REVR]);
		}
		for (int b = 0; b < BUSES; b++)
			stream.put(b, i, bus[b]);
	}
}
