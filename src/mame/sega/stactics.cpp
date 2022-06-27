// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo
/****************************************************************************

    Sega "Space Tactics" Driver

    Frank Palazzolo (palazzol@home.com)

Master processor - Intel 8080A

Memory Map:

0000-2fff ROM                       R
4000-47ff RAM                       R/W
5000-5fff switches/status           R
6000-6fff dips sw                   R
6000-600f Coin rjct/palette select  W
6010-601f sound triggers            W
6020-602f lamp latch                W
6030-603f speed latch               W
6040-605f shot related              W
6060-606f score display             W
60a0-60e0 sound triggers2           W
7000-7fff RNG/swit                  R     LS Nibble are a VBlank counter
                                          used as a RNG
8000-8fff swit/stat                 R
8000-8fff offset RAM                W
9000-9fff V pos reg.                R     Reads counter from an encoder wheel
a000-afff H pos reg.                R     Reads counter from an encoder wheel
b000-bfff VRAM B                    R/W   alphanumerics, bases, barrier,
                                          enemy bombs
d000-dfff VRAM D                    R/W   furthest aliens (scrolling)
e000-efff VRAM E                    R/W   middle aliens (scrolling)
f000-ffff VRAM F                    R/W   closest aliens (scrolling)

--------------------------------------------------------------------

At this time, emulation is missing:

Sound (all discrete and a 76477)
Verify Color PROM resistor values (Last 8 colors)

****************************************************************************/

#include "emu.h"
#include "stactics.h"
#include "stactics.lh"

#include "cpu/i8085/i8085.h"



/*************************************
 *
 *  Mirror motor handling
 *
 *************************************/

WRITE_LINE_MEMBER(stactics_state::motor_w)
{
	m_motor_on = state;
}


READ_LINE_MEMBER(stactics_state::motor_not_ready_r)
{
	// if the motor is self-centering, but not centered yet
	return (!m_motor_on && (m_horiz_pos != 0 || m_vert_pos != 0));
}


uint8_t stactics_state::vert_pos_r()
{
	return 0x70 - m_vert_pos;
}


uint8_t stactics_state::horiz_pos_r()
{
	return m_horiz_pos + 0x88;
}


void stactics_state::move_motor()
{
	// monitor motor under joystick control
	if (m_motor_on)
	{
		const int in3 = m_in3->read();
		const int in4 = m_fake->read();

		/* up */
		if (!(in4 & 0x01) && m_vert_pos > -128)
			m_vert_pos--;

		/* down */
		if (!(in4 & 0x02) && m_vert_pos < 127)
			m_vert_pos++;

		/* left */
		if (!(in3 & 0x20)  && m_horiz_pos < 127)
			m_horiz_pos++;

		/* right */
		if (!(in3 & 0x40) && m_horiz_pos > -128)
			m_horiz_pos--;
	}

		/* monitor motor under self-centering control */
	else
	{
		if (m_horiz_pos > 0)
			m_horiz_pos--;
		else if (m_horiz_pos < 0)
			m_horiz_pos++;

		if (m_vert_pos > 0)
			m_vert_pos--;
		else if (m_vert_pos < 0)
			m_vert_pos++;
	}
}



/*************************************
 *
 *  Random number generator
 *
 *************************************/

CUSTOM_INPUT_MEMBER(stactics_state::get_rng)
{
	/* this is a 555 timer, but cannot read one of the resistor values */
	return machine().rand() & 0x07;
}



/*************************************
 *
 *  Coin lockout
 *
 *************************************/

WRITE_LINE_MEMBER(stactics_state::coin_lockout_1_w)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}


WRITE_LINE_MEMBER(stactics_state::coin_lockout_2_w)
{
	machine().bookkeeping().coin_lockout_w(1, !state);
}



/*************************************
 *
 *  Interrupt system
 *
 *************************************/

INTERRUPT_GEN_MEMBER(stactics_state::interrupt)
{
	move_motor();

	device.execute().set_input_line(0, HOLD_LINE);
}



/*************************************
 *
 *  Data CPU memory handlers
 *
 *************************************/

void stactics_state::main_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x40ff).mirror(0x0700).ram();
	map(0x5000, 0x5000).mirror(0x0fff).portr("IN0");
	map(0x6000, 0x6000).mirror(0x0fff).portr("IN1");
	map(0x6000, 0x6007).mirror(0x0f08).w(m_outlatch, FUNC(ls259_device::write_d0));
	map(0x6010, 0x6017).mirror(0x0f08).w("audiolatch", FUNC(ls259_device::write_d0));
	map(0x6020, 0x6027).mirror(0x0f08).w("lamplatch", FUNC(ls259_device::write_d0));
	map(0x6030, 0x6030).mirror(0x0f0f).w(FUNC(stactics_state::speed_latch_w));
	map(0x6040, 0x6040).mirror(0x0f0f).w(FUNC(stactics_state::shot_trigger_w));
	map(0x6050, 0x6050).mirror(0x0f0f).w(FUNC(stactics_state::shot_flag_clear_w));
	map(0x6060, 0x606f).mirror(0x0f00).writeonly().share("display_buffer");
	map(0x6070, 0x609f).mirror(0x0f00).nopw();
	/* map(0x60a0, 0x60ef).mirror(0x0f00).w(FUNC(stactics_state::sound2_w)); */
	map(0x60f0, 0x60ff).mirror(0x0f00).nopw();
	map(0x7000, 0x7000).mirror(0x0fff).portr("IN2");
	map(0x8000, 0x8000).mirror(0x0fff).portr("IN3");
	map(0x8000, 0x87ff).mirror(0x0800).w(FUNC(stactics_state::scroll_ram_w));
	map(0x9000, 0x9000).mirror(0x0fff).r(FUNC(stactics_state::vert_pos_r));
	map(0xa000, 0xa000).mirror(0x0fff).r(FUNC(stactics_state::horiz_pos_r));
	map(0xb000, 0xbfff).ram().share("videoram_b");
	map(0xc000, 0xcfff).noprw();
	map(0xd000, 0xdfff).ram().share("videoram_d");
	map(0xe000, 0xefff).ram().share("videoram_e");
	map(0xf000, 0xffff).ram().share("videoram_f");
}



/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( stactics )
	PORT_START("IN0")   /*  IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, motor_not_ready_r)

	PORT_START("IN1")   /* IN1 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x00, "High Score Initial Entry" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")   /* IN2 */
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(stactics_state, get_rng)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, frame_count_d3_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN3")   /* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, shot_standby_r)
	PORT_DIPNAME( 0x04, 0x04, "Number of Barriers" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Barriers" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(stactics_state, not_shot_arrive_r)

	PORT_START("FAKE")  /* FAKE */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
INPUT_PORTS_END



/*************************************
 *
 *  Start
 *
 *************************************/

void stactics_state::machine_start()
{
	m_vert_pos = 0;
	m_horiz_pos = 0;
	m_motor_on = false;

	save_item(NAME(m_vert_pos));
	save_item(NAME(m_horiz_pos));
	save_item(NAME(m_motor_on));
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void stactics_state::stactics(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 15.46848_MHz_XTAL / 8); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &stactics_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(stactics_state::interrupt));

	LS259(config, m_outlatch); // 50
	m_outlatch->q_out_cb<0>().set(FUNC(stactics_state::coin_lockout_1_w)); // COIN REJECT 1
	m_outlatch->q_out_cb<1>().set(FUNC(stactics_state::coin_lockout_2_w)); // COIN REJECT 2
	m_outlatch->q_out_cb<6>().set(FUNC(stactics_state::palette_bank_w)); // FLM COL 0
	m_outlatch->q_out_cb<7>().set(FUNC(stactics_state::palette_bank_w)); // FLM COL 1

	ls259_device &audiolatch(LS259(config, "audiolatch")); // 58 - TODO: implement these switches
	audiolatch.q_out_cb<0>().set_nop(); // MUTE
	audiolatch.q_out_cb<1>().set_nop(); // INV. DISTANCE A
	audiolatch.q_out_cb<2>().set_nop(); // INV. DISTANCE B
	audiolatch.q_out_cb<3>().set_nop(); // UFO
	audiolatch.q_out_cb<4>().set_nop(); // INVADER
	audiolatch.q_out_cb<5>().set_nop(); // EMEGENCY (sic)
	audiolatch.q_out_cb<6>().set(FUNC(stactics_state::motor_w)); // overlaps rocket sound
	audiolatch.q_out_cb<7>().set_nop(); // SOUND ON

	ls259_device &lamplatch(LS259(config, "lamplatch")); // 96
	lamplatch.q_out_cb<0>().set(FUNC(stactics_state::base_lamp_w<4>));
	lamplatch.q_out_cb<1>().set(FUNC(stactics_state::base_lamp_w<3>));
	lamplatch.q_out_cb<2>().set(FUNC(stactics_state::base_lamp_w<2>));
	lamplatch.q_out_cb<3>().set(FUNC(stactics_state::base_lamp_w<1>));
	lamplatch.q_out_cb<4>().set(FUNC(stactics_state::base_lamp_w<0>));
	lamplatch.q_out_cb<5>().set(FUNC(stactics_state::start_lamp_w));
	lamplatch.q_out_cb<6>().set(FUNC(stactics_state::barrier_lamp_w));

	/* video hardware */
	stactics_video(config);

	/* audio hardware */
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( stactics )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-218x",     0x0000, 0x0800, CRC(b1186ad2) SHA1(88929a183ac0499619b3e07241f3b5a0c89bdab1) )
	ROM_LOAD( "epr-219x",     0x0800, 0x0800, CRC(3b86036d) SHA1(6ad5e14dcfdbc6d2a0a32ae7f18ce41ab4b51eec) )
	ROM_LOAD( "epr-220x",     0x1000, 0x0800, CRC(c58702da) SHA1(93936c46810722d435f9ddb0641defb741743dee) )
	ROM_LOAD( "epr-221x",     0x1800, 0x0800, CRC(e327639e) SHA1(024929b65c71eaeb6d234a14d7535a7d5b98b8d3) )
	ROM_LOAD( "epr-222y",     0x2000, 0x0800, CRC(24dd2bcc) SHA1(f77c59beccc1a77e3bfc2928ff532d6e221ff42d) )
	ROM_LOAD( "epr-223x",     0x2800, 0x0800, CRC(7fef0940) SHA1(5b2af55f75ef0130f9202b6a916a96dbd601fcfa) )

	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pr54",         0x0000, 0x0800, CRC(9640bd6e) SHA1(dd12952a6591f2056ac1b5688dca0a3a2ef69f2d) )      /* color/priority PROM */
	ROM_LOAD( "pr55",         0x0800, 0x0800, CRC(f162673b) SHA1(83743780b6c1f8014df24fa0650000b7cb137d92) )      /* timing PROM (unused)    */
	ROM_LOAD( "pr65",         0x1000, 0x0020, CRC(a1506b9d) SHA1(037c3db2ea40eca459e8acba9d1506dd28d72d10) )      /* timing PROM (unused)    */
	ROM_LOAD( "pr66",         0x1020, 0x0020, CRC(78dcf300) SHA1(37034cc0cfa4a8ec47937a2a34b77ec56b387a9b) )      /* timing PROM (unused)    */

	ROM_REGION( 0x0820, "user1", 0 )
	ROM_LOAD( "epr-217",      0x0000, 0x0800, CRC(38259f5f) SHA1(1f4182ffc2d78fca22711526bb2ae2cfe040173c) )      /* LED fire beam data      */
	ROM_LOAD( "pr67",         0x0800, 0x0020, CRC(b27874e7) SHA1(c24bc78c4b2ae01aaed5d994ce2e7c5e0f2eece8) )      /* LED timing ROM (unused) */
ROM_END



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAMEL( 1981, stactics, 0, stactics, stactics, stactics_state, empty_init, ORIENTATION_FLIP_X, "Sega", "Space Tactics", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE, layout_stactics )
