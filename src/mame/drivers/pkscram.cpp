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
#include "sound/2203intf.h"
#include "machine/nvram.h"


class pkscram_state : public driver_device
{
public:
	pkscram_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_pkscramble_fgtilemap_ram(*this, "fgtilemap_ram"),
		m_pkscramble_mdtilemap_ram(*this, "mdtilemap_ram"),
		m_pkscramble_bgtilemap_ram(*this, "bgtilemap_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	UINT16 m_out;
	UINT8 m_interrupt_line_active;
	required_shared_ptr<UINT16> m_pkscramble_fgtilemap_ram;
	required_shared_ptr<UINT16> m_pkscramble_mdtilemap_ram;
	required_shared_ptr<UINT16> m_pkscramble_bgtilemap_ram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE16_MEMBER(pkscramble_fgtilemap_w);
	DECLARE_WRITE16_MEMBER(pkscramble_mdtilemap_w);
	DECLARE_WRITE16_MEMBER(pkscramble_bgtilemap_w);
	DECLARE_WRITE16_MEMBER(pkscramble_output_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_md_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_pkscramble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
};


enum { interrupt_scanline=192 };


WRITE16_MEMBER(pkscram_state::pkscramble_fgtilemap_w)
{
	COMBINE_DATA(&m_pkscramble_fgtilemap_ram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(pkscram_state::pkscramble_mdtilemap_w)
{
	COMBINE_DATA(&m_pkscramble_mdtilemap_ram[offset]);
	m_md_tilemap->mark_tile_dirty(offset >> 1);
}

WRITE16_MEMBER(pkscram_state::pkscramble_bgtilemap_w)
{
	COMBINE_DATA(&m_pkscramble_bgtilemap_ram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset >> 1);
}

// input bit 0x20 in port1 should stay low until bit 0x20 is written here, then
// it should stay high for some time (currently we cheat keeping the input always active)
WRITE16_MEMBER(pkscram_state::pkscramble_output_w)
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

	coin_counter_w(machine(), 0, data & 0x80);
}

static ADDRESS_MAP_START( pkscramble_map, AS_PROGRAM, 16, pkscram_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ffff)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x040000, 0x0400ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x041000, 0x043fff) AM_RAM // main ram
	AM_RANGE(0x044000, 0x044fff) AM_RAM_WRITE(pkscramble_fgtilemap_w) AM_SHARE("fgtilemap_ram") // fg tilemap
	AM_RANGE(0x045000, 0x045fff) AM_RAM_WRITE(pkscramble_mdtilemap_w) AM_SHARE("mdtilemap_ram") // md tilemap (just a copy of fg?)
	AM_RANGE(0x046000, 0x046fff) AM_RAM_WRITE(pkscramble_bgtilemap_w) AM_SHARE("bgtilemap_ram") // bg tilemap
	AM_RANGE(0x047000, 0x047fff) AM_RAM // unused
	AM_RANGE(0x048000, 0x048fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x049000, 0x049001) AM_READ_PORT("DSW")
	AM_RANGE(0x049004, 0x049005) AM_READ_PORT("INPUTS")
	AM_RANGE(0x049008, 0x049009) AM_WRITE(pkscramble_output_w)
	AM_RANGE(0x049010, 0x049011) AM_WRITENOP
	AM_RANGE(0x049014, 0x049015) AM_WRITENOP
	AM_RANGE(0x049018, 0x049019) AM_WRITENOP
	AM_RANGE(0x04901c, 0x04901d) AM_WRITENOP
	AM_RANGE(0x049020, 0x049021) AM_WRITENOP
	AM_RANGE(0x04900c, 0x04900f) AM_DEVREADWRITE8("ymsnd", ym2203_device, read, write, 0x00ff)
	AM_RANGE(0x052086, 0x052087) AM_WRITENOP
ADDRESS_MAP_END


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
	PORT_BIT( 0x0020, IP_ACTIVE_LOW,  IPT_SPECIAL ) // Hopper status
	PORT_DIPNAME( 0x0040, 0x0000, "Coin Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


TILE_GET_INFO_MEMBER(pkscram_state::get_bg_tile_info)
{
	int tile  = m_pkscramble_bgtilemap_ram[tile_index*2];
	int color = m_pkscramble_bgtilemap_ram[tile_index*2 + 1] & 0x7f;

	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

TILE_GET_INFO_MEMBER(pkscram_state::get_md_tile_info)
{
	int tile  = m_pkscramble_mdtilemap_ram[tile_index*2];
	int color = m_pkscramble_mdtilemap_ram[tile_index*2 + 1] & 0x7f;

	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

TILE_GET_INFO_MEMBER(pkscram_state::get_fg_tile_info)
{
	int tile  = m_pkscramble_fgtilemap_ram[tile_index*2];
	int color = m_pkscramble_fgtilemap_ram[tile_index*2 + 1] & 0x7f;

	SET_TILE_INFO_MEMBER(0,tile,color,0);
}

TIMER_DEVICE_CALLBACK_MEMBER(pkscram_state::scanline_callback)
{
	if (param == interrupt_scanline)
	{
		if (m_out & 0x2000)
			m_maincpu->set_input_line(1, ASSERT_LINE);
		timer.adjust(m_screen->time_until_pos(param + 1), param+1);
		m_interrupt_line_active = 1;
	}
	else
	{
		if (m_interrupt_line_active)
			m_maincpu->set_input_line(1, CLEAR_LINE);
		timer.adjust(m_screen->time_until_pos(interrupt_scanline), interrupt_scanline);
		m_interrupt_line_active = 0;
	}
}

void pkscram_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pkscram_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_md_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pkscram_state::get_md_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,32,32);
	m_fg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(pkscram_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8,32,32);

	m_md_tilemap->set_transparent_pen(15);
	m_fg_tilemap->set_transparent_pen(15);
}

UINT32 pkscram_state::screen_update_pkscramble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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


static GFXDECODE_START( pkscram )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 0x80 )
GFXDECODE_END

WRITE_LINE_MEMBER(pkscram_state::irqhandler)
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
	timer_device *scanline_timer = machine().device<timer_device>("scan_timer");
	scanline_timer->adjust(m_screen->time_until_pos(interrupt_scanline), interrupt_scanline);
}

static MACHINE_CONFIG_START( pkscramble, pkscram_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000 )
	MCFG_CPU_PROGRAM_MAP(pkscramble_map)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", pkscram_state,  irq1_line_hold) /* only valid irq */

	MCFG_NVRAM_ADD_0FILL("nvram")


	MCFG_TIMER_DRIVER_ADD("scan_timer", pkscram_state, scanline_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 24*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pkscram_state, screen_update_pkscramble)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pkscram)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 12000000/4)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(pkscram_state, irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END


ROM_START( pkscram )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "pk1.6e", 0x00000, 0x10000, CRC(80e972e5) SHA1(cbbc6e1e3fbb65b40164e140f368d8fff85c1521) )
	ROM_LOAD16_BYTE( "pk2.6j", 0x00001, 0x10000, CRC(752c86d1) SHA1(2e0c669307bed6f9eab957b0e1316416e653a72f) )

	ROM_REGION( 0x40000, "gfx1", 0 ) /* gfx */
	ROM_LOAD16_BYTE( "pk3.1c", 0x00000, 0x20000, CRC(0b18f2bc) SHA1(32892589442884ba02a1c6059ecb94e4ef516b86) )
	ROM_LOAD16_BYTE( "pk4.1e", 0x00001, 0x20000, CRC(a232d993) SHA1(1b7b15cf0fabf3b2b2e429506a78ff4c08f4f7a5) )
ROM_END


GAME( 1993, pkscram, 0, pkscramble, pkscramble, driver_device, 0, ROT0, "Cosmo Electronics Corporation", "PK Scramble", MACHINE_SUPPORTS_SAVE)
