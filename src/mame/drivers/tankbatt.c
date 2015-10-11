// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

Tank Battalion memory map (preliminary)

driver by Brad Oliver

$0000-$000f : bullet ram, first entry is player's bullet
$0010-$01ff : zero page & stack
$0200-$07ff : RAM
$0800-$0bff : videoram
$0c00-$0c1f : I/O

Read:
    $0c00-$0c03 : p1 joystick
    $0c04
    $0c07       : stop at grid self-test if bit 7 is low
    $0c0f       : stop at first self-test if bit 7 is low

    $0c18       : Cabinet, 0 = table, 1 = upright
    $0c19-$0c1a : Coinage, 00 = free play, 01 = 2 coin 1 credit, 10 = 1 coin 2 credits, 11 = 1 coin 1 credit
    $0c1b-$0c1c : Bonus, 00 = 10000, 01 = 15000, 10 = 20000, 11 = none
    $0c1d       : Tanks, 0 = 3, 1 = 2
    $0c1e-$0c1f : ??

Write:
    $0c00-$0c01 : p1/p2 start leds
    $0c02       : ?? written to at end of IRQ, either 0 or 1 - coin counter?
    $0c03       : ?? written to during IRQ if grid test is on
    $0c08       : ?? written to during IRQ if grid test is on
    $0c09       : Sound - coin ding
    $0c0a       : NMI enable (active low) ?? game only ??
    $0c0b       : Sound - background noise, 0 - low rumble, 1 - high rumble
    $0c0c       : Sound - player fire
    $0c0d       : Sound - explosion
    $0c0f       : NMI enable (active high) ?? demo only ??

    $0c10       : IRQ ack ??
    $0c18       : Watchdog ?? Not written to while game screen is up

$2000-$3fff : ROM

TODO:
    . Needs proper discrete emulation
    . Resistor values on the color prom need to be verified

Changes:
    28 Feb 98 LBO
        . Fixed the coin interrupts
        . Fixed the color issues, should be 100% if I guessed at the resistor values properly
        . Fixed the 2nd player cocktail joystick, had the polarity reversed
        . Hacked the sound sample triggers so they work better

Known issues:
    . The 'moving' tank rumble noise seems to keep playing a second too long
    . Sample support is all a crapshoot. I have no idea how it really works

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/samples.h"
#include "includes/tankbatt.h"


void tankbatt_state::machine_start()
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_sound_enable));
}

WRITE8_MEMBER(tankbatt_state::led_w)
{
	set_led_status(machine(), offset,data & 1);
}

READ8_MEMBER(tankbatt_state::in0_r)
{
	int val;

	val = ioport("P1")->read();
	return ((val << (7 - offset)) & 0x80);
}

READ8_MEMBER(tankbatt_state::in1_r)
{
	int val;

	val = ioport("P2")->read();
	return ((val << (7 - offset)) & 0x80);
}

READ8_MEMBER(tankbatt_state::dsw_r)
{
	int val;

	val = ioport("DSW")->read();
	return ((val << (7 - offset)) & 0x80);
}

WRITE8_MEMBER(tankbatt_state::interrupt_enable_w)
{
	m_nmi_enable = !data;
	m_sound_enable = !data;

	/* hack - turn off the engine noise if the normal game nmi's are disabled */
	if (data) m_samples->stop(2);
}

WRITE8_MEMBER(tankbatt_state::demo_interrupt_enable_w)
{
	m_nmi_enable = data;
}

WRITE8_MEMBER(tankbatt_state::sh_expl_w)
{
	if (m_sound_enable)
	{
		m_samples->start(1, 3);
	}
}

WRITE8_MEMBER(tankbatt_state::sh_engine_w)
{
	if (m_sound_enable)
	{
		if (data)
			m_samples->start(2, 2, true);
		else
			m_samples->start(2, 1, true);
	}
	else m_samples->stop(2);
}

WRITE8_MEMBER(tankbatt_state::sh_fire_w)
{
	if (m_sound_enable)
	{
		m_samples->start(0, 0);
	}
}

WRITE8_MEMBER(tankbatt_state::irq_ack_w)
{
	/* 0x6e written at the end of the irq routine, could be either irq ack or a coin sample */
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(tankbatt_state::coincounter_w)
{
	coin_counter_w(machine(), 0,data & 1);
	coin_counter_w(machine(), 1,data & 1);
}

WRITE8_MEMBER(tankbatt_state::coinlockout_w)
{
	coin_lockout_w(machine(), 0,data & 1);
	coin_lockout_w(machine(), 1,data & 1);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tankbatt_state )
	AM_RANGE(0x0000, 0x000f) AM_RAM AM_SHARE("bulletsram")
	AM_RANGE(0x0010, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0bff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x0c00, 0x0c07) AM_READ(in0_r)
	AM_RANGE(0x0c00, 0x0c01) AM_WRITE(led_w)
	AM_RANGE(0x0c02, 0x0c02) AM_WRITE(coincounter_w)
	AM_RANGE(0x0c03, 0x0c03) AM_WRITE(coinlockout_w)
	AM_RANGE(0x0c08, 0x0c0f) AM_READ(in1_r)
	AM_RANGE(0x0c08, 0x0c08) AM_WRITENOP //coin counter mirror?
	AM_RANGE(0x0c0a, 0x0c0a) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0x0c0b, 0x0c0b) AM_WRITE(sh_engine_w)
	AM_RANGE(0x0c0c, 0x0c0c) AM_WRITE(sh_fire_w)
	AM_RANGE(0x0c0d, 0x0c0d) AM_WRITE(sh_expl_w) // bit 7 == led for the start 2 button
	AM_RANGE(0x0c0e, 0x0c0e) AM_WRITENOP //bit 7 == led for the start 1 button
	AM_RANGE(0x0c0f, 0x0c0f) AM_WRITE(demo_interrupt_enable_w)
	AM_RANGE(0x0c10, 0x0c10) AM_WRITE(irq_ack_w)
	AM_RANGE(0x0c18, 0x0c1f) AM_READ(dsw_r)
	AM_RANGE(0x0c18, 0x0c18) AM_WRITENOP    /* watchdog ?? */
	AM_RANGE(0x6000, 0x7fff) AM_ROM AM_REGION("maincpu",0)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu",0) //mirror for the reset/irq vectors
	AM_RANGE(0x2000, 0xffff) AM_READNOP //anything else might be left-over for a diagnostic ROM or something related to the discrete sound HW
ADDRESS_MAP_END

INTERRUPT_GEN_MEMBER(tankbatt_state::interrupt)
{
	if (m_nmi_enable) device.execute().set_input_line(INPUT_LINE_NMI,PULSE_LINE);
}

INPUT_CHANGED_MEMBER(tankbatt_state::coin_inserted)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}

static INPUT_PORTS_START( tankbatt )
	PORT_START("P1")    /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tankbatt_state,coin_inserted, 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tankbatt_state,coin_inserted, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("P2")    /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )         PORT_DIPLOCATION("DSW:8")

	PORT_START("DSW")   /* DSW */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("DSW:3,4")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x18, DEF_STR( None ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DSW:6" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	1,  /* 1 bit per pixel */
	{ 0 },  /* only one bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout bulletlayout =
{
	/* there is no gfx ROM for this one, it is generated by the hardware */
	3,3,    /* 3*3 box */
	1,  /* just one */
	1,  /* 1 bit per pixel */
	{ 8*8 },
	{ 2, 2, 2 },   /* I "know" that this bit of the */
	{ 2, 2, 2 },   /* graphics ROMs is 1 */
	0   /* no use */
};


static GFXDECODE_START( tankbatt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx1", 0, bulletlayout, 0, 256 )
GFXDECODE_END



static const char *const tankbatt_sample_names[] =
{
	"*tankbatt",
	"fire",
	"engine1",
	"engine2",
	"explode1",
	0   /* end of array */
};


static MACHINE_CONFIG_START( tankbatt, tankbatt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 1000000) /* 1 MHz ???? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tankbatt_state,  interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tankbatt_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tankbatt)
	MCFG_PALETTE_ADD("palette", 256*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(tankbatt_state, tankbatt)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(3)
	MCFG_SAMPLES_NAMES(tankbatt_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tankbatt )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tb1-1.1a",  0x0000, 0x0800, CRC(278a0b8c) SHA1(11ea8fe8401b3cd986616a30a759c0ac1a5ce73b) )
	ROM_LOAD( "tb1-2.1b",  0x0800, 0x0800, CRC(e0923370) SHA1(8d3dbea877bed9f9c267d8002dc180f6eb1e5a8f) )
	ROM_LOAD( "tb1-3.1c",  0x1000, 0x0800, CRC(85005ea4) SHA1(91583081803a5ef600fb90bee34be9edd87f157e) )
	ROM_LOAD( "tb1-4.1d",  0x1800, 0x0800, CRC(3dfb5bcf) SHA1(aa24bf74f4d5dc81baf3843196c837e0b731077b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tb1-5.2k",  0x0000, 0x0800, CRC(aabd4fb1) SHA1(5cff659b531d0f1b6faa503f7c06045c3a209a84) )

	ROM_REGION( 0x0200, "proms", 0 ) /* prom is a Fujitsu MB7052 or equivalent */
	ROM_LOAD( "bct1-1.l3", 0x0000, 0x0100, CRC(d17518bc) SHA1(f3b0deffa586808bc59e9a24ec1699c54ebe84cc) )
ROM_END

ROM_START( tankbattb ) /* board with "NAMCO" removed from gfx1 rom, otherwise identical to original */
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "tb1-1.1a",  0x0000, 0x0800, CRC(278a0b8c) SHA1(11ea8fe8401b3cd986616a30a759c0ac1a5ce73b) ) // a.1a
	ROM_LOAD( "tb1-2.1b",  0x0800, 0x0800, CRC(e0923370) SHA1(8d3dbea877bed9f9c267d8002dc180f6eb1e5a8f) ) // b.1b
	ROM_LOAD( "tb1-3.1c",  0x1000, 0x0800, CRC(85005ea4) SHA1(91583081803a5ef600fb90bee34be9edd87f157e) ) // c.1c
	ROM_LOAD( "tb1-4.1d",  0x1800, 0x0800, CRC(3dfb5bcf) SHA1(aa24bf74f4d5dc81baf3843196c837e0b731077b) ) // d.1d

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "e.2k",  0x0000, 0x0800, CRC(249f4e1b) SHA1(8654e8f9aa042ba49f20a58ff21879a593da57a3) )

	ROM_REGION( 0x0200, "proms", 0 ) /* prom is a Fujitsu MB7052 or equivalent */
	ROM_LOAD( "bct1-1.l3", 0x0000, 0x0100, CRC(d17518bc) SHA1(f3b0deffa586808bc59e9a24ec1699c54ebe84cc) ) // dm74s287n.3l
ROM_END

GAME( 1980, tankbatt,  0,        tankbatt, tankbatt, driver_device, 0, ROT90, "Namco",   "Tank Battalion", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tankbattb, tankbatt, tankbatt, tankbatt, driver_device, 0, ROT90, "bootleg", "Tank Battalion (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
