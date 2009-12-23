/***************************************************************************

Tora Tora (c) 1980 GamePlan

driver by Nicola Salmoria

TODO:
- The game doesn't seem to work right. It also reads some unmapped memory
  addresses, are the two things related? Missing ROMs? There's an empty
  socket for U3 on the board, which should map at 5000-57ff, however the
  game reads mostly from 4800-4fff, which would be U6 according to the
  schematics.

- The manual mentions dip switch settings and the schematics show the switches,
  the game reads them but ignores them, forcing 1C/1C and 3 lives.
  Maybe the dump is from a proto?

***************************************************************************/

#include "driver.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "sound/sn76477.h"


typedef struct _toratora_state toratora_state;
struct _toratora_state
{
	/* memory pointers */
	UINT8 *    videoram;
	size_t     videoram_size;

	/* video-related */
	int        timer;
	UINT8      last;
	UINT8      clear_tv;

	/* devices */
	const device_config *maincpu;
	const device_config *pia_u1;
	const device_config *pia_u2;
	const device_config *pia_u3;
};



/*************************************
 *
 *  Input handling
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( cb2_u3_w )
{
	logerror("DIP tristate %sactive\n",(state & 1) ? "in" : "");
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

static VIDEO_UPDATE( toratora )
{
	toratora_state *state = (toratora_state *)screen->machine->driver_data;
	offs_t offs;

	for (offs = 0; offs < state->videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;
		UINT8 data = state->videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			data = data << 1;
			x = x + 1;
		}

		/* the video system clears as it writes out the pixels */
		if (state->clear_tv)
			state->videoram[offs] = 0;
	}

	state->clear_tv = 0;

	return 0;
}


static WRITE8_HANDLER( clear_tv_w )
{
	toratora_state *state = (toratora_state *)space->machine->driver_data;
	state->clear_tv = 1;
}


/*************************************
 *
 *  Coin counter
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( port_b_u1_w )
{
	if (pia6821_get_port_b_z_mask(device) & 0x20)
		coin_counter_w(device->machine, 0, 1);
	else
		coin_counter_w(device->machine, 0, data & 0x20);
}


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( main_cpu_irq )
{
	toratora_state *toratora = (toratora_state *)device->machine->driver_data;
	int combined_state = pia6821_get_irq_a(device) | pia6821_get_irq_b(device);

	logerror("GEN IRQ: %x\n", combined_state);
	cpu_set_input_line(toratora->maincpu, 0, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


static INTERRUPT_GEN( toratora_timer )
{
	toratora_state *state = (toratora_state *)device->machine->driver_data;
	state->timer++;	/* timer counting at 16 Hz */

	/* also, when the timer overflows (16 seconds) watchdog would kick in */
	if (state->timer & 0x100)
		popmessage("watchdog!");

	if (state->last != (input_port_read(device->machine, "INPUT") & 0x0f))
	{
		state->last = input_port_read(device->machine, "INPUT") & 0x0f;
		generic_pulse_irq_line(device, 0);
	}
	pia6821_set_input_a(state->pia_u1, input_port_read(device->machine, "INPUT") & 0x0f, 0);
	pia6821_ca1_w(state->pia_u1, 0, input_port_read(device->machine, "INPUT") & 0x10);
	pia6821_ca2_w(state->pia_u1, 0, input_port_read(device->machine, "INPUT") & 0x20);
}

static READ8_HANDLER( timer_r )
{
	toratora_state *state = (toratora_state *)space->machine->driver_data;
	return state->timer;
}

static WRITE8_HANDLER( clear_timer_w )
{
	toratora_state *state = (toratora_state *)space->machine->driver_data;
	state->timer = 0;
}


/*************************************
 *
 *  Audio hardware
 *
 *************************************/

static const sn76477_interface sn76477_intf =
{
	RES_K(47),	/*  4 noise_res                */
//  RES_K(120), /*  5 filter_res               */
	RES_M(1.2),	/*  5 filter_res               */
	CAP_P(470), /*  6 filter_cap               */
	RES_K(680),	/*  7 decay_res                */
	CAP_U(0.2),	/*  8 attack_decay_cap         */
	RES_K(3.3), /* 10 attack_res               */
	0,		    /* 11 amplitude_res (variable) */
	RES_K(50),	/* 12 feedback_res             */
	0,			/* 16 vco_voltage (variable)   */
	CAP_U(0.1),	/* 17 vco_cap                  */
	RES_K(51),	/* 18 vco_res                  */
	5.0,		/* 19 pitch_voltage (N/C)      */
	RES_K(470),	/* 20 slf_res                  */
	CAP_U(0.1),	/* 21 slf_cap                  */
	CAP_U(0.1),	/* 23 oneshot_cap              */
	RES_M(1),	/* 24 oneshot_res              */
	0,			/* 22 vco (variable)           */
	0,			/* 26 mixer A (variable)       */
	0,			/* 25 mixer B (variable)       */
	0,			/* 27 mixer C (variable)       */
	0,			/* 1  envelope 1 (variable)    */
	0,			/* 28 envelope 2 (variable)    */
	1			/* 9  enable (variable)        */
};


static WRITE8_DEVICE_HANDLER( port_a_u2_u3_w )
{
	sn76477_vco_voltage_w(device, 2.35 * (data & 0x7f) / 128.0);
	sn76477_enable_w(device, (data >> 7) & 0x01);
}


static WRITE8_DEVICE_HANDLER( port_b_u2_u3_w )
{
	static const double resistances[] =
	{
	  0,  /* N/C */
	  RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750) + RES_M(1.5),
	  RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360) + RES_K(750),
	  RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200) + RES_K(360),
	  RES_K(47) + RES_K(47) + RES_K(91) + RES_K(200),
	  RES_K(47) + RES_K(47) + RES_K(91),
	  RES_K(47) + RES_K(47) + RES_K(91),
	  RES_K(47)
	};

	sn76477_mixer_a_w      (device, (data >> 0) & 0x01);
	sn76477_mixer_b_w      (device, (data >> 1) & 0x01);
	sn76477_mixer_c_w      (device, (data >> 2) & 0x01);
	sn76477_envelope_1_w   (device, (data >> 3) & 0x01);
	sn76477_envelope_2_w   (device, (data >> 4) & 0x01);
	sn76477_amplitude_res_w(device, resistances[(data >> 5)] * 2);  /* the *2 shouldn't be neccassary, but... */
}


static WRITE_LINE_DEVICE_HANDLER( ca2_u2_u3_w )
{
	sn76477_vco_w(device, state);
}


/*************************************
 *
 *  Machine setup
 *
 *************************************/

static const pia6821_interface pia_u1_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(port_b_u1_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_LINE(main_cpu_irq),		/* IRQA */
	DEVCB_LINE(main_cpu_irq)		/* IRQB */
};


static const pia6821_interface pia_u2_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("sn1", port_a_u2_u3_w),		/* port A out */
	DEVCB_DEVICE_HANDLER("sn1", port_b_u2_u3_w),		/* port B out */
	DEVCB_DEVICE_LINE("sn1", ca2_u2_u3_w),				/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


static const pia6821_interface pia_u3_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_INPUT_PORT("DSW"),		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_DEVICE_HANDLER("sn2", port_a_u2_u3_w),		/* port A out */
	DEVCB_DEVICE_HANDLER("sn2", port_b_u2_u3_w),		/* port B out */
	DEVCB_DEVICE_LINE("sn2", ca2_u2_u3_w),				/* line CA2 out */
	DEVCB_LINE(cb2_u3_w),								/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};




/*************************************
 *
 *  Memory handlers
 *
 *  No mirrors, all addresses are
 *  fully decoded by the hardware!
 *
 *************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x7fff) AM_ROM  /* not fully populated */
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_BASE_SIZE_MEMBER(toratora_state, videoram, videoram_size)
	AM_RANGE(0xa000, 0xf047) AM_NOP
	AM_RANGE(0xf048, 0xf049) AM_NOP
	AM_RANGE(0xf04a, 0xf04a) AM_WRITE(clear_tv_w)	/* the read is mark *LEDEN, but not used */
	AM_RANGE(0xf04b, 0xf04b) AM_READWRITE(timer_r, clear_timer_w)
	AM_RANGE(0xa04c, 0xf09f) AM_NOP
	AM_RANGE(0xf0a0, 0xf0a3) AM_DEVREADWRITE("pia_u1", pia6821_r, pia6821_w)
	AM_RANGE(0xf0a4, 0xf0a7) AM_DEVREADWRITE("pia_u3", pia6821_r, pia6821_w)
	AM_RANGE(0xf0a8, 0xf0ab) AM_DEVREADWRITE("pia_u2", pia6821_r, pia6821_w)
	AM_RANGE(0xf0ac, 0xf7ff) AM_NOP
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( toratora )
	PORT_START("INPUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x02, "0" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( toratora )
{
	toratora_state *state = (toratora_state *)machine->driver_data;

	state->maincpu = devtag_get_device(machine, "maincpu");
	state->pia_u1 = devtag_get_device(machine, "pia_u1");
	state->pia_u2 = devtag_get_device(machine, "pia_u2");
	state->pia_u3 = devtag_get_device(machine, "pia_u3");

	state_save_register_global(machine, state->timer);
	state_save_register_global(machine, state->last);
	state_save_register_global(machine, state->clear_tv);
}

static MACHINE_RESET( toratora )
{
	toratora_state *state = (toratora_state *)machine->driver_data;

	state->timer = 0xff;
	state->last = 0;
	state->clear_tv = 0;
}

static MACHINE_DRIVER_START( toratora )

	/* driver data */
	MDRV_DRIVER_DATA(toratora_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6800,500000)	/* ?????? game speed is entirely controlled by this */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_PERIODIC_INT(toratora_timer,16)	/* timer counting at 16 Hz */

	MDRV_PIA6821_ADD("pia_u1", pia_u1_intf)
	MDRV_PIA6821_ADD("pia_u2", pia_u2_intf)
	MDRV_PIA6821_ADD("pia_u3", pia_u3_intf)

	MDRV_MACHINE_START(toratora)
	MDRV_MACHINE_RESET(toratora)

	/* video hardware */
	MDRV_VIDEO_UPDATE(toratora)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0,256-1,8,248-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("sn1", SN76477, 0)
	MDRV_SOUND_CONFIG(sn76477_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("sn2", SN76477, 0)
	MDRV_SOUND_CONFIG(sn76477_intf)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( toratora )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tora.u1",  0x1000, 0x0800, CRC(413c743a) SHA1(a887dfaaee557327a1699bb424488b934dab8612) )
	ROM_LOAD( "tora.u10", 0x1800, 0x0800, CRC(dc771b1c) SHA1(1bd81decb4d0a854878227c52d45ac0eea0602ec) )
	ROM_LOAD( "tora.u2",  0x2000, 0x0800, CRC(c574c664) SHA1(9f41a53ca51d04e5bec7525fe83c5f4bdfcf128d) )
	ROM_LOAD( "tora.u9",  0x2800, 0x0800, CRC(b67aa11f) SHA1(da9e77255640a4b32eed2be89b686b98a248bd72) )
	ROM_LOAD( "tora.u11", 0xf800, 0x0800, CRC(55135d6f) SHA1(c48f180a9d6e894aafe87b2daf74e9a082f4600e) )
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, toratora, 0, toratora, toratora, 0, ROT90, "GamePlan", "Tora Tora (prototype?)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
