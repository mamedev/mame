// license:BSD-3-Clause
// copyright-holders:

/*
Exciting Fever by Cosmo

LS9310 PCB

MC68000P10 CPU
10.000 MHz XTAL
2x UM6264AK-10LL RAM
EPL16P8BP PLD
EPL10P8BP PLD
Xilinx XC3020-100 FPGA (on small riser board)
13.383 MHz XTAL
4x KM6264AL-10L RAM
2x M5M5178AP-25 RAM
YM2203C OPN
5x bank of 8 switches
reset button

TODO:
- correct bgtiles decode
- visible area
- missing tiles
- video registers
- remainings inputs (probably missing at least reels stop, payout and keyout)
- DIPs (DIPs definitions in test mode)
- outputs
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cosmo68k_state : public driver_device
{
public:
	cosmo68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgtileram(*this, "fgtileram%u", 0U),
		m_bgtileram(*this, "bgtileram%u", 0U)
	{ }

	void exfever(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr_array<uint16_t, 2> m_fgtileram;
	required_shared_ptr_array<uint16_t, 2> m_bgtileram;

	tilemap_t *m_fg_tilemap[2] {};
	tilemap_t *m_bg_tilemap[2] {};

	uint16_t m_display_ctrl = 0;

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_fgtile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_bgtile_info);
	template <uint8_t Which> void fgtileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template <uint8_t Which> void bgtileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void cosmo68k_state::video_start()
{
	m_fg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cosmo68k_state::get_fgtile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cosmo68k_state::get_fgtile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cosmo68k_state::get_bgtile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cosmo68k_state::get_bgtile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_bg_tilemap[1]->set_transparent_pen(0x0f);
	m_fg_tilemap[0]->set_transparent_pen(0x0f);
	m_fg_tilemap[1]->set_transparent_pen(0x0f);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(cosmo68k_state::get_fgtile_info)
{
	uint16_t const tile = m_fgtileram[Which][tile_index * 2];
	uint16_t const color = m_fgtileram[Which][tile_index * 2 + 1];

	tileinfo.set(0, tile, color, 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(cosmo68k_state::get_bgtile_info)
{
	uint16_t const tile = m_bgtileram[Which][tile_index * 2];
	uint16_t const color = m_bgtileram[Which][tile_index * 2 + 1];

	tileinfo.set(1, tile, color, 0);
}

template <uint8_t Which>
void cosmo68k_state::fgtileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fgtileram[Which][offset]);
	m_fg_tilemap[Which]->mark_tile_dirty(offset / 2);
}

template <uint8_t Which>
void cosmo68k_state::bgtileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bgtileram[Which][offset]);
	m_bg_tilemap[Which]->mark_tile_dirty(offset / 2);
}

uint32_t cosmo68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	// m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	// m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(m_display_ctrl, 11)) m_fg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	if (BIT(m_display_ctrl, 12)) m_fg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void cosmo68k_state::machine_start()
{
	save_item(NAME(m_display_ctrl));
}


void cosmo68k_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x040000, 0x043fff).ram(); // work RAM
	map(0x044000, 0x044001).portr("DSW1");
	map(0x044002, 0x044003).portr("DSW2");
	map(0x044004, 0x044005).portr("DSW3");
	map(0x044008, 0x044009).portr("IN0");
	map(0x04400a, 0x04400b).portr("IN1");
	map(0x04400c, 0x04400d).portr("IN2");
	map(0x044010, 0x044011).lw16(NAME([this] (uint16_t data) {  m_display_ctrl = data; if (data) logerror("%s display_ctrl_w: %02x\n", machine().describe_context(), data); }));
	map(0x044020, 0x044023).rw("ym", FUNC(ym2203_device::read), FUNC(ym2203_device::write)).umask16(0x00ff);
	map(0x046000, 0x046fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x047000, 0x047fff).ram(); // ??
	map(0x048000, 0x049fff).ram().w(FUNC(cosmo68k_state::fgtileram_w<0>)).share(m_fgtileram[0]); // VRAM FRONT 0 in RAM test
	map(0x04a000, 0x04bfff).ram().w(FUNC(cosmo68k_state::fgtileram_w<1>)).share(m_fgtileram[1]); // VRAM FRONT 1 in RAM test
	map(0x04c000, 0x04dfff).ram().w(FUNC(cosmo68k_state::bgtileram_w<0>)).share(m_bgtileram[0]); // VRAM BACK 0 in RAM test
	map(0x04e000, 0x04ffff).ram().w(FUNC(cosmo68k_state::bgtileram_w<1>)).share(m_bgtileram[1]); // VRAM BACK 1 in RAM test
}


static INPUT_PORTS_START( exfever )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_GAMBLE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Test ) ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_packed_msb, 0, 0x80 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_16x16x4_packed_msb, 0, 0x80 ) // TODO: wrong
GFXDECODE_END


void cosmo68k_state::exfever(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cosmo68k_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(cosmo68k_state::irq2_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(cosmo68k_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x800);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2203_device &ym(YM2203(config, "ym", 13'383'000 / 4)); // divider not verified, may use the other XTAL, too
	ym.irq_handler().set_inputline(m_maincpu, M68K_IRQ_1);
	ym.add_route(ALL_OUTPUTS, "mono", 1.00);
}


ROM_START( exfever )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ex-h_l4a_cosmo.l4", 0x00000, 0x20000, CRC(2b207aaf) SHA1(c89117e2d983928ee9d2e4dee3095f356edd9807) ) // 1xxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "ex-h_p4a_cosmo.p4", 0x00001, 0x20000, CRC(7170a6dc) SHA1(bbed14809e1b15c4edd9d0347946f29a1969b056) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "ex-h.h4", 0x00000, 0x80000, CRC(7c1303d1) SHA1(48d7033f1a05b188dd6608f8b35ec58018d5f44a) ) // handwritten label

	ROM_REGION( 0x100000, "bgtiles", 0 )
	ROM_LOAD16_BYTE( "a4_cosmo.a4", 0x00000, 0x80000, CRC(19a96be6) SHA1(a6324cac9200824f9cc9b59f8bd8f9495571265d) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "c4_cosmo.c4", 0x00001, 0x80000, CRC(41249567) SHA1(ee49fad6bc3a0cf809d118bb20a523c5f5fa3aaa) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 1993, exfever, 0, exfever, exfever, cosmo68k_state, empty_init, ROT0, "Cosmo", "Exciting Fever", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
