// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick

#include "emu.h"
#include "yss236.h"
#include "yss236_fx.h"
#include "ymp706.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>

#define LOG_REG (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(YSS236, yss236_device, "yss236", "Yamaha YSS236-F VOP3")

namespace {

constexpr float PI = 3.14159265f;

// Cutoff cell of byte 0, the per-octave step, and the firmware's ceiling.
constexpr float CUT_BASE = 3085.0f;
constexpr float CUT_OCTAVE = 169.0f * 12.0f;
constexpr float CUT_MAX = 24576.0f;

// Estimate: a segment that reaches its target takes 2^(rate/16) ticks.
// Rate 70 is about 7 ms, 197 1.7 s, 253 19 s.
constexpr float EG_RATE_STEP = 16.0f;

// Estimate: FEG depth cell * level / 256 in cutoff words. Depth 63 at
// level +50 is then about 9 octaves, the whole cutoff range.
constexpr float FEG_SCALE = 1.0f / 256.0f;

// Estimate: LFO speed 1 is about 0.18 Hz.
constexpr int LFO_SPEED_SHIFT = 18;

}

yss236_device::yss236_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, YSS236, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_irq(*this),
	m_tick_timer(nullptr),
	m_filter_chip(false),
	m_effect_chip(false),
	m_stream(nullptr),
	m_fx_params{ },
	m_client{ nullptr, nullptr },
	m_map{ },
	m_select(0),
	m_seg_done(0),
	m_seg_enable(0)
{
	for (int i = 0; i < CHANS; i++)
		m_eg_slot[i] = m_slot_eg[i] = u8(i);
}

yss236_device::~yss236_device() = default;

void yss236_device::device_start()
{
	if (m_filter_chip)
		m_tick_timer = timer_alloc(FUNC(yss236_device::tick), this);
	if (m_effect_chip)
	{
		m_stream = stream_alloc(8, 2, sample_rate());
		m_fx = std::make_unique<yss236_fx::FxSection>();
		m_fx->init(float(sample_rate()));
		save_item(NAME(m_fx_params));
	}

	// Exponential approach to the aim. The target is 2/3 of the way there,
	// so it is crossed after ln(3) time constants.
	m_eg_k[0] = 1.0f;
	m_eg_k[255] = 0.0f;
	for (int r = 1; r < 255; r++)
		m_eg_k[r] = 1.0f - std::exp(-std::log(3.0f) / std::exp2(float(r) / EG_RATE_STEP));

	save_item(NAME(m_reg));
	save_item(NAME(m_prog));
	save_item(NAME(m_coef));
	save_item(NAME(m_tag));
	save_item(NAME(m_slot));
	save_item(NAME(m_cfg));
	save_item(NAME(m_lfo_reg));
	save_item(NAME(m_select));
	save_item(NAME(m_seg_done));
	save_item(NAME(m_seg_enable));
	save_item(STRUCT_MEMBER(m_chan, cut));
	save_item(STRUCT_MEMBER(m_chan, reso));
	save_item(STRUCT_MEMBER(m_chan, gain));
	save_item(STRUCT_MEMBER(m_chan, feg_depth));
	save_item(STRUCT_MEMBER(m_chan, lfo_depth));
	save_item(STRUCT_MEMBER(m_chan, type));
	save_item(STRUCT_MEMBER(m_eg, level));
	save_item(STRUCT_MEMBER(m_eg, target));
	save_item(STRUCT_MEMBER(m_eg, aim));
	save_item(STRUCT_MEMBER(m_eg, rate));
	save_item(STRUCT_MEMBER(m_eg, side));
	save_item(STRUCT_MEMBER(m_lfo, phase));
	save_item(STRUCT_MEMBER(m_lfo, speed));
	save_item(STRUCT_MEMBER(m_lfo, wave));
	save_item(STRUCT_MEMBER(m_lfo, phase0));
	save_item(STRUCT_MEMBER(m_lfo, sh));
	save_item(STRUCT_MEMBER(m_voice, z));
	save_item(STRUCT_MEMBER(m_voice, s1));
	save_item(STRUCT_MEMBER(m_voice, s2));
}

void yss236_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	std::fill(&m_prog[0][0], &m_prog[0][0] + STEPS * 5, 0);
	std::fill(std::begin(m_coef), std::end(m_coef), 0);
	std::fill(std::begin(m_tag), std::end(m_tag), 0);
	std::fill(&m_slot[0][0], &m_slot[0][0] + 128 * 2, 0);
	std::fill(&m_cfg[0][0], &m_cfg[0][0] + 4 * 16, 0);
	std::fill(&m_lfo_reg[0][0], &m_lfo_reg[0][0] + 16 * 4, 0);
	m_select = 0;
	m_seg_done = 0;
	m_seg_enable = 0;
	for (chan &c : m_chan)
	{
		c = chan{};
		c.gain = 4096;
	}
	std::fill(std::begin(m_eg), std::end(m_eg), eg{});
	std::fill(std::begin(m_lfo), std::end(m_lfo), lfo{});
	std::fill(std::begin(m_voice), std::end(m_voice), voice{});
	for (int s = 0; s < CHANS; s++)
		update_slot(s);
	if (m_tick_timer)
	{
		attotime const period = attotime::from_hz(sample_rate()) * TICK;
		m_tick_timer->adjust(period, 0, period);
	}
	m_irq(0);
}

void yss236_device::device_post_load()
{
	for (int s = 0; s < CHANS; s++)
		update_slot(s);
	if (m_fx)
	{
		m_fx->init(float(sample_rate()));
		m_fx->configure(m_fx_params);
	}
}

void yss236_device::fx_params_w(u8 const *fx)
{
	if (!m_fx || !std::memcmp(m_fx_params, fx, sizeof(m_fx_params)))
		return;
	m_stream->update();
	std::memcpy(m_fx_params, fx, sizeof(m_fx_params));
	m_fx->configure(m_fx_params);
}

void yss236_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		float in[8];
		for (int c = 0; c < 8; c++)
			in[c] = stream.get(c, i);
		float l, r;
		m_fx->process(in, l, r);
		stream.put(0, i, l);
		stream.put(1, i, r);
	}
}

void yss236_device::map(address_map &map)
{
	map(0x000, 0x0ff).rw(FUNC(yss236_device::reg_r), FUNC(yss236_device::reg_w));
}

void yss236_device::add_client(ymp706_device &tg)
{
	for (ymp706_device *&c : m_client)
	{
		if (c == &tg)
			return;
		if (!c)
		{
			c = &tg;
			return;
		}
	}
}

void yss236_device::sync_clients()
{
	for (ymp706_device *c : m_client)
		if (c)
			c->sync();
}

void yss236_device::update_irq()
{
	m_irq(seg_pending() ? ASSERT_LINE : CLEAR_LINE);
}

void yss236_device::arm_eg(eg &g)
{
	// Done fires when the level passes the target on its way to the aim.
	float const d = g.level - float(g.target);
	if (d > 0.0f)
		g.side = 1;
	else if (d < 0.0f)
		g.side = -1;
	else
		g.side = g.aim > g.target ? -1 : (g.aim < g.target ? 1 : 0);
}

float yss236_device::lfo_value(lfo const &l)
{
	float const x = float(l.phase) * (1.0f / 4294967296.0f);
	switch (l.wave)
	{
	case 1: return 1.0f - 2.0f * x;
	case 2: return 2.0f * x - 1.0f;
	case 3: return x < 0.5f ? 1.0f : -1.0f;
	case 4: return std::sin(2.0f * PI * x);
	case 5: return l.sh;
	default:
		if (x < 0.25f)
			return 4.0f * x;
		if (x < 0.75f)
			return 2.0f - 4.0f * x;
		return 4.0f * x - 4.0f;
	}
}

TIMER_CALLBACK_MEMBER(yss236_device::tick)
{
	// Audio up to now is rendered with the old EG and LFO values.
	sync_clients();

	u16 done = 0;
	for (int e = 0; e < CHANS; e++)
	{
		eg &g = m_eg[e];
		if (g.rate == 0)
			g.level = float(g.target);
		else if (g.rate != 0xff)
			g.level += (float(g.aim) - g.level) * m_eg_k[g.rate];
		g.level = std::clamp(g.level, -512.0f, 511.0f);
		if (g.side)
		{
			float const d = g.level - float(g.target);
			if ((g.side > 0 && d <= 0.0f) || (g.side < 0 && d >= 0.0f))
			{
				done |= 1 << e;
				g.side = 0;
			}
		}

		lfo &l = m_lfo[e];
		u32 const was = l.phase;
		l.phase += u32(l.speed) << LFO_SPEED_SHIFT;
		if (l.wave == 5 && l.phase < was)
			l.sh = float(s16(machine().rand() & 0xffff)) * (1.0f / 32768.0f);
	}

	for (int s = 0; s < CHANS; s++)
		update_slot(s);

	if (done)
	{
		m_seg_done |= done;
		update_irq();
	}
}

u16 yss236_device::reg_r(offs_t offset)
{
	const u16 index = offset & (REGS - 1);
	if (m_filter_chip && index == PORT_EGQ)
	{
		const u16 pending = seg_pending();
		return pending ? (0x10 | std::countr_zero(pending)) : 0;
	}
	return m_reg[index];
}

void yss236_device::note_coef(u16 addr, u16 data)
{
	for (int s = 0; s < CHANS; s++)
	{
		slot_map const &m = m_map[s];
		chan &c = m_chan[s];
		if (m.cut && addr == m.cut)
			c.cut = data;
		else if (m.reso && addr == m.reso)
			c.reso = data;
		else if (m.feg_depth && addr == m.feg_depth)
			c.feg_depth = s16(data);
		else if (m.lfo_depth && addr == m.lfo_depth)
			c.lfo_depth = s16(data);
		else if (m.gain && addr == m.gain)
			c.gain = data;
		else
			continue;
		update_slot(s);
	}
}

void yss236_device::note_prog(u16 step)
{
	// Program word 4 (reg 6) bits 7-13 pick the tap the slot outputs.
	u8 const tap = (m_prog[step][4] >> 7) & 0x7f;
	for (int s = 0; s < CHANS; s++)
	{
		slot_map const &m = m_map[s];
		if (!m.prog || m.prog != step)
			continue;
		auto const t = std::find(std::begin(m.tap), std::end(m.tap), tap);
		if (t != std::end(m.tap))
		{
			m_chan[s].type = u8(t - std::begin(m.tap));
			update_slot(s);
		}
	}
}

void yss236_device::reg_w(offs_t offset, u16 data)
{
	const u16 index = offset & (REGS - 1);
	LOGMASKED(LOG_REG, "%s: w [%02x] = %04x (sel %04x)\n", machine().describe_context(), index, data, m_select);

	m_reg[index] = data;
	if (index == 0)
	{
		m_select = data;
		return;
	}

	// With bit 15 of the latch set, each data write advances the address.
	const u16 step = m_select & (STEPS - 1);
	if (BIT(m_select, 15))
		m_select = (m_select & ~(STEPS - 1)) | ((step + 1) & (STEPS - 1));

	// Shadow what the chip is given, indexed as the latch addresses it.
	switch (index)
	{
	case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: m_prog[step][10 - index] = data; break;
	case 0x0b: m_coef[step] = data; break;
	case 0x0c: m_tag[step] = u8(data); break;
	case 0x0d: case 0x0e: m_slot[step & 0x7f][index - 0x0d] = data; break;
	case 0x16: m_cfg[0][step & 15] = data; break;
	case 0x28: case 0x29: case 0x2a: m_cfg[index - 0x27][step & 15] = data; break;
	case 0x24: case 0x25: case 0x26: case 0x27: m_lfo_reg[step & 15][index - 0x24] = data; break;
	}
	if (!m_filter_chip)
		return;

	sync_clients();
	switch (index)
	{
	case 0x06:
		note_prog(step);
		break;

	case 0x0b:
		note_coef(step, data);
		break;

	case PORT_EGQ:
		m_seg_enable = data;
		update_irq();
		break;

	case PORT_LFO_SYNC:
		// Mask of EG channels, not the selected one.
		for (int e = 0; e < CHANS; e++)
			if (BIT(data, e))
				m_lfo[e].phase = u32(m_lfo[e].phase0) << 27;
		break;

	case PORT_RATE:
	case PORT_TARGET:
	case PORT_AIM:
	case PORT_LFO_SPEED:
	case PORT_LFO_WAVE:
		if (step < CHANS)
		{
			eg &g = m_eg[step];
			lfo &l = m_lfo[step];
			switch (index)
			{
			case PORT_RATE:
				g.rate = u8(data);
				g.side = 0;
				if (g.rate == 0)
					g.level = float(g.target);
				m_seg_done &= ~(1 << step);
				update_irq();
				break;
			case PORT_TARGET:
				g.target = s16(data);
				if (g.rate == 0)
					g.level = float(g.target);
				arm_eg(g);
				break;
			case PORT_AIM:
				g.aim = s16(data);
				break;
			case PORT_LFO_SPEED:
				l.speed = data & 0x3ff;
				break;
			case PORT_LFO_WAVE:
				l.wave = (data >> 5) & 7;
				l.phase0 = data & 0x1f;
				break;
			}
			update_slot(m_eg_slot[step]);
		}
		break;
	}
}

void yss236_device::update_slot(int slot)
{
	chan &c = m_chan[slot];
	// A cleared cutoff cell (key on) passes the note dry.
	c.bypass = c.cut < u16(CUT_BASE);
	if (c.bypass)
		return;

	eg const &g = m_eg[m_slot_eg[slot]];
	lfo const &l = m_lfo[m_slot_eg[slot]];
	float word = float(c.cut) + float(c.feg_depth) * g.level * FEG_SCALE;
	if (c.lfo_depth)
		word += float(c.lfo_depth) * 0.5f * lfo_value(l);
	word = std::clamp(word, CUT_BASE, CUT_MAX);

	float const sr = float(sample_rate());
	// The measured corner of cutoff byte 0 is 17.4 Hz.
	float const fc = std::min(17.4f * std::exp2((word - CUT_BASE) / CUT_OCTAVE), sr * 0.45f);

	// Feedback tracks the cube of the 1.15 resonance word.
	float const a = std::clamp(float(c.reso) / 32768.0f, 0.0f, 1.0f);
	float const k = std::min(3.65f * a * a * a, 3.4f);

	c.in_gain = float(c.gain) * (1.0f / 4096.0f);
	if (c.type >= 2)
	{
		// LPF12, HPF, BPF, BEF: two-pole state variable filter.
		c.g = std::tan(PI * fc / sr);
		c.r = 0.5f / (0.707f + 2.0f * k);
		c.k = 0.0f;
	}
	else
	{
		// LPF24 and LPF18: zero-delay ladders of 4 and 3 one-poles, fed back
		// from the last pole. The corner above is the cascade's, so each pole is tuned
		// sharper. A 3-pole ladder needs twice the feedback to ring as much.
		bool const four = c.type == 0;
		float const g = std::tan(PI * std::min(fc * (four ? 2.3f : 2.0f), sr * 0.45f) / sr);
		c.g = g / (1.0f + g);
		c.k = four ? k : 2.0f * k;
		c.r = 0.0f;
	}
}

void yss236_device::filter_frame(int base, float *notes)
{
	for (int i = 0; i < CHANS; i++)
		notes[i] = filter_voice(base + i, notes[i]);
}

float yss236_device::filter_voice(int n, float x)
{
	chan const &c = m_chan[n & 15];
	voice &v = m_voice[n];
	if (c.bypass)
		return x;
	x *= c.in_gain;

	float y;
	if (c.type >= 2)
	{
		float const g = c.g;
		float const r2 = 2.0f * c.r;
		float const hp = (x - (r2 + g) * v.s1 - v.s2) / (1.0f + r2 * g + g * g);
		float const bp = g * hp + v.s1;
		v.s1 = g * hp + bp;
		float const lp = g * bp + v.s2;
		v.s2 = g * bp + lp;
		switch (c.type)
		{
		case 2:  y = lp; break;
		case 3:  y = hp; break;
		case 4:  y = bp; break;
		default: y = lp + hp; break;
		}
	}
	else
	{
		// Solve the feedback loop for this sample, then run the poles.
		// Feedback costs passband level. Half of it is made up at the input.
		int const poles = c.type == 0 ? 4 : 3;
		float const G = c.g;
		float sum = 0.0f;
		float gn = 1.0f;
		for (int i = poles - 1; i >= 0; i--)
		{
			sum += gn * v.z[i] * (1.0f - G);
			gn *= G;
		}
		float const in = x * (1.0f + 0.5f * c.k);
		float u = in - c.k * (gn * in + sum) / (1.0f + c.k * gn);
		for (int i = 0; i < poles; i++)
		{
			float const d = (u - v.z[i]) * G;
			u = d + v.z[i];
			v.z[i] = std::clamp(u + d, -4.0f, 4.0f);
		}
		y = u;
	}
	return std::clamp(y, -2.0f, 2.0f);
}
