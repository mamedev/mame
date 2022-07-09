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
	m_pia[0]->cb2_handler().set(FUNC(cmi01a_device::pia_0_cb2_w));
	m_pia[0]->irqa_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set(m_irq_merger, FUNC(input_merger_device::in_w<1>));
	if (m_channel == DEBUG_CHANNEL)
	{
		m_pia[0]->set_log(true, 0);
	}

	PIA6821(config, m_pia[1], 0); // 6821 D6/7/8/9
	m_pia[1]->readca1_handler().set(FUNC(cmi01a_device::zx_r));
	m_pia[1]->readcb1_handler().set(FUNC(cmi01a_device::eosi_r));
	m_pia[1]->readpa_handler().set(FUNC(cmi01a_device::pia_1_a_r));
	m_pia[1]->writepa_handler().set(FUNC(cmi01a_device::pia_1_a_w));
	m_pia[1]->readpb_handler().set(FUNC(cmi01a_device::pia_1_b_r));
	m_pia[1]->writepb_handler().set(FUNC(cmi01a_device::pia_1_b_w));
	m_pia[1]->ca2_handler().set(FUNC(cmi01a_device::eload_w));
	m_pia[1]->cb2_handler().set(FUNC(cmi01a_device::wpe_w));
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
	m_current_sample = m_wave_ram[m_segment_cnt & 0x3fff];
	m_current_sample_addr = m_segment_cnt;
	set_segment_cnt((m_segment_cnt + 1) & 0x7fff);
}

void cmi01a_device::device_start()
{
	m_wave_ram = std::make_unique<u8[]>(0x4000);

	m_bcas_q2_timer = timer_alloc(FUNC(cmi01a_device::bcas_q2_tick), this);
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

	m_segment_cnt = 0;
	m_new_addr = 0;
	m_vol_latch = 0;
	m_flt_latch = 0;
	m_rp = 0;
	m_ws = 0;
	m_dir = 0;
	m_env = 0;
	m_pia0_cb2_state = 1;
	m_bcas_q2 = false;
	m_bcas_q1 = false;
	m_rstb = true;

	m_ptm_o1 = 0;
	m_ptm_o2 = 0;
	m_ptm_o3 = 0;

	m_load = false;
	m_nload = true;
	m_run = false;
	m_gzx = true;
	m_nwpe = true;
	m_wpe = false;
	m_tri = true;
	m_pia1_ca2 = false;

	m_eclk = false;
	m_env_clk = false;
	m_ediv_out = true;
	m_ediv_rate = 3;
	std::fill(std::begin(m_envdiv_toggles), std::end(m_envdiv_toggles), false);

	m_pitch = 0;
	m_octave = 0;

	//m_bcas_q2_timer->adjust(attotime::never);
	m_bcas_q2_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));
	m_zcint_pulse_timer->adjust(attotime::never);
	m_rstb_pulse_timer->adjust(attotime::never);
	m_sample_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(cmi01a_device::zcint_pulse_cb)
{
	m_pia[0]->ca1_w(1);
}

TIMER_CALLBACK_MEMBER(cmi01a_device::rstb_pulse_cb)
{
	m_rstb = true;
}

void cmi01a_device::pulse_zcint()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d pulsing zcint\n", m_channel);
	m_pia[0]->ca1_w(0);
	m_zcint_pulse_timer->adjust(attotime::from_nsec(4700));

	pulse_gzx();
}

void cmi01a_device::pulse_gzx()
{
	if (m_pia1_ca2)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d pulse GZX, resetting envelope, RP is now %02x and direction is now %s\n", m_channel, m_rp, m_dir == ENV_DIR_UP ? "UP" : "DOWN");
		m_env = m_rp;
	}
	else
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d pulse GZX, latching direction, direction is now %s\n", m_channel, m_dir == ENV_DIR_UP ? "UP" : "DOWN");
	}

	m_env_dir = m_dir;
	update_envelope_tri();

	if (m_nwpe)
	{
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d Pulse GZX, /WPE is %d, resetting segment\n", m_channel, m_nwpe);
		reset_waveform_segment();
	}
}

void cmi01a_device::reset_waveform_segment()
{
	set_segment_cnt(0x4000 | (m_ws << 7) | (m_segment_cnt & 0x007f));
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d reset_waveform_segment, m_segment_cnt is now %04x\n", m_channel, m_segment_cnt);
}

int cmi01a_device::notload_r()
{
	return m_nload;
}

void cmi01a_device::notload_w(int state)
{
	const bool old_load = m_load;
	m_load = state ? false : true;
	m_nload = state ? true : false;

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d notload_w: %d\n", m_channel, state);
	if (old_load != m_load)
	{
		check_segment_load();
	}
}

void cmi01a_device::check_segment_load()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d check_segment_load: m_load is %d, m_run is %d\n", m_channel, m_load ? 1 : 0, m_run ? 1 : 0);
	set_segment_cnt(0x4000 | (m_ws << 7));
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d check_segment_load: resetting waveform segment (now %04x)\n", m_channel, m_segment_cnt);

	if (m_pia1_ca2)
	{
		load_envelope();
	}

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d beginning load with m_segment_cnt %04x, nwpe is %d\n", m_channel, m_segment_cnt, m_nwpe);
	m_pia[1]->cb1_w(1);
}

void cmi01a_device::load_envelope()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d loading envelope (new rate %02x, dir is %s)\n", m_channel, m_rp, m_dir ? "DOWN" : "UP");
	m_env = m_rp;
	m_env_dir = m_dir;
	update_envelope_tri();
}

void cmi01a_device::update_envelope_tri()
{
	const bool old_tri = m_tri;
	if (m_env_dir == ENV_DIR_DOWN)
	{
		m_ediv_rate = ((m_env >> 2) & 0x3c) | 0x03;
		m_tri = (m_env == 0x00);
	}
	else
	{
		m_ediv_rate = ((~m_env >> 2) & 0x3c) | 0x03;
		m_tri = (m_env == 0xff);
	}

	if (old_tri != m_tri)
	{
		m_pia[0]->cb1_w(m_tri ? 1 : 0);
	}
}

void cmi01a_device::pia_1_a_w(u8 data)
{
	m_pitch &= 0x0ff;
	m_pitch |= (data & 3) << 8;
	m_octave = (data >> 2) & 0x0f;
}

u8 cmi01a_device::pia_1_a_r()
{
	return ((m_pitch >> 8) & 3) | (m_octave << 2);
}

void cmi01a_device::pia_1_b_w(u8 data)
{
	m_pitch &= 0xf00;
	m_pitch |= data;
}

u8 cmi01a_device::pia_1_b_r()
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

TIMER_CALLBACK_MEMBER(cmi01a_device::bcas_q2_tick)
{
	if (m_zx_ff == m_ptm_o1)
	{
		const bool old_q1 = m_bcas_q1;
		m_bcas_q1 = !m_bcas_q1;
		m_ptm->set_c2(m_bcas_q1);
		m_ptm->set_c3(m_bcas_q1);
		if (old_q1 && !m_bcas_q1)
		{
			m_bcas_q2 = !m_bcas_q2;
			//if (m_channel == DEBUG_CHANNEL) LOG("CH%d: q2 tick, FF:%d vs. O1:%d, %d\n", m_channel, m_zx_ff, m_ptm_o1, m_bcas_q2);
			if (m_nload)
			{
				m_ptm->set_c1(m_bcas_q2);
			}
		}
	}
}

void cmi01a_device::zx_tick()
{
	bool zx_flag = BIT(m_segment_cnt, 6);

	// Update ZX input to PIA 1
	m_pia[1]->ca1_w(zx_flag);

	if (m_channel == DEBUG_CHANNEL) LOG("CH%d: ZX timer callback, toggling ZX flag (now %d)\n", m_channel, zx_flag);

	if (m_pitch_log != nullptr)
	{
		fprintf(m_pitch_log, "Note, Crossing %d:\n", m_pitch_crossing);
		if (m_channel == DEBUG_CHANNEL) LOG("Note, Crossing %d:\n", m_pitch_crossing);
		m_pitch_crossing++;
	}
	dump_state();

	// 74LS74 A12 (1) is clocked by /ZX, so a 1->0 transition of the ZX flag is a positive clock transition
	if (!zx_flag)
	{
		// Pulse /ZCINT if the O1 output of the PTM has changed
		if (m_ptm_o1 != m_zx_ff)
		{
			if (m_channel == DEBUG_CHANNEL) LOG("CH%d: Pulsing ZCInt\n", m_channel);
			m_bcas_q1 = false;
			m_bcas_q2 = false;
			pulse_zcint();
		}

		if (m_channel == DEBUG_CHANNEL) LOG("CH%d: Setting ZX flip-flop to %d (zx_tick) (was %d)\n", m_channel, m_ptm_o1, m_zx_ff);
		m_zx_ff = m_ptm_o1;
	}
}

void cmi01a_device::wpe_w(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CB2 (/WPE): %d\n", m_channel, state);
	m_nwpe = state ? true : false;
	m_wpe = state ? false : true;
}

int cmi01a_device::eload_r()
{
	return m_pia1_ca2;
}

void cmi01a_device::eload_w(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CA2 write (permit ELOAD): %d\n", m_channel, state);
	//int old = m_pia1_ca2;
	m_pia1_ca2 = state;
	/*if (!old && state)
	{
		m_env = m_rp;
		if (m_env_dir == ENV_DIR_DOWN)
		{
			m_ediv_rate = ((m_env >> 2) & 0x3c) | 0x03;
		}
		else
		{
			m_ediv_rate = ((~m_env >> 2) & 0x3c) | 0x03;
		}
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d loading m_env with %02x, m_ediv_rate %02x, current direction is %s\n", m_channel, m_rp, m_ediv_rate, m_env_dir == ENV_DIR_DOWN ? "DOWN" : "UP");
	}*/
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
			if (m_channel == DEBUG_CHANNEL) LOG("CH%d, Clocking envelope down (rate %02x), new m_env: %02x\n", m_channel, m_ediv_rate, m_env);
		}
	}
	else
	{
		if (m_env < 0xff)
		{
			m_env++;
			if (m_channel == DEBUG_CHANNEL) LOG("CH%d, Clocking envelope up (rate %02x), new m_env: %02x\n", m_channel, m_ediv_rate, m_env);
		}
	}
	update_envelope_tri();
}

void cmi01a_device::tick_ediv()
{
	const bool envdiv_enable_a = m_env_clk;
	const bool envdiv_enable_b = m_env_clk && m_envdiv_toggles[0];
	const bool envdiv_enable_c = m_env_clk && m_envdiv_toggles[0] && m_envdiv_toggles[1];
	const bool envdiv_enable_d = m_env_clk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2];
	const bool envdiv_enable_e = m_env_clk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3];
	const bool envdiv_enable_f = m_env_clk && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && m_envdiv_toggles[4];

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

	const bool envdiv_out_f = m_env_clk && BIT(m_ediv_rate, 5) && !m_envdiv_toggles[0];
	const bool envdiv_out_e = m_env_clk && BIT(m_ediv_rate, 4) && m_envdiv_toggles[0] && !m_envdiv_toggles[1];
	const bool envdiv_out_d = m_env_clk && BIT(m_ediv_rate, 3) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && !m_envdiv_toggles[2];
	const bool envdiv_out_c = m_env_clk && BIT(m_ediv_rate, 2) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && !m_envdiv_toggles[3];
	const bool envdiv_out_b = m_env_clk && BIT(m_ediv_rate, 1) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && !m_envdiv_toggles[4];
	const bool envdiv_out_a = m_env_clk && BIT(m_ediv_rate, 0) && m_envdiv_toggles[0] && m_envdiv_toggles[1] && m_envdiv_toggles[2] && m_envdiv_toggles[3] && m_envdiv_toggles[4] && !m_envdiv_toggles[5];

	m_ediv_out = !(envdiv_out_f || envdiv_out_e || envdiv_out_d || envdiv_out_c || envdiv_out_b || envdiv_out_a);

	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d ticking ediv, rate: %02x, toggles %d%d%d%d%d%d, %d\n", m_channel, m_ediv_rate, m_envdiv_toggles[5], m_envdiv_toggles[4], m_envdiv_toggles[3], m_envdiv_toggles[2], m_envdiv_toggles[1], m_envdiv_toggles[0], m_ediv_out ? 1 : 0);
}

void cmi01a_device::update_eclk()
{
	const bool old_eclk = m_eclk;
	m_eclk = (m_ptm_o2 && m_zx_ff) || (m_ptm_o3 && !m_zx_ff);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d: eclk = (%d && %d) || (%d && %d) = %d\n", m_channel, m_ptm_o2, m_zx_ff, m_ptm_o3, m_zx_ff ? 0 : 1, m_eclk ? 1 : 0);

	if (old_eclk == m_eclk)
		return;

	tick_ediv();

	const bool old_env_clk = m_env_clk;
	m_env_clk = ((!m_load && m_eclk) || (m_load && m_ediv_out));
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d %susing divider for envelope\n", m_channel, m_load ? "" : "NOT ");
	//const bool a = !(m_nload && eclk);
	//const bool b = !(m_load && m_ediv_out);

	//m_env_clk = !(a && b);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d checking envelope: A:  !(!load(%d) && eclk(%d)) = %d\n", m_channel, m_nload ? 1 : 0, eclk ? 0 : 1, a);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d checking envelope: B:  !(load(%d) || eout(%d)) = %d\n", m_channel, m_load ? 1 : 0, m_ediv_out ? 1 : 0, b);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d checking envelope: C: !(%d && %d) = %d\n", m_channel, a ? 1 : 0, b ? 1 : 0, m_env_clk ? 1 : 0);
	if (!old_env_clk && m_env_clk)
	{
		//if (m_channel == DEBUG_CHANNEL) LOG("CH%d Clocking envelope: (!%d && %d) || (%d && %d)\n", m_channel, m_load ? 1 : 0, eclk ? 1 : 0, m_load ? 1 : 0, m_ediv_out ? 1 : 0);
		clock_envelope();
	}
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

	m_rstb = false;
	m_rstb_pulse_timer->adjust(attotime::from_nsec(47000));

	/*if (m_channel == DEBUG_CHANNEL)
	{
		char wavename[256];
		sprintf(wavename, "wave_ch%d.bin", m_channel);
		FILE *wavefile = fopen(wavename, "wb");
		for (u16 i = 0; i < 0x4000; i++)
		{
			fwrite(&m_wave_ram[i], 1, 1, wavefile);
		}
		fclose(wavefile);
	}*/

	//attotime sample_frequency = attotime::from_ticks(2010500 / (u32)cfreq, 2010500);
	m_sample_timer->adjust(attotime::from_hz(cfreq), 0, attotime::from_hz(cfreq));
	//m_sample_timer->adjust(sample_frequency, 0, sample_frequency);
	//if (m_channel == DEBUG_CHANNEL) LOG("CH%d running voice: Adjusted freq: %f\n", m_channel, sample_frequency.as_hz());

	// HACK: Reset the envelope clock divider for consistency in debugging.
	m_ediv_out = true;
	std::fill(std::begin(m_envdiv_toggles), std::end(m_envdiv_toggles), false);

	if (m_load)
	{
		int samples = 0x4000 - (m_segment_cnt & 0x3fff);
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d voice is %04x samples long, m_segment_cnt is %04x\n", m_channel, samples, m_segment_cnt);
	}

	if (m_pitch == 0x035c && m_channel == DEBUG_CHANNEL)
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
	dump_state();
}

void cmi01a_device::pia_0_cb2_w(int state)
{
	int old_state = m_pia0_cb2_state;
	m_pia0_cb2_state = state;
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA0 CB2 (RUN): %d, m_load is %d\n", m_channel, state, m_load ? 1 : 0);

	m_stream->update();

	/* RUN */
	if (!old_state && m_pia0_cb2_state)
	{
		m_run = true;

		/* Clear /EOSI */
		m_pia[1]->cb1_w(1);

		check_segment_load();

		/* Clear ZX */
		m_pia[1]->ca1_w(0);

		/* Clear /ZCINT */
		m_pia[0]->ca1_w(1);
		m_zcint_pulse_timer->adjust(attotime::never);

		run_voice();

		m_ptm->set_g1(0);
		m_ptm->set_g2(0);
		m_ptm->set_g3(0);

		m_bcas_q1 = false;
		m_bcas_q2 = false;
		if (m_nload)
		{
			m_bcas_q2_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));
		}
		else
		{
			m_bcas_q2_timer->adjust(attotime::never);
		}
	}

	if (old_state && !m_pia0_cb2_state)
	{
		m_sample_timer->adjust(attotime::never);
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d stopping voice because RUN was set to 0\n", m_channel, m_segment_cnt);
		m_run = false;
		m_current_sample = 0;
		m_current_sample_addr = 0x4000;
		m_env_clk = false;

		/* Set /EOSI */
		m_pia[1]->cb1_w(0);
		m_segment_cnt &= ~0x4000;
		//check_segment_load();

		m_ptm->set_g1(1);
		m_ptm->set_g2(1);
		m_ptm->set_g3(1);

		m_zcint_pulse_timer->adjust(attotime::never);
		if (m_channel == DEBUG_CHANNEL) LOG("CH%d: Setting ZX flip-flop to 0 (run stop)\n", m_channel);
		//set_zx_flipflop(0);
		int old_ff = m_zx_ff;
		m_zx_ff = 0;
		if (old_ff != m_zx_ff)
		{
			//pulse_zcint();
		}

		if (m_pitch_log != nullptr)
		{
			fclose(m_pitch_log);
			m_pitch_log = nullptr;
			m_pitch_index++;
		}
	}
}

void cmi01a_device::update_wave_addr(int inc)
{
	u16 old_cnt = m_segment_cnt;

	if (inc)
		++m_segment_cnt;

	/* Update end of sound interrupt flag */
	m_pia[1]->cb1_w(BIT(m_segment_cnt, 14));

	/* Update zero crossing flag */
	m_pia[1]->ca1_w(BIT(m_segment_cnt, 6));

	/* Clock a latch on a transition */
	if ((old_cnt & 0x40) && !(m_segment_cnt & 0x40))
	{
		m_pia[1]->ca2_w(1);
		m_pia[1]->ca2_w(0);
	}

	/* Zero crossing interrupt is a pulse */
}

void cmi01a_device::set_segment_cnt(u16 segment_cnt)
{
	u16 old_cnt = m_segment_cnt;
	m_segment_cnt = segment_cnt;

	/* Update end of sound interrupt flag */
	m_pia[1]->cb1_w(BIT(m_segment_cnt, 14));

	/* Update zero crossing flag */
	m_pia[1]->ca1_w(BIT(m_segment_cnt, 6));

	/* Clock a latch on a transition */
	if (BIT(old_cnt, 6) != BIT(m_segment_cnt, 6))
	{
		if (m_load)
		{
			m_ptm->set_c1(BIT(~m_segment_cnt, 6));
		}
		zx_tick();
	}
}

void cmi01a_device::ptm_o1(int state)
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PTM O1: %d\n", m_channel, state);
	m_ptm_o1 = state;
	if (m_ptm_o1 != m_zx_ff)
	{
		m_bcas_q1 = false;
		m_bcas_q2 = false;
	}
}

void cmi01a_device::ptm_o2(int state)
{
	m_ptm_o2 = state;
	update_eclk();
}

void cmi01a_device::ptm_o3(int state)
{
	m_ptm_o3 = state;
	update_eclk();
}

int cmi01a_device::eosi_r()
{
	if (m_channel == DEBUG_CHANNEL) LOG("CH%d PIA1 CB1 Read (/EOSI): %d (%04x)\n", m_channel, BIT(m_segment_cnt, 14), m_segment_cnt);
	return BIT(m_segment_cnt, 14);
}

int cmi01a_device::zx_r()
{
	return BIT(m_segment_cnt, 6);
}

void cmi01a_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0x0:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d Porthole Write to %04x: %02x\n", machine().describe_context(), m_channel, m_segment_cnt & 0x3fff, data);
			if (m_new_addr)
				m_new_addr = 0;

			m_wave_ram[m_segment_cnt & 0x3fff] = data;
			update_wave_addr(1);
			break;

		case 0x3:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Down (%02x)\n", machine().describe_context(), m_channel, data);
			m_env_dir = ENV_DIR_DOWN;
			update_envelope_tri();
			break;

		case 0x4:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Up (%02x)\n", machine().describe_context(), m_channel, data);
			m_env_dir = ENV_DIR_UP;
			update_envelope_tri();
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
			if (m_new_addr)
			{
				m_new_addr = 0;
				break;
			}
			data = m_wave_ram[m_segment_cnt & 0x3fff];
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d Porthole Read: %02x\n", machine().describe_context(), m_channel, data);
			update_wave_addr(1);
			break;

		case 0x3:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Down (R)\n", machine().describe_context(), m_channel);
			m_env_dir = ENV_DIR_DOWN;
			update_envelope_tri();
			break;

		case 0x4:
			if (m_channel == DEBUG_CHANNEL) LOG("%s: CH%d set Envelope Dir Up (R)\n", machine().describe_context(), m_channel);
			m_env_dir = ENV_DIR_UP;
			update_envelope_tri();
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
	if (m_channel != DEBUG_CHANNEL || m_pitch != 0x035c)
		return;

	/*LOG("Here's the current state of CH%d's things\n", m_channel);
	LOG("    m_zx_fff: %d\n", m_zx_ff);
	LOG("    m_rstb: %d\n", m_rstb);
	LOG("    m_segment_cnt: %04x\n", m_segment_cnt);
	LOG("    m_new_addr: %d\n", m_new_addr);
	LOG("    m_vol_latch: %02x\n", m_vol_latch);
	LOG("    m_flt_latch: %02x\n", m_flt_latch);
	LOG("    m_rp: %02x\n", m_rp);
	LOG("    m_ws: %02x\n", m_ws);
	LOG("    m_dir: %d\n", m_dir);
	LOG("    m_env_dir: %d\n", m_env_dir);
	LOG("    m_env: %02x\n", m_env);
	LOG("    m_pia0_cb2_state: %d\n", m_pia0_cb2_state);
	LOG("    m_ptm_o1: %d\n", m_ptm_o1);
	LOG("    m_ptm_o2: %d\n", m_ptm_o2);
	LOG("    m_ptm_o3: %d\n", m_ptm_o3);
	LOG("    m_load: %d\n", m_load);
	LOG("    m_nload: %d\n", m_nload);
	LOG("    m_run: %d\n", m_run);
	LOG("    m_gzx: %d\n", m_gzx);
	LOG("    m_wpe: %d\n", m_wpe);
	LOG("    m_nwpe: %d\n", m_nwpe);
	LOG("    m_tri: %d\n", m_tri);
	LOG("    m_pia1_ca2: %d\n", m_pia1_ca2);
	LOG("    m_eclk: %d\n", m_eclk);
	LOG("    m_env_clk: %d\n", m_env_clk);
	LOG("    m_ediv_out: %d\n", m_ediv_out);
	LOG("    m_ediv_rate: %02x\n", m_ediv_rate);
	LOG("    m_pitch: %04x\n", m_pitch);
	LOG("    m_octave: %d\n", m_octave);
	LOG("PIA 0:\n");
	m_pia[0]->dump_state();
	LOG("PIA 1:\n");
	m_pia[1]->dump_state();
	LOG("PTM:\n");
	m_ptm->dump_state();
	LOG("Input Merger:\n");
	m_irq_merger->dump_state();*/

	if (m_pitch_log != nullptr)
	{
		fprintf(m_pitch_log, "Here's the current state of CH%d's things\n", m_channel);
		fprintf(m_pitch_log, "    m_zx_fff: %d\n", m_zx_ff);
		fprintf(m_pitch_log, "    m_rstb: %d\n", m_rstb);
		fprintf(m_pitch_log, "    m_segment_cnt: %04x\n", m_segment_cnt);
		fprintf(m_pitch_log, "    m_new_addr: %d\n", m_new_addr);
		fprintf(m_pitch_log, "    m_vol_latch: %02x\n", m_vol_latch);
		fprintf(m_pitch_log, "    m_flt_latch: %02x\n", m_flt_latch);
		fprintf(m_pitch_log, "    m_rp: %02x\n", m_rp);
		fprintf(m_pitch_log, "    m_ws: %02x\n", m_ws);
		fprintf(m_pitch_log, "    m_dir: %d\n", m_dir);
		fprintf(m_pitch_log, "    m_env_dir: %d\n", m_env_dir);
		fprintf(m_pitch_log, "    m_env: %02x\n", m_env);
		fprintf(m_pitch_log, "    m_pia0_cb2_state: %d\n", m_pia0_cb2_state);
		fprintf(m_pitch_log, "    m_ptm_o1: %d\n", m_ptm_o1);
		fprintf(m_pitch_log, "    m_ptm_o2: %d\n", m_ptm_o2);
		fprintf(m_pitch_log, "    m_ptm_o3: %d\n", m_ptm_o3);
		fprintf(m_pitch_log, "    m_load: %d\n", m_load);
		fprintf(m_pitch_log, "    m_nload: %d\n", m_nload);
		fprintf(m_pitch_log, "    m_run: %d\n", m_run);
		fprintf(m_pitch_log, "    m_gzx: %d\n", m_gzx);
		fprintf(m_pitch_log, "    m_wpe: %d\n", m_wpe);
		fprintf(m_pitch_log, "    m_nwpe: %d\n", m_nwpe);
		fprintf(m_pitch_log, "    m_tri: %d\n", m_tri);
		fprintf(m_pitch_log, "    m_pia1_ca2: %d\n", m_pia1_ca2);
		fprintf(m_pitch_log, "    m_eclk: %d\n", m_eclk);
		fprintf(m_pitch_log, "    m_env_clk: %d\n", m_env_clk);
		fprintf(m_pitch_log, "    m_ediv_out: %d\n", m_ediv_out);
		fprintf(m_pitch_log, "    m_ediv_rate: %02x\n", m_ediv_rate);
		fprintf(m_pitch_log, "    m_pitch: %04x\n", m_pitch);
		fprintf(m_pitch_log, "    m_octave: %d\n", m_octave);
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
