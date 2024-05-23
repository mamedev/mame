// license:BSD-3-Clause
// copyright-holders: Luca Elia

/***************************************************************************

                          -= Power Instinct =-
                            (C) 1993 Atlus

                driver by   Luca Elia (l.elia@tin.it)

Set 1
    CPU:    MC68000, Z80 (for sound)
    Sound:  2x OKI6295 + 1x YM2203
Bootleg Set 1
    CPU:    MC68000
    Sound:  OKIM6295
Bootleg Set 2
    CPU:    MC68000, Z80 (for sound)
    Sound:  2x OKI6295 (Sound code supports an additional YM2203, but it's not fitted)

Note:
- To enter test mode press F2 (Test)
  Use 9 (Service Coin) to change page.
- In powerinsa there is a hidden test mode screen because it's a bootleg
  without a sound CPU. Set 18ff08 to 4 during test mode that calls the
  data written to $10001e "sound code".

TODO:
- sprites flip y (not used by the game)
- graphic system and PCB design is similar as macross2 era hardwares in nmk16.cpp;
  is it mergeable?

***************************************************************************/

#include "emu.h"

#include "nmk16.h"
#include "nmk16spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nmk112.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class powerins_state : public nmk16_state
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag) :
		nmk16_state(mconfig, type, tag)
	{ }

	void powerins(machine_config &config);
	void powerinsa(machine_config &config);
	void powerinsb(machine_config &config);
	void powerinsc(machine_config &config);

protected:
	virtual void video_start() override;

	void base(machine_config &config);

	void screen_vblank_bootleg(int state);

	void main_program_map(address_map &map);

private:
	u8 bootleg_fake_ym2203_r();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void screen_vblank(int state);

	void get_colour_6bit(u32 &colour, u32 &pri_mask);
	void get_flip_extcode(u16 attr, int &flipx, int &flipy, int &code);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void audio_program_map(address_map &map);
	void bootleg_audio_io_map(address_map &map);
};

class powerinsa_state : public powerins_state
{
public:
	using powerins_state::powerins_state;

	void powerinsa(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void okibank_w(u8 data);

	void program_map(address_map &map);
	void oki_map(address_map &map);
};

/***************************************************************************

                          -= Power Instinct =-
                            (C) 1993 Atlus

                driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 1
        W           shows layer 2
        A           shows the sprites

        Keys can be used together!

        [ 2 Scrolling Layers ]

        Each Layer is made of various pages of 256x256 pixels.

            [Layer 0]
                Pages:              16x2
                Tiles:              16x16x4
                Scroll:             X,Y

            [Layer 1]
                Pages:              2x1
                Tiles:              8x8x4
                Scroll:             No

        [ 256 Sprites ]

        Each sprite is made of a variable amount of 16x16 tiles.
        Size can therefore vary from 16x16 (1 tile) to 256x256
        (16x16 tiles)


**************************************************************************/

/***************************************************************************
                          [ Tiles Format VRAM 0]

Offset:

0.w     fedc ---- ---- ----     Color Low  Bits
        ---- b--- ---- ----     Color High Bit
        ---- -a98 7654 3210     Code (Banked)


***************************************************************************/

// Layers are made of 256x256 pixel pages
/*
#define TILES_PER_PAGE_X    (0x10)
#define TILES_PER_PAGE_Y    (0x10)
#define TILES_PER_PAGE      (TILES_PER_PAGE_X * TILES_PER_PAGE_Y)

#define DIM_NX_0            (0x100)
#define DIM_NY_0            (0x20)
*/

TILE_GET_INFO_MEMBER(powerins_state::get_bg_tile_info)
{
	uint16_t code = m_bgvideoram[0][tile_index];
	tileinfo.set(1,
			(code & 0x07ff) | (m_bgbank << 11),
			((code & 0xf000) >> 12) | ((code & 0x0800) >> 7),
			0);
}


/***************************************************************************
                          [ Tiles Format VRAM 1]

Offset:

0.w     fedc ---- ---- ----     Color
        ---- ba98 7654 3210     Code


***************************************************************************/

/*
#define DIM_NX_1    (0x40)
#define DIM_NY_1    (0x20)
*/

/***************************************************************************


                                video_start


***************************************************************************/

void powerins_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(powerins_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(powerins_state::tilemap_scan_pages)), 16, 16, 256, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(powerins_state::common_get_tx_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 64, 32);
	m_bg_tilemap[1] = nullptr;

	m_tx_tilemap->set_transparent_pen(15);

	video_init();
	 // fixed offset
	m_bg_tilemap[0]->set_scrolldx(32, 32);
	m_tx_tilemap->set_scrolldx(32, 32);
}


/***************************************************************************


                                Sprites Drawing


***************************************************************************/



/* --------------------------[ Sprites Format ]----------------------------

Offset:     Format:                 Value:

    00      fedc ba98 7654 321-     -
            ---- ---- ---- ---0     Display this sprite

    02      fed- ---- ---- ----     -
            ---c ---- ---- ----     Flip X
            ---- ba9- ---- ----     -
            ---- ---8 ---- ----     Code High Bit
            ---- ---- 7654 ----     Number of tiles along Y, minus 1 (1-16)
            ---- ---- ---- 3210     Number of tiles along X, minus 1 (1-16)

    04                              Unused?

    06      f--- ---- ---- ----     -
            -edc ba98 7654 3210     Code Low Bits

    08                              X

    0A                              Unused?

    0C                              Y

    0E      fedc ba98 76-- ----     -
            ---- ---- --54 3210     Color


------------------------------------------------------------------------ */


void powerins_state::get_colour_6bit(u32 &colour, u32 &pri_mask)
{
	colour &= 0x3f;
	pri_mask |= GFX_PMASK_2; // under foreground
}

void powerins_state::get_flip_extcode(u16 attr, int &flipx, int &flipy, int &code)
{
	flipx = (attr & 0x1000) >> 12;
	code = (code & 0x7fff) | ((attr & 0x100) << 7);
}


/***************************************************************************


                                Screen Drawing


***************************************************************************/


u32 powerins_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
if (machine().input().code_pressed(KEYCODE_Z))
{
	int msk = 0;

	if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
	if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
//  if (machine().input().code_pressed(KEYCODE_E))  msk |= 4;
	if (machine().input().code_pressed(KEYCODE_A))  msk |= 8;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	screen.priority().fill(0, cliprect);
	if (layers_ctrl&1)      bg_update(screen, bitmap, cliprect, 0);
	else                    bitmap.fill(0, cliprect);
	if (layers_ctrl&2)      tx_update(screen, bitmap, cliprect);
	if (layers_ctrl&8)      draw_sprites(screen, bitmap, cliprect, m_spriteram_old2.get());
	return 0;
}

void powerins_state::screen_vblank(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(4, HOLD_LINE);
		m_dma_timer->adjust(attotime::from_usec(256)); // 256 USEC after VBOUT, same as nmk16.cpp?
	}
}

void powerins_state::screen_vblank_bootleg(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(4, HOLD_LINE);
		// bootlegs don't have DMA?
		memcpy(m_spriteram_old2.get(),m_spriteram_old.get(), 0x1000);
		memcpy(m_spriteram_old.get(), m_mainram + m_sprdma_base / 2, 0x1000);
	}
}


/***************************************************************************

                                Memory Maps

***************************************************************************/


void powerinsa_state::okibank_w(u8 data)
{
	m_okibank[0]->set_entry(data & 7);
}

u8 powerins_state::bootleg_fake_ym2203_r()
{
	return 0x01;
}

void powerins_state::main_program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x100001).portr("SYSTEM");
	map(0x100002, 0x100003).portr("P1_P2");
	map(0x100008, 0x100009).portr("DSW1");
	map(0x10000a, 0x10000b).portr("DSW2");
	map(0x100015, 0x100015).w(FUNC(powerins_state::flipscreen_w));
	map(0x100016, 0x100017).nopw();    // IRQ enable or Z80 sound reset like in Macross 2?
	map(0x100019, 0x100019).w(FUNC(powerins_state::tilebank_w));
	map(0x10001f, 0x10001f).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x120000, 0x120fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x130000, 0x130007).ram().w(FUNC(powerins_state::scroll_w<0>)).umask16(0x00ff);
	map(0x140000, 0x143fff).ram().w(FUNC(powerins_state::bgvideoram_w<0>)).share(m_bgvideoram[0]);
	map(0x170000, 0x170fff).mirror(0x1000).ram().w(FUNC(powerins_state::txvideoram_w)).share(m_txvideoram);
	map(0x180000, 0x18ffff).ram().share(m_mainram);
}

// powerinsa: same as the original one but without the sound CPU (and inferior sound HW)
void powerinsa_state::program_map(address_map &map)
{
	main_program_map(map);
	map(0x100031, 0x100031).w(FUNC(powerinsa_state::okibank_w));
	map(0x10003f, 0x10003f).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void powerins_state::audio_program_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
//  map(0xe000, 0xe000).nopw(); // ? written only once ?
//  map(0xe001, 0xe001).nopw(); // ?
}

void powerins_state::bootleg_audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(powerins_state::bootleg_fake_ym2203_r)).nopw();
	map(0x01, 0x01).noprw();
	map(0x80, 0x80).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x88, 0x88).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x90, 0x97).w("nmk112", FUNC(nmk112_device::okibank_w));
}

void powerinsa_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom().region("oki1", 0);
	map(0x30000, 0x3ffff).bankr(m_okibank[0]);
}


/***************************************************************************

                                Input Ports

***************************************************************************/

static INPUT_PORTS_START( powerins )
	PORT_START("SYSTEM")    // $100000
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_P2")     // $100002
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("DSW1")      // $100008
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Free_Play ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coin_B ) )         PORT_DIPLOCATION("SW1:7,6,5")
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0070, 0x0070, DEF_STR( Coin_A ) )         PORT_DIPLOCATION("SW1:4,3,2")
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, "2 Coins/1 Credit (1 to continue)" )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On )  )

	PORT_START("DSW2")      // $10000a
	PORT_DIPNAME( 0x0001, 0x0001, "Coin Chutes" )             PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, "1 Chute" )
	PORT_DIPSETTING(      0x0000, "2 Chutes" )
	PORT_DIPNAME( 0x0002, 0x0002, "Join In Mode" )            PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
/*
    In "Join In" mode, a second player can join even if one player has already
    begun to play.  Please refer to chart below:

    Join In Mode  Credit                  Join In     Game Over
    -----------------------------------------------------------------------------------------------
    Join In OFF   1C per Player           Anytime     Winner of VS Plays Computer
    Join In ON    1C = VS Mode 2 players  Cannot      After win VS Game Over for both players

*/
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Blood Color" )             PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0010, "Red" )
	PORT_DIPSETTING(      0x0000, "Blue" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Game_Time ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Short" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(      0x0040, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( powerinj )
	PORT_INCLUDE(powerins)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************

                                Graphics Layouts

***************************************************************************/

static GFXDECODE_START( gfx_powerins )
	GFXDECODE_ENTRY( "fgtile",  0, gfx_8x8x4_packed_msb,               0x200, 0x10 )
	GFXDECODE_ENTRY( "bgtile",  0, gfx_8x8x4_col_2x2_group_packed_msb, 0x000, 0x20 )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x400, 0x40 )
GFXDECODE_END

static GFXDECODE_START( gfx_powerinsc )
	GFXDECODE_ENTRY( "bgtile",  0x280000, gfx_8x8x4_packed_lsb,               0x200, 0x10 )
	GFXDECODE_ENTRY( "bgtile",  0,        gfx_8x8x4_col_2x2_group_packed_lsb, 0x000, 0x20 )
	GFXDECODE_ENTRY( "sprites", 0,        gfx_8x8x4_col_2x2_group_packed_lsb, 0x400, 0x40 ) // TODO: wrong decode and ROM loading
GFXDECODE_END

/***************************************************************************

                                Machine Drivers

***************************************************************************/

void powerinsa_state::machine_start()
{
	m_okibank[0]->configure_entries(0, 5, memregion("oki1")->base() + 0x30000, 0x10000);
}

void powerins_state::base(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &powerins_state::main_program_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(14'000'000) / 2, 448, 0, 320, 278, 16, 240); // confirmed
	m_screen->set_screen_update(FUNC(powerins_state::screen_update));
	m_screen->screen_vblank().set(FUNC(powerins_state::screen_vblank));
	m_screen->set_palette(m_palette);

	NMK_16BIT_SPRITE(config, m_spritegen, XTAL(14'000'000) / 2);
	m_spritegen->set_colpri_callback(FUNC(powerins_state::get_colour_6bit));
	m_spritegen->set_ext_callback(FUNC(powerins_state::get_flip_extcode));
	m_spritegen->set_mask(0x3ff, 0x3ff);
	m_spritegen->set_screen_size(320, 256);
	m_spritegen->set_max_sprite_clock(448 * 263); // not verified?
	m_spritegen->set_videoshift(32);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_powerins);
	PALETTE(config, m_palette).set_format(palette_device::RRRRGGGGBBBBRGBx, 2048);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
}

void powerins_state::powerins(machine_config &config)
{
	base(config);

	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &powerins_state::audio_program_map);
	m_audiocpu->set_addrmap(AS_IO, &powerins_state::macross2_sound_io_map);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(12'000'000) / 8));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 2.0);

	OKIM6295(config, m_oki[0], XTAL(16'000'000) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.15);

	OKIM6295(config, m_oki[1], XTAL(16'000'000) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.15);

	nmk112_device &nmk112(NMK112(config, "nmk112", 0));
	nmk112.set_rom0_tag("oki1");
	nmk112.set_rom1_tag("oki2");
}

void powerinsa_state::powerinsa(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &powerinsa_state::program_map);

	m_screen->set_raw(XTAL(14'318'181) / 2, 456, 0, 320, 262, 16, 240); // ~60hz, XTAL verified, correct params?
	m_screen->screen_vblank().set(FUNC(powerinsa_state::screen_vblank_bootleg));

	OKIM6295(config, m_oki[0], 990'000, okim6295_device::PIN7_LOW); // pin7 not verified
	m_oki[0]->set_addrmap(0, &powerinsa_state::oki_map);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.15);
}

void powerins_state::powerinsb(machine_config &config)
{
	base(config);

	// basic machine hardware
	Z80(config, m_audiocpu, XTAL(12'000'000) / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &powerins_state::audio_program_map);
	m_audiocpu->set_addrmap(AS_IO, &powerins_state::macross2_sound_io_map);

	m_screen->set_raw(XTAL(14'318'181) / 2, 456, 0, 320, 262, 16, 240); // ~60hz, XTAL verified, correct params?
	m_screen->screen_vblank().set(FUNC(powerins_state::screen_vblank_bootleg));

	m_audiocpu->set_addrmap(AS_IO, &powerins_state::bootleg_audio_io_map);
	m_audiocpu->set_periodic_int(FUNC(powerins_state::irq0_line_hold), attotime::from_hz(120));  // YM2203 rate is at 150??

	OKIM6295(config, m_oki[0], XTAL(16'000'000) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.15);

	OKIM6295(config, m_oki[1], XTAL(16'000'000) / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.15);

	nmk112_device &nmk112(NMK112(config, "nmk112", 0));
	nmk112.set_rom0_tag("oki1");
	nmk112.set_rom1_tag("oki2");

	// Sound code talks to one YM2203, but it's not fitted on the board
}

void powerins_state::powerinsc(machine_config &config)
{
	powerinsb(config);

	m_gfxdecode->set_info(gfx_powerinsc);
}

/***************************************************************************

                                ROMs Loading

***************************************************************************/

/*

Gouketsuji Ichizoku (Power Instinct Japan)
Atlus, 1993

PCB Layout
----------

OS93095 (C) ATLUS 1993 MADE IN JAPAN
|---------------------------------------------------------------|
|LA4460  VOL  YM2203 Z80   93095-2  12MHz      AAA64K1P-35(x8)  |
|4558 YM3014  M6295          6264                               |
|                  93095-11 |------|          CXK58258BP-35L(x8)|
|          M6295   93095-10 |NMK112|                            |
|                  93095-9  |      |                            |
|   16MHz          93095-8  |------|           |------||------| |
|    DSW1  DSW2   |------|    22               |NMK009||NMK009| |
|J                |NMK005|                     |      ||      | |
|A                |      |                     |------||------| |
|M                |------|            |------|                  |
|M                            6116    |NMK008|         93095-19 |
|A |------|6116               6116    |      |                  |
|  |NMK111|6116                       |------|         93095-18 |
|  |      |       |------|                                      |
|  |------|       |NMK901|    62256    93095-4         93095-17 |
| 6264            |      |    62256    93095-3J                 |
| 6264    93095-7 |------|          |------------|     93095-16 |
||------|                    20     |   68000    |              |
||NMK111| 93095-6 |---| |---|       |------------|     93095-15 |
||      |         |NMK| |NMK|                                   |
||------| 93095-5 |903| |903|                          93095-14 |
||------|         |---| |---||---| 21                           |
||NMK111| 93095-1            |NMK|                     93095-13 |
||      |          6116      |902|    14MHz                     |
||------|          6116      |---|                     93095-12 |
|---------------------------------------------------------------|
Notes:
      68000 clock - 12.000MHz
      Z80 clock   - 6.000MHz [12/2]
      6295 clocks - 4.000MHz [16/4], sample rate = 4000000 / 165
      YM2203 clock- 1.500MHz [12/8]
      VSync       - 56Hz
      HSync       - 15.35kHz

      ROMs -
            -1, -2   : 27C1001 EPROM
            -3, -4   : 27C4096 EPROM
            -5, -6   : 8M 42 pin mask ROM (578200)
            -7       : 4M 40 pin mask ROM (574200)
            -8 to -19: 8M 42 pin mask ROM (578200)
            20       : 82S129 PROM
            21       : 82S135 PROM
            22       : 82S123 PROM

*/

ROM_START( powerins )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "93095-3a.u108", 0x00000, 0x80000, CRC(9825ea3d) SHA1(567fd8e3d866a58a68608ea20c5d3fc16cf9f444) )
	ROM_LOAD16_WORD_SWAP( "93095-4.u109",  0x80000, 0x80000, CRC(d3d7a782) SHA1(7846de0ebb09bd9b2534cd451ff9aa5175e60647) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "93095-2.u90",  0x00000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "93095-5.u16",  0x000000, 0x100000, CRC(b1371808) SHA1(15fca313314ff2e0caff35841a2fdda97f6235a8) )
	ROM_LOAD( "93095-6.u17",  0x100000, 0x100000, CRC(29c85d80) SHA1(abd54f9c8bade21ea918a426627199da04193165) )
	ROM_LOAD( "93095-7.u18",  0x200000, 0x080000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "93095-1.u15",  0x000000, 0x020000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "93095-12.u116", 0x000000, 0x100000, CRC(35f3c2a3) SHA1(70efebfe248401ba3d766dc0e4bcc2846cd0d9a0) )
	ROM_LOAD16_WORD_SWAP( "93095-13.u117", 0x100000, 0x100000, CRC(1ebd45da) SHA1(99b0ac734890673064b2a4b4b57ff2694e338dea) )
	ROM_LOAD16_WORD_SWAP( "93095-14.u118", 0x200000, 0x100000, CRC(760d871b) SHA1(4887122ad0518c90f08c11a7a6b694f3fd218498) )
	ROM_LOAD16_WORD_SWAP( "93095-15.u119", 0x300000, 0x100000, CRC(d011be88) SHA1(837409a2584abdf22e022b0f06181a600a974cbe) )
	ROM_LOAD16_WORD_SWAP( "93095-16.u120", 0x400000, 0x100000, CRC(a9c16c9c) SHA1(a34e81324c875c2a57f778d1dbdda8da81850a29) )
	ROM_LOAD16_WORD_SWAP( "93095-17.u121", 0x500000, 0x100000, CRC(51b57288) SHA1(821473d51565bc0a8b9a979723ce1307b97e517e) )
	ROM_LOAD16_WORD_SWAP( "93095-18.u122", 0x600000, 0x100000, CRC(b135e3f2) SHA1(339fb4007ca0f379b7554a1c4f711f494a371fb2) )
	ROM_LOAD16_WORD_SWAP( "93095-19.u123", 0x700000, 0x100000, CRC(67695537) SHA1(4c78ce3e36f27d2a6a9e50e8bf896335d4d0958a) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "93095-10.u48", 0x040000, 0x100000, CRC(329ac6c5) SHA1(e809b94e2623141f5a48995cfa97fe1ead7ab40b) )
	ROM_LOAD( "93095-11.u49", 0x140000, 0x100000, CRC(75d6097c) SHA1(3c89a7c9b12087e2d969b822419d3e5f98f5cb1d) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "93095-8.u46",  0x040000, 0x100000, CRC(f019bedb) SHA1(4b6e10f85671c75b666e547887d403d6e607cec8) )
	ROM_LOAD( "93095-9.u47",  0x140000, 0x100000, CRC(adc83765) SHA1(9e760443f9de21c1bb7e33eaa1541023fcdc60ab) )

	ROM_REGION( 0x0220, "proms", 0 ) // unknown
	ROM_LOAD( "22.u81",       0x000000, 0x0020, CRC(67d5ec4b) SHA1(87d32948a0c88277dcdd0eaa035bde40fc7db5fe) )
	ROM_LOAD( "21.u71",       0x000020, 0x0100, CRC(182cd81f) SHA1(3a76bea81b34ea7ccf56044206721058aa5b03e6) )
	ROM_LOAD( "20.u54",       0x000100, 0x0100, CRC(38bd0e2f) SHA1(20d311869642cd96bb831fdf4a458e0d872f03eb) )
ROM_END

ROM_START( powerinsj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "93095-3j.u108", 0x00000, 0x80000, CRC(3050a3fb) SHA1(e7e729bf62266e2e78ccd84cf937abb99de18ad5) )
	ROM_LOAD16_WORD_SWAP( "93095-4.u109",  0x80000, 0x80000, CRC(d3d7a782) SHA1(7846de0ebb09bd9b2534cd451ff9aa5175e60647) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "93095-2.u90",  0x00000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "93095-5.u16",  0x000000, 0x100000, CRC(b1371808) SHA1(15fca313314ff2e0caff35841a2fdda97f6235a8) )
	ROM_LOAD( "93095-6.u17",  0x100000, 0x100000, CRC(29c85d80) SHA1(abd54f9c8bade21ea918a426627199da04193165) )
	ROM_LOAD( "93095-7.u18",  0x200000, 0x080000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "93095-1.u15",  0x000000, 0x020000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "93095-12.u116", 0x000000, 0x100000, CRC(35f3c2a3) SHA1(70efebfe248401ba3d766dc0e4bcc2846cd0d9a0) )
	ROM_LOAD16_WORD_SWAP( "93095-13.u117", 0x100000, 0x100000, CRC(1ebd45da) SHA1(99b0ac734890673064b2a4b4b57ff2694e338dea) )
	ROM_LOAD16_WORD_SWAP( "93095-14.u118", 0x200000, 0x100000, CRC(760d871b) SHA1(4887122ad0518c90f08c11a7a6b694f3fd218498) )
	ROM_LOAD16_WORD_SWAP( "93095-15.u119", 0x300000, 0x100000, CRC(d011be88) SHA1(837409a2584abdf22e022b0f06181a600a974cbe) )
	ROM_LOAD16_WORD_SWAP( "93095-16.u120", 0x400000, 0x100000, CRC(a9c16c9c) SHA1(a34e81324c875c2a57f778d1dbdda8da81850a29) )
	ROM_LOAD16_WORD_SWAP( "93095-17.u121", 0x500000, 0x100000, CRC(51b57288) SHA1(821473d51565bc0a8b9a979723ce1307b97e517e) )
	ROM_LOAD16_WORD_SWAP( "93095-18.u122", 0x600000, 0x100000, CRC(b135e3f2) SHA1(339fb4007ca0f379b7554a1c4f711f494a371fb2) )
	ROM_LOAD16_WORD_SWAP( "93095-19.u123", 0x700000, 0x100000, CRC(67695537) SHA1(4c78ce3e36f27d2a6a9e50e8bf896335d4d0958a) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "93095-10.u48", 0x040000, 0x100000, CRC(329ac6c5) SHA1(e809b94e2623141f5a48995cfa97fe1ead7ab40b) )
	ROM_LOAD( "93095-11.u49", 0x140000, 0x100000, CRC(75d6097c) SHA1(3c89a7c9b12087e2d969b822419d3e5f98f5cb1d) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "93095-8.u46",  0x040000, 0x100000, CRC(f019bedb) SHA1(4b6e10f85671c75b666e547887d403d6e607cec8) )
	ROM_LOAD( "93095-9.u47",  0x140000, 0x100000, CRC(adc83765) SHA1(9e760443f9de21c1bb7e33eaa1541023fcdc60ab) )

	ROM_REGION( 0x0220, "proms", 0 ) // unknown
	ROM_LOAD( "22.u81",       0x000000, 0x0020, CRC(67d5ec4b) SHA1(87d32948a0c88277dcdd0eaa035bde40fc7db5fe) )
	ROM_LOAD( "21.u71",       0x000020, 0x0100, CRC(182cd81f) SHA1(3a76bea81b34ea7ccf56044206721058aa5b03e6) )
	ROM_LOAD( "20.u54",       0x000100, 0x0100, CRC(38bd0e2f) SHA1(20d311869642cd96bb831fdf4a458e0d872f03eb) )
ROM_END

ROM_START( powerinspu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "3.p000.v4.0a.u116.27c240", 0x000000, 0x80000, CRC(d1dd5a3f) SHA1(b2a52a2bbdf63eddc04bae2b4322d6d320f35e89) )
	ROM_LOAD16_WORD_SWAP( "4.p000.v3.8.u117.27c4096", 0x080000, 0x80000, CRC(9c0f23cf) SHA1(9ac78939a743c340aa51ff1b05817866124acd34) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "2.sound 9.20.u74.27c1001",  0x000000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "ba0.s0.27c040", 0x000000, 0x80000, CRC(1975b4b8) SHA1(cb400967744fa602df1bd2d88950dfdbdc77073f) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ba1.s1.27c040", 0x080000, 0x80000, CRC(376e4919) SHA1(12baa17382c176838df1b5ef86f1fa6dbcb978dd) )
	ROM_LOAD( "ba2.s2.27c040", 0x100000, 0x80000, CRC(0d5ff532) SHA1(4febdb9cdacd85903a4a28e8df945dee0ce85558) )
	ROM_LOAD( "ba3.s3.27c040", 0x180000, 0x80000, CRC(99b25791) SHA1(82f4bb5780826773d2e5f7143afb3ba209f57652) )
	ROM_LOAD( "ba4.s4.27c040", 0x200000, 0x80000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "1.text 1080.u16.27c010", 0x000000, 0x20000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fo0.mo0.27c040", 0x000001, 0x80000, CRC(8b9b89c9) SHA1(f1d39d1a62e40a14642d8f22fc38b764465a8daa) ) // located on OS93089 SUB daughterboard
	ROM_LOAD16_BYTE( "fe0.me0.27c040", 0x000000, 0x80000, CRC(4d127bdf) SHA1(26a7c277e7660a7c7c0c11cacadf815d2487ba8a) )
	ROM_LOAD16_BYTE( "fo1.mo1.27c040", 0x100001, 0x80000, CRC(298eb50e) SHA1(2b922c1473bb559a1e8bd6221619141658179bb9) )
	ROM_LOAD16_BYTE( "fe1.me1.27c040", 0x100000, 0x80000, CRC(57e6d283) SHA1(4701576c8663ba47f388a02e61ef078a9dbbd212) )
	ROM_LOAD16_BYTE( "fo2.mo2.27c040", 0x200001, 0x80000, CRC(fb184167) SHA1(20924d3f35509f2f6af61f565b852ea72326d02c) )
	ROM_LOAD16_BYTE( "fe2.me2.27c040", 0x200000, 0x80000, CRC(1b752a4d) SHA1(1b13f28af208542bee9da298d6e9db676cbc0845) )
	ROM_LOAD16_BYTE( "fo3.mo3.27c040", 0x300001, 0x80000, CRC(2f26ba7b) SHA1(026f960fa4de09ed940dd83a3db467c3676c5024) )
	ROM_LOAD16_BYTE( "fe3.me3.27c040", 0x300000, 0x80000, CRC(0263d89b) SHA1(526b8ed05dffcbe98a44372bd55ad7b0ba91fc0f) )
	ROM_LOAD16_BYTE( "fo4.mo4.27c040", 0x400001, 0x80000, CRC(c4633294) SHA1(9578f516eaf09e743ee0262ce227f811bea1be8f) )
	ROM_LOAD16_BYTE( "fe4.me4.27c040", 0x400000, 0x80000, CRC(5e4b5655) SHA1(f86509e75ec0c68f728715a5a325f6d1a30cfd93) )
	ROM_LOAD16_BYTE( "fo5.mo5.27c040", 0x500001, 0x80000, CRC(4d4b0e4e) SHA1(782c5edc533f10757cb18d2411046e44aa075ba1) )
	ROM_LOAD16_BYTE( "fe5.me5.27c040", 0x500000, 0x80000, CRC(7e9f2d2b) SHA1(cfee03c38a6c781ad370638748244a164b83d588) )
	ROM_LOAD16_BYTE( "fo6.mo6.27c040", 0x600001, 0x80000, CRC(0e7671f2) SHA1(301af5c4229451cba9fdf40285dd7243626ffed4) )
	ROM_LOAD16_BYTE( "fe6.me6.27c040", 0x600000, 0x80000, CRC(ee59b1ec) SHA1(437bc50c3b32c2edee549f5021345f1c924896b4) )
	ROM_LOAD16_BYTE( "fo7.mo7.27c040", 0x700001, 0x80000, CRC(9ab1998c) SHA1(fadaa4a46cefe0093ee1ebeddbae63143fa7bb5a) )
	ROM_LOAD16_BYTE( "fe7.me7.27c040", 0x700000, 0x80000, CRC(1ab0c88a) SHA1(8bc72732f5911e0d4e0cf12fd2fb12d67e03299e) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "ao0.ad00.27c040", 0x040000, 0x80000, CRC(8cd6824e) SHA1(aa6d8917558de4f2aa8d80527209b9fe91122eb3) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ao1.ad01.27c040", 0x0c0000, 0x80000, CRC(e31ae04d) SHA1(c08d58a4250d8bdb68b8e5012624f345936520e1) )
	ROM_LOAD( "ao2.ad02.27c040", 0x140000, 0x80000, CRC(c4c9f599) SHA1(1d74acd626406052bec533a918ca24e14a2578f2) )
	ROM_LOAD( "ao3.ad03.27c040", 0x1c0000, 0x80000, CRC(f0a9f0e1) SHA1(4221e0824cdc8bcd6ea1c3811f4e3b7cd99478f2) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "ad10.ad10.27c040", 0x040000, 0x80000, CRC(62557502) SHA1(d72abdaec1c6f55f9b0099b7a8a297e0e14f920c) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ad11.ad11.27c040", 0x0c0000, 0x80000, CRC(dbc86bd7) SHA1(6f1bc3c7e6976fdcd4b2341cea07002fb0cefb14) )
	ROM_LOAD( "ad12.ad12.27c040", 0x140000, 0x80000, CRC(5839a2bd) SHA1(53988086ef97b2671044f6da9d97b1886900b64d) )
	ROM_LOAD( "ad13.ad13.27c040", 0x1c0000, 0x80000, CRC(446f9dc3) SHA1(5c81eb9a7cbea995db9a10d3b6460d02e104825f) )

	ROM_REGION( 0x0220, "proms", 0 ) // unknown
	ROM_LOAD( "22.u81",       0x000000, 0x0020, CRC(67d5ec4b) SHA1(87d32948a0c88277dcdd0eaa035bde40fc7db5fe) )
	ROM_LOAD( "21.u71",       0x000020, 0x0100, CRC(182cd81f) SHA1(3a76bea81b34ea7ccf56044206721058aa5b03e6) )
	ROM_LOAD( "20.u54",       0x000100, 0x0100, CRC(38bd0e2f) SHA1(20d311869642cd96bb831fdf4a458e0d872f03eb) )
ROM_END

ROM_START( powerinspj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "3.p000.pc_j_12-1_155e.u116", 0x000000, 0x80000, CRC(4ea18490) SHA1(6b933b7ee11c65adf15430c8b185feaddb1c0bb0) ) // labeled:  PC J 12/1 155E
	ROM_LOAD16_WORD_SWAP( "4.p000.f_p4.u117",           0x080000, 0x80000, CRC(9c0f23cf) SHA1(9ac78939a743c340aa51ff1b05817866124acd34) ) // labeled:  F P4

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "2.sound 9.20.u74.27c1001",  0x000000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "ba0.s0.27c040", 0x000000, 0x80000, CRC(1975b4b8) SHA1(cb400967744fa602df1bd2d88950dfdbdc77073f) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ba1.s1.27c040", 0x080000, 0x80000, CRC(376e4919) SHA1(12baa17382c176838df1b5ef86f1fa6dbcb978dd) )
	ROM_LOAD( "ba2.s2.27c040", 0x100000, 0x80000, CRC(0d5ff532) SHA1(4febdb9cdacd85903a4a28e8df945dee0ce85558) )
	ROM_LOAD( "ba3.s3.27c040", 0x180000, 0x80000, CRC(99b25791) SHA1(82f4bb5780826773d2e5f7143afb3ba209f57652) )
	ROM_LOAD( "ba4.s4.27c040", 0x200000, 0x80000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "1.text 1080.u16.27c010", 0x000000, 0x20000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "fo0.mo0.27c040", 0x000001, 0x80000, CRC(8b9b89c9) SHA1(f1d39d1a62e40a14642d8f22fc38b764465a8daa) ) // located on OS93089 SUB daughterboard
	ROM_LOAD16_BYTE( "fe0.me0.27c040", 0x000000, 0x80000, CRC(4d127bdf) SHA1(26a7c277e7660a7c7c0c11cacadf815d2487ba8a) )
	ROM_LOAD16_BYTE( "fo1.mo1.27c040", 0x100001, 0x80000, CRC(298eb50e) SHA1(2b922c1473bb559a1e8bd6221619141658179bb9) )
	ROM_LOAD16_BYTE( "fe1.me1.27c040", 0x100000, 0x80000, CRC(57e6d283) SHA1(4701576c8663ba47f388a02e61ef078a9dbbd212) )
	ROM_LOAD16_BYTE( "fo2.mo2.27c040", 0x200001, 0x80000, CRC(fb184167) SHA1(20924d3f35509f2f6af61f565b852ea72326d02c) )
	ROM_LOAD16_BYTE( "fe2.me2.27c040", 0x200000, 0x80000, CRC(1b752a4d) SHA1(1b13f28af208542bee9da298d6e9db676cbc0845) )
	ROM_LOAD16_BYTE( "fo3.mo3.27c040", 0x300001, 0x80000, CRC(2f26ba7b) SHA1(026f960fa4de09ed940dd83a3db467c3676c5024) )
	ROM_LOAD16_BYTE( "fe3.me3.27c040", 0x300000, 0x80000, CRC(0263d89b) SHA1(526b8ed05dffcbe98a44372bd55ad7b0ba91fc0f) )
	ROM_LOAD16_BYTE( "fo4.mo4.27c040", 0x400001, 0x80000, CRC(c4633294) SHA1(9578f516eaf09e743ee0262ce227f811bea1be8f) )
	ROM_LOAD16_BYTE( "fe4.me4.27c040", 0x400000, 0x80000, CRC(5e4b5655) SHA1(f86509e75ec0c68f728715a5a325f6d1a30cfd93) )
	ROM_LOAD16_BYTE( "fo5.mo5.27c040", 0x500001, 0x80000, CRC(4d4b0e4e) SHA1(782c5edc533f10757cb18d2411046e44aa075ba1) )
	ROM_LOAD16_BYTE( "fe5.me5.27c040", 0x500000, 0x80000, CRC(7e9f2d2b) SHA1(cfee03c38a6c781ad370638748244a164b83d588) )
	ROM_LOAD16_BYTE( "fo6.mo6.27c040", 0x600001, 0x80000, CRC(0e7671f2) SHA1(301af5c4229451cba9fdf40285dd7243626ffed4) )
	ROM_LOAD16_BYTE( "fe6.me6.27c040", 0x600000, 0x80000, CRC(ee59b1ec) SHA1(437bc50c3b32c2edee549f5021345f1c924896b4) )
	ROM_LOAD16_BYTE( "fo7.mo7.27c040", 0x700001, 0x80000, CRC(9ab1998c) SHA1(fadaa4a46cefe0093ee1ebeddbae63143fa7bb5a) )
	ROM_LOAD16_BYTE( "fe7.me7.27c040", 0x700000, 0x80000, CRC(1ab0c88a) SHA1(8bc72732f5911e0d4e0cf12fd2fb12d67e03299e) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "ao0.ad00.27c040", 0x040000, 0x80000, CRC(8cd6824e) SHA1(aa6d8917558de4f2aa8d80527209b9fe91122eb3) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ao1.ad01.27c040", 0x0c0000, 0x80000, CRC(e31ae04d) SHA1(c08d58a4250d8bdb68b8e5012624f345936520e1) )
	ROM_LOAD( "ao2.ad02.27c040", 0x140000, 0x80000, CRC(c4c9f599) SHA1(1d74acd626406052bec533a918ca24e14a2578f2) )
	ROM_LOAD( "ao3.ad03.27c040", 0x1c0000, 0x80000, CRC(f0a9f0e1) SHA1(4221e0824cdc8bcd6ea1c3811f4e3b7cd99478f2) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "ad10.ad10.27c040", 0x040000, 0x80000, CRC(62557502) SHA1(d72abdaec1c6f55f9b0099b7a8a297e0e14f920c) ) // located on OS93089 SUB daughterboard
	ROM_LOAD( "ad11.ad11.27c040", 0x0c0000, 0x80000, CRC(dbc86bd7) SHA1(6f1bc3c7e6976fdcd4b2341cea07002fb0cefb14) )
	ROM_LOAD( "ad12.ad12.27c040", 0x140000, 0x80000, CRC(5839a2bd) SHA1(53988086ef97b2671044f6da9d97b1886900b64d) )
	ROM_LOAD( "ad13.ad13.27c040", 0x1c0000, 0x80000, CRC(446f9dc3) SHA1(5c81eb9a7cbea995db9a10d3b6460d02e104825f) )

	ROM_REGION( 0x0220, "proms", 0 ) // unknown
	ROM_LOAD( "22.u81",       0x000000, 0x0020, CRC(67d5ec4b) SHA1(87d32948a0c88277dcdd0eaa035bde40fc7db5fe) )
	ROM_LOAD( "21.u71",       0x000020, 0x0100, CRC(182cd81f) SHA1(3a76bea81b34ea7ccf56044206721058aa5b03e6) )
	ROM_LOAD( "20.u54",       0x000100, 0x0100, CRC(38bd0e2f) SHA1(20d311869642cd96bb831fdf4a458e0d872f03eb) )
ROM_END

/***************************************************************************

                                Power Instinct

Location     Device       File ID     Checksum
----------------------------------------------
             27C240        ROM1         4EA1    [ MAIN PROGRAM ]
             27C240        ROM2         FE60    [ PROGRAM DATA ]
             27C010        ROM3         B9F7    [  CHARACTER   ]
             27C040        ROM4         2780    [  BACKGROUND  ]
             27C040        ROM5         98E0    [   PCM DATA   ]
            23C1600        ROM6         D9E9    [  BACKGROUND  ]
            23C1600        ROM7         8B04    [  MOTION OBJ  ]
            23C1600        ROM8         54B2    [  MOTION OBJ  ]
            23C1600        ROM9         C7C8    [  MOTION OBJ  ]
            23C1600        ROM10        852A    [  MOTION OBJ  ]

Notes:  This archive is of a bootleg version

Brief hardware overview
-----------------------

Main processor  -  68000
                -  TPC1020AFN-084C (CPLD)

Sound processor -  Main processor
                -  K-665-9249      (M6295)

***************************************************************************/

ROM_START( powerinsa )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "rom1", 0x000000, 0x080000, CRC(b86c84d6) SHA1(2ec0933130925dfae859ea6abe62a8c92385aee8) )
	ROM_LOAD16_WORD_SWAP( "rom2", 0x080000, 0x080000, CRC(d3d7a782) SHA1(7846de0ebb09bd9b2534cd451ff9aa5175e60647) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "rom6",  0x000000, 0x200000, CRC(b6c10f80) SHA1(feece0aeaa01a455d0c4885a3699f8bda14fe00f) )
	ROM_LOAD( "rom4",  0x200000, 0x080000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "rom3",  0x000000, 0x020000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_WORD_SWAP( "rom10", 0x000000, 0x200000, CRC(efad50e8) SHA1(89e8c307b927e987a32d22ab4ab7f3be037cca03) )
	ROM_LOAD16_WORD_SWAP( "rom9",  0x200000, 0x200000, CRC(08229592) SHA1(759679e89832b475adfdc783630d9ee2c105b0f3) )
	ROM_LOAD16_WORD_SWAP( "rom8",  0x400000, 0x200000, CRC(b02fdd6d) SHA1(1e2c52b4e9999f0b564fcf13ff41b097ad7d0c39) )
	ROM_LOAD16_WORD_SWAP( "rom7",  0x600000, 0x200000, CRC(92ab9996) SHA1(915ec8f383cc3652c3816a9b56ee54e22e104a5c) )

	ROM_REGION( 0x080000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "rom5", 0x000000, 0x080000, CRC(88579c8f) SHA1(13083934ab294c9b08d3e36f55c00a6a2e5a0507) )
ROM_END

/***************************************************************************

Power Instinct
Atlus, 1993

This is a bootleg US version with different sound hardware to the existing bootleg set.
The PCB is very large and has 2 plug-in daughterboards and many mask ROMs.
The addition of the contents of the mask ROMs would probably equal the contents of presumably
larger mask ROMs found on the original PCB....

PCB Layout

|-------------------------------------------------------------|
|   M6295    4A 5A                             62256  62256   |
|   M6295    4B 5B                             62256  62256   |
|            4C 5C                             62256  62256   |
|            4D 5D                             62256  62256   |
| Z80        16MHz                             62256  62256   |
| 1F                                           62256  62256   |
| 6264       6116                              62256  62256   |
|            6116                              62256  62256   |
|J                                                            |
|A                                                            |
|M                                                            |
|M                                                            |
|A                                    82S123  11G 12G 13G  14G|
|                                             11I             |
|                           TPC1020  6116     11J             |
|DSW1        6116  6N                6116     11K     13K     |
|DSW2        6116              6264           11L     13L  14M|
|                      82S147  6264           11O     13O  14N|
|                                             11P 12P 13P  14P|
|      2Q    62256                            11Q     13Q     |
|      2R    62256                                    13R     |
|      68000                                                  |
| 12MHz              14.31818MHz                              |
|-------------------------------------------------------------|

Notes:
      68000 clock: 12.000MHz
      Z80 clock  :  6.000MHz
      M6295 clock:  4.000MHz (both); sample rate = 4000000/165 (both)
      VSync      :  60Hz

      ROMs 1F and 6N are 1M mask (MX27C1000), all other ROMs are 4M mask (MX27C4000).
      ROMS at 5* are located on a plug-in daughterboard.
      ROMS at 11*, 12*, 13G, 13P and 14* are located on a plug-in daughterboard.
      82S123 and 82S147 are PROMs.

***************************************************************************/

ROM_START( powerinsb )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2q.bin", 0x000000, 0x80000, CRC(11bf3f2a) SHA1(c840add78da9b19839c667f9bbd77e0a7c560ed7) )
	ROM_LOAD16_BYTE( "2r.bin", 0x000001, 0x80000, CRC(d8d621be) SHA1(91d501ac661c1ff52c85eee96c455c008a7dad1c) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "1f.bin",  0x000000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x280000, "bgtile", 0 )
	ROM_LOAD( "13k.bin", 0x000000, 0x80000, CRC(1975b4b8) SHA1(cb400967744fa602df1bd2d88950dfdbdc77073f) )
	ROM_LOAD( "13l.bin", 0x080000, 0x80000, CRC(376e4919) SHA1(12baa17382c176838df1b5ef86f1fa6dbcb978dd) )
	ROM_LOAD( "13o.bin", 0x100000, 0x80000, CRC(0d5ff532) SHA1(4febdb9cdacd85903a4a28e8df945dee0ce85558) )
	ROM_LOAD( "13q.bin", 0x180000, 0x80000, CRC(99b25791) SHA1(82f4bb5780826773d2e5f7143afb3ba209f57652) )
	ROM_LOAD( "13r.bin", 0x200000, 0x80000, CRC(2dd76149) SHA1(975e4d371fdfbbd9a568da4d4c91ffd3f0ae636e) )

	ROM_REGION( 0x100000, "fgtile", 0 )
	ROM_LOAD( "6n.bin", 0x000000, 0x20000, CRC(6a579ee0) SHA1(438e87b930e068e0cf7352e614a14049ebde6b8a) )

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "14g.bin", 0x000001, 0x80000, CRC(8b9b89c9) SHA1(f1d39d1a62e40a14642d8f22fc38b764465a8daa) )
	ROM_LOAD16_BYTE( "11g.bin", 0x000000, 0x80000, CRC(4d127bdf) SHA1(26a7c277e7660a7c7c0c11cacadf815d2487ba8a) )
	ROM_LOAD16_BYTE( "13g.bin", 0x100001, 0x80000, CRC(298eb50e) SHA1(2b922c1473bb559a1e8bd6221619141658179bb9) )
	ROM_LOAD16_BYTE( "11i.bin", 0x100000, 0x80000, CRC(57e6d283) SHA1(4701576c8663ba47f388a02e61ef078a9dbbd212) )
	ROM_LOAD16_BYTE( "12g.bin", 0x200001, 0x80000, CRC(fb184167) SHA1(20924d3f35509f2f6af61f565b852ea72326d02c) )
	ROM_LOAD16_BYTE( "11j.bin", 0x200000, 0x80000, CRC(1b752a4d) SHA1(1b13f28af208542bee9da298d6e9db676cbc0845) )
	ROM_LOAD16_BYTE( "14m.bin", 0x300001, 0x80000, CRC(2f26ba7b) SHA1(026f960fa4de09ed940dd83a3db467c3676c5024) )
	ROM_LOAD16_BYTE( "11k.bin", 0x300000, 0x80000, CRC(0263d89b) SHA1(526b8ed05dffcbe98a44372bd55ad7b0ba91fc0f) )
	ROM_LOAD16_BYTE( "14n.bin", 0x400001, 0x80000, CRC(c4633294) SHA1(9578f516eaf09e743ee0262ce227f811bea1be8f) )
	ROM_LOAD16_BYTE( "11l.bin", 0x400000, 0x80000, CRC(5e4b5655) SHA1(f86509e75ec0c68f728715a5a325f6d1a30cfd93) )
	ROM_LOAD16_BYTE( "14p.bin", 0x500001, 0x80000, CRC(4d4b0e4e) SHA1(782c5edc533f10757cb18d2411046e44aa075ba1) )
	ROM_LOAD16_BYTE( "11o.bin", 0x500000, 0x80000, CRC(7e9f2d2b) SHA1(cfee03c38a6c781ad370638748244a164b83d588) )
	ROM_LOAD16_BYTE( "13p.bin", 0x600001, 0x80000, CRC(0e7671f2) SHA1(301af5c4229451cba9fdf40285dd7243626ffed4) )
	ROM_LOAD16_BYTE( "11p.bin", 0x600000, 0x80000, CRC(ee59b1ec) SHA1(437bc50c3b32c2edee549f5021345f1c924896b4) )
	ROM_LOAD16_BYTE( "12p.bin", 0x700001, 0x80000, CRC(9ab1998c) SHA1(fadaa4a46cefe0093ee1ebeddbae63143fa7bb5a) )
	ROM_LOAD16_BYTE( "11q.bin", 0x700000, 0x80000, CRC(1ab0c88a) SHA1(8bc72732f5911e0d4e0cf12fd2fb12d67e03299e) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "4a.bin", 0x040000, 0x80000, CRC(8cd6824e) SHA1(aa6d8917558de4f2aa8d80527209b9fe91122eb3) )
	ROM_LOAD( "4b.bin", 0x0c0000, 0x80000, CRC(e31ae04d) SHA1(c08d58a4250d8bdb68b8e5012624f345936520e1) )
	ROM_LOAD( "4c.bin", 0x140000, 0x80000, CRC(c4c9f599) SHA1(1d74acd626406052bec533a918ca24e14a2578f2) )
	ROM_LOAD( "4d.bin", 0x1c0000, 0x80000, CRC(f0a9f0e1) SHA1(4221e0824cdc8bcd6ea1c3811f4e3b7cd99478f2) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "5a.bin", 0x040000, 0x80000, CRC(62557502) SHA1(d72abdaec1c6f55f9b0099b7a8a297e0e14f920c) )
	ROM_LOAD( "5b.bin", 0x0c0000, 0x80000, CRC(dbc86bd7) SHA1(6f1bc3c7e6976fdcd4b2341cea07002fb0cefb14) )
	ROM_LOAD( "5c.bin", 0x140000, 0x80000, CRC(5839a2bd) SHA1(53988086ef97b2671044f6da9d97b1886900b64d) )
	ROM_LOAD( "5d.bin", 0x1c0000, 0x80000, CRC(446f9dc3) SHA1(5c81eb9a7cbea995db9a10d3b6460d02e104825f) )

	ROM_REGION( 0x0220, "proms", 0 ) // unknown
	ROM_LOAD( "82s123.bin", 0x0000, 0x0020, CRC(67d5ec4b) SHA1(87d32948a0c88277dcdd0eaa035bde40fc7db5fe) )
	ROM_LOAD( "82s147.bin", 0x0020, 0x0200, CRC(d7818542) SHA1(e94f8004c804f260874a117d59dfa0637c5d3d73) )
ROM_END

ROM_START( powerinsc )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "10.040.u41", 0x000000, 0x80000, CRC(88e1244b) SHA1(595a560b807eab9576ed057a7e532c83860e9c40) )
	ROM_LOAD16_BYTE( "11.040.u39", 0x000001, 0x80000, CRC(46cd506f) SHA1(4585824f65e2b7da9f815fee92bb5f6d250a286d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 code
	ROM_LOAD( "1.010.u2",  0x000000, 0x20000, CRC(4b123cc6) SHA1(ed61d3a2ab20c86b91fd7bafa717be3ce26159be) )

	ROM_REGION( 0x300000, "bgtile", 0 )
	ROM_LOAD16_BYTE( "33.040.u99",  0x000000, 0x80000, CRC(9b56a394) SHA1(f9451d8d5a911f4daa0f57af496dae08d320b3b2) )
	ROM_LOAD16_BYTE( "22.040.u97",  0x000001, 0x80000, CRC(1e693f05) SHA1(049eeabd9b4f55f2f314f4f6871b1a0e1ec39517) )
	ROM_LOAD16_BYTE( "32.040.u100", 0x100000, 0x80000, CRC(7749bc80) SHA1(ceee996e694865dfbc48b5365731f4903ca674f1) )
	ROM_LOAD16_BYTE( "21.040.u98",  0x100001, 0x80000, CRC(e1586a71) SHA1(29df13a35a0c679bad0955961e7e0e70f93482c9) )
	ROM_LOAD16_BYTE( "31.040.u102", 0x200000, 0x80000, CRC(ac5a2952) SHA1(1b15873045cd65aa823c81b293b20ef6c20c6aef) )
	ROM_LOAD16_BYTE( "20.040.u101", 0x200001, 0x80000, CRC(e4b2823c) SHA1(1ef41ff625ad82dcc85994f87e1d82fc11e26dd8) )

	// TODO: check sprites' ROM loading
	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "30.040.u82", 0x000000, 0x80000,  CRC(6668d29d) SHA1(41b7ab49b72a1ffc7618c3fc45a1c1bbe1d84d21) )
	ROM_LOAD16_BYTE( "19.040.u91", 0x000001, 0x80000,  CRC(17659d0c) SHA1(394b5dbb4461d0c05599d1ecd9fe92de999970fa) )
	ROM_LOAD16_BYTE( "29.040.u85", 0x100000, 0x80000,  CRC(c349e556) SHA1(a89d4292a6f3b3cfd5b85f8db6de207831e779e6) )
	ROM_LOAD16_BYTE( "18.040.u84", 0x100001, 0x80000,  CRC(8716f8d3) SHA1(469a967784b5ab44d91839ff0dd2b361f664ad7e) )
	ROM_LOAD16_BYTE( "28.040.u88", 0x200000, 0x80000,  CRC(f93f10ce) SHA1(de254d7c904de95d676cd283276ef8a5cafde588) )
	ROM_LOAD16_BYTE( "17.040.u87", 0x200001, 0x80000,  CRC(fa1ef844) SHA1(be11c84186f5e0a7990c005ed27b5f71e50bc450) )
	ROM_LOAD16_BYTE( "27.040.u91", 0x300000, 0x80000,  CRC(962f3455) SHA1(3637cd6047f94bc4fc8dd8d7fbc1a48b99993b0b) )
	ROM_LOAD16_BYTE( "16.040.u90", 0x300001, 0x80000,  CRC(e1a37b42) SHA1(37aa82ba166ff6549c9428e42bbad10f252d14f8) )
	ROM_LOAD16_BYTE( "26.040.u93", 0x400000, 0x80000,  CRC(6e65099c) SHA1(5c261367f086d52ec5680a0a9c0f85992c4473b9) )
	ROM_LOAD16_BYTE( "15.040.u81", 0x400001, 0x80000,  CRC(035316d3) SHA1(c1c6f243213f05a53f0fc4f3df530895c34355a9) )
	ROM_LOAD16_BYTE( "25.040.u94", 0x500000, 0x80000,  CRC(a250dea8) SHA1(6b4c5ad35f4f4cdab516118a21c58617044c3208) )
	ROM_LOAD16_BYTE( "14.040.u96", 0x500001, 0x80000,  CRC(dd976689) SHA1(ba7e80a94e6c6bb7a5b569fb5440e774cd89b79d) )
	ROM_LOAD16_BYTE( "24.040.u95", 0x600000, 0x80000,  CRC(851008f4) SHA1(cd6e5d8e6807fc3224022ca53f02f390e1232b06) )
	ROM_LOAD16_BYTE( "13.040.u89", 0x600001, 0x80000,  CRC(867262d6) SHA1(bf0b13a5bb818741150d09be44968779c55c5b96) )
	ROM_LOAD16_BYTE( "23.040.u96", 0x700000, 0x80000,  CRC(625c5b7b) SHA1(ddac164cd92459bdce5905b31eccded9b1c06086) )
	ROM_LOAD16_BYTE( "12.040.u92", 0x700001, 0x80000,  CRC(08c4e478) SHA1(172dd9532a9240014afb4817b61a3e8122be8f0c) )

	ROM_REGION( 0x240000, "oki1", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "9.040.u32", 0x040000, 0x80000, CRC(8cd6824e) SHA1(aa6d8917558de4f2aa8d80527209b9fe91122eb3) )
	ROM_LOAD( "8.040.u30", 0x0c0000, 0x80000, CRC(e31ae04d) SHA1(c08d58a4250d8bdb68b8e5012624f345936520e1) )
	ROM_LOAD( "7.040.u33", 0x140000, 0x80000, CRC(c4c9f599) SHA1(1d74acd626406052bec533a918ca24e14a2578f2) )
	ROM_LOAD( "6.040.u31", 0x1c0000, 0x80000, CRC(f0a9f0e1) SHA1(4221e0824cdc8bcd6ea1c3811f4e3b7cd99478f2) )

	ROM_REGION( 0x240000, "oki2", 0 )   // 8 bit ADPCM (banked)
	ROM_LOAD( "5.040.u36", 0x040000, 0x80000, CRC(62557502) SHA1(d72abdaec1c6f55f9b0099b7a8a297e0e14f920c) )
	ROM_LOAD( "4.040.u34", 0x0c0000, 0x80000, CRC(dbc86bd7) SHA1(6f1bc3c7e6976fdcd4b2341cea07002fb0cefb14) )
	ROM_LOAD( "3.040.u37", 0x140000, 0x80000, CRC(5839a2bd) SHA1(53988086ef97b2671044f6da9d97b1886900b64d) )
	ROM_LOAD( "2.040.u35", 0x1c0000, 0x80000, CRC(446f9dc3) SHA1(5c81eb9a7cbea995db9a10d3b6460d02e104825f) )
ROM_END

} // anonymous namespace


// all supported sets give a 93.10.20 date
GAME( 1993, powerins,   0,        powerins,  powerins, powerins_state,  empty_init, ROT0, "Atlus",   "Power Instinct (USA)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1993, powerinsj,  powerins, powerins,  powerinj, powerins_state,  empty_init, ROT0, "Atlus",   "Gouketsuji Ichizoku (Japan)",            MACHINE_SUPPORTS_SAVE )
GAME( 1993, powerinspu, powerins, powerins,  powerinj, powerins_state,  empty_init, ROT0, "Atlus",   "Power Instinct (USA, prototype)",        MACHINE_SUPPORTS_SAVE ) // boots as 93.10.20 just like the other sets, but code is different
GAME( 1993, powerinspj, powerins, powerins,  powerinj, powerins_state,  empty_init, ROT0, "Atlus",   "Gouketsuji Ichizoku (Japan, prototype)", MACHINE_SUPPORTS_SAVE ) // boots as 93.10.20 just like the other sets, but code is different
GAME( 1993, powerinsa,  powerins, powerinsa, powerins, powerinsa_state, empty_init, ROT0, "bootleg", "Power Instinct (USA, bootleg set 1)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, powerinsb,  powerins, powerinsb, powerins, powerins_state,  empty_init, ROT0, "bootleg", "Power Instinct (USA, bootleg set 2)",    MACHINE_SUPPORTS_SAVE )
GAME( 1993, powerinsc,  powerins, powerinsc, powerins, powerins_state,  empty_init, ROT0, "bootleg", "Power Instinct (USA, bootleg set 3)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // different sprites' format not implemented
