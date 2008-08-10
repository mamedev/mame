/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/i8039/i8039.h"
#include "sound/sn76477.h"
#include "sound/dac.h"
#include "includes/n8080.h"

static int n8080_hardware;

static emu_timer* sound_timer[3];

static int helifire_dac_phase;

static const double ATTACK_RATE = 10e-6 * 500;
static const double DECAY_RATE = 10e-6 * 16000;

static double helifire_dac_volume;
static double helifire_dac_timing;

static UINT16 prev_sound_pins;
static UINT16 curr_sound_pins;

static int mono_flop[3];



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


static void spacefev_update_SN76477_status(void)
{
	double dblR0 = RES_M(1.0);
	double dblR1 = RES_M(1.5);

	if (!mono_flop[0])
	{
		dblR0 = 1 / (1 / RES_K(150) + 1 / dblR0); /* ? */
	}
	if (!mono_flop[1])
	{
		dblR1 = 1 / (1 / RES_K(620) + 1 / dblR1); /* ? */
	}

	sn76477_decay_res_w(0, dblR0);

	sn76477_vco_res_w(0, dblR1);

	sn76477_enable_w(0,
		!mono_flop[0] &&
		!mono_flop[1] &&
		!mono_flop[2]);

	sn76477_vco_w(0, mono_flop[1]);

	sn76477_mixer_b_w(0, mono_flop[0]);
}


static void sheriff_update_SN76477_status(void)
{
	if (mono_flop[1])
	{
		sn76477_vco_voltage_w(0, 5);
	}
	else
	{
		sn76477_vco_voltage_w(0, 0);
	}

	sn76477_enable_w(0,
		!mono_flop[0] &&
		!mono_flop[1]);

	sn76477_vco_w(0, mono_flop[0]);

	sn76477_mixer_b_w(0, !mono_flop[0]);
}


static void update_SN76477_status(void)
{
	if (n8080_hardware == 1)
	{
		spacefev_update_SN76477_status();
	}
	if (n8080_hardware == 2)
	{
		sheriff_update_SN76477_status();
	}
}


static void start_mono_flop(int n, attotime expire)
{
	mono_flop[n] = 1;

	update_SN76477_status();

	timer_adjust_oneshot(sound_timer[n], expire, n);
}


static void stop_mono_flop(int n)
{
	mono_flop[n] = 0;

	update_SN76477_status();

	timer_adjust_oneshot(sound_timer[n], attotime_never, n);
}


static TIMER_CALLBACK( stop_mono_flop_callback )
{
	stop_mono_flop(param);
}


static void spacefev_sound_pins_changed(void)
{
	UINT16 changes = ~curr_sound_pins & prev_sound_pins;

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
		start_mono_flop(0, ATTOTIME_IN_USEC(550 * 36 * 100));
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(1, ATTOTIME_IN_USEC(550 * 22 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(2, ATTOTIME_IN_USEC(550 * 22 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		cpunum_set_input_line(Machine, 1, 0, PULSE_LINE);
	}
}


static void sheriff_sound_pins_changed(void)
{
	UINT16 changes = ~curr_sound_pins & prev_sound_pins;

	if (changes & (1 << 0x6))
	{
		stop_mono_flop(1);
	}
	if (changes & (1 << 0x6))
	{
		start_mono_flop(0, ATTOTIME_IN_USEC(550 * 33 * 33));
	}
	if (changes & (1 << 0x4))
	{
		start_mono_flop(1, ATTOTIME_IN_USEC(550 * 33 * 33));
	}
	if (changes & ((1 << 0x2) | (1 << 0x3) | (1 << 0x5)))
	{
		cpunum_set_input_line(Machine, 1, 0, PULSE_LINE);
	}
}


static void helifire_sound_pins_changed(void)
{
	UINT16 changes = ~curr_sound_pins & prev_sound_pins;

	/* ((curr_sound_pins >> 0xA) & 1) not emulated */
	/* ((curr_sound_pins >> 0xB) & 1) not emulated */
	/* ((curr_sound_pins >> 0xC) & 1) not emulated */

	if (changes & (1 << 6))
	{
		cpunum_set_input_line(Machine, 1, 0, PULSE_LINE);
	}
}


static void sound_pins_changed(void)
{
	if (n8080_hardware == 1)
	{
		spacefev_sound_pins_changed();
	}
	if (n8080_hardware == 2)
	{
		sheriff_sound_pins_changed();
	}
	if (n8080_hardware == 3)
	{
		helifire_sound_pins_changed();
	}

	prev_sound_pins = curr_sound_pins;
}


static void delayed_sound_1(int data)
{
	static UINT8 prev_data = 0;

	curr_sound_pins &= ~(
		(1 << 0x7) |
		(1 << 0x5) |
		(1 << 0x6) |
		(1 << 0x3) |
		(1 << 0x4) |
		(1 << 0x1));

	if (~data & 0x01) curr_sound_pins |= 1 << 0x7;
	if (~data & 0x02) curr_sound_pins |= 1 << 0x5; /* pulse */
	if (~data & 0x04) curr_sound_pins |= 1 << 0x6; /* pulse */
	if (~data & 0x08) curr_sound_pins |= 1 << 0x3; /* pulse (except in Helifire) */
	if (~data & 0x10) curr_sound_pins |= 1 << 0x4; /* pulse (except in Helifire) */
	if (~data & 0x20) curr_sound_pins |= 1 << 0x1;

	if (n8080_hardware == 1)
	{
		if (data & ~prev_data & 0x10)
		{
			spacefev_start_red_cannon();
		}

		spacefev_red_screen = data & 0x08;
	}

	sound_pins_changed();

	prev_data = data;
}


static TIMER_CALLBACK( delayed_sound_1_callback )
{
	delayed_sound_1(param);
}


static void delayed_sound_2(int data)
{
	curr_sound_pins &= ~(
		(1 << 0x8) |
		(1 << 0x9) |
		(1 << 0xA) |
		(1 << 0xB) |
		(1 << 0x2) |
		(1 << 0xC));

	if (~data & 0x01) curr_sound_pins |= 1 << 0x8;
	if (~data & 0x02) curr_sound_pins |= 1 << 0x9;
	if (~data & 0x04) curr_sound_pins |= 1 << 0xA;
	if (~data & 0x08) curr_sound_pins |= 1 << 0xB;
	if (~data & 0x10) curr_sound_pins |= 1 << 0x2; /* pulse */
	if (~data & 0x20) curr_sound_pins |= 1 << 0xC;

	if (n8080_hardware == 1)
	{
		flip_screen_set_no_update(data & 0x20);
	}
	if (n8080_hardware == 3)
	{
		helifire_flash = data & 0x20;
	}

	sound_pins_changed();
}


static TIMER_CALLBACK( delayed_sound_2_callback )
{
	delayed_sound_2(param);
}


WRITE8_HANDLER( n8080_sound_1_w )
{
	timer_call_after_resynch(NULL, data, delayed_sound_1_callback); /* force CPUs to sync */
}
WRITE8_HANDLER( n8080_sound_2_w )
{
	timer_call_after_resynch(NULL, data, delayed_sound_2_callback); /* force CPUs to sync */
}


static READ8_HANDLER( n8080_8035_p1_r )
{
	UINT8 val = 0;

	if ((curr_sound_pins >> 0xB) & 1) val |= 0x01;
	if ((curr_sound_pins >> 0xA) & 1) val |= 0x02;
	if ((curr_sound_pins >> 0x9) & 1) val |= 0x04;
	if ((curr_sound_pins >> 0x8) & 1) val |= 0x08;
	if ((curr_sound_pins >> 0x5) & 1) val |= 0x10;
	if ((curr_sound_pins >> 0x3) & 1) val |= 0x20;
	if ((curr_sound_pins >> 0x2) & 1) val |= 0x40;
	if ((curr_sound_pins >> 0x1) & 1) val |= 0x80;

	return val;
}


static READ8_HANDLER( n8080_8035_t0_r )
{
	return (curr_sound_pins >> 0x7) & 1;
}
static READ8_HANDLER( n8080_8035_t1_r )
{
	return (curr_sound_pins >> 0xC) & 1;
}


static READ8_HANDLER( helifire_8035_t0_r )
{
	return (curr_sound_pins >> 0x3) & 1;
}
static READ8_HANDLER( helifire_8035_t1_r )
{
	return (curr_sound_pins >> 0x4) & 1;
}


static READ8_HANDLER( helifire_8035_external_ram_r )
{
	UINT8 val = 0;

	if ((curr_sound_pins >> 0x7) & 1) val |= 0x01;
	if ((curr_sound_pins >> 0x8) & 1) val |= 0x02;
	if ((curr_sound_pins >> 0x9) & 1) val |= 0x04;
	if ((curr_sound_pins >> 0x1) & 1) val |= 0x08;

	return val;
}


static READ8_HANDLER( helifire_8035_p2_r )
{
	return ((curr_sound_pins >> 0xC) & 1) ? 0x10 : 0x00; /* not used */
}


static WRITE8_HANDLER( n8080_dac_w )
{
	DAC_data_w(0, data & 0x80);
}


static WRITE8_HANDLER( helifire_dac_w )
{
	DAC_data_w(0, data * helifire_dac_volume);
}


static WRITE8_HANDLER( helifire_sound_ctrl_w )
{
	helifire_dac_phase = data & 0x80;

	/* data & 0x40 not emulated */
	/* data & 0x20 not emulated */

	if (helifire_dac_phase)
	{
		helifire_dac_timing = ATTACK_RATE * log(1 - helifire_dac_volume);
	}
	else
	{
		helifire_dac_timing = DECAY_RATE * log(helifire_dac_volume);
	}

	helifire_dac_timing += attotime_to_double(timer_get_time());
}


static TIMER_CALLBACK( spacefev_vco_voltage_timer )
{
	double voltage = 0;

	if (mono_flop[2])
	{
		voltage = 5 * (1 - exp(- attotime_to_double(timer_timeelapsed(sound_timer[2])) / 0.22));
	}

	sn76477_vco_voltage_w(0, voltage);
}


static TIMER_CALLBACK( helifire_dac_volume_timer )
{
	double t = helifire_dac_timing - attotime_to_double(timer_get_time());

	if (helifire_dac_phase)
	{
		helifire_dac_volume = 1 - exp(t / ATTACK_RATE);
	}
	else
	{
		helifire_dac_volume = exp(t / DECAY_RATE);
	}
}


static MACHINE_RESET( spacefev_sound )
{
	n8080_hardware = 1;

	timer_pulse(ATTOTIME_IN_HZ(1000), NULL, 0, spacefev_vco_voltage_timer);

	sound_timer[0] = timer_alloc(stop_mono_flop_callback, NULL);
	sound_timer[1] = timer_alloc(stop_mono_flop_callback, NULL);
	sound_timer[2] = timer_alloc(stop_mono_flop_callback, NULL);

	mono_flop[0] = 0;
	mono_flop[1] = 0;
	mono_flop[2] = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


static MACHINE_RESET( sheriff_sound )
{
	n8080_hardware = 2;

	sound_timer[0] = timer_alloc(stop_mono_flop_callback, NULL);
	sound_timer[1] = timer_alloc(stop_mono_flop_callback, NULL);

	mono_flop[0] = 0;
	mono_flop[1] = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);
}


static MACHINE_RESET( helifire_sound )
{
	n8080_hardware = 3;

	timer_pulse(ATTOTIME_IN_HZ(1000), NULL, 0, helifire_dac_volume_timer);

	helifire_dac_volume = 1;
	helifire_dac_timing = 0;

	delayed_sound_1(0);
	delayed_sound_2(0);

	helifire_dac_phase = 0;
}


static ADDRESS_MAP_START( n8080_sound_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( n8080_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(n8080_8035_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(n8080_8035_t1_r)
	AM_RANGE(I8039_p1, I8039_p1) AM_READ(n8080_8035_p1_r)

	AM_RANGE(I8039_p2, I8039_p2) AM_WRITE(n8080_dac_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( helifire_sound_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(I8039_t0, I8039_t0) AM_READ(helifire_8035_t0_r)
	AM_RANGE(I8039_t1, I8039_t1) AM_READ(helifire_8035_t1_r)
	AM_RANGE(I8039_p2, I8039_p2) AM_READ(helifire_8035_p2_r)

	AM_RANGE(0x00, 0x7f) AM_READ(helifire_8035_external_ram_r)

	AM_RANGE(I8039_p1, I8039_p1) AM_WRITE(helifire_dac_w)
	AM_RANGE(I8039_p2, I8039_p2) AM_WRITE(helifire_sound_ctrl_w)
ADDRESS_MAP_END


MACHINE_DRIVER_START( spacefev_sound )

	/* basic machine hardware */
	MDRV_CPU_ADD("audio", I8035, 6000000)
	MDRV_CPU_PROGRAM_MAP(n8080_sound_cpu_map, 0)
	MDRV_CPU_IO_MAP(n8080_sound_io_map, 0)

	MDRV_MACHINE_RESET(spacefev_sound)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(spacefev_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( sheriff_sound )

	/* basic machine hardware */
	MDRV_CPU_ADD("audio", I8035, 6000000)
	MDRV_CPU_PROGRAM_MAP(n8080_sound_cpu_map, 0)
	MDRV_CPU_IO_MAP(n8080_sound_io_map, 0)

	MDRV_MACHINE_RESET(sheriff_sound)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MDRV_SOUND_ADD("sn", SN76477, 0)
	MDRV_SOUND_CONFIG(sheriff_sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.35)
MACHINE_DRIVER_END


MACHINE_DRIVER_START( helifire_sound )

	/* basic machine hardware */
	MDRV_CPU_ADD("audio", I8035, 6000000)
	MDRV_CPU_PROGRAM_MAP(n8080_sound_cpu_map, 0)
	MDRV_CPU_IO_MAP(helifire_sound_io_map, 0)

	MDRV_MACHINE_RESET(helifire_sound)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("dac", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END
