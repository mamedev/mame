// license:BSD-3-Clause
// copyright-holders:ramacat, David Haywood

// XaviX Audio Renderer overview:
// - The sound core mixes up to 16 voices per sample at a fixed render rate.
// - Each voice reads 8-bit inverted sign-magnitude PCM, with four wave modes (WM):
//     WM0: RAM wavetable (CPU can live-edit)      WM1: noise LFSR
//     WM2: ROM wavetable w/ loop                  WM3: ROM one-shot (stops at 0x80)
// - Phase is 18.14 fixed-point; per-voice 'rate' (from wave_control>>2) is scaled by 'phase_step_per_tick()'.
// - Byte value 0x80 is a sentinel: WM2 loops to 'loop_position'; WM3 disables the voice.
// - Envelope Modes (VM) control per-side levels:
//     VM0: direct LA/RA registers (no clock); VM1: nibble table; VM2: byte stream; VM3: exponential-ish decay.
// - Envelope advance is driven by per-group tempo tp[0..3] and 'cyclerate':
//     tempo_to_period_samples(tp) -> ticks; VM1/2/3 step on ticks; VM0 ignores tempo.
// - Hardware defaults tp[*]=0 pauses ticks -> VM1/2/3 won’t finish; we use a generic fallback prevent stuck voices.
// - Pitch bends: 'step_pitch()' slews 'rate' by +/-1 per render tick toward wave_control>>2,
//   producing smooth glides without zippering.
// - Mixer visit order emulates hardware scan; order changes when DAC broadcast mode is set.
// - Per-voice volume = sample * voice gain(4-bit) * master volume * envelope L/R, then summed.
// - 'capacity' may drop higher-index voices; 'monaural' averages L/R at the end.
// - Output gain uses a small discrete table; results are clamped to 16-bit before stream write.
// - 'cyclerate' retunes tempo periods on the fly; countdowns are rescaled to avoid phase jumps.
// - Title code controls registers via a memory-mapped page; RAM reads reflect live CPU edits.
// - Noise (WM1) uses a 16-bit LFSR with a fixed/nonzero seed; seed writes reinitialize it.
// - DAC "gap/lead/lag" bits are exposed but many titles leave them at reset defaults.
// - IRQs: per-group tempo timers latch pending bits; VM ticking doesn’t imply an IRQ unless
//   the group enable bit is set (emulation mirrors that behavior).

#include "emu.h"
#include "xavix.h"

#define VERBOSE (0)

#define LOG_CFG        (1U << 1)
#define LOG_TEMPO      (1U << 2)
#define LOG_TIMER      (1U << 3)
#define LOG_IRQ        (1U << 4)
#define LOG_VOICE      (1U << 5)
#define LOG_WAVE       (1U << 6)
#define LOG_ENV        (1U << 7)
#define LOG_ENV_DATA   (1U << 8)
#define LOG_PITCH      (1U << 9)
#define LOG_SCAN       (1U << 10)
#define LOG_CLIP       (1U << 11)
#define LOG_NOISE      (1U << 12)

//#define LOGCTX(mask, fmt, ...) LOGMASKED((mask), "%s: " fmt, machine().describe_context(), ##__VA_ARGS__)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(XAVIX_SOUND, xavix_sound_device, "xavix_sound", "XaviX Sound")

namespace
{
	// Internal sequencer rate works out to be 167'791 Hz.
	// - default cyclerate at reset is 0x0f; 2 phase updates per 16-state frame -> CORE_CLK / (( cyclerate + 1 ) * 8 )
	// - titles never seem to change cyclerate, and if they did, it's not possible to dynamically change the MAME stream after stream_alloc.
	// - 42Mhz CPU devices also run their sound core at 21MHz.
	// - DAC timing knobs:
	//     - lead/lag shift the latch strobe within the 16-cycle frame,
	//     - gap!=0 issues a second write strobe (at 3−lag+gap), duplicating the held sample within the frame.
	//     - no titles found that change these from the defaults, so unable to test.
	//   These alter *when* a channel is latched (and can duplicate it), but not the state-machine rate itself.

	static constexpr uint8_t MIXER_ORDER_MULTIPLEX[16] = { 0x0, 0xa, 0x7, 0xd, 0xc, 0x6, 0xb, 0x1, 0x4, 0xe, 0x3, 0x9, 0x8, 0x2, 0xf, 0x5 };
	static constexpr uint8_t MIXER_ORDER_BROADCAST[16] = { 0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf };
	static constexpr int AMP_TABLE[8] = { 2, 4, 8, 12, 16, 20, 20, 20 };
}

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
	m_sequencer_rate_hz = clock() / ((0x0f + 1u) * 8u); // Hardware Rate (167'791)
	m_stream = stream_alloc(0, 2, m_sequencer_rate_hz);
	LOGMASKED(LOG_CFG, "[cfg] start render_rate=%uHz\n", m_sequencer_rate_hz);

	// dividers
	save_item(NAME(m_tempo_div));
	save_item(NAME(m_cyclerate_div));

	// voice parameters
	save_item(STRUCT_MEMBER(m_voice, enabled));
	save_item(STRUCT_MEMBER(m_voice, position));
	save_item(STRUCT_MEMBER(m_voice, loop_position));
	save_item(STRUCT_MEMBER(m_voice, start_position));

	save_item(STRUCT_MEMBER(m_voice, bank));
	save_item(STRUCT_MEMBER(m_voice, rate));
	save_item(STRUCT_MEMBER(m_voice, type));
	save_item(STRUCT_MEMBER(m_voice, vol));

	save_item(STRUCT_MEMBER(m_voice, noise_state));

	// envelope parameters
	save_item(STRUCT_MEMBER(m_voice, env_rom_base_left));
	save_item(STRUCT_MEMBER(m_voice, env_rom_base_right));

	save_item(STRUCT_MEMBER(m_voice, env_vol_left));
	save_item(STRUCT_MEMBER(m_voice, env_vol_right));

	save_item(STRUCT_MEMBER(m_voice, env_pos_left));
	save_item(STRUCT_MEMBER(m_voice, env_pos_right));
	save_item(STRUCT_MEMBER(m_voice, env_bank));
	save_item(STRUCT_MEMBER(m_voice, env_mode));

	save_item(STRUCT_MEMBER(m_voice, env_period_samples));
	save_item(STRUCT_MEMBER(m_voice, env_countdown));
	save_item(STRUCT_MEMBER(m_voice, env_active_left));
	save_item(STRUCT_MEMBER(m_voice, env_active_right));

	// dac parameters
	save_item(STRUCT_MEMBER(m_mix, monaural));
	save_item(STRUCT_MEMBER(m_mix, capacity));
	save_item(STRUCT_MEMBER(m_mix, amp));
	save_item(STRUCT_MEMBER(m_mix, dac));
	save_item(STRUCT_MEMBER(m_mix, gap));
	save_item(STRUCT_MEMBER(m_mix, lead));
	save_item(STRUCT_MEMBER(m_mix, lag));
	save_item(STRUCT_MEMBER(m_mix, mastervol));
	save_item(STRUCT_MEMBER(m_mix, gain));

	// log triggers
	save_item(STRUCT_MEMBER(m_voice, log_env_started));
	save_item(STRUCT_MEMBER(m_voice, log_env_stopped));
	save_item(STRUCT_MEMBER(m_voice, log_env_paused));
}

void xavix_sound_device::device_reset()
{
	// hardware default on init
	m_cyclerate_div = 0x0f;

	// TODO: on tempo group (tp[0..3]) defaults:
	// - On reset the hardware tempo registers power up as 0x00. When tp==0, the tempo
	//   tick for that group is halted: VM1/VM2/VM3 envelopes do not advance, and the
	//   group’s audio timer IRQ source remains idle. VM0 (direct levels) is unaffected.
	// - Many titles either never program any tp or only write tp[3] (e.g. hikara, popira).
	//   If left at 0x00, non-VM0 voices can 'stick on' because their envelopes never
	//   progress to zero.
	// - We set a generic default of 0x40 which is a compromise across titles.
	// - This results in the engine also generating audio IRQs for those the affected tempo groups, however,
	//   in practice this does not seem to cause any issues and reflects the actual hardware behaviour.
	// - With this generic default, some games sound unnaturally long/echoey (e.g. taitons2) while others become too abrupt.
	// - When device-/title-specific initialization is confirmed, replace the generic default
	//   with those exact defaults and remove the heuristic.
	for (int i = 0; i < 4; i++)
		m_tempo_div[i] = 0x40; // initialised by hardware to 0x00

	for (int v = 0; v < 16; v++)
	{
		m_voice[v].enabled = 0;
		m_voice[v].position = 0;
		m_voice[v].loop_position = 0;
		m_voice[v].bank = 0;

		m_voice[v].env_vol_left = 0x00;
		m_voice[v].env_vol_right = 0x00;

		m_voice[v].env_period_samples = envelope_period_ticks(m_tempo_div[v & 3]);
		m_voice[v].env_countdown = m_voice[v].env_period_samples;
		m_voice[v].env_active_left = 1;
		m_voice[v].env_active_right = 1;
		m_voice[v].noise_state = 0;

	}

	// hardware defaults on init
	m_mix.monaural = 0;
	m_mix.capacity = 0;
	m_mix.amp = 2;
	m_mix.dac = 0;
	m_mix.gap = 0;
	m_mix.lead = 0;
	m_mix.lag = 0;
	m_mix.mastervol = 0xff;
	m_mix.gain = 2;

	LOGMASKED(LOG_CFG, "[cfg] reset cyclerate=%02x tempo_default=%02x monaural=%d capacity=%d amp=%d\n",
		m_cyclerate_div, m_tempo_div[0], m_mix.monaural, m_mix.capacity, m_mix.amp);
}

void xavix_sound_device::sound_stream_update(sound_stream &stream)
{
	int out_pos = 0;
	int num_samples = stream.samples();

	while (num_samples-- != 0)
	{
		const uint8_t* visit_order = m_mix.dac ? MIXER_ORDER_BROADCAST : MIXER_ORDER_MULTIPLEX;

		// capacity (00=16ch, 01=8ch, 1x=4ch), applied to the *visit order* like the hardware does.
		const uint8_t cap = m_mix.capacity & 0x03;
		const int allowed_channels = (cap & 0x02) ? 4 : (cap & 0x01) ? 8 : 16;

		LOGMASKED(LOG_SCAN, "[scan] order=%s capacity=%u allowed=%d\n",
			m_mix.dac ? "broadcast" : "multiplex", unsigned(cap), allowed_channels);

		int64_t total_l = 0;
		int64_t total_r = 0;

		// visit voices in the observed hardware order; drop anything past the capacity window
		for (int idx = 0; idx < 16; idx++)
		{
			if (idx >= allowed_channels)
				continue;

			const int v = visit_order[idx];
			if (!m_voice[v].enabled)
				continue;

			int32_t sample = 0;
			uint8_t raw = 0;
			uint8_t wv = 0x80;

			// WM1 (noise)
			if (m_voice[v].type == 1)
			{
				uint16_t state = m_voice[v].noise_state ? m_voice[v].noise_state : 0xace1u;
				const uint16_t feedback = ((state >> 0) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5)) & 1;
				state = (state >> 1) | (feedback << 15);
				if (state == 0) {
					state = 0xace1u; // TODO: Use actual seed value
					LOGMASKED(LOG_NOISE, "[noise] v=%2d reseed=ACE1\n", v);
				}
				m_voice[v].noise_state = state;
				const int8_t noise = int8_t(((state >> 8) & 0xff) ^ 0x80);
				sample = int32_t(noise);
			}
			else
			{
				const uint32_t phase = m_voice[v].position >> 14;

				if (m_voice[v].type == 0)
				{
					// TODO: is anything using this voice type?
					// should it read from map(0x7400, 0x757f).ram() which is tested by gungunrv?
					raw = 0x80;
				}
				else
				{
					const uint32_t pos = (m_voice[v].bank << 16) | phase;
					raw = m_readsamples_cb(pos);
				}

				if (raw == 0x80)
				{
					sample = 0;
					if (m_voice[v].type == 3)
					{
						LOGMASKED(LOG_WAVE, "[wave] v=%2d one-shot end @%06x\n",
							v, (m_voice[v].bank << 16) | (m_voice[v].position >> 14));
						m_voice[v].enabled = 0; // one-shot ends
						continue;
					}
					else if (m_voice[v].type == 2 || m_voice[v].type == 0)
					{
						LOGMASKED(LOG_WAVE, "[wave] v=%2d loop -> %06x\n",
							v, unsigned(m_voice[v].loop_position >> 14));
						if ((m_voice[v].position >> 14) != (m_voice[v].loop_position >> 14))
							m_voice[v].position = m_voice[v].loop_position;
					}
				}
				else
				{
					// inverted sign-magnitude to signed PCM
					wv = ((raw & 0x7f) == 0) ? 0x80 : (uint8_t)((~raw & 0x80) | (raw & 0x7f));
					sample = int32_t(wv) - 128;
				}
			}

			// per-voice + master + envelope
			const int32_t gn   = (m_voice[v].vol & 0x0f);
			const int32_t mvol = m_mix.mastervol;
			const int32_t env_l = m_voice[v].env_vol_left;
			const int32_t env_r = m_voice[v].env_vol_right;

			const int64_t base = int64_t(sample) * int64_t(gn);

			int64_t left  = (base * int64_t(mvol)) / 255;
			int64_t right = (base * int64_t(mvol)) / 255;

			total_l += (left  * int64_t(env_l)) / 255;
			total_r += (right * int64_t(env_r)) / 255;

			// advance voice state
			m_voice[v].position += phase_step_per_tick(m_voice[v].rate);
			step_envelope(v);
			step_pitch(v);

			// auto-disable if both sides silent
			if (m_voice[v].env_vol_left == 0 && m_voice[v].env_vol_right == 0)
			{
				m_voice[v].enabled = 0;
				continue;
			}
		}

		auto apply_final_gain = [&](int64_t x) -> int64_t
		{
			// capacity: 00=16ch, 01=8ch, 1x=4ch
			const uint8_t cap = m_mix.capacity & 0x03;
			const int allowed = (cap & 0x02) ? 4 : (cap & 0x01) ? 8 : 16;
			const int amp = m_mix.gain;

			// post-mix is multiplied by the amp code and divided by ~1000;
			// approximate that non-power-of-two divide with a 3/128 fixed-point factor.
			// this is imperfect as it will depend on the analgue path implementation surrounding the XaviX core
			// which will likely vary based on the physical characteristics of the device.
			int64_t y = x * int64_t(amp) * int64_t(allowed) * 3;
			return (y >= 0) ? ((y + 64) >> 7) : -(((-y) + 64) >> 7);
		};

		total_l = apply_final_gain(total_l);
		total_r = apply_final_gain(total_r);

		if (total_l > 32767 || total_l < -32768 || total_r > 32767 || total_r < -32768)
		{
			LOGMASKED(LOG_CLIP, "[clip] L=%lld R=%lld\n",
				(long long)total_l, (long long)total_r);
		}

		int32_t out_l = (total_l > 32767) ? 32767 : (total_l < -32768 ? -32768 : int32_t(total_l));
		int32_t out_r = (total_r > 32767) ? 32767 : (total_r < -32768 ? -32768 : int32_t(total_r));

		if (m_mix.monaural)
		{
			const int32_t mono = (out_l + out_r) / 2;
			out_l = out_r = mono;
		}

		stream.add_int(0, out_pos, out_l, 32768);
		stream.add_int(1, out_pos, out_r, 32768);
		out_pos++;
	}
}


bool xavix_sound_device::is_voice_enabled(int voice)
{
	m_stream->update();
	return m_voice[voice].enabled ? true : false;
}

void xavix_sound_device::enable_voice(int voice, bool update_only)
{
	m_stream->update();
	const int base = voice * 0x10;

	LOGMASKED(LOG_VOICE, "[voice] enable v=%2d update_only=%d type(prev)=%u env_mode(prev)=%u\n",
		voice, update_only ? 1 : 0, unsigned(m_voice[voice].type & 3), unsigned(m_voice[voice].env_mode & 3));

	// Wave registers
	const uint16_t wave_control = (m_readregs_cb(base + 0x1) << 8) | m_readregs_cb(base + 0x0);
	const uint16_t wave_addr = (m_readregs_cb(base + 0x3) << 8) | m_readregs_cb(base + 0x2);
	const uint16_t wave_loop_addr = (m_readregs_cb(base + 0x5) << 8) | m_readregs_cb(base + 0x4);
	const uint8_t  wave_addr_bank = m_readregs_cb(base + 0x6);
	const uint8_t  new_type = wave_control & 0x3;
	const uint32_t new_rate = wave_control >> 2;

	// Envelope registers
	const uint8_t  env_config = m_readregs_cb(base + 0x8);
	const uint16_t env_addr_left = (m_readregs_cb(base + 0xB) << 8) | m_readregs_cb(base + 0xA);
	const uint16_t env_addr_right = (m_readregs_cb(base + 0xD) << 8) | m_readregs_cb(base + 0xC);
	const uint8_t  env_addr_bank = m_readregs_cb(base + 0xE);
	//const uint8_t  env_vol_reg = m_readregs_cb(base + 0xF); // unused but read for completeness
	//(void)env_vol_reg;

	// Always refresh fields that may be live-tweaked from RAM
	m_voice[voice].vol = env_config & 0x0f;
	m_voice[voice].env_mode = (env_config >> 4) & 0x03;
	m_voice[voice].env_bank = env_addr_bank;
	m_voice[voice].env_rom_base_left = env_addr_left;
	m_voice[voice].env_rom_base_right = env_addr_right;
	m_voice[voice].type = new_type;
	m_voice[voice].rate = new_rate;

	LOGMASKED(LOG_VOICE,
		"[voice] v=%2d regs type=%u rate=%u bank=%02x start=%06x loop=%06x\n"
		"  vm=%u vol=%u env_bank=%02x la=%04x ra=%04x\n",
		voice,
		unsigned(new_type), unsigned(new_rate), unsigned(wave_addr_bank),
		unsigned((wave_addr_bank << 16) | wave_addr),
		unsigned((wave_addr_bank << 16) | wave_loop_addr),
		unsigned(m_voice[voice].env_mode), unsigned(m_voice[voice].vol),
		unsigned(m_voice[voice].env_bank), unsigned(env_addr_left), unsigned(env_addr_right));

	if (new_type == 1 && m_voice[voice].noise_state == 0)
		m_voice[voice].noise_state = wave_addr ? wave_addr : 0xace1u;

	if (update_only)
		return;

	// udance enables what seems like an invalid voice when it should be silent, not clear what the hardware would do in this case
	if (!update_only && !(new_type & 1) && wave_addr == wave_loop_addr)
	{
		m_voice[voice].enabled = 0;
		return;
	}

	// Full (re)start
	m_voice[voice].enabled = 1;
	m_voice[voice].bank = wave_addr_bank;
	m_voice[voice].position = uint32_t(wave_addr) << 14;
	// TODO: Work out why subtracting a single sample from the wave loop address is needed for loops to be in tune.
	m_voice[voice].loop_position = (wave_loop_addr ? (uint32_t(wave_loop_addr - 1) << 14) : 0);
	m_voice[voice].start_position = m_voice[voice].position;
	m_voice[voice].noise_state = wave_addr ? wave_addr : 0xace1u;

	// Envelope tempo
	const uint8_t tp = m_tempo_div[voice & 3];
	m_voice[voice].env_period_samples = tp ? envelope_period_ticks(tp) : 0;
	m_voice[voice].env_countdown = m_voice[voice].env_period_samples; // 0 means envelope paused
	m_voice[voice].env_active_left = 1;
	m_voice[voice].env_active_right = 1;

	// Initial envelope values per voice mode
	if (m_voice[voice].env_mode == 0)
	{
		const int base = voice * 0x10;
		const uint8_t la = m_readregs_cb(base + 0xa);
		const uint8_t ra = m_readregs_cb(base + 0xc);
		m_voice[voice].env_vol_left = la;
		m_voice[voice].env_vol_right = ra;
	}
	else if (m_voice[voice].env_mode == 1)
	{
		// Start from the configured 16-bit addresses
		const uint16_t start_l = env_addr_left;
		const uint16_t start_r = env_addr_right;

		auto inc_low_nibble = [](uint16_t x) {
			return uint16_t((x & 0xfff0) | ((x + 1) & 0x000f));
			};

		// Fetch first envelope levels
		const uint8_t v_l = fetch_env_byte_direct(voice, false, start_l);
		const uint8_t v_r = fetch_env_byte_direct(voice, true, start_r);
		m_voice[voice].env_vol_left = v_l;
		m_voice[voice].env_vol_right = v_r;

		// Prime per-side phase counters to the *next* nibble entry
		m_voice[voice].env_pos_left = inc_low_nibble(start_l);
		m_voice[voice].env_pos_right = inc_low_nibble(start_r);

		// Keep both sides active;
		m_voice[voice].env_active_left = 1;
		m_voice[voice].env_active_right = 1;

		// Mirror current pointer low bytes to LA/RA (hardware behavior)
		const int regbase = voice * 0x10;
		m_writeregs_cb(regbase + 0x0a, uint8_t(start_l)); // LA low
		m_writeregs_cb(regbase + 0x0c, uint8_t(start_r)); // RA low
		return;
	}

	else if (m_voice[voice].env_mode == 2)
	{
		const int regbase = voice * 0x10;
		// VM2 streams: seed both sides from their 16-bit ROM addresses
		m_voice[voice].env_pos_left = env_addr_left;
		m_voice[voice].env_pos_right = env_addr_right;

		auto prime_side = [&](int channel) {
			uint32_t& pos = channel ? m_voice[voice].env_pos_right : m_voice[voice].env_pos_left;
			uint8_t& lvl = channel ? m_voice[voice].env_vol_right : m_voice[voice].env_vol_left;
			const uint16_t addr = uint16_t(pos & 0xffff);
			const uint8_t  val = fetch_env_byte_direct(voice, channel, addr);
			lvl = val;

			if (channel) m_writeregs_cb(regbase + 0x0c, uint8_t(addr)); // ra low mirror
			else       m_writeregs_cb(regbase + 0x0a, uint8_t(addr)); // la low mirror

			pos = uint16_t(addr + 1);
			};

		prime_side(0);
		prime_side(1);
		return;
	}
	else if (m_voice[voice].env_mode == 3)
	{
		// VM3 starts from the register-programmed levels
		m_voice[voice].env_vol_left = m_readregs_cb(base + 0xa);
		m_voice[voice].env_vol_right = m_readregs_cb(base + 0xc);
	}

	m_voice[voice].log_env_started = 0;
	m_voice[voice].log_env_stopped = 0;
	m_voice[voice].log_env_paused = 0;
}

void xavix_sound_device::disable_voice(int voice)
{
	m_stream->update();
	m_voice[voice].enabled = 0;
	m_voice[voice].log_env_started = 0;
	m_voice[voice].log_env_stopped = 0;
	m_voice[voice].log_env_paused = 0;
}

uint8_t xavix_sound_device::sound_volume_r()
{
	return m_mix.mastervol;
}

void xavix_sound_device::sound_volume_w(uint8_t data)
{
	m_stream->update();
	set_mastervol(data);
	LOGMASKED(LOG_CFG, "[cfg] mixer mastervol=%u\n", unsigned(data));
}

uint8_t xavix_sound_device::sound_mixer_r()
{
	return (m_mix.monaural ? 0x80 : 0x00)
		| ((m_mix.capacity & 0x03) << 4)
		| (m_mix.amp & 0x07);
}

void xavix_sound_device::sound_mixer_w(uint8_t data)
{
	m_stream->update();
	m_mix.monaural = BIT(data, 7);
	m_mix.capacity = BIT(data, 4, 2);
	m_mix.amp      = BIT(data, 0, 3);

	set_dac_gain(m_mix.amp);
	set_output_mode(m_mix.monaural);

	LOGMASKED(LOG_CFG, "[cfg] mixer monaural=%d capacity=%d amp=%d\n",
		m_mix.monaural, m_mix.capacity, m_mix.amp);
}

uint8_t xavix_sound_device::dac_control_r()
{
	return (BIT(m_mix.dac, 0) << 7)
		| (BIT(m_mix.gap, 0, 2)  << 5)
		| (BIT(m_mix.lead, 0, 3) << 2)
		|  BIT(m_mix.lag, 0, 2);
}

void xavix_sound_device::dac_control_w(uint8_t data)
{
	m_stream->update();
	m_mix.dac  = BIT(data, 7);
	m_mix.gap  = BIT(data, 5, 2);
	m_mix.lead = BIT(data, 2, 3);
	m_mix.lag  = BIT(data, 0, 2);

	LOGMASKED(LOG_CFG, "[cfg] dac dac=%d gap=%d lead=%d lag=%d\n",
		m_mix.dac, m_mix.gap, m_mix.lead, m_mix.lag);
}

void xavix_sound_device::set_mastervol(uint8_t data)
{
	m_mix.mastervol = data;
}

void xavix_sound_device::set_dac_gain(uint8_t amp_data)
{
	m_mix.gain = AMP_TABLE[amp_data & 0x07];
}

void xavix_sound_device::set_output_mode(bool mono)
{
	m_mix.monaural = mono;
}

uint32_t xavix_sound_device::tempo_to_period_ticks(uint8_t tp) const
{
	if (!tp) return 0;                                 // paused
	const uint32_t s = uint32_t(m_cyclerate_div) + 1;  // cyclerate+1
	uint64_t samples = (uint64_t(s) * 1024u + (tp / 2)) / tp; // round-nearest
	if (samples < 1) samples = 1;
	if (samples > 2'000'000u) samples = 2'000'000u;
	return uint32_t(samples);
}

uint32_t xavix_sound_device::envelope_period_ticks(uint8_t tp) const
{
	const uint32_t base = tempo_to_period_ticks(tp);
	if (!base) return 0;
	const uint64_t scaled = uint64_t(base) * 16u;      // envelope cadence
	return uint32_t(scaled > 64'000'000u ? 64'000'000u : scaled);
}

attotime xavix_sound_device::tempo_period(uint8_t tempo) const
{
	const uint32_t samples = tempo_to_period_ticks(tempo);
	if (!samples) return attotime::never;
	return attotime::from_ticks(samples, m_sequencer_rate_hz); // engine ticks
}

double xavix_sound_device::tempo_tick_hz(uint8_t tempo) const
{
	const uint32_t samples = tempo_to_period_ticks(tempo);
	return samples ? (double(m_sequencer_rate_hz) / double(samples)) : 0.0;
}

uint32_t xavix_sound_device::phase_step_per_tick(uint32_t rate) const
{
	if (!rate) return 0;
	uint64_t step = rate; // sequencer rate equals timing base
	return uint32_t(step ? step : 1);
}

void xavix_sound_device::set_tempo(int index, uint8_t value)
{
	if (index < 0 || index > 3)
		return;

	if (m_stream)
		m_stream->update();

	m_tempo_div[index] = value; // 0 = pause

	LOGMASKED(LOG_TEMPO, "[tempo] tp[%d]=%02x\n", index, m_tempo_div[index]);

	for (int v = 0; v < 16; v++)
	{
		if ((v & 3) != index)
			continue;

		xavix_voice& vv = m_voice[v];

		const uint32_t newp = (value == 0) ? 0u : envelope_period_ticks(value);

		vv.env_period_samples = newp;

		if (newp)
		{
			// running (or changed rate): arm a full period before the next tick
			vv.env_countdown = newp;
		}

		LOGMASKED(LOG_TEMPO, "[tempo] v=%2d period=%u countdown=%u\n", v, newp, vv.env_countdown);
	}
}

void xavix_sound_device::set_cyclerate(uint8_t value)
{
	if (m_stream)
		m_stream->update();

	m_cyclerate_div = value;
	LOGMASKED(LOG_TEMPO, "[tempo] cyclerate=%02x\n", m_cyclerate_div);

	// Recompute per-voice periods
	for (int v = 0; v < 16; v++)
	{
		xavix_voice& vv = m_voice[v];
		const uint8_t tp = m_tempo_div[v & 3];

		const uint32_t oldp_raw = vv.env_period_samples; // may be 0
		const uint32_t newp = (tp == 0) ? 0 : envelope_period_ticks(tp);

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
	const uint32_t addr = (uint32_t(v.env_bank) << 16) | uint32_t((base + (idx & 0xffff)) & 0xffff);
	return m_readsamples_cb(addr);
}

uint8_t xavix_sound_device::fetch_env_byte_direct(int voice, int channel, uint16_t addr)
{
	const uint8_t  bank = m_voice[voice].env_bank;
	const uint32_t rom = (uint32_t(bank) << 16) | uint32_t(addr);
	const uint8_t  val = m_readsamples_cb(rom);

	return val;
}

void xavix_sound_device::step_envelope(int voice)
{
	xavix_voice& v = m_voice[voice];

	bool& logged_start = v.log_env_started;
	bool& logged_stop = v.log_env_stopped;
	bool& logged_pause = v.log_env_paused;

	// If the voice is disabled, clear flags so next enable logs a fresh START.
	if (!v.enabled)
	{
		logged_start = false;
		logged_stop = false;
		logged_pause = false;
		return;
	}

	// One-time START log when we first process an enabled voice.
	if (!logged_start)
	{
		const uint8_t group = voice & 3;
		const uint8_t tp = m_tempo_div[group];
		const uint32_t period = (tp == 0) ? 0u : envelope_period_ticks(tp);

		LOGMASKED(LOG_ENV,
			"[env] start v=%2d vm=%u wm=%u gn=%u tp[%u]=%02x cr=%u period=%u\n"
			"  env_bank=%02x la_base=%04x ra_base=%04x\n"
			"  l=%02x r=%02x posl=%04x posr=%04x wbank=%02x waddr=%06x loop=%06x rate=%u\n",
			voice, unsigned(v.env_mode & 3), unsigned(v.type & 3), unsigned(v.vol & 0x0f),
			unsigned(group), unsigned(tp), unsigned(m_cyclerate_div + 1), unsigned(period),
			unsigned(v.env_bank), unsigned(v.env_rom_base_left), unsigned(v.env_rom_base_right),
			unsigned(v.env_vol_left), unsigned(v.env_vol_right),
			unsigned(v.env_pos_left & 0xffff), unsigned(v.env_pos_right & 0xffff),
			unsigned(v.bank), unsigned((v.bank << 16) | (v.position >> 14)),
			unsigned(v.loop_position >> 14), unsigned(v.rate));

		logged_start = true;
		logged_stop = false;
	}

	const uint32_t target_period = envelope_period_ticks(m_tempo_div[voice & 3]);

	// Smooth re-tune to new period
	if (v.env_period_samples != target_period)
	{
		const uint32_t oldp = v.env_period_samples ? v.env_period_samples : 1;
		v.env_countdown = (uint64_t)v.env_countdown * target_period / oldp;
		v.env_period_samples = target_period;
	}

	// Tick gate
	if (v.env_period_samples == 0)
	{
		if (!logged_pause)
		{
			LOGMASKED(LOG_ENV, "[env] pause  v=%2d vm=%u tp=%02x\n",
				voice, unsigned(v.env_mode & 3), unsigned(m_tempo_div[voice & 3]));
			logged_pause = true;
		}
		return; // tempo paused (tp==0)
	}
	else if (logged_pause)
	{
		LOGMASKED(LOG_ENV, "[env] resume v=%2d vm=%u tp=%02x\n",
			voice, unsigned(v.env_mode & 3), unsigned(m_tempo_div[voice & 3]));
		logged_pause = false;
	}
	if (v.env_countdown) { v.env_countdown--; return; }
	v.env_countdown = v.env_period_samples;

	// VM0: direct registers reflected into levels
	if (v.env_mode == 0)
	{
		const int base = voice * 0x10;
		v.env_vol_left = m_readregs_cb(base + 0xa); // LA
		v.env_vol_right = m_readregs_cb(base + 0xc); // RA
		if ((v.env_vol_left | v.env_vol_right) == 0 && !logged_stop)
		{
			LOGMASKED(LOG_ENV, "[env] stop   v=%2d vm=0 wm=%u L=00 R=00\n", voice, unsigned(v.type & 3));
			logged_stop = true;
		}
		return;
	}

	// VM1: nibble-table
	if (v.env_mode == 1)
	{
		auto advance_nibble = [](uint16_t value) {
			return uint16_t((value & 0xfff0) | ((value + 1) & 0x000f));
		};

		auto step_side = [&](int channel)
		{
			if (channel ? !v.env_active_right : !v.env_active_left)
				return;

			uint32_t &ptr32 = channel ? v.env_pos_right : v.env_pos_left;
			const uint16_t ptr = uint16_t(ptr32);
			const uint8_t level = fetch_env_byte_direct(voice, channel, ptr);

			if (channel)
				v.env_vol_right = level;
			else
				v.env_vol_left = level;

			const uint16_t next_ptr = advance_nibble(ptr);
			ptr32 = next_ptr;

			const int regbase = voice * 0x10;
			if (channel)
				m_writeregs_cb(regbase + 0x0c, uint8_t(next_ptr));
			else
				m_writeregs_cb(regbase + 0x0a, uint8_t(next_ptr));
		};

		step_side(0);
		step_side(1);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!logged_stop)
			{
				LOGMASKED(LOG_ENV, "[env] stop   v=%2d vm=1 wm=%u L=%02x R=%02x\n",
					voice, unsigned(v.type & 3), unsigned(v.env_vol_left), unsigned(v.env_vol_right));
				logged_stop = true;
			}
			v.env_active_left = 0;
			v.env_active_right = 0;
		}
		return;
	}
	// VM2: linear ROM stream per side
	if (v.env_mode == 2)
	{
		const int regbase = voice * 0x10;

		auto step_side = [&](int channel)
		{
			if (channel ? !v.env_active_right : !v.env_active_left)
				return;

			uint32_t &ptr32 = channel ? v.env_pos_right : v.env_pos_left;
			const uint16_t ptr = uint16_t(ptr32);
			const uint8_t level = fetch_env_byte_direct(voice, channel, ptr);

			if (channel)
				v.env_vol_right = level;
			else
				v.env_vol_left = level;

			if (channel)
				m_writeregs_cb(regbase + 0x0c, uint8_t(ptr));
			else
				m_writeregs_cb(regbase + 0x0a, uint8_t(ptr));

			ptr32 = uint16_t(ptr + 1);
		};

		step_side(0);
		step_side(1);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!logged_stop)
			{
				LOGMASKED(LOG_ENV, "[env] stop   v=%2d vm=2 wm=%u L=%02x R=%02x\n",
					voice, unsigned(v.type & 3), unsigned(v.env_vol_left), unsigned(v.env_vol_right));
				logged_stop = true;
			}
			v.env_active_left = false;
			v.env_active_right = false;
		}
		return;
	}
	// VM3: exponential-ish decay
	if (v.env_mode == 3)
	{
		auto decay = [](uint8_t x) -> uint8_t
		{
			const uint8_t high = x >> 4;
			const uint8_t low = x & 0x0f;
			const uint8_t subtract = high + (low ? 1 : 0);
			return (subtract >= x) ? 0 : uint8_t(x - subtract);
		};

		v.env_vol_left = decay(v.env_vol_left);
		v.env_vol_right = decay(v.env_vol_right);

		if ((v.env_vol_left | v.env_vol_right) == 0)
		{
			if (!logged_stop)
			{
				LOGMASKED(LOG_ENV, "[env] stop   v=%2d vm=3 wm=%u (decayed)\n",
					voice, unsigned(v.type & 3));
				logged_stop = true;
			}
			v.env_active_left = false;
			v.env_active_right = false;
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

	const uint32_t target = uint32_t(wave_control >> 2);
	const uint32_t old = v.rate;

	if (v.rate < target)      v.rate += 1;
	else if (v.rate > target) v.rate -= 1;

	if (v.rate != old)
		LOGMASKED(LOG_PITCH, "[pitch] v=%2d %u->%u target=%u\n",
			voice, old, v.rate, target);
}

uint8_t xavix_state::sound_current_page() const
{
	return m_sound_regbase & 0x3f;
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
	LOGMASKED(LOG_VOICE, "[voice] startstop offs=%d data=%02x prev=%02x\n",
		offset, data, m_soundreg16_0[offset]);
	for (int i = 0; i < 8; i++)
	{
		const int voice_state      = BIT(data, i);
		const int old_voice_state  = BIT(m_soundreg16_0[offset], i);
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
	LOGMASKED(LOG_ENV, "[env] update offs=%d mask=%02x\n", offset, data);
	for (int i = 0; i < 8; i++)
	{
		if (BIT(data, i))
		{
			const int voice = (offset * 8 + i);
			m_sound->enable_voice(voice, true);
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
}

uint8_t xavix_state::sound_cyclerate_r()
{
	return m_cyclerate;
}

void xavix_state::sound_cyclerate_w(uint8_t data)
{
	m_cyclerate = data; // store for readback / debug
	if (m_sound) m_sound->set_cyclerate(data);
	LOGMASKED(LOG_CFG, "[cfg] cyclerate=%02x\n", m_cyclerate);
}

uint8_t xavix_state::sound_volume_r() { return m_sound->sound_volume_r(); }
void    xavix_state::sound_volume_w(uint8_t data) { m_sound->sound_volume_w(data); }

uint8_t xavix_state::sound_mixer_r() { return m_sound->sound_mixer_r(); }
void    xavix_state::sound_mixer_w(uint8_t data) { m_sound->sound_mixer_w(data); }

uint8_t xavix_state::sound_dac_control_r() { return m_sound->dac_control_r(); }
void    xavix_state::sound_dac_control_w(uint8_t data) { m_sound->dac_control_w(data); }

// tempo registers
uint8_t xavix_state::sound_tp0_r() { return m_tp[0]; }
uint8_t xavix_state::sound_tp1_r() { return m_tp[1]; }
uint8_t xavix_state::sound_tp2_r() { return m_tp[2]; }
uint8_t xavix_state::sound_tp3_r() { return m_tp[3]; }

void xavix_state::sound_tp0_w(uint8_t data)
{
	m_tp[0] = data;
	if (m_sound) m_sound->set_tempo(0, data);
	LOGMASKED(LOG_TEMPO, "[tempo] tp[%d]=%02x\n", 0, data);
	reprogram_sound_timer(0);
}

void xavix_state::sound_tp1_w(uint8_t data)
{
	m_tp[1] = data;
	if (m_sound) m_sound->set_tempo(1, data);
	LOGMASKED(LOG_TEMPO, "[tempo] tp[%d]=%02x\n", 1, data);
	reprogram_sound_timer(1);
}

void xavix_state::sound_tp2_w(uint8_t data)
{
	m_tp[2] = data;
	if (m_sound) m_sound->set_tempo(2, data);
	LOGMASKED(LOG_TEMPO, "[tempo] tp[%d]=%02x\n", 2, data);
	reprogram_sound_timer(2);
}

void xavix_state::sound_tp3_w(uint8_t data)
{
	m_tp[3] = data;
	if (m_sound) m_sound->set_tempo(3, data);
	LOGMASKED(LOG_TEMPO, "[tempo] tp[%d]=%02x\n", 3, data);
	reprogram_sound_timer(3);
}

uint8_t xavix_state::sound_irq_status_r()
{
	// UK e-kara carts check the upper nibble for sound-timer IRQ source
	return m_sound_irqstatus;
}

void xavix_state::sound_irq_status_w(uint8_t data)
{
	const uint8_t old_enable = m_sound_irqstatus & 0x0f;

	const uint8_t clear_mask = (data >> 4) & 0x0f;
	if (clear_mask)
		m_sound_irqstatus &= ~(clear_mask << 4);

	const uint8_t new_enable = data & 0x0f;
	m_sound_irqstatus = (m_sound_irqstatus & 0xf0) | new_enable;

	const uint8_t pending = (m_sound_irqstatus >> 4) & 0x0f;
	LOGMASKED(LOG_IRQ, "[irq] status_w %02x old_en=%02x new_en=%02x clear=%02x pending=%02x\n",
		data, old_enable, new_enable, clear_mask, pending);

	const uint8_t changed = old_enable ^ new_enable;
	if (changed)
	{
		for (int t = 0; t < 4; t++)
			if (changed & (1 << t))
				reprogram_sound_timer(t);
	}

	refresh_sound_irq_state();
	update_irqs();
}

// used by ekara (UK cartridges), rad_bass, rad_crdn
TIMER_CALLBACK_MEMBER(xavix_state::sound_timer_done)
{
	// param = timer number 0,1,2 or 3
	const uint8_t enable_mask = 1U << param;
	if (!BIT(m_sound_irqstatus, param))
		return;

	m_sound_irqstatus |= (enable_mask << 4);
	LOGMASKED(LOG_TIMER, "[timer] %d latch pending=%02x\n",
		param, (m_sound_irqstatus >> 4) & 0x0f);
	refresh_sound_irq_state();
	update_irqs();
}

void xavix_state::refresh_sound_irq_state()
{
	const uint8_t enable = m_sound_irqstatus & 0x0f;
	const uint8_t pending = (m_sound_irqstatus >> 4) & 0x0f;

	if (enable & pending)
		m_irqsource |= 0x80;
	else
		m_irqsource &= ~0x80;

	LOGMASKED(LOG_IRQ,   "[irq] line %s enable=%02x pending=%02x\n",
		((enable & pending) ? "assert" : "clear"), enable, pending);
}

void xavix_state::reprogram_sound_timer(int index)
{
	if (index < 0 || index >= 4)
		return;
	if (!m_sound_timer[index])
		return;

	const uint8_t mask = 1U << index;
	if (!(m_sound_irqstatus & mask))
	{
		LOGMASKED(LOG_TIMER, "[timer] %d stop (enable=0)\n", index);
		m_sound_timer[index]->adjust(attotime::never, index);
		return;
	}

	const uint8_t tempo = m_tp[index];
	if (tempo == 0)
	{
		LOGMASKED(LOG_TIMER, "[timer] %d stop (tempo=0)\n", index);
		m_sound_timer[index]->adjust(attotime::never, index);
		return;
	}

	const double frequency = m_sound->tempo_tick_hz(tempo);
	if (frequency <= 0.0)
	{
		LOGMASKED(LOG_TIMER, "[timer] %d stop (period unavailable)\n", index);
		m_sound_timer[index]->adjust(attotime::never, index);
		return;
	}

	const attotime period = attotime::from_hz(frequency);
	const std::string period_text = period.as_string(18);
	m_sound_timer[index]->adjust(period, index, period);
	LOGMASKED(LOG_TIMER, "[timer] %d arm tempo=%02x freq=%.6fHz period=%s\n",
		index, tempo, frequency, period_text.c_str());
}
