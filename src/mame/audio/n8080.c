// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/

#include "emu.h"
#include "includes/n8080.h"

static const double ATTACK_RATE = 10e-6 * 500;
static const double DECAY_RATE = 10e-6 * 16000;


void n8080_state::spacefev_update_SN76477_status()
{
	double dblR0 = RES_M(1.0);
	double dblR1 = RES_M(1.5);

	if (!m_mono_flop[0])
	{
		dblR0 = 1 / (1 / RES_K(150) + 1 / dblR0); /* ? */
	}
	if (!m_mono_flop[1])
	{
		dblR1 = 1 / (1 / RES_K(620) + 1 / dblR1); /* ? */
	}

	m_sn->decay_res_w(dblR0);

	m_sn->vco_res_w(dblR1);

	m_sn->enable_w(
		!m_mono_flop[0] &&
		!m_mono_flop[1] &&
		!m_mono_flop[2]);

	m_sn->vco_w(m_mono_flop[1]);

	m_sn->mixer_b_w(m_mono_flop[0]);
}


void n8080_state::sheriff_update_SN76477_status()
{
	if (m_mono_flop[1])
	{
		m_sn->vco_voltage_w(5);
	}
	else
	{
		m_sn->vco_voltage_w(0);
	}

	m_sn->enable_w(
		!m_mono_flop[0] &&
		!m_mono_flop[1]);

	m_sn->vco_w(m_mono_flop[0]);

	m_sn->mixer_b_w(!m_mono_flop[0]);
}


void n8080_state::update_SN76477_status()
{
	if (m_n8080_hardware == 1)
	{
		spacefev_update_SN76477_status();
	}
	if (m_n8080_hardware == 2)
	{
		sheriff_update_SN76477_status();
	}
}


void n8080_state::start_mono_flop( int n, const attotime &expire )
{
	m_mono_flop[n] = 1;

	update_SN76477_status();

	m_sound_timer[n]->adjust(expire, n);
}


void n8080_state::stop_mono_flop( int n )
{
	m_mono_flop[n] = 0;

	update_SN76477_status();

	m_sound_timer[n]->adjust(attotime::never, n);
}


TIMER_CALLBACK_MEMBER( n8080_state::stop_mono_flop_callback )
{
	stop_mono_flop(param);
}


void n8080_state::spacefev_sound_pins_changed()
{
	UINT16 changes = ~m_curr_sound_pins & m_prev_sound_pins;

	if (changes & (1 << 0x3))
	{
		stop_mono_flop(1);
	}
	if (changes & ((1 << 0x3) | (1 << 0x6)))
	{
		stop_mono_flop(2);
	}
	if (changes & (1 << 0x3))
	{
		start_mono_flop(0, attotime::from_usec(550 * 36 * 100));
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(1, attotime::from_usec(550 * 22 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(2, attotime::from_usec(550 * 22 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		generic_pulse_irq_line(m_audiocpu, 0, 2);
	}
}


void n8080_state::sheriff_sound_pins_changed()
{
	UINT16 changes = ~m_curr_sound_pins & m_prev_sound_pins;

	if (changes & (1 << 0x6))
	{
		stop_mono_flop(1);
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(0, attotime::from_usec(550 * 33 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(1, attotime::from_usec(550 * 33 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		generic_pulse_irq_line(m_audiocpu, 0, 2);
	}
}


void n8080_state::helifire_sound_pins_changed()
{
	UINT16 changes = ~m_curr_sound_pins & m_prev_sound_pins;

	/* ((m_curr_sound_pins >> 0xa) & 1) not emulated */
	/* ((m_curr_sound_pins >> 0xb) & 1) not emulated */
	/* ((m_curr_sound_pins >> 0xc) & 1) not emulated */

	if (changes & (1 << 6))
	{
		generic_pulse_irq_line(m_audiocpu, 0, 2);
	}
}


void n8080_state::sound_pins_changed()
{
	if (m_n8080_hardware == 1)
		spacefev_sound_pins_changed();
	if (m_n8080_hardware == 2)
		sheriff_sound_pins_changed();
	if (m_n8080_hardware == 3)
		helifire_sound_pins_changed();

	m_prev_sound_pins = m_curr_sound_pins;
}


void n8080_state::delayed_sound_1( int data )
{
	m_curr_sound_pins &= ~(
		(1 << 0x7) |
		(1 << 0x5) |
		(1 << 0x6) |
		(1 << 0x3) |
		(1 << 0x4) |
		(1 << 0x1));

	if (~data & 0x01) m_curr_sound_pins |= 1 << 0x7;
	if (~data & 0x02) m_curr_sound_pins |= 1 << 0x5; /* pulse */
	if (~data & 0x04) m_curr_sound_pins |= 1 << 0x6; /* pulse */
	if (~data & 0x08) m_curr_sound_pins |= 1 << 0x3; /* pulse (except in Helifire) */
	if (~data & 0x10) m_curr_sound_pins |= 1 << 0x4; /* pulse (except in Helifire) */
	if (~data & 0x20) m_curr_sound_pins |= 1 << 0x1;

	if (m_n8080_hardware == 1)
	{
		if (data & ~m_prev_snd_data & 0x10)
		{
			spacefev_start_red_cannon();
		}

		m_spacefev_red_screen = data & 0x08;
	}

	sound_pins_changed();

	m_prev_snd_data = data;
}


TIMER_CALLBACK_MEMBER( n8080_state::delayed_sound_1_callback )
{
	delayed_sound_1(param);
}


void n8080_state::delayed_sound_2( int data )
{
	m_curr_sound_pins &= ~(
		(1 << 0x8) |
		(1 << 0x9) |
		(1 << 0xa) |
		(1 << 0xb) |
		(1 << 0x2) |
		(1 << 0xc));

	if (~data & 0x01) m_curr_sound_pins |= 1 << 0x8;
	if (~data & 0x02) m_curr_sound_pins |= 1 << 0x9;
	if (~data & 0x04) m_curr_sound_pins |= 1 << 0xa;
	if (~data & 0x08) m_curr_sound_pins |= 1 << 0xb;
	if (~data & 0x10) m_curr_sound_pins |= 1 << 0x2; /* pulse */
	if (~data & 0x20) m_curr_sound_pins |= 1 << 0xc;

	if (m_n8080_hardware == 1)
		flip_screen_set(data & 0x20);
	if (m_n8080_hardware == 3)
		m_helifire_flash = data & 0x20;

	sound_pins_changed();
}


TIMER_CALLBACK_MEMBER( n8080_state::delayed_sound_2_callback )
{
	delayed_sound_2(param);
}


WRITE8_MEMBER(n8080_state::n8080_sound_1_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_1_callback), this), data); /* force CPUs to sync */
}

WRITE8_MEMBER(n8080_state::n8080_sound_2_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_2_callback), this), data); /* force CPUs to sync */
}


READ8_MEMBER(n8080_state::n8080_8035_p1_r)
{
	UINT8 val = 0;

	if ((m_curr_sound_pins >> 0xb) & 1) val |= 0x01;
	if ((m_curr_sound_pins >> 0xa) & 1) val |= 0x02;
	if ((m_curr_sound_pins >> 0x9) & 1) val |= 0x04;
	if ((m_curr_sound_pins >> 0x8) & 1) val |= 0x08;
	if ((m_curr_sound_pins >> 0x5) & 1) val |= 0x10;
	if ((m_curr_sound_pins >> 0x3) & 1) val |= 0x20;
	if ((m_curr_sound_pins >> 0x2) & 1) val |= 0x40;
	if ((m_curr_sound_pins >> 0x1) & 1) val |= 0x80;

	return val;
}


READ8_MEMBER(n8080_state::n8080_8035_t0_r)
{
	return (m_curr_sound_pins >> 0x7) & 1;
}
READ8_MEMBER(n8080_state::n8080_8035_t1_r)
{
	return (m_curr_sound_pins >> 0xc) & 1;
}


READ8_MEMBER(n8080_state::helifire_8035_t0_r)
{
	return (m_curr_sound_pins >> 0x3) & 1;
}
READ8_MEMBER(n8080_state::helifire_8035_t1_r)
{
	return (m_curr_sound_pins >> 0x4) & 1;
}


READ8_MEMBER(n8080_state::helifire_8035_external_ram_r)
{
	UINT8 val = 0;

	if ((m_curr_sound_pins >> 0x7) & 1) val |= 0x01;
	if ((m_curr_sound_pins >> 0x8) & 1) val |= 0x02;
	if ((m_curr_sound_pins >> 0x9) & 1) val |= 0x04;
	if ((m_curr_sound_pins >> 0x1) & 1) val |= 0x08;

	return val;
}


READ8_MEMBER(n8080_state::helifire_8035_p2_r)
{
	return ((m_curr_sound_pins >> 0xc) & 1) ? 0x10 : 0x00; /* not used */
}


WRITE8_MEMBER(n8080_state::n8080_dac_w)
{
	m_dac->write_unsigned8(data & 0x80);
}


WRITE8_MEMBER(n8080_state::helifire_dac_w)
{
	m_dac->write_unsigned8(data * m_helifire_dac_volume);
}


WRITE8_MEMBER(n8080_state::helifire_sound_ctrl_w)
{
	m_helifire_dac_phase = data & 0x80;

	/* data & 0x40 not emulated */
	/* data & 0x20 not emulated */

	if (m_helifire_dac_phase)
	{
		m_helifire_dac_timing = ATTACK_RATE * log(1 - m_helifire_dac_volume);
	}
	else
	{
		m_helifire_dac_timing = DECAY_RATE * log(m_helifire_dac_volume);
	}

	m_helifire_dac_timing += machine().time().as_double();
}


TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::spacefev_vco_voltage_timer)
{
	double voltage = 0;

	if (m_mono_flop[2])
	{
		voltage = 5 * (1 - exp(- m_sound_timer[2]->elapsed().as_double() / 0.22));
	}

	m_sn->vco_voltage_w(voltage);
}


TIMER_DEVICE_CALLBACK_MEMBER(n8080_state::helifire_dac_volume_timer)
{
	double t = m_helifire_dac_timing - machine().time().as_double();

	if (m_helifire_dac_phase)
	{
		m_helifire_dac_volume = 1 - exp(t / ATTACK_RATE);
	}
	else
	{
		m_helifire_dac_volume = exp(t / DECAY_RATE);
	}
}


SOUND_START_MEMBER(n8080_state,spacefev)
{
	m_sound_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::stop_mono_flop_callback), this));
	m_sound_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::stop_mono_flop_callback), this));
	m_sound_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::stop_mono_flop_callback), this));

	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_n8080_hardware));
	save_item(NAME(m_mono_flop));
}

SOUND_RESET_MEMBER(n8080_state,spacefev)
{
	m_n8080_hardware = 1;

	m_mono_flop[0] = 0;
	m_mono_flop[1] = 0;
	m_mono_flop[2] = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


SOUND_START_MEMBER(n8080_state,sheriff)
{
	m_sound_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::stop_mono_flop_callback), this));
	m_sound_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(n8080_state::stop_mono_flop_callback), this));

	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_n8080_hardware));
	save_item(NAME(m_mono_flop));
}

SOUND_RESET_MEMBER(n8080_state,sheriff)
{
	m_n8080_hardware = 2;

	m_mono_flop[0] = 0;
	m_mono_flop[1] = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


SOUND_START_MEMBER(n8080_state,helifire)
{
	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_n8080_hardware));
	save_item(NAME(m_helifire_dac_volume));
	save_item(NAME(m_helifire_dac_timing));
	save_item(NAME(m_helifire_dac_phase));
}

SOUND_RESET_MEMBER(n8080_state,helifire)
{
	m_n8080_hardware = 3;

	m_helifire_dac_volume = 1;
	m_helifire_dac_timing = 0;
	m_helifire_dac_phase = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


static ADDRESS_MAP_START( n8080_sound_cpu_map, AS_PROGRAM, 8, n8080_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( n8080_sound_io_map, AS_IO, 8, n8080_state )
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(n8080_8035_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(n8080_8035_t1_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(n8080_8035_p1_r)

	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(n8080_dac_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( helifire_sound_io_map, AS_IO, 8, n8080_state )
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(helifire_8035_t0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(helifire_8035_t1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READ(helifire_8035_p2_r)

	AM_RANGE(0x00, 0x7f) AM_READ(helifire_8035_external_ram_r)

	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(helifire_dac_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(helifire_sound_ctrl_w)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( spacefev_sound )

	MCFG_SOUND_START_OVERRIDE(n8080_state,spacefev)
	MCFG_SOUND_RESET_OVERRIDE(n8080_state,spacefev)

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(n8080_sound_io_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("vco_timer", n8080_state, spacefev_vco_voltage_timer, attotime::from_hz(1000))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(36), RES_K(150), CAP_N(1)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_M(1))                    // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1.0), RES_K(20))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                    // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_N(1), RES_M(1.5))    // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(0)                       // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_N(47), RES_M(1))        // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_N(47), RES_K(820))  // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( sheriff_sound )

	MCFG_SOUND_START_OVERRIDE(n8080_state,sheriff)
	MCFG_SOUND_RESET_OVERRIDE(n8080_state,sheriff)

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(n8080_sound_io_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(RES_K(36), RES_K(100), CAP_N(1)) // noise + filter
	MCFG_SN76477_DECAY_RES(RES_K(620))                  // decay_res
	MCFG_SN76477_ATTACK_PARAMS(CAP_U(1.0), RES_K(20))   // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(150))                    // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(47))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_N(1), RES_M(1.5))    // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(0)                       // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_N(47), RES_M(1.5))      // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(CAP_N(47), RES_K(560))  // oneshot caps + res
	MCFG_SN76477_VCO_MODE(0)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(0, 0, 0)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 0)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( helifire_sound )

	MCFG_SOUND_START_OVERRIDE(n8080_state,helifire)
	MCFG_SOUND_RESET_OVERRIDE(n8080_state,helifire)

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(helifire_sound_io_map)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("helifire_dac", n8080_state, helifire_dac_volume_timer, attotime::from_hz(1000))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END
