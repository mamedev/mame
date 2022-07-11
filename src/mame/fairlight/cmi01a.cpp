// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Fairlight CMI-01A Channel Controller Card

***************************************************************************/

#include "emu.h"
#include "cmi01a.h"

#define VERBOSE     (1)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CMI01A_CHANNEL_CARD, cmi01a_device, "cmi_01a", "Fairlight CMI-01A Channel Card")

#define DEBUG_CHANNEL (0)

cmi01a_device::cmi01a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CMI01A_CHANNEL_CARD, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_merger(*this, "cmi01a_irq")
	, m_pia(*this, "cmi01a_pia_%u", 0U)
	, m_ptm(*this, "cmi01a_ptm")
	, m_stream(nullptr)
	, m_irq_cb(*this)
{
}

void cmi01a_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia[0], 0); // 6821 C6/7/8/9
	m_pia[0]->readcb1_handler().set(FUNC(cmi01a_device::tri_r));
	m_pia[0]->readpa_handler().set(FUNC(cmi01a_device::ws_dir_r));
	m_pia[0]->writepa_handler().set(FUNC(cmi01a_device::ws_dir_w));
	m_pia[0]->readpb_handler().set(FUNC(cmi01a_device::rp_r));
	m_pia[0]->writepb_handler().set(FUNC(cmi01a_device::rp_w));
	m_pia[0]->ca2_handler().set(FUNC(cmi01a_device::notload_w));
	m_pia[0]->cb2_handler().set(FUNC(cmi01a_device::run_w));
	m_pia[0]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<1>));
	if (m_channel == DEBUG_CHANNEL)
	{
		m_pia[0]->set_log(true, 0);
	}

	PIA6821(config, m_pia[1], 0); // 6821 D6/7/8/9
	m_pia[1]->readca1_handler().set(FUNC(cmi01a_device::zx_r));
	m_pia[1]->readcb1_handler().set(FUNC(cmi01a_device::eosi_r));
	m_pia[1]->readpa_handler().set(FUNC(cmi01a_device::pitch_octave_r));
	m_pia[1]->writepa_handler().set(FUNC(cmi01a_device::pitch_octave_w));
	m_pia[1]->readpb_handler().set(FUNC(cmi01a_device::pitch_lsb_r));
	m_pia[1]->writepb_handler().set(FUNC(cmi01a_device::pitch_lsb_w));
	m_pia[1]->ca2_handler().set(FUNC(cmi01a_device::permit_eload_w));
	m_pia[1]->cb2_handler().set(FUNC(cmi01a_device::not_wpe_w));
	m_pia[1]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<2>));
	m_pia[1]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<3>));
	if (m_channel == DEBUG_CHANNEL)
	{
		m_pia[1]->set_log(true, 1);
	}

	PTM6840(config, m_ptm, DERIVED_CLOCK(1, 1));
	m_ptm->o1_callback().set(FUNC(cmi01a_device::ptm_o1));
	m_ptm->o2_callback().set(FUNC(cmi01a_device::ptm_o2));
	m_ptm->o3_callback().set(FUNC(cmi01a_device::ptm_o3));
	m_ptm->irq_callback().set(m_irq_merger, FUNC(input_merger_device::in_w<4>));
	if (m_channel == DEBUG_CHANNEL)
	{
		m_ptm->set_log(true, 0);
	}

	INPUT_MERGER_ANY_HIGH(config, m_irq_merger).output_handler().set(FUNC(cmi01a_device::cmi01a_irq));
}


void cmi01a_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	static u16 last_addr = 0;
	if (m_run)
	{
		auto &buf = outputs[0];

		for (int sampindex = 0; sampindex < buf.samples(); sampindex++)
		{
			s32 sample = (s32)(s8)(m_current_sample ^ 0x80);
			if (m_channel != DEBUG_CHANNEL && VERBOSE)
				sample = 0;
			s32 env32 = (s32)m_env;
			s32 vol32 = (s32)m_vol_latch;
			u32 sample32 = (u32)((sample * env32 * vol32) >> 8);
			s16 sample16 = (s16)(u16)sample32;
			if (m_current_sample_addr != last_addr)
			{
				last_addr = m_current_sample_addr;
				if (m_channel == DEBUG_CHANNEL) LOG("CH%d sample[%04x]: %02x, env %08x, vol %08x, s32 %08x, s16 %04x)\n", m_channel, last_addr, m_current_sample, env32, vol32, (u32)sample32, (u16)sample16);
			}
			buf.put_int(sampindex, sample16, 32768);
		}
	}
	else
	{
		outputs[0].fill(0);
	}
}

void cmi01a_device::device_resolve_objects()
{
	m_irq_cb.resolve_safe();
}

TIMER_CALLBACK_MEMBER(cmi01a_device::update_sample)
{
	m_stream->update();
	m_current_sample_addr = ((m_wave_addr_msb << 7) | m_wave_addr_lsb) & 0x3fff;
	m_current_sample = m_wave_ram[m_current_sample_addr];
	set_wave_addr_lsb((m_wave_addr_lsb + 1) & 0x7f);
}

void cmi01a_device::device_start()
{
	m_wave_ram = std::make_unique<u8[]>(0x4000);

	m_bcas_q1_timer = timer_alloc(FUNC(cmi01a_device::bcas_q1_tick), this);
	m_zcint_pulse_timer = timer_alloc(FUNC(cmi01a_device::zcint_pulse_cb), this);
	m_rstb_pulse_timer = timer_alloc(FUNC(cmi01a_device::rstb_pulse_cb), this);
	m_sample_timer = timer_alloc(FUNC(cmi01a_device::update_sample), this);

	m_stream = stream_alloc(0, 1, 48000);

	//m_ptm->set_external_clocks(0, clock() / 4, clock() / 4);
	m_ptm->set_external_clocks(0, 0, 0);
}

void cmi01a_device::device_reset()
{
	m_ptm->set_g1(1);
	m_ptm->set_g2(1);
	m_ptm->set_g3(1);

	m_current_sample = 0;
	m_current_sample_addr = 0x4000;

	m_new_addr = false;
	m_vol_latch = 0;
	m_flt_latch = 0;
	m_rp = 0;
	m_ws = 0;
	m_dir = 0;
	m_env = 0;
	m_bcas_q2 = false;
	m_bcas_q1 = false;
	m_not_rstb = true;

	m_ptm_o1 = 0;
	m_ptm_o2 = 0;
	m_ptm_o3 = 0;

	m_run = false;
	m_gzx = true;
	m_not_wpe = false;
	m_tri = true;
	m_permit_eload = false;

	m_eclk = false;
	m_env_clk = false;
	m_ediv_out = true;
	m_env_divider = 3;
	std::fill(std::begin(m_envdiv_toggles), std::end(m_envdiv_toggles), false);

	m_pitch = 0;
	m_octave = 0;

	//m_bcas_q1_timer->adjust(attotime::never);
	m_bcas_q1_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));
	m_zcint_pulse_timer->adjust(attotime::never);
	m_rstb_pulse_timer->adjust(attotime::never);
	m_sample_timer->adjust(attotime::never);
}

int cmi01a_device::notload_r()
{
	return m_not_load;
}

void cmi01a_device::notload_w(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d notload_w: %d\n", m_channel, state);
	set_not_load(state);
}

void cmi01a_device::load_envelope()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d loading envelope (new rate %02x, dir is %s)\n", m_channel, m_rp, m_dir ? "DOWN" : "UP");
	m_env = m_rp;
	m_env_dir = m_dir;
	update_envelope_tri();
}

void cmi01a_device::pitch_octave_w(u8 data)
{
	m_pitch &= 0x0ff;
	m_pitch |= (data & 3) << 8;
	m_octave = (data >> 2) & 0x0f;
}

u8 cmi01a_device::pitch_octave_r()
{
	return ((m_pitch >> 8) & 3) | (m_octave << 2);
}

void cmi01a_device::pitch_lsb_w(u8 data)
{
	m_pitch &= 0xf00;
	m_pitch |= data;
}

u8 cmi01a_device::pitch_lsb_r()
{
	return (u8)m_pitch;
}

void cmi01a_device::rp_w(u8 data)
{
	m_rp = data;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: RP write: %02x\n", m_channel, data);
}

u8 cmi01a_device::rp_r()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: RP read: %02x\n", m_channel, m_rp);
	return m_rp;
}

void cmi01a_device::ws_dir_w(u8 data)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: WS/DIR write: %02x, pending direction is %s\n", m_channel, data, BIT(data, 7) ? "DOWN" : "UP");
	m_ws = data & 0x7f;
	m_dir = (data >> 7) & 1;
	try_load_upper_wave_addr();
}

u8 cmi01a_device::ws_dir_r()
{
	u8 data = m_ws | (m_dir << 7);
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: WS/DIR read: %02x\n", m_channel, data);
	return data;
}

int cmi01a_device::tri_r()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: PIA0 CB1 Read (/TRI): %d\n", m_channel, m_tri);
	return m_tri;
}

void cmi01a_device::cmi01a_irq(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: Setting IRQ to %d, state %02x\n", m_channel, state, m_irq_merger->m_state);
	m_irq_cb(state ? ASSERT_LINE : CLEAR_LINE);
}

void cmi01a_device::not_wpe_w(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CB2 (/WPE): %d\n", m_channel, state);
	set_not_wpe(state);
}

void cmi01a_device::permit_eload_w(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CA2 write (permit ELOAD): %d\n", m_channel, state);
	m_permit_eload = state;
	update_not_eload();
}

void cmi01a_device::clock_envelope()
{
	if (m_tri)
		return;

	m_stream->update();
	if (m_env_dir == ENV_DIR_DOWN)
	{
		if (m_env > 0)
		{
			m_env--;
			if (m_channel == DEBUG_CHANNEL) LOG("CH%d, Clocking envelope down (rate %02x), new m_env: %02x\n", m_channel, m_env_divider, m_env);
		}
	}
	else
	{
		if (m_env < 0xff)
		{
			m_env++;
			if (m_channel == DEBUG_CHANNEL) LOG("CH%d, Clocking envelope up (rate %02x), new m_env: %02x\n", m_channel, m_env_divider, m_env);
		}
	}
	update_envelope_tri();
}

void cmi01a_device::tick_ediv()
{
	const bool envdiv_enable_a = m_eclk;
	const bool envdiv_enable_b = m_eclk && m_envdiv_toggles[0];
	const bool envdiv_enable_c = m_eclk && m_envdiv_toggles[0] && m_envdiv_toggles[1];
	const bool envdiv_enable_d = m_eclk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2];
	const bool envdiv_enable_e = m_eclk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3];
	const bool envdiv_enable_f = m_eclk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && m_envdiv_toggles[4];

	if (envdiv_enable_f)
		m_envdiv_toggles[5] = !m_envdiv_toggles[5];
	if (envdiv_enable_e)
		m_envdiv_toggles[4] = !m_envdiv_toggles[4];
	if (envdiv_enable_d)
		m_envdiv_toggles[3] = !m_envdiv_toggles[3];
	if (envdiv_enable_c)
		m_envdiv_toggles[2] = !m_envdiv_toggles[2];
	if (envdiv_enable_b)
		m_envdiv_toggles[1] = !m_envdiv_toggles[1];
	if (envdiv_enable_a)
		m_envdiv_toggles[0] = !m_envdiv_toggles[0];

	const bool envdiv_out_f = m_eclk && BIT(m_env_divider, 5) && !m_envdiv_toggles[0];
	const bool envdiv_out_e = m_eclk && BIT(m_env_divider, 4) && m_envdiv_toggles[0] && !m_envdiv_toggles[1];
	const bool envdiv_out_d = m_eclk && BIT(m_env_divider, 3) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && !m_envdiv_toggles[2];
	const bool envdiv_out_c = m_eclk && BIT(m_env_divider, 2) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && !m_envdiv_toggles[3];
	const bool envdiv_out_b = m_eclk && BIT(m_env_divider, 1) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && !m_envdiv_toggles[4];
	const bool envdiv_out_a = m_eclk && BIT(m_env_divider, 0) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && m_envdiv_toggles[4] && !m_envdiv_toggles[5];

	m_ediv_out = !(envdiv_out_f || envdiv_out_e || envdiv_out_d || envdiv_out_c || envdiv_out_b || envdiv_out_a);

	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d ticking ediv, rate: %02x, toggles %d%d%d%d%d%d, %d\n", m_channel, m_env_divider, m_envdiv_toggles[5], m_envdiv_toggles[4], m_envdiv_toggles[3], m_envdiv_toggles[2], m_envdiv_toggles[1], m_envdiv_toggles[0], m_ediv_out ? 1 : 0);
}

void cmi01a_device::run_voice()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d running voice: Pitch = %04x\n", m_channel, (u16)m_pitch);
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d running voice: o_val = %x\n", m_channel, m_octave);

	double cfreq = ((0x800 | (m_pitch << 1)) * m_mosc) / 4096.0;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d running voice: cfreq = %f (%04x * %f) / 4096.0\n", m_channel, cfreq, 0x800 | (m_pitch << 1), m_mosc, cfreq);

	if (cfreq > m_mosc)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d Voice run has excessive frequency\n", m_channel);
	}

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d Running voice\n", m_channel);
	/* Octave register enabled? */
	if (!BIT(m_octave, 3))
		cfreq /= (double)(2 << ((7 ^ m_octave) & 7));

	cfreq /= 16.0;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d running voice: Final freq: %f\n", m_channel, cfreq);

	if (cfreq > 100000.0)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d Voice run has excessive final frequency\n", m_channel);
	}

	m_sample_timer->adjust(attotime::from_hz(cfreq), 0, attotime::from_hz(cfreq));

	/*if (m_pitch == 0x035c && m_channel == DEBUG_CHANNEL)
	{
		char log_name[256];
		sprintf(log_name, "cmi_note%d.log", m_pitch_index);
		printf("Writing %s\n", log_name);
		m_pitch_log = fopen(log_name, "w");
		m_pitch_crossing = 1;
	}
	if (m_pitch_log != nullptr)
	{
		fprintf(m_pitch_log, "Note, Initial Settings:\n");
	}
	dump_state();*/
}

void cmi01a_device::run_w(int state)
{
	bool old_run = m_run;
	m_run = state;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA0 CB2 (RUN): %d, LOAD is %d\n", m_channel, m_run, !m_not_load);

	m_stream->update();

	/* RUN */
	if (!old_run && m_run)
	{
		if (m_channel == DEBUG_CHANNEL) dump_state();
		pulse_rstb();
		run_voice();

		m_ptm->set_g1(0);
		m_ptm->set_g2(0);
		m_ptm->set_g3(0);
	}

	if (old_run && !m_run)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d stopping voice because RUN was set to 0\n", m_channel, (m_wave_addr_msb << 7) | m_wave_addr_lsb);
		m_sample_timer->adjust(attotime::never);
		m_current_sample = 0;
		m_current_sample_addr = 0x4000;

		pulse_rstb();

		m_ptm->set_g1(1);
		m_ptm->set_g2(1);
		m_ptm->set_g3(1);

		set_zx_flipflop_state(false);

		if (m_pitch_log != nullptr)
		{
			fclose(m_pitch_log);
			m_pitch_log = nullptr;
			m_pitch_index++;
		}
	}
}

void cmi01a_device::pulse_rstb()
{
	set_not_rstb(false);
	m_rstb_pulse_timer->adjust(attotime::from_nsec(47000));
	m_new_addr = true;
}

TIMER_CALLBACK_MEMBER(cmi01a_device::rstb_pulse_cb)
{
	set_not_rstb(true);
}

void cmi01a_device::set_not_rstb(const bool not_rstb)
{
	if (not_rstb == m_not_rstb)
		return;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d setting !RSTB to %d\n", m_channel, not_rstb);
	m_not_rstb = not_rstb;
	update_gzx();
	if (!m_not_rstb)
	{
		set_wave_addr_lsb(0);
		set_wave_addr_msb(0x80 | m_ws);
	}
}

void cmi01a_device::update_bcas_q1_enable()
{
	const bool old_enable = m_bcas_q1_enabled;
	m_bcas_q1_enabled = (m_zx_ff == m_ptm_o1);

	if (!old_enable && m_bcas_q1_enabled)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d enabling BCAS Q1 timer, m_zx_ff and O1 are %d\n", m_channel, m_ptm_o1);
		m_bcas_q1_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));
	}
	else if (old_enable && !m_bcas_q1_enabled)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d disabling BCAS Q1 timer, m_zx_ff and O1 are %d\n", m_channel, m_ptm_o1);
		m_bcas_q1_timer->adjust(attotime::never);
	}
}

TIMER_CALLBACK_MEMBER(cmi01a_device::bcas_q1_tick)
{
	const bool old_q1 = m_bcas_q1;
	m_bcas_q1 = !m_bcas_q1;
	m_ptm->set_c2(m_bcas_q1);
	m_ptm->set_c3(m_bcas_q1);
	if (old_q1 && !m_bcas_q1)
	{
		//if (m_channel == DEBUG_CHANNEL) LOG("CH%d updating PTM C1\n", m_channel);
		m_bcas_q2 = !m_bcas_q2;
		update_ptm_c1();
	}
}

void cmi01a_device::set_zx_flipflop_clock(const bool zx_ff_clk)
{
	if (zx_ff_clk == m_zx_ff_clk || !m_run)
		return;

	m_zx_ff_clk = zx_ff_clk;

	if (m_zx_ff_clk)
		set_zx_flipflop_state(m_ptm_o1);
}

void cmi01a_device::set_zx_flipflop_state(const bool zx_ff)
{
	if (zx_ff == m_zx_ff)
		return;

	m_zx_ff = zx_ff;

	update_bcas_q1_enable();
	pulse_zcint();
}

void cmi01a_device::pulse_zcint()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d pulsing zcint\n", m_channel);
	set_not_zcint(false);
	m_zcint_pulse_timer->adjust(attotime::from_nsec(4700));
}

TIMER_CALLBACK_MEMBER(cmi01a_device::zcint_pulse_cb)
{
	set_not_zcint(true);
}

void cmi01a_device::set_not_zcint(const bool not_zcint)
{
	if (not_zcint == m_not_zcint)
		return;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d Setting /ZCINT to %d\n", m_channel, not_zcint);
	m_not_zcint = not_zcint;
	m_pia[0]->ca1_w(not_zcint);
	update_gzx();
}

void cmi01a_device::set_not_load(const bool not_load)
{
	if (not_load == m_not_load)
		return;

	m_not_load = not_load;
	pulse_rstb();
	update_ptm_c1();
}

void cmi01a_device::update_gzx()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d updating GZX to !%d || !%d (%d)\n", m_channel, m_not_rstb, m_not_zcint, !m_not_rstb || !m_not_zcint);
	set_gzx(!m_not_rstb || !m_not_zcint);
}

void cmi01a_device::set_gzx(const bool gzx)
{
	if (gzx == m_gzx)
		return;

	m_gzx = gzx;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d setting GZX to %d\n", m_channel, m_gzx);
	update_upper_wave_addr_load();
	update_not_eload();
	if (m_gzx)
		set_envelope_dir(m_dir);
}

void cmi01a_device::update_not_eload()
{
	set_not_eload(!(m_permit_eload && m_gzx));
}

void cmi01a_device::set_not_eload(const bool not_eload)
{
	if (not_eload == m_not_eload)
		return;

	m_not_eload = not_eload;
	try_load_envelope();
}

void cmi01a_device::try_load_envelope()
{
	if (m_not_eload)
		return;

	set_envelope(m_rp);
}

void cmi01a_device::set_envelope(const u8 env)
{
	if (env == m_env)
		return;

	m_env = env;
	update_envelope_divider();
	update_envelope_tri();
}

void cmi01a_device::update_envelope_divider()
{
	if (m_env_dir == ENV_DIR_UP)
		m_env_divider = (~m_env >> 2) & 0x3c;
	else
		m_env_divider = (m_env >> 2) & 0x3c;
	m_env_divider |= 0x03;
}

void cmi01a_device::set_envelope_dir(const int env_dir)
{
	if (env_dir == m_env_dir)
		return;

	m_env_dir = env_dir;
	update_envelope_divider();
	update_envelope_tri();
}

void cmi01a_device::update_envelope_clock()
{
	const bool old_eclk = m_eclk;
	m_eclk = (m_ptm_o2 && m_zx_ff) || (m_ptm_o3 && !m_zx_ff);

	if (old_eclk == m_eclk)
		return;

	tick_ediv();

	const bool old_env_clk = m_env_clk;
	m_env_clk = ((m_not_load && m_eclk) || (!m_not_load && m_ediv_out));

	if (!old_env_clk && m_env_clk)
	{
		//if (m_channel == DEBUG_CHANNEL) LOG("CH%d Clocking envelope: (!%d && %d) || (%d && %d)\n", m_channel, !m_not_load, m_eclk, !m_not_load, m_ediv_out);
		clock_envelope();
	}
}

void cmi01a_device::update_envelope_tri()
{
	if (m_env_dir == ENV_DIR_DOWN)
		m_tri = (m_env == 0x00);
	else
		m_tri = (m_env == 0xff);

	m_pia[0]->cb1_w(m_tri);
}

void cmi01a_device::set_not_wpe(const bool not_wpe)
{
	if (not_wpe == m_not_wpe)
		return;

	m_not_wpe = not_wpe;
	update_upper_wave_addr_load();
}

void cmi01a_device::update_upper_wave_addr_load()
{
	const bool c10_and_out = (!m_not_wpe && m_gzx);
	set_upper_wave_addr_load(c10_and_out || !m_not_rstb);
}

void cmi01a_device::set_upper_wave_addr_load(const bool upper_wave_addr_load)
{
	if (upper_wave_addr_load == m_upper_wave_addr_load)
		return;

	m_upper_wave_addr_load = upper_wave_addr_load;
	try_load_upper_wave_addr();
}

void cmi01a_device::try_load_upper_wave_addr()
{
	if (!m_upper_wave_addr_load)
		return;

	set_wave_addr_msb(0x80 | m_ws);
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d loading upper wave addr, now %04x\n", m_channel, (m_wave_addr_msb << 7) | m_wave_addr_lsb);
}

void cmi01a_device::set_wave_addr_lsb(const u8 wave_addr_lsb)
{
	if (wave_addr_lsb == m_wave_addr_lsb)
		return;

	m_wave_addr_lsb = wave_addr_lsb;
	set_zx(BIT(m_wave_addr_lsb, 6));
}

void cmi01a_device::set_wave_addr_msb(const u8 wave_addr_msb)
{
	if (wave_addr_msb == m_wave_addr_msb)
		return;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d setting wave addr MSB to %02x\n", m_channel, m_wave_addr_msb);
	m_wave_addr_msb = wave_addr_msb;
	m_pia[1]->cb1_w(BIT(m_wave_addr_msb, 7));
}

void cmi01a_device::set_wave_addr_msb_clock(const bool wave_addr_msb_clock)
{
	if (wave_addr_msb_clock == m_wave_addr_msb_clock)
		return;

	m_wave_addr_msb_clock = wave_addr_msb_clock;
	if (m_wave_addr_msb_clock)
	{
		set_wave_addr_msb(m_wave_addr_msb + 1);
	}
}

void cmi01a_device::set_zx(const bool zx)
{
	if (zx == m_zx)
		return;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d setting ZX to %d (%04x)\n", m_channel, zx, (m_wave_addr_msb << 7) | m_wave_addr_lsb);
	m_zx = zx;
	set_wave_addr_msb_clock(!(!m_not_load && m_zx));
	m_pia[1]->ca1_w(m_zx);
	set_zx_flipflop_clock(!m_zx);
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d Updating PTM clock (%d), (!LOAD(%d) && Q2(%d)) || (LOAD(%d) && !ZX(%d))\n", m_channel, (m_not_load && m_bcas_q2) || (!m_not_load && !m_zx), m_not_load, m_bcas_q2, !m_not_load, m_zx);
	update_ptm_c1();
}

void cmi01a_device::update_ptm_c1()
{
	const bool ptm_c1 = (m_not_load && m_bcas_q2) || (!m_not_load && !m_zx);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d C1 to %d\n", m_channel, ptm_c1);
	m_ptm->set_c1(ptm_c1);
}

void cmi01a_device::ptm_o1(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PTM O1: %d\n", m_channel, state);
	m_ptm_o1 = state;
	update_bcas_q1_enable();
}

void cmi01a_device::ptm_o2(int state)
{
	m_ptm_o2 = state;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PTM O2: %d\n", m_channel, m_ptm_o2);
	update_envelope_clock();
}

void cmi01a_device::ptm_o3(int state)
{
	m_ptm_o3 = state;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PTM O3: %d\n", m_channel, m_ptm_o3);
	update_envelope_clock();
}

int cmi01a_device::eosi_r()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CB1 Read (/EOSI): %d (%04x)\n", m_channel, BIT(m_wave_addr_msb, 7), (m_wave_addr_msb << 7) | m_wave_addr_lsb);
	return BIT(m_wave_addr_msb, 7);
}

int cmi01a_device::zx_r()
{
	return BIT(m_wave_addr_lsb, 6);
}

void cmi01a_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x0:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d Porthole Write to %04x: %02x\n", machine().describe_context(), m_channel, ((m_wave_addr_msb << 7) | m_wave_addr_lsb) & 0x3fff, data);
			if (m_new_addr)
				m_new_addr = false;

			m_wave_ram[((m_wave_addr_msb << 7) | m_wave_addr_lsb) & 0x3fff] = data;
			set_wave_addr_lsb((m_wave_addr_lsb + 1) & 0x7f);
			break;

		case 0x3:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Down (%02x)\n", machine().describe_context(), m_channel, data);
			set_envelope_dir(ENV_DIR_DOWN);
			break;

		case 0x4:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Up (%02x)\n", machine().describe_context(), m_channel, data);
			set_envelope_dir(ENV_DIR_UP);
			break;

		case 0x5:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Volume Latch: %02x\n", machine().describe_context(), m_channel, data);
			m_vol_latch = data;
			break;

		case 0x6:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Filter Latch: %02x\n", machine().describe_context(), m_channel, data);
			m_flt_latch = data;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PIA0 Write: %d = %02x\n", machine().describe_context(), m_channel, offset & 3, data);
			m_pia[0]->write(offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PIA1 Write: %d = %02x\n", machine().describe_context(), m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			m_pia[1]->write((BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			/* PTM addressing is a little funky */
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PTM Write: %d = %02x\n", machine().describe_context(), m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			m_ptm->write((a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: Unknown channel card write to E0%02X = %02X\n", machine().describe_context(), offset, data);
			break;
	}
}

u8 cmi01a_device::read(offs_t offset)
{
	if (machine().side_effects_disabled())
		return 0;

	u8 data = 0;

	switch (offset)
	{
		case 0x0:
			data = m_wave_ram[((m_wave_addr_msb << 7) | m_wave_addr_lsb) & 0x3fff];
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d Porthole Read from %04x: %02x\n", machine().describe_context(), m_channel, ((m_wave_addr_msb << 7) | m_wave_addr_lsb) & 0x3fff, data);
			if (!m_new_addr)
			{
				set_wave_addr_lsb((m_wave_addr_lsb + 1) & 0x7f);
			}
			m_new_addr = false;
			break;

		case 0x3:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Down (R)\n", machine().describe_context(), m_channel);
			set_envelope_dir(ENV_DIR_DOWN);
			break;

		case 0x4:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Up (R)\n", machine().describe_context(), m_channel);
			set_envelope_dir(ENV_DIR_UP);
			break;

		case 0x5:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d read Volume Latch (ff)\n", machine().describe_context(), m_channel);
			data = 0xff;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			data = m_pia[0]->read(offset & 3);
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PIA0 Read: %d = %02x\n", machine().describe_context(), m_channel, offset & 3, data);
			break;

		case 0xc: case 0xd: case 0xe: case 0xf:
			data = m_pia[1]->read((BIT(offset, 0) << 1) | BIT(offset, 1));
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PIA1 Read: %d = %02x\n", machine().describe_context(), m_channel, (BIT(offset, 0) << 1) | BIT(offset, 1), data);
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		{
			int a0 = offset & 1;
			int a1 = (m_ptm_o1 && BIT(offset, 3)) || (!BIT(offset, 3) && BIT(offset, 2));
			int a2 = BIT(offset, 1);

			data = m_ptm->read((a2 << 2) | (a1 << 1) | a0);

			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d PTM Read: %d = %02x\n", machine().describe_context(), m_channel, (a2 << 2) | (a1 << 1) | a0, data);
			break;
		}

		default:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: Unknown channel card %d read from E0%02X\n", machine().describe_context(), m_channel, offset);
			break;
	}

	if (m_channel == DEBUG_CHANNEL) LOG("%s: channel card %d read: %02x = %02x\n", machine().describe_context(), m_channel, offset, data);

	return data;
}

void cmi01a_device::dump_state()
{
	if (m_channel != DEBUG_CHANNEL)
		return;

	LOG("Here's the current state of CH%d's things\n", m_channel);
	LOG("    m_zx_ff_clk: %d\n", m_zx_ff_clk);
	LOG("    m_zx_ff: %d\n", m_zx_ff);
	LOG("    m_not_rstb: %d\n", m_not_rstb);
	LOG("    m_upper_wave_addr_load: %d\n", m_upper_wave_addr_load);
	LOG("    m_zx: %d\n", m_zx);
	LOG("    m_not_eload: %d\n", m_not_eload);
	LOG("    m_not_load: %d\n", m_not_load);
	LOG("    m_bcas_q1_enabled: %d\n", m_bcas_q1_enabled);
	LOG("    m_eclk: %d\n", m_eclk);
	LOG("    m_env_clk: %d\n", m_env_clk);
	LOG("    m_env_divider: %02x\n", m_env_divider);
	LOG("    m_wave_addr_msb: %04x\n", m_wave_addr_msb);
	LOG("    m_wave_addr_lsb: %04x\n", m_wave_addr_lsb);
	LOG("    m_new_addr: %d\n", m_new_addr);
	LOG("    m_vol_latch: %02x\n", m_vol_latch);
	LOG("    m_flt_latch: %02x\n", m_flt_latch);
	LOG("    m_rp: %02x\n", m_rp);
	LOG("    m_ws: %02x\n", m_ws);
	LOG("    m_dir: %d\n", m_dir);
	LOG("    m_env_dir: %d\n", m_env_dir);
	LOG("    m_env: %02x\n", m_env);
	LOG("    m_run: %d\n", m_run);
	LOG("    m_not_zcint: %d\n", m_not_zcint);
	LOG("    m_gzx: %d\n", m_gzx);
	LOG("    m_not_wpe: %d\n", m_not_wpe);
	LOG("    m_bcas_q2: %d\n", m_bcas_q2);
	LOG("    m_bcas_q1: %d\n", m_bcas_q1);
	LOG("    m_ptm_o1: %d\n", m_ptm_o1);
	LOG("    m_ptm_o2: %d\n", m_ptm_o2);
	LOG("    m_ptm_o3: %d\n", m_ptm_o3);
	LOG("    m_tri: %d\n", m_tri);
	LOG("    m_permit_eload: %d\n", m_permit_eload);
	LOG("    m_ediv_out: %d\n", m_ediv_out);
	LOG("    m_pitch: %04x\n", m_pitch);
	LOG("    m_octave: %02x\n", m_octave);
	LOG("PIA 0:\n");
	m_pia[0]->dump_state();
	LOG("PIA 1:\n");
	m_pia[1]->dump_state();
	LOG("PTM:\n");
	m_ptm->dump_state();
	LOG("Input Merger:\n");
	m_irq_merger->dump_state();

	if (m_pitch_log != nullptr)
	{
		fprintf(m_pitch_log, "Here's the current state of CH%d's things\n", m_channel);
		fprintf(m_pitch_log, "    m_zx_ff_clk: %d\n", m_zx_ff_clk);
		fprintf(m_pitch_log, "    m_zx_ff: %d\n", m_zx_ff);
		fprintf(m_pitch_log, "    m_not_rstb: %d\n", m_not_rstb);
		fprintf(m_pitch_log, "    m_upper_wave_addr_load: %d\n", m_upper_wave_addr_load);
		fprintf(m_pitch_log, "    m_zx: %d\n", m_zx);
		fprintf(m_pitch_log, "    m_not_eload: %d\n", m_not_eload);
		fprintf(m_pitch_log, "    m_not_load: %d\n", m_not_load);
		fprintf(m_pitch_log, "    m_bcas_q1_enabled: %d\n", m_bcas_q1_enabled);
		fprintf(m_pitch_log, "    m_eclk: %d\n", m_eclk);
		fprintf(m_pitch_log, "    m_env_clk: %d\n", m_env_clk);
		fprintf(m_pitch_log, "    m_env_divider: %02x\n", m_env_divider);
		fprintf(m_pitch_log, "    m_wave_addr_msb: %04x\n", m_wave_addr_msb);
		fprintf(m_pitch_log, "    m_wave_addr_lsb: %04x\n", m_wave_addr_lsb);
		fprintf(m_pitch_log, "    m_new_addr: %d\n", m_new_addr);
		fprintf(m_pitch_log, "    m_vol_latch: %02x\n", m_vol_latch);
		fprintf(m_pitch_log, "    m_flt_latch: %02x\n", m_flt_latch);
		fprintf(m_pitch_log, "    m_rp: %02x\n", m_rp);
		fprintf(m_pitch_log, "    m_ws: %02x\n", m_ws);
		fprintf(m_pitch_log, "    m_dir: %d\n", m_dir);
		fprintf(m_pitch_log, "    m_env_dir: %d\n", m_env_dir);
		fprintf(m_pitch_log, "    m_env: %02x\n", m_env);
		fprintf(m_pitch_log, "    m_run: %d\n", m_run);
		fprintf(m_pitch_log, "    m_not_zcint: %d\n", m_not_zcint);
		fprintf(m_pitch_log, "    m_gzx: %d\n", m_gzx);
		fprintf(m_pitch_log, "    m_not_wpe: %d\n", m_not_wpe);
		fprintf(m_pitch_log, "    m_bcas_q2: %d\n", m_bcas_q2);
		fprintf(m_pitch_log, "    m_bcas_q1: %d\n", m_bcas_q1);
		fprintf(m_pitch_log, "    m_ptm_o1: %d\n", m_ptm_o1);
		fprintf(m_pitch_log, "    m_ptm_o2: %d\n", m_ptm_o2);
		fprintf(m_pitch_log, "    m_ptm_o3: %d\n", m_ptm_o3);
		fprintf(m_pitch_log, "    m_tri: %d\n", m_tri);
		fprintf(m_pitch_log, "    m_permit_eload: %d\n", m_permit_eload);
		fprintf(m_pitch_log, "    m_ediv_out: %d\n", m_ediv_out);
		fprintf(m_pitch_log, "    m_pitch: %04x\n", m_pitch);
		fprintf(m_pitch_log, "    m_octave: %02x\n", m_octave);
		fprintf(m_pitch_log, "PIA 0:\n");
		m_pia[0]->dump_state(m_pitch_log);
		fprintf(m_pitch_log, "PIA 1:\n");
		m_pia[1]->dump_state(m_pitch_log);
		fprintf(m_pitch_log, "PTM:\n");
		m_ptm->dump_state(m_pitch_log);
		fprintf(m_pitch_log, "Input Merger:\n");
		m_irq_merger->dump_state(m_pitch_log);
	}
}
