// license:BSD-3-Clause
// copyright-holders:

/*
PS-M3 PCB

TMP68301AF
16 MHz XTAL
24 MHz XTAL
6x TC55257DPL-70L RAM (2 near CPU, 2 near GFX ROMs, 2 near data (?) ROMs)
Oki M6295
84-pin chip stickered PS NO-266 (near GFX ROMs)
160-pin chip stickered PS NO-FS (near data (?) ROMs)
4x bank of 8 switches

TODO:
- IRQs are wrong and just stubbed enough to make it do something;
- stops after showing first card. IRQ problem or protection? Some suspect reads with checks;
- DIPs ports are correct but they don't affect the game / never change in the DIP test screen;
- fully recover password (checked from 0x1ce6 on. It starts with 19490817 but where are last two digits checked?)
*/

#include "emu.h"

#include "cpu/m68000/tmp68301.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class ps_m3_state : public driver_device
{
public:
	ps_m3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_vram(*this, "vram%u", 0U)
	{
	}

	void ps_m3(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr_array<uint16_t, 8> m_vram;

	tilemap_t *m_tilemap[8] {};
	uint8_t m_irq_source = 0;

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_alt_tile_info);
	template <uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void counters_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);
};


void ps_m3_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	// TODO
	m_tilemap[4] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<4>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[5] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<5>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[6] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<6>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tilemap[7] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ps_m3_state::get_alt_tile_info<7>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	for (int i = 1; i < 0x08; i++)
		m_tilemap[i]->set_transparent_pen(0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(ps_m3_state::get_tile_info)
{
	int const tile = m_vram[Which][tile_index * 2 + 0];
	int const color = (m_vram[Which][tile_index * 2 + 1] & 0xff00) >> 8;

	tileinfo.set(0, tile, color, 0);
}

// TODO
template <uint8_t Which>
TILE_GET_INFO_MEMBER(ps_m3_state::get_alt_tile_info)
{
	int const tile = m_vram[Which][tile_index * 2 + 1];
	//int const color = ; // TODO

	tileinfo.set(1, tile, 0, 0);
}

template <uint8_t Which>
void ps_m3_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset >> 1);
}

uint32_t ps_m3_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//bitmap.fill(m_palette->black_pen(), cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 0);
	//m_tilemap[4]->draw(screen, bitmap, cliprect, 0, 0);
	//m_tilemap[5]->draw(screen, bitmap, cliprect, 0, 0);
	//m_tilemap[6]->draw(screen, bitmap, cliprect, 0, 0);
	//m_tilemap[7]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void ps_m3_state::counters_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // COIN A
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3)); // COIN D

	if (data & 0xf6)
		logerror("unknown counters_w bits: %02x\n", data);
}


void ps_m3_state::program_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40ffff).ram();
	map(0x800000, 0x801fff).ram().w(FUNC(ps_m3_state::vram_w<0>)).share(m_vram[0]);
	map(0x802000, 0x803fff).ram().w(FUNC(ps_m3_state::vram_w<1>)).share(m_vram[1]);
	map(0x804000, 0x805fff).ram().w(FUNC(ps_m3_state::vram_w<2>)).share(m_vram[2]);
	map(0x806000, 0x807fff).ram().w(FUNC(ps_m3_state::vram_w<3>)).share(m_vram[3]);
	map(0x808000, 0x809fff).ram().w(FUNC(ps_m3_state::vram_w<4>)).share(m_vram[4]);
	map(0x80a000, 0x80bfff).ram().w(FUNC(ps_m3_state::vram_w<5>)).share(m_vram[5]);
	map(0x80c000, 0x80dfff).ram().w(FUNC(ps_m3_state::vram_w<6>)).share(m_vram[6]);
	map(0x80e000, 0x80ffff).ram().w(FUNC(ps_m3_state::vram_w<7>)).share(m_vram[7]);
	map(0x880000, 0x88002d).ram();
	map(0x88002f, 0x88002f).lr8(NAME([this] () -> uint16_t { return m_irq_source; }));
	map(0xa00000, 0xa03fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xa04000, 0xa1ffff).ram();
	map(0xa80013, 0xa80013).w(FUNC(ps_m3_state::counters_w));
	map(0xa80020, 0xa80021).portr("DSW1");
	map(0xa80022, 0xa80023).portr("DSW2");
	map(0xa80024, 0xa80025).portr("DSW3");
	map(0xa80026, 0xa80027).portr("DSW4");
	map(0xa80028, 0xa80029).portr("IN0");
	map(0xa8002a, 0xa8002b).portr("IN1");
	map(0xa8002c, 0xa8002d).portr("IN2");
	map(0xa8002e, 0xa8002f).portr("IN3");
}


static INPUT_PORTS_START( dreamcha )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // Start
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // right in pw screen
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) // Bet
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) // left in pw screen
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3) // Coin A
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(3) // keyout / payout ?
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(3) // Coin D
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(3) // book keeping screens. Also shows DIPs.
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4) // Service coin? registers same as coin A in bookkeeping. Also gets out of the password screen to the game
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(4)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static GFXDECODE_START( gfx_ps_m3 )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar, 0x1800, 0x10)
	GFXDECODE_ENTRY( "tiles2", 0, gfx_16x16x8_raw, 0, 16 ) // TODO: wrong
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(ps_m3_state::scanline_cb) // TODO: almost certainly incorrect
{
	int const scanline = param;

	if (scanline == 240)
	{
		m_maincpu->set_input_line(1, HOLD_LINE);
		m_irq_source = 2;
	}

	if (scanline == 0)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_irq_source = 0;
	}
}


void ps_m3_state::ps_m3(machine_config &config)
{
	// basic machine hardware
	tmp68301_device &maincpu(TMP68301(config, m_maincpu, 16_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &ps_m3_state::program_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(ps_m3_state::scanline_cb), "screen", 0, 1);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(ps_m3_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 1);
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ps_m3);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x2000); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( dreamcha )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fsq100.u2", 0x00000, 0x80000, CRC(06c1c0a2) SHA1(988b1ce596ab897e5464a495a2b1c72379d9ef1e) )
	ROM_LOAD16_BYTE( "fsi100.u1", 0x00001, 0x80000, CRC(3c6e1aa0) SHA1(0fc846445962e184db3beb6595009e670578c227) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD( "dhc0.u12", 0x000000, 0x80000, CRC(5c270895) SHA1(5b35d75348a1cafa2d787abf9f191404e287f14d) )
	ROM_LOAD( "dhc1.u13", 0x080000, 0x80000, CRC(8fae9189) SHA1(0ef9cb5343c75d716c555ea756abf67e011f332c) )
	ROM_LOAD( "dhc2.u14", 0x100000, 0x80000, CRC(2047b3c7) SHA1(8a978ec50858570414e5caba0e7e2d9454464927) )
	ROM_LOAD( "dhc3.u15", 0x180000, 0x80000, CRC(36686061) SHA1(dab490fafe886d05af1308d8ffd977e1aaf04ada) )

	ROM_REGION( 0x100000, "tiles2", 0 ) // TODO: verify ROM loading
	ROM_LOAD16_BYTE( "dhdc0.u16", 0x00000, 0x80000, CRC(f62cfca5) SHA1(76e89baebabe22b48db723d02504036cccc2286c) ) // FIXED BITS (0xxxxxxx). Is this correct? Dumps always the same
	ROM_LOAD16_BYTE( "dhdc1.u17", 0x00001, 0x80000, CRC(1b11fa22) SHA1(57af78e979a50201d89ebdefe076cf538fc1d2dd) )

	ROM_REGION( 0x100000, "oki", 0 ) // TODO: verify how it's banked and adjust ROM loading accordingly
	ROM_LOAD( "dhs0.u29", 0x00000, 0x80000, CRC(a0dccbd0) SHA1(5771e1729c33bd0851d566ac50de9e9b1ae8ae57) ) // 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "dhs1.u28", 0x80000, 0x80000, CRC(4ecb8245) SHA1(55bbd2fb1215d1b4b8d53beb08f901b213922ff8) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF

	//ROM_REGION( 0x4000, "nvram", ROMREGION_ERASE00 ) // to bypass license expiration
	// TODO
ROM_END

} // anonymous namespace


// possibly Dream Chance
GAME( 2004, dreamcha, 0, ps_m3, dreamcha, ps_m3_state, empty_init, ROT0, "Paradise", "Dream Chance", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
