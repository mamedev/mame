// license:BSD-3-Clause
// copyright-holders: David Haywood

// Aquarium (c)1996 Excellent Systems

/*

AQUARIUM
EXCELLENT SYSTEMS
ES-9206
+------------------------------------------------+
|                  6116      23C16000   23C16000*|
|         YM2151   6116      23C16000*  23C16000*|
|                    +-------+ +-------+         |
|     1.056MHz M6295 |ES-9303| |ES 9207|         |
|YM3012              |       | |       | AS7C256 |
|                    +-------+ +-------+ AS7C256 |
|J                                       AS7C256 |
|A                    4         AS7C256  AS7C256 |
|M                Z80B   32MHz                   |
|M                5      68000P-16  14.318MHz    |
|A                    PAL   AS7C256      AS7C256 |
|                        +-------+       AS7C256 |
|         PAL            |ES-9208|  PB1       3  |
|     1   PAL  6         |       |   8   27C4096*|
|                        +-------+               |
|SW4*SW3* SW2 SW1    AS7C256       2        7    |
+------------------------------------------------+

   CPU: TMP68HC000P-16
 Sound: Z0840006PSC Z80B
        OKI M6295, YM2151 & YM3012 DAC
   OSC: 32MHz, 14.31818MHz & 1056kHz resonator
Custom: EXCELLENT SYSTEM ES-9208 347101 (QFP160)
        EXCELLENT SYSTEM LTD. ES 9207 9343 T (QFP208)
        ES-9303 EXCELLENT 9338 C001 (QFP120)
 Other: PB1 - Push button reset

* Denotes unpopulated components


Notes:
- A bug in the program code causes the OKI to be reset on the very
  first coin inserted.

// Sound banking + video references
// https://www.youtube.com/watch?v=nyAQPrkt_a4
// https://www.youtube.com/watch?v=0gn2Kj2M46Q


*/

#include "emu.h"

#include "excellent_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/mb3773.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_AUDIOBANK (1U << 1)
#define LOG_OKI       (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_AUDIOBANK | LOG_OKI)

#include "logmacro.h"

#define LOGAUDIOBANK(...) LOGMASKED(LOG_AUDIOBANK, __VA_ARGS__)
#define LOGOKI(...)       LOGMASKED(LOG_OKI,       __VA_ARGS__)


namespace {

class aquarium_state : public driver_device
{
public:
	aquarium_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mid_videoram(*this, "mid_videoram"),
		m_bak_videoram(*this, "bak_videoram"),
		m_txt_videoram(*this, "txt_videoram"),
		m_scroll(*this, "scroll"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog")
	{ }

	void init_aquarium();

	void aquarium(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_mid_videoram;
	required_shared_ptr<u16> m_bak_videoram;
	required_shared_ptr<u16> m_txt_videoram;
	required_shared_ptr<u16> m_scroll;
	required_memory_bank m_audiobank;

	// video-related
	tilemap_t  *m_txt_tilemap = nullptr;
	tilemap_t  *m_mid_tilemap = nullptr;
	tilemap_t  *m_bak_tilemap = nullptr;
	std::unique_ptr<u8[]> m_decoded_gfx[2];

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<excellent_spr_device> m_sprgen;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<mb3773_device> m_watchdog;

	void watchdog_w(u8 data);
	void z80_bank_w(u8 data);
	u8 oki_r();
	void oki_w(u8 data);

	std::unique_ptr<u8[]> expand_gfx(int low, int hi);
	void txt_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void mid_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void bak_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	TILE_GET_INFO_MEMBER(get_mid_tile_info);
	TILE_GET_INFO_MEMBER(get_bak_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u8 snd_bitswap(u8 scrambled_data);
	void aquarium_colpri_cb(u32 &colour, u32 &pri_mask);

	void main_map(address_map &map) ATTR_COLD;
	void snd_map(address_map &map) ATTR_COLD;
	void snd_portmap(address_map &map) ATTR_COLD;
};


// TXT Layer
TILE_GET_INFO_MEMBER(aquarium_state::get_txt_tile_info)
{
	const u32 tileno = (m_txt_videoram[tile_index] & 0x0fff);
	const u32 colour = (m_txt_videoram[tile_index] & 0xf000) >> 12;
	tileinfo.set(0, tileno, colour, 0);

	tileinfo.category = (m_txt_videoram[tile_index] & 0x8000) >> 15;
}

void aquarium_state::txt_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txt_videoram[offset]);
	m_txt_tilemap->mark_tile_dirty(offset);
}

// MID Layer
TILE_GET_INFO_MEMBER(aquarium_state::get_mid_tile_info)
{
	const u32 tileno = (m_mid_videoram[tile_index * 2] & 0x0fff);
	const u32 colour = (m_mid_videoram[tile_index * 2 + 1] & 0x001f);
	const int flag = TILE_FLIPYX((m_mid_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	tileinfo.set(1, tileno, colour, flag);

	tileinfo.category = (m_mid_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

void aquarium_state::mid_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_mid_videoram[offset]);
	m_mid_tilemap->mark_tile_dirty(offset / 2);
}

// BAK Layer
TILE_GET_INFO_MEMBER(aquarium_state::get_bak_tile_info)
{
	const u32 tileno = (m_bak_videoram[tile_index * 2] & 0x0fff);
	const u32 colour = (m_bak_videoram[tile_index * 2 + 1] & 0x001f);
	const int flag = TILE_FLIPYX((m_bak_videoram[tile_index * 2 + 1] & 0x300) >> 8);

	tileinfo.set(2, tileno, colour, flag);

	tileinfo.category = (m_bak_videoram[tile_index * 2 + 1] & 0x20) >> 5;
}

void aquarium_state::bak_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bak_videoram[offset]);
	m_bak_tilemap->mark_tile_dirty(offset / 2);
}

void aquarium_state::video_start()
{
	m_txt_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);
	m_mid_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_mid_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_bak_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aquarium_state::get_bak_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_txt_tilemap->set_transparent_pen(0);
	m_mid_tilemap->set_transparent_pen(0);
	m_bak_tilemap->set_transparent_pen(0);
}

void aquarium_state::aquarium_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0;
	if (colour & 8)
		pri_mask |= (GFX_PMASK_2 | GFX_PMASK_4 | GFX_PMASK_8);
}

uint32_t aquarium_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_mid_tilemap->set_scrollx(0, m_scroll[0]);
	m_mid_tilemap->set_scrolly(0, m_scroll[1]);
	m_bak_tilemap->set_scrollx(0, m_scroll[2]);
	m_bak_tilemap->set_scrolly(0, m_scroll[3]);
	m_txt_tilemap->set_scrollx(0, m_scroll[4]);
	m_txt_tilemap->set_scrolly(0, m_scroll[5]);

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect); // WDUD logo suggests this

	m_bak_tilemap->draw(screen, bitmap, cliprect, 0, 1);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 1, 4);

	m_bak_tilemap->draw(screen, bitmap, cliprect, 1, 8);
	m_sprgen->aquarium_draw_sprites(screen, bitmap, cliprect);
	m_mid_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_txt_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void aquarium_state::watchdog_w(u8 data)
{
	m_watchdog->write_line_ck(BIT(data, 7));
	// bits 0 & 1 also used
}

void aquarium_state::z80_bank_w(u8 data)
{
	// uses bits ---x --xx
	data = bitswap<8>(data, 7, 6, 5, 2, 3, 1, 4, 0);

	LOGAUDIOBANK("aquarium bank %04x\n", data);
	// aquarium bank 0003 00ff - correct (title)   011
	// aquarium bank 0006 00ff - correct (select)  110
	// aquarium bank 0005 00ff - level 1 (correct)
	// (all music seems correct w/regards the reference video)

	m_audiobank->set_entry(data & 0x7);
}

u8 aquarium_state::snd_bitswap(u8 scrambled_data)
{
	return bitswap<8>(scrambled_data, 0, 1, 2, 3, 4, 5, 6, 7);
}

u8 aquarium_state::oki_r()
{
	return snd_bitswap(m_oki->read());
}

void aquarium_state::oki_w(u8 data)
{
	LOGOKI("%s:Writing %04x to the OKI M6295\n", machine().describe_context(), snd_bitswap(data));
	m_oki->write(snd_bitswap(data));
}


void aquarium_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0xc00000, 0xc00fff).ram().w(FUNC(aquarium_state::mid_videoram_w)).share(m_mid_videoram);
	map(0xc01000, 0xc01fff).ram().w(FUNC(aquarium_state::bak_videoram_w)).share(m_bak_videoram);
	map(0xc02000, 0xc03fff).ram().w(FUNC(aquarium_state::txt_videoram_w)).share(m_txt_videoram);
	map(0xc80000, 0xc81fff).rw(m_sprgen, FUNC(excellent_spr_device::read), FUNC(excellent_spr_device::write)).umask16(0x00ff);
	map(0xd00000, 0xd00fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xd80014, 0xd8001f).writeonly().share(m_scroll);
	map(0xd80068, 0xd80069).nopw();        // probably not used
	map(0xd80080, 0xd80081).portr("DSW");
	map(0xd80082, 0xd80083).nopr(); // stored but not read back ? check code at 0x01f440
	map(0xd80084, 0xd80085).portr("INPUTS");
	map(0xd80086, 0xd80087).portr("SYSTEM");
	map(0xd80088, 0xd80088).w(FUNC(aquarium_state::watchdog_w));
	map(0xd8008b, 0xd8008b).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xff0000, 0xffffff).ram();
}

void aquarium_state::snd_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_audiobank);
}

void aquarium_state::snd_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x02, 0x02).rw(FUNC(aquarium_state::oki_r), FUNC(aquarium_state::oki_w));
	map(0x04, 0x04).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x06, 0x06).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w)); // only written with 0 for some reason
	map(0x08, 0x08).w(FUNC(aquarium_state::z80_bank_w));
}

static INPUT_PORTS_START( aquarium )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )           PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000c, 0x000c, "Winning Rounds (Player VS CPU)" )    PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x000c, "1/1" )
	PORT_DIPSETTING(      0x0000, "1/1" )                   // Not used or listed in manual
	PORT_DIPSETTING(      0x0008, "2/3" )
	PORT_DIPSETTING(      0x0004, "3/5" )
	PORT_DIPNAME( 0x0030, 0x0030, "Winning Rounds (Player VS Player)" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0030, "1/1" )
	PORT_DIPSETTING(      0x0000, "1/1" )                   // Not used or listed in manual
	PORT_DIPSETTING(      0x0020, "2/3" )
	PORT_DIPSETTING(      0x0010, "3/5" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW1:7" )            // Listed in the manual as always OFF
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )            // Listed in the manual as always OFF
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x0800, 0x0800, "SW2:4" )            // Listed in the manual as always OFF
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )            // Listed in the manual as always OFF
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x8000, "SW2:8" )            // Listed in the manual as always OFF

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  // used in testmode, but not in game?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  // used in testmode, but not in game?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x1000, IP_ACTIVE_LOW )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
INPUT_PORTS_END

static const gfx_layout layout_5bpp_hi =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0, 1) },
	{ STEP16(0, 16) },
	16*16
};

static GFXDECODE_START( gfx_aquarium )
	GFXDECODE_ENTRY( "txt",    0, gfx_8x8x4_packed_msb,   0x200, 16 )
	GFXDECODE_ENTRY( "mid",    0, gfx_16x16x4_packed_msb, 0x400, 32 ) // low 4bpp of 5bpp data
	GFXDECODE_ENTRY( "bak",    0, gfx_16x16x4_packed_msb, 0x400, 32 ) // low 4bpp of 5bpp data
	GFXDECODE_ENTRY( "bak_hi", 0, layout_5bpp_hi,         0x400, 32 ) // hi 1bpp of 5bpp data
	GFXDECODE_ENTRY( "mid_hi", 0, layout_5bpp_hi,         0x400, 32 ) // hi 1bpp of 5bpp data
GFXDECODE_END

void aquarium_state::aquarium(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(32'000'000) / 2); // clock not verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &aquarium_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(aquarium_state::irq1_line_hold));

	Z80(config, m_audiocpu, XTAL(32'000'000) / 6); // clock not verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &aquarium_state::snd_map);
	m_audiocpu->set_addrmap(AS_IO, &aquarium_state::snd_portmap);

	// Confirmed IC type, even though some other Excellent games from this period use a MAX693.
	MB3773(config, m_watchdog, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 64*8);
	m_screen->set_visarea(2*8, 42*8-1, 2*8, 34*8-1);
	m_screen->set_screen_update(FUNC(aquarium_state::screen_update));
	m_screen->screen_vblank().set(m_sprgen, FUNC(excellent_spr_device::vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aquarium);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 0x1000 / 2);

	EXCELLENT_SPRITE(config, m_sprgen, 0);
	m_sprgen->set_palette(m_palette);
	m_sprgen->set_color_base(0x300);
	m_sprgen->set_colpri_callback(FUNC(aquarium_state::aquarium_colpri_cb));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2151_device &ymsnd(YM2151(config, "ymsnd", XTAL(14'318'181) / 4)); // clock not verified on PCB
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "speaker", 0.45, 0);
	ymsnd.add_route(1, "speaker", 0.45, 1);

	OKIM6295(config, m_oki, XTAL(1'056'000), okim6295_device::PIN7_HIGH); // pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.47, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.47, 1);
}

ROM_START( aquarium )
	ROM_REGION( 0x080000, "maincpu", 0 )     // 68000
	ROM_LOAD16_WORD_SWAP( "aquar3.13h",  0x000000, 0x080000, CRC(f197991e) SHA1(0a217d735e2643605dbfd6ee20f98f46b37d4838) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "excellent_5.10c",  0x000000, 0x40000, CRC(fa555be1) SHA1(07236f2b2ba67e92984b9ddf4a8154221d535245) )

	ROM_REGION( 0x080000, "mid", 0 ) // BG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_1.15b", 0x000000, 0x080000, CRC(575df6ac) SHA1(071394273e512666fe124facdd8591a767ad0819) ) // 4bpp
	// data is expanded here from mid_hi
	ROM_REGION( 0x020000, "mid_hi", 0 ) // BG Tiles
	ROM_LOAD( "excellent_6.15d", 0x000000, 0x020000, CRC(9065b146) SHA1(befc218bbcd63453ea7eb8f976796d36f2b2d552) ) // 1bpp

	ROM_REGION( 0x080000, "bak", 0 ) // BG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_8.14g", 0x000000, 0x080000, CRC(915520c4) SHA1(308207cb20f1ed6df365710c808644a6e4f07614) ) // 4bpp
	// data is expanded here from bak_hi
	ROM_REGION( 0x020000, "bak_hi", 0 ) // BG Tiles
	ROM_LOAD( "excellent_7.17g", 0x000000, 0x020000, CRC(b96b2b82) SHA1(2b719d0c185d1eca4cd9ea66bed7842b74062288) ) // 1bpp

	ROM_REGION( 0x060000, "txt", 0 ) // FG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_2.17e", 0x000000, 0x020000, CRC(aa071b05) SHA1(517415bfd8e4dd51c6eb03a25c706f8613d34a09) )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD16_WORD_SWAP( "d23c8000.1f",   0x000000, 0x0100000, CRC(14758b3c) SHA1(b372ccb42acb55a3dd15352a9d4ed576878a6731) ) // PCB denotes 23C16000 but a 23C8000 MASK is used

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "excellent_4.7d",  0x000000, 0x80000, CRC(9a4af531) SHA1(bb201b7a6c9fd5924a0d79090257efffd8d4aba1) )
ROM_END

ROM_START( aquariumj )
	ROM_REGION( 0x080000, "maincpu", 0 )     // 68000
	ROM_LOAD16_WORD_SWAP( "excellent_3.13h",  0x000000, 0x080000, CRC(344509a1) SHA1(9deb610732dee5066b3225cd7b1929b767579235) )

	ROM_REGION( 0x40000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "excellent_5.10c",  0x000000, 0x40000, CRC(fa555be1) SHA1(07236f2b2ba67e92984b9ddf4a8154221d535245) )

	ROM_REGION( 0x080000, "mid", 0 ) // BG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_1.15b", 0x000000, 0x080000, CRC(575df6ac) SHA1(071394273e512666fe124facdd8591a767ad0819) ) // 4bpp
	// data is expanded here from mid_hi
	ROM_REGION( 0x020000, "mid_hi", 0 ) // BG Tiles
	ROM_LOAD( "excellent_6.15d", 0x000000, 0x020000, CRC(9065b146) SHA1(befc218bbcd63453ea7eb8f976796d36f2b2d552) ) // 1bpp

	ROM_REGION( 0x080000, "bak", 0 ) // BG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_8.14g", 0x000000, 0x080000, CRC(915520c4) SHA1(308207cb20f1ed6df365710c808644a6e4f07614) ) // 4bpp
	// data is expanded here from bak_hi
	ROM_REGION( 0x020000, "bak_hi", 0 ) // BG Tiles
	ROM_LOAD( "excellent_7.17g", 0x000000, 0x020000, CRC(b96b2b82) SHA1(2b719d0c185d1eca4cd9ea66bed7842b74062288) ) // 1bpp

	ROM_REGION( 0x060000, "txt", 0 ) // FG Tiles
	ROM_LOAD16_WORD_SWAP( "excellent_2.17e", 0x000000, 0x020000, CRC(aa071b05) SHA1(517415bfd8e4dd51c6eb03a25c706f8613d34a09) )

	ROM_REGION( 0x200000, "spritegen", 0 )
	ROM_LOAD16_WORD_SWAP( "d23c8000.1f",   0x000000, 0x0100000, CRC(14758b3c) SHA1(b372ccb42acb55a3dd15352a9d4ed576878a6731) ) // PCB denotes 23C16000 but a 23C8000 MASK is used

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "excellent_4.7d",  0x000000, 0x80000, CRC(9a4af531) SHA1(bb201b7a6c9fd5924a0d79090257efffd8d4aba1) )
ROM_END

std::unique_ptr<u8[]> aquarium_state::expand_gfx(int low, int hi)
{
	/* The BG tiles are 5bpp, this rearranges the data from
	   the ROMs containing the 1bpp data so we can decode it
	   correctly */
	gfx_element *gfx_l = m_gfxdecode->gfx(low);
	gfx_element *gfx_h = m_gfxdecode->gfx(hi);

	// allocate memory for the assembled data
	auto decoded = std::make_unique<u8[]>(gfx_l->elements() * gfx_l->width() * gfx_l->height());

	// loop over elements
	u8 *dest = decoded.get();
	for (int c = 0; c < gfx_l->elements(); c++)
	{
		const u8 *c0base = gfx_l->get_data(c);
		const u8 *c1base = gfx_h->get_data(c);

		// loop over height
		for (int y = 0; y < gfx_l->height(); y++)
		{
			const u8 *c0 = c0base;
			const u8 *c1 = c1base;

			for (int x = 0; x < gfx_l->width(); x++)
			{
				u8 hi_data = *c1++;
				*dest++ = (*c0++ & 0xf) | ((hi_data << 4) & 0x10);
			}
			c0base += gfx_l->rowbytes();
			c1base += gfx_h->rowbytes();
		}
	}

	gfx_l->set_raw_layout(decoded.get(), gfx_l->width(), gfx_l->height(), gfx_l->elements(), 8 * gfx_l->width(), 8 * gfx_l->width() * gfx_l->height());
	gfx_l->set_granularity(32);
	m_gfxdecode->set_gfx(hi, nullptr);
	return decoded;
}

void aquarium_state::init_aquarium()
{
	m_decoded_gfx[0] = expand_gfx(1, 4);
	m_decoded_gfx[1] = expand_gfx(2, 3);

	u8 *z80 = memregion("audiocpu")->base();

	// configure and set up the sound bank
	m_audiobank->configure_entries(0, 0x8, &z80[0x00000], 0x8000);
	m_audiobank->set_entry(0x00);
}

} // anonymous namespace


GAME( 1996, aquarium,  0,        aquarium, aquarium, aquarium_state, init_aquarium, ROT0, "Excellent System", "Aquarium (US)",    MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
GAME( 1996, aquariumj, aquarium, aquarium, aquarium, aquarium_state, init_aquarium, ROT0, "Excellent System", "Aquarium (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_COCKTAIL )
