// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

XOR WORLD (c) 1990 Gaelco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Memory Map:
-----------
0x000000-0x01ffff   ROM
0x200000-0x200001   Input #1
0x400000-0x400001   Input #2
0x600000-0x600001   DIPSW #1 + EEPROM read
0x800000-0x800001   Philips SAA1099P write register
0x800002-0x800003   Philips SAA1099P control register
0xa00008-0xa0000d   EEPROM write/ctrl
0xffc000-0xffc7ff   Screen      (8x8 tiles  32x32       (256x256))
0xffc800-0xffc87f   Sprite RAM
0xffc800-0xffc801   INT 2 ACK\Watchdog timer
0xffc802-0xffc803   INT 6 ACK/Watchdog timer
0xffc884-0xffffff   Work RAM

Interrupts:
    Level 2 INT updates the timer
    Level 6 INT is vblank

EEPROM chip: 93C46

***************************************************************************/

#include "emu.h"
#include "includes/xorworld.h"

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "sound/saa1099.h"

#include "screen.h"
#include "speaker.h"


/****************************************************************
                Init machine
****************************************************************/



/****************************************************************
                EEPROM read/write/control
****************************************************************/

WRITE16_MEMBER(xorworld_state::irq2_ack_w)
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

WRITE16_MEMBER(xorworld_state::irq6_ack_w)
{
	m_maincpu->set_input_line(6, CLEAR_LINE);
}

void xorworld_state::xorworld_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x200001).portr("P1");
	map(0x400000, 0x400001).portr("P2");
	map(0x600000, 0x600001).portr("DSW");
	map(0x800000, 0x800003).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
	map(0xa00000, 0xa0000f).w("mainlatch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0xffc000, 0xffc7ff).ram().w(FUNC(xorworld_state::videoram_w)).share("videoram");
	map(0xffc800, 0xffc87f).ram().share("spriteram");
	map(0xffc880, 0xffc881).w(FUNC(xorworld_state::irq2_ack_w)).nopr();
	map(0xffc882, 0xffc883).w(FUNC(xorworld_state::irq6_ack_w)).nopr();
	map(0xffc884, 0xffffff).ram();
}


static INPUT_PORTS_START( xorworld )
	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* used for accessing the NVRAM */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // xor
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // rotate
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,                                                            /* 8x8 tiles */
	0x10000/16,                                                     /* 4096 tiles */
	4,                                                              /* 4 bpp */
	{ 1*0x10000*8, 1*0x10000*8+4, 0, 4 },                           /* plane offsets */
	{ 0*8, 0*8+1, 0*8+2, 0*8+3, 1*8+0, 1*8+1, 1*8+2, 1*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,                                                              /* 16x16 sprites */
	0x10000/64,                                                         /* 1024 sprites */
	4,                                                                  /* 4 bpp */
	{ 1*0x10000*8, 1*0x10000*8+4, 0, 4 },                               /* plane offsets */
	{ 0*8, 0*8+1, 0*8+2, 0*8+3, 1*8+0, 1*8+1, 1*8+2, 1*8+3,
		32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3},
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};


static GFXDECODE_START( gfx_xorworld )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout,  0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, spritelayout, 0, 64 )
GFXDECODE_END


void xorworld_state::xorworld(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10000000);   // 10 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &xorworld_state::xorworld_map);
	//m_maincpu->set_vblank_int("screen", FUNC(xorworld_state::irq6_line_assert)); // irq 4 or 6
	//m_maincpu->set_periodic_int(FUNC(xorworld_state::irq2_line_assert), attotime::from_hz(3*60)); //timed irq, unknown timing
	// Simple fix - but this sounds good!! -Valley Bell
	m_maincpu->set_vblank_int("screen", FUNC(xorworld_state::irq2_line_assert)); // irq 4 or 6
	m_maincpu->set_periodic_int(FUNC(xorworld_state::irq6_line_assert), attotime::from_hz(3*60)); //timed irq, unknown timing

	config.m_minimum_quantum = attotime::from_hz(60);

	EEPROM_93C46_16BIT(config, m_eeprom);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<4>().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)); // CS (active low)
	mainlatch.q_out_cb<5>().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)); // SK (active high)
	mainlatch.q_out_cb<6>().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)); // EEPROM data (DIN)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(xorworld_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_xorworld);
	PALETTE(config, m_palette, FUNC(xorworld_state::xorworld_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", 8000000 /* guess */).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( xorworld )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "c13.bin", 0x000000, 0x010000, CRC(615a864d) SHA1(db07eef19d26a4daa0bcc17ac24d237483f93bf6) )
	ROM_LOAD16_BYTE( "b13.bin", 0x000001, 0x010000, CRC(632e8ee5) SHA1(ec53e632c762f72ad1fe3fab85111bdcc1e818ae) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "d9.bin", 0x000000, 0x010000, CRC(da8d4d65) SHA1(41bcc15f26066bd820b44c0f258e70d0102953c9) )
	ROM_LOAD( "d10.bin",    0x010000, 0x010000, CRC(3b1d6f24) SHA1(bedf60a4cbf20492b8a846b6a7b578f8fe8dbde9) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "b4.bin",   0x0000, 0x0100, CRC(75e468af) SHA1(b5fd1a086c27ca2e837cbbf1b7e57dfdd369b0d0) )  /* Red palette ROM (4 bits) */
	ROM_LOAD( "b7.bin",   0x0100, 0x0100, CRC(7e1cd146) SHA1(fd26a28f90c50ffcb0fe7718820c81eb9fe79e66) )  /* Green palette ROM (4 bits) */
	ROM_LOAD( "b5.bin",   0x0200, 0x0100, CRC(c1b9d9f9) SHA1(c4b02bf60db449fb308a5eb3e41c43299ad8e3e3) )  /* Blue palette ROM (4 bits) */
ROM_END


#define PATCH(data) *rom = data; rom++

void xorworld_state::init_xorworld()
{
	/*  patch some strange protection (without this, strange characters appear
	    after level 5 and some pieces don't rotate properly some times) */

	uint16_t *rom = (uint16_t *)(memregion("maincpu")->base() + 0x1390);

	PATCH(0x4239); PATCH(0x00ff); PATCH(0xe196);    /* clr.b $ffe196 */
	PATCH(0x4239); PATCH(0x00ff); PATCH(0xe197);    /* clr.b $ffe197 */
	PATCH(0x4239); PATCH(0x00ff); PATCH(0xe0bc);    /* clr.b $ffe0bc */
	PATCH(0x41f9); PATCH(0x00ff); PATCH(0xcfce);    /* lea $ffcfce,A0 */
	PATCH(0x3e3c); PATCH(0x000f);                   /* move #$f,D7 */
	PATCH(0x4218);                                  /* clr.b (A0)+ */
	PATCH(0x51cf); PATCH(0xfffc);                   /* dbra D7,$13ac */
	PATCH(0x4e75);                                  /* rts */

	PATCH(0x31ff);                                  /* adjust checksum */
}


GAME( 1990, xorworld, 0, xorworld, xorworld, xorworld_state, init_xorworld, ROT0, "Gaelco", "Xor World (prototype)", MACHINE_SUPPORTS_SAVE )
