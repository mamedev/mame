// license:???
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Orbit Driver

  game 0 = beginner slow
  game 1 = beginner medium
  game 2 = beginner fast
  game 3 = intermediate slow
  game 4 = intermediate fast
  game 5 = expert fast shells only
  game 6 = expert slow
  game 7 = expert medium
  game 8 = expert fast
  game 9 = super expert

  Flip screen DIP doesn't work because it's not supported by the game.

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "includes/orbit.h"
#include "sound/discrete.h"


#define MASTER_CLOCK        XTAL_12_096MHz

/*************************************
 *
 *  Interrupts and timing
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(orbit_state::nmi_32v)
{
	int scanline = param;
	int nmistate = (scanline & 32) && (m_misc_flags & 4);
	m_maincpu->set_input_line(INPUT_LINE_NMI, nmistate ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(orbit_state::irq_off)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(orbit_state::orbit_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
	machine().scheduler().timer_set(m_screen->time_until_vblank_end(), timer_expired_delegate(FUNC(orbit_state::irq_off),this));
}



/*************************************
 *
 *  Bit flags
 *
 *************************************/

void orbit_state::update_misc_flags(address_space &space, UINT8 val)
{
	m_misc_flags = val;

	/* BIT0 => UNUSED       */
	/* BIT1 => LOCKOUT      */
	/* BIT2 => NMI ENABLE   */
	/* BIT3 => HEAT RST LED */
	/* BIT4 => PANEL BUS OC */
	/* BIT5 => PANEL STROBE */
	/* BIT6 => HYPER LED    */
	/* BIT7 => WARNING SND  */

	m_discrete->write(space, ORBIT_WARNING_EN, BIT(m_misc_flags, 7));

	output().set_led_value(0, BIT(m_misc_flags, 3));
	output().set_led_value(1, BIT(m_misc_flags, 6));

	machine().bookkeeping().coin_lockout_w(0, !BIT(m_misc_flags, 1));
	machine().bookkeeping().coin_lockout_w(1, !BIT(m_misc_flags, 1));
}


WRITE8_MEMBER(orbit_state::orbit_misc_w)
{
	UINT8 bit = offset >> 1;

	if (offset & 1)
		update_misc_flags(space, m_misc_flags | (1 << bit));
	else
		update_misc_flags(space, m_misc_flags & ~(1 << bit));
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( orbit_map, AS_PROGRAM, 8, orbit_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x0800) AM_MIRROR(0x07ff) AM_READ_PORT("P1")
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x07ff) AM_READ_PORT("P2")
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x07ff) AM_READ_PORT("DSW1")
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07ff) AM_READ_PORT("DSW2")
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x07ff) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x3000, 0x33bf) AM_MIRROR(0x0400) AM_RAM_WRITE(orbit_playfield_w) AM_SHARE("playfield_ram")
	AM_RANGE(0x33c0, 0x33ff) AM_MIRROR(0x0400) AM_RAM AM_SHARE("sprite_ram")
	AM_RANGE(0x3800, 0x3800) AM_MIRROR(0x00ff) AM_WRITE(orbit_note_w)
	AM_RANGE(0x3900, 0x3900) AM_MIRROR(0x00ff) AM_WRITE(orbit_noise_amp_w)
	AM_RANGE(0x3a00, 0x3a00) AM_MIRROR(0x00ff) AM_WRITE(orbit_note_amp_w)
	AM_RANGE(0x3c00, 0x3c0f) AM_MIRROR(0x00f0) AM_WRITE(orbit_misc_w)
	AM_RANGE(0x3e00, 0x3e00) AM_MIRROR(0x00ff) AM_WRITE(orbit_noise_rst_w)
	AM_RANGE(0x3f00, 0x3f00) AM_MIRROR(0x00ff) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( orbit )
	PORT_START("P1")    /* 0800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) /* actually buttons */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")    /* 1000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) /* actually buttons */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")  /* 1800 */
	PORT_DIPNAME( 0x07, 0x00, "Play Time Per Credit" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING( 0x00, "0:30" )
	PORT_DIPSETTING( 0x01, "1:00" )
	PORT_DIPSETTING( 0x02, "1:30" )
	PORT_DIPSETTING( 0x03, "2:00" )
	PORT_DIPSETTING( 0x04, "2:30" )
	PORT_DIPSETTING( 0x05, "3:00" )
	PORT_DIPSETTING( 0x06, "3:30" )
	PORT_DIPSETTING( 0x07, "4:00" )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_DIPSETTING( 0x08, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x10, DEF_STR( French ) )
	PORT_DIPSETTING( 0x18, DEF_STR( German ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Free_Play )) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x20, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "DSW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "DSW1:8" )

	PORT_START("DSW2")  /* 2000 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game Reset") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ))
	PORT_DIPSETTING( 0x00, DEF_STR( Off ))
	PORT_DIPSETTING( 0x08, DEF_STR( On ))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Heat Reset") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_DIPNAME( 0x20, 0x20, "NEXT TEST" ) /* should be off */
	PORT_DIPSETTING( 0x20, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x40, "DIAG TEST" ) /* should be off */
	PORT_DIPSETTING( 0x40, DEF_STR( Off ))
	PORT_DIPSETTING( 0x00, DEF_STR( On ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("BUTTONS")   /* 2800 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 7 / Strong Gravity") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 6 / Stars") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 5 / Unlimited Supplies") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 4 / Space Stations") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 3 / Black Hole") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 2 / Zero Gravity") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 1 / Negative Gravity") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Game 0 / Bounce Back") PORT_CODE(KEYCODE_0_PAD)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout orbit_full_sprite_layout =
{
	8, 32,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP32(0,8) },
	0x100
};


static const gfx_layout orbit_upper_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	0x100
};


static const gfx_layout orbit_lower_sprite_layout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0x80,8) },
	0x100
};


static const gfx_layout orbit_tile_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	0x40
};


static GFXDECODE_START( orbit )
	GFXDECODE_ENTRY( "gfx1", 0, orbit_full_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, orbit_upper_sprite_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0, orbit_lower_sprite_layout, 0, 1 )
	GFXDECODE_SCALE( "gfx2", 0, orbit_tile_layout, 0, 1, 2, 2 )
GFXDECODE_END



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void orbit_state::machine_start()
{
	save_item(NAME(m_misc_flags));
	save_item(NAME(m_flip_screen));
}

void orbit_state::machine_reset()
{
	update_misc_flags(generic_space(), 0);
	m_flip_screen = 0;
}


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( orbit, orbit_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, MASTER_CLOCK / 16)
	MCFG_CPU_PROGRAM_MAP(orbit_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", orbit_state,  orbit_interrupt)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("32v", orbit_state, nmi_32v, "screen", 0, 32)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK*2, 384*2, 0, 256*2, 261*2, 0, 240*2)
	MCFG_SCREEN_UPDATE_DRIVER(orbit_state, screen_update_orbit)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", orbit)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(orbit)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( orbit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "033701.h2", 0x6800, 0x400, CRC(6de43b85) SHA1(1643972f45d3a0dd6540158c575cd84cee2b0c9a) )
	ROM_LOAD_NIB_HIGH( "033693.l2", 0x6800, 0x400, CRC(8878409e) SHA1(a14e0161705bbc230f0aec1837ebc41d62178368) )
	ROM_LOAD_NIB_LOW ( "033702.h1", 0x6C00, 0x400, CRC(8166bdcb) SHA1(b7ae6cd46b4aff6e1e1ec9273cf068dec4a8cd46) )
	ROM_LOAD_NIB_HIGH( "033694.l1", 0x6C00, 0x400, CRC(5337a8ee) SHA1(1606bfa652bb5253c387f11c96d77d7a84983344) )
	ROM_LOAD_NIB_LOW ( "033699.f2", 0x7000, 0x400, CRC(b498b36f) SHA1(5d150af193196fccd7c20ba731a020a9ae75e516) )
	ROM_LOAD_NIB_HIGH( "033691.m2", 0x7000, 0x400, CRC(6cbabb21) SHA1(fffb3f7be73c72b4775d8cdfe174c75ae4389cba) )
	ROM_LOAD_NIB_LOW ( "033700.f1", 0x7400, 0x400, CRC(9807c922) SHA1(b6b62530b24d967104f632540ef98f2b4780c3ed) )
	ROM_LOAD_NIB_HIGH( "033692.m1", 0x7400, 0x400, CRC(96167d1b) SHA1(6f272b2f1b30aa94f51ea5710f4114bfdea19f2c) )
	ROM_LOAD_NIB_LOW ( "033697.e2", 0x7800, 0x400, CRC(19ccf0dc) SHA1(7d12c4985bd0a25ef518246faf2849e5a0cf600b) )
	ROM_LOAD_NIB_HIGH( "033689.n2", 0x7800, 0x400, CRC(ea3b70c1) SHA1(5e985fed057f362deaeb5e4049c4e8c1d449d6e1) )
	ROM_LOAD_NIB_LOW ( "033698.e1", 0x7C00, 0x400, CRC(356a7c32) SHA1(a3496c0f9d9f3e2e0b452cdc0e908dc93d179990) )
	ROM_RELOAD(                     0xFC00, 0x400 )
	ROM_LOAD_NIB_HIGH( "033690.n1", 0x7C00, 0x400, CRC(f756ebd4) SHA1(4e473541b712078c6a81901714a6243de348e543) )
	ROM_RELOAD(                     0xFC00, 0x400 )

	ROM_REGION( 0x1000, "gfx1", 0 )  /* sprites */
	ROM_LOAD( "033712.b7", 0x0000, 0x800, CRC(cfd43bf2) SHA1(dbca0da6ed355aac921bae5adeef2f384f5fa2c3) )
	ROM_LOAD( "033713.d7", 0x0800, 0x800, CRC(5ac89f4d) SHA1(747889b33cd83510a640e68fb4581a3e881c43a3) )

	ROM_REGION( 0x200, "gfx2", 0 )   /* tiles */
	ROM_LOAD( "033711.a7", 0x0000, 0x200, CRC(9987174a) SHA1(d2117b6e6d64c29aef8ad8c94256baea493bce5c) )

	ROM_REGION( 0x100, "proms", 0 )                  /* sync, unused */
	ROM_LOAD( "033688.p6", 0x0000, 0x100, CRC(ee66ddba) SHA1(5b9ae4cbf019375c8d54528b69280413c641c4f2) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1978, orbit, 0, orbit, orbit, driver_device, 0, 0, "Atari", "Orbit", MACHINE_SUPPORTS_SAVE )
