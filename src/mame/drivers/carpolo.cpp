// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Exidy Car Polo hardware

    driver by Zsolt Vasvari

    Games supported:
        * Car Polo

    Known issues:
        * sound

     Original Bugs:
        * if you insert a coin too fast (before the GAME OVER sign appears),
          the cars will chase *away* from the ball

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/74153.h"
#include "machine/6821pia.h"
#include "includes/carpolo.h"



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, carpolo_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x3000, 0x30ff) AM_WRITEONLY AM_SHARE("alpharam")
	AM_RANGE(0x4000, 0x400f) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x5400, 0x5403) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x5800, 0x5803) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(carpolo_ball_screen_collision_cause_r)
	AM_RANGE(0xa001, 0xa001) AM_READ(carpolo_car_ball_collision_x_r)
	AM_RANGE(0xa002, 0xa002) AM_READ(carpolo_car_ball_collision_y_r)
	AM_RANGE(0xa003, 0xa003) AM_READ(carpolo_car_car_collision_cause_r)
	AM_RANGE(0xa004, 0xa004) AM_READ(carpolo_car_border_collision_cause_r)
	AM_RANGE(0xa005, 0xa005) AM_READ(carpolo_car_ball_collision_cause_r)
	AM_RANGE(0xa006, 0xa006) AM_READ(carpolo_car_goal_collision_cause_r)
	AM_RANGE(0xa007, 0xa007) AM_READ_PORT("IN1")
	AM_RANGE(0xb000, 0xb000) AM_WRITE(carpolo_ball_screen_interrupt_clear_w)
	AM_RANGE(0xb001, 0xb001) AM_WRITE(carpolo_timer_interrupt_clear_w)
	AM_RANGE(0xb003, 0xb003) AM_WRITE(carpolo_car_car_interrupt_clear_w)
	AM_RANGE(0xb004, 0xb004) AM_WRITE(carpolo_car_border_interrupt_clear_w)
	AM_RANGE(0xb005, 0xb005) AM_WRITE(carpolo_car_ball_interrupt_clear_w)
	AM_RANGE(0xb006, 0xb006) AM_WRITE(carpolo_car_goal_interrupt_clear_w)
	AM_RANGE(0xc000, 0xc000) AM_READ(carpolo_interrupt_cause_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( carpolo )
	PORT_START("IN0")       /* IN0 */
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT (0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* the value read from here is used directly,
	   the result is calculated by 60/value */
	PORT_START("IN1")       /* IN1 */
	PORT_BIT (0x03, IP_ACTIVE_HIGH, IPT_UNUSED )    /* the lowest 2 bits of the counter */
	PORT_DIPNAME( 0xfc, 0x3c, "Game Sec/Real Sec" )
	PORT_DIPSETTING(    0x00, "0.23 (Longest)" )
	PORT_DIPSETTING(    0xfc, "0.24" )
	PORT_DIPSETTING(    0xf8, "0.24" )
	PORT_DIPSETTING(    0xf4, "0.25" )
	PORT_DIPSETTING(    0xf0, "0.25" )
	PORT_DIPSETTING(    0xec, "0.25" )
	PORT_DIPSETTING(    0xe8, "0.26" )
	PORT_DIPSETTING(    0xe4, "0.26" )
	PORT_DIPSETTING(    0xe0, "0.27" )
	PORT_DIPSETTING(    0xdc, "0.27" )
	PORT_DIPSETTING(    0xd8, "0.28" )
	PORT_DIPSETTING(    0xd4, "0.28" )
	PORT_DIPSETTING(    0xd0, "0.29" )
	PORT_DIPSETTING(    0xcc, "0.29" )
	PORT_DIPSETTING(    0xc8, "0.30" )
	PORT_DIPSETTING(    0xc4, "0.31" )
	PORT_DIPSETTING(    0xc0, "0.31" )
	PORT_DIPSETTING(    0xbc, "0.32" )
	PORT_DIPSETTING(    0xb8, "0.33" )
	PORT_DIPSETTING(    0xb4, "0.33" )
	PORT_DIPSETTING(    0xb0, "0.34" )
	PORT_DIPSETTING(    0xac, "0.35" )
	PORT_DIPSETTING(    0xa8, "0.36" )
	PORT_DIPSETTING(    0xa4, "0.37" )
	PORT_DIPSETTING(    0xa0, "0.38" )
	PORT_DIPSETTING(    0x9c, "0.38" )
	PORT_DIPSETTING(    0x98, "0.39" )
	PORT_DIPSETTING(    0x94, "0.41" )
	PORT_DIPSETTING(    0x90, "0.42" )
	PORT_DIPSETTING(    0x8c, "0.44" )
	PORT_DIPSETTING(    0x84, "0.46" )
	PORT_DIPSETTING(    0x80, "0.47" )
	PORT_DIPSETTING(    0x7c, "0.48" )
	PORT_DIPSETTING(    0x78, "0.50" )
	PORT_DIPSETTING(    0x74, "0.52" )
	PORT_DIPSETTING(    0x70, "0.54" )
	PORT_DIPSETTING(    0x6c, "0.56" )
	PORT_DIPSETTING(    0x68, "0.58" )
	PORT_DIPSETTING(    0x64, "0.60" )
	PORT_DIPSETTING(    0x60, "0.63" )
	PORT_DIPSETTING(    0x5c, "0.65" )
	PORT_DIPSETTING(    0x58, "0.68" )
	PORT_DIPSETTING(    0x54, "0.71" )
	PORT_DIPSETTING(    0x50, "0.75" )
	PORT_DIPSETTING(    0x4c, "0.79" )
	PORT_DIPSETTING(    0x48, "0.83" )
	PORT_DIPSETTING(    0x44, "0.88" )
	PORT_DIPSETTING(    0x40, "0.94" )
	PORT_DIPSETTING(    0x3c, "1.00" )
	PORT_DIPSETTING(    0x38, "1.07" )
	PORT_DIPSETTING(    0x34, "1.15" )
	PORT_DIPSETTING(    0x30, "1.25" )
	PORT_DIPSETTING(    0x2c, "1.36" )
	PORT_DIPSETTING(    0x28, "1.50" )
	PORT_DIPSETTING(    0x24, "1.67" )
	PORT_DIPSETTING(    0x20, "1.88" )
	PORT_DIPSETTING(    0x1c, "2.14" )
	PORT_DIPSETTING(    0x18, "2.50" )
	PORT_DIPSETTING(    0x14, "3.00" )
	PORT_DIPSETTING(    0x10, "3.75" )
	PORT_DIPSETTING(    0x0c, "5.00" )
	PORT_DIPSETTING(    0x08, "7.50" )
	PORT_DIPSETTING(    0x04, "15.00 (Shortest)" )

	PORT_START("DIAL0")     /* IN2 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("DIAL1")     /* IN3 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("DIAL2")     /* IN4 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(3)

	PORT_START("DIAL3")     /* IN5 */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(4)

	PORT_START("PEDALS")    /* IN6 - accelerator pedals */
	PORT_BIT (0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT (0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT (0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT (0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT (0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT (0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)

	PORT_START("IN2")       /* IN7 - forward/reverse */
	PORT_BIT (0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	1,
	{ 0 },
	{ STEP4(3*0x100*8+4,1),
		STEP4(2*0x100*8+4,1),
		STEP4(1*0x100*8+4,1),
		STEP4(0*0x100*8+4,1) },
	{ STEP16(0,8) },
	16*8
};

static const gfx_layout goallayout =
{
	8,32,
	1,
	4,
	{ 4, 5, 6, 7 },
	{ STEP8(0,8) },
	{ STEP8(192*8,8*8),
		STEP16( 0*8,8*8),
		STEP8(128*8,8*8) },
	0
};

static const gfx_layout alphalayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( carpolo )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,         12 )
	GFXDECODE_ENTRY( "gfx2", 0, goallayout,   12*2,      2 )
	GFXDECODE_ENTRY( "gfx3", 0, alphalayout,  12*2+2*16, 4 )
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( carpolo, carpolo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_11_289MHz/12)       /* 940.75 kHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", carpolo_state,  carpolo_timer_interrupt)   /* this not strictly VBLANK,
                                                       but it's supposed to happen 60
                                                       times a sec, so it's a good place */

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(carpolo_state, pia_0_port_b_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(carpolo_state, pia_0_port_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(carpolo_state, pia_0_port_b_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(carpolo_state, coin1_interrupt_clear_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(carpolo_state,coin2_interrupt_clear_w))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(carpolo_state, pia_1_port_a_r))
	MCFG_PIA_READPB_HANDLER(READ8(carpolo_state, pia_1_port_b_r))
	MCFG_PIA_CA2_HANDLER(WRITELINE(carpolo_state, coin3_interrupt_clear_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(carpolo_state, coin4_interrupt_clear_w))

	MCFG_DEVICE_ADD("7474_2s_1", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(carpolo_state, carpolo_7474_2s_1_q_cb))

	MCFG_DEVICE_ADD("7474_2s_2", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(carpolo_state, carpolo_7474_2s_2_q_cb))

	MCFG_DEVICE_ADD("7474_2u_1", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(carpolo_state, carpolo_7474_2u_1_q_cb))

	MCFG_DEVICE_ADD("7474_2u_2", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(WRITELINE(carpolo_state, carpolo_7474_2u_2_q_cb))

	MCFG_DEVICE_ADD("7474_1f_1", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1f_2", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1d_1", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1d_2", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1c_1", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1c_2", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1a_1", TTL7474, 0)
	MCFG_DEVICE_ADD("7474_1a_2", TTL7474, 0)

	MCFG_DEVICE_ADD("74148_3s", TTL74148, 0)
	MCFG_74148_OUTPUT_CB(carpolo_state, ttl74148_3s_cb)

	MCFG_DEVICE_ADD("74153_1k", TTL74153, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 255)
	MCFG_SCREEN_UPDATE_DRIVER(carpolo_state, screen_update_carpolo)
	MCFG_SCREEN_VBLANK_DRIVER(carpolo_state, screen_eof_carpolo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", carpolo)
	MCFG_PALETTE_ADD("palette", 12*2+2*16+4*2)
	MCFG_PALETTE_INIT_OWNER(carpolo_state,carpolo)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( carpolo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4000.6c",   0xf000, 0x0200, CRC(9d2e75a5) SHA1(c249d0b31de452738516f04a7bc3fb472d54f79d) )
	ROM_LOAD( "4001.6d",   0xf200, 0x0200, CRC(69fb3768) SHA1(5fcc0807e560de0d73f8bab6943f3cad5ee324c9) )
	ROM_LOAD( "4002.6h",   0xf400, 0x0200, CRC(5db179c7) SHA1(83615cdc1e3d8930cbdafbd0d327e1d6611faefd) )
	ROM_LOAD( "4003.6k",   0xf600, 0x0200, CRC(0f992f09) SHA1(dfb8d96b94850166a1b5332f200ae9ceeffc1ce6) )
	ROM_LOAD( "4004.6e",   0xf800, 0x0200, CRC(be731610) SHA1(70b035f75dd6eace94b4d7a8a344f136b8af743d) )
	ROM_LOAD( "4005.6f",   0xfa00, 0x0200, CRC(7332f84f) SHA1(be548329918c4ac512fd6027d6bcd16d20c9dd98) )
	ROM_LOAD( "4006.6l",   0xfc00, 0x0200, CRC(8479c350) SHA1(391c737947498aad4d478639cbbc72181d680fce) )
	ROM_LOAD( "4007a.6n",  0xfe00, 0x0200, CRC(c6a619de) SHA1(c1b650a0126791fa733f89d9e9bdeeb605486a2c) )

	ROM_REGION( 0x0400, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "1024.10w",  0x0000, 0x0100, CRC(eedacc7e) SHA1(d89628f013039ca387cafe22180de71e1553cffc) )
	ROM_LOAD( "1023.10v",  0x0100, 0x0100, CRC(45df6c74) SHA1(a986b62b4c263c5d217bae0d51e74197f5288180) )
	ROM_LOAD( "1022.10u",  0x0200, 0x0100, CRC(00868768) SHA1(2388e428db300a1e0005cccb9165ec604518033d) )
	ROM_LOAD( "1021.10t",  0x0300, 0x0100, CRC(a508af9c) SHA1(219ba776d8cccf6726519aff17e37f2a6a85d0d1) )

	ROM_REGION( 0x0100, "gfx2", 0 ) /* goal */
	ROM_LOAD( "1020.6v",   0x0000, 0x0100, CRC(5e89fbcd) SHA1(6be171168924cd8aa94ff5e1994faecb6f303bd9) )

	ROM_REGION( 0x0200, "gfx3", 0 ) /* alpha */
	ROM_LOAD( "2513.4l",   0x0000, 0x0200, BAD_DUMP CRC(f80d8889) SHA1(ca573543dcce1221459d5693c476cef14bfac4f4)  ) /* MIA - stolen from Night Driver */

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "328.5u",    0x0000, 0x0020, CRC(f295e0fc) SHA1(974a0481e0c6d5c0b6f0129653d8ed87880916e0) )       /* color PROM */
	ROM_LOAD( "325.6t",    0x0020, 0x0020, CRC(b8b44022) SHA1(29fe6159c8d239c322296cef68ad59bcf290f246) )       /* horizontal timing */
	ROM_LOAD( "326.6w",    0x0040, 0x0020, CRC(628ae3d1) SHA1(e6d43d2b5e8ec4b8c1adf6f29c2c9a43ab67ff50) )       /* vertical timing */

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "327.10s",   0x0000, 0x0020, CRC(e047d24d) SHA1(2ea7afc8d97c906295bf2af929e0515f6c34137f) )       /* sprite image map */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

DRIVER_INIT_MEMBER(carpolo_state,carpolo)
{
	size_t i, len;
	UINT8 *ROM;


	/* invert gfx PROM since the bits are active LO */
	ROM = memregion("gfx2")->base();
	len = memregion("gfx2")->bytes();
	for (i = 0;i < len; i++)
		ROM[i] ^= 0x0f;
}

GAME( 1977, carpolo, 0, carpolo, carpolo, carpolo_state, carpolo, ROT0, "Exidy", "Car Polo", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
