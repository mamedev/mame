/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/sn76477.h"
#include "sound/dac.h"
#include "includes/n8080.h"

static const double ATTACK_RATE = 10e-6 * 500;
static const double DECAY_RATE = 10e-6 * 16000;

static const sn76477_interface sheriff_sn76477_interface =
{
	RES_K(36)  ,  /* 04 */
	RES_K(100) ,  /* 05 */
	CAP_N(1)   ,  /* 06 */
	RES_K(620) ,  /* 07 */
	CAP_U(1)   ,  /* 08 */
	RES_K(20)  ,  /* 10 */
	RES_K(150) ,  /* 11 */
	RES_K(47)  ,  /* 12 */
	0          ,  /* 16 */
	CAP_N(1)   ,  /* 17 */
	RES_M(1.5) ,  /* 18 */
	0          ,  /* 19 */
	RES_M(1.5) ,  /* 20 */
	CAP_N(47)  ,  /* 21 */
	CAP_N(47)  ,  /* 23 */
	RES_K(560) ,  /* 24 */
	0,			  /* 22 vco */
	0,			  /* 26 mixer A */
	0,			  /* 25 mixer B */
	0,			  /* 27 mixer C */
	1,			  /* 1  envelope 1 */
	0,			  /* 28 envelope 2 */
	1			  /* 9  enable */
};


static const sn76477_interface spacefev_sn76477_interface =
{
	RES_K(36)  ,  /* 04 */
	RES_K(150) ,  /* 05 */
	CAP_N(1)   ,  /* 06 */
	RES_M(1)   ,  /* 07 */
	CAP_U(1)   ,  /* 08 */
	RES_K(20)  ,  /* 10 */
	RES_K(150) ,  /* 11 */
	RES_K(47)  ,  /* 12 */
	0          ,  /* 16 */
	CAP_N(1)   ,  /* 17 */
	RES_M(1.5) ,  /* 18 */
	0          ,  /* 19 */
	RES_M(1)   ,  /* 20 */
	CAP_N(47)  ,  /* 21 */
	CAP_N(47)  ,  /* 23 */
	RES_K(820) ,  /* 24 */
	0,			  /* 22 vco */
	0,			  /* 26 mixer A */
	0,			  /* 25 mixer B */
	0,			  /* 27 mixer C */
	1,			  /* 1  envelope 1 */
	0,			  /* 28 envelope 2 */
	1			  /* 9  enable */
};


static void spacefev_update_SN76477_status( device_t *sn )
{
	n8080_state *state = sn->machine().driver_data<n8080_state>();
	double dblR0 = RES_M(1.0);
	double dblR1 = RES_M(1.5);

	if (!state->m_mono_flop[0])
	{
		dblR0 = 1 / (1 / RES_K(150) + 1 / dblR0); /* ? */
	}
	if (!state->m_mono_flop[1])
	{
		dblR1 = 1 / (1 / RES_K(620) + 1 / dblR1); /* ? */
	}

	sn76477_decay_res_w(sn, dblR0);

	sn76477_vco_res_w(sn, dblR1);

	sn76477_enable_w(sn,
		!state->m_mono_flop[0] &&
		!state->m_mono_flop[1] &&
		!state->m_mono_flop[2]);

	sn76477_vco_w(sn, state->m_mono_flop[1]);

	sn76477_mixer_b_w(sn, state->m_mono_flop[0]);
}


static void sheriff_update_SN76477_status( device_t *sn )
{
	n8080_state *state = sn->machine().driver_data<n8080_state>();
	if (state->m_mono_flop[1])
	{
		sn76477_vco_voltage_w(sn, 5);
	}
	else
	{
		sn76477_vco_voltage_w(sn, 0);
	}

	sn76477_enable_w(sn,
		!state->m_mono_flop[0] &&
		!state->m_mono_flop[1]);

	sn76477_vco_w(sn, state->m_mono_flop[0]);

	sn76477_mixer_b_w(sn, !state->m_mono_flop[0]);
}


static void update_SN76477_status( device_t *device )
{
	n8080_state *state = device->machine().driver_data<n8080_state>();
	if (state->m_n8080_hardware == 1)
	{
		spacefev_update_SN76477_status(device);
	}
	if (state->m_n8080_hardware == 2)
	{
		sheriff_update_SN76477_status(device);
	}
}


static void start_mono_flop( device_t *sn, int n, attotime expire )
{
	n8080_state *state = sn->machine().driver_data<n8080_state>();
	state->m_mono_flop[n] = 1;

	update_SN76477_status(sn);

	state->m_sound_timer[n]->adjust(expire, n);
}


static void stop_mono_flop( device_t *sn, int n )
{
	n8080_state *state = sn->machine().driver_data<n8080_state>();
	state->m_mono_flop[n] = 0;

	update_SN76477_status(sn);

	state->m_sound_timer[n]->adjust(attotime::never, n);
}


static TIMER_CALLBACK( stop_mono_flop_callback )
{
	stop_mono_flop(machine.device("snsnd"), param);
}


static void spacefev_sound_pins_changed( running_machine &machine )
{
	device_t *sn = machine.device("snsnd");
	n8080_state *state = machine.driver_data<n8080_state>();
	UINT16 changes = ~state->m_curr_sound_pins & state->m_prev_sound_pins;

	if (changes & (1 << 0x3))
	{
		stop_mono_flop(sn, 1);
	}
	if (changes & ((1 << 0x3) | (1 << 0x6)))
	{
		stop_mono_flop(sn, 2);
	}
	if (changes & (1 << 0x3))
	{
		start_mono_flop(sn, 0, attotime::from_usec(550 * 36 * 100));
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(sn, 1, attotime::from_usec(550 * 22 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(sn, 2, attotime::from_usec(550 * 22 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		generic_pulse_irq_line(machine.device("audiocpu"), 0, 2);
	}
}


static void sheriff_sound_pins_changed( running_machine &machine )
{
	device_t *sn = machine.device("snsnd");
	n8080_state *state = machine.driver_data<n8080_state>();
	UINT16 changes = ~state->m_curr_sound_pins & state->m_prev_sound_pins;

	if (changes & (1 << 0x6))
	{
		stop_mono_flop(sn, 1);
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(sn, 0, attotime::from_usec(550 * 33 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(sn, 1, attotime::from_usec(550 * 33 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		generic_pulse_irq_line(machine.device("audiocpu"), 0, 2);
	}
}


static void helifire_sound_pins_changed( running_machine &machine )
{
	n8080_state *state = machine.driver_data<n8080_state>();
	UINT16 changes = ~state->m_curr_sound_pins & state->m_prev_sound_pins;

	/* ((state->m_curr_sound_pins >> 0xa) & 1) not emulated */
	/* ((state->m_curr_sound_pins >> 0xb) & 1) not emulated */
	/* ((state->m_curr_sound_pins >> 0xc) & 1) not emulated */

	if (changes & (1 << 6))
	{
		generic_pulse_irq_line(machine.device("audiocpu"), 0, 2);
	}
}


static void sound_pins_changed( running_machine &machine )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	if (state->m_n8080_hardware == 1)
		spacefev_sound_pins_changed(machine);
	if (state->m_n8080_hardware == 2)
		sheriff_sound_pins_changed(machine);
	if (state->m_n8080_hardware == 3)
		helifire_sound_pins_changed(machine);

	state->m_prev_sound_pins = state->m_curr_sound_pins;
}


static void delayed_sound_1( running_machine &machine, int data )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_curr_sound_pins &= ~(
		(1 << 0x7) |
		(1 << 0x5) |
		(1 << 0x6) |
		(1 << 0x3) |
		(1 << 0x4) |
		(1 << 0x1));

	if (~data & 0x01) state->m_curr_sound_pins |= 1 << 0x7;
	if (~data & 0x02) state->m_curr_sound_pins |= 1 << 0x5; /* pulse */
	if (~data & 0x04) state->m_curr_sound_pins |= 1 << 0x6; /* pulse */
	if (~data & 0x08) state->m_curr_sound_pins |= 1 << 0x3; /* pulse (except in Helifire) */
	if (~data & 0x10) state->m_curr_sound_pins |= 1 << 0x4; /* pulse (except in Helifire) */
	if (~data & 0x20) state->m_curr_sound_pins |= 1 << 0x1;

	if (state->m_n8080_hardware == 1)
	{
		if (data & ~state->m_prev_snd_data & 0x10)
		{
			spacefev_start_red_cannon(machine);
		}

		state->m_spacefev_red_screen = data & 0x08;
	}

	sound_pins_changed(machine);

	state->m_prev_snd_data = data;
}


static TIMER_CALLBACK( delayed_sound_1_callback )
{
	delayed_sound_1(machine, param);
}


static void delayed_sound_2( running_machine &machine, int data )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_curr_sound_pins &= ~(
		(1 << 0x8) |
		(1 << 0x9) |
		(1 << 0xa) |
		(1 << 0xb) |
		(1 << 0x2) |
		(1 << 0xc));

	if (~data & 0x01) state->m_curr_sound_pins |= 1 << 0x8;
	if (~data & 0x02) state->m_curr_sound_pins |= 1 << 0x9;
	if (~data & 0x04) state->m_curr_sound_pins |= 1 << 0xa;
	if (~data & 0x08) state->m_curr_sound_pins |= 1 << 0xb;
	if (~data & 0x10) state->m_curr_sound_pins |= 1 << 0x2; /* pulse */
	if (~data & 0x20) state->m_curr_sound_pins |= 1 << 0xc;

	if (state->m_n8080_hardware == 1)
		state->flip_screen_set_no_update(data & 0x20);
	if (state->m_n8080_hardware == 3)
		state->m_helifire_flash = data & 0x20;

	sound_pins_changed(machine);
}


static TIMER_CALLBACK( delayed_sound_2_callback )
{
	delayed_sound_2(machine, param);
}


WRITE8_MEMBER(n8080_state::n8080_sound_1_w)
{
	machine().scheduler().synchronize(FUNC(delayed_sound_1_callback), data); /* force CPUs to sync */
}

WRITE8_MEMBER(n8080_state::n8080_sound_2_w)
{
	machine().scheduler().synchronize(FUNC(delayed_sound_2_callback), data); /* force CPUs to sync */
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
	machine().device<dac_device>("dac")->write_unsigned8(data & 0x80);
}


WRITE8_MEMBER(n8080_state::helifire_dac_w)
{
	machine().device<dac_device>("dac")->write_unsigned8(data * m_helifire_dac_volume);
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


static TIMER_DEVICE_CALLBACK( spacefev_vco_voltage_timer )
{
	device_t *sn = timer.machine().device("snsnd");
	n8080_state *state = timer.machine().driver_data<n8080_state>();
	double voltage = 0;

	if (state->m_mono_flop[2])
	{
		voltage = 5 * (1 - exp(- state->m_sound_timer[2]->elapsed().as_double() / 0.22));
	}

	sn76477_vco_voltage_w(sn, voltage);
}


static TIMER_DEVICE_CALLBACK( helifire_dac_volume_timer )
{
	n8080_state *state = timer.machine().driver_data<n8080_state>();
	double t = state->m_helifire_dac_timing - timer.machine().time().as_double();

	if (state->m_helifire_dac_phase)
	{
		state->m_helifire_dac_volume = 1 - exp(t / ATTACK_RATE);
	}
	else
	{
		state->m_helifire_dac_volume = exp(t / DECAY_RATE);
	}
}


MACHINE_START( spacefev_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_sound_timer[0] = machine.scheduler().timer_alloc(FUNC(stop_mono_flop_callback));
	state->m_sound_timer[1] = machine.scheduler().timer_alloc(FUNC(stop_mono_flop_callback));
	state->m_sound_timer[2] = machine.scheduler().timer_alloc(FUNC(stop_mono_flop_callback));

	state->save_item(NAME(state->m_prev_snd_data));
	state->save_item(NAME(state->m_prev_sound_pins));
	state->save_item(NAME(state->m_curr_sound_pins));
	state->save_item(NAME(state->m_n8080_hardware));
	state->save_item(NAME(state->m_mono_flop));
}

MACHINE_RESET( spacefev_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();
	state->m_n8080_hardware = 1;

	state->m_mono_flop[0] = 0;
	state->m_mono_flop[1] = 0;
	state->m_mono_flop[2] = 0;
	state->m_prev_snd_data = 0;
	state->m_prev_sound_pins = 0;
	state->m_curr_sound_pins = 0;

	delayed_sound_1(machine, 0);
	delayed_sound_2(machine, 0);
}


MACHINE_START( sheriff_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->m_sound_timer[0] = machine.scheduler().timer_alloc(FUNC(stop_mono_flop_callback));
	state->m_sound_timer[1] = machine.scheduler().timer_alloc(FUNC(stop_mono_flop_callback));

	state->save_item(NAME(state->m_prev_snd_data));
	state->save_item(NAME(state->m_prev_sound_pins));
	state->save_item(NAME(state->m_curr_sound_pins));
	state->save_item(NAME(state->m_n8080_hardware));
	state->save_item(NAME(state->m_mono_flop));
}

MACHINE_RESET( sheriff_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();
	state->m_n8080_hardware = 2;

	state->m_mono_flop[0] = 0;
	state->m_mono_flop[1] = 0;
	state->m_prev_snd_data = 0;
	state->m_prev_sound_pins = 0;
	state->m_curr_sound_pins = 0;

	delayed_sound_1(machine, 0);
	delayed_sound_2(machine, 0);
}


MACHINE_START( helifire_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();

	state->save_item(NAME(state->m_prev_snd_data));
	state->save_item(NAME(state->m_prev_sound_pins));
	state->save_item(NAME(state->m_curr_sound_pins));
	state->save_item(NAME(state->m_n8080_hardware));
	state->save_item(NAME(state->m_helifire_dac_volume));
	state->save_item(NAME(state->m_helifire_dac_timing));
	state->save_item(NAME(state->m_helifire_dac_phase));
}

MACHINE_RESET( helifire_sound )
{
	n8080_state *state = machine.driver_data<n8080_state>();
	state->m_n8080_hardware = 3;

	state->m_helifire_dac_volume = 1;
	state->m_helifire_dac_timing = 0;
	state->m_helifire_dac_phase = 0;
	state->m_prev_snd_data = 0;
	state->m_prev_sound_pins = 0;
	state->m_curr_sound_pins = 0;

	delayed_sound_1(machine, 0);
	delayed_sound_2(machine, 0);
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

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(n8080_sound_io_map)

	MCFG_TIMER_ADD_PERIODIC("vco_timer", spacefev_vco_voltage_timer, attotime::from_hz(1000))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SOUND_CONFIG(spacefev_sn76477_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( sheriff_sound )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(n8080_sound_io_map)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("snsnd", SN76477, 0)
	MCFG_SOUND_CONFIG(sheriff_sn76477_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( helifire_sound )

	/* basic machine hardware */
	MCFG_CPU_ADD("audiocpu", I8035, 6000000)
	MCFG_CPU_PROGRAM_MAP(n8080_sound_cpu_map)
	MCFG_CPU_IO_MAP(helifire_sound_io_map)

	MCFG_TIMER_ADD_PERIODIC("helifire_dac", helifire_dac_volume_timer, attotime::from_hz(1000) )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END
