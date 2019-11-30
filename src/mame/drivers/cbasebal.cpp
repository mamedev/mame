// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Capcom Baseball


    Somewhat similar to the "Mitchell hardware", but different enough to
    deserve its own driver.

    TODO:
    - understand what bit 6 of input port 0x12 is
    - unknown bit 5 of bankswitch register

***************************************************************************/

#include "emu.h"
#include "includes/cbasebal.h"

#include "cpu/z80/z80.h"
#include "machine/kabuki.h"  // needed for decoding functions only
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ym2413.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE8_MEMBER(cbasebal_state::cbasebal_bankswitch_w)
{
	/* bits 0-4 select ROM bank */
	//logerror("%04x: bankswitch %02x\n", m_maincpu->pc(), data);
	membank("bank1")->set_entry(data & 0x1f);
	membank("bank1d")->set_entry(data & 0x1f);

	/* bit 5 used but unknown */

	/* bits 6-7 select RAM bank */
	m_rambank = (data & 0xc0) >> 6;
}


READ8_MEMBER(cbasebal_state::bankedram_r)
{
	switch (m_rambank)
	{
	case 2:
		return cbasebal_textram_r(space, offset);   /* VRAM */
	case 1:
		if (offset < 0x800)
			return m_palette->basemem().read8(offset);
		else
			return 0;
	default:
		return cbasebal_scrollram_r(space, offset); /* SCROLL */
	}
}

WRITE8_MEMBER(cbasebal_state::bankedram_w)
{
	switch (m_rambank)
	{
	case 2:
		cbasebal_textram_w(space, offset, data);
		break;
	case 1:
		if (offset < 0x800)
			m_palette->write8(offset, data);
		break;
	default:
		cbasebal_scrollram_w(space, offset, data);
		break;
	}
}

WRITE8_MEMBER(cbasebal_state::cbasebal_coinctrl_w)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x04);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x08);
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void cbasebal_state::cbasebal_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).rw(FUNC(cbasebal_state::bankedram_r), FUNC(cbasebal_state::bankedram_w)).share("palette");  /* palette + vram + scrollram */
	map(0xe000, 0xfdff).ram();     /* work RAM */
	map(0xfe00, 0xffff).ram().share("spriteram");
}

void cbasebal_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank0d");
	map(0x8000, 0xbfff).bankr("bank1d");
}

void cbasebal_state::cbasebal_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(cbasebal_state::cbasebal_bankswitch_w));
	map(0x01, 0x01).portw("IO_01");
	map(0x02, 0x02).portw("IO_02");
	map(0x03, 0x03).portw("IO_03");
	map(0x05, 0x05).w("oki", FUNC(okim6295_device::write));
	map(0x06, 0x07).w("ymsnd", FUNC(ym2413_device::write));
	map(0x08, 0x09).w(FUNC(cbasebal_state::cbasebal_scrollx_w));
	map(0x0a, 0x0b).w(FUNC(cbasebal_state::cbasebal_scrolly_w));
	map(0x10, 0x10).portr("P1");
	map(0x11, 0x11).portr("P2");
	map(0x12, 0x12).portr("SYSTEM");
	map(0x13, 0x13).w(FUNC(cbasebal_state::cbasebal_gfxctrl_w));
	map(0x14, 0x14).w(FUNC(cbasebal_state::cbasebal_coinctrl_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( cbasebal )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")       /* ? */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "IO_01" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)

	PORT_START( "IO_02" )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)

	PORT_START( "IO_03" )
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout cbasebal_textlayout =
{
	8,8,    /* 8*8 characters */
	4096,   /* 4096 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 16 consecutive bytes */
};

static const gfx_layout cbasebal_tilelayout =
{
	16,16,  /* 16*16 tiles */
	4096,   /* 4096 tiles */
	4,      /* 4 bits per pixel */
	{ 4096*64*8+4, 4096*64*8+0,4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every tile takes 64 consecutive bytes */
};

static const gfx_layout cbasebal_spritelayout =
{
	16,16,  /* 16*16 sprites */
	4096,   /* 2048 sprites */
	4,      /* 4 bits per pixel */
	{ 4096*64*8+4, 4096*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 33*8+0, 33*8+1, 33*8+2, 33*8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( gfx_cbasebal )
	GFXDECODE_ENTRY( "gfx1", 0, cbasebal_textlayout,   256,  8 ) /* colors 256- 287 */
	GFXDECODE_ENTRY( "gfx2", 0, cbasebal_tilelayout,   768, 16 ) /* colors 768-1023 */
	GFXDECODE_ENTRY( "gfx3", 0, cbasebal_spritelayout, 512,  8 ) /* colors 512- 639 */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void cbasebal_state::machine_start()
{
	save_item(NAME(m_rambank));
	save_item(NAME(m_tilebank));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_text_on));
	save_item(NAME(m_bg_on));
	save_item(NAME(m_obj_on));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

void cbasebal_state::machine_reset()
{
	m_rambank = 0;
	m_tilebank = 0;
	m_spritebank = 0;
	m_text_on = 0;
	m_bg_on = 0;
	m_obj_on = 0;
	m_flipscreen = 0;
	m_scroll_x[0] = 0;
	m_scroll_x[1] = 0;
	m_scroll_y[0] = 0;
	m_scroll_y[1] = 0;
}

void cbasebal_state::cbasebal(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 6000000);   /* ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &cbasebal_state::cbasebal_map);
	m_maincpu->set_addrmap(AS_IO, &cbasebal_state::cbasebal_portmap);
	m_maincpu->set_addrmap(AS_OPCODES, &cbasebal_state::decrypted_opcodes_map);
	m_maincpu->set_vblank_int("screen", FUNC(cbasebal_state::irq0_line_hold));   /* ??? */

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	screen.set_screen_update(FUNC(cbasebal_state::screen_update_cbasebal));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cbasebal);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_444, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1056000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.50); // clock frequency & pin 7 not verified

	YM2413(config, "ymsnd", 3579545).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cbasebal )
	ROM_REGION( 0x90000, "maincpu", 0 )
	ROM_LOAD( "cbj10.11j",    0x00000, 0x08000, CRC(bbff0acc) SHA1(db9e2c89e030255851789caaf85f24dc73609d9b) )
	ROM_LOAD( "cbj07.16f",    0x10000, 0x20000, CRC(8111d13f) SHA1(264e21e824c87f55da326440c6ed71e1c287a63e) )
	ROM_LOAD( "cbj06.14f",    0x30000, 0x20000, CRC(9aaa0e37) SHA1(1a7b96b44c66b58f06707aafb1806520747b8c76) )
	ROM_LOAD( "cbj05.13f",    0x50000, 0x20000, CRC(d0089f37) SHA1(32354c3f4693a65e297791c4d8faac3aa9cff5a1) )
	/* 0x70000-0x8ffff empty (space for 04) */

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "cbj13.16m",    0x00000, 0x10000, CRC(2359fa0a) SHA1(3a37532ea43dd4b150c53a240d35a57a9b76d23d) )  /* text */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "cbj02.1f",     0x00000, 0x20000, CRC(d6740535) SHA1(2ece885525718fd5fe52b8fa4c07930695b89659) )  /* tiles */
	ROM_LOAD( "cbj03.2f",     0x20000, 0x20000, CRC(88098dcd) SHA1(caddebeea581129d6a62fc9f7f354d61eef175c7) )
	ROM_LOAD( "cbj08.1j",     0x40000, 0x20000, CRC(5f3344bf) SHA1(1d3193078108e86e31bbfce15a8d2443cfbf2ff6) )
	ROM_LOAD( "cbj09.2j",     0x60000, 0x20000, CRC(aafffdae) SHA1(26e76b55fff49811df8e5b1f165be20ec8dd196a) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "cbj11.1m",     0x00000, 0x20000, CRC(bdc1507d) SHA1(efeaf3066acfb7186d73ad8e5b291d6e61965de2) )  /* sprites */
	ROM_LOAD( "cbj12.2m",     0x20000, 0x20000, CRC(973f3efe) SHA1(d776499d5ac4bc23eb5d1f28b88447cc07d8ac99) )
	ROM_LOAD( "cbj14.1n",     0x40000, 0x20000, CRC(765dabaa) SHA1(742d1c50b65f649f23eac7976fe26c2d7400e4e1) )
	ROM_LOAD( "cbj15.2n",     0x60000, 0x20000, CRC(74756de5) SHA1(791d6620cdb563f0b3a717432aa4647981b0a10e) )

	ROM_REGION( 0x80000, "oki", 0 ) /* OKIM */
	ROM_LOAD( "cbj01.1e",     0x00000, 0x20000, CRC(1d8968bd) SHA1(813e475d1d0c343e7dad516f1fe564d00c9c27fb) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void cbasebal_state::init_cbasebal()
{
	uint8_t *src = memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();
	m_decoded = std::make_unique<uint8_t[]>(size);
	pang_decode(src, m_decoded.get(), size);
	membank("bank1")->configure_entries(0, 32, src + 0x10000, 0x4000);
	membank("bank0d")->set_base(m_decoded.get());
	membank("bank1d")->configure_entries(0, 32, m_decoded.get() + 0x10000, 0x4000);
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1989, cbasebal, 0, cbasebal, cbasebal, cbasebal_state, init_cbasebal, ROT0, "Capcom", "Capcom Baseball (Japan)", MACHINE_SUPPORTS_SAVE )
