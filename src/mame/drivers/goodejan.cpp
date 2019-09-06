// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************************

Seibu Mahjong games (distributed by Tecmo)

CPU: V30 D70116C-10
Sound: Z80 YM3812 M6295
OSC: 12.000MHz 16.000MHz 7.15909MHz


ToDo:
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


*******************************************************************************************/

#include "emu.h"
#include "audio/seibu.h"

#include "cpu/nec/nec.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "video/seibu_crtc.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


class goodejan_state : public driver_device, public seibu_sound_common
{
public:
	goodejan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_crtc(*this, "crtc"),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc1_vram(*this, "sc1_vram"),
		m_sc2_vram(*this, "sc2_vram"),
		m_sc3_vram(*this, "sc3_vram"),
		m_spriteram16(*this, "sprite_ram")
	{ }

	void totmejan(machine_config &config);
	void goodejan(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<seibu_crtc_device> m_crtc;

	required_shared_ptr<uint16_t> m_sc0_vram;
	required_shared_ptr<uint16_t> m_sc1_vram;
	required_shared_ptr<uint16_t> m_sc2_vram;
	required_shared_ptr<uint16_t> m_sc3_vram;
	required_shared_ptr<uint16_t> m_spriteram16;

	tilemap_t *m_sc0_tilemap;
	tilemap_t *m_sc1_tilemap;
	tilemap_t *m_sc2_tilemap;
	tilemap_t *m_sc3_tilemap;

	uint16_t m_mux_data;
	uint16_t m_seibucrtc_sc0bank;
	uint16_t m_layer_en;
	uint16_t m_scrollram[6];

	DECLARE_WRITE16_MEMBER(gfxbank_w);
	DECLARE_READ16_MEMBER(mahjong_panel_r);
	DECLARE_WRITE16_MEMBER(mahjong_panel_w);
	DECLARE_WRITE16_MEMBER(seibucrtc_sc0vram_w);
	DECLARE_WRITE16_MEMBER(seibucrtc_sc1vram_w);
	DECLARE_WRITE16_MEMBER(seibucrtc_sc2vram_w);
	DECLARE_WRITE16_MEMBER(seibucrtc_sc3vram_w);
	DECLARE_WRITE16_MEMBER(layer_en_w);
	DECLARE_WRITE16_MEMBER(layer_scroll_w);

	TILE_GET_INFO_MEMBER(seibucrtc_sc0_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc1_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc2_tile_info);
	TILE_GET_INFO_MEMBER(seibucrtc_sc3_tile_info);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void seibucrtc_sc0bank_w(uint16_t data);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_io_map(address_map &map);
	void goodejan_io_map(address_map &map);
	void goodejan_map(address_map &map);
	void totmejan_io_map(address_map &map);
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

#if 0
/*******************************
* 0x1a - Layer Dynamic Paging?
*******************************/
#define SEIBU_CRTC_DYN_PAGING   (m_seibucrtc_vregs[0x001a/2])
#define SEIBU_CRTC_SC3_PAGE_SEL (SEIBU_CRTC_DYN_PAGING & 0x0002)

/*******************************
* 0x1c - Layer Enable
*******************************/
#define SEIBU_CRTC_LAYER_EN     (m_seibucrtc_vregs[0x001c/2])
#define SEIBU_CRTC_ENABLE_SC0   (!(SEIBU_CRTC_LAYER_EN & 0x0001))
#define SEIBU_CRTC_ENABLE_SC2   (!(SEIBU_CRTC_LAYER_EN & 0x0002))
#define SEIBU_CRTC_ENABLE_SC1   (!(SEIBU_CRTC_LAYER_EN & 0x0004))
#define SEIBU_CRTC_ENABLE_SC3   (!(SEIBU_CRTC_LAYER_EN & 0x0008))
#define SEIBU_CRTC_ENABLE_SPR   (!(SEIBU_CRTC_LAYER_EN & 0x0010))

/************************************
* 0x20 - Screen 0 (BG) scroll x
************************************/
#define SEIBU_CRTC_SC0_SX   (m_seibucrtc_vregs[0x0020/2])

/************************************
* 0x22 - Screen 0 (BG) scroll y
************************************/
#define SEIBU_CRTC_SC0_SY   (m_seibucrtc_vregs[0x0022/2])

/************************************
* 0x24 - Screen 1 (FG) scroll x
************************************/
#define SEIBU_CRTC_SC1_SX   (m_seibucrtc_vregs[0x0028/2])

/************************************
* 0x26 - Screen 1 (FG) scroll y
************************************/
#define SEIBU_CRTC_SC1_SY   (m_seibucrtc_vregs[0x002a/2])

/************************************
* 0x28 - Screen 2 (MD) scroll x
************************************/
#define SEIBU_CRTC_SC2_SX   (m_seibucrtc_vregs[0x0024/2])

/************************************
* 0x2a - Screen 2 (MD) scroll y
************************************/
#define SEIBU_CRTC_SC2_SY   (m_seibucrtc_vregs[0x0026/2])

/************************************
* 0x2c - Fix screen scroll x (global)
************************************/
#define SEIBU_CRTC_FIX_SX   (m_seibucrtc_vregs[0x002c/2])

/************************************
* 0x2e - Fix screen scroll y (global)
************************************/
#define SEIBU_CRTC_FIX_SY   (m_seibucrtc_vregs[0x002e/2])

#endif

/*******************************
*
* Write RAM accesses
*
*******************************/

WRITE16_MEMBER( goodejan_state::seibucrtc_sc0vram_w )
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( goodejan_state::seibucrtc_sc2vram_w )
{
	COMBINE_DATA(&m_sc2_vram[offset]);
	m_sc2_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( goodejan_state::seibucrtc_sc1vram_w )
{
	COMBINE_DATA(&m_sc1_vram[offset]);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( goodejan_state::seibucrtc_sc3vram_w )
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	m_sc3_tilemap->mark_tile_dirty(offset);
}

void goodejan_state::seibucrtc_sc0bank_w(uint16_t data)
{
	m_seibucrtc_sc0bank = data & 1;
	m_sc0_tilemap->mark_all_dirty();
}


/*******************************
*
* Tilemap info accesses
*
*******************************/

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc0_tile_info )
{
	int tile = m_sc0_vram[tile_index] & 0xfff;
	int color = (m_sc0_vram[tile_index] >> 12) & 0x0f;
	tile+=(m_seibucrtc_sc0bank<<12);
	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc2_tile_info )
{
	int tile = m_sc2_vram[tile_index] & 0xfff;
	int color = (m_sc2_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc1_tile_info )
{
	int tile = m_sc1_vram[tile_index] & 0xfff;
	int color = (m_sc1_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(3, tile, color, 0);
}

TILE_GET_INFO_MEMBER( goodejan_state::seibucrtc_sc3_tile_info )
{
	int tile = m_sc3_vram[tile_index] & 0xfff;
	int color = (m_sc3_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(4, tile, color, 0);
}

void goodejan_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
{
	int offs,fx,fy,x,y,color,sprite;
	int dx,dy,ax,ay;

	for (offs = 0x400-4;offs >= 0;offs -= 4)
	{
		if ((m_spriteram16[offs+0]&0x8000)!=0x8000) continue;
		sprite = m_spriteram16[offs+1];
		if ((sprite>>14)!=pri) continue;
		sprite &= 0x1fff;

		y = m_spriteram16[offs+3];
		x = m_spriteram16[offs+2];

		if (x&0x8000) x=0-(0x200-(x&0x1ff));
		else x&=0x1ff;
		if (y&0x8000) y=0-(0x200-(y&0x1ff));
		else y&=0x1ff;

		color = m_spriteram16[offs+0]&0x3f;
		fx = m_spriteram16[offs+0]&0x4000;
		fy = m_spriteram16[offs+0]&0x2000;
		dy=((m_spriteram16[offs+0]&0x0380)>>7)+1;
		dx=((m_spriteram16[offs+0]&0x1c00)>>10)+1;

		for (ax=0; ax<dx; ax++)
			for (ay=0; ay<dy; ay++) {
				if (!fx)
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,x+ax*16,y+ay*16,15);
				else
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
						sprite++,
						color,fx,fy,x+(dx-1-ax)*16,y+ay*16,15);
			}
	}
}

void goodejan_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(goodejan_state::seibucrtc_sc0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc2_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(goodejan_state::seibucrtc_sc2_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc1_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(goodejan_state::seibucrtc_sc1_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc3_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(goodejan_state::seibucrtc_sc3_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

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
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	m_sc0_tilemap->set_scrollx(0, (SEIBU_CRTC_SC0_SX) & 0x1ff );
	m_sc0_tilemap->set_scrolly(0, (SEIBU_CRTC_SC0_SY) & 0x1ff );
	m_sc2_tilemap->set_scrollx(0, (SEIBU_CRTC_SC2_SX) & 0x1ff );
	m_sc2_tilemap->set_scrolly(0, (SEIBU_CRTC_SC2_SY) & 0x1ff );
	m_sc1_tilemap->set_scrollx(0, (SEIBU_CRTC_SC1_SX) & 0x1ff );
	m_sc1_tilemap->set_scrolly(0, (SEIBU_CRTC_SC1_SY) & 0x1ff );
	m_sc3_tilemap->set_scrollx(0, (0) & 0x1ff );
	m_sc3_tilemap->set_scrolly(0, (0) & 0x1ff );

	if(SEIBU_CRTC_ENABLE_SC0) { m_sc0_tilemap->draw(screen, bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(bitmap,cliprect, 2); }
	if(SEIBU_CRTC_ENABLE_SC2) { m_sc2_tilemap->draw(screen, bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(bitmap,cliprect, 1); }
	if(SEIBU_CRTC_ENABLE_SC1) { m_sc1_tilemap->draw(screen, bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(bitmap,cliprect, 0); }
	if(SEIBU_CRTC_ENABLE_SC3) { m_sc3_tilemap->draw(screen, bitmap, cliprect, 0,0); }
	if(SEIBU_CRTC_ENABLE_SPR) { draw_sprites(bitmap,cliprect, 3); }

	return 0;
}


#define GOODEJAN_MHZ1 7159090
#define GOODEJAN_MHZ2 16000000
#define GOODEJAN_MHZ3 12000000


WRITE16_MEMBER(goodejan_state::gfxbank_w)
{
	seibucrtc_sc0bank_w((data & 0x100)>>8);
}

/* Multiplexer device for the mahjong panel */
READ16_MEMBER(goodejan_state::mahjong_panel_r)
{
	uint16_t ret;
	ret = 0xffff;

	switch(m_mux_data)
	{
		case 1:    ret = ioport("KEY0")->read(); break;
		case 2:    ret = ioport("KEY1")->read(); break;
		case 4:    ret = ioport("KEY2")->read(); break;
		case 8:    ret = ioport("KEY3")->read(); break;
		case 0x10: ret = ioport("KEY4")->read(); break;
	}

	return ret;
}

WRITE16_MEMBER(goodejan_state::mahjong_panel_w)
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
	map(0x8000, 0x807f).lrw16("crtc_rw",
							  [this](address_space &space, offs_t offset, u16 mem_mask) {
								  return m_crtc->read(space, offset ^ 0x20, mem_mask);
							  },
							  [this](address_space &space, offs_t offset, u16 data, u16 mem_mask) {
								  m_crtc->write(space, offset ^ 0x20, data, mem_mask);
							  });
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

		PORT_DIPNAME( 0x0300, 0x0100, "Starting Points" )             PORT_DIPLOCATION("DSWB:1,2")
	PORT_DIPSETTING(      0x0300, "1500" )
	PORT_DIPSETTING(      0x0200, "2000" )
	PORT_DIPSETTING(      0x0100, "1000" )
	PORT_DIPSETTING(      0x0000, "3000" )

	PORT_DIPUNKNOWN_DIPLOC( 0x0400, 0x0400, "DSWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0800, 0x0800, "DSWB:4" )
/*  [totmejan] Game definitely uses these, reads these 2 bits and stores at address 01A28h as 0-1st bit;
    Sub-routine at E7C19h does some arithmetic operations depending on these.
    I cound't understand whats going on. Call performs just before dealing tiles. */

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
		PORT_BIT( 0xfffe, IP_ACTIVE_LOW, IPT_UNKNOWN ) // 0x0002 must be kept low to work as service coin
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
	GFXDECODE_ENTRY( "spr_gfx", 0,tilelayout, 0x200, 0x40 ) /* Sprites */
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x000, 0x30 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x300, 0x10 ) /* Text */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x100, 0x10 ) /* Text */
GFXDECODE_END

WRITE_LINE_MEMBER(goodejan_state::vblank_irq)
{
	if (state)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x208/4); // V30
/* vector 0x00c is just a reti */
}

WRITE16_MEMBER( goodejan_state::layer_en_w )
{
	m_layer_en = data;
}

WRITE16_MEMBER( goodejan_state::layer_scroll_w )
{
	COMBINE_DATA(&m_scrollram[offset]);
}

void goodejan_state::goodejan(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, GOODEJAN_MHZ2/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &goodejan_state::goodejan_map);
	m_maincpu->set_addrmap(AS_IO, &goodejan_state::goodejan_io_map);

	z80_device &audiocpu(Z80(config, "audiocpu", GOODEJAN_MHZ1/2));
	audiocpu.set_addrmap(AS_PROGRAM, &goodejan_state::seibu_sound_map);
	audiocpu.set_irq_acknowledge_callback("seibu_sound", FUNC(seibu_sound_device::im0_vector_cb));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1); //TODO: dynamic resolution
	screen.set_screen_update(FUNC(goodejan_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(goodejan_state::vblank_irq));

	SEIBU_CRTC(config, m_crtc, 0);
	m_crtc->layer_en_callback().set(FUNC(goodejan_state::layer_en_w));
	m_crtc->layer_scroll_callback().set(FUNC(goodejan_state::layer_scroll_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_goodejan);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_444, 0x1000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", GOODEJAN_MHZ1/2));
	ymsnd.irq_handler().set("seibu_sound", FUNC(seibu_sound_device::fm_irqhandler));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki(OKIM6295(config, "oki", GOODEJAN_MHZ2/16, okim6295_device::PIN7_LOW));
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

GAME( 1991, totmejan,  0,        totmejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Tottemo E Jong",                                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, goodejan,  0,        goodejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, goodejana, goodejan, goodejan, goodejan, goodejan_state, empty_init, ROT0, "Seibu Kaihatsu (Tecmo license)", "Good E Jong -Kachinuki Mahjong Syoukin Oh!!- (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
