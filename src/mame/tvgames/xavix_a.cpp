// license:BSD-3-Clause
// copyright-holders:Ramacat, David Haywood

#include "emu.h"
#include "xavix.h"

// #define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device, "xavix_sound", "XaviX Sound")

xavix_sound_device::xavix_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XAVIX_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_readregs_cb(*this, 0xff)
	, m_writeregs_cb(*this)
	, m_readsamples_cb(*this, 0x80)
{
}

void xavix_sound_device::device_start()
{
	m_stream = stream_alloc(0, 2, 163840);

	save_item(NAME(m_tp_dev));
	save_item(NAME(m_pitch_countdown));
	save_item(NAME(m_cyclerate_dev));

	save_item(STRUCT_MEMBER(m_voice, enabled));
	save_item(STRUCT_MEMBER(m_voice, position));
	save_item(STRUCT_MEMBER(m_voice, loopposition));
	save_item(STRUCT_MEMBER(m_voice, loopendposition));
	save_item(STRUCT_MEMBER(m_voice, startposition));

	save_item(STRUCT_MEMBER(m_voice, envpositionleft));
	save_item(STRUCT_MEMBER(m_voice, envpositionright));
	save_item(STRUCT_MEMBER(m_voice, envbank));
	save_item(STRUCT_MEMBER(m_voice, envmode));

	save_item(STRUCT_MEMBER(m_voice, bank));
	save_item(STRUCT_MEMBER(m_voice, rate));
	save_item(STRUCT_MEMBER(m_voice, type));
	save_item(STRUCT_MEMBER(m_voice, vol));

	save_item(STRUCT_MEMBER(m_voice, env_rom_base_left));
	save_item(STRUCT_MEMBER(m_voice, env_rom_base_right));

	save_item(STRUCT_MEMBER(m_voice, env_vol_left));
	save_item(STRUCT_MEMBER(m_voice, env_vol_right));

	save_item(STRUCT_MEMBER(m_voice, env_period_samples));
	save_item(STRUCT_MEMBER(m_voice, env_countdown));
	save_item(STRUCT_MEMBER(m_voice, env_active_left));
	save_item(STRUCT_MEMBER(m_voice, env_active_right));
	save_item(STRUCT_MEMBER(m_voice, env_phase));
	save_item(STRUCT_MEMBER(m_voice, la_byte));
	save_item(STRUCT_MEMBER(m_voice, ra_byte));

	save_item(STRUCT_MEMBER(m_mix, monoural));
	save_item(STRUCT_MEMBER(m_mix,capacity));
	save_item(STRUCT_MEMBER(m_mix,amp));
	save_item(STRUCT_MEMBER(m_mix,dac));
	save_item(STRUCT_MEMBER(m_mix,gap));
	save_item(STRUCT_MEMBER(m_mix,lead));
	save_item(STRUCT_MEMBER(m_mix,lag));
	save_item(STRUCT_MEMBER(m_mix,mastervol));
	save_item(STRUCT_MEMBER(m_mix,gain));
}

void xavix_sound_device::device_reset()
{
	m_cyclerate_dev = 0x0f;

	// hack, does not seem to get set up properly?
	for (int i = 0; i < 4; i++)
		if (m_tp_dev[i] == 0)
			m_tp_dev[i] = 1;

	for (int v = 0; v < 16; v++)
	{
		m_voice[v].enabled = 0;
		m_voice[v].position = 0;
		m_voice[v].loopposition = 0;
		m_voice[v].bank = 0;

		m_voice[v].env_vol_left = 0x00;
		m_voice[v].env_vol_right = 0x00;

		m_voice[v].env_period_samples = tempo_to_period_samples(m_tp_dev[v & 3]);
		m_voice[v].env_countdown = m_voice[v].env_period_samples;
		m_voice[v].env_active_left = 1;
		m_voice[v].env_active_right = 1;

		m_pitch_countdown[v] = 0;
	}

	m_mix.monoural = 0;
	m_mix.capacity = 0;
	m_mix.amp = 2;
	m_mix.dac = 0;
	m_mix.gap = 0;
	m_mix.lead = 0;
	m_mix.lag = 0;

	m_mix.mastervol = 0xff;
	m_mix.gain = 2;
}

void xavix_sound_device::sound_stream_update(sound_stream &stream)
{
	// multiplexed DAC channel visit order
	static const uint8_t kMuxVisitOrder[16] = {	0x0, 0xa, 0x7, 0xd,	0xc, 0x6, 0xB, 0x1,	0x4, 0xe, 0x3, 0x9,	0x8, 0x2, 0xf, 0x5 };

	int outpos = 0;
	int num_samples = stream.samples();

	while (num_samples-- != 0)
	{
		int64_t total_l = 0;
		int64_t total_r = 0;

		// visit voices in order hardware reads seem to indicate
		for (int idx = 0; idx < 16; idx++)
		{
			const int v = kMuxVisitOrder[idx];
			if (!m_voice[v].enabled)
				continue;

			int32_t sample = 0;
			uint8_t raw = 0;
			uint8_t wv = 0x80;

			// WM1 (square) or ROM path
			if (m_voice[v].type == 1)
			{
				const bool sq = ((m_voice[v].position >> 14) & 1) != 0;
				wv = sq ? 0x81 : 0x7f;
				sample = int32_t(wv) - 128;
			}
			else
			{
				const uint32_t pos = (m_voice[v].bank << 16) | (m_voice[v].position >> 14);
				raw = m_readsamples_cb(pos);

				if (raw == 0x80)
				{
					sample = 0;
					if (m_voice[v].type == 3)
					{
						m_voice[v].enabled = 0;
						continue;
					}
					else if (m_voice[v].type == 2)
					{
						if ((m_voice[v].position >> 14) != (m_voice[v].loopposition >> 14))
							m_voice[v].position = m_voice[v].loopposition;
					}
				}
				else
				{
					wv = ((raw & 0x7f) == 0)
						? 0x80
						: (uint8_t)((~raw & 0x80) | (raw & 0x7f));
					sample = int32_t(wv) - 128;
				}
			}

			// mix
			const int32_t gn = (m_voice[v].vol & 0x0f);
			const int32_t mvol = m_mix.mastervol;
			const int32_t env_l = m_voice[v].env_vol_left;
			const int32_t env_r = m_voice[v].env_vol_right;

			const int64_t base = int64_t(sample) * int64_t(gn);

			int64_t left = (base * int64_t(mvol)) / 255;
			left = (left * int64_t(env_l)) / 255;
			total_l += left;

			int64_t right = (base * int64_t(mvol)) / 255;
			right = (right * int64_t(env_r)) / 255;
			total_r += right;

			// advance phase
			m_voice[v].position += m_voice[v].rate;

			// engine work for this voice happens in this slot
			step_envelope(v);
			step_pitch(v);

			if (m_voice[v].env_vol_left == 0 && m_voice[v].env_vol_right == 0)
			{
				m_voice[v].enabled = 0;
				continue;
			}
		}

		total_l *= m_mix.gain;
		total_r *= m_mix.gain;

		int32_t out_l = (total_l > 32767) ? 32767 : (total_l < -32768 ? -32768 : int32_t(total_l));
		int32_t out_r = (total_r > 32767) ? 32767 : (total_r < -32768 ? -32768 : int32_t(total_r));

		if (m_mix.monoural)
		{
			const int32_t mono = (out_l + out_r) / 2;
			out_l = out_r = mono;
		}

		stream.add_int(0, outpos, out_l, 32768);
		stream.add_int(1, outpos, out_r, 32768);

		outpos++;
	}
}

bool xavix_sound_device::is_voice_enabled(int voice)
{
	m_stream->update();
	return m_voice[voice].enabled ? true : false;
}

static inline uint16_t inc_low_nibble(uint16_t x)
{
	return uint16_t((x & 0xfff0) | ((x + 1) & 0x000f));
};

inline void xavix_sound_device::step_side1(int channel, int voice, const uint8_t la, const uint8_t ra)
{
	uint32_t& pos = channel ? m_voice[voice].envpositionright : m_voice[voice].envpositionleft;
	uint8_t& lvl = channel ? m_voice[voice].env_vol_right : m_voice[voice].env_vol_left;
	const uint16_t addr = pos;
	const uint8_t  val = fetch_env_byte_direct(voice, channel, addr);
	lvl = val;
	const bool adv = channel ? (ra != 0) : (la != 0);
	if (adv)
		pos = uint16_t(addr + 1);
};

void xavix_sound_device::enable_voice(int voice, bool update_only)
{
	m_stream->update();
	const int base = voice * 0x10;

	// Wave registers
	const uint16_t wave_control = (m_readregs_cb(base + 0x1) << 8) | m_readregs_cb(base + 0x0);
	const uint16_t wave_addr = (m_readregs_cb(base + 0x3) << 8) | m_readregs_cb(base + 0x2);
	const uint16_t wave_loop_addr = (m_readregs_cb(base + 0x5) << 8) | m_readregs_cb(base + 0x4);
	const uint8_t  wave_addr_bank = m_readregs_cb(base + 0x6);

	// Envelope registers
	const uint8_t  env_config = m_readregs_cb(base + 0x8);
	const uint16_t env_addr_left = (m_readregs_cb(base + 0xb) << 8) | m_readregs_cb(base + 0xa);
	const uint16_t env_addr_right = (m_readregs_cb(base + 0xd) << 8) | m_readregs_cb(base + 0xc);
	const uint8_t  env_addr_bank = m_readregs_cb(base + 0xe);
	// const uint8_t  env_vol_reg = m_readregs_cb(base + 0xf); // (read but not used here)

	// Always refresh fields that may be live-tweaked from RAM
	m_voice[voice].vol = env_config & 0x0f;
	m_voice[voice].envmode = (env_config >> 4) & 0x03;
	m_voice[voice].envbank = env_addr_bank;
	m_voice[voice].env_rom_base_left = env_addr_left;
	m_voice[voice].env_rom_base_right = env_addr_right;

	// Update-only: do NOT re-init pointers/values mid-stream
	if (update_only)
		return;

	// Full (re)start
	m_voice[voice].enabled = 1;
	m_voice[voice].bank = wave_addr_bank;
	m_voice[voice].position = uint32_t(wave_addr) << 14;
	m_voice[voice].loopposition = (wave_loop_addr ? (uint32_t(wave_loop_addr - 1) << 14) : 0);
	m_voice[voice].type = wave_control & 0x3;
	m_voice[voice].rate = wave_control >> 2;
	m_voice[voice].startposition = m_voice[voice].position;

	// Env tempo
	const uint8_t tp = m_tp_dev[voice & 3];
	m_voice[voice].env_period_samples = tp ? tempo_to_period_samples(tp) : 0;
	m_voice[voice].env_countdown = m_voice[voice].env_period_samples; // 0 means paused
	m_voice[voice].env_active_left = 1;
	m_voice[voice].env_active_right = 1;
	m_voice[voice].env_phase = 0;

	// Initial envelope values per voice mode
	if (m_voice[voice].envmode == 0)
	{
		const int base = voice * 0x10;
		const uint8_t la = m_readregs_cb(base + 0xA);
		const uint8_t ra = m_readregs_cb(base + 0xC);
		m_voice[voice].env_vol_left = la;
		m_voice[voice].env_vol_right = ra;
	}
	else if (m_voice[voice].envmode == 1)
	{
		// Start from the configured 16-bit addresses (not LA/RA mirrors)
		const uint16_t start_l = env_addr_left;
		const uint16_t start_r = env_addr_right;

		// Fetch first envelope levels
		const uint8_t v_l = fetch_env_byte_direct(voice, false, start_l);
		const uint8_t v_r = fetch_env_byte_direct(voice, true, start_r);
		m_voice[voice].env_vol_left = v_l;
		m_voice[voice].env_vol_right = v_r;

		// Prime per-side phase counters to the *next* nibble entry
		m_voice[voice].envpositionleft = inc_low_nibble(start_l);
		m_voice[voice].envpositionright = inc_low_nibble(start_r);

		// Keep both sides active; engine will step from envposition{L,R}
		m_voice[voice].env_active_left = 1;
		m_voice[voice].env_active_right = 1;

		// Mirror current pointer low bytes to LA/RA (hardware behavior)
		const int regbase = voice * 0x10;
		m_writeregs_cb(regbase + 0x0a, uint8_t(start_l)); // LA low
		m_writeregs_cb(regbase + 0x0c, uint8_t(start_r)); // RA low
		return;
	}

	else if (m_voice[voice].envmode == 2)
	{
		const int base = voice * 0x10;
		const uint8_t la = m_readregs_cb(base + 0xA); // enable for L
		const uint8_t ra = m_readregs_cb(base + 0xC); // enable for R

		step_side1(0, voice, la, ra);
		step_side1(1, voice, la, ra);
		return;
	}
	else if (m_voice[voice].envmode == 3)
	{
		// do nothing; VM3 decays from current register values
	}
}

void xavix_sound_device::disable_voice(int voice)
{
	m_stream->update();
	m_voice[voice].enabled = 0;
}

uint8_t xavix_sound_device::sound_volume_r()
{
	return m_mix.mastervol;
}

void xavix_sound_device::sound_volume_w(uint8_t data)
{
	set_mastervol(data);
}

uint8_t xavix_sound_device::sound_mixer_r()
{
	return (m_mix.monoural ? 0x80 : 0x00)
		| ((m_mix.capacity & 0x03) << 4)
		| (m_mix.amp & 0x07);
}

void xavix_sound_device::sound_mixer_w(uint8_t data)
{
	m_mix.monoural = (data >> 7) & 0x01;
	m_mix.capacity = (data >> 4) & 0x03;
	m_mix.amp = data & 0x07;

	set_dac_gain(m_mix.amp);
	set_output_mode(m_mix.monoural);

	LOG(" sound_mixer_w monoural=%d  capacity=%d  amp=%d\n",
		m_mix.monoural, m_mix.capacity, m_mix.amp);
}

uint8_t xavix_sound_device::dac_control_r()
{
	return (m_mix.dac << 7)
		| ((m_mix.gap & 0x03) << 5)
		| ((m_mix.lead & 0x07) << 2)
		| (m_mix.lag & 0x03);
}

void xavix_sound_device::dac_control_w(uint8_t data)
{
	m_mix.dac = (data >> 7) & 0x01;
	m_mix.gap = (data >> 5) & 0x03;
	m_mix.lead = (data >> 2) & 0x07;
	m_mix.lag = data & 0x03;

	LOG(" sound_dac_control_w dac=%d gap=%d lead=%d lag=%d\n",
		m_mix.dac, m_mix.gap, m_mix.lead, m_mix.lag);
}

void xavix_sound_device::set_mastervol(uint8_t data)
{
	m_mix.mastervol = data;
}

void xavix_sound_device::set_dac_gain(uint8_t amp_data)
{
	static const int s_amp_table[8] = { 2, 4, 8, 12, 16, 20, 20, 20 };
	m_mix.gain = s_amp_table[amp_data & 0x07];
}

void xavix_sound_device::set_output_mode(bool mono)
{
	m_mix.monoural = mono;
}

uint32_t xavix_sound_device::tempo_to_period_samples(uint8_t tp) const
{
	if (tp == 0)
		return 0; // paused

	const uint32_t samplate_div = uint32_t(m_cyclerate_dev) + 1;
	const uint32_t tp_units = uint32_t(tp) << 4;
	const uint64_t period = uint64_t(samplate_div) * tp_units * 16u;
	return period > 2'000'000 ? 2'000'000u : uint32_t(period);
}

void xavix_sound_device::set_tempo(int index, uint8_t value)
{
	if (index < 0 || index > 3)
		return;

	if (m_stream)
		m_stream->update();

	m_tp_dev[index] = value; // 0 = pause

	LOG(" [snd] tp%d <= %02x\n", index, m_tp_dev[index]);

	for (int v = 0; v < 16; v++)
	{
		if ((v & 3) != index)
			continue;

		xavix_voice& vv = m_voice[v];

		const uint32_t newp = (value == 0) ? 0u : tempo_to_period_samples(value);

		vv.env_period_samples = newp;

		if (newp)
		{
			// running (or changed rate): arm a full period before the next tick
			vv.env_countdown = newp;
		}

		LOG(" [snd]   v=%d period -> %u (countdown=%u)\n", v, newp, vv.env_countdown);
	}
}

void xavix_sound_device::set_cyclerate(uint8_t value)
{
	if (m_stream)
		m_stream->update();

	m_cyclerate_dev = value;

	// Recompute per-voice periods from TPx with the new samplate
	for (int v = 0; v < 16; v++)
	{
		xavix_voice& vv = m_voice[v];
		const uint8_t tp = m_tp_dev[v & 3];

		const uint32_t oldp_raw = vv.env_period_samples;          // may be 0
		const uint32_t newp = (tp == 0) ? 0 : tempo_to_period_samples(tp);

		if (oldp_raw && newp)
			vv.env_countdown = (uint64_t)vv.env_countdown * newp / oldp_raw; // scale to next tick
		else if (!oldp_raw && newp)
			vv.env_countdown = newp; // arm one full period

		vv.env_period_samples = newp;
	}
}

uint8_t xavix_sound_device::fetch_env_byte(int voice, int channel, uint32_t idx)
{
	const xavix_voice& v = m_voice[voice];
	const uint16_t base = channel ? v.env_rom_base_right : v.env_rom_base_left;
	const uint32_t addr = (uint32_t(v.envbank) << 16) | uint32_t((base + (idx & 0xffff)) & 0xffff);
	return m_readsamples_cb(addr);
}

uint8_t xavix_sound_device::fetch_env_byte_direct(int voice, int channel, uint16_t addr)
{
	const uint8_t  bank = m_voice[voice].envbank;
	const uint32_t rom = (uint32_t(bank) << 16) | uint32_t(addr);
	const uint8_t  val = m_readsamples_cb(rom);

	// One-shot “first fetch” debug per voice & side to verify addressing.
	static bool first_seen[16][2] = { { false } };
	const int side = channel ? 1 : 0;
	if (!first_seen[voice][side])
		first_seen[voice][side] = true;

	return val;
}

inline void xavix_sound_device::step_side_env_vm1(int channel, xavix_voice v, int voice)
{
	if (channel ? !v.env_active_right : !v.env_active_left)
		return;

	const uint16_t base = channel ? v.env_rom_base_right : v.env_rom_base_left;
	uint32_t& phase = channel ? v.envpositionright : v.envpositionleft;

	const uint8_t  ph = uint8_t(phase & 0x000f);
	const uint16_t addr = uint16_t((base & 0xfff0) | ph);
	const uint8_t  lvl = fetch_env_byte_direct(voice, channel, addr);

	if (channel)
		v.env_vol_right = lvl;
	else
		v.env_vol_left = lvl;

	const int regbase = voice * 0x10;

	if (channel)
		m_writeregs_cb(regbase + 0x0c, uint8_t(addr)); // RA low
	else
		m_writeregs_cb(regbase + 0x0a, uint8_t(addr)); // LA low

	phase = uint16_t((phase & 0xfff0) | ((ph + 1) & 0x0f));
};

inline void xavix_sound_device::step_side_env_vm2(int channel, xavix_voice v, int voice)
{
	if (channel ? !v.env_active_right : !v.env_active_left)
		return;

	uint32_t& pos = channel ? v.envpositionright : v.envpositionleft;

	const uint8_t val = fetch_env_byte_direct(voice, channel, pos);

	if (channel)
		v.env_vol_right = val;
	else
		v.env_vol_left = val;

	pos = uint16_t(pos + 1);
};

inline uint8_t xavix_sound_device::decay(uint8_t x)
{
	return uint8_t(x - (x >> 4) - ((x & 0x0f) ? 1 : 0));
};

void xavix_sound_device::step_envelope(int voice)
{
	// Per-voice one-shot logging flags, local to this function only.
	static bool s_logged_start[16] = { false };
	static bool s_logged_stop[16] = { false };

	xavix_voice& v = m_voice[voice];

	// If the voice is disabled, clear flags so next enable logs a fresh START.
	if (!v.enabled)
	{
		s_logged_start[voice] = false;
		s_logged_stop[voice] = false;
		return;
	}

	// One-time START log when we first process an enabled voice.
	if (!s_logged_start[voice])
	{
		const uint8_t group = voice & 3;
		const uint8_t tp = m_tp_dev[group];
		const uint32_t period = (tp == 0) ? 0u : tempo_to_period_samples(tp);

		LOG("[ENV START] v=%d  vm=%u  wm=%u  gn=%u  tp[%u]=%02x  cr=%u  period=%u\n"
			"           envbank=%02x  LA_base=%04x  RA_base=%04x\n"
			"           L=%02x R=%02x  posL=%04x posR=%04x  wbank=%02x waddr=%06x loop=%06x rate=%u\n",
			voice, (unsigned)(v.envmode & 3), (unsigned)(v.type & 3), (unsigned)(v.vol & 0x0f),
			(unsigned)group, (unsigned)tp, (unsigned)(m_cyclerate_dev + 1), (unsigned)period,
			(unsigned)v.envbank, (unsigned)v.env_rom_base_left, (unsigned)v.env_rom_base_right,
			(unsigned)v.env_vol_left, (unsigned)v.env_vol_right,
			(unsigned)(v.envpositionleft & 0xffff), (unsigned)(v.envpositionright & 0xffff),
			(unsigned)v.bank, (unsigned)((v.bank << 16) | (v.position >> 14)),
			(unsigned)(v.loopposition >> 14), (unsigned)v.rate);

		s_logged_start[voice] = true;
		s_logged_stop[voice] = false;
	}

	const uint32_t target_period = tempo_to_period_samples(m_tp_dev[voice & 3]);

	// Smooth re-tune to new period
	if (v.env_period_samples != target_period)
	{
		const uint32_t oldp = v.env_period_samples ? v.env_period_samples : 1;
		v.env_countdown = (uint64_t)v.env_countdown * target_period / oldp;
		v.env_period_samples = target_period;
	}

	// Tick gate
	if (v.env_period_samples == 0)
		return;

	if (v.env_countdown)
	{
		v.env_countdown--;
		return;
	}

	v.env_countdown = v.env_period_samples;

	// ---- VM0 : direct registers reflected into levels ----
	if (v.envmode == 0)
	{
		const int base = voice * 0x10;
		v.env_vol_left = m_readregs_cb(base + 0xa); // LA
		v.env_vol_right = m_readregs_cb(base + 0xc); // RA
		if ((v.env_vol_left | v.env_vol_right) == 0 && !s_logged_stop[voice])
		{
			LOG("[ENV STOP ] v=%d  vm=0  wm=%u  L=00 R=00 (VM0 both zero)\n",
				voice, (unsigned)(v.type & 3));
			s_logged_stop[voice] = true;
		}
		return;
	}

	// ---- VM1 : nibble-table ----
	if (v.envmode == 1)
	{
		step_side_env_vm1(0, v, voice);
		step_side_env_vm1(1, v, voice);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!s_logged_stop[voice])
			{
				LOG("[ENV STOP ] v=%d  vm=1  wm=%u  L=%02x R=%02x  (VM1 L|R==0)\n",
					voice, (unsigned)(v.type & 3),
					(unsigned)v.env_vol_left, (unsigned)v.env_vol_right);
				s_logged_stop[voice] = true;
			}
			v.env_active_left = 0;
			v.env_active_right = 0;
		}
		return;
	}

	// ---- VM2 : linear ROM stream per side ----
	if (v.envmode == 2)
	{
		step_side_env_vm2(0, v, voice);
		step_side_env_vm2(1, v, voice);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!s_logged_stop[voice])
			{
				LOG("[ENV STOP ] v=%d  vm=2  wm=%u  L=%02x R=%02x  (VM2 L|R==0)\n",
					voice, (unsigned)(v.type & 3),
					(unsigned)v.env_vol_left, (unsigned)v.env_vol_right);
				s_logged_stop[voice] = true;
			}
			v.env_active_left = 0;
			v.env_active_right = 0;
		}
		return;
	}

	// ---- VM3 : exponential-ish decay ----
	if (v.envmode == 3)
	{
		v.env_vol_left = decay(v.env_vol_left);
		v.env_vol_right = decay(v.env_vol_right);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!s_logged_stop[voice])
			{
				LOG("[ENV STOP ] v=%d  vm=3  wm=%u  (VM3 decayed to 0)\n",
					voice, (unsigned)(v.type & 3));
				s_logged_stop[voice] = true;
			}
			v.env_active_left = 0;
			v.env_active_right = 0;
		}
	}
}

void xavix_sound_device::step_pitch(int voice)
{
	xavix_voice& v = m_voice[voice];
	if (!v.enabled)
		return;

	const int base = voice * 0x10;
	const uint16_t wave_control =
		(uint16_t(m_readregs_cb(base + 0x01)) << 8) |
		uint16_t(m_readregs_cb(base + 0x00));

	const uint32_t target = uint32_t(wave_control >> 2) & 0x3fff;
	uint32_t current = v.rate & 0x3fff;

	// Early out if matched
	if (current == target)
		return;

	const int32_t diff = int32_t(target) - int32_t(current);
	const bool    up = (diff > 0);

	uint8_t j = up ? (m_mix.lead & 0x07) : (m_mix.lag & 0x03);

	if ((v.type & 0x3) == 1 && j > 0)
		j -= 1;

	uint32_t mag = uint32_t(up ? diff : -diff);
	uint32_t step = (j ? (mag >> j) : mag);

	if (j)
	{
		const uint32_t frac_mask = (1u << j) - 1u;
		const uint32_t frac = mag & frac_mask;
		m_pitch_countdown[voice] += frac;
		if (m_pitch_countdown[voice] >= (1u << j))
		{
			m_pitch_countdown[voice] -= (1u << j);
			step += 1;
		}
	}
	else
	{
		// No fraction when j==0; keep accumulator quiet
		m_pitch_countdown[voice] = 0;
	}

	// Enforce a minimum step so very small diffs still move
	const uint32_t min_step = uint32_t(m_mix.gap & 0x03) + 1u;
	if (step < min_step) step = min_step;

	// Apply step with no overshoot (borrow/carry guarded)
	if (up)
	{
		current += step;
		if (current > target)
			current = target;
	}
	else
	{
		if (current > step)
			current -= step;
		else
			current = 0;

		if (current < target)
			current = target;
	}

	v.rate = current & 0x3fff;
}

uint8_t xavix_state::sound_regram_read_cb(offs_t offset)
{
	// 0x00 would be zero page memory; assume it's not valid
	if ((m_sound_regbase & 0x3f) != 0x00)
	{
		const uint16_t memorybase = (m_sound_regbase & 0x3f) << 8;
		return m_mainram[memorybase + offset];
	}
	return 0x00;
}

void xavix_state::sound_regram_write_cb(offs_t offset, u8 data)
{
	if ((m_sound_regbase & 0x3f) != 0x00)
	{
		const uint16_t memorybase = (m_sound_regbase & 0x3f) << 8;
		m_mainram[memorybase + offset] = data;
	}
}

uint8_t xavix_state::sound_voice_startstop_r(offs_t offset)
{
	return m_soundreg16_0[offset];
}

void xavix_state::sound_voice_startstop_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		const int mask = (1 << i);
		const int voice_state = (data & mask);
		const int old_voice_state = (m_soundreg16_0[offset] & mask);
		if (voice_state != old_voice_state)
		{
			const int voice = (offset * 8 + i);
			if (voice_state) m_sound->enable_voice(voice, false);
			else             m_sound->disable_voice(voice);
		}
	}
	m_soundreg16_0[offset] = data;
}

uint8_t xavix_state::sound_voice_updateenv_r(offs_t offset)
{
	// On real hardware, might be read-only or always return 0.
	return 0x00;
}

void xavix_state::sound_voice_updateenv_w(offs_t offset, uint8_t data)
{
	for (int i = 0; i < 8; i++)
	{
		if (data & (1 << i))
		{
			const int voice = (offset * 8 + i);
			m_sound->enable_voice(voice, true); // refresh params/envelope from regs/ROM
		}
	}
}

uint8_t xavix_state::sound_voice_status_r(offs_t offset)
{
	uint8_t ret = 0x00;
	for (int i = 0; i < 8; i++)
	{
		const int voice = (offset * 8 + i);
		if (m_sound->is_voice_enabled(voice))
			ret |= 1 << i;
	}
	return ret;
}

uint8_t xavix_state::sound_regbase_r()
{
	return m_sound_regbase & 0x3f; // upper bits read as 0
}

void xavix_state::sound_regbase_w(uint8_t data)
{
	// upper 6 bits of RAM address where the per-voice register sets live
	m_sound_regbase = data & 0x3f;
	//LOG("%s: sound_regbase_w %02x (sound regs at %02x00-%02xff)\n",
	//  machine().describe_context(), data, m_sound_regbase & 0x3f, m_sound_regbase & 0x3f);
}

//-------------------------------------------------
//  cyclerate / samplate (0x75F8)
//-------------------------------------------------

uint8_t xavix_state::sound_cyclerate_r()
{
	return m_cyclerate;
}

void xavix_state::sound_cyclerate_w(uint8_t data)
{
	m_cyclerate = data; // store for readback / debug
	if (m_sound) m_sound->set_cyclerate(data);
	LOG("  sound_cyclerate_w %02x\n", m_cyclerate);
}

uint8_t xavix_state::sound_volume_r() { return m_sound->sound_volume_r(); }
void    xavix_state::sound_volume_w(uint8_t data) { m_sound->sound_volume_w(data); }

uint8_t xavix_state::sound_mixer_r() { return m_sound->sound_mixer_r(); }
void    xavix_state::sound_mixer_w(uint8_t data) { m_sound->sound_mixer_w(data); }

uint8_t xavix_state::sound_dac_control_r() { return m_sound->dac_control_r(); }
void    xavix_state::sound_dac_control_w(uint8_t data) { m_sound->dac_control_w(data); }

// tempo registers
uint8_t xavix_state::sound_tp0_r() { LOG("%s: sound_tp0_r\n", machine().describe_context()); return m_tp[0]; }
uint8_t xavix_state::sound_tp1_r() { LOG("%s: sound_tp1_r\n", machine().describe_context()); return m_tp[1]; }
uint8_t xavix_state::sound_tp2_r() { LOG("%s: sound_tp2_r\n", machine().describe_context()); return m_tp[2]; }
uint8_t xavix_state::sound_tp3_r() { LOG("%s: sound_tp3_r\n", machine().describe_context()); return m_tp[3]; }

void xavix_state::sound_tp0_w(uint8_t data) { m_tp[0] = data; if (m_sound) m_sound->set_tempo(0, data); LOG(" sound_tp0_w %02x\n", data); }
void xavix_state::sound_tp1_w(uint8_t data) { m_tp[1] = data; if (m_sound) m_sound->set_tempo(1, data); LOG(" sound_tp1_w %02x\n", data); }
void xavix_state::sound_tp2_w(uint8_t data) { m_tp[2] = data; if (m_sound) m_sound->set_tempo(2, data); LOG(" sound_tp2_w %02x\n", data); }
void xavix_state::sound_tp3_w(uint8_t data) { m_tp[3] = data; if (m_sound) m_sound->set_tempo(3, data); LOG(" sound_tp3_w %02x\n", data); }

uint8_t xavix_state::sound_irq_status_r()
{
	// UK e-kara carts check the upper nibble for sound-timer IRQ source
	return m_sound_irqstatus;
}

void xavix_state::sound_irq_status_w(uint8_t data)
{
	// these look like irq ack bits, 4 sources?
	// related to sound_timer0_w ,  sound_timer1_w,  sound_timer2_w,  sound_timer3_w  ?

	for (int t = 0; t < 4; t++)
	{
		int bit = (1 << t) << 4;

		if (data & bit)
		{
			m_sound_irqstatus &= ~data & bit;
		}
	}

	// check if all interrupts have been cleared to see if the line should be lowered
	if (m_sound_irqstatus & 0xf0)
		m_irqsource |= 0x80;
	else
		m_irqsource &= ~0x80;

	update_irqs();


	for (int t = 0; t < 4; t++)
	{
		int bit = 1 << t;

		if ((m_sound_irqstatus & bit) != (data & bit))
		{
			if (data & bit)
			{
				/* period should be based on m_sndtimer[t] at least, maybe also some other regs?

				   rad_crdn : sound_timer0_w 06
				   ddrfammt, popira etc. : sound_timer3_w 80
				   so higher value definitely needs to be faster? (unless there's another multiplier elsewhere)

				   11 is too fast (popira checked on various tracks, finish before getting to 100% then jump to 100%) where is this multiplier coming from? clock divided?
				   10 seems close to correct for ddrfammt, popira, might need fine tuning.  seems too slow for rad_crdn / rad_bass?
				   tweaked to 10.3f stay in time with the first song in https://www.youtube.com/watch?v=3x1C9bhC2rc

				   the usual clock divided by 2 would be 10.738636 but that's too high
				*/
				attotime period = attotime::from_hz(10.3f * (m_tp[t]));
				m_sound_timer[t]->adjust(period, t, period);
			}
			else
			{
				m_sound_timer[t]->adjust(attotime::never, t);
			}
		}
	}

	// see if we're enabling any timers (should probably check if they're already running so we don't end up restarting them)
	m_sound_irqstatus |= data & 0x0f; // look like IRQ enable flags - 4 sources? voices? timers?


	//LOG("%s: sound_irqstatus_w %02x\n", machine().describe_context(), data);
}

// used by ekara (UK cartridges), rad_bass, rad_crdn
TIMER_CALLBACK_MEMBER(xavix_state::sound_timer_done)
{
	// param = timer number 0,1,2 or 3
	const int bit = (1 << param) << 4;
	m_sound_irqstatus |= bit;

	// if any of the sound timers are causing an interrupt...
	if (m_sound_irqstatus & 0xf0) m_irqsource |= 0x80;
	else                          m_irqsource &= ~0x80;

	update_irqs();
}
