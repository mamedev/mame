// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************************

Seibu Mahjong games (distributed by Tecmo)

CPU: V30 D70116C-10
Sound: Z80 YM3812 M6295
OSC: 12.000MHz 16.000MHz 7.15909MHz


TODO:
 Clean up inputs (some missing dips)
 Some sprite flickers on attract mode
 totmejan: Are the "dots" behind the girls in attract mode correct?

Tottemo E Jong PCB Layout

|---------------------------------------------------------------|
|LA4460  YM3812  M6295  E-JAN.U0911 6116        Z80A 7.15909MHz |
|                                                               |
|                                    5.U1016                    |
|                SEI0100                        PAL             |
|                                                               |
|                           E-JAN.U064          4.U061          |
|                                                      SEI0160  |
|           SEI0181                             3.U063          |
|                                                               |
|                                                               |
|           PAL             SEI0200           6264        PAL   |
|                           TC110G21AF                          |
|           82S135.U083                 PAL   6264              |
|                                                               |
|  DSW     DSW     SEI0220BP                                    |
|                                              62256     62256  |
|  E-JAN.U078                PAL                                |
|                                                               |
| PAL                        PAL               1.U022    2.U023 |
|            SEI0210                                            |
| PAL                        PAL                                |
|                                                               |
| 12MHz                16MHz        V30                         |
|---------------------------------------------------------------|
Notes:
      V30 clock - 8.000MHz [16/2]
      Z80 clock - 3.579545MHz [7.15909/2]
      YM3812 clock - 3.579545MHz [7.15909/2]
      M6295 clock - 1.000MHz [16/16], Pin 7 HIGH
      VSync - 60Hz
      HSync - 15.38kHz

Good E Jong has SEI0211 instead of SEI0210, but PCB layout is otherwise identical.


Diagnostic Menu:
    Press and keep P1 Start and Reset
    You'll see cross hatch test screen, then press P1 Start again.


Secret menu hack [totmejan only] (I couldn't find official way to enter, so it's a hack):
    Mame internal debugger:
    PC=ECFFD ; 'SECRET MENU'
    Keys: BACKSPC, ENTER, Z, P1 START

    PC=E2EE2; 'TODAY: DATA' screen
    Keys: Z

    PC=ECC72; 'HMODE' screen
    TODO: find an actual way to access above, at worst find a suitable entry point -AS;

*******************************************************************************************/

#include "emu.h"

#include "sei021x_sei0220_spr.h"
#include "seibu_crtc.h"

#include "seibusound.h"

#include "cpu/nec/nec.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class goodejan_state : public driver_device, public seibu_sound_common
{
public:
	goodejan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_screen(*this, "screen")
		, m_crtc(*this, "crtc")
		, m_sc0_vram(*this, "sc0_vram")
		, m_sc1_vram(*this, "sc1_vram")
		, m_sc2_vram(*this, "sc2_vram")
		, m_sc3_vram(*this, "sc3_vram")
		, m_spriteram16(*this, "sprite_ram")
		, m_key(*this, "KEY%u", 0)
	{ }

	void totmejan(machine_config &config);
	void goodejan(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sei0210_device> m_spritegen;
	required_device<screen_device> m_screen;
	required_device<seibu_crtc_device> m_crtc;

	required_shared_ptr<uint16_t> m_sc0_vram;
	required_shared_ptr<uint16_t> m_sc1_vram;
	required_shared_ptr<uint16_t> m_sc2_vram;
	required_shared_ptr<uint16_t> m_sc3_vram;
	required_shared_ptr<uint16_t> m_spriteram16;

	required_ioport_array<5> m_key;

	tilemap_t *m_sc0_tilemap = nullptr;
	tilemap_t *m_sc1_tilemap = nullptr;
	tilemap_t *m_sc2_tilemap = nullptr;
	tilemap_t *m_sc3_tilemap = nullptr;

	uint16_t m_mux_data = 0;
	uint16_t m_seibucrtc_sc0bank = 0;
	uint16_t m_layer_en = 0;
	uint16_t m_scrollram[6]{};

	void gfxbank_w(uint16_t data);
	uint16_t mahjong_panel_r();
	void mahjong_panel_w(uint16_t data);
	void seibucrtc_sc0vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc1vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc2vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void seibucrtc_sc3vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void layer_en_w(uint16_t data);
	void layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	TILE_GET_INFO_MEMBER(seibucrtc_sc0_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc1_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc2_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc3_tile_info);

	void vblank_irq(int state);

	uint32_t pri_cb(uint8_t pri, uint8_t ext);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void common_io_map(address_map &map) ATTR_COLD;
	void goodejan_io_map(address_map &map) ATTR_COLD;
	void goodejan_map(address_map &map) ATTR_COLD;
	void totmejan_io_map(address_map &map) ATTR_COLD;
};

/*******************************
*
* Macros for the video registers
*
*******************************/

#define SEIBU_CRTC_ENABLE_SC0   (!(m_layer_en & 0x0001))
#define SEIBU_CRTC_ENABLE_SC2   (!(m_layer_en & 0x0002))
#define SEIBU_CRTC_ENABLE_SC1   (!(m_layer_en & 0x0004))
#define SEIBU_CRTC_ENABLE_SC3   (!(m_layer_en & 0x0008))
#define SEIBU_CRTC_ENABLE_SPR   (!(m_layer_en & 0x0010))

/************************************
* 0x20 - Screen 0 (BG) scroll x
************************************/
#define SEIBU_CRTC_SC0_SX   (m_scrollram[0])

/************************************
* 0x22 - Screen 0 (BG) scroll y
************************************/
#define SEIBU_CRTC_SC0_SY   (m_scrollram[1])

/************************************
* 0x24 - Screen 1 (FG) scroll x
************************************/
#define SEIBU_CRTC_SC1_SX   (m_scrollram[4])

/************************************
* 0x26 - Screen 1 (FG) scroll y
************************************/
#define SEIBU_CRTC_SC1_SY   (m_scrollram[5])

/************************************
* 0x28 - Screen 2 (MD) scroll x
************************************/
#define SEIBU_CRTC_SC2_SX   (m_scrollram[2])

/************************************
* 0x2a - Screen 2 (MD) scroll y
************************************/
#define SEIBU_CRTC_SC2_SY   (m_scrollram[3])

/*******************************
*
* Write RAM accesses
*
*******************************/

void goodejan_state::seibucrtc_sc0vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	m_sc0_tilemap->mark_tile_dirty(offset);
}

void goodejan_state::seibucrtc_sc2vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc2_vram[offset]);
	m_sc2_tilemap->mark_tile_dirty(offset);
}

void goodejan_state::seibucrtc_sc1vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc1_vram[offset]);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

void goodejan_state::seibucrtc_sc3vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	m_sc3_tilemap->mark_tile_dirty(offset);
}


/*******************************
*
* Tilemap info accesses
*
*******************************/

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc0_tile_info )
{
	u32 tile = m_sc0_vram[tile_index] & 0xfff;
	u32 color = (m_sc0_vram[tile_index] >> 12) & 0x0f;
	tile += (m_seibucrtc_sc0bank << 12);
	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc2_tile_info )
{
	u32 tile = m_sc2_vram[tile_index] & 0xfff;
	u32 color = (m_sc2_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc1_tile_info )
{
	u32 tile = m_sc1_vram[tile_index] & 0xfff;
	u32 color = (m_sc1_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc3_tile_info )
{
	u32 tile = m_sc3_vram[tile_index] & 0xfff;
	u32 color = (m_sc3_vram[tile_index] >> 12) & 0x0f;
	tileinfo.set(3, tile, color, 0);
}

uint32_t goodejan_state::pri_cb(uint8_t pri, uint8_t ext)
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

void goodejan_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goodejan_state::seibucrtc_sc0_tile_info)), TILEMAP_SCAN_ROWS, 16,16,32,32);
	m_sc2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goodejan_state::seibucrtc_sc2_tile_info)), TILEMAP_SCAN_ROWS, 16,16,32,32);
	m_sc1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goodejan_state::seibucrtc_sc1_tile_info)), TILEMAP_SCAN_ROWS, 16,16,32,32);
	m_sc3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(goodejan_state::seibucrtc_sc3_tile_info)), TILEMAP_SCAN_ROWS, 8,8,32,32);

	m_sc2_tilemap->set_transparent_pen(15);
	m_sc1_tilemap->set_transparent_pen(15);
	m_sc3_tilemap->set_transparent_pen(15);

	m_seibucrtc_sc0bank = 0;

	save_item(NAME(m_mux_data));
	save_item(NAME(m_seibucrtc_sc0bank));
	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}

uint32_t goodejan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	m_sc0_tilemap->set_scrollx(0, (SEIBU_CRTC_SC0_SX) & 0x1ff );
	m_sc0_tilemap->set_scrolly(0, (SEIBU_CRTC_SC0_SY) & 0x1ff );
	m_sc2_tilemap->set_scrollx(0, (SEIBU_CRTC_SC2_SX) & 0x1ff );
	m_sc2_tilemap->set_scrolly(0, (SEIBU_CRTC_SC2_SY) & 0x1ff );
	m_sc1_tilemap->set_scrollx(0, (SEIBU_CRTC_SC1_SX) & 0x1ff );
	m_sc1_tilemap->set_scrolly(0, (SEIBU_CRTC_SC1_SY) & 0x1ff );

	if(SEIBU_CRTC_ENABLE_SC0) { m_sc0_tilemap->draw(screen, bitmap, cliprect, 0, 1); }
	if(SEIBU_CRTC_ENABLE_SC2) { m_sc2_tilemap->draw(screen, bitmap, cliprect, 0, 2); }
	if(SEIBU_CRTC_ENABLE_SC1) { m_sc1_tilemap->draw(screen, bitmap, cliprect, 0, 4); }
	if(SEIBU_CRTC_ENABLE_SC3) { m_sc3_tilemap->draw(screen, bitmap, cliprect, 0, 8); }
	if(SEIBU_CRTC_ENABLE_SPR) { m_spritegen->draw_sprites(screen, bitmap, cliprect, m_spriteram16, m_spriteram16.bytes()); }

	return 0;
}


void goodejan_state::gfxbank_w(uint16_t data)
{
	m_seibucrtc_sc0bank = BIT(data, 8);
	m_sc0_tilemap->mark_all_dirty();
}

/* Multiplexer device for the mahjong panel */
uint16_t goodejan_state::mahjong_panel_r()
{
	u16 ret = 0xffff;

	for (int i = 0; i < 5; i++)
	{
		if (BIT(m_mux_data, i))
			ret &= m_key[i]->read();
	}

	return ret;
}

void goodejan_state::mahjong_panel_w(uint16_t data)
{
	m_mux_data = data;
}

void goodejan_state::goodejan_map(address_map &map)
{
	map(0x00000, 0x0afff).ram();
	map(0x0c000, 0x0c7ff).ram().w(FUNC(goodejan_state::seibucrtc_sc0vram_w)).share("sc0_vram");
	map(0x0c800, 0x0cfff).ram().w(FUNC(goodejan_state::seibucrtc_sc3vram_w)).share("sc3_vram");
	map(0x0d000, 0x0dfff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	/*Guess: these two aren't used/initialized at all.*/
	map(0x0e000, 0x0e7ff).ram().w(FUNC(goodejan_state::seibucrtc_sc1vram_w)).share("sc1_vram");
	map(0x0e800, 0x0efff).ram().w(FUNC(goodejan_state::seibucrtc_sc2vram_w)).share("sc2_vram");
	map(0x0f800, 0x0ffff).ram().share("sprite_ram");
	map(0xc0000, 0xfffff).rom();
}

/* totmejan CRTC is at 8000-804f,goodejan is at 8000-807f */
void goodejan_state::common_io_map(address_map &map)
{
	map(0x9000, 0x9001).w(FUNC(goodejan_state::gfxbank_w));
	map(0xb000, 0xb003).nopw();
	map(0xb004, 0xb005).w(FUNC(goodejan_state::mahjong_panel_w));

	map(0xc000, 0xc001).portr("DSW1");
	map(0xc002, 0xc003).r(FUNC(goodejan_state::mahjong_panel_r));
	map(0xc004, 0xc005).portr("DSW2"); // switches
	map(0xd000, 0xd00f).rw("seibu_sound", FUNC(seibu_sound_device::main_r), FUNC(seibu_sound_device::main_w)).umask16(0x00ff);
}

void goodejan_state::totmejan_io_map(address_map &map)
{
	common_io_map(map);
	map(0x8000, 0x804f).rw(m_crtc, FUNC(seibu_crtc_device::read), FUNC(seibu_crtc_device::write));
}

void goodejan_state::goodejan_io_map(address_map &map)
{
	common_io_map(map);
	map(0x8000, 0x807f).lrw16(
							  NAME([this](offs_t offset) {
								  return m_crtc->read(offset ^ 0x20);
							  }),
							  NAME([this](offs_t offset, u16 data) {
								  m_crtc->write(offset ^ 0x20, data);
							  }));
}

static INPUT_PORTS_START( goodejan )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/*  These games seems have 2 DIP Switches, DIPSW-A and DIPSW-B
    Game reads these switches at port C000h (16 bit) with two calls (subroutine 0EF522h [totmejan])
        Needs to be rearranged and cleaned up (DSW1 current holds all dips and DSW2 appears to be additional inputs) */

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x0001, 0x0001, "DSWA:1" )
	PORT_DIPNAME( 0x001e, 0x001e, DEF_STR( Coinage ) )            PORT_DIPLOCATION("DSWA:2,3,4,5")
	PORT_DIPSETTING(      0x001e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0016, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0012, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Credit(s) to Start" )          PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPNAME( 0x0040, 0x0040, "Cross Hatch Test" )            PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x0080, 0x0080, "DSWA:8" )
	PORT_DIPNAME( 0x0300, 0x0300, "Starting Points" )             PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(      0x0300, "1500" )
	PORT_DIPSETTING(      0x0200, "2000" )
	PORT_DIPSETTING(      0x0100, "1000" )
	PORT_DIPSETTING(      0x0000, "3000" )
/*
 *  [totmejan]
 *  Before every hand game calls a subroutine at 0xe7c19, and reads these 2 bits from 0x01A28 work RAM buffer.
 *  This affects the tile distribution RNG via a complex algo for the end goal of giving the player a more or less
 *  favorable hand depending on the setting.
 */
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )         PORT_DIPLOCATION("DSWB:3,4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Demo_Sounds ) )        PORT_DIPLOCATION("DSWB:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Explicit Scenes" )             PORT_DIPLOCATION("DSWB:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x4000, "DSWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x8000, "DSWB:8" )

	PORT_START("DSW2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xfffc, IP_ACTIVE_LOW, IPT_UNKNOWN ) // 0x0002 must be kept low to work as service coin
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 12, 8, 4, 0 },
	{
		0,  1,  2,  3,
		16, 17, 18, 19,
		512,513,514,515,
		528,529,530,531
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	32*32,
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 12, 8, 4, 0 },
	{
		0,  1,  2,  3,
		16, 17, 18, 19
	},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32  },
	8*32
};

static GFXDECODE_START( gfx_goodejan )
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x000, 0x30 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x300, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x100, 0x10 ) /* Text */
GFXDECODE_END

static GFXDECODE_START( gfx_goodejan_spr )
	GFXDECODE_ENTRY( "spr_gfx", 0, tilelayout, 0x200, 0x40 ) /* Sprites */
GFXDECODE_END

void goodejan_state::vblank_irq(int state)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x208/4); // V30
/* vector 0x00c is just a reti */
}

void goodejan_state::layer_en_w(uint16_t data)
{
	m_layer_en = data;
}

void goodejan_state::layer_scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scrollram[offset]);
}

void goodejan_state::goodejan(machine_config &config)
{
	constexpr XTAL GOODEJAN_MHZ1 = XTAL(7'159'090);
	constexpr XTAL GOODEJAN_MHZ2 = XTAL(16'000'000);
	constexpr XTAL GOODEJAN_MHZ3 = XTAL(12'000'000);

	/* basic machine hardware */
	V30(config, m_maincpu, GOODEJAN_MHZ2/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &goodejan_state::goodejan_map);
	m_maincpu->set_addrmap(AS_IO, &goodejan_state::goodejan_io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", GOODEJAN_MHZ1/2));
	audiocpu.set_addrmap(AS_PROGRAM, &goodejan_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// guess: assume ~59.61 Hz like toki, assume clock coming from the otherwise unused 12 MHz XTal
	// (audio one don't give valid ranges for the provided HSync)
	m_screen->set_raw(GOODEJAN_MHZ3/2, 390, 0, 256, 258, 16, 240);
	m_screen->set_screen_update(FUNC(goodejan_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(goodejan_state::vblank_irq));

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(goodejan_state::layer_en_w));
	m_crtc->layer_scroll_callback().set(FUNC(goodejan_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goodejan);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x800);

	SEI0211(config, m_spritegen, GOODEJAN_MHZ3, m_palette, gfx_goodejan_spr);
	m_spritegen->set_pri_callback(FUNC(goodejan_state::pri_cb));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", GOODEJAN_MHZ1/2));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", GOODEJAN_MHZ2/16, okim6295_device::PIN7_HIGH));
	oki.add_route(ALL_OUTPUTS, "mono", 0.40);

	seibu_sound_device &seibu_sound(SEIBU_SOUND(config, "seibu_sound", 0));
	seibu_sound.int_callback().set_inputline("audiocpu", 0);
	seibu_sound.set_rom_tag("audiocpu");
	seibu_sound.set_rombank_tag("seibu_bank1");
	seibu_sound.ym_read_callback().set("ymsnd", FUNC(ym3812_device::read));
	seibu_sound.ym_write_callback().set("ymsnd", FUNC(ym3812_device::write));
}

void goodejan_state::totmejan(machine_config &config)
{
	goodejan(config);
	m_maincpu->set_addrmap(AS_IO, &goodejan_state::totmejan_io_map);

	SEI0210(config.replace(), m_spritegen, XTAL(12'000'000), m_palette, gfx_goodejan_spr);
	m_spritegen->set_pri_callback(FUNC(goodejan_state::pri_cb));

}

ROM_START( totmejan )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.022",  0xc0000, 0x20000, CRC(63c3c54f) SHA1(3116b73b848a1f7391a47b994951ba1af92ba298) )
	ROM_LOAD16_BYTE( "2.023",  0xc0001, 0x20000, CRC(c0b9892f) SHA1(127f439a9e625d5a0f5e88102fed6500433cd9cc) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(8bfdb304) SHA1(454fd84eb7d9338f0b5f8de0ffae541d17b958d5) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x0000, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e-jan.078", 0x000000, 0x080000, CRC(ff9ee9d8) SHA1(5e49e9a666630ca9867ee96b9d2b8d6f503b25df) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e-jan.064", 0x000000, 0x100000, CRC(5f6185ee) SHA1(599e4a574672cd1571032e879b3032d06b70e4e2) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(61b5ae88) SHA1(16105a4e97765454079deda8eaa456d60d44e906) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(29fb6ad2) SHA1(8a9c4625472daefca7fb73a9ef3717e86c3d632f) )

	ROM_REGION( 0x80000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "e-jan.0911", 0x00000, 0x80000, CRC(a7fb93c2) SHA1(c2e1300f142032c087c96e1a785af28a6d678947) )

	ROM_REGION( 0x100, "proms", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

ROM_START( goodejan )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.022",  0xc0000, 0x20000, CRC(8555122f) SHA1(92e1ec02fb81ae972eb7492b5d226b40ca65c70d) )
	ROM_LOAD16_BYTE( "2.023",  0xc0001, 0x20000, CRC(32704d74) SHA1(9722b7f1e506a17e0fa5234e05f79333cd99a364) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(732e9eae) SHA1(d306610f08630708bbbb97d71e9ed4d7e027579a) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x0000, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e_jan2obj.078", 0x000000, 0x080000, CRC(0f892ef2) SHA1(188ae43db1c48fb6870aa45c64718e901831499b) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e_jan2scr.064", 0x000000, 0x100000, CRC(71654822) SHA1(fe2a128413999085e321e455aeebda0360d38cb8) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(f355564a) SHA1(35140cc86504d6fdaba00b520d226724bac9f546) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(5bdf7225) SHA1(a8eded9dc5be1db20cddbed1ae8c22de1674de2a) )

	ROM_REGION( 0x80000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "e-jan.911", 0x00000, 0x80000, CRC(6d2cbc35) SHA1(61f47e2a94b8877906224f46d8301a26a0b9e55f) )

	ROM_REGION( 0x100, "proms", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

ROM_START( goodejana )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "1.u022",  0xc0000, 0x20000, CRC(d496cdd1) SHA1(144a9d8850b3b62520b71efd2ed1459bd673ac92) )
	ROM_LOAD16_BYTE( "2.u023",  0xc0001, 0x20000, CRC(5eda77bb) SHA1(ac54125988f9c929207becf0dcbab72eff4f054a) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "5.1016", 0x000000, 0x08000,  CRC(732e9eae) SHA1(d306610f08630708bbbb97d71e9ed4d7e027579a) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0x0000, 0x018000, 0x08000 )

	ROM_REGION( 0x080000, "spr_gfx", 0 )
	ROM_LOAD( "e_jan2obj.078", 0x000000, 0x080000, CRC(0f892ef2) SHA1(188ae43db1c48fb6870aa45c64718e901831499b) )

	ROM_REGION( 0x100000, "bg_gfx", 0 )
	ROM_LOAD( "e_jan2scr.064", 0x000000, 0x100000, CRC(71654822) SHA1(fe2a128413999085e321e455aeebda0360d38cb8) )

	ROM_REGION( 0x20000, "md_gfx", ROMREGION_ERASEFF )
	/*Empty*/
	ROM_REGION( 0x20000, "fg_gfx", ROMREGION_ERASEFF )
	/*Empty*/

	ROM_REGION( 0x20000, "tx_gfx", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "3.063",  0x00000, 0x10000, CRC(f355564a) SHA1(35140cc86504d6fdaba00b520d226724bac9f546) )
	ROM_LOAD16_BYTE( "4.061",  0x00001, 0x10000, CRC(5bdf7225) SHA1(a8eded9dc5be1db20cddbed1ae8c22de1674de2a) )

	ROM_REGION( 0x80000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "e-jan.911", 0x00000, 0x80000, CRC(6d2cbc35) SHA1(61f47e2a94b8877906224f46d8301a26a0b9e55f) )

	ROM_REGION( 0x100, "proms", 0 ) /* not used */
	ROM_LOAD( "fmj08.083", 0x000, 0x100, CRC(9657b7ad) SHA1(e9b469c2b3534593f7fe0ea19cbbf93b55957e42) )
ROM_END

} // anonymous namespace

GAME( 1991, totmejan,  0,        totmejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Tottemo E Jong",                                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, goodejan,  0,        goodejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, goodejana, goodejan, goodejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
