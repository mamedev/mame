// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************************************************************

   Super Duck (c) 1992 Comad

   hardware appears to be roughly based off Bionic Commando, close to the
   Tiger Road / F1-Dream based Pushman / Bouncing Balls.


PCB Clocks as measured:

Crystal 1: 8mhz
Crystal 2: 24mhz

All clock timing comes from crystal 1
 68k - 8mhz
 Z80 - 2mhz
 OKI M6295 - 1mhz

There are known flyers that show an alternate title of HELL OUT
 The title Hell Out fits the story line better and is likely the original title
 of the game with Super Duck being a title change for a specific country region.

 The graphics ROMs contain tiles for the HELL OUT title, but with the current
 ROM set there is no known way to display the alternate title.

*********************************************************************************/


#include "emu.h"

#include "tigeroad_spr.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "video/bufsprite.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_UNKWRITE     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE)

#include "logmacro.h"

#define LOGUNKWRITE(...)     LOGMASKED(LOG_UNKWRITE,     __VA_ARGS__)


namespace {

class supduck_state : public driver_device
{
public:
	supduck_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_spriteram(*this, "spriteram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_text_videoram(*this, "textvideoram")
		, m_fore_videoram(*this, "forevideoram")
		, m_back_videoram(*this, "backvideoram")
		, m_okibank(*this, "okibank")
	{ }

	void supduck(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;

	// shared pointers
	required_shared_ptr<uint16_t> m_text_videoram;
	required_shared_ptr<uint16_t> m_fore_videoram;
	required_shared_ptr<uint16_t> m_back_videoram;
	required_memory_bank m_okibank;

	tilemap_t *m_text_tilemap = nullptr;
	tilemap_t *m_fore_tilemap = nullptr;
	tilemap_t *m_back_tilemap = nullptr;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void text_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fore_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void back_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void coincounter_w(uint8_t data);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);

	void okibank_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_fore_tile_info);
	TILE_GET_INFO_MEMBER(get_back_tile_info);

};



TILEMAP_MAPPER_MEMBER(supduck_state::tilemap_scan)
{
	// where does each page start?
	int pagesize = 0x8 * 0x8;

	int offset = ((col & ~0x7) / 0x8) * (pagesize);
	offset += ((row ^ 0x3f) & 0x7) * 0x8;
	offset += col & 0x7;

	offset &= 0x3ff;

	offset += (((row ^ 0x3f) & ~0x7) / 0x8) * 0x400;


	return offset;
}

void supduck_state::video_start()
{
	m_text_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supduck_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fore_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supduck_state::get_fore_tile_info)), tilemap_mapper_delegate(*this, FUNC(supduck_state::tilemap_scan)), 32, 32, 128, 64);
	m_back_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(supduck_state::get_back_tile_info)), tilemap_mapper_delegate(*this, FUNC(supduck_state::tilemap_scan)), 32, 32, 128, 64);

	m_text_tilemap->set_transparent_pen(0x3);
	m_fore_tilemap->set_transparent_pen(0xf);

	m_text_tilemap->set_scrolldx(128, 128);
	m_text_tilemap->set_scrolldy(  6,   6);
	m_fore_tilemap->set_scrolldx(128, 128);
	m_fore_tilemap->set_scrolldy(  6,   6);
	m_back_tilemap->set_scrolldx(128, 128);
	m_back_tilemap->set_scrolldy(  6,   6);
}

uint32_t supduck_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_back_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fore_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	m_spritegen->draw_sprites(bitmap, cliprect, m_spriteram->buffer(), m_spriteram->bytes(), flip_screen(), true);

	m_text_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void supduck_state::text_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_text_videoram[offset]);
	m_text_tilemap->mark_tile_dirty(offset);
}

void supduck_state::fore_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fore_videoram[offset]);
	m_fore_tilemap->mark_tile_dirty(offset);
}

void supduck_state::back_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_back_videoram[offset]);
	m_back_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(supduck_state::get_text_tile_info) // same as tigeroad.cpp
{
	int data = m_text_videoram[tile_index];
	int attr = data >> 8;
	int code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
	int color = attr & 0x0f;
	int flags = (attr & 0x10) ? TILE_FLIPY : 0;

	tileinfo.set(0, code, color, flags);
}

TILE_GET_INFO_MEMBER(supduck_state::get_fore_tile_info)
{
	int data = m_fore_videoram[tile_index];
	int code = data & 0xff;
	if (data & 0x4000) code |= 0x100;
	if (data & 0x8000) code |= 0x200;

	int color = (data & 0x0f00) >> 8;
	int flags = (data & 0x2000) ? TILE_FLIPX : 0;
		flags |=(data & 0x1000) ? TILE_FLIPY : 0;


	tileinfo.set(1, code, color, flags);
}

TILE_GET_INFO_MEMBER(supduck_state::get_back_tile_info)
{
	int data = m_back_videoram[tile_index];

	int code = data & 0xff;
	if (data & 0x4000) code |= 0x100;
	if (data & 0x8000) code |= 0x200;

	int color = (data & 0x0f00) >> 8;
	int flags = (data & 0x2000) ? TILE_FLIPX : 0;
		flags |=(data & 0x1000) ? TILE_FLIPY : 0;

	tileinfo.set(2, code, color, flags);
}



void supduck_state::coincounter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 7));

	if (data != 0)
		LOGUNKWRITE("unknown write at 0xfe4000: %02x\n", data & 0x3f);
}


void supduck_state::scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	data &= mem_mask;

	switch (offset)
	{
	case 0:
		m_back_tilemap->set_scrollx(0, data);
		break;
	case 1:
		m_back_tilemap->set_scrolly(0, -data - 32 * 8);
		break;
	case 2:
		m_fore_tilemap->set_scrollx(0, data);
		break;
	case 3:
		m_fore_tilemap->set_scrolly(0, -data - 32 * 8);
		break;
	}
}



void supduck_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().nopw();
	map(0xfe0000, 0xfe1fff).ram().share("spriteram");

	map(0xfe4000, 0xfe4001).portr("P1_P2");
	map(0xfe4000, 0xfe4000).w(FUNC(supduck_state::coincounter_w));
	map(0xfe4002, 0xfe4003).portr("SYSTEM");
	map(0xfe4002, 0xfe4002).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0xfe4004, 0xfe4005).portr("DSW");

	map(0xfe8000, 0xfe8007).w(FUNC(supduck_state::scroll_w));
	map(0xfe800e, 0xfe800f).nopw(); // watchdog or irqack

	map(0xfec000, 0xfecfff).ram().w(FUNC(supduck_state::text_videoram_w)).share(m_text_videoram);
	map(0xff0000, 0xff3fff).ram().w(FUNC(supduck_state::back_videoram_w)).share(m_back_videoram);
	map(0xff4000, 0xff7fff).ram().w(FUNC(supduck_state::fore_videoram_w)).share(m_fore_videoram);
	map(0xff8000, 0xff87ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffc000, 0xffffff).ram(); // working RAM
}

void supduck_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(supduck_state::okibank_w));
	map(0x9800, 0x9800).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void supduck_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

void supduck_state::okibank_w(uint8_t data)
{
	// bit 0x80 is written on startup?

	m_okibank->set_entry(data & 0x03);
}


static INPUT_PORTS_START( supduck )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // not sure, probably wrong
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP-A:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DIP-A:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Game Sound" ) PORT_DIPLOCATION("DIP-A:6") // Kills all sounds except for Coin-In
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x00c0, 0x0080, DEF_STR( Lives ) ) PORT_DIPLOCATION("DIP-A:7,8")
	PORT_DIPSETTING(      0x00c0, "2" )
	PORT_DIPSETTING(      0x0080, "3" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Character Test" ) PORT_DIPLOCATION("DIP-B:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "DIP-B:8" )
INPUT_PORTS_END


static const gfx_layout vramlayout=
{
	8,8,    // 8*8 characters
	RGN_FRAC(1,1),   // 1024 character
	2,      // 2 bitplanes
	{ 4,0 },
	{ STEP4(0,1), STEP4(4*2,1) },
	{ STEP8(0,4*2*2) },
	128   // every character takes 128 consecutive bytes
};

// same as the ROM tilemap layout from tigeroad
static const gfx_layout tile_layout =
{
	32, 32,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ STEP4(0,1),        STEP4(4*2,1),          STEP4(4*2*2*32,1), STEP4(4*2*2*32+4*2,1),
	  STEP4(4*2*2*64,1), STEP4(4*2*2*64+4*2,1), STEP4(4*2*2*96,1), STEP4(4*2*2*96+4*2,1) },
	{ STEP32(0,4*2*2) },
	256 * 8
};


static GFXDECODE_START( gfx_supduck )
	GFXDECODE_ENTRY( "chars",   0, vramlayout,  768, 64 )    // colors 768-1023
	GFXDECODE_ENTRY( "fgtiles", 0, tile_layout,   0, 16 )    // colors   0-  63
	GFXDECODE_ENTRY( "bgtiles", 0, tile_layout, 256, 16 )    // colors 256- 319
GFXDECODE_END


void supduck_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("okibank")->base(), 0x20000);
	m_okibank->set_entry(0);
}


void supduck_state::supduck(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(8'000'000)); // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &supduck_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(supduck_state::irq2_line_hold)); // 2 & 4?

	Z80(config, m_audiocpu, XTAL(8'000'000) / 4); // 2MHz - verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &supduck_state::sound_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6000000, 384, 128, 0, 262, 22, 246); // hsync is 50..77, vsync is 257..259
	screen.set_screen_update(FUNC(supduck_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_supduck);

	TIGEROAD_SPRITE(config, m_spritegen, 0);
	m_spritegen->set_palette(m_palette);
	m_spritegen->set_color_base(512);    // colors 512- 767

	PALETTE(config, m_palette).set_format(palette_device::xRGBRRRRGGGGBBBB_bit4, 0x800 / 2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(8'000'000) / 8, okim6295_device::PIN7_HIGH)); // 1MHz - Verified on PCB, pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki.set_addrmap(0, &supduck_state::oki_map);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( supduck )
	ROM_REGION( 0x40000, "maincpu", 0 )      // 68000 code
	ROM_LOAD16_BYTE( "5.u16n", 0x00000, 0x20000, CRC(837a559a) SHA1(ed5ad744a4145dfbef56ad2e6eec3ff14c20de1c) )
	ROM_LOAD16_BYTE( "6.u16l", 0x00001, 0x20000, CRC(508e9905) SHA1(2da3f12caa29066b4d54b22573cfdfcea8916f99) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.su6",  0x00000, 0x8000, CRC(d75863ea) SHA1(497d11b86f4f69134943fc3448d195c6e7acbe8f) )

	ROM_REGION( 0x08000, "chars", 0 )
	ROM_LOAD( "3.cu15",   0x00000, 0x8000, CRC(b1cacca4) SHA1(b4a486618197cf2b85a121b5640cd773b2d453fc) )

	ROM_REGION( 0x80000, "fgtiles", 0 )
	ROM_LOAD( "7.uu29",   0x00000, 0x20000, CRC(f3251b20) SHA1(8ebb9b98324de14356c9a57ae8a77dc4118fb5c2) )
	ROM_LOAD( "8.uu30",   0x20000, 0x20000, CRC(03c60cbd) SHA1(bf3be7161f69187350eb9d9d4209b93f8b67d0f1) )
	ROM_LOAD( "9.uu31",   0x40000, 0x20000, CRC(9b6d3430) SHA1(ade2decc5bcf817498b1198a2244d1c65bc20bea) )
	ROM_LOAD( "10.uu32",  0x60000, 0x20000, CRC(beed2616) SHA1(c077a3de4a6d451a568694ab70e85830d585a41d) )

	ROM_REGION( 0x80000, "bgtiles", 0 )
	ROM_LOAD( "11.ul29",   0x00000, 0x20000, CRC(1b6958a4) SHA1(ca93f898702e14ece24d5cfced38d622d3596d0f) )
	ROM_LOAD( "12.ul30",   0x20000, 0x20000, CRC(3e6bd24b) SHA1(f93b5c78d815bd30ecb9cfe2cd257548e467e852) )
	ROM_LOAD( "13.ul31",   0x40000, 0x20000, CRC(bff7b7cd) SHA1(2f65cadcfcc02fe31ba721eea9f45d4a729e4374) )
	ROM_LOAD( "14.ul32",   0x60000, 0x20000, CRC(97a7310b) SHA1(76b82bfea64b59890c0ba2e1688b7321507a4da7) )

	ROM_REGION( 0x80000, "spritegen", 0 )
	ROM_LOAD32_BYTE( "15.u1d",   0x00000, 0x20000, CRC(81bf1f27) SHA1(7a66630a2da85387904917d3c136880dffcb9649) )
	ROM_LOAD32_BYTE( "16.u2d",   0x00001, 0x20000, CRC(9573d6ec) SHA1(9923be782bae47c49913d01554bcf3e5efb5395b) )
	ROM_LOAD32_BYTE( "17.u1c",   0x00002, 0x20000, CRC(21ef14d4) SHA1(66e389aaa1186921a07da9a9a9eda88a1083ad42) )
	ROM_LOAD32_BYTE( "18.u2c",   0x00003, 0x20000, CRC(33dd0674) SHA1(b95dfcc16d939bac77f338b8a8cada19328a1993) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.su12",   0x00000, 0x20000, CRC(745d42fb) SHA1(f9aee3ddbad3cc2f3a7002ee0d762eb041967e1e) ) // static sample data

	ROM_REGION( 0x80000, "okibank", 0 )
	ROM_LOAD( "1.su13",   0x00000, 0x80000, CRC(7fb1ed42) SHA1(77ec86a6454398e329066aa060e9b6a39085ce71) ) // banked sample data
ROM_END

} // anonymous namespace


GAME( 1992, supduck, 0, supduck, supduck, supduck_state, empty_init, ROT0, "Comad", "Super Duck", MACHINE_SUPPORTS_SAVE )
