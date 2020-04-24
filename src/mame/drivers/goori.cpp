// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
//#include "sound/3812intf.h"
//#include "sound/okim6295.h"
//#include "sound/ym2151.h"
#include "speaker.h"

class goori_state : public driver_device
{
public:
	goori_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki")
	{ }

	void goori(machine_config &config);

protected:

	virtual void machine_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void goori_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<okim6295_device> m_oki;


private:

};

uint32_t goori_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	return 0;
}

void goori_state::goori_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram(); // RAM main
	map(0x400000, 0x400fff).ram(); // RAM tilemaps
	map(0x600000, 0x603fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x700000, 0x701fff).ram(); // RAM sprites
}

static INPUT_PORTS_START( goori )
INPUT_PORTS_END

static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,8,9,10,11 },
	{ 0,4,16,20,32,36,48,52, 512+0, 512+4, 512+16, 512+20, 512+32, 512+36, 512+48, 512+52 },
	{   STEP8(0,8*8), STEP8(1024,8*8)},
	8*8*8*4,
};

static GFXDECODE_START( gfx_unico )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0x0, 0x20 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x8, 0x0, 0x20 ) // [1] Layers
GFXDECODE_END

void goori_state::machine_start()
{
}

void goori_state::goori(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2); /* 16MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &goori_state::goori_map);
	m_maincpu->set_vblank_int("screen", FUNC(goori_state::irq4_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 224);
	screen.set_visarea(0, 384-1, 0, 224-1);
	screen.set_screen_update(FUNC(goori_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_unico);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x2000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	/*
	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(14'318'181)/4)); // not verified
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.40);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.40);

	OKIM6295(config, m_oki, 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.80);
	*/
}

ROM_START( goori )
	ROM_REGION( 0x100000, "maincpu", 0 ) // first half empty
	ROM_LOAD16_BYTE( "goori02.4m", 0x000000, 0x040000, CRC(65a8568e) SHA1(c1a07f3a009df4af898ab62c15416073fcf768d9) )
	ROM_CONTINUE(0x000000, 0x040000)
	ROM_LOAD16_BYTE( "goori03.4m", 0x000001, 0x040000, CRC(de4b8818) SHA1(599ab6a354ab21d50c1b8c11e980f0b16f18e4dd) )
	ROM_CONTINUE(0x000001, 0x040000)

	ROM_REGION( 0x400000, "gfx1", ROMREGION_INVERT ) // sprites?
	ROM_LOAD( "goori04ol.16m", 0x000000, 0x200000, CRC(f26451b9) SHA1(c6818a44115d3efed2566442295dc0b253057602) )
	ROM_LOAD( "goori05oh.16m", 0x200000, 0x200000, CRC(058ceaec) SHA1(8639d41685a6f3fb2d81b9aaf3c160666de8155d) )

	ROM_REGION( 0x400000, "gfx2", ROMREGION_INVERT ) // bgs?
	ROM_LOAD( "goori06cl.16m", 0x000000, 0x200000, CRC(8603a662) SHA1(fbe5ccb3fded60b431ffee27471158c95a8328f8) )
	ROM_LOAD( "goori07ch.16m", 0x200000, 0x200000, CRC(4223383e) SHA1(aa17eab343dad3f6eab05a844081370e3eebcd2e) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "goori01.2m", 0x000000, 0x040000, CRC(c74351b9) SHA1(397f4b6aea23e6619e099c5cc99f38bae74bc3e8) )
ROM_END

GAME( 1999, goori, 0,       goori, goori, goori_state,    empty_init, ROT0, "Unico", "Goori Goori", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

