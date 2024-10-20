// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Sega Super Derby II hardware

    preliminary driver by Phil Bennett

    Games supported:
        * Super Derby II


 TODO: Add PCB layouts

 PCBs:

 834-5526? - Main PCB? (Has yet to be found)
 834-5527  - Tilemaps and video output?
 834-5528  - Sprites?
 834-5529  - Satellite PCB (see sg1000a.cpp)

 Customs:

 315-5011
 315-5012
 315-5025
 315-5050

 The video CPUs are waiting for commands from another source.
 It is assumed that there is at least one other PCB (834-5526?) which
 drives the video PCBs and perhaps generates sound.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "speaker.h"
#include "screen.h"
#include "video/resnet.h"


namespace {

class sderby2_state : public driver_device
{
public:
	sderby2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_proms(*this, "proms"),
		m_palette(*this, "palette")
	{ }

	void sderby2(machine_config &config);

	void init_sderby2();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void sderby2_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	[[maybe_unused]] void palette_w(offs_t offset, uint8_t data);
	uint8_t host_r();
	void main_nmi(uint8_t data);
	void sub_nmi(uint8_t data);
	uint8_t sub_r();
	void host_io_40_w(uint8_t data);
	uint8_t sub_io_0_r();
	[[maybe_unused]] uint8_t sub_unk_r();
	[[maybe_unused]] void sub_unk_w(uint8_t data);

	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_region_ptr<uint8_t> m_proms;
	required_device<palette_device> m_palette;

	uint8_t sub_data = 0;
	uint8_t main_data = 0;
	uint8_t host_io_40 = 0;
	void main_io_map(address_map &map) ATTR_COLD;
	void main_program_map(address_map &map) ATTR_COLD;
	void sub_io_map(address_map &map) ATTR_COLD;
	void sub_program_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void sderby2_state::sderby2_palette(palette_device &palette) const
{

}

void sderby2_state::palette_w(offs_t offset, uint8_t data)
{
	const rgb_t color = m_palette->pen_color(data & 0xff);
	m_palette->set_pen_color(0x100 + offset, color);
}

void sderby2_state::video_start()
{

}


uint32_t sderby2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

uint8_t sderby2_state::host_r()
{
	return main_data;
}

void sderby2_state::main_nmi(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	main_data = data;
}

void sderby2_state::host_io_40_w(uint8_t data)
{
	host_io_40 = data;
}

uint8_t sderby2_state::sub_r()
{
	return sub_data;
}

void sderby2_state::sub_nmi(uint8_t data)
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	sub_data = data;
}

uint8_t sderby2_state::sub_unk_r()
{
	return machine().rand();
}

void sderby2_state::sub_unk_w(uint8_t data)
{

}

uint8_t sderby2_state::sub_io_0_r()
{
	return host_io_40;
}


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

/*
 1 x D4168  (8kx8) - Work RAM (banked?)
 2 x TC5565 (8kx8)
 2 x TC5533 (4kx8)
 2 x TC5533 (4kx8)
12 x 2148   (1024x4)  (near palette ROMs?) - 4 for each screen

 2KB of palette per screen? (0x7FF)

*/
void sderby2_state::main_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xcfff).ram();
	map(0xe000, 0xe3ff).ram();
	map(0xe400, 0xe7ff).ram();
	map(0xe800, 0xebff).ram();
	map(0xec00, 0xefff).ram();
	map(0xf000, 0xffff).ram(); // Is this banked?
}

void sderby2_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x10).r(FUNC(sderby2_state::host_r));
	map(0x20, 0x20).w(FUNC(sderby2_state::sub_nmi));
	map(0x30, 0x30).nopw(); // Written with 0x12 byte sequence at start
	map(0x40, 0x40).w(FUNC(sderby2_state::host_io_40_w)); // Occasionally written
}


/*
 1 x D4168 (8kx8)  Near Z80
 6 x 2148  (1024x4) Near Z80
18 x 2148  (1024x4) Middle
18 x 2148  (1024x4) Side
 4 x 2148  (1024x4) Near 316-5012 and 316-5011
*/
void sderby2_state::sub_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();

	map(0xc000, 0xcfff).ram(); // Tested at FFF, 1016, 102D
	map(0xd000, 0xd3ff).ram(); // 2KB Tested
	map(0xd400, 0xd7ff).ram(); // 2KB Tested at 105B
	map(0xd800, 0xdbff).ram(); // 2KB Tested at 105B
	map(0xdc00, 0xdfff).ram(); // 2KB Tested at 10B7

	map(0xe000, 0xffff).ram(); // Tested at FE8
}

void sderby2_state::sub_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(sderby2_state::sub_io_0_r));
	map(0x20, 0x20).r(FUNC(sderby2_state::sub_r));
	map(0x40, 0x40).w(FUNC(sderby2_state::main_nmi));
	map(0x60, 0x60).nopw(); // Written with 0x12 byte sequence at start
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sderby2 )
	PORT_START("P1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics decoding
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	1024,
	4,
	{ 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_sderby2 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 1024 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0, 1024 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void sderby2_state::machine_start()
{

}

void sderby2_state::machine_reset()
{

}

void sderby2_state::sderby2(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(3'579'545));
	m_maincpu->set_addrmap(AS_PROGRAM, &sderby2_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &sderby2_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sderby2_state::irq0_line_hold));

	Z80(config, m_subcpu, XTAL(3'579'545));
	m_subcpu->set_addrmap(AS_PROGRAM, &sderby2_state::sub_program_map);
	m_subcpu->set_addrmap(AS_IO, &sderby2_state::sub_io_map);
	m_subcpu->set_vblank_int("screen", FUNC(sderby2_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256 - 1, 0, 256 - 1);
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	screen.set_screen_update(FUNC(sderby2_state::screen_update));
	screen.set_palette(m_palette);
	PALETTE(config, m_palette, FUNC(sderby2_state::sderby2_palette), 256+256*3);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sderby2);

	// sound hardware
}

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( sderby2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6205.3s",   0x00000, 0x4000, CRC(c341de53) SHA1(9c273a2c1bd9739b1ea28438d578ab5eb3abfc4a) )
	ROM_LOAD( "epr-6206.3t",   0x04000, 0x4000, CRC(e700b601) SHA1(e0418e00bfb2ccb59ede9f081e12795d5d94287e) )

	ROM_REGION( 0x18000, "gfx1", 0 ) // Character tiles
	ROM_LOAD( "epr-6213.12h",  0x00000, 0x4000, CRC(d07fe96b) SHA1(cfd5ddaa843bc40c73966fa74a9b2c3f909f15bd) )
	ROM_LOAD( "epr-6214.12i",  0x04000, 0x4000, CRC(44fa1e2b) SHA1(88778b24b0f6f678bfb78c93cfa480195a4295bb) )
	ROM_LOAD( "epr-6215.12k",  0x08000, 0x4000, CRC(ab54d286) SHA1(897256b6709e1a4da9daba92b6bde39ccfccd8c1) )

	ROM_REGION( 0x18000, "gfx2", 0 ) // Character tiles?
	ROM_LOAD( "epr-6216.13h",  0x00000, 0x4000, CRC(c1f84ee6) SHA1(a16bc96c82725993d50db958827e474b7988415f) )
	ROM_LOAD( "epr-6217.13i",  0x04000, 0x4000, CRC(e0e7c85a) SHA1(31ac590290466ca76fc31d29c392195947922caa) )
	ROM_LOAD( "epr-6218.13k",  0x08000, 0x4000, CRC(6470b00b) SHA1(c32b11f840096428e18e1283e0622a315470f497) )

	ROM_REGION( 0x18000, "gfx3", 0 ) // Sprites?
	ROM_LOAD( "epr-6207.8v",   0x00000, 0x4000, CRC(7fbbaceb) SHA1(67905982db35fab7d62f1907af1e0df0c9af265c) )
	ROM_LOAD( "epr-6208.8w",   0x04000, 0x4000, CRC(dde795de) SHA1(7e7f63c14af42c0f1f52f670f6b4fa3d305fce07) )
	ROM_LOAD( "epr-6209.8x",   0x08000, 0x4000, CRC(bd97a8d5) SHA1(df71b6bbc2a5f4e7ad713253eebe3c7b5364cff6) )
	ROM_LOAD( "epr-6210.9v",   0x0c000, 0x4000, CRC(18b6cd90) SHA1(10619a4ff6c73cfa9ef7c4ce21f41ea61f2bb420) )
	ROM_LOAD( "epr-6211.9w",   0x10000, 0x4000, CRC(7dafbbdf) SHA1(6ac019e7b2d7523779ebc683ac2190e08f3183c0) )
	ROM_LOAD( "epr-6212.9x",   0x14000, 0x4000, CRC(1d2459e6) SHA1(201185fb8a5292acff4dec1f6d4c4b4dccf3c0a7) )

	ROM_REGION( 0x08000, "saku", 0 ) // 'SAKU ROM DATA' - Track positions?
	ROM_LOAD( "epr-6221.12r",  0x00000, 0x4000, CRC(c023ff18) SHA1(39691040a2222ca356f4cddfe1e3c97925c73837) )
	// 0x2cb5: C1h (Phil) vs 81h (Hammy)
	ROM_LOAD( "epr-6222.12s",  0x04000, 0x4000, CRC(62a687ea) SHA1(3a571d652a98785da0d87d561a322e8a21d939c8) )

	// 834-5528
	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD( "epr-6240.1l",   0x00000, 0x4000, CRC(cd53736b) SHA1(eb865df8dd3f9a5b572cfae5cb79b4bb650b6739) )
	ROM_LOAD( "epr-6241.1m",   0x04000, 0x4000, CRC(527b2c0d) SHA1(ca979eff0a3435f428e1771e07743ea5372e75b0) )

	ROM_REGION( 0x20000, "gfx4", 0)
	ROM_LOAD( "epr-6242.7t",   0x00000, 0x8000, CRC(ef72ab7c) SHA1(67b19080fc639866582b14739aa32138f2debf6e) )
	ROM_LOAD( "epr-6243.11t",  0x08000, 0x8000, CRC(3741fd86) SHA1(52aafb5168b25b78a82eafd8f507fd7afd352be2) )
	ROM_LOAD( "epr-6246.12m",  0x10000, 0x8000, CRC(bc2a3af4) SHA1(3f1ad33b53327f2714c3a1ca4eeb5296f769ceb3) )
	ROM_LOAD( "epr-6247.12r",  0x18000, 0x8000, CRC(cedfa342) SHA1(3d780779e55237b0396ed0d7183ac7b00de871cb) )

	ROM_REGION( 0x1800, "proms", 0) // Color PROMs
	ROM_LOAD( "pr-6226.1b", 0x00000, 0x0100, CRC(f04684fb) SHA1(bbb757faaf7d98096b85266e9d43ce305a456285) ) // All 82S129
	ROM_LOAD( "pr-6225.2b", 0x00100, 0x0100, CRC(c903e226) SHA1(787ce629e4a352ba9f2d7241fe78444700f23983) )
	ROM_LOAD( "pr-6224.3b", 0x00200, 0x0100, CRC(8862c6e6) SHA1(bc327f985cd147606f83b682cce0a93913ed6aaa) )
	ROM_LOAD( "pr-6223.2c", 0x00300, 0x0100, CRC(12354f92) SHA1(5fe3f92c174d68930536a6f59f8d75386b7fdefe) )

	ROM_LOAD( "pr-6226.4b", 0x00400, 0x0100, CRC(f04684fb) SHA1(bbb757faaf7d98096b85266e9d43ce305a456285) )
	ROM_LOAD( "pr-6225.5b", 0x00500, 0x0100, CRC(c903e226) SHA1(787ce629e4a352ba9f2d7241fe78444700f23983) )
	ROM_LOAD( "pr-6224.6b", 0x00600, 0x0100, CRC(8862c6e6) SHA1(bc327f985cd147606f83b682cce0a93913ed6aaa) )
	ROM_LOAD( "pr-6223.5c", 0x00700, 0x0100, CRC(12354f92) SHA1(5fe3f92c174d68930536a6f59f8d75386b7fdefe) )

	ROM_LOAD( "pr-6226.7b", 0x00800, 0x0100, CRC(f04684fb) SHA1(bbb757faaf7d98096b85266e9d43ce305a456285) )
	ROM_LOAD( "pr-6225.8b", 0x00900, 0x0100, CRC(c903e226) SHA1(787ce629e4a352ba9f2d7241fe78444700f23983) )
	ROM_LOAD( "pr-6224.9b", 0x00a00, 0x0100, CRC(8862c6e6) SHA1(bc327f985cd147606f83b682cce0a93913ed6aaa) )
	ROM_LOAD( "pr-6223.9c", 0x00b00, 0x0100, CRC(12354f92) SHA1(5fe3f92c174d68930536a6f59f8d75386b7fdefe) )

	ROM_REGION( 0x0800, "proms_misc", 0)
	ROM_LOAD( "pr-6235.7p",  0x0000, 0x0020, CRC(981ea339) SHA1(4483ead7fe8bc4188b71775c09c5b5927f0028b1) ) // 82S123
	ROM_LOAD( "pr-6236.7r",  0x0040, 0x0020, CRC(5ebe0b9d) SHA1(e3ecf25eff5f04a785ee88248cad58cc4d8d5122) ) // 82S123
	ROM_LOAD( "pr-6237.12x", 0x0100, 0x0100, CRC(5514cde3) SHA1(967af563258c5324bf4b1ca8dcece929d1a69880) ) // TBP28L22N
	ROM_LOAD( "pr-6237.12w", 0x0200, 0x0100, CRC(5514cde3) SHA1(967af563258c5324bf4b1ca8dcece929d1a69880) ) // TBP28L22N
	ROM_LOAD( "pr-6237.13w", 0x0300, 0x0100, CRC(5514cde3) SHA1(967af563258c5324bf4b1ca8dcece929d1a69880) ) // TBP28L22N
	ROM_LOAD( "82s141.8n",   0x0400, 0x0200, CRC(2b35954a) SHA1(ceef7aed9b8a3603d56e79a7953c15a7db96aded) )

	// TODO: Document PALs
	// 315-5080 = PAL14H4 (PROTECTED)
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void sderby2_state::init_sderby2()
{

}

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1985, sderby2, 0, sderby2, sderby2, sderby2_state, init_sderby2, ROT0, "Sega", "Super Derby II", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
