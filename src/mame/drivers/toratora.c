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
#include "machine/6821pia.h"
#include "sound/sn76477.h"


static UINT8 *toratora_videoram;
static size_t toratora_videoram_size;

static UINT8 clear_tv;



/*************************************
 *
 *  Input handling
 *
 *************************************/

static READ8_HANDLER( port_b_u3_r )
{
	logerror("%04x: read DIP\n",activecpu_get_pc());
	return readinputport(1);
}


static WRITE8_HANDLER( cb2_u3_w )
{
	logerror("DIP tristate %sactive\n",(data & 1) ? "in" : "");
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

static VIDEO_UPDATE( toratora )
{
	offs_t offs;

	for (offs = 0; offs < toratora_videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;
		UINT8 data = toratora_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? RGB_WHITE : RGB_BLACK;
			*BITMAP_ADDR32(bitmap, y, x) = pen;

			data = data << 1;
			x = x + 1;
		}

		/* the video system clears as it writes out the pixels */
		if (clear_tv)
			toratora_videoram[offs] = 0;
	}

	clear_tv = 0;

	return 0;
}


static WRITE8_HANDLER( clear_tv_w )
{
	clear_tv = 1;
}



/*************************************
 *
 *  Coin counter
 *
 *************************************/

static WRITE8_HANDLER( port_b_u1_w )
{
	if (pia_get_port_b_z_mask(0) & 0x20)
		coin_counter_w(0, 1);
	else
		coin_counter_w(0, data & 0x20);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static void main_cpu_irq(int state)
{
logerror("GEN IRQ: %x\n", state);
	cpunum_set_input_line(0, 0, state ? ASSERT_LINE : CLEAR_LINE);
}





static int timer;

static INTERRUPT_GEN( toratora_timer )
{
static UINT8 last = 0;
	timer++;	/* timer counting at 16 Hz */

	/* also, when the timer overflows (16 seconds) watchdog would kick in */
	if (timer & 0x100) popmessage("watchdog!");


	if (last != (input_port_0_r(0) & 0x0f))
	{
		last = input_port_0_r(0) & 0x0f;
		cpunum_set_input_line(0, 0, PULSE_LINE);
	}
	pia_set_input_a(0, input_port_0_r(0) & 0x0f, 0);

	pia_set_input_ca1(0, input_port_0_r(0) & 0x10);

	pia_set_input_ca2(0, input_port_0_r(0) & 0x20);
}

static READ8_HANDLER( timer_r )
{
	return timer;
}

static WRITE8_HANDLER( clear_timer_w )
{
	timer = 0;
}



/*************************************
 *
 *  Audio hardware
 *
 *************************************/

static struct SN76477interface sn76477_interface =
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


static void port_a_u2_u3_w(int which, UINT8 data)
{
	SN76477_vco_voltage_w(which, 2.35 * (data & 0x7f) / 128.0);
	SN76477_enable_w(which, (data >> 7) & 0x01);
}


static void port_b_u2_u3_w(int which, UINT8 data)
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

	SN76477_mixer_a_w      (which, (data >> 0) & 0x01);
	SN76477_mixer_b_w      (which, (data >> 1) & 0x01);
	SN76477_mixer_c_w      (which, (data >> 2) & 0x01);
	SN76477_envelope_1_w   (which, (data >> 3) & 0x01);
	SN76477_envelope_2_w   (which, (data >> 4) & 0x01);
	SN76477_amplitude_res_w(which, resistances[(data >> 5)] * 2);  /* the *2 shouldn't be neccassary, but... */
}


static void ca2_u2_u3_w(int which, UINT8 data)
{
	SN76477_vco_w(which, data);
}


static WRITE8_HANDLER( port_a_u2_w )
{
	port_a_u2_u3_w(0, data);
}


static WRITE8_HANDLER( port_a_u3_w )
{
	port_a_u2_u3_w(1, data);
}


static WRITE8_HANDLER( port_b_u2_w )
{
	port_b_u2_u3_w(0, data);
}


static WRITE8_HANDLER( port_b_u3_w )
{
	port_b_u2_u3_w(1, data);
}


static WRITE8_HANDLER( ca2_u2_w )
{
	ca2_u2_u3_w(0, data);
}


static WRITE8_HANDLER( ca2_u3_w )
{
	ca2_u2_u3_w(1, data);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

static const pia6821_interface pia_u1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, port_b_u1_w, 0, 0,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq,
};

static const pia6821_interface pia_u2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ port_a_u2_w, port_b_u2_w, ca2_u2_w, 0,
	/*irqs   : A/B             */ 0, 0,
};


static const pia6821_interface pia_u3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, port_b_u3_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ port_a_u3_w, port_b_u3_w, ca2_u3_w, cb2_u3_w,
	/*irqs   : A/B             */ 0, 0,
};



static MACHINE_START( toratora )
{
	pia_config(0, &pia_u1_intf);
	pia_config(1, &pia_u3_intf);
	pia_config(2, &pia_u2_intf);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( toratora )
{
	pia_reset();
}



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
	AM_RANGE(0x8000, 0x9fff) AM_RAM AM_BASE(&toratora_videoram) AM_SIZE(&toratora_videoram_size)
	AM_RANGE(0xa000, 0xf047) AM_NOP
	AM_RANGE(0xf048, 0xf049) AM_NOP
	AM_RANGE(0xf04a, 0xf04a) AM_WRITE(clear_tv_w)	/* the read is mark *LEDEN, but not used */
	AM_RANGE(0xf04b, 0xf04b) AM_READWRITE(timer_r, clear_timer_w)
	AM_RANGE(0xa04c, 0xf09f) AM_NOP
	AM_RANGE(0xf0a0, 0xf0a3) AM_READWRITE(pia_0_r, pia_0_w)
	AM_RANGE(0xf0a4, 0xf0a7) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0xf0a8, 0xf0ab) AM_READWRITE(pia_2_r, pia_2_w)
	AM_RANGE(0xf0ac, 0xf7ff) AM_NOP
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( toratora )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START
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

static MACHINE_DRIVER_START( toratora )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6800,500000)	/* ?????? game speed is entirely controlled by this */
	MDRV_CPU_PROGRAM_MAP(main_map,0)
	MDRV_CPU_PERIODIC_INT(toratora_timer,16)	/* timer counting at 16 Hz */

	MDRV_MACHINE_START(toratora)
	MDRV_MACHINE_RESET(toratora)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_UPDATE(toratora)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0,256-1,8,248-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(SN76477, 0)
	MDRV_SOUND_CONFIG(sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(SN76477, 0)
	MDRV_SOUND_CONFIG(sn76477_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( toratora )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
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

GAME( 1980, toratora, 0, toratora, toratora, 0, ROT90, "GamePlan", "Tora Tora (prototype?)", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND )
