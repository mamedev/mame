// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Barry Rodewald
/*

  Malzak

  Produced by Kitronix, cabinet artwork suggests it was
  released in 1980.

  Was sold around New Zealand in the early '80s.
  Sold about 500 sets, fairly decent for the time.

  There are more games on this hardware, as far as I am aware.

  Driver by Reip, Barry Rodewald
  SAA 5050 display code "borrowed" from the Philips 2000T/2000M driver in MESS

  Basic memory map

    0x0000 - 0x0fff | S2650 code [malzak.5, malzak.4, malzak.2 (data only)]
    0x1000 - 0x13ff | Work RAM
    0x1400 - 0x14ff | S2636 #1 video
    0x1500 - 0x15ff | S2636 #2 video
    0x1600 - 0x16ff | Playfield gfx
    0x1700 - 0x17ff | Work RAM - contains hiscore table, coin count
    0x1800 - 0x1fff | SAA 5050 video RAM

  TODO:
  - implement sprite-playfield collisions;
  - sprite-playfield colors are dubious at best;
  - Tiles may be flipped for playfield in final stage;
  - Ranking table doesn't sort properly when player gets a better score than
    the ones listed, CPU core bug?
  - improve gfx layers superimposing, honor PAL timings;
  - sound (2x SN76477)
  - sprite->sprite collisions aren't quite perfect
    (you can often fly through flying missiles) <- does this still occur -AS?

  Notes:
  - Test mode in Malzak II should be enabled by setting a POT to position
  '4' and pressing button 1 three times.  Then the POT is set to position
  '1' to go back to the game.

  Guru Notes:
  This board looks like an early proto, it's hand etched, with
  a ton of wire mods. I'm told the game is kind of like Scramble.
  It's probably _ultra_ rare too.

  The board looks like pure crap actually, with lot's of resistors
  jumpering tracks and a partial harness that is wired directly to
  the logic chips which has been *ruthlessly* chopped off!

  Here's what I can see.....

  EPROMs (x5, dumped, MALZAK.1 by itself, the other 4 grouped together)
  S 8039 2650AI (DIP 40)
  S 8051 2636N (DIP 40)
  S 8112 2636N (DIP 40)
  5101 (SRAM, x3)
  6MHz Xtal
  2114 (SRAM, x4)
  SAA 5020 (DIP 24)
  SAA 5050 (DIP 28)
  SN76477 (x2, DIP28)
  INS/DP8212N P8212 (x2, DIP 24)
  plenty of logic chips, resistors, caps etc

  Note: There's no PALs or PROMs

*/

#include "emu.h"
#include "malzak.h"

#include "machine/nvram.h"
#include "sound/sn76477.h"
#include "speaker.h"

#include <algorithm>


uint8_t malzak_state::fake_VRLE_r()
{
	return (m_s2636[0]->read_data(0xcb) & 0x3f) + (m_screen->vblank() ? 0x40 : 0x00);
}

uint8_t malzak_state::s2636_portA_r()
{
	// POT switch position, read from port A of the first S2636
	// Not sure of the correct values to return, but these should
	// do based on the game code.
	switch (ioport("POT")->read())
	{
		case 0:  // Normal play
			return 0xf0;
		case 1:
			return 0x90;
		case 2:
			return 0x70;
		case 3:  // Change settings
			return 0x00;
		default:
			return 0xf0;
	}
}

void malzak_state::malzak_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).bankr("mainbank");
	map(0x1000, 0x10ff).mirror(0x6000).ram();
	map(0x1100, 0x11ff).mirror(0x6000).ram();
	map(0x1200, 0x12ff).mirror(0x6000).ram();
	map(0x1300, 0x13ff).mirror(0x6000).ram();
	map(0x1400, 0x14ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x14cb, 0x14cb).mirror(0x6000).r(FUNC(malzak_state::fake_VRLE_r));
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).ram().w(FUNC(malzak_state::playfield_w));
	map(0x1700, 0x17ff).mirror(0x6000).ram();
	map(0x1800, 0x1fff).mirror(0x6000).ram().share("videoram");
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}


void malzak_state::malzak2_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).bankr("mainbank");
	map(0x1000, 0x10ff).mirror(0x6000).ram();
	map(0x1100, 0x11ff).mirror(0x6000).ram();
	map(0x1200, 0x12ff).mirror(0x6000).ram();
	map(0x1300, 0x13ff).mirror(0x6000).ram();
	map(0x1400, 0x14ff).mirror(0x6000).rw(m_s2636[0], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x14cb, 0x14cb).mirror(0x6000).r(FUNC(malzak_state::fake_VRLE_r));
	map(0x14cc, 0x14cc).mirror(0x6000).r(FUNC(malzak_state::s2636_portA_r));
	map(0x1500, 0x15ff).mirror(0x6000).rw(m_s2636[1], FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1600, 0x16ff).mirror(0x6000).ram().w(FUNC(malzak_state::playfield_w));
	map(0x1700, 0x17ff).mirror(0x6000).ram().share("nvram");
	map(0x1800, 0x1fff).mirror(0x6000).ram().share("videoram");
	map(0x2000, 0x2fff).rom();
	map(0x4000, 0x4fff).rom();
	map(0x6000, 0x6fff).rom();
}


uint8_t malzak_state::s2650_data_r()
{
//  popmessage("S2650 data port read");
	return 0xff;
}

void malzak_state::port40_w(uint8_t data)
{
//  Bit 0 is constantly set high during gameplay
//  Bit 4 is set high, then low, upon death
//  Bit 1 is set high on boot, and on the title screens.
//  Bits 1-3 are all set high upon death, until the game continues
//  Bit 6 is used only in Malzak II, and is set high after checking
//        the selected version
//  logerror("%s S2650: port 0x40 write: 0x%02x\n", machine().describe_context(), data);
	m_mainbank->set_entry((data & 0x40) >> 6);
	// bit 7 is set at final stage
	u8 gfx_bank = ((data & 0x80) >> 7);
	if (m_playfield_bank != gfx_bank)
	{
		m_playfield_bank = gfx_bank;
		m_playfield_tilemap->mark_all_dirty();
	}
}

uint8_t malzak_state::collision_r()
{
	// s2636 (0 only?) <-> tilemap collision detection
	// yyyy ---- y collision
	// 0100 -> upper tilemap border (pixel 160)
	// 1101 -> lower (pixel 448)
	// pix / 32 = suggested value+1
	// ---- xxxx x collision
	// TODO: verify scroll offsets

	// High 4 bits seem to refer to the row affected.
	if(++m_collision_counter > 15)
		m_collision_counter = 0;

	//  logerror("I/O port 0x00 read\n");
	return 0xd0 + m_collision_counter;
}

void malzak_state::malzak_io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(malzak_state::collision_r)); // returns where a collision can occur.
	map(0x40, 0x40).w(FUNC(malzak_state::port40_w));  // possibly sound codes for dual SN76477s
	map(0x60, 0x60).w(FUNC(malzak_state::port60_w));  // possibly playfield scroll X offset
	map(0x80, 0x80).portr("IN0");  //controls
	map(0xa0, 0xa0).nopw();  // echoes I/O port read from port 0x80
	map(0xc0, 0xc0).w(FUNC(malzak_state::portc0_w));  // possibly playfield row selection for writing and/or collisions
}

void malzak_state::malzak_data_map(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(malzak_state::s2650_data_r));  // read upon death
}


static INPUT_PORTS_START( malzak )

	/* Malzak has an 8-way stick
	   and only one button (firing and bomb dropping on the same button) */

	PORT_START("IN0")       /* I/O port 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    )

	PORT_START("POT")
	/* No POT switch on Malzak as far as I know */

INPUT_PORTS_END

static INPUT_PORTS_START( malzak2 )

	/* Same as Malzak, but with additional POT switch, and
	   possibly a reset button too. */

	PORT_START("IN0")       /* I/O port 0x80 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    )

	PORT_START("POT")       /* Fake DIP switch to handle the POT switch */
	PORT_DIPNAME( 0x03, 0x00, "POT switch position" )
	PORT_DIPSETTING( 0x00, "1" )  // Normal play
	PORT_DIPSETTING( 0x01, "2" )
	PORT_DIPSETTING( 0x02, "3" )
	PORT_DIPSETTING( 0x03, "4" )  // Change settings

INPUT_PORTS_END


static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
//  { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*16
//  8*8
};


static GFXDECODE_START( gfx_malzak )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0,  16*16 )
GFXDECODE_END

void malzak_state::machine_start()
{
	m_mainbank->configure_entries(0, 2, memregion("user2")->base(), 0x400);

	save_item(NAME(m_playfield_code));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));
	save_item(NAME(m_collision_counter));
}

void malzak_state::machine_reset()
{
	std::fill(std::begin(m_playfield_code), std::end(m_playfield_code), 0);
}

void malzak_state::malzak(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 3800000/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &malzak_state::malzak_map);
	m_maincpu->set_addrmap(AS_IO, &malzak_state::malzak_io_map);
	m_maincpu->set_addrmap(AS_DATA, &malzak_state::malzak_data_map);
	m_maincpu->sense_handler().set(m_screen, FUNC(screen_device::vblank));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: convert to PAL set_raw
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(480, 512);  /* vert size is a guess */
	m_screen->set_visarea(0, 479, 0, 479);
	m_screen->set_screen_update(FUNC(malzak_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_malzak);
	PALETTE(config, m_palette, FUNC(malzak_state::palette_init), 128);

	S2636(config, m_s2636[0], 0);
	m_s2636[0]->set_offsets(0, -16);  // -8, -16
	m_s2636[0]->add_route(ALL_OUTPUTS, "mono", 0.25);

	S2636(config, m_s2636[1], 0);
	m_s2636[1]->set_offsets(0, -16);  // -9, -16
	m_s2636[1]->add_route(ALL_OUTPUTS, "mono", 0.25);

	SAA5050(config, m_trom, 6000000);
	m_trom->d_cb().set(FUNC(malzak_state::videoram_r));
	m_trom->set_screen_size(42, 24, 64);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	sn76477_device &sn1(SN76477(config, "sn1"));
	sn1.set_noise_params(0, 0, 0);
	sn1.set_decay_res(0);
	sn1.set_attack_params(0, RES_K(100));
	sn1.set_amp_res(RES_K(56));
	sn1.set_feedback_res(RES_K(10));
	sn1.set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	sn1.set_pitch_voltage(5.0);
	sn1.set_slf_params(CAP_U(1.0), RES_K(120));
	sn1.set_oneshot_params(0, 0);
	sn1.set_vco_mode(0);
	sn1.set_mixer_params(1, 1, 1);
	sn1.set_envelope_params(1, 1);
	sn1.set_enable(1);
	sn1.add_route(ALL_OUTPUTS, "mono", 0.25);

	sn76477_device &sn2(SN76477(config, "sn2"));
	sn2.set_noise_params(0, 0, 0);
	sn2.set_decay_res(0);
	sn2.set_attack_params(0, RES_K(100));
	sn2.set_amp_res(RES_K(56));
	sn2.set_feedback_res(RES_K(10));
	sn2.set_vco_params(0, CAP_U(0.1), RES_K(8.2));
	sn2.set_pitch_voltage(5.0);
	sn2.set_slf_params(CAP_U(1.0), RES_K(120));
	sn2.set_oneshot_params(0, 0);
	sn2.set_vco_mode(0);
	sn2.set_mixer_params(1, 1, 1);
	sn2.set_envelope_params(1, 1);
	sn2.set_enable(1);
	sn2.add_route(ALL_OUTPUTS, "mono", 0.25);
}

void malzak_state::malzak2(machine_config &config)
{
	malzak(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &malzak_state::malzak2_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

ROM_START( malzak )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "malzak.5",     0x0000, 0x0800, CRC(75355c98) SHA1(7036ed5d9ee38585b1a6bc204d410d5fb5ddd81f) )
	ROM_CONTINUE( 0x2000, 0x0800 )
	ROM_LOAD( "malzak.4",     0x0800, 0x0400, CRC(744c81e3) SHA1(c08d6df3cf2808a5f99d8247fc19a59be88121a9) )
	ROM_CONTINUE( 0x4000, 0x0c00 )
	// Terrain data
	ROM_LOAD( "malzak.3",     0x4400, 0x0800, CRC(b947229e) SHA1(37b88b5aa91a483fcfe60a9bdd67a66f6378c487) )

	// Screen data
	ROM_REGION(0x0800, "user2", 0)
	ROM_LOAD( "malzak.2",     0x0000, 0x0800, CRC(2a12ad67) SHA1(f89a50b62311a170004c061abd8dedc3ebd84748) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "malzak.1",     0x0000, 0x0800, CRC(74d5ff7b) SHA1(cae326370dc83b86542f9d070e2dc91b1b833356) )
ROM_END

ROM_START( malzak2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "malz1a.bin",   0x000000, 0x000800, CRC(5c3cb14c) SHA1(2d3b5703cb9a47e34aa593f0e8d42d4e67c167d9) )
	ROM_CONTINUE( 0x2000, 0x0800 )

	ROM_LOAD( "malz2b.bin",     0x0800, 0x0400, CRC(2af8aace) SHA1(7aaf03d4848c2cce72b2b3729661e7826834ad44) )
	ROM_CONTINUE( 0x4000, 0x0400 )
	ROM_CONTINUE( 0x2800, 0x0400 )
	ROM_CONTINUE( 0x6000, 0x0400 )
	// Terrain data
	ROM_LOAD( "malz3c.bin",     0x4400, 0x0800, CRC(54d6a02e) SHA1(80c550d74da770689fe451cb0ee8e550a63b1b96) )

	// Screen data
	ROM_REGION(0x0800, "user2", 0)
	ROM_LOAD( "malz4d.bin",     0x0000, 0x0800, CRC(5c6ca415) SHA1(e7571519ac7911507d2c1cf975a7663f41321cb9) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "malzak.1",     0x0000, 0x0800, CRC(74d5ff7b) SHA1(cae326370dc83b86542f9d070e2dc91b1b833356) )

	ROM_REGION( 0x0100, "nvram", 0 )
	// default nvram so that game boots in version II off the bat
	ROM_LOAD( "malzak2.nv",    0x0000, 0x0100, CRC(aadf03d8) SHA1(9d751c4249faab7f5d88d0c99f33468f439641ad) )
ROM_END


GAME( 19??, malzak,  0,      malzak,  malzak,  malzak_state, empty_init, ROT0, "Kitronix", "Malzak",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, malzak2, malzak, malzak2, malzak2, malzak_state, empty_init, ROT0, "Kitronix", "Malzak II", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
