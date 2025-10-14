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
#include "carpolo.h"

#include "machine/74153.h"
#include "machine/6821pia.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void carpolo_state::main_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x3000, 0x30ff).writeonly().share(m_alpharam);
	map(0x4000, 0x400f).writeonly().share(m_spriteram);
	map(0x5400, 0x5403).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5800, 0x5803).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xa000, 0xa000).r(FUNC(carpolo_state::ball_screen_collision_cause_r));
	map(0xa001, 0xa001).r(FUNC(carpolo_state::car_ball_collision_x_r));
	map(0xa002, 0xa002).r(FUNC(carpolo_state::car_ball_collision_y_r));
	map(0xa003, 0xa003).r(FUNC(carpolo_state::car_car_collision_cause_r));
	map(0xa004, 0xa004).r(FUNC(carpolo_state::car_border_collision_cause_r));
	map(0xa005, 0xa005).r(FUNC(carpolo_state::car_ball_collision_cause_r));
	map(0xa006, 0xa006).r(FUNC(carpolo_state::car_goal_collision_cause_r));
	map(0xa007, 0xa007).portr("IN1");
	map(0xb000, 0xb000).w(FUNC(carpolo_state::ball_screen_interrupt_clear_w));
	map(0xb001, 0xb001).w(FUNC(carpolo_state::timer_interrupt_clear_w));
	map(0xb003, 0xb003).w(FUNC(carpolo_state::car_car_interrupt_clear_w));
	map(0xb004, 0xb004).w(FUNC(carpolo_state::car_border_interrupt_clear_w));
	map(0xb005, 0xb005).w(FUNC(carpolo_state::car_ball_interrupt_clear_w));
	map(0xb006, 0xb006).w(FUNC(carpolo_state::car_goal_interrupt_clear_w));
	map(0xc000, 0xc000).r(FUNC(carpolo_state::interrupt_cause_r));
	map(0xf000, 0xffff).rom();
}



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

static GFXDECODE_START( gfx_carpolo )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0,         12 )
	GFXDECODE_ENTRY( "gfx2", 0, goallayout,   12*2,      2 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x1,    12*2+2*16, 4 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void carpolo_state::carpolo(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 11.289_MHz_XTAL / 12); // 940.75 kHz
	m_maincpu->set_addrmap(AS_PROGRAM, &carpolo_state::main_map);

	pia6821_device &pia0(PIA6821(config, "pia0", 11.289_MHz_XTAL / 12));
	pia0.readpb_handler().set(FUNC(carpolo_state::pia_0_port_b_r));
	pia0.writepa_handler().set(FUNC(carpolo_state::pia_0_port_a_w));
	pia0.writepb_handler().set(FUNC(carpolo_state::pia_0_port_b_w));
	pia0.ca2_handler().set(FUNC(carpolo_state::coin1_interrupt_clear_w));
	pia0.cb2_handler().set(FUNC(carpolo_state::coin2_interrupt_clear_w));

	pia6821_device &pia1(PIA6821(config, "pia1", 11.289_MHz_XTAL / 12));
	pia1.readpa_handler().set(FUNC(carpolo_state::pia_1_port_a_r));
	pia1.readpb_handler().set(FUNC(carpolo_state::pia_1_port_b_r));
	pia1.ca2_handler().set(FUNC(carpolo_state::coin3_interrupt_clear_w));
	pia1.cb2_handler().set(FUNC(carpolo_state::coin4_interrupt_clear_w));

	TTL7474(config, m_ttl7474_2s_1, 0);
	m_ttl7474_2s_1->comp_output_cb().set(FUNC(carpolo_state::ttl7474_2s_1_q_cb));

	TTL7474(config, m_ttl7474_2s_2, 0);
	m_ttl7474_2s_2->comp_output_cb().set(FUNC(carpolo_state::ttl7474_2s_2_q_cb));

	TTL7474(config, m_ttl7474_2u_1, 0);
	m_ttl7474_2u_1->comp_output_cb().set(FUNC(carpolo_state::ttl7474_2u_1_q_cb));

	TTL7474(config, m_ttl7474_2u_2, 0);
	m_ttl7474_2u_2->comp_output_cb().set(FUNC(carpolo_state::ttl7474_2u_2_q_cb));

	TTL7474(config, m_ttl7474_1f_1, 0);
	TTL7474(config, m_ttl7474_1f_2, 0);
	TTL7474(config, m_ttl7474_1d_1, 0);
	TTL7474(config, m_ttl7474_1d_2, 0);
	TTL7474(config, m_ttl7474_1c_1, 0);
	TTL7474(config, m_ttl7474_1c_2, 0);
	TTL7474(config, m_ttl7474_1a_1, 0);
	TTL7474(config, m_ttl7474_1a_2, 0);

	TTL74148(config, m_ttl74148_3s, 0);
	m_ttl74148_3s->out_cb().set(FUNC(carpolo_state::ttl74148_3s_cb));

	TTL153(config, m_ttl74153_1k);
	m_ttl74153_1k->za_cb().set(FUNC(carpolo_state::ls153_za_w)); // pia1 pb5
	m_ttl74153_1k->zb_cb().set(FUNC(carpolo_state::ls153_zb_w)); // pia1 pb4

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(11.289_MHz_XTAL / 2, 336, 0, 240, 280, 0, 256);
	screen.set_screen_update(FUNC(carpolo_state::screen_update));
	screen.screen_vblank().set(FUNC(carpolo_state::screen_vblank));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_carpolo);
	PALETTE(config, m_palette, FUNC(carpolo_state::carpolo_palette), 12*2+2*16+4*2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(carpolo))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:player_crash1", "PL1_CRASH.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:player_crash2", "PL2_CRASH.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:player_crash3", "PL3_CRASH.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:player_crash4", "PL4_CRASH.IN", 0);

	// Temporarily just tied to an arbitrary value to preserve lack of audio.
	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "PL1_CRASH.GND").set_mult_offset(10000.0 / 32768.0, 0.0);
}



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

	ROM_REGION( 0x0400, "gfx1", 0 ) // sprites
	ROM_LOAD( "1024.10w",  0x0000, 0x0100, CRC(eedacc7e) SHA1(d89628f013039ca387cafe22180de71e1553cffc) )
	ROM_LOAD( "1023.10v",  0x0100, 0x0100, CRC(45df6c74) SHA1(a986b62b4c263c5d217bae0d51e74197f5288180) )
	ROM_LOAD( "1022.10u",  0x0200, 0x0100, CRC(00868768) SHA1(2388e428db300a1e0005cccb9165ec604518033d) )
	ROM_LOAD( "1021.10t",  0x0300, 0x0100, CRC(a508af9c) SHA1(219ba776d8cccf6726519aff17e37f2a6a85d0d1) )

	ROM_REGION( 0x0100, "gfx2", ROMREGION_INVERT ) // goal
	ROM_LOAD( "1020.6v",   0x0000, 0x0100, CRC(5e89fbcd) SHA1(6be171168924cd8aa94ff5e1994faecb6f303bd9) )

	ROM_REGION( 0x0200, "gfx3", 0 ) // alpha
	ROM_LOAD( "2513.4l",   0x0000, 0x0200, CRC(2b41dd66) SHA1(8417b51ba2c4ba9989ef6a04a63720ce36e4660f) BAD_DUMP ) // MIA - stolen from Circus and changed the 0

	ROM_REGION( 0x0060, "proms", 0 )
	ROM_LOAD( "328.5u",    0x0000, 0x0020, CRC(f295e0fc) SHA1(974a0481e0c6d5c0b6f0129653d8ed87880916e0) ) // color PROM
	ROM_LOAD( "325.6t",    0x0020, 0x0020, CRC(b8b44022) SHA1(29fe6159c8d239c322296cef68ad59bcf290f246) ) // horizontal timing
	ROM_LOAD( "326.6w",    0x0040, 0x0020, CRC(628ae3d1) SHA1(e6d43d2b5e8ec4b8c1adf6f29c2c9a43ab67ff50) ) // vertical timing

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "327.10s",   0x0000, 0x0020, CRC(e047d24d) SHA1(2ea7afc8d97c906295bf2af929e0515f6c34137f) ) // sprite image map
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1977, carpolo, 0, carpolo, carpolo, carpolo_state, empty_init, ROT0, "Exidy", "Car Polo", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND )
