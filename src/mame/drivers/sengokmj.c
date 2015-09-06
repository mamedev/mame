// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Pierpaolo Prazzoli
/******************************************************************************************

Sengoku Mahjong (c) 1991 Sigma

driver by Angelo Salese & Pierpaolo Prazzoli

Uses the same Seibu custom chips of the D-Con HW.

TODO:
- Find what the remaining video C.R.T. registers does;
- Fix sprites bugs at a start of a play;
- Check NVRAM boudaries;
- How the "SW Service Mode" (press F2 during gameplay) really works (inputs etc)? Nothing mapped works with it...

Notes:
- Some strings written in the sound rom:
  "SENGOKU-MAHJONG Z80 PROGRAM ROM VERSION 1.00 WRITTEN BY K.SAEKI" at location 0x00c0-0x00ff.
  "Copyright 1990/1991 Sigma" at location 0x770-0x789.
- To bypass the startup message, toggle "Reset" dip-switch or reset with F3.
- If the Work RAM is not hooked-up (areas $67xx),a sound sample is played.I can't understand what it says though,
  appears to japanese words for "RAM failed".
- Playing with the debugger I've found this -> http://img444.imageshack.us/img444/2980/0000ti3.png (notice the "credit" at
  the bottom). Maybe a non-BET version exists? Or there's a jumper setting?

CPU:    uPD70116C-8 (V30)
Sound:  Z80-A
        YM3812
        M6295
OSC:    14.31818MHz
        16.000MHz
Chips:  SEI0100 (YM3931, main/sub cpu interface)
        SEI0160
        SEI0200 (tilemap chip)
        SEI0210
        SEI0220 (sprite chip)


MAH1-1-1.915  samples

MAH1-2-1.013  sound prg. (ic1013:27c512)

MM01-1-1.21   main prg.
MM01-2-1.24

RS006.89      video timing?

RSSENGO0.64   chr.
RSSENGO1.68

RSSENGO2.72   chr.

*******************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "audio/seibu.h"
#include "sound/3812intf.h"
#include "video/seibu_crtc.h"
#include "machine/nvram.h"


class sengokmj_state : public driver_device
{
public:
	sengokmj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sc0_vram(*this, "sc0_vram"),
		m_sc1_vram(*this, "sc1_vram"),
		m_sc2_vram(*this, "sc2_vram"),
		m_sc3_vram(*this, "sc3_vram"),
		m_spriteram16(*this, "sprite_ram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_sc0_vram;
	required_shared_ptr<UINT16> m_sc1_vram;
	required_shared_ptr<UINT16> m_sc2_vram;
	required_shared_ptr<UINT16> m_sc3_vram;
	required_shared_ptr<UINT16> m_spriteram16;

	tilemap_t *m_sc0_tilemap;
	tilemap_t *m_sc1_tilemap;
	tilemap_t *m_sc2_tilemap;
	tilemap_t *m_sc3_tilemap;

	UINT16 m_mux_data;
	UINT8 m_hopper_io;
	UINT16 m_layer_en;
	UINT16 m_scrollram[6];

	DECLARE_READ16_MEMBER(mahjong_panel_r);
	DECLARE_WRITE16_MEMBER(mahjong_panel_w);
	DECLARE_WRITE16_MEMBER(out_w);
	DECLARE_READ16_MEMBER(system_r);
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

	INTERRUPT_GEN_MEMBER(interrupt);

	virtual void machine_start();
	virtual void video_start();

	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri);
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
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

WRITE16_MEMBER( sengokmj_state::seibucrtc_sc0vram_w )
{
	COMBINE_DATA(&m_sc0_vram[offset]);
	m_sc0_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( sengokmj_state::seibucrtc_sc2vram_w )
{
	COMBINE_DATA(&m_sc2_vram[offset]);
	m_sc2_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( sengokmj_state::seibucrtc_sc1vram_w )
{
	COMBINE_DATA(&m_sc1_vram[offset]);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

WRITE16_MEMBER( sengokmj_state::seibucrtc_sc3vram_w )
{
	COMBINE_DATA(&m_sc3_vram[offset]);
	m_sc3_tilemap->mark_tile_dirty(offset);
}

/*******************************
*
* Tilemap info accesses
*
*******************************/

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc0_tile_info )
{
	int tile = m_sc0_vram[tile_index] & 0xfff;
	int color = (m_sc0_vram[tile_index] >> 12) & 0x0f;
//  tile+=(m_seibucrtc_sc0bank<<12);
	SET_TILE_INFO_MEMBER(1, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc2_tile_info )
{
	int tile = m_sc2_vram[tile_index] & 0xfff;
	int color = (m_sc2_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(2, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc1_tile_info )
{
	int tile = m_sc1_vram[tile_index] & 0xfff;
	int color = (m_sc1_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(3, tile, color, 0);
}

TILE_GET_INFO_MEMBER( sengokmj_state::seibucrtc_sc3_tile_info )
{
	int tile = m_sc3_vram[tile_index] & 0xfff;
	int color = (m_sc3_vram[tile_index] >> 12) & 0x0f;
	SET_TILE_INFO_MEMBER(4, tile, color, 0);
}

void sengokmj_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect,int pri)
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

void sengokmj_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sengokmj_state::seibucrtc_sc0_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sengokmj_state::seibucrtc_sc2_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sengokmj_state::seibucrtc_sc1_tile_info),this),TILEMAP_SCAN_ROWS,16,16,32,32);
	m_sc3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sengokmj_state::seibucrtc_sc3_tile_info),this),TILEMAP_SCAN_ROWS,8,8,64,32);

	m_sc2_tilemap->set_transparent_pen(15);
	m_sc1_tilemap->set_transparent_pen(15);
	m_sc3_tilemap->set_transparent_pen(15);

	save_item(NAME(m_layer_en));
	save_item(NAME(m_scrollram));
}

UINT32 sengokmj_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0x7ff), cliprect); //black pen

	/* TODO: offsetted? */
	m_sc0_tilemap->set_scrollx(0, (SEIBU_CRTC_SC0_SX + 128) & 0x1ff );
	m_sc0_tilemap->set_scrolly(0, (SEIBU_CRTC_SC0_SY) & 0x1ff );
	m_sc2_tilemap->set_scrollx(0, (SEIBU_CRTC_SC2_SX + 128) & 0x1ff );
	m_sc2_tilemap->set_scrolly(0, (SEIBU_CRTC_SC2_SY) & 0x1ff );
	m_sc1_tilemap->set_scrollx(0, (SEIBU_CRTC_SC1_SX + 128) & 0x1ff );
	m_sc1_tilemap->set_scrolly(0, (SEIBU_CRTC_SC1_SY) & 0x1ff );
	m_sc3_tilemap->set_scrollx(0, (128) & 0x1ff );
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


void sengokmj_state::machine_start()
{
	save_item(NAME(m_mux_data));
	save_item(NAME(m_hopper_io));
}


/* Multiplexer device for the mahjong panel */
READ16_MEMBER(sengokmj_state::mahjong_panel_r)
{
	const char *const mpnames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "UNUSED" };
	int i;
	UINT16 res = 0xffff;

	for(i=0;i<5;i++)
	{
		if(m_mux_data & 1 << i)
			res = ioport(mpnames[i])->read();
	}

	return res;
}

WRITE16_MEMBER(sengokmj_state::mahjong_panel_w)
{
	m_mux_data = (data & 0x3f00) >> 8;

	if(data & 0xc0ff)
		logerror("Write to mux %04x\n",data);
}

WRITE16_MEMBER(sengokmj_state::out_w)
{
	/* ---- ---- ---x ---- J.P. Signal (?)*/
	/* ---- ---- ---- -x-- Coin counter (done AFTER you press start)*/
	/* ---- ---- ---- --x- Cash enable (lockout)*/
	/* ---- ---- ---- ---x Hopper 10 */
	coin_lockout_w(machine(), 0,~data & 2);
	coin_lockout_w(machine(), 1,~data & 2);
	coin_counter_w(machine(), 0,data & 4);
	m_hopper_io = ((data & 1)<<6);
//  popmessage("%02x",m_hopper_io);
}

READ16_MEMBER(sengokmj_state::system_r)
{
	return (ioport("SYSTEM")->read() & 0xffbf) | m_hopper_io;
}

static ADDRESS_MAP_START( sengokmj_map, AS_PROGRAM, 16, sengokmj_state )
	AM_RANGE(0x00000, 0x07fff) AM_RAM
	AM_RANGE(0x08000, 0x09fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0c000, 0x0c7ff) AM_RAM_WRITE(seibucrtc_sc0vram_w) AM_SHARE("sc0_vram")
	AM_RANGE(0x0c800, 0x0cfff) AM_RAM_WRITE(seibucrtc_sc1vram_w) AM_SHARE("sc1_vram")
	AM_RANGE(0x0d000, 0x0d7ff) AM_RAM_WRITE(seibucrtc_sc2vram_w) AM_SHARE("sc2_vram")
	AM_RANGE(0x0d800, 0x0e7ff) AM_RAM_WRITE(seibucrtc_sc3vram_w) AM_SHARE("sc3_vram")
	AM_RANGE(0x0e800, 0x0f7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x0f800, 0x0ffff) AM_RAM AM_SHARE("sprite_ram")
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sengokmj_io_map, AS_IO, 16, sengokmj_state )
	AM_RANGE(0x4000, 0x400f) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	/*Areas from 8000-804f are for the custom Seibu CRTC.*/
	AM_RANGE(0x8000, 0x804f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)

//  AM_RANGE(0x8080, 0x8081) CRTC extra register?
//  AM_RANGE(0x80c0, 0x80c1) CRTC extra register?
//  AM_RANGE(0x8100, 0x8101) AM_WRITENOP // always 0
	AM_RANGE(0x8180, 0x8181) AM_WRITE(out_w)
	AM_RANGE(0x8140, 0x8141) AM_WRITE(mahjong_panel_w)
	AM_RANGE(0xc000, 0xc001) AM_READ_PORT("DSW1")
	AM_RANGE(0xc002, 0xc003) AM_READ(mahjong_panel_r)
	AM_RANGE(0xc004, 0xc005) AM_READ(system_r) //switches
ADDRESS_MAP_END


static INPUT_PORTS_START( sengokmj )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Re-start" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Double G" )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Double L" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Kamon" )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW1:6" )
	PORT_DIPNAME( 0x0040, 0x0040, "Out Sw" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Hopper" ) PORT_DIPLOCATION("SW1:8") //game gives hopper error with this off.
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x0001, 0x0001, "Door" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0002, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0004, 0x0004, "Opt. 1st" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Reset" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Cash" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
//  0x40 Hopper
	PORT_DIPNAME( 0x0080, 0x0080, "Meter" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 sprites  */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0,
				3+32*16, 2+32*16, 1+32*16, 0+32*16, 16+3+32*16, 16+2+32*16, 16+1+32*16, 16+0+32*16 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16,
			16*16, 18*16, 20*16, 22*16, 24*16, 26*16, 28*16, 30*16 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 16+3, 16+2, 16+1, 16+0 },
	{ 0*16, 2*16, 4*16, 6*16, 8*16, 10*16, 12*16, 14*16 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( sengokmj )
	GFXDECODE_ENTRY( "spr_gfx",0, tilelayout, 0x000, 0x40 ) /* Sprites */
	GFXDECODE_ENTRY( "bg_gfx", 0, tilelayout, 0x400, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "md_gfx", 0, tilelayout, 0x500, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "fg_gfx", 0, tilelayout, 0x600, 0x10 ) /* Tiles */
	GFXDECODE_ENTRY( "tx_gfx", 0, charlayout, 0x700, 0x10 ) /* Text */
GFXDECODE_END

INTERRUPT_GEN_MEMBER(sengokmj_state::interrupt)
{
	device.execute().set_input_line_and_vector(0,HOLD_LINE,0xc8/4);
}

WRITE16_MEMBER( sengokmj_state::layer_en_w )
{
	m_layer_en = data;
}

WRITE16_MEMBER( sengokmj_state::layer_scroll_w )
{
	COMBINE_DATA(&m_scrollram[offset]);
}


static MACHINE_CONFIG_START( sengokmj, sengokmj_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 16000000/2) /* V30-8 */
	MCFG_CPU_PROGRAM_MAP(sengokmj_map)
	MCFG_CPU_IO_MAP(sengokmj_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sengokmj_state,  interrupt)

	SEIBU_SOUND_SYSTEM_CPU(14318180/4)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1) //TODO: dynamic resolution
	MCFG_SCREEN_UPDATE_DRIVER(sengokmj_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(sengokmj_state, layer_en_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(sengokmj_state, layer_scroll_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sengokmj)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END


ROM_START( sengokmj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "mm01-1-1.21",  0xc0000, 0x20000, CRC(74076b46) SHA1(64b0ed5a8c32e21157ae12fe40519e4c605b329c) )
	ROM_LOAD16_BYTE( "mm01-2-1.24",  0xc0001, 0x20000, CRC(f1a7c131) SHA1(d0fbbdedbff8f05da0e0296baa41369bc41a67e4) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* 64k code for sound Z80 */
	ROM_LOAD( "mah1-2-1.013", 0x000000, 0x08000, CRC(6a4f31b8) SHA1(5e1d7ed299c1fd65c7a43faa02831220f4251733) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", 0,     0x018000, 0x08000 )

	ROM_REGION( 0x100000, "spr_gfx", 0 ) /*Sprites gfx rom*/
	ROM_LOAD( "rssengo2.72", 0x00000, 0x100000, CRC(fb215ff8) SHA1(f98c0a53ad9b97d209dd1f85c994fc17ec585bd7) )

	ROM_REGION( 0x200000, "gfx_tiles", 0 ) /*Tiles data,to be reloaded*/
	ROM_LOAD( "rssengo0.64", 0x000000, 0x100000, CRC(36924b71) SHA1(814b2c69ab9876ccc57774e5718c05059ea23150) )
	ROM_LOAD( "rssengo1.68", 0x100000, 0x100000, CRC(1bbd00e5) SHA1(86391323b8e0d3b7e09a5914d87fb2adc48e5af4) )

	ROM_REGION( 0x080000, "bg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x000000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "md_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x080000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "fg_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x100000, 0x00000, 0x080000)

	ROM_REGION( 0x080000, "tx_gfx", 0 )
	ROM_COPY( "gfx_tiles" , 0x180000, 0x00000, 0x080000)

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "mah1-1-1.915", 0x00000, 0x20000, CRC(d4612e95) SHA1(937c5dbd25c89d4f4178b0bed510307020c5f40e) )

	ROM_REGION( 0x200, "user1", 0 ) /* not used */
	ROM_LOAD( "rs006.89", 0x000, 0x200, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */
ROM_END

GAME( 1991, sengokmj, 0, sengokmj, sengokmj, driver_device, 0, ROT0, "Sigma", "Sengoku Mahjong [BET] (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
/*Non-Bet Version?*/
