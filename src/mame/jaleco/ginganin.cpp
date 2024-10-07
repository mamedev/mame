// license:BSD-3-Clause
// copyright-holders: Luca Elia
/***************************************************************************

                            Ginga Ninkyouden
                            (C) 1987 Jaleco

                    driver by Luca Elia (l.elia@tin.it)

CPU   : 68000 68B09
SOUND : YM2149 Y8950(MSX AUDIO)
OSC.  : 6.000MHz 3.579545MHz

* CTC uses MB-8873E (MC-6840)

                    Interesting routines (main CPU)
                    -------------------------------

Interrupts: 1-7]    d17a:   clears 20018 etc.

f4b2    print string:   a1->(char)*,0x25(%) d7.w=color  a0->screen (30000)
f5d6    print 7 digit BCD number: d0.l to (a1)+ color $3000


                    Interesting locations (main CPU)
                    --------------------------------

20014   # of players (1-2)
20018   cleared by interrupts
2001c   credits (max 9)
20020   internal timer?
20024   initial lives
20058   current lives p1
2005c   current lives p2
20070   coins
200a4   time
200a8   energy

60008       values: 0 1 ffff
6000c       bit:    0   flip sceen? <-  70002>>14
                    1   ?           <-

6000e   soundlatch  <- 20038 2003c 20040


                                To Do
                                -----

- Game doesn't init paletteram / tilemaps properly, ending up with MAME
  palette defaults at start-up and missing text layer if you coin it up
  too soon.
- In later levels a couple sprites lingers on top of screen;
- Sometimes a credit sample also gets overwritten with additional spurious
  playback of all samples;
^ all these might be just BTANBs ...

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_VREGS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_VREGS)

#include "logmacro.h"

#define LOGVREGS(...)     LOGMASKED(LOG_VREGS,     __VA_ARGS__)


namespace {

class ginganin_state : public driver_device
{
public:
	ginganin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_txtram(*this, "txtram"),
		m_spriteram(*this, "spriteram"),
		m_vregs(*this, "vregs"),
		m_fgram(*this, "fgram"),
		m_bgrom(*this, "bgrom"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void ginganin(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<u16> m_txtram;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_fgram;
	required_region_ptr<u8> m_bgrom;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_tx_tilemap = nullptr;
	u16 m_layers_ctrl = 0;
	u8 m_flipscreen = 0;
#ifdef MAME_DEBUG
	int m_posx = 0;
	int m_posy = 0;
#endif

	void fgram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void txtram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_txt_tile_info);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/**************************************************************************

Note:   if MAME_DEBUG is defined, pressing Z with:

        Q       shows background
        W       shows foreground
        E       shows frontmost (text) layer
        A       shows sprites

        Keys can be used together!


[Screen]
    Visible Size:       256H x 240V
    Dynamic Colors:     256 x 4
    Color Space:        16R x 16G x 16B

[Scrolling layers]
    Format (all layers):    Offset:     0x400    0x000
                            Bit:        fedc---- --------   Color
                                        ----ba98 76543210   Code

    [Background]
        Size:               8192 x 512  (static: stored in ROM)
        Scrolling:          X,Y         (registers: $60006.w, $60004.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $300-$3ff

    [Foreground]
        Size:               4096 x 512
        Scrolling:          X,Y         (registers: $60002.w, $60000.w)
        Tiles Size:         16 x 16
        Tiles Number:       $400
        Colors:             $200-$2ff

    [Frontmost]
        Size:               256 x 256
        Scrolling:          -
        Tiles Size:         8 x 8
        Tiles Number:       $200
        Colors:             $000-$0ff


[Sprites]
    On Screen:          256
    In ROM:             $a00
    Colors:             $100-$1ff
    Format:             See Below


**************************************************************************/

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

// Background - Resides in ROM

static constexpr u8 BG_GFX = 0;
static constexpr u16 BG_NX = (16 * 32);
static constexpr u8 BG_NY = (16 * 2);

TILE_GET_INFO_MEMBER(ginganin_state::get_bg_tile_info)
{
	const u32 code = m_bgrom[2 * tile_index + 0] * 256 + m_bgrom[2 * tile_index + 1];
	tileinfo.set(BG_GFX,
			code,
			code >> 12,
			0);
}


// Foreground - Resides in RAM

static constexpr u8 FG_GFX = 1;
static constexpr u16 FG_NX = (16 * 16);
static constexpr u8 FG_NY = (16 * 2);

TILE_GET_INFO_MEMBER(ginganin_state::get_fg_tile_info)
{
	const u16 code = m_fgram[tile_index];
	tileinfo.set(FG_GFX,
			code,
			code >> 12,
			0);
}

void ginganin_state::fgram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_fgram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}


// Frontmost (text) Layer - Resides in RAM

static constexpr u8 TXT_GFX = 2;
static constexpr u8 TXT_NX = 32;
static constexpr u8 TXT_NY = 32;

TILE_GET_INFO_MEMBER(ginganin_state::get_txt_tile_info)
{
	const u16 code = m_txtram[tile_index];
	tileinfo.set(TXT_GFX,
			code,
			code >> 12,
			0);
}

void ginganin_state::txtram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_txtram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}


void ginganin_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ginganin_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, BG_NX, BG_NY);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ginganin_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, FG_NX, FG_NY);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ginganin_state::get_txt_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, TXT_NX, TXT_NY);

	m_fg_tilemap->set_transparent_pen(15);
	m_tx_tilemap->set_transparent_pen(15);
}


void ginganin_state::vregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vregs[offset]);
	data = m_vregs[offset];

	switch (offset)
	{
	case 0:
		m_fg_tilemap->set_scrolly(0, data);
		break;
	case 1:
		m_fg_tilemap->set_scrollx(0, data);
		break;
	case 2:
		m_bg_tilemap->set_scrolly(0, data);
		break;
	case 3:
		m_bg_tilemap->set_scrollx(0, data);
		break;
	case 4:
		m_layers_ctrl = data;
		break;
/*  case 5:
 *      break;
 */
	case 6:
		m_flipscreen = !(data & 1);
		machine().tilemap().set_flip_all(m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);
		break;
	case 7:
		m_soundlatch->write(data);
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		break;
	default:
		LOGVREGS("CPU #0 PC %06X : Warning, videoreg %04X <- %04X\n", m_maincpu->pc(), offset, data);
	}
}



/* --------------------------[ Sprites Format ]----------------------------

Offset:         Values:         Format:

0000.w          y position      fedc ba9- ---- ----     unused
                                ---- ---8 ---- ----     subtract 256
                                ---- ---- 7654 3210     position

0002.w          x position      See above

0004.w          code            f--- ---- ---- ----     y flip
                                -e-- ---- ---- ----     x flip
                                --dc ---- ---- ----     unused?
                                ---- ba98 7654 3210     code

0006.w          colour          fedc ---- ---- ----     colour code
                                ---- ba98 7654 3210     unused?

------------------------------------------------------------------------ */

void ginganin_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < (m_spriteram.bytes() >> 1); offs += 4)
	{
		int y = m_spriteram[offs + 0];
		int x = m_spriteram[offs + 1];
		const u32 code = m_spriteram[offs + 2];
		const u16 attr = m_spriteram[offs + 3];
		int flipx = code & 0x4000;
		int flipy = code & 0x8000;

		x = (x & 0xff) - (x & 0x100);
		y = (y & 0xff) - (y & 0x100);

		if (m_flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(3)->transpen(bitmap, cliprect,
				code & 0x3fff,
				attr >> 12,
				flipx, flipy,
				x, y, 15);
	}
}


u32 ginganin_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl1 = m_layers_ctrl;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;

	if (machine().input().code_pressed(KEYCODE_Q)) { msk |= 0xfff1;}
	if (machine().input().code_pressed(KEYCODE_W)) { msk |= 0xfff2;}
	if (machine().input().code_pressed(KEYCODE_E)) { msk |= 0xfff4;}
	if (machine().input().code_pressed(KEYCODE_A)) { msk |= 0xfff8;}
	if (msk != 0) layers_ctrl1 &= msk;

#define SETSCROLL \
	m_bg_tilemap->set_scrollx(0, m_posx); \
	m_bg_tilemap->set_scrolly(0, m_posy); \
	m_fg_tilemap->set_scrollx(0, m_posx); \
	m_fg_tilemap->set_scrolly(0, m_posy); \
	popmessage("B>%04X:%04X F>%04X:%04X",m_posx%(BG_NX*16),m_posy%(BG_NY*16),m_posx%(FG_NX*16),m_posy%(FG_NY*16));

	if (machine().input().code_pressed(KEYCODE_L)) { m_posx +=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_J)) { m_posx -=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_K)) { m_posy +=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_I)) { m_posy -=8; SETSCROLL }
	if (machine().input().code_pressed(KEYCODE_H)) { m_posx = m_posy = 0; SETSCROLL }
}
#endif

	if (layers_ctrl1 & 1)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	else
		bitmap.fill(0, cliprect);

	if (layers_ctrl1 & 2)
		m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (layers_ctrl1 & 8)
		draw_sprites(bitmap, cliprect);
	if (layers_ctrl1 & 4)
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void ginganin_state::main_map(address_map &map)
{
	// PC=0x408 ROM area 10000-13fff is written at POST with: 0000 0000 0000 0001,
	// looks a debugging left-over for GFX patching (causes state garbage if hooked as RAM write mirror)
	map(0x000000, 0x01ffff).rom().nopw();
	map(0x020000, 0x023fff).ram();
	map(0x030000, 0x0307ff).ram().w(FUNC(ginganin_state::txtram_w)).share(m_txtram);
	map(0x040000, 0x0407ff).ram().share(m_spriteram);
	map(0x050000, 0x0507ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x060000, 0x06000f).ram().w(FUNC(ginganin_state::vregs_w)).share(m_vregs);
	map(0x068000, 0x06bfff).ram().w(FUNC(ginganin_state::fgram_w)).share(m_fgram);
	map(0x070000, 0x070001).portr("P1_P2");
	map(0x070002, 0x070003).portr("DSW");
}


void ginganin_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0807).rw("6840ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x1800, 0x1800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x2000, 0x2001).w("ymsnd", FUNC(y8950_device::write));
	map(0x2800, 0x2801).w("psg", FUNC(ym2149_device::address_data_w));
	map(0x4000, 0xffff).rom().region("audiocpu", 0x4000);
}


static INPUT_PORTS_START( ginganin )
	PORT_START("P1_P2")     // 70000.w
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")       // 70002.w
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Infinite Lives")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Free Play & Invulnerability")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "2")
	PORT_DIPSETTING(      0x0300, "3")
	PORT_DIPSETTING(      0x0100, "4")
	PORT_DIPSETTING(      0x0200, "5")
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )  // probably unused
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )  // it does something
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Freeze" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



static GFXDECODE_START( gfx_ginganin )
	GFXDECODE_ENTRY( "bgtiles",  0, gfx_8x8x4_col_2x2_group_packed_msb, 256*3, 16 )
	GFXDECODE_ENTRY( "fgtiles",  0, gfx_8x8x4_col_2x2_group_packed_msb, 256*2, 16 )
	GFXDECODE_ENTRY( "txttiles", 0, gfx_8x8x4_packed_msb,               256*0, 16 )
	GFXDECODE_ENTRY( "sprites",  0, gfx_8x8x4_col_2x2_group_packed_msb, 256*1, 16 )
GFXDECODE_END


void ginganin_state::machine_start()
{
	save_item(NAME(m_layers_ctrl));
	save_item(NAME(m_flipscreen));
}

void ginganin_state::machine_reset()
{
	m_layers_ctrl = 0;
	m_flipscreen = 0;
}

void ginganin_state::ginganin(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ginganin_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(ginganin_state::irq1_line_hold)); // ? (vectors 1-7 contain the same address)

	static constexpr XTAL SOUND_CLOCK = 3.579545_MHz_XTAL;

	MC6809(config, m_audiocpu, SOUND_CLOCK); // MBL68B09?
	m_audiocpu->set_addrmap(AS_PROGRAM, &ginganin_state::sound_map);

	ptm6840_device &ptm(PTM6840(config, "6840ptm", SOUND_CLOCK / 4));
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	GENERIC_LATCH_8(config, m_soundlatch);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 0 + 16 , 255 - 16);
	screen.set_screen_update(FUNC(ginganin_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ginganin);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 1024);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2149(config, "psg", SOUND_CLOCK / 2).add_route(ALL_OUTPUTS, "mono", 0.10);
	Y8950(config, "ymsnd", SOUND_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.0); // The Y8950 is basically a YM3526 with ADPCM built in
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ginganin )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gn_02.bin", 0x00000, 0x10000, CRC(4a4e012f) SHA1(7c94a5b6b71e037af355f3aa4623be1f585db8dc) )
	ROM_LOAD16_BYTE( "gn_01.bin", 0x00001, 0x10000, CRC(30256fcb) SHA1(dc15e0da88ae5cabe0150f7290508c3d58c06c11) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gn_05.bin", 0x00000, 0x10000, CRC(e76e10e7) SHA1(b16f10a1a01b7b04221c9bf1b0d157e936bc5fb5) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "gn_15.bin", 0x000000, 0x10000, CRC(1b8ac9fb) SHA1(1e5ee2a565fa262f1e48c1088d84c6f42d84b4e3) )
	ROM_LOAD( "gn_14.bin", 0x010000, 0x10000, CRC(e73fe668) SHA1(fa39fddd7448d3fc6b539506e33b951db205afa1) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "gn_12.bin", 0x000000, 0x10000, CRC(c134a1e9) SHA1(8bace0f0169e61f1b7254393fa9cad6dca09c335) )
	ROM_LOAD( "gn_13.bin", 0x010000, 0x10000, CRC(1d3bec21) SHA1(305823c78cad9288f918178e1c24cb0459ba2a6e) )

	ROM_REGION( 0x04000, "txttiles", 0 )
	ROM_LOAD( "gn_10.bin", 0x000000, 0x04000, CRC(ae371b2d) SHA1(d5e03b085586ed2bf40713f432bcf12e07318226) )

	ROM_REGION( 0x50000, "sprites", 0 )
	ROM_LOAD( "gn_06.bin", 0x000000, 0x10000, CRC(bdc65835) SHA1(53222fc3ec15e641289abb754657b0d59b88b66b) )
	ROM_CONTINUE(          0x040000, 0x10000 )
	ROM_LOAD( "gn_07.bin", 0x010000, 0x10000, CRC(c2b8eafe) SHA1(a042a200efd4e7361e9ab516085c9fc8067e28b4) )
	ROM_LOAD( "gn_08.bin", 0x020000, 0x10000, CRC(f7c73c18) SHA1(102700e2217bcd1532af56ee6a00ad608c8217db) )
	ROM_LOAD( "gn_09.bin", 0x030000, 0x10000, CRC(a5e07c3b) SHA1(cdda02cd847330575612cb33d1bb38a5d50a3e6d) )

	ROM_REGION( 0x08000, "bgrom", 0 )
	ROM_LOAD( "gn_11.bin", 0x00000, 0x08000, CRC(f0d0e605) SHA1(0c541e8e036573be1d99ecb71fdb4568ca8cc269) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "gn_04.bin", 0x00000, 0x10000, CRC(0ed9133b) SHA1(77f628e8ec28016efac2d906146865ca4ec54bd5) )
	ROM_LOAD( "gn_03.bin", 0x10000, 0x10000, CRC(f1ba222c) SHA1(780c0bd0045bac1e1bb3209576383db90504fbf3) )

ROM_END

ROM_START( ginganina )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x10000, CRC(6da1d8a3) SHA1(ea81f2934fa7901563e886f3d600edd08ec0ea24) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x10000, CRC(0bd32d59) SHA1(5ab2c0e4a1d9cafbd3448d981103508debd7ed96) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gn_05.bin", 0x00000, 0x10000, CRC(e76e10e7) SHA1(b16f10a1a01b7b04221c9bf1b0d157e936bc5fb5) )

	ROM_REGION( 0x20000, "bgtiles", 0 )
	ROM_LOAD( "gn_15.bin", 0x000000, 0x10000, CRC(1b8ac9fb) SHA1(1e5ee2a565fa262f1e48c1088d84c6f42d84b4e3) )
	ROM_LOAD( "gn_14.bin", 0x010000, 0x10000, CRC(e73fe668) SHA1(fa39fddd7448d3fc6b539506e33b951db205afa1) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "gn_12.bin", 0x000000, 0x10000, CRC(c134a1e9) SHA1(8bace0f0169e61f1b7254393fa9cad6dca09c335) )
	ROM_LOAD( "gn_13.bin", 0x010000, 0x10000, CRC(1d3bec21) SHA1(305823c78cad9288f918178e1c24cb0459ba2a6e) )

	ROM_REGION( 0x04000, "txttiles", 0 )
	ROM_LOAD( "10.bin", 0x000000, 0x04000, CRC(48a20745) SHA1(69855b0402feca4ba9632142e569c652ca05b9fa) )

	ROM_REGION( 0x50000, "sprites", 0 )
	ROM_LOAD( "gn_06.bin", 0x000000, 0x10000, CRC(bdc65835) SHA1(53222fc3ec15e641289abb754657b0d59b88b66b) )
	ROM_CONTINUE(          0x040000, 0x10000 )
	ROM_LOAD( "gn_07.bin", 0x010000, 0x10000, CRC(c2b8eafe) SHA1(a042a200efd4e7361e9ab516085c9fc8067e28b4) )
	ROM_LOAD( "gn_08.bin", 0x020000, 0x10000, CRC(f7c73c18) SHA1(102700e2217bcd1532af56ee6a00ad608c8217db) )
	ROM_LOAD( "gn_09.bin", 0x030000, 0x10000, CRC(a5e07c3b) SHA1(cdda02cd847330575612cb33d1bb38a5d50a3e6d) )

	ROM_REGION( 0x08000, "bgrom", 0 )
	ROM_LOAD( "gn_11.bin", 0x00000, 0x08000, CRC(f0d0e605) SHA1(0c541e8e036573be1d99ecb71fdb4568ca8cc269) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "gn_04.bin", 0x00000, 0x10000, CRC(0ed9133b) SHA1(77f628e8ec28016efac2d906146865ca4ec54bd5) )
	ROM_LOAD( "gn_03.bin", 0x10000, 0x10000, CRC(f1ba222c) SHA1(780c0bd0045bac1e1bb3209576383db90504fbf3) )
ROM_END

} // anonymous namespace


GAME( 1987, ginganin,  0,        ginganin, ginganin, ginganin_state, empty_init, ROT0, "Jaleco", "Ginga Ninkyouden (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, ginganina, ginganin, ginganin, ginganin, ginganin_state, empty_init, ROT0, "Jaleco", "Ginga Ninkyouden (set 2)", MACHINE_SUPPORTS_SAVE )
