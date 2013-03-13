/*******************************************************************************************

    Popo Bear (c) 2000 BMC

    preliminary driver by Angelo Salese

    TODO:
    - auto-animation speed is erratic (way too fast);
    - tilemap effects (scrolling, colscroll, linescroll);
	  (high score table?)
	- BMC logo isn't copied correctly after an attract loop, RAM not cleared on a reset either?
    - BGM seems quite off, YM2413 core bug?
    - I/Os;
    - IRQ generation;
    - Port 0x620000 is quite a mystery, some silly protection?

============================================================================================
Popo Bear - BMC-A00211
(c) 2000 - Bao Ma Technology Co., LTD

|-----------------------------------------|
| DIP2 DIP4  UM3567(YM2413)               |J
| DIP1 DIP3                               |A
|           TA-A-901                      |M
| EN-A-701  EN-A-801  U6295(OKI)          |M
| EN-A-501  EN-A-601                      |A
| EN-A-301  EN-A-401                      |
|                                         |C
|                   AIA90610              |O
|                   BMC-68pin  AIA90423   |N
|                   plcc (68k) BMC-160pin |N
|                                         |E
|                                    OSC  |C
|                                 42.000  |T
|-----------------------------------------|

1 - BMC AIA90423 - 160-Pin ASIC, FGPA, Video?
1 - BMC AIA90610 - 68 Pin CPU (Likely 16 MHz, 68-lead plastic LCC 68000)
1 - UM3567 (YM2413) Sound
1 - U6295 (OKI6295) Sound
1 - 42.000MHz XTAL
4 - 8 Position DIP switches

JAMMA CONNECTOR
Component Side   A   B   Solder Side
           GND   1   1   GND
           GND   2   2   GND
           +5v   3   3   +5v
           +5v   4   4   +5v
                 5   5
          +12v   6   6   +12v
                 7   7
    Coin Meter   8   8
                 9   9
       Speaker  10   10  GND
                11   11
           Red  12   12  Green
          Blue  13   13  Syn
           GND  14   14
          Test  15   15
         Coin1  16   16  Coin2
      1P Start  17   17  2P Start
         1P Up  18   18  2P Up
       1P Down  19   19  2P Down
       1P Left  20   20  2P Left
      1P Right  21   21  2P Right
          1P A  22   22  2P A
          1P B  23   23  2P B
          1P C  24   24  2P C
                25   25
                26   26
           GND  27   27  GND
           GND  28   28  GND
*******************************************************************************************/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

class popobear_state : public driver_device
{
public:
	popobear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spr(*this, "spr"),
		m_vregs(*this, "vregs")
	
	{ 
		tilemap_base[0] = 0xf0000;
		tilemap_base[1] = 0xf4000;
		tilemap_base[2] = 0xf8000;
		tilemap_base[3] = 0xfc000;

		tilemap_size[0] = 0x04000;
		tilemap_size[1] = 0x04000;
		tilemap_size[2] = 0x04000;
		tilemap_size[3] = 0x04000;	
	}

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_spr;
	required_shared_ptr<UINT16> m_vregs;

	UINT16* m_vram;
	UINT16* m_vram_rearranged;

	int tilemap_base[4];
	int tilemap_size[4];

	DECLARE_READ8_MEMBER(popo_620000_r);
	DECLARE_WRITE8_MEMBER(popobear_irq_ack_w);
	virtual void video_start();
	UINT32 screen_update_popobear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(popobear_irq);
	void draw_layer(bitmap_ind16 &bitmap,const rectangle &cliprect, UINT8 layer_n);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	int m_gfx_index;
	tilemap_t    *m_bg_tilemap[4];
	TILE_GET_INFO_MEMBER(get_popobear_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_popobear_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_popobear_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_popobear_bg3_tile_info);

	DECLARE_WRITE16_MEMBER(popo_vram_w)
	{
		COMBINE_DATA(&m_vram[offset]);

		// the graphic data for the tiles is in a strange order, rearrange it so that we can use it as tiles..
		int swapped_offset = BITSWAP32(offset, /* unused bits */ 31,30,29,28,27,26,25,24,23,22,21,20,19, /* end unused bits */
	
		18,17,16,15,14,13,12,
		
		8,7,6,5,4,3,2,

		11,10,9, /* y tile address bits */
		
		1,0 /* x tile address bits */);



		COMBINE_DATA(&m_vram_rearranged[swapped_offset]);
		machine().gfx[m_gfx_index]->mark_dirty((swapped_offset)/32);

		// unfortunately tilemaps and tilegfx share the same ram so we're always dirty if we write to RAM
		m_bg_tilemap[0]->mark_all_dirty();
		m_bg_tilemap[1]->mark_all_dirty();
		m_bg_tilemap[2]->mark_all_dirty();
		m_bg_tilemap[3]->mark_all_dirty();

	}
	DECLARE_READ16_MEMBER(popo_vram_r) { return m_vram[offset]; }

};


static const gfx_layout popobear_char_layout =
{
	8,8,
	0x4000,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 8,0,24,16,40,32,56,48 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};


TILE_GET_INFO_MEMBER(popobear_state::get_popobear_bg0_tile_info)
{
	int base = tilemap_base[0];
	int tileno = m_vram[base/2 + tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER(popobear_state::get_popobear_bg1_tile_info)
{
	int base = tilemap_base[1];
	int tileno = m_vram[base/2 + tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER(popobear_state::get_popobear_bg2_tile_info)
{
	int base = tilemap_base[2];
	int tileno = m_vram[base/2 + tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}

TILE_GET_INFO_MEMBER(popobear_state::get_popobear_bg3_tile_info)
{
	int base = tilemap_base[3];
	int tileno = m_vram[base/2 + tile_index];
	SET_TILE_INFO_MEMBER(0, tileno, 0, 0);
}




void popobear_state::video_start()
{
	/* find first empty slot to decode gfx */
	for (m_gfx_index = 0; m_gfx_index < MAX_GFX_ELEMENTS; m_gfx_index++)
		if (machine().gfx[m_gfx_index] == 0)
			break;

	assert(m_gfx_index != MAX_GFX_ELEMENTS);

	m_vram = auto_alloc_array_clear(machine(), UINT16, 0x100000/2);
	m_vram_rearranged = auto_alloc_array_clear(machine(), UINT16, 0x100000/2);


	/* create the char set (gfx will then be updated dynamically from RAM) */
	machine().gfx[m_gfx_index] = auto_alloc(machine(), gfx_element(machine(), popobear_char_layout, (UINT8 *)m_vram_rearranged, machine().total_colors() / 16, 0));

	m_bg_tilemap[0] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(popobear_state::get_popobear_bg0_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[1] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(popobear_state::get_popobear_bg1_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[2] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(popobear_state::get_popobear_bg2_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_bg_tilemap[3] = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(popobear_state::get_popobear_bg3_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_bg_tilemap[0]->set_transparent_pen(0);
	m_bg_tilemap[1]->set_transparent_pen(0);
	m_bg_tilemap[2]->set_transparent_pen(0);
	m_bg_tilemap[3]->set_transparent_pen(0);

}


void popobear_state::draw_layer(bitmap_ind16 &bitmap,const rectangle &cliprect, UINT8 layer_n)
{
	UINT16* vreg = (UINT16 *)m_vregs;
	const UINT8 vreg_base[] = { 0x10/2, 0x14/2 };
	int xscroll,yscroll;

	if(layer_n & 2)
	{
		xscroll = vreg[vreg_base[(layer_n & 1) ^ 1]+2/2] & 0x1ff;
		yscroll = vreg[vreg_base[(layer_n & 1) ^ 1]+0/2] & 0x1ff;
	}
	else
	{
		xscroll = 0;
		yscroll = 0;
	}
	popmessage("%04x %04x",vreg[vreg_base[0]+0/2],vreg[vreg_base[1]+0/2]);


	m_bg_tilemap[layer_n]->set_scrolly(0, yscroll);
	m_bg_tilemap[layer_n]->set_scrollx(0, xscroll);



	m_bg_tilemap[layer_n]->draw(bitmap, cliprect, 0, 0);

}


void popobear_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	// ERROR: This cast is NOT endian-safe without the use of BYTE/WORD/DWORD_XOR_* macros!
	UINT8* vram = reinterpret_cast<UINT8 *>(m_spr.target());
	int i;

	/*
	???? ---- ---- ---- unused?
	---- xxxx ---- ---- priority?
	---- ---- x--- ---- Y direction
	---- ---- -x-- ---- X direction
	---- ---- --xx ---- size (height & width)
	---- ---- ---- xx-- color bank
	---- ---- ---- --xx ??
	*/

	/* 0x106 = 8 x 8 */
	/* 0x*29 = 32 x 32 */
	for(i = 0x800-8;i >= 0; i-=8)
	{
		int y = vram[i+0x7f800+2]|(vram[i+0x7f800+3]<<8);
		int x = vram[i+0x7f800+4]|(vram[i+0x7f800+5]<<8);
		int spr_num = vram[i+0x7f800+6]|(vram[i+0x7f800+7]<<8);
		int param = vram[i+0x7f800+0]|(vram[i+0x7f800+1]<<8);
		int width = 8 << ((param & 0x30)>>4);
		int height = width; // sprites are always square?

		int color_bank = ((param & 0xc)<<4);
		int x_dir = param & 0x40;
		int y_dir = param & 0x80;

		if (x&0x8000) x-= 0x10000;
		if (y&0x8000) y-= 0x10000;

		if(param == 0)
			continue;

		spr_num <<= 3;

		for(int yi=0;yi<height;yi++)
		{
			int y_draw = (y_dir) ? y+(height - yi) : y+yi;

			for(int xi=0;xi<width;xi++)
			{
				UINT8 pix = (vram[spr_num^1] & 0xff);
				int x_draw = (x_dir) ? x+(width - xi) : x+xi;

				if(cliprect.contains(x_draw, y_draw) && pix)
					bitmap.pix16(y_draw, x_draw) = machine().pens[pix+0x100+color_bank];

				spr_num++;
			}
		}
	}
}

UINT32 popobear_state::screen_update_popobear(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	//popmessage("%04x",m_vregs[0/2]);

	draw_layer(bitmap,cliprect,3);
	draw_layer(bitmap,cliprect,2);
	draw_layer(bitmap,cliprect,1);
	draw_layer(bitmap,cliprect,0);
	draw_sprites(bitmap,cliprect);

	return 0;
}

/* ??? */
READ8_MEMBER(popobear_state::popo_620000_r)
{
	return 9;
}

WRITE8_MEMBER(popobear_state::popobear_irq_ack_w)
{
	int i;

	for(i=0;i<8;i++)
	{
		if(data & 1 << i)
			m_maincpu->set_input_line(i, CLEAR_LINE);
	}
}

static ADDRESS_MAP_START( popobear_mem, AS_PROGRAM, 16, popobear_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x210000, 0x21ffff) AM_RAM
	AM_RANGE(0x280000, 0x2fffff) AM_RAM AM_SHARE("spr") // unknown boundaries, 0x2ff800 contains a sprite list, lower area = sprite gfx
	AM_RANGE(0x300000, 0x3fffff) AM_READWRITE( popo_vram_r, popo_vram_w ) // tile definitions + tilemaps
	          

	/* Most if not all of these are vregs */
	AM_RANGE(0x480000, 0x48001f) AM_RAM AM_SHARE("vregs")
	AM_RANGE(0x480020, 0x480023) AM_RAM
	AM_RANGE(0x480028, 0x48002d) AM_RAM
//  AM_RANGE(0x480020, 0x480021) AM_NOP //AM_READ_LEGACY(popo_480020_r) AM_WRITE_LEGACY(popo_480020_w)
//  AM_RANGE(0x480028, 0x480029) AM_NOP //AM_WRITE_LEGACY(popo_480028_w)
//  AM_RANGE(0x48002c, 0x48002d) AM_NOP //AM_WRITE_LEGACY(popo_48002c_w)
	AM_RANGE(0x480030, 0x480031) AM_WRITE8(popobear_irq_ack_w, 0x00ff)
	AM_RANGE(0x480034, 0x480035) AM_RAM // coin counter or coin lockout
	AM_RANGE(0x48003a, 0x48003b) AM_RAM //AM_READ_LEGACY(popo_48003a_r) AM_WRITE_LEGACY(popo_48003a_w)

	AM_RANGE(0x480400, 0x4807ff) AM_RAM AM_WRITE(paletteram_xBBBBBGGGGGRRRRR_word_w) AM_SHARE("paletteram")

	AM_RANGE(0x500000, 0x500001) AM_READ_PORT("IN0")
	AM_RANGE(0x520000, 0x520001) AM_READ_PORT("IN1")
	AM_RANGE(0x540000, 0x540001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x550000, 0x550003) AM_DEVWRITE8_LEGACY("ymsnd", ym2413_w, 0x00ff )

	AM_RANGE(0x600000, 0x600001) AM_WRITENOP
	AM_RANGE(0x620000, 0x620001) AM_READ8(popo_620000_r,0xff00) AM_WRITENOP
	AM_RANGE(0x800000, 0xbfffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( popobear )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x00, "Coin_A" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, "Freeplay" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hard ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Arrow" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2:4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2:5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(popobear_state::popobear_irq)
{
	int scanline = param;

	/* Order is trusted (5 as vblank-out makes the title screen logo spinning to behave wrongly) */
	if(scanline == 240)
		m_maincpu->set_input_line(3, ASSERT_LINE);

	if(scanline == 0)
		m_maincpu->set_input_line(5, ASSERT_LINE);

	/* TODO: actually a timer irq, tied with YM2413 sound chip (controls BGM tempo) */
	if(scanline == 64 || scanline == 192)
		m_maincpu->set_input_line(2, ASSERT_LINE);
}

static MACHINE_CONFIG_START( popobear, popobear_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_42MHz/4)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_CPU_PROGRAM_MAP(popobear_mem)
	// levels 2,3,5 look interesting
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", popobear_state, irq5_line_assert)
	//MCFG_CPU_PERIODIC_INT_DRIVER(popobear_state, irq2_line_assert, 120)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", popobear_state, popobear_irq, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(popobear_state, screen_update_popobear)

	MCFG_SCREEN_SIZE(128*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 479, 0, 239)
	MCFG_PALETTE_LENGTH(256*2)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_42MHz/16)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_42MHz/32, OKIM6295_PIN7_LOW)  // XTAL CORRECT, DIVISOR GUESSED
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( popobear )
	ROM_REGION( 0x1000000, "maincpu", 0 ) /* 68000 Code + gfx data */
	ROM_LOAD16_BYTE( "popobear_en-a-301_1.6.u3", 0x000001, 0x020000, CRC(b934adf6) SHA1(93431c7a19af812b549aad35cc1176a81805ffab) )
	ROM_LOAD16_BYTE( "popobear_en-a-401_1.6.u4", 0x000000, 0x020000, CRC(0568af9c) SHA1(920531dbc4bbde2d1db062bd5c48b97dd50b7185) )
	ROM_LOAD16_BYTE( "popobear_en-a-501.u5",     0x800001, 0x100000, CRC(185901a9) SHA1(7ff82b5751645df53435eaa66edce589684cc5c7) )
	ROM_LOAD16_BYTE( "popobear_en-a-601.u6",     0x800000, 0x100000, CRC(84fa9f3f) SHA1(34dd7873f88b0dae5fb81fe84e82d2b6b49f7332) )
	ROM_LOAD16_BYTE( "popobear_en-a-701.u7",     0xa00001, 0x100000, CRC(45eba6d0) SHA1(0278602ed57ac45040619d590e6cc85e2cfeed31) )
	ROM_LOAD16_BYTE( "popobear_en-a-801.u8",     0xa00000, 0x100000, CRC(2760f2e6) SHA1(58af59f486c9df930f7c124f89154f8f389a5bd7) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "popobear_ta-a-901.u9", 0x00000, 0x40000,  CRC(f1e94926) SHA1(f4d6f5b5811d90d0069f6efbb44d725ff0d07e1c) )
ROM_END

GAME( 2000, popobear,    0, popobear,    popobear, driver_device,    0, ROT0,  "BMC", "PoPo Bear", GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
