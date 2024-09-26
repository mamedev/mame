// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

PCB# -   ANIMAL-01  Cosmo Electronics Corporation
68000 + OSC 8MHz
YM2203 + YM3014 + OSC 12MHz
DIPSw 8-position x2
RAM - 6264 (x2), TC5588 (x2), CXK5814 (x2)
3.6V battery

driver by David Haywood and few bits by Pierpaolo Prazzoli

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/ymopn.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class pkscram_state : public driver_device
{
public:
	pkscram_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_scan_timer(*this, "scan_timer"),
		m_fgtilemap_ram(*this, "fgtilemap_ram"),
		m_mdtilemap_ram(*this, "mdtilemap_ram"),
		m_bgtilemap_ram(*this, "bgtilemap_ram")
	{ }

	void pkscramble(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void pkscramble_map(address_map &map) ATTR_COLD;

	void pkscramble_fgtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pkscramble_mdtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pkscramble_bgtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void pkscramble_output_w(uint16_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	void irqhandler(int state);

	uint32_t screen_update_pkscramble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_scan_timer;
	required_shared_ptr<uint16_t> m_fgtilemap_ram;
	required_shared_ptr<uint16_t> m_mdtilemap_ram;
	required_shared_ptr<uint16_t> m_bgtilemap_ram;

	uint16_t m_out = 0;
	uint8_t m_interrupt_line_active = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_md_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
};


enum { interrupt_scanline=192 };


void pkscram_state::pkscramble_fgtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgtilemap_ram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

void pkscram_state::pkscramble_mdtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mdtilemap_ram[offset]);
	m_md_tilemap->mark_tile_dirty(offset >> 1);
}

void pkscram_state::pkscramble_bgtilemap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgtilemap_ram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

// input bit 0x20 in port1 should stay low until bit 0x20 is written here, then
// it should stay high for some time (currently we cheat keeping the input always active)
void pkscram_state::pkscramble_output_w(uint16_t data)
{
	// OUTPUT
	// BIT
	// 0x0001 -> STL
	// 0x0002 -> SPL1
	// 0x0004 -> SPL2
	// 0x0008 -> SPL3
	// 0x0010 -> MSK
	// 0x0020 -> HPM
	// 0x0040 -> CNT1
	// 0x0080 -> CNT2
	// 0x0100 -> LED1
	// 0x0200 -> LED2
	// 0x0400 -> LED3
	// 0x0800 -> LED4
	// 0x1000 -> LED5

	// 0x2000 and 0x4000 are used too
	// 0x2000 -> vblank interrupt enable
	// 0x4000 -> set on every second frame - not used

	m_out = data;

	if (!(m_out & 0x2000) && m_interrupt_line_active)
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
		m_interrupt_line_active = 0;
	}

	machine().bookkeeping().coin_counter_w(0, data & 0x80);
}

void pkscram_state::pkscramble_map(address_map &map)
{
	map.global_mask(0x7ffff);
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x0400ff).ram().share("nvram");
	map(0x041000, 0x043fff).ram(); // main ram
	map(0x044000, 0x044fff).ram().w(FUNC(pkscram_state::pkscramble_fgtilemap_w)).share("fgtilemap_ram"); // fg tilemap
	map(0x045000, 0x045fff).ram().w(FUNC(pkscram_state::pkscramble_mdtilemap_w)).share("mdtilemap_ram"); // md tilemap (just a copy of fg?)
	map(0x046000, 0x046fff).ram().w(FUNC(pkscram_state::pkscramble_bgtilemap_w)).share("bgtilemap_ram"); // bg tilemap
	map(0x047000, 0x047fff).ram(); // unused
	map(0x048000, 0x048fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x049000, 0x049001).portr("DSW");
	map(0x049004, 0x049005).portr("INPUTS");
	map(0x049008, 0x049009).w(FUNC(pkscram_state::pkscramble_output_w));
	map(0x049010, 0x049011).nopw();
	map(0x049014, 0x049015).nopw();
	map(0x049018, 0x049019).nopw();
	map(0x04901c, 0x04901d).nopw();
	map(0x049020, 0x049021).nopw();
	map(0x04900c, 0x04900f).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write)).umask16(0x00ff);
	map(0x052086, 0x052087).nopw();
}


static INPUT_PORTS_START( pkscramble )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0003, "Level" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPSETTING(      0x0005, "5" )
	PORT_DIPSETTING(      0x0006, "6" )
	PORT_DIPSETTING(      0x0007, "7" )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0100, "Coin to Start" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x3800, 0x0800, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, "No Credit" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE( 0x8000, IP_ACTIVE_HIGH )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) // Kick
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) // Left
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) // Center
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // Right
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_CUSTOM ) // Hopper status
	PORT_DIPNAME( 0x0040, 0x0000, "Coin Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


TILE_GET_INFO_MEMBER(pkscram_state::get_bg_tile_info)
{
	int const tile  = m_bgtilemap_ram[tile_index*2];
	int const color = m_bgtilemap_ram[tile_index*2 + 1] & 0x7f;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(pkscram_state::get_md_tile_info)
{
	int const tile  = m_mdtilemap_ram[tile_index*2];
	int const color = m_mdtilemap_ram[tile_index*2 + 1] & 0x7f;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(pkscram_state::get_fg_tile_info)
{
	int const tile  = m_fgtilemap_ram[tile_index*2];
	int const color = m_fgtilemap_ram[tile_index*2 + 1] & 0x7f;

	tileinfo.set(0, tile, color, 0);
}

TIMER_DEVICE_CALLBACK_MEMBER(pkscram_state::scanline_callback)
{
	if (param == interrupt_scanline)
	{
		if (m_out & 0x2000)
			m_maincpu->set_input_line(1, ASSERT_LINE);
		m_scan_timer->adjust(m_screen->time_until_pos(param + 1), param+1);
		m_interrupt_line_active = 1;
	}
	else
	{
		if (m_interrupt_line_active)
			m_maincpu->set_input_line(1, CLEAR_LINE);
		m_scan_timer->adjust(m_screen->time_until_pos(interrupt_scanline), interrupt_scanline);
		m_interrupt_line_active = 0;
	}
}

void pkscram_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pkscram_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pkscram_state::get_md_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(pkscram_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8,32,32);

	m_md_tilemap->set_transparent_pen(15);
	m_fg_tilemap->set_transparent_pen(15);
}

uint32_t pkscram_state::screen_update_pkscramble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_md_tilemap->draw(screen, bitmap, cliprect, 0,0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 12, 8, 4, 0, 28, 24, 20, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( gfx_pkscram )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 0x80 )
GFXDECODE_END

void pkscram_state::irqhandler(int state)
{
	if(m_out & 0x10)
		m_maincpu->set_input_line(2, state ? ASSERT_LINE : CLEAR_LINE);
}

void pkscram_state::machine_start()
{
	save_item(NAME(m_out));
	save_item(NAME(m_interrupt_line_active));
}

void pkscram_state::machine_reset()
{
	m_out = 0;
	m_interrupt_line_active=0;
	m_scan_timer->adjust(m_screen->time_until_pos(interrupt_scanline), interrupt_scanline);
}

void pkscram_state::pkscramble(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pkscram_state::pkscramble_map);
	//m_maincpu->set_vblank_int("screen", FUNC(pkscram_state::irq1_line_hold)); /* only valid irq */

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, m_scan_timer).configure_generic(FUNC(pkscram_state::scanline_callback));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 24*8-1);
	m_screen->set_screen_update(FUNC(pkscram_state::screen_update_pkscramble));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x800);
	GFXDECODE(config, m_gfxdecode, "palette", gfx_pkscram);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 12000000/4));
	ymsnd.irq_handler().set(FUNC(pkscram_state::irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.60);
}


ROM_START( pkscram )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "pk1.6e", 0x00000, 0x10000, CRC(80e972e5) SHA1(cbbc6e1e3fbb65b40164e140f368d8fff85c1521) )
	ROM_LOAD16_BYTE( "pk2.6j", 0x00001, 0x10000, CRC(752c86d1) SHA1(2e0c669307bed6f9eab957b0e1316416e653a72f) )

	ROM_REGION( 0x40000, "gfx1", 0 ) /* gfx */
	ROM_LOAD16_BYTE( "pk3.1c", 0x00000, 0x20000, CRC(0b18f2bc) SHA1(32892589442884ba02a1c6059ecb94e4ef516b86) )
	ROM_LOAD16_BYTE( "pk4.1e", 0x00001, 0x20000, CRC(a232d993) SHA1(1b7b15cf0fabf3b2b2e429506a78ff4c08f4f7a5) )
ROM_END

} // anonymous namespace


GAME( 1993, pkscram, 0, pkscramble, pkscramble, pkscram_state, empty_init, ROT0, "Cosmo Electronics Corporation", "PK Scramble", MACHINE_SUPPORTS_SAVE )
