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

  TODO - I/O ports (0x00 for sprite->background collisions)
         sound (2x SN76477)
         playfield graphics may be banked, tiles above 0x1f are incorrect
         sprite->sprite collisions aren't quite perfect
           (you can often fly through flying missiles)

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
#include "cpu/s2650/s2650.h"
#include "sound/sn76477.h"
#include "video/saa5050.h"
#include "machine/nvram.h"
#include "includes/malzak.h"


READ8_MEMBER(malzak_state::fake_VRLE_r)
{
	return (m_s2636_0->read_data(space, 0xcb) & 0x3f) + (m_screen->vblank() ? 0x40 : 0x00);
}

READ8_MEMBER(malzak_state::s2636_portA_r)
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

static ADDRESS_MAP_START( malzak_map, AS_PROGRAM, 8, malzak_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0fff) AM_ROMBANK("bank1")
	AM_RANGE(0x1000, 0x10ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1100, 0x11ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1200, 0x12ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1300, 0x13ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x14cb, 0x14cb) AM_MIRROR(0x6000) AM_READ(fake_VRLE_r)
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_device, read_data, write_data)
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_1", s2636_device, read_data, write_data)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_RAM_WRITE(malzak_playfield_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1800, 0x1fff) AM_MIRROR(0x6000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( malzak2_map, AS_PROGRAM, 8, malzak_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0fff) AM_ROMBANK("bank1")
	AM_RANGE(0x1000, 0x10ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1100, 0x11ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1200, 0x12ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x1300, 0x13ff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x14cb, 0x14cb) AM_MIRROR(0x6000) AM_READ(fake_VRLE_r)
	AM_RANGE(0x14cc, 0x14cc) AM_MIRROR(0x6000) AM_READ(s2636_portA_r)
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_0", s2636_device, read_data, write_data)
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_DEVREADWRITE("s2636_1", s2636_device, read_data, write_data)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_RAM_WRITE(malzak_playfield_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x1800, 0x1fff) AM_MIRROR(0x6000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x6000, 0x6fff) AM_ROM
ADDRESS_MAP_END


READ8_MEMBER(malzak_state::s2650_data_r)
{
	popmessage("S2650 data port read");
	return 0xff;
}

WRITE8_MEMBER(malzak_state::port40_w)
{
//  Bit 0 is constantly set high during gameplay
//  Bit 4 is set high, then low, upon death
//  Bit 1 is set high on boot, and on the title screens.
//  Bits 1-3 are all set high upon death, until the game continues
//  Bit 6 is used only in Malzak II, and is set high after checking
//        the selected version
//  logerror("S2650 [0x%04x]: port 0x40 write: 0x%02x\n", m_maincpu->safe_pc(), data);
	membank("bank1")->set_entry((data & 0x40) >> 6);
}

WRITE8_MEMBER(malzak_state::port60_w)
{
	m_malzak_x = data;
	//  logerror("I/O: port 0x60 write 0x%02x\n", data);
}

WRITE8_MEMBER(malzak_state::portc0_w)
{
	m_malzak_y = data;
	//  logerror("I/O: port 0xc0 write 0x%02x\n", data);
}

READ8_MEMBER(malzak_state::collision_r)
{
	// High 4 bits seem to refer to the row affected.
	if(++m_collision_counter > 15)
		m_collision_counter = 0;

	//  logerror("I/O port 0x00 read\n");
	return 0xd0 + m_collision_counter;
}

static ADDRESS_MAP_START( malzak_io_map, AS_IO, 8, malzak_state )
	AM_RANGE(0x00, 0x00) AM_READ(collision_r) // returns where a collision can occur.
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w)  // possibly sound codes for dual SN76477s
	AM_RANGE(0x60, 0x60) AM_WRITE(port60_w)  // possibly playfield scroll X offset
	AM_RANGE(0x80, 0x80) AM_READ_PORT("IN0")  //controls
	AM_RANGE(0xa0, 0xa0) AM_WRITENOP  // echoes I/O port read from port 0x80
	AM_RANGE(0xc0, 0xc0) AM_WRITE(portc0_w)  // possibly playfield row selection for writing and/or collisions
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READ(s2650_data_r)  // read upon death
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END


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

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

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

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

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


static GFXDECODE_START( malzak )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,         0,  16 )
GFXDECODE_END


PALETTE_INIT_MEMBER(malzak_state, malzak)
{
	int i;

	for (i = 0; i < 8 * 8; i++)
	{
		palette.set_pen_color(i * 2 + 0, pal1bit(i >> 3), pal1bit(i >> 4), pal1bit(i >> 5));
		palette.set_pen_color(i * 2 + 1, pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}
}


READ8_MEMBER(malzak_state::videoram_r)
{
	return m_videoram[offset];
}

void malzak_state::machine_start()
{
	membank("bank1")->configure_entries(0, 2, memregion("user2")->base(), 0x400);

	m_saa5050 = machine().device("saa5050");

	save_item(NAME(m_playfield_code));
	save_item(NAME(m_malzak_x));
	save_item(NAME(m_malzak_y));
}

void malzak_state::machine_reset()
{
	memset(m_playfield_code, 0, 256 * sizeof(int));

	m_malzak_x = 0;
	m_malzak_y = 0;
}

static MACHINE_CONFIG_START( malzak, malzak_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, 3800000/4)
	MCFG_CPU_PROGRAM_MAP(malzak_map)
	MCFG_CPU_IO_MAP(malzak_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(480, 512)  /* vert size is a guess */
	MCFG_SCREEN_VISIBLE_AREA(0, 479, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(malzak_state, screen_update_malzak)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", malzak)
	MCFG_PALETTE_ADD("palette", 128)
	MCFG_PALETTE_INIT_OWNER(malzak_state, malzak)

	MCFG_DEVICE_ADD("s2636_0", S2636, 0)
	MCFG_S2636_OFFSETS(0, -16)  // -8, -16
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DEVICE_ADD("s2636_1", S2636, 0)
	MCFG_S2636_OFFSETS(0, -16)  // -9, -16
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DEVICE_ADD("saa5050", SAA5050, 6000000)
	MCFG_SAA5050_D_CALLBACK(READ8(malzak_state, videoram_r))
	MCFG_SAA5050_SCREEN_SIZE(42, 24, 64)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(0)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(1, 1, 1)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 1)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("sn2", SN76477, 0)
	MCFG_SN76477_NOISE_PARAMS(0, 0, 0)                  // noise + filter: N/C
	MCFG_SN76477_DECAY_RES(0)                           // decay_res: N/C
	MCFG_SN76477_ATTACK_PARAMS(0, RES_K(100))           // attack_decay_cap + attack_res
	MCFG_SN76477_AMP_RES(RES_K(56))                     // amplitude_res
	MCFG_SN76477_FEEDBACK_RES(RES_K(10))                // feedback_res
	MCFG_SN76477_VCO_PARAMS(0, CAP_U(0.1), RES_K(8.2))  // VCO volt + cap + res
	MCFG_SN76477_PITCH_VOLTAGE(5.0)                     // pitch_voltage
	MCFG_SN76477_SLF_PARAMS(CAP_U(1.0), RES_K(120))     // slf caps + res
	MCFG_SN76477_ONESHOT_PARAMS(0, 0)                   // oneshot caps + res: N/C
	MCFG_SN76477_VCO_MODE(0)                            // VCO mode
	MCFG_SN76477_MIXER_PARAMS(1, 1, 1)                  // mixer A, B, C
	MCFG_SN76477_ENVELOPE_PARAMS(1, 1)                  // envelope 1, 2
	MCFG_SN76477_ENABLE(1)                              // enable
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( malzak2, malzak )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(malzak2_map)

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

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
ROM_END


GAME( 19??, malzak,   0,       malzak,  malzak, driver_device,  0,        ROT0, "Kitronix", "Malzak", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 19??, malzak2, malzak,   malzak2, malzak2, driver_device, 0,        ROT0, "Kitronix", "Malzak II", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
