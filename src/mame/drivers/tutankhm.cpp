// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

    Tutankham

    driver by Mirko Buffoni
    based on original work by Rob Jarrett

    I include here the document based on Rob Jarrett's research because it's
    really exaustive.

    Sound board: uses the same board as Pooyan.

    Note:
    * The sound board uses a 14.318 MHz xtal.
    * The cpu/video board uses a 18.432 MHz xtal.

    Todo:
    * Discrete filters
    * Starfield


    Custom Chip 084 (Starfield generation)

    * Inputs:
    *
    *      NE555 ==> 10 (approx 1 HZ)
    *       401  ==>  1  (crystal clock, most likely around 6 MHz)
    *       109  ==>  2  (Video related)
    *       403  ==>  3  (Video related)
    *       403  ==>  4  (Video related)
    *       405  ==>  5  (Video related)
    *       406  ==>  6  (Video related)
    *       420  ==> 20  (Video related)
    *       421  ==> 21  (Video related)
    *       422  ==> 22  (Video related)
    *       423  ==> 23  (Video horizontal signal H1)
    *0x8206 HFF  ==>  7 (Horizontal flip)
    *0x8204 407  ==>  8  (Enable ???)
    *
    *Outputs:
    *   13      Red
    *   14      Red
    *   15      Green
    *   16      Green
    *   17      Blue
    *   18      Blue


***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "includes/konamipt.h"
#include "audio/timeplt.h"
#include "includes/tutankhm.h"


/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(tutankhm_state::tutankhm_interrupt)
{
	/* flip flops cause the interrupt to be signalled every other frame */
	m_irq_toggle ^= 1;
	if (m_irq_toggle && m_irq_enable)
		device.execute().set_input_line(0, ASSERT_LINE);
}


WRITE8_MEMBER(tutankhm_state::irq_enable_w)
{
	m_irq_enable = data & 1;
	if (!m_irq_enable)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}


/*************************************
 *
 *  Bank selection
 *
 *************************************/

WRITE8_MEMBER(tutankhm_state::tutankhm_bankselect_w)
{
	membank("bank1")->set_entry(data & 0x0f);
}


/*************************************
 *
 *  Outputs
 *
 *************************************/

WRITE8_MEMBER(tutankhm_state::sound_mute_w)
{
	machine().sound().system_mute(data & 1);
}


WRITE8_MEMBER(tutankhm_state::tutankhm_coin_counter_w)
{
	coin_counter_w(machine(), offset ^ 1, data);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, tutankhm_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8000, 0x800f) AM_MIRROR(0x00f0) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8100, 0x8100) AM_MIRROR(0x000f) AM_RAM AM_SHARE("scroll")
	AM_RANGE(0x8120, 0x8120) AM_MIRROR(0x000f) AM_READ(watchdog_reset_r)
	AM_RANGE(0x8160, 0x8160) AM_MIRROR(0x000f) AM_READ_PORT("DSW2") /* DSW2 (inverted bits) */
	AM_RANGE(0x8180, 0x8180) AM_MIRROR(0x000f) AM_READ_PORT("IN0")  /* IN0 I/O: Coin slots, service, 1P/2P buttons */
	AM_RANGE(0x81a0, 0x81a0) AM_MIRROR(0x000f) AM_READ_PORT("IN1")  /* IN1: Player 1 I/O */
	AM_RANGE(0x81c0, 0x81c0) AM_MIRROR(0x000f) AM_READ_PORT("IN2")  /* IN2: Player 2 I/O */
	AM_RANGE(0x81e0, 0x81e0) AM_MIRROR(0x000f) AM_READ_PORT("DSW1") /* DSW1 (inverted bits) */
	AM_RANGE(0x8200, 0x8200) AM_MIRROR(0x00f8) AM_READNOP AM_WRITE(irq_enable_w)
	AM_RANGE(0x8202, 0x8203) AM_MIRROR(0x00f8) AM_WRITE(tutankhm_coin_counter_w)
	AM_RANGE(0x8204, 0x8204) AM_MIRROR(0x00f8) AM_WRITENOP // starfield?
	AM_RANGE(0x8205, 0x8205) AM_MIRROR(0x00f8) AM_WRITE(sound_mute_w)
	AM_RANGE(0x8206, 0x8206) AM_MIRROR(0x00f8) AM_WRITE(tutankhm_flip_screen_x_w)
	AM_RANGE(0x8207, 0x8207) AM_MIRROR(0x00f8) AM_WRITE(tutankhm_flip_screen_y_w)
	AM_RANGE(0x8300, 0x8300) AM_MIRROR(0x00ff) AM_WRITE(tutankhm_bankselect_w)
	AM_RANGE(0x8600, 0x8600) AM_MIRROR(0x00ff) AM_DEVWRITE("timeplt_audio", timeplt_audio_device, sh_irqtrigger_w)
	AM_RANGE(0x8700, 0x8700) AM_MIRROR(0x00ff) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x8800, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9fff) AM_ROMBANK("bank1")
	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


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
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

MACHINE_START_MEMBER(tutankhm_state,tutankhm)
{
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x1000);

	save_item(NAME(m_irq_toggle));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_flip_x));
	save_item(NAME(m_flip_y));
}

MACHINE_RESET_MEMBER(tutankhm_state,tutankhm)
{
	m_irq_toggle = 0;
	m_irq_enable = 0;
	m_flip_x = 0;
	m_flip_y = 0;
}

static MACHINE_CONFIG_START( tutankhm, tutankhm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_18_432MHz/12)   /* 1.5 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tutankhm_state,  tutankhm_interrupt)

	MCFG_MACHINE_START_OVERRIDE(tutankhm_state,tutankhm)
	MCFG_MACHINE_RESET_OVERRIDE(tutankhm_state,tutankhm)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)  /* not sure about the visible area */
	MCFG_SCREEN_UPDATE_DRIVER(tutankhm_state, screen_update_tutankhm)

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_FORMAT(BBGGGRRR)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(timeplt_sound)
MACHINE_CONFIG_END


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
	ROM_REGION( 0x20000, "maincpu", 0 )      /* 64k for M6809 CPU code + 64k for ROM banks */
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
	ROM_REGION(  0x10000 , "tpsound", 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "s1.7a", 0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "s2.8a", 0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END


ROM_START( tutankhms )
	/* ROMS located on the KT-3203-1B board. */
	ROM_REGION( 0x20000, "maincpu", 0 )      /* 64k for M6809 CPU code + 64k for ROM banks */
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
	ROM_REGION(  0x10000, "tpsound", 0 ) /* 64k for Z80 sound CPU code */
	ROM_LOAD( "s1.7a", 0x0000, 0x1000, CRC(b52d01fa) SHA1(9b6cf9ea51d3a87c174f34d42a4b1b5f38b48723) )
	ROM_LOAD( "s2.8a", 0x1000, 0x1000, CRC(9db5c0ce) SHA1(b5bc1d89a7f7d7a0baae64390c37ee11f69a0e76) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, tutankhm, 0,        tutankhm, tutankhm, driver_device, 0, ROT90, "Konami", "Tutankham", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS)
GAME( 1982, tutankhms,tutankhm, tutankhm, tutankhm, driver_device, 0, ROT90, "Konami (Stern Electronics license)", "Tutankham (Stern Electronics)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS)
