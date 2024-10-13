// license:BSD-3-Clause
// copyright-holders:

/*
Banpresto medal games with Seibu customs

Confirmed games (but there are probably several more):
Terebi Denwa Doraemon
Terebi Denwa Super Mario World
Terebi Denwa Thomas the Tank Engine and Friends
Mario Undoukai

The following is from Mario Undoukai PCB pics:

BS9001-2 main PCB + BS9001-A ROM PCB
Main components:
- MC68000P10 main CPU
- 16.000 MHz XTAL (near M68000)
- 8-dip bank
- SEI0160 Seibu custom
- SEI0181 Seibu custom
- SEI0200 Seibu custom
- SEI0211 Seibu custom
- SEI0220BP Seibu custom
- 14.31818 MHz XTAL (near Seibu video customs)
- OKI M6295
The audio section also has unpopulated spaces marked for a Z80, a YM2203 and a SEI01008


TODO:
- fix printer or hopper emulation (passes check at start but then fails when giving out tickets at game end for marioun,
  when coining up for tvdenwad)
- the GFX emulation was adapted from other drivers using the Seibu customs, it might need more adjustments
- verify Oki banking (needs someone who understands Japanese to check if speech makes sense when it gets called)
- lamps
- controls / dips need to be completed and better arranged
- identify and hookup RTC
*/

#include "emu.h"

#include "sei021x_sei0220_spr.h"
#include "seibu_crtc.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/rp5c01.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class banprestoms_state : public driver_device
{
public:
	banprestoms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_ticket(*this, "ticket")
		, m_rtc(*this, "rtc")
		, m_vram(*this, "vram%u", 0U)
		, m_spriteram(*this, "sprite_ram")
		, m_okibank(*this, "okibank")
	{ }

	void banprestoms(machine_config &config);

	void init_oki();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sei0211_device> m_spritegen;
	required_device<ticket_dispenser_device> m_ticket;
	required_device<lh5045_device> m_rtc;

	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr<uint16_t> m_spriteram;

	required_memory_bank m_okibank;

	tilemap_t *m_tilemap[4];

	uint16_t m_layer_en;
	uint16_t m_scrollram[6];

	void okibank_w(uint16_t data);

	template <uint8_t Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	template <uint8_t Which> TILE_GET_INFO_MEMBER(tile_info);

	uint32_t pri_cb(uint8_t pri, uint8_t ext);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void prg_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


void banprestoms_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x40000);
	m_okibank->set_entry(0);
}


void banprestoms_state::okibank_w(uint16_t data)
{
	m_okibank->set_entry(data & 0x03);

	m_ticket->motor_w(BIT(data, 4)); // bit 3 is suspect, too
	// TODO: what do the other bits do?
}


void banprestoms_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[3] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(banprestoms_state::tile_info<3>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tilemap[1]->set_transparent_pen(15);
	m_tilemap[2]->set_transparent_pen(15);
	m_tilemap[3]->set_transparent_pen(15);

	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}


template <uint8_t Which>
void banprestoms_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Which][offset]);
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(banprestoms_state::tile_info)
{
	int tile = m_vram[Which][tile_index] & 0xfff;
	int color = (m_vram[Which][tile_index] >> 12) & 0x0f;
	tileinfo.set(Which, tile, color, 0);
}

uint32_t banprestoms_state::pri_cb(uint8_t pri, uint8_t ext)
{
	switch (pri)
	{
		case 0: return GFX_PMASK_8;
		case 1: return GFX_PMASK_8 | GFX_PMASK_4;
		case 2: return GFX_PMASK_8 | GFX_PMASK_4 | GFX_PMASK_2;
		case 3:
		default: return 0;
	}
}

uint32_t banprestoms_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	m_tilemap[0]->set_scrollx(0, m_scrollram[0]);
	m_tilemap[0]->set_scrolly(0, m_scrollram[1] - 16);
	m_tilemap[1]->set_scrollx(0, m_scrollram[2]);
	m_tilemap[1]->set_scrolly(0, m_scrollram[3] - 16);
	m_tilemap[2]->set_scrollx(0, m_scrollram[4]);
	m_tilemap[2]->set_scrolly(0, m_scrollram[5] - 16);
	m_tilemap[3]->set_scrollx(0, 128);
	m_tilemap[3]->set_scrolly(0, -16);

	if (BIT(~m_layer_en, 0)) { m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 1); }
	if (BIT(~m_layer_en, 1)) { m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 2); }
	if (BIT(~m_layer_en, 2)) { m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 4); }
	if (BIT(~m_layer_en, 3)) { m_tilemap[3]->draw(screen, bitmap, cliprect, 0, 8); }
	if (BIT(~m_layer_en, 4)) { m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram, m_spriteram.bytes()); }

	return 0;
}

void banprestoms_state::layer_en_w(uint16_t data)
{
	m_layer_en = data;
}

void banprestoms_state::layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrollram[offset]);
}


void banprestoms_state::prg_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x080000, 0x0807ff).ram().share("nvram");
	map(0x080800, 0x080fff).ram().w(FUNC(banprestoms_state::vram_w<0>)).share(m_vram[0]);
	map(0x081000, 0x0817ff).ram().w(FUNC(banprestoms_state::vram_w<1>)).share(m_vram[1]);
	map(0x081800, 0x081fff).ram().w(FUNC(banprestoms_state::vram_w<2>)).share(m_vram[2]);
	map(0x082000, 0x082fff).ram().w(FUNC(banprestoms_state::vram_w<3>)).share(m_vram[3]);
	map(0x083000, 0x0837ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x083800, 0x083fff).ram().share(m_spriteram);
	map(0x0a0001, 0x0a0001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0c0000, 0x0c004f).rw("crtc", FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
	map(0x0c0080, 0x0c0081).nopw(); // CRTC related ?
	map(0x0c00c0, 0x0c00c1).nopw(); // CRTC related ?
	map(0x0c0100, 0x0c0101).w(FUNC(banprestoms_state::okibank_w));
//  map(0x0c0140, 0x0c0141).nopw(); // in marioun bit 3 is lamp according to test mode
	map(0x0e0000, 0x0e0001).portr("DSW1");
	map(0x0e0002, 0x0e0003).portr("IN1");
	map(0x0e0004, 0x0e0005).portr("IN2");

	// Expects a '1' when entering RTC test (RTC /BSY line?)
	map(0x0e0006, 0x0e0007).lr8(NAME([](offs_t offset) { return 1; }));
	map(0x100000, 0x10001f).rw(m_rtc, FUNC(lh5045_device::read), FUNC(lh5045_device::write)).umask16(0x00ff);
}

void banprestoms_state::oki_map(address_map &map)
{
	map(0x00000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( tvdenwad )
	PORT_START("IN1")
	// TODO: convert to keypad
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) // 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) // 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) // 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // 6
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) // 7
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON8 ) // 8
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON9 ) // 9
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON10 ) // 0, why active high?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON11 ) // #
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON12 ) // *
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED ) // couldn't find anything from here on
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON13 ) // Card Emp in switch test
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON14 ) // Card Pos in switch test
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r) // Card Pay in switch test
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED ) // ?
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // ?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON16 ) // Hook in switch test
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED ) // couldn't find anything from here on
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START("DSW1") // marked SW0913 on PCB
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "Every 3rd cycle" ) // 1st 2 cycles mute, then sound, then 2 cycles mute and so on
	PORT_DIPSETTING(    0x80, "Every 2nd cycle" ) // 1st cycle mute, then sound, then 1 cycle mute and so on
	PORT_DIPSETTING(    0xc0, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( marioun ) // inputs defined as IPT_UNKNOWN don't show any effect in switch test in test mode
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // upper space on the feet platform
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) // right space on the feet platform
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) // lower space on the feet platform
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) // left space on the feet platform
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) // left central space on the feet platform
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) // right central space on the feet platform
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0200, IP_ACTIVE_LOW ) // also used to navigate in test mode
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) // Card Emp
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON8 ) // Card Pos
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r) // Card Pay
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("DSW1") // marked SW0913 on PCB
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3") // some of these are difficulty (i.e. see Bowser being quicker or slower in the 100m dash)
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, "Every 4th cycle" ) // 1st cycle sound, then 3 cycles mute, then 1 cycle sound and so on
	PORT_DIPSETTING(    0x80, "Every 2nd cycle" ) // 1st cycle sound, then 1 cycle mute, then 1 cycle sound and so on
	PORT_DIPSETTING(    0xc0, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
				3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16 },
	128*8
};

static GFXDECODE_START( gfx_banprestoms )
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x100, 0x10 ) // TODO, doesn't seem to be used by the dumped games
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x100, 0x10 ) // TODO, doesn't seem to be used by the dumped games
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x100, 0x10 )
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x200, 0x10 )
GFXDECODE_END

static GFXDECODE_START( gfx_banprestoms_spr )
	GFXDECODE_ENTRY( "spr_gfx",0, tilelayout, 0x000, 0x40 )
GFXDECODE_END


void banprestoms_state::banprestoms(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &banprestoms_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(banprestoms_state::irq4_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(100)); // TODO: period is guessed

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: copied from other drivers using the same CRTC
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 320-1, 16, 240-1);
	screen.set_screen_update(FUNC(banprestoms_state::screen_update));
	screen.set_palette(m_palette);

	seibu_crtc_device &crtc(SEIBU_CRTC(config, "crtc", 0));
	crtc.layer_en_callback().set(FUNC(banprestoms_state::layer_en_w));
	crtc.layer_scroll_callback().set(FUNC(banprestoms_state::layer_scroll_w));

	LH5045(config, m_rtc, XTAL(32'768));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_banprestoms);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400); // TODO: copied from other drivers using the same CRTC

	SEI0211(config, m_spritegen, XTAL(14'318'181), m_palette, gfx_banprestoms_spr);
	m_spritegen->set_pri_callback(FUNC(banprestoms_state::pri_cb));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 1000000, okim6295_device::PIN7_HIGH)); // TODO: check frequency and pin 7.
	oki.set_addrmap(0, &banprestoms_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);
}


ROM_START( tvdenwad )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s82_a02.u15",  0x00000, 0x20000, CRC(479f790f) SHA1(a544c6493feb74ec8727f6b7f491ce6e5d24e316) )
	ROM_LOAD16_BYTE( "s82_a01.u14",  0x00001, 0x20000, CRC(ac7cccdb) SHA1(5c0877e1831663113aa3284e323bf7665b5baa71) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s82_a05.u119", 0x00000, 0x80000, CRC(980be413) SHA1(d35cb6bb2299fc34226c59c3c97f8789dd1f71ce) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s82_a04.u18", 0x00000, 0x80000, CRC(55f31697) SHA1(58011800b2e2c6cac55a3881a9970bbd325d74ad) ) // TODO: are all of the following used? in attract it seems only fg_gfx and tx_gfx are used

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s82_a03.u17", 0x000000, 0x100000, CRC(80014273) SHA1(9155248e9b7f8b7a9962b29b9063f9fe5ba471de) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

ROM_START( tvdenwam )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s40_b02.u15",  0x00000, 0x20000, CRC(9d633126) SHA1(5879d84ace23927fd85f6d32c7d94f0ff8d52927) )
	ROM_LOAD16_BYTE( "s40_b01.u14",  0x00001, 0x20000, CRC(72973fba) SHA1(584a7fd7e82a1797c134ffbb81cf256f79f6f915) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s40_a05.u119", 0x00000, 0x80000, CRC(bec55644) SHA1(e3a7ad626845d709bc6cd6a70bda6686b07c0c5f) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s40_a04.u18", 0x00000, 0x80000, CRC(49d21bb1) SHA1(42080305ed2125207427e37e260c655b79d7d170) )

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s40_a03.u17", 0x000000, 0x100000, CRC(55591e28) SHA1(b7edc7ff9f009805c16ca0d10f938540b30907e7) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

ROM_START( tvdenwat )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s72_b02.u15",  0x00000, 0x20000, CRC(db713202) SHA1(4883152273925207929757da0eae333ac18aa585) )
	ROM_LOAD16_BYTE( "s72_b01.u14",  0x00001, 0x20000, CRC(fe4e48ff) SHA1(2a31520ddd3c4a61daf2868114cfd1f1a5f3f201) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s72_a05.u119", 0x00000, 0x80000, CRC(9fda5789) SHA1(5d1e37d42c9f255253bcb3714be8fd04238b47fa) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s72_a04.u18", 0x00000, 0x80000, CRC(66fbbf66) SHA1(4d46204661282c311ace3c749d9484c2e3e69500) )

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s72_a03.u17", 0x000000, 0x100000, CRC(e7d09642) SHA1(0cee5193497698239f44ef457d92f1d1d042c42b) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

ROM_START( marioun )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s98_b02.u15",  0x00000, 0x20000, CRC(d88eecfc) SHA1(5a15a1f925ae10e439e5aee8f3ef5a2fa956b80f) )
	ROM_LOAD16_BYTE( "s98_b01.u14",  0x00001, 0x20000, CRC(60f5e7b0) SHA1(ccbdc42d0d59ec4f96ced682afd879c9fa9f24cc) )

	ROM_REGION( 0x80000, "spr_gfx", 0 )
	ROM_LOAD( "s98_a05.u119", 0x00000, 0x80000, CRC(b8317dd8) SHA1(37f0be38607e40d7925faf9731b95577cbd56bb0) )

	ROM_REGION( 0x80000, "gfx_tiles", 0 )
	ROM_LOAD( "s98_a04.u18", 0x00000, 0x80000, CRC(b107c5a0) SHA1(d4e7ef71bfb9a10e72b6292405d0378c95ebba25) ) // TODO: are all of the following used? in attract it seems only fg_gfx and tx_gfx are used

	ROM_REGION( 0x80000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x80000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x00000, 0x00000, 0x80000)

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "s98_a03.u17", 0x000000, 0x100000, CRC(d6b89223) SHA1(7d7ab34decb994caac82178455f014628cf070b8) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "sc001.u110", 0x000, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc002.u235", 0x200, 0x104, NO_DUMP ) // tibpal16l8-25cn
	ROM_LOAD( "sc003.u36",  0x400, 0x155, NO_DUMP ) // 18cv8pc-25
	ROM_LOAD( "sc004c.u68", 0x600, 0x117, NO_DUMP ) // gal16v8a
	ROM_LOAD( "sc006.u248", 0x800, 0x117, NO_DUMP ) // gal16v8a
ROM_END

void banprestoms_state::init_oki() // The Oki mask ROM is in an unusual format, load it so that MAME can make use of it
{
	uint8_t *okirom = memregion("oki")->base();
	std::vector<uint8_t> buffer(0x100000);
	memcpy(&buffer[0], okirom, 0x100000);

	for (int i = 0; i < 0x80000; i += 4)
		okirom[i / 2] = buffer[i];

	for (int i = 1; i < 0x80000; i += 4)
		okirom[(i - 1) / 2 + 0x40000] = buffer[i];

	for (int i = 2; i < 0x80000; i += 4)
		okirom[i / 2] = buffer[i];

	for (int i = 3; i < 0x80000; i += 4)
		okirom[(i - 1) / 2 + 0x40000] = buffer[i];

	for (int i = 0x80000; i < 0x100000; i += 4)
		okirom[i / 2 + 0x40000] = buffer[i];

	for (int i = 0x80001; i < 0x100000; i += 4)
		okirom[(i - 1) / 2 + 0x80000] = buffer[i];

	for (int i = 0x80002; i < 0x100000; i += 4)
		okirom[i / 2 + 0x40000] = buffer[i];

	for (int i = 0x80003; i < 0x100000; i += 4)
		okirom[(i - 1) / 2 + 0x80000] = buffer[i];
}

} // Anonymous namespace


GAME( 1991, tvdenwad, 0, banprestoms, tvdenwad, banprestoms_state, init_oki, ROT0, "Banpresto", "Terebi Denwa Doraemon",                            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, tvdenwam, 0, banprestoms, tvdenwad, banprestoms_state, init_oki, ROT0, "Banpresto", "Terebi Denwa Super Mario World",                   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1992, tvdenwat, 0, banprestoms, tvdenwad, banprestoms_state, init_oki, ROT0, "Banpresto", "Terebi Denwa Thomas the Tank Engine and Friends",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1993, marioun,  0, banprestoms, marioun,  banprestoms_state, init_oki, ROT0, "Banpresto", "Super Mario World - Mario Undoukai",               MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
