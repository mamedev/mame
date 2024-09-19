// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, David Haywood,Stephane Humbert
/***************************************************************************

3x3 Puzzle
Ace Enterprise

Driver by Mariusz Wojcieszek, David Haywood and Stephh

Typical and simple Korean hardware.....

68000 @ 10MHz
OKI M6295 @ 1MHz (4/4)
Lattice pLSI1032 for graphics generation
6264 SRAM x 2
6116 SRAM x 8
62256 SRAM x2 (main program RAM)
DIP SW 8-position X2 (SW1 SW2)
OSC 27MHz, 22.1184MHz, 10MHz
XTAL 4MHz

Additional ROMs with 'a' in label are probably to convert
the game back to normal version as current set on the PCB
has adult graphics (sets provided are 'Normal' and 'Enterprise')

Notes:
 Casanova: The game code has several minor bugs / glitches such as:

     * Using 1C/2C and inserting 5 coins causes graphics corruption as the
       game places data on the screen beyond the defined character data
       used for the numbers 0 through 9
     * On the intro, parts of the tilemaps are not being copied correctly
       causing bits of the charcter's hat to vanish
     * Background colors inexplicably change in certain places between frames
     * Dipswitch descriptions on the DIP INFO page do not match actual effects
       of said dipswitches

  These are not emulation bugs and have been verified on a real PCB

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class _3x3puzzle_state : public driver_device
{
public:
	_3x3puzzle_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram%u", 1U)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
	{ }

	void _3x3puzzle(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr_array<uint16_t, 3> m_videoram;

	std::unique_ptr<uint16_t[]> m_videoram_buffer[3];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;

	// video-related
	tilemap_t   *m_tilemap[3] = {nullptr, nullptr, nullptr};

	int       m_oki_bank = 0;
	uint16_t  m_gfx_control = 0;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template <unsigned Which> TILE_GET_INFO_MEMBER(get_tile_info);

	void gfx_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tilemap1_scrollx_w(uint16_t data);
	void tilemap1_scrolly_w(uint16_t data);

	void _3x3puzzle_map(address_map &map) ATTR_COLD;
};


template <unsigned Which>
TILE_GET_INFO_MEMBER(_3x3puzzle_state::get_tile_info)
{
	tileinfo.set(Which,
			m_videoram_buffer[Which][tile_index],
			0,
			0);
}

void _3x3puzzle_state::gfx_ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// does this have registers to control when the actual tile/palette
	// data is copied to a private buffer?

	// bit 5 (0x20) cleared when palette is written
	// bit 4 (0x10) screen width - 1: 512 pixels, 0: 320 pixels
	// bit 3 (0x08) is set when tilemap scroll registers are written
	// bit 1,2(0x06) OKI banking
	// bit 0 (0x01) set in Casanova intro (could be OKI bank instead of bit 2?)

	//printf("%04x\n",data&0xc7);
	COMBINE_DATA(&m_gfx_control);

	if (BIT(data, 4))
	{
		m_screen->set_visible_area(0*8, 64*8-1, 0*8, 30*8-1);
	}
	else
	{
		m_screen->set_visible_area(0*8, 40*8-1, 0*8, 30*8-1);
	}

	if ((data & 0x06) != m_oki_bank)
	{
		m_oki_bank = data & 0x06;
		m_oki->set_rom_bank(m_oki_bank >> 1);
	}
}

void _3x3puzzle_state::tilemap1_scrollx_w(uint16_t data)
{
	m_tilemap[0]->set_scrollx(data);
}

void _3x3puzzle_state::tilemap1_scrolly_w(uint16_t data)
{
	m_tilemap[0]->set_scrolly(data);
}

void _3x3puzzle_state::video_start()
{
	for (int i = 0; i < 3; i++)
	{
		size_t const videoram_size = m_videoram[i].length();
		m_videoram_buffer[i] = make_unique_clear<uint16_t[]>(videoram_size);
		save_pointer(NAME(m_videoram_buffer[i]), videoram_size, i);
	}
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_3x3puzzle_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_3x3puzzle_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(_3x3puzzle_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1]->set_transparent_pen(0);
	m_tilemap[2]->set_transparent_pen(0);
}

uint32_t _3x3puzzle_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);

	// guess based on register use and Casanova intro
	if (BIT(m_gfx_control, 5))
	{
		for (int i = 0; i < 3; i++)
		{
			size_t const videoram_size = m_videoram[i].length();
			for (size_t offset = 0; offset < videoram_size; offset++)
			{
				m_videoram_buffer[i][offset] = m_videoram[i][offset];
				m_tilemap[i]->mark_tile_dirty(offset);
			}
		}
	}

	return 0;
}

void _3x3puzzle_state::_3x3puzzle_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x2007ff).ram().share(m_videoram[0]);
	map(0x201000, 0x201fff).ram().share(m_videoram[1]);
	map(0x202000, 0x202fff).ram().share(m_videoram[2]);
	map(0x280000, 0x280001).portr("VBLANK");
	map(0x300000, 0x3005ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x400000, 0x400001).w(FUNC(_3x3puzzle_state::tilemap1_scrollx_w));
	map(0x480000, 0x480001).w(FUNC(_3x3puzzle_state::tilemap1_scrolly_w));
	map(0x500000, 0x500001).portr("P1");
	map(0x580000, 0x580001).portr("SYS");
	map(0x600000, 0x600001).portr("DSW01");
	map(0x700001, 0x700001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800000, 0x800001).w(FUNC(_3x3puzzle_state::gfx_ctrl_w));
	map(0x880000, 0x880001).nopr(); // read, but no tested afterwards
}

static INPUT_PORTS_START( _3x3puzzle )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // must be active_low or coins won't work
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("VBLANK")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("DSW01")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Coinage ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Free Play / Debug mode" )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( casanova )
	PORT_INCLUDE( _3x3puzzle )

	PORT_MODIFY("SYS")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("DSW01") // Do NOT trust "DIP INFO" for correct settings! At least Coinage is WRONG!
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) ) // DIP switch info shows 2 Coins / Credit
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) ) // DIP switch info shows 3 Coins / Credit
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) ) // DIP switch info shows 5 Coins / Credit
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )
	PORT_DIPNAME( 0x0080, 0x0080, "Dip Info" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0100, 0x0100, "SW2:1" )        // DSW2 bank, not used?
	PORT_DIPUNUSED_DIPLOC( 0x0200, 0x0200, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0400, 0x0400, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )

INPUT_PORTS_END

static GFXDECODE_START( gfx_3x3puzzle )
	GFXDECODE_ENTRY( "tiles1", 0, gfx_16x16x8_raw, 0x000, 1 )
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x8x8_raw,   0x100, 1 )
	GFXDECODE_ENTRY( "tiles3", 0, gfx_8x8x8_raw,   0x200, 1 )
GFXDECODE_END

void _3x3puzzle_state::machine_start()
{
	save_item(NAME(m_oki_bank));
	save_item(NAME(m_gfx_control));
}

void _3x3puzzle_state::machine_reset()
{
	m_oki_bank = 0;
	m_gfx_control = 0;
}


void _3x3puzzle_state::_3x3puzzle(machine_config &config)
{
	constexpr XTAL MAIN_CLOCK = XTAL(10'000'000);

	// basic machine hardware
	M68000(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &_3x3puzzle_state::_3x3puzzle_map);
	m_maincpu->set_vblank_int("screen", FUNC(_3x3puzzle_state::irq4_line_hold));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(_3x3puzzle_state::screen_update));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_3x3puzzle);

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x600 / 2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, XTAL(4'000'000)/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 3x3puzzl )
	ROM_REGION( 0x80000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "1.bin",    0x000000, 0x20000, CRC(e9c39ee7) SHA1(8557eeaff33ac8e11fd545482bd9e48f9a58eba3) )
	ROM_LOAD16_BYTE( "2.bin",    0x000001, 0x20000, CRC(524963be) SHA1(05428ccc823c35b6c4d182a1dff1c9aa6b71e616) )

	ROM_REGION( 0x200000, "tiles1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("3.bin", 0x000000, 0x080000, CRC(53c2aa6a) SHA1(d2ebb10eb8311ff5226793e7b373e152b21c602c) )
	ROM_LOAD32_BYTE("4.bin", 0x000001, 0x080000, CRC(fb0b76fd) SHA1(23c9a5979452c21381107641d5cd49b34ad00471) )
	ROM_LOAD32_BYTE("5.bin", 0x000002, 0x080000, CRC(b6c1e108) SHA1(44c97e0582b9ee85465040d56eda9efd06c25533) )
	ROM_LOAD32_BYTE("6.bin", 0x000003, 0x080000, CRC(47cb0e8e) SHA1(13d39b1eb4fcfeafc8d821871750f37377449b80) )

	ROM_REGION( 0x080000, "tiles2", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("7.bin",  0x000003, 0x020000, CRC(45b1f58b) SHA1(7faee993b87ef14eef1c5ba3fcc7d0747494dbf5) )
	ROM_LOAD32_BYTE("8.bin",  0x000002, 0x020000, CRC(c0d404a7) SHA1(7991e711f6933bc3809f3c562e21a775a9a2dcf3) )
	ROM_LOAD32_BYTE("9.bin",  0x000001, 0x020000, CRC(6b303aa9) SHA1(1750da9148978f59904c2bf9e99f967e5bdd5a92) )
	ROM_LOAD32_BYTE("10.bin", 0x000000, 0x020000, CRC(6d0107bc) SHA1(a6f30a482586136304af510eee0a93df450673bd) )

	ROM_REGION( 0x080000, "tiles3", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("11.bin", 0x000003, 0x020000, CRC(e124c0b5) SHA1(9b819691b7ca7f0561f4fce05083c8507e138bfe) )
	ROM_LOAD32_BYTE("12.bin", 0x000002, 0x020000, CRC(ae4a8707) SHA1(f54337e4b666dc2f38df0ea96e9c8f4d7c4ebe52) )
	ROM_LOAD32_BYTE("13.bin", 0x000001, 0x020000, CRC(f06925d1) SHA1(a45b72da57c7d967dec2fcad12e6a7864f5442e8) )
	ROM_LOAD32_BYTE("14.bin", 0x000000, 0x020000, CRC(07252636) SHA1(00730c203e20af9e18a792e26de7aeac5d090ebf) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD("15.bin", 0x000000, 0x080000, CRC(d3aff355) SHA1(117f7bbd6cab370f65e308d78291732dfc079365) )
ROM_END

ROM_START( 3x3puzzla )
	ROM_REGION( 0x80000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "1a.bin",    0x000000, 0x20000, CRC(425c5896) SHA1(78d709b729f160b1e20a61a795361113dbb4fb52) )
	ROM_LOAD16_BYTE( "2a.bin",    0x000001, 0x20000, CRC(4db710b7) SHA1(df7a3496aac9cfdaee4fd504d88772b07a8fdb2b) )

	ROM_REGION( 0x200000, "tiles1", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("3a.bin", 0x000000, 0x080000, CRC(33bff952) SHA1(11d06462041e7cf8aa2ae422ed74ba23f6934478) )
	ROM_LOAD32_BYTE("4a.bin", 0x000001, 0x080000, CRC(222996d8) SHA1(fb8fd45a43e78dd9700ffb46fa886f62e7c32e61) )
	ROM_LOAD32_BYTE("5a.bin", 0x000002, 0x080000, CRC(5b209844) SHA1(8a34958ecd9c26272708237028e279dd347c729f) )
	ROM_LOAD32_BYTE("6a.bin", 0x000003, 0x080000, CRC(f1136bba) SHA1(486c41bd08c4039bfe46500540fc4ae0f1497232) )

	ROM_REGION( 0x080000, "tiles2", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("7.bin",  0x000003, 0x020000, CRC(45b1f58b) SHA1(7faee993b87ef14eef1c5ba3fcc7d0747494dbf5) )
	ROM_LOAD32_BYTE("8.bin",  0x000002, 0x020000, CRC(c0d404a7) SHA1(7991e711f6933bc3809f3c562e21a775a9a2dcf3) )
	ROM_LOAD32_BYTE("9.bin",  0x000001, 0x020000, CRC(6b303aa9) SHA1(1750da9148978f59904c2bf9e99f967e5bdd5a92) )
	ROM_LOAD32_BYTE("10.bin", 0x000000, 0x020000, CRC(6d0107bc) SHA1(a6f30a482586136304af510eee0a93df450673bd) )

	ROM_REGION( 0x080000, "tiles3", ROMREGION_ERASE00 )
	ROM_LOAD32_BYTE("11.bin", 0x000003, 0x020000, CRC(e124c0b5) SHA1(9b819691b7ca7f0561f4fce05083c8507e138bfe) )
	ROM_LOAD32_BYTE("12.bin", 0x000002, 0x020000, CRC(ae4a8707) SHA1(f54337e4b666dc2f38df0ea96e9c8f4d7c4ebe52) )
	ROM_LOAD32_BYTE("13.bin", 0x000001, 0x020000, CRC(f06925d1) SHA1(a45b72da57c7d967dec2fcad12e6a7864f5442e8) )
	ROM_LOAD32_BYTE("14.bin", 0x000000, 0x020000, CRC(07252636) SHA1(00730c203e20af9e18a792e26de7aeac5d090ebf) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD("15.bin", 0x000000, 0x080000, CRC(d3aff355) SHA1(117f7bbd6cab370f65e308d78291732dfc079365) )
ROM_END

ROM_START( casanova )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "casanova.u7", 0x00001, 0x40000, CRC(869c2bf2) SHA1(58d349fa92880b20e9a58c576002972e46cd7eb2) )
	ROM_LOAD16_BYTE( "casanova.u8", 0x00000, 0x40000, CRC(9df77f4b) SHA1(e2da1440406be715b349c9bf5263cb7bd8ef69d9) )

	ROM_REGION( 0x0c0000, "oki", 0 ) // Samples
	ROM_LOAD( "casanova.su2", 0x00000, 0x80000, CRC(84a8320e) SHA1(4d0b4120174b2aa726db8e324d5614e3f0cefe95) )
	ROM_LOAD( "casanova.su3", 0x80000, 0x40000, CRC(334a2d1a) SHA1(d3eb5627a711a78c52a1fdd573a7f91442ccfa49) )

	ROM_REGION( 0x400000, "tiles1", 0 )
	ROM_LOAD32_BYTE( "casanova.u23", 0x000000, 0x80000, CRC(4bd4e5b1) SHA1(13759d086ef2dba26129022bade12be11b81258e) )
	ROM_LOAD32_BYTE( "casanova.u25", 0x000001, 0x80000, CRC(5461811b) SHA1(03301c836ba378e527867de25ee15abd3a0434ac))
	ROM_LOAD32_BYTE( "casanova.u27", 0x000002, 0x80000, CRC(dd178379) SHA1(990109db9d0ce693cf7371109cb0d4745b8dde59))
	ROM_LOAD32_BYTE( "casanova.u29", 0x000003, 0x80000, CRC(36469f9e) SHA1(d4603bf99aef953e2eb49c1862d66961246e88c2) )
	ROM_LOAD32_BYTE( "casanova.u81", 0x200000, 0x80000, CRC(9eafd37d) SHA1(bc9e7a035849f23da48c9d923188c61188d93c43) )
	ROM_LOAD32_BYTE( "casanova.u83", 0x200001, 0x80000, CRC(9d4ce407) SHA1(949c7f329bd348beff4f14ac7b506c8aef212ad8) )
	ROM_LOAD32_BYTE( "casanova.u85", 0x200002, 0x80000, CRC(113c6e3a) SHA1(e90d78c4415d244004734a481501f8040f8aa468) )
	ROM_LOAD32_BYTE( "casanova.u87", 0x200003, 0x80000, CRC(61bd80f8) SHA1(13b93f2638c37a5dec5b4016c058f486f9cbadae) )

	ROM_REGION( 0x200000, "tiles2", 0 )
	ROM_LOAD32_BYTE( "casanova.u39", 0x000000, 0x80000, CRC(97d4095a) SHA1(4b1fde984025fae240bf64f812d67bc9cbf3a60c) )
	ROM_LOAD32_BYTE( "casanova.u41", 0x000001, 0x80000, CRC(95f67e82) SHA1(34b4350efbe22eb57871b009016adc2660842030) )
	ROM_LOAD32_BYTE( "casanova.u43", 0x000002, 0x80000, CRC(1462d7d6) SHA1(5637c2d0df5866b72d0c8804f23694fa5a025c8d) )
	ROM_LOAD32_BYTE( "casanova.u45", 0x000003, 0x80000, CRC(530d78bc) SHA1(56d6f593da9211d4785f35a9796d593beeb6b224) )

	ROM_REGION( 0x200000, "tiles3", 0 )
	ROM_LOAD32_BYTE( "casanova.u48", 0x000000, 0x80000, CRC(af9f59c5) SHA1(8620579045632ec6a4cd8fc4bff48428c94c8139) )
	ROM_LOAD32_BYTE( "casanova.u50", 0x000001, 0x80000, CRC(c73b5e98) SHA1(07d0be244aba084bd1ef099b547fe1c8e813cbeb) )
	ROM_LOAD32_BYTE( "casanova.u52", 0x000002, 0x80000, CRC(708f779c) SHA1(2272be3971d8983695f9fa7c840d94bdc0e4b0e6) )
	ROM_LOAD32_BYTE( "casanova.u54", 0x000003, 0x80000, CRC(e60bf0db) SHA1(503738b3b83a37ff812beed6c71e915072e5b10f) )
ROM_END

} // anonymous namespace


GAME( 1998, 3x3puzzl,  0,        _3x3puzzle, _3x3puzzle, _3x3puzzle_state, empty_init, ROT0, "Ace Enterprise", "3X3 Puzzle (Enterprise)", MACHINE_SUPPORTS_SAVE ) // 1998. 5. 28
GAME( 1998, 3x3puzzla, 3x3puzzl, _3x3puzzle, _3x3puzzle, _3x3puzzle_state, empty_init, ROT0, "Ace Enterprise", "3X3 Puzzle (Normal)",     MACHINE_SUPPORTS_SAVE ) // 1998. 5. 28
GAME( 199?, casanova,  0,        _3x3puzzle, casanova,   _3x3puzzle_state, empty_init, ROT0, "Promat",         "Casanova",                MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
