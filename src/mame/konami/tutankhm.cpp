// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

    Tutankham

    driver by Mirko Buffoni
    based on original work by Rob Jarrett

    I include here the document based on Rob Jarrett's research because it's
    really exhaustive.

    Sound board: uses the same board as Pooyan.

    Note:
    * The sound board uses a 14.318 MHz xtal.
    * The cpu/video board uses a 18.432 MHz xtal.

    Todo:
    * Discrete filters
    * Starfield


    Custom Chip 084 (Starfield generation)


                  ----------------------
   NE555 1Hz      | 10              24 | VCC
                  |                    |
                  |                    |
   18Mhz      401 | 1               19 | 419 Reset Enable
    6MHz      109 | 2                  |
 256H*        403 | 3   Custom 084     |
 Vblank*      404 | 4                  |
 /SYNC*       405 | 5               18 | Blue
 Aux. Enable  406 | 6               17 | Blue
 Stars Enable 407 | 7                  |
 Horz.Flip    HFF | 8               16 | Green
       GND        | 11              15 | Green
                  |                    |
 H8Q          423 | 23              14 | Red
 V1           422 | 22              13 | Red
 V2           421 | 21                 |
 0x8120       420 | 20                 |
                  ----------------------

 line 420 seems to be related to 419 which is connect to reset circuit -> watchdog

 There are two star fields implemented here. The active one can be selected in
 Machine Configuration.

 a) Scramble Starfield

 1/3 star width like scramble/galaxian, starfield code same as scramble.
 This one is similar to the Konami starfield video below.

 b) Bootleg Starfield

 Guru provided schematics of a daughter-board which on a Konami bootleg replaced
 custom chip 084. This starfield matches the starfield (picture) of this bootleg.

 None of the two alterntives reproduces the star field which can be observed in
 the Stern videos below.

 FIXME: For an exact emulation we need a 084 at some time.
 FIXME: The Konami video below is different from tutankhm. Looks like we
        are missing an early board dump


 Starfield videos ....

 Konami
 https://www.youtube.com/watch?v=YwVjJtQK2n4

 Stern
 https://www.youtube.com/watch?v=g6nv6jHFP80
 https://www.youtube.com/watch?v=D_hmvFi2ehw
 https://www.youtube.com/watch?v=EZNqH-JPzPM
 https://www.youtube.com/watch?v=1MPvSIEIpFU

 */


/***************************************************************************/

#include "emu.h"
#include "tutankhm.h"
#include "konamipt.h"

#include "cpu/m6809/m6809.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Interrupts
 *
 *************************************/

void tutankhm_state::vblank_irq(int state)
{
	/* flip flops cause the interrupt to be signalled every other frame */
	if (state)
	{
		m_irq_toggle ^= 1;
		if (m_irq_toggle && m_irq_enable)
			m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}


void tutankhm_state::irq_enable_w(int state)
{
	m_irq_enable = state;
	if (!m_irq_enable)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}


/*************************************
 *
 *  Bank selection
 *
 *************************************/

void tutankhm_state::bankselect_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x0f);
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

void tutankhm_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


void tutankhm_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void tutankhm_state::sound_on_w(uint8_t data)
{
	m_timeplt_audio->sh_irqtrigger_w(0);
	m_timeplt_audio->sh_irqtrigger_w(1);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void tutankhm_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share(m_videoram);
	map(0x8000, 0x800f).mirror(0x00f0).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	//0x8100 -> Custom 089 D9 Pin 15
	map(0x8100, 0x8100).mirror(0x000f).ram().share(m_scroll);

	/* a read here produces a 1-0-1 write to line 420 (084).
	 * This most likely resets some sort of timer implemented by the 084 custom chip
	 * which would on line 419 trigger a reset.
	 */
	//0x8720 -> Custom 084 F3 Pin 20
	map(0x8120, 0x8120).mirror(0x000f).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	//0x8740 -> Custom 089 D9 Pin 11 - Unknown, not used
	map(0x8160, 0x8160).mirror(0x000f).portr("DSW2"); /* DSW2 (inverted bits) */
	map(0x8180, 0x8180).mirror(0x000f).portr("IN0");  /* IN0 I/O: Coin slots, service, 1P/2P buttons */
	map(0x81a0, 0x81a0).mirror(0x000f).portr("IN1");  /* IN1: Player 1 I/O */
	map(0x81c0, 0x81c0).mirror(0x000f).portr("IN2");  /* IN2: Player 2 I/O */
	map(0x81e0, 0x81e0).mirror(0x000f).portr("DSW1"); /* DSW1 (inverted bits) */
	map(0x8200, 0x8207).mirror(0x00f8).nopr().w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x8300, 0x8300).mirror(0x00ff).w(FUNC(tutankhm_state::bankselect_w));
	map(0x8600, 0x8600).mirror(0x00ff).w(FUNC(tutankhm_state::sound_on_w));
	map(0x8700, 0x8700).mirror(0x00ff).w(m_timeplt_audio, FUNC(timeplt_audio_device::sound_data_w));

	map(0x8800, 0x8fff).ram();
	map(0x9000, 0x9fff).bankr(m_mainbank);
	map(0xa000, 0xffff).rom();
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tutankhm )
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )       PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Flash Bomb" )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "1 per Life" )
	PORT_DIPSETTING(    0x00, "1 per Game" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	PORT_START("IN1")
	KONAMI8_MONO_4WAY_B123_UNK
	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_2WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Flash Bomb")

	PORT_START("IN2")
	KONAMI8_COCKTAIL_4WAY_B123_UNK
	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Flash Bomb")

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("STARS")
	PORT_CONFNAME( 0x01, 0x01, "Starfield selection" )
	PORT_CONFSETTING(    0x00, "Konami HW bootleg (6MHz stars)" )
	PORT_CONFSETTING(    0x01, "Scramble implementation" )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void tutankhm_state::machine_start()
{
	m_mainbank->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x1000);

	m_star_mode = 0;

	save_item(NAME(m_irq_toggle));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_flipscreen_x));
	save_item(NAME(m_flipscreen_y));
	//rgb_t m_star_color[64];
	//std::unique_ptr<uint8_t[]> m_stars;
	save_item(NAME(m_stars_enabled));
	save_item(NAME(m_stars_blink_state));
	save_item(NAME(m_star_mode));
}

void tutankhm_state::machine_reset()
{
	m_irq_toggle = 0;
}

void tutankhm_state::tutankhm(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(18'432'000)/12);   /* 1.5 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &tutankhm_state::main_map);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // C3
	mainlatch.q_out_cb<0>().set(FUNC(tutankhm_state::irq_enable_w));
	mainlatch.q_out_cb<1>().set_nop(); // PAY OUT - not used
	mainlatch.q_out_cb<2>().set(FUNC(tutankhm_state::coin_counter_2_w));
	mainlatch.q_out_cb<3>().set(FUNC(tutankhm_state::coin_counter_1_w));
	mainlatch.q_out_cb<4>().set(FUNC(tutankhm_state::stars_enable_w));
	mainlatch.q_out_cb<5>().set(m_timeplt_audio, FUNC(timeplt_audio_device::mute_w));
	mainlatch.q_out_cb<6>().set(FUNC(tutankhm_state::flip_screen_x_w));
	mainlatch.q_out_cb<7>().set(FUNC(tutankhm_state::flip_screen_y_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(GALAXIAN_PIXEL_CLOCK, GALAXIAN_HTOTAL, GALAXIAN_HBEND, GALAXIAN_HBSTART, GALAXIAN_VTOTAL, GALAXIAN_VBEND, GALAXIAN_VBSTART);
	PALETTE(config, m_palette).set_format(1, tutankhm_state::raw_to_rgb_func, 16);

	m_screen->set_screen_update(FUNC(tutankhm_state::screen_update));
	m_screen->screen_vblank().set(FUNC(tutankhm_state::vblank_irq));

	/* sound hardware */
	TIMEPLT_AUDIO(config, m_timeplt_audio);

	/* blinking frequency is determined by 555 counter with Ra=100k, Rb=10k, C=10uF */
	TIMER(config, "stars").configure_periodic(FUNC(tutankhm_state::scramble_stars_blink_timer), PERIOD_OF_555_ASTABLE(100000, 10000, 0.00001));
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
    Tutankham

    CPU/Video Board: KT-3203-1B
    Sound Board:     KT-5112-2B
*/

ROM_START( tutankhm )
	/* ROMS located on the  KT-3203-1B board. */
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )      /* 64k for M6809 CPU code + 64k for ROM banks */
	ROM_LOAD( "m1.1h", 0x0a000, 0x1000, CRC(da18679f) SHA1(8d2a3665db937d0e1d19300ae22277d9db61fcbc) ) /* program ROMs */
	ROM_LOAD( "m2.2h", 0x0b000, 0x1000, CRC(a0f02c85) SHA1(29a78b3ffd6b597772953543b02dd59acf5af38c) )
	ROM_LOAD( "3j.3h", 0x0c000, 0x1000, CRC(ea03a1ab) SHA1(27a3cca0595bac642caaf9ee2f276814442c8721) ) /* Name guessed */
	ROM_LOAD( "m4.4h", 0x0d000, 0x1000, CRC(bd06fad0) SHA1(bd10bbb413d8dd362072522e902575d819fa8336) )
	ROM_LOAD( "m5.5h", 0x0e000, 0x1000, CRC(bf9fd9b0) SHA1(458ea2ff5eedaaa02e32444dd6004d2eaadbdeab) )
	ROM_LOAD( "j6.6h", 0x0f000, 0x1000, CRC(fe079c5b) SHA1(0757490aaa1cea4f4bbe1230d811a0d917f59e52) ) /* Name guessed */
	ROM_LOAD( "c1.1i", 0x10000, 0x1000, CRC(7eb59b21) SHA1(664d3e08df0f3d6690838810b6fe273eec3b7821) ) /* graphic ROMs (banked) -- only 9 of 12 are filled */
	ROM_LOAD( "c2.2i", 0x11000, 0x1000, CRC(6615eff3) SHA1(e8455eab03f66642880595cfa0e9be285bf9fad0) )
	ROM_LOAD( "c3.3i", 0x12000, 0x1000, CRC(a10d4444) SHA1(683899e1014ee075b16d9d2610c3c5b5c4efedb6) )
	ROM_LOAD( "c4.4i", 0x13000, 0x1000, CRC(58cd143c) SHA1(e4ab27c09858cede478f4ed3ac6d7392e383a470) )
	ROM_LOAD( "c5.5i", 0x14000, 0x1000, CRC(d7e7ae95) SHA1(7068797770a6c42dc733b253bf6b7376eb6e071e) )
	ROM_LOAD( "c6.6i", 0x15000, 0x1000, CRC(91f62b82) SHA1(2a78039ee63226978544142727d00d1ccc6d2ab4) )
	ROM_LOAD( "c7.7i", 0x16000, 0x1000, CRC(afd0a81f) SHA1(cf10308a0fa4ffabd0deeb186b5602468028ff92) )
	ROM_LOAD( "c8.8i", 0x17000, 0x1000, CRC(dabb609b) SHA1(773b99b670db41a9de58d14b51f81ce0c446ca84) )
	ROM_LOAD( "c9.9i", 0x18000, 0x1000, CRC(8ea9c6a6) SHA1(fe1b299f8760fc5418179d3569932ee2c4dff461) )
	/* the other banks (1900-1fff) are empty */

	/* ROMS located on the KT-5112-2B board. */
	ROM_REGION(  0x3000, "timeplt_audio:tpsound", ROMREGION_ERASE00 ) /* 12k for Z80 sound CPU code */
	ROM_LOAD( "s1.7a", 0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "s2.8a", 0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END


/*
A PCB picture shows the following label format:

TUTANKHAM     (C)
RA1   1I (25)
1982    STERN

Visible ROM labels on the KT-3203-1B PCB:
TUTANKHAM   RA1 1I (25)  1982   STERN
TUTANKHAM   RA1 2I (25)  1982   STERN
TUTANKHAM   RA1 3I (25)  1982   STERN
TUTANKHAM   RA1 4I (25)  1982   STERN
TUTANKHAM   RA1 5I (25)  1982   STERN
TUTANKHAM   RA1 6I (25)  1982   STERN
TUTANKHAM   RA1 7I (25)  1982   STERN
TUTANKHAM   RA1 8I (25)  1982   STERN
TUTANKHAM   RA1 9I (25)  1982   STERN

ROMs labels on the KT-5112-2B PCB:
TUTANKHAM   RA1 8A (26)  1982   STERN    (in socket 7A)
TUTANKHAM   RA1 10E (25)  1982   STERN    (in socket 8A)

*/
ROM_START( tutankhms )
	/* ROMS located on the KT-3203-1B board. */
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00 )      /* 64k for M6809 CPU code + 64k for ROM banks */
	ROM_LOAD( "m1.1h", 0x0a000, 0x1000, CRC(da18679f) SHA1(8d2a3665db937d0e1d19300ae22277d9db61fcbc) ) /* program ROMs */
	ROM_LOAD( "m2.2h", 0x0b000, 0x1000, CRC(a0f02c85) SHA1(29a78b3ffd6b597772953543b02dd59acf5af38c) )
	ROM_LOAD( "3a.3h", 0x0c000, 0x1000, CRC(2d62d7b1) SHA1(910718f36735f2614cda0c3a1abdfa995d82dbd2) )
	ROM_LOAD( "m4.4h", 0x0d000, 0x1000, CRC(bd06fad0) SHA1(bd10bbb413d8dd362072522e902575d819fa8336) )
	ROM_LOAD( "m5.5h", 0x0e000, 0x1000, CRC(bf9fd9b0) SHA1(458ea2ff5eedaaa02e32444dd6004d2eaadbdeab) )
	ROM_LOAD( "a6.6h", 0x0f000, 0x1000, CRC(c43b3865) SHA1(3112cf831c5b6318337e591ccb0003aeab722652) )
	ROM_LOAD( "c1.1i", 0x10000, 0x1000, CRC(7eb59b21) SHA1(664d3e08df0f3d6690838810b6fe273eec3b7821) ) /* graphic ROMs (banked) -- only 9 of 12 are filled */
	ROM_LOAD( "c2.2i", 0x11000, 0x1000, CRC(6615eff3) SHA1(e8455eab03f66642880595cfa0e9be285bf9fad0) )
	ROM_LOAD( "c3.3i", 0x12000, 0x1000, CRC(a10d4444) SHA1(683899e1014ee075b16d9d2610c3c5b5c4efedb6) )
	ROM_LOAD( "c4.4i", 0x13000, 0x1000, CRC(58cd143c) SHA1(e4ab27c09858cede478f4ed3ac6d7392e383a470) )
	ROM_LOAD( "c5.5i", 0x14000, 0x1000, CRC(d7e7ae95) SHA1(7068797770a6c42dc733b253bf6b7376eb6e071e) )
	ROM_LOAD( "c6.6i", 0x15000, 0x1000, CRC(91f62b82) SHA1(2a78039ee63226978544142727d00d1ccc6d2ab4) )
	ROM_LOAD( "c7.7i", 0x16000, 0x1000, CRC(afd0a81f) SHA1(cf10308a0fa4ffabd0deeb186b5602468028ff92) )
	ROM_LOAD( "c8.8i", 0x17000, 0x1000, CRC(dabb609b) SHA1(773b99b670db41a9de58d14b51f81ce0c446ca84) )
	ROM_LOAD( "c9.9i", 0x18000, 0x1000, CRC(8ea9c6a6) SHA1(fe1b299f8760fc5418179d3569932ee2c4dff461) )
	/* the other banks (1900-1fff) are empty */

	/* ROMS located on the KT-5112-2B board. */
	ROM_REGION(  0x3000, "timeplt_audio:tpsound", ROMREGION_ERASE00 ) /* 12k for Z80 sound CPU code */
	ROM_LOAD( "s1.7a", 0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "s2.8a", 0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, tutankhm, 0,        tutankhm, tutankhm, tutankhm_state, empty_init, ROT90, "Konami", "Tutankham", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS)
GAME( 1982, tutankhms,tutankhm, tutankhm, tutankhm, tutankhm_state, empty_init, ROT90, "Konami (Stern Electronics license)", "Tutankham (Stern Electronics)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS)
