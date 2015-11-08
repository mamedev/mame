// license:???
// copyright-holders:insideoutboy, Nicola Salmoria
/***************************************************************************

    Car Jamboree
    Omori Electric CAD (OEC) 1983

    c14                c.d19
    c13                c.d18           c10
    c12                                c9
    c11         2125   2125
                2125   2125
                2125   2125  2114 2114
                2125   2125  2114 2114
                2125   2125            c8
                2125   2125            c7
                                       c6
                                       c5
                                       c4
                                       c3
    5101                               c2
    5101                               c1
                                       6116
    18.432MHz
              6116
    Z80A      c15
                                       Z80A
           8910         SW
           8910

Notes:

- some sprite glitches from sprite number/colour changes happening on
  different frames, possibly original behaviour. eg cars changing colour
  just before exploding, animals displaying as the wrong sprite for one
  frame when entering the arena

- colours look wrong, maybe address bitswap?

- background colour calculation is a guess

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "includes/carjmbre.h"

/*************************************
 *
 *  Memory maps
 *
 *************************************/

WRITE8_MEMBER(carjmbre_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}


static ADDRESS_MAP_START( carjmbre_map, AS_PROGRAM, 8, carjmbre_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
//  AM_RANGE(0x8800, 0x8800) AM_READNOP         // watchdog?
	AM_RANGE(0x8803, 0x8803) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x8805, 0x8805) AM_WRITE(carjmbre_bgcolor_w)   // guessed
	AM_RANGE(0x8806, 0x8806) AM_WRITE(carjmbre_8806_w)      // video related?
	AM_RANGE(0x8807, 0x8807) AM_WRITE(carjmbre_flipscreen_w)
//  AM_RANGE(0x8fc1, 0x8fc1) AM_WRITENOP        // overrun during initial screen clear
//  AM_RANGE(0x8fe1, 0x8fe1) AM_WRITENOP        // overrun during initial screen clear
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(carjmbre_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9800, 0x985f) AM_MIRROR(0x80) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("P1")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("P2")
	AM_RANGE(0xb800, 0xb800) AM_READ_PORT("DSW") AM_WRITE(soundlatch_byte_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( carjmbre_sound_map, AS_PROGRAM, 8, carjmbre_state )
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x1000) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( carjmbre_sound_io_map, AS_IO, 8, carjmbre_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x10, 0x10) AM_WRITENOP            //?? written on init/0xff sound command reset
	AM_RANGE(0x20, 0x21) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x22, 0x22) AM_WRITENOP            //?? written before and after 0x21 with same value
	AM_RANGE(0x24, 0x24) AM_READNOP             //??
	AM_RANGE(0x30, 0x31) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x32, 0x32) AM_WRITENOP            //?? written before and after 0x31 with same value
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( carjmbre )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )     //coin error if held high for 1s
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )     //or if many coins inserted quickly
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "Free") // 250 (cheat)
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "10k, then every 100k" )
	PORT_DIPSETTING(    0x20, "20k, then every 100k" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout carjmbre_charlayout =
{
	8,8,
	RGN_FRAC(2,4),
	2,
	{ RGN_FRAC(0,4), RGN_FRAC(2,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout carjmbre_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	2,
	{ RGN_FRAC(2,4), RGN_FRAC(0,4) },
	{ STEP8(0,1), STEP8(256*16*8,1) },
	{ STEP16(0,8) },
	16*8
};

static GFXDECODE_START( carjmbre )
	GFXDECODE_ENTRY( "gfx1", 0, carjmbre_charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, carjmbre_spritelayout, 0, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void carjmbre_state::machine_reset()
{
	m_flipscreen = 0;
	m_bgcolor = 0;
}

INTERRUPT_GEN_MEMBER(carjmbre_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( carjmbre, carjmbre_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/6)
	MCFG_CPU_PROGRAM_MAP(carjmbre_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", carjmbre_state,  vblank_irq)


	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18_432MHz/6/2)
	MCFG_CPU_PROGRAM_MAP(carjmbre_sound_map)
	MCFG_CPU_IO_MAP(carjmbre_sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", carjmbre_state,  irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(carjmbre_state, screen_update_carjmbre)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", carjmbre)
	MCFG_PALETTE_ADD("palette", 64)
	MCFG_PALETTE_INIT_OWNER(carjmbre_state, carjmbre)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18_432MHz/6/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18_432MHz/6/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.12)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( carjmbre )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c1",      0x0000, 0x1000, CRC(62b21739) SHA1(710e5c52f27603aa8f864f6f28d7272f21271d60) )
	ROM_LOAD( "c2",      0x1000, 0x1000, CRC(9ab1a0fa) SHA1(519cf67b98e62b2b42232788ba01ab6637880afc) )
	ROM_LOAD( "c3",      0x2000, 0x1000, CRC(bb29e100) SHA1(93e3cfcf7f8b0b36327f402d9a64c04c3b2c7549) )
	ROM_LOAD( "c4",      0x3000, 0x1000, CRC(c63d8f97) SHA1(9f08fd1cd24a1fb4011864c06580985e009d9af4) )
	ROM_LOAD( "c5",      0x4000, 0x1000, CRC(4d593942) SHA1(30cc649a4be3d7f3705f55d8d0dadb0b63d59ec9) )
	ROM_LOAD( "c6",      0x5000, 0x1000, CRC(fb576963) SHA1(5bf5c54a7c12aa55272629c12b414bf49cda0f1f) )
	ROM_LOAD( "c7",      0x6000, 0x1000, CRC(2b8c4511) SHA1(428a48d6b14455d66720a115bc5f35293dc50de7) )
	ROM_LOAD( "c8",      0x7000, 0x1000, CRC(51cc22a7) SHA1(f614368bfee04f084c70bf145801ac46e5631acb) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "c15",     0x0000, 0x1000, CRC(7d7779d1) SHA1(f8f5246be4cc9632076d3330fc3d3343b911dfee) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c9",     0x0000, 0x1000, CRC(2accb821) SHA1(ce2804536fc1abd3377dc864c8c9976ca28c1b6e) )
	ROM_LOAD( "c10",    0x1000, 0x1000, CRC(75ddbe56) SHA1(5e1363967a822265618793ccb74bf3ef5e0e00b5) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c11",    0x0000, 0x1000, CRC(d90cd126) SHA1(7ee110cf19b45ee654016ba0ce92f3db6ea2ed92) )
	ROM_LOAD( "c12",    0x1000, 0x1000, CRC(b3bb39d7) SHA1(89c901be6fae2356ce4d2653e94bf28d6bcf41fe) )
	ROM_LOAD( "c13",    0x2000, 0x1000, CRC(3004010b) SHA1(00d5d2185014159112eb90d8ed50092a3b4ab664) )
	ROM_LOAD( "c14",    0x3000, 0x1000, CRC(fb5f0d31) SHA1(7a27af91efc836bb48c6ed3b283b7c5f7b31c4b5) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "c.d19",  0x0000, 0x0020, CRC(220bceeb) SHA1(46b9f867d014596e2aa7503f104dc721965f0ed5) )
	ROM_LOAD( "c.d18",  0x0020, 0x0020, CRC(7b9ed1b0) SHA1(ec5e1f56e5a2fc726083866c08ac0e1de0ed6ace) )
ROM_END

/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1983, carjmbre, 0, carjmbre, carjmbre, driver_device, 0, ROT90, "Omori Electric Co., Ltd.", "Car Jamboree", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
