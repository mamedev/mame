// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/

#include "emu.h"
#include "n8080.h"
#include "speaker.h"


constexpr double ATTACK_RATE = 10e-6 * 500;
constexpr double DECAY_RATE = 10e-6 * 16000;


void spacefev_state::update_SN76477_status()
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


void sheriff_state::update_SN76477_status()
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


void spacefev_state::sound_pins_changed()
{
	uint16_t changes = ~m_curr_sound_pins & m_prev_sound_pins;

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

	bool irq_active = (~m_curr_sound_pins & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5))) != 0;
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}


void sheriff_state::sound_pins_changed()
{
	uint16_t changes = ~m_curr_sound_pins & m_prev_sound_pins;

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

	bool irq_active = (~m_curr_sound_pins & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5))) != 0;
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}


void helifire_state::sound_pins_changed()
{
	//uint16_t changes = ~m_curr_sound_pins & m_prev_sound_pins;

	/* ((m_curr_sound_pins >> 0xa) & 1) not emulated */
	/* ((m_curr_sound_pins >> 0xb) & 1) not emulated */
	/* ((m_curr_sound_pins >> 0xc) & 1) not emulated */

	bool irq_active = (~m_curr_sound_pins & (1 << 6)) != 0;
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}


void n8080_state::sound_pins_changed()
{
}


void n8080_state::delayed_sound_1(int data)
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

	sound_pins_changed();

	m_prev_sound_pins = m_curr_sound_pins;
	m_prev_snd_data = data;
}

void spacefev_state::delayed_sound_1(int data)
{
	if (data & ~m_prev_snd_data & 0x10)
		start_red_cannon();

	m_red_screen = data & 0x08;

	n8080_state::delayed_sound_1(data);
}


TIMER_CALLBACK_MEMBER( n8080_state::delayed_sound_1_callback )
{
	delayed_sound_1(param);
}


void n8080_state::delayed_sound_2(int data)
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

	sound_pins_changed();

	m_prev_sound_pins = m_curr_sound_pins;
}

void spacefev_state::delayed_sound_2(int data)
{
	flip_screen_set(data & 0x20);

	n8080_state::delayed_sound_2(data);
}

void helifire_state::delayed_sound_2(int data)
{
	m_flash = data & 0x20;

	n8080_state::delayed_sound_2(data);
}


TIMER_CALLBACK_MEMBER( n8080_state::delayed_sound_2_callback )
{
	delayed_sound_2(param);
}


void n8080_state::n8080_sound_1_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_1_callback), this), data); /* force CPUs to sync */
}

void n8080_state::n8080_sound_2_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(n8080_state::delayed_sound_2_callback), this), data); /* force CPUs to sync */
}


uint8_t n8080_state::n8080_8035_p1_r()
{
	uint8_t val = 0;

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


READ_LINE_MEMBER(n8080_state::n8080_8035_t0_r)
{
	return (m_curr_sound_pins >> 0x7) & 1;
}
READ_LINE_MEMBER(n8080_state::n8080_8035_t1_r)
{
	return (m_curr_sound_pins >> 0xc) & 1;
}


READ_LINE_MEMBER(helifire_state::helifire_8035_t0_r)
{
	return (m_curr_sound_pins >> 0x3) & 1;
}
READ_LINE_MEMBER(helifire_state::helifire_8035_t1_r)
{
	return (m_curr_sound_pins >> 0x4) & 1;
}


uint8_t helifire_state::helifire_8035_external_ram_r()
{
	uint8_t val = 0;

	if ((m_curr_sound_pins >> 0x7) & 1) val |= 0x01;
	if ((m_curr_sound_pins >> 0x8) & 1) val |= 0x02;
	if ((m_curr_sound_pins >> 0x9) & 1) val |= 0x04;
	if ((m_curr_sound_pins >> 0x1) & 1) val |= 0x08;

	return val;
}


uint8_t helifire_state::helifire_8035_p2_r()
{
	return ((m_curr_sound_pins >> 0xc) & 1) ? 0x10 : 0x00; /* not used */
}


void n8080_state::n8080_dac_w(uint8_t data)
{
	m_n8080_dac->write(BIT(data, 7));
}


void helifire_state::sound_ctrl_w(uint8_t data)
{
	m_dac_phase = data & 0x80;

	/* data & 0x40 not emulated */
	/* data & 0x20 not emulated */

	if (m_dac_phase)
	{
		m_dac_timing = ATTACK_RATE * log(1 - m_dac_volume);
	}
	else
	{
		m_dac_timing = DECAY_RATE * log(m_dac_volume);
	}

	m_dac_timing += machine().time().as_double();
}


TIMER_DEVICE_CALLBACK_MEMBER(spacefev_state::vco_voltage_timer)
{
	double voltage = 0;

	if (m_mono_flop[2])
	{
		voltage = 5 * (1 - exp(- m_sound_timer[2]->elapsed().as_double() / 0.22));
	}

	m_sn->vco_voltage_w(voltage);
}


TIMER_DEVICE_CALLBACK_MEMBER(helifire_state::dac_volume_timer)
{
	double t = m_dac_timing - machine().time().as_double();

	if (m_dac_phase)
	{
		m_dac_volume = 1 - exp(t / ATTACK_RATE);
	}
	else
	{
		m_dac_volume = exp(t / DECAY_RATE);
	}

	m_dac->set_output_gain(ALL_OUTPUTS, m_dac_volume);
}


void spacefev_state::sound_start()
{
	m_sound_timer[0] = timer_alloc(FUNC(spacefev_state::stop_mono_flop_callback), this);
	m_sound_timer[1] = timer_alloc(FUNC(spacefev_state::stop_mono_flop_callback), this);
	m_sound_timer[2] = timer_alloc(FUNC(spacefev_state::stop_mono_flop_callback), this);

	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_mono_flop));
}

void spacefev_state::sound_reset()
{
	m_mono_flop[0] = 0;
	m_mono_flop[1] = 0;
	m_mono_flop[2] = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


void sheriff_state::sound_start()
{
	m_sound_timer[0] = timer_alloc(FUNC(sheriff_state::stop_mono_flop_callback), this);
	m_sound_timer[1] = timer_alloc(FUNC(sheriff_state::stop_mono_flop_callback), this);

	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_mono_flop));
}

void sheriff_state::sound_reset()
{
	m_mono_flop[0] = 0;
	m_mono_flop[1] = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


void helifire_state::sound_start()
{
	save_item(NAME(m_prev_snd_data));
	save_item(NAME(m_prev_sound_pins));
	save_item(NAME(m_curr_sound_pins));
	save_item(NAME(m_dac_volume));
	save_item(NAME(m_dac_timing));
	save_item(NAME(m_dac_phase));
}

void helifire_state::sound_reset()
{
	m_dac_volume = 1;
	m_dac_timing = 0;
	m_dac_phase = 0;
	m_prev_snd_data = 0;
	m_prev_sound_pins = 0;
	m_curr_sound_pins = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


void n8080_state::n8080_sound_cpu_map(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x03ff).rom();
}

void helifire_state::sound_io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0x7f).r(FUNC(helifire_state::helifire_8035_external_ram_r));
}

void spacefev_state::spacefev_sound(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_audiocpu, 6_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &spacefev_state::n8080_sound_cpu_map);
	m_audiocpu->t0_in_cb().set(FUNC(spacefev_state::n8080_8035_t0_r));
	m_audiocpu->t1_in_cb().set(FUNC(spacefev_state::n8080_8035_t1_r));
	m_audiocpu->p1_in_cb().set(FUNC(spacefev_state::n8080_8035_p1_r));
	m_audiocpu->p2_out_cb().set(FUNC(spacefev_state::n8080_dac_w));

	TIMER(config, "vco_timer").configure_periodic(FUNC(spacefev_state::vco_voltage_timer), attotime::from_hz(1000));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	DAC_1BIT(config, m_n8080_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15);

	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(36), RES_K(150), CAP_N(1));
	m_sn->set_decay_res(RES_M(1));
	m_sn->set_attack_params(CAP_U(1.0), RES_K(20));
	m_sn->set_amp_res(RES_K(150));
	m_sn->set_feedback_res(RES_K(47));
	m_sn->set_vco_params(0, CAP_N(1), RES_M(1.5));
	m_sn->set_pitch_voltage(0);
	m_sn->set_slf_params(CAP_N(47), RES_M(1));
	m_sn->set_oneshot_params(CAP_N(47), RES_K(820));
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "speaker", 0.35);
}

void sheriff_state::sheriff_sound(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_audiocpu, 6_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sheriff_state::n8080_sound_cpu_map);
	m_audiocpu->t0_in_cb().set(FUNC(sheriff_state::n8080_8035_t0_r));
	m_audiocpu->t1_in_cb().set(FUNC(sheriff_state::n8080_8035_t1_r));
	m_audiocpu->p1_in_cb().set(FUNC(sheriff_state::n8080_8035_p1_r));
	m_audiocpu->p2_out_cb().set(FUNC(sheriff_state::n8080_dac_w));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	DAC_1BIT(config, m_n8080_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15);

	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(36), RES_K(100), CAP_N(1));
	m_sn->set_decay_res(RES_K(620));
	m_sn->set_attack_params(CAP_U(1.0), RES_K(20));
	m_sn->set_amp_res(RES_K(150));
	m_sn->set_feedback_res(RES_K(47));
	m_sn->set_vco_params(0, CAP_N(1), RES_M(1.5));
	m_sn->set_pitch_voltage(0);
	m_sn->set_slf_params(CAP_N(47), RES_M(1.5));
	m_sn->set_oneshot_params(CAP_N(47), RES_K(560));
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "speaker", 0.35);
}

void helifire_state::helifire_sound(machine_config &config)
{
	/* basic machine hardware */
	I8035(config, m_audiocpu, 6_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &helifire_state::n8080_sound_cpu_map);
	m_audiocpu->set_addrmap(AS_IO, &helifire_state::sound_io_map);
	m_audiocpu->t0_in_cb().set(FUNC(helifire_state::helifire_8035_t0_r));
	m_audiocpu->t1_in_cb().set(FUNC(helifire_state::helifire_8035_t1_r));
	m_audiocpu->p2_in_cb().set(FUNC(helifire_state::helifire_8035_p2_r));
	m_audiocpu->p1_out_cb().set("helifire_dac", FUNC(dac_byte_interface::data_w));
	m_audiocpu->p2_out_cb().set(FUNC(helifire_state::sound_ctrl_w));

	TIMER(config, "helifire_dac_volume_timer").configure_periodic(FUNC(helifire_state::dac_volume_timer), attotime::from_hz(1000));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // unknown DAC
}
