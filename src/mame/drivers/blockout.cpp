// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Block Out

    driver by Nicola Salmoria

    DIP locations verified for:
    - blockout (manual)

****************************************************************************

   Agress PCB Info
   Palco System Corp., 1991

   This game runs on an original unmodified Technos 'Block Out' PCB.
   All of the Technos identifications are hidden under 'Palco' or 'Agress' stickers.

   PCB Layout (Applies to both Agress and Block Out)
   ----------

   PS-05307 (sticker)
   TA-0029-P1-02 (printed on Block Out PCB under the sticker)

   |--------------------------------------------------------|
   | M51516            YM2151                    3.579545MHz|
   |           YM3012     6116                              |
   |   MB3615  1.056MHz   PALCO3.73            20MHz        |
   |           M6295                                        |
   |   MB3615                            82S129PR.25  28MHz |
   |           PALCO4.78                                    |
   |                      Z80            |-------|          |
   |                                     |TECHNOS|          |
   |J         2018     6264              |TJ-001 |          |
   |A                                    |(QFP80)|          |
   |M         2018                       |-------|          |
   |M                                                       |
   |A                  6264                                 |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |                               MB81461-12               |
   |  DSW1(8)                      MB81461-12               |
   |                               MB81461-12               |
   |  DSW2(8)                      MB81461-12               |
   |         PALCO2.91                                      |
   |                 PALCO1.81               68000          |
   |--------------------------------------------------------|

   Notes:
      68000 clock : 10.000MHz
      Z80 clock   : 3.579545MHz
      M6295 clock : 1.056MHz, sample rate = 1056000 / 132
      YM2151 clock: 3.579545MHz
      VSync       : 58Hz

      PROM is used for video timing etc, without it, no graphics are displayed,
      only 'Insert Coin' and the manufacturer text/year on the title screen.

      palco1.81 \ Main program   27C010
      palco2.91 /                  "
      palco3.73   OKI samples    27C256
      palco4.78   Z80 program    27C010

***************************************************************************/

#include "emu.h"
#include "includes/blockout.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "speaker.h"


#define MAIN_CLOCK XTAL(10'000'000)
#define AUDIO_CLOCK XTAL(3'579'545)

WRITE16_MEMBER(blockout_state::blockout_irq6_ack_w)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

WRITE16_MEMBER(blockout_state::blockout_irq5_ack_w)
{
	m_maincpu->set_input_line(5, CLEAR_LINE);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void blockout_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100001).portr("P1");
	map(0x100002, 0x100003).portr("P2");
	map(0x100004, 0x100005).portr("SYSTEM");
	map(0x100006, 0x100007).portr("DSW1");
	map(0x100008, 0x100009).portr("DSW2");
	map(0x100010, 0x100011).w(FUNC(blockout_state::blockout_irq6_ack_w));
	map(0x100012, 0x100013).w(FUNC(blockout_state::blockout_irq5_ack_w));
	map(0x100015, 0x100015).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x100016, 0x100017).nopw();    /* don't know, maybe reset sound CPU */
	map(0x180000, 0x1bffff).ram().w(FUNC(blockout_state::blockout_videoram_w)).share("videoram");
	map(0x1d4000, 0x1dffff).ram(); /* work RAM */
	map(0x1f4000, 0x1fffff).ram(); /* work RAM */
	map(0x200000, 0x207fff).ram().share("frontvideoram");
	map(0x208000, 0x21ffff).ram(); /* ??? */
	map(0x280002, 0x280003).w(FUNC(blockout_state::blockout_frontcolor_w));
	map(0x280200, 0x2805ff).ram().w(FUNC(blockout_state::blockout_paletteram_w)).share("paletteram");
}

void blockout_state::agress_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100001).portr("P1");
	map(0x100002, 0x100003).portr("P2");
	map(0x100004, 0x100005).portr("SYSTEM");
	map(0x100006, 0x100007).portr("DSW1");
	map(0x100008, 0x100009).portr("DSW2");
	map(0x100010, 0x100011).w(FUNC(blockout_state::blockout_irq6_ack_w));
	map(0x100012, 0x100013).w(FUNC(blockout_state::blockout_irq5_ack_w));
	map(0x100015, 0x100015).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x100016, 0x100017).nopw();    /* don't know, maybe reset sound CPU */
	map(0x180000, 0x1bffff).ram().w(FUNC(blockout_state::blockout_videoram_w)).share("videoram");
	map(0x1d4000, 0x1dffff).ram(); /* work RAM */
	map(0x1f4000, 0x1fffff).ram(); /* work RAM */
	map(0x200000, 0x207fff).ram().share("frontvideoram");
	map(0x208000, 0x21ffff).ram(); /* ??? */
	map(0x280002, 0x280003).w(FUNC(blockout_state::blockout_frontcolor_w));
	map(0x280200, 0x2805ff).ram().w(FUNC(blockout_state::blockout_paletteram_w)).share("paletteram");
}

void blockout_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x8800, 0x8801).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( blockout )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Drop Button") // on top of stick
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 B Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 C Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Drop Button") // on top of stick
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B Button")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C Button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	/* the following two are supposed to control Coin 2, but they don't work. */
	/* This happens on the original board too. */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )        /* According to the manual SW1:3,4 should be the same settings as SW1:1,2 */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPNAME( 0x10, 0x10, "1 Coin to Continue" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )          /* Same price as Coinage setting */
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )           /* Always 1 Coin */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )        /* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Rotate Buttons" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )        /* Listed as "Unused" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A Button") PORT_DIPLOCATION("SW2:7") /* Listed as "Unused" */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A Button") PORT_PLAYER(2) PORT_DIPLOCATION("SW2:8") /* Listed as "Unused" */
INPUT_PORTS_END

static INPUT_PORTS_START( blockoutj )
	PORT_INCLUDE( blockout )

	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 A Button (Drop)")

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 A Button (Drop)") PORT_PLAYER(2)

	PORT_MODIFY("DSW2")
	/* The following switch is 2/3 rotate buttons on the other sets, option doesn't exist in the Japan version */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	/* these can still be used on the difficutly select even if they can't be used for rotating pieces in this version */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )        /* Listed as "Unused" */
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 )        PORT_DIPLOCATION("SW2:7")
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_DIPLOCATION("SW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( agress )
	PORT_INCLUDE( blockout )

	// Button 1 & 2 looks identical gameplay wise (i.e. stop slots after each ten levels)
	PORT_MODIFY("P1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button 3 (Bomb)")

	PORT_MODIFY("P2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Button 3 (Bomb)")

	/* factory shipment setting is all dips OFF */
	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, "Opening Cut" )           PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )

	PORT_MODIFY("DSW2")
	/* Engrish here. The manual says "Number of Prayers". Maybe related to lives? */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Players ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END


/*************************************
 *
 *  Sound interface
 *
 *************************************/

/* handler called by the 2151 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(blockout_state::irq_handler)
{
	m_audiocpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void blockout_state::machine_start()
{
	save_item(NAME(m_color));
}

void blockout_state::machine_reset()
{
	m_color = 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(blockout_state::blockout_scanline)
{
	int scanline = param;

	if(scanline == 250) // vblank-out irq
		m_maincpu->set_input_line(6, ASSERT_LINE);

	if(scanline == 0) // vblank-in irq or directly tied to coin inputs (TODO: check)
		m_maincpu->set_input_line(5, ASSERT_LINE);
}

void blockout_state::blockout(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MAIN_CLOCK);       /* MRH - 8.76 makes gfx/adpcm samples sync better -- but 10 is correct speed*/
	m_maincpu->set_addrmap(AS_PROGRAM, &blockout_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(blockout_state::blockout_scanline), "screen", 0, 1);

	Z80(config, m_audiocpu, AUDIO_CLOCK);  /* 3.579545 MHz */
	m_audiocpu->set_addrmap(AS_PROGRAM, &blockout_state::audio_map);


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	/* assume same as ddragon3 with adjusted visible display area */
	m_screen->set_raw(XTAL(28'000'000) / 4, 448, 0, 320, 272, 10, 250);
	m_screen->set_screen_update(FUNC(blockout_state::screen_update_blockout));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(513);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", AUDIO_CLOCK));
	ymsnd.irq_handler().set(FUNC(blockout_state::irq_handler));
	ymsnd.add_route(0, "lspeaker", 0.60);
	ymsnd.add_route(1, "rspeaker", 0.60);

	okim6295_device &oki(OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}

void blockout_state::agress(machine_config &config)
{
	blockout(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &blockout_state::agress_map);
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( blockout )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "bo29a0-2.bin", 0x00000, 0x20000, CRC(b0103427) SHA1(53cac2adc04783abbde21e9f3c0e655f22f68f69) )
	ROM_LOAD16_BYTE( "bo29a1-2.bin", 0x00001, 0x20000, CRC(5984d5a2) SHA1(4b350856d0313d40eaa3d8a8d9e310f74bc20398) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )    /* unknown */
ROM_END

ROM_START( blockout2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "29a0",         0x00000, 0x20000, CRC(605f931e) SHA1(65fa7227dafde1fc8564e09fa949fe575b394d8a) )
	ROM_LOAD16_BYTE( "29a1",         0x00001, 0x20000, CRC(38f07000) SHA1(e4070e3067d77cc1b0d8d0c63786f2729c5c703a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )    /* unknown */
ROM_END

ROM_START( blockoutj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "2.bin",         0x00000, 0x20000, CRC(e16cf065) SHA1(541b30b054cf08f10d6ca4746423759f4326c005) )
	ROM_LOAD16_BYTE( "1.bin",         0x00001, 0x20000, CRC(950b28a3) SHA1(7d1635ac2a3fc1efdd2f78cd6038bd7b4c907b1b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bo29e3-0.bin", 0x0000, 0x8000, CRC(3ea01f78) SHA1(5fc4ad4d9f03d7c26d2afc3e7ede75589e40b0d8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "bo29e2-0.bin", 0x0000, 0x20000, CRC(15c5a99d) SHA1(89091eda454a028fd1f17501584bd589baf6d523) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7114h.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )    /* unknown */
ROM_END

ROM_START( agress )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "palco1.81",         0x00000, 0x20000, CRC(3acc917a) SHA1(14960588673458d862daf14a8d7474af6c95c2ad) )
	ROM_LOAD16_BYTE( "palco2.91",         0x00001, 0x20000, CRC(abfd5bcc) SHA1(bf0ea8ba00750ea2ddf2b8afc96393bf8a730068) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "palco3.73", 0x0000, 0x8000, CRC(2a21c97d) SHA1(7f71bf18db3e6ff9c69c589268450e66c6585cdd) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "palco4.78", 0x0000, 0x20000, CRC(9dfd9cfe) SHA1(5ea8f98bc0cd117cde81c04f02aa33199afe8231) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "82s129pr.25",   0x0000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )   /* unknown */
ROM_END

// this is probably an original English version with copyright year hacked
ROM_START( agressb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 2*128k for 68000 code */
	ROM_LOAD16_BYTE( "palco1.ic81",  0x00000, 0x20000, CRC(a1875175) SHA1(6c9946bcd4fe7987d4f817ea25bfc76432188883) )
	ROM_LOAD16_BYTE( "palco2.ic91",  0x00001, 0x20000, CRC(ab3182c3) SHA1(788a3e7cf6ef889262f3d72af8be9ec951eb397b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "palco3.ic73",  0x000000, 0x08000, CRC(2a21c97d) SHA1(7f71bf18db3e6ff9c69c589268450e66c6585cdd) )

	ROM_REGION( 0x40000, "oki", 0 ) /* 128k for ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "palco4.ic78",  0x000000, 0x20000, CRC(9dfd9cfe) SHA1(5ea8f98bc0cd117cde81c04f02aa33199afe8231) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "prom29-mb7114h.ic25", 0x000000, 0x0100, CRC(b25bbda7) SHA1(840f1470886bd0019db3cd29e3d1d80205a65f48) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

void blockout_state::init_agress()
{
	/*
	 * agress checks at F3A that this is mirrored, blockout glitches if you mirror to it
	 * But actually mirroring this VRAM makes display to be offset
	 * (clearly visible with text being on top bank instead of bottom during gameplay)
	 * There are many possible solutions to this:
	 * A) reads are actually ORed between upper and lower banks
	 * B) VRAM is initialized with same pattern checked, or extreme open bus occurs for the second uninitalized bank.
	 * C) Agress isn't truly identical to Block Out HW wise, it really mirrors VRAM data and offsets display
	 * D) it's not supposed to enter into trace mode at all, cause of the bogus mirror check (trace exception
	 *    occurs at very beginning of the program execution)
	 * For now let's use D and just patch the TRACE exception that causes the bogus mirror check
	 */

	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	rom[0x82/2] = 0x2700;
}

GAME( 1989, blockout,  0,        blockout, blockout,  blockout_state, empty_init,  ROT0, "Technos Japan / California Dreams", "Block Out (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, blockout2, blockout, blockout, blockout,  blockout_state, empty_init,  ROT0, "Technos Japan / California Dreams", "Block Out (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, blockoutj, blockout, blockout, blockoutj, blockout_state, empty_init,  ROT0, "Technos Japan / California Dreams", "Block Out (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, agress,    0,        agress,   agress,    blockout_state, init_agress, ROT0, "Palco",   "Agress - Missile Daisenryaku (Japan)",           MACHINE_SUPPORTS_SAVE )
GAME( 2003, agressb,   agress,   agress,   agress,    blockout_state, init_agress, ROT0, "bootleg", "Agress - Missile Daisenryaku (English bootleg)", MACHINE_SUPPORTS_SAVE )
