// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Andrew Gardner, Roberto Fresca, Luca Elia
/*******************************************************************************************

King Derby (c) 1981 Tatsumi

driver by Andrew Gardner, Angelo Salese & Roberto Fresca
original cowrace.c by Luca Elia

TODO:
- remaining video issues, priorities, sprites etc.;
- inputs;
- Understand how tilemap color offsets really works;
- Discrete sound part? There's a Rossini's "William Tell" bgm on the Chinese bootlegs,
  I think it's tied with ay8910 port B. Update: the 1986 version sounds that just fine, could be
  that this is a btanb;
- unknown memories;
- Garbage on the window tilemap if you win, it could be a btanb (masked by the color prom).
- the name "King Derby" is a raw guess, there's a chance that it uses a different name
  (but there isn't any title screen in the game?)
- Fix I/O in the 1986 bootleg version;

MC6845 interface:
  screen size:  384x272    registers 00 & 04. (value-1)
  visible area: 256x224    registers 01 & 06.

  the clocks are a guess, but is the only logical combination I found to get a reasonable vertical of ~53Hz.

============================================================================================

file   : readme.txt
author : Stefan Lindberg
created: 2009-01-03
updated: *
version: 1.0


Unknown Tazmi game, 1981?


Note:
Untested PCB.
A bet/gamble game i presume, possible "King Derby".
The PCB is marked 1981.

See included PCB pics.



Roms:

Name           Size     CRC32           Chip Type
----------------------------------------------------------------------------
im1_yk.g1      4096     0x1921605d      D2732D
im2_yk.f1      4096     0x8504314e      M5L2732K
im3_yk.e1      4096     0xb034314e      M5L2732K
im4_d.d6       4096     0x20f2d999      M5L2732K
im5_d.c6       4096     0xc192cecc      D2732D
im6_d.b6       4096     0x257f4e0d      D2732D
s1.d1          4096     0x26974007      D2732D
s10_a.l8       4096     0x37b2736f      D2732D
s2.e1          4096     0xbedebfa7      D2732D
s3.f1          4096     0x0aa59571      D2732D
s4.g1          4096     0xccd5fb0e      D2732D
s5.d2          4096     0x32613df3      D2732D
s6.e2          4096     0xa151c422      D2732D
s7.f2          4096     0x7cfcee55      D2732D
s8.g2          4096     0xad667c05      D2732D
s9_a.ka        4096     0xca82cd81      D2732D
sg1_b.e1       4096     0x92ef3c13      D2732D

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2203intf.h"
#include "machine/nvram.h"
#include "kingdrby.lh"


class kingdrby_state : public driver_device
{
public:
	kingdrby_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_attr(*this, "attr"),
		m_spriteram(*this, "spriteram"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_sound_cmd;
	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_attr;
	tilemap_t *m_sc0_tilemap;
	tilemap_t *m_sc0w_tilemap;
	tilemap_t *m_sc1_tilemap;
	UINT8 m_p1_hopper;
	UINT8 m_p2_hopper;
	UINT8 m_mux_data;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(sc0_vram_w);
	DECLARE_WRITE8_MEMBER(sc0_attr_w);
	DECLARE_WRITE8_MEMBER(led_array_w);
	DECLARE_WRITE8_MEMBER(kingdrbb_lamps_w);
	DECLARE_READ8_MEMBER(hopper_io_r);
	DECLARE_WRITE8_MEMBER(hopper_io_w);
	DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(outport2_w);
	DECLARE_READ8_MEMBER(input_mux_r);
	DECLARE_READ8_MEMBER(key_matrix_r);
	DECLARE_READ8_MEMBER(sound_cmd_r);
	DECLARE_WRITE8_MEMBER(outportb_w);
	TILE_GET_INFO_MEMBER(get_sc0_tile_info);
	TILE_GET_INFO_MEMBER(get_sc1_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(kingdrby);
	DECLARE_PALETTE_INIT(kingdrbb);
	UINT32 screen_update_kingdrby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#define CLK_1   XTAL_20MHz
#define CLK_2   XTAL_3_579545MHz


/*************************************
 *
 *  Video Hardware
 *
 *************************************/


/*
tile
xxxx xxxx tile number
attr
xxxx ---- basic color?
---- x--- priority
---- -xx- extra color bank?
---- ---x tile bank
*/

TILE_GET_INFO_MEMBER(kingdrby_state::get_sc0_tile_info)
{
	int tile = m_vram[tile_index] | m_attr[tile_index]<<8;
	int color = (m_attr[tile_index] & 0x06)>>1;

	tile&=0x1ff;

	SET_TILE_INFO_MEMBER(1,
			tile,
			color|0x40,
			0);
}

TILE_GET_INFO_MEMBER(kingdrby_state::get_sc1_tile_info)
{
	int tile = m_vram[tile_index] | m_attr[tile_index]<<8;
	int color = (m_attr[tile_index] & 0x06)>>1;

	tile&=0x1ff;
	//original 0xc
	//0x13
	//

	SET_TILE_INFO_MEMBER(1,
			tile,
			color|0x40,
			0);

	tileinfo.category = (m_attr[tile_index] & 0x08)>>3;
}

void kingdrby_state::video_start()
{
	m_sc0_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingdrby_state::get_sc0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,24);
	m_sc1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingdrby_state::get_sc1_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,24);
	m_sc0w_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(kingdrby_state::get_sc0_tile_info),this),TILEMAP_SCAN_ROWS,8,8,32,32);

	m_sc1_tilemap->set_transparent_pen(0);
}

static const UINT8 hw_sprite[16] =
{
	0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x22,
	0x22, 0x22, 0x22, 0x22, 0x22, 0x11, 0x22, 0x22
};

void kingdrby_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 *spriteram = m_spriteram;
	int count = 0;

	/*sprites not fully understood.*/
	for(count=0;count<0x48;count+=4)
	{
		int x,y,spr_offs,colour,fx,dx,dy,h,w;

		spr_offs = (spriteram[count]);
		spr_offs &=0x7f;
		spr_offs*=4;
		colour = (spriteram[count+3] & 0xf0)>>4;
		fx = spriteram[count] & 0x80;
		y = (spriteram[count+1] == 0) ? 0 : 0x100-spriteram[count+1];
		x = spriteram[count+2] - ((spriteram[count+3] & 1)<<8);

		/* TODO: hardcoded via a table, there must be some other way to do this */
		h = (hw_sprite[colour] & 0xf0) >> 4;
		w = (hw_sprite[colour] & 0x0f) >> 0;

		if(h == 1 && w == 1)
		{
			spr_offs /= 4;
			/* TODO: horse number signs */
			spr_offs  = 0x16c + (((spr_offs & 8) << 2) ^ 0x20) + ((spr_offs & 4) << 2) + (spr_offs & 3);
		}

		if(fx)
		{
			for(dy=0;dy<h;dy++)
				for(dx=0;dx<w;dx++)
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,spr_offs++,colour,1,0,((x+16*w)-(dx+1)*16),(y+dy*16),0);
		}
		else
		{
			for(dy=0;dy<h;dy++)
				for(dx=0;dx<w;dx++)
					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,spr_offs++,colour,0,0,(x+dx*16),(y+dy*16),0);
		}
	}
}

UINT32 kingdrby_state::screen_update_kingdrby(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();
	rectangle clip;
	m_sc0_tilemap->set_scrollx(0, m_vram[0x342]);
	m_sc0_tilemap->set_scrolly(0, m_vram[0x341]);
	m_sc1_tilemap->set_scrollx(0, m_vram[0x342]);
	m_sc1_tilemap->set_scrolly(0, m_vram[0x341]);
	m_sc0w_tilemap->set_scrolly(0, 32);

	/* maybe it needs two window tilemaps? (one at the top, the other at the bottom)*/
	clip.set(visarea.min_x, 256, 192, visarea.max_y);

	/*TILEMAP_DRAW_CATEGORY + TILEMAP_DRAW_OPAQUE doesn't suit well?*/
	m_sc0_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	m_sc1_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_CATEGORY(1),0);
	m_sc0w_tilemap->draw(screen, bitmap, clip, 0,0);

	return 0;
}

WRITE8_MEMBER(kingdrby_state::sc0_vram_w)
{
	m_vram[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
	m_sc0w_tilemap->mark_tile_dirty(offset);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(kingdrby_state::sc0_attr_w)
{
	m_attr[offset] = data;
	m_sc0_tilemap->mark_tile_dirty(offset);
	m_sc0w_tilemap->mark_tile_dirty(offset);
	m_sc1_tilemap->mark_tile_dirty(offset);
}

/*************************************
 *
 *  I/O
 *
 *************************************/

/* hopper I/O */

READ8_MEMBER(kingdrby_state::hopper_io_r)
{
	return (ioport("HPIO")->read() & 0x3f) | m_p1_hopper | m_p2_hopper;
}

WRITE8_MEMBER(kingdrby_state::hopper_io_w)
{
	m_p1_hopper = (data & 0x8)<<3;
	m_p2_hopper = (data & 0x4)<<5;
//  printf("%02x\n",data);
}

WRITE8_MEMBER(kingdrby_state::sound_cmd_w)
{
	m_soundcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	m_sound_cmd = data;
	/* soundlatch is unneeded since we are already using perfect interleave. */
	// soundlatch_byte_w(space,0, data);
}


/* No idea about what's this (if it's really a mux etc.)*/
WRITE8_MEMBER(kingdrby_state::outport2_w)
{
//  popmessage("PPI1 port C(upper) out: %02X", data);
	m_mux_data = data & 0x80;
}

READ8_MEMBER(kingdrby_state::input_mux_r)
{
	if(m_mux_data & 0x80)
		return ioport("MUX0")->read();
	else
		return ioport("MUX1")->read();
}

READ8_MEMBER(kingdrby_state::key_matrix_r)
{
	UINT16 p1_val,p2_val;
	UINT8 p1_res,p2_res;

	p1_val = ioport("KEY_1P")->read();
	p2_val = ioport("KEY_2P")->read();

	p1_res = 0;
	p2_res = 0;

	switch(p1_val)
	{
		case 0x0001: p1_res = 0x01; break;
		case 0x0002: p1_res = 0x02; break;
		case 0x0004: p1_res = 0x03; break;
		case 0x0008: p1_res = 0x04; break;
		case 0x0010: p1_res = 0x05; break;
		case 0x0020: p1_res = 0x06; break;
		case 0x0040: p1_res = 0x07; break;
		case 0x0080: p1_res = 0x08; break;
		case 0x0100: p1_res = 0x09; break;
		case 0x0200: p1_res = 0x0a; break;
		case 0x0400: p1_res = 0x0b; break;
		case 0x0800: p1_res = 0x0c; break;
		case 0x1000: p1_res = 0x0d; break;
		case 0x2000: p1_res = 0x0e; break;
		case 0x4000: p1_res = 0x0f; break;
	}

	switch(p2_val)
	{
		case 0x0001: p2_res = 0x01; break;
		case 0x0002: p2_res = 0x02; break;
		case 0x0004: p2_res = 0x03; break;
		case 0x0008: p2_res = 0x04; break;
		case 0x0010: p2_res = 0x05; break;
		case 0x0020: p2_res = 0x06; break;
		case 0x0040: p2_res = 0x07; break;
		case 0x0080: p2_res = 0x08; break;
		case 0x0100: p2_res = 0x09; break;
		case 0x0200: p2_res = 0x0a; break;
		case 0x0400: p2_res = 0x0b; break;
		case 0x0800: p2_res = 0x0c; break;
		case 0x1000: p2_res = 0x0d; break;
		case 0x2000: p2_res = 0x0e; break;
		case 0x4000: p2_res = 0x0f; break;
	}

	return p1_res | (p2_res<<4);
}

READ8_MEMBER(kingdrby_state::sound_cmd_r)
{
	return m_sound_cmd;
}

static const UINT8 led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };

WRITE8_MEMBER(kingdrby_state::led_array_w)
{
	/*
	offset = directly tied with the button (i.e. offset 1 = 1-2, offset 2 = 1-3 etc.)
	data = xxxx ---- p2 array
	       ---- xxxx p1 array
	they goes from 0 to 5, to indicate the number.
	If one player bets something, the other led will toggle between p1 and p2 bets.
	*/
	output().set_digit_value(0xf + offset, led_map[(data & 0xf0) >> 4]);
	output().set_digit_value(0x0 + offset, led_map[(data & 0x0f) >> 0]);

}

/*************************************
 *
 * Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, kingdrby_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM AM_MIRROR(0xc00) AM_SHARE("share1")
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(sc0_vram_w) AM_SHARE("vram")
	AM_RANGE(0x5000, 0x53ff) AM_RAM_WRITE(sc0_attr_w) AM_SHARE("attr")
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io_map, AS_IO, 8, kingdrby_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_NOP //interrupt ack
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 8, kingdrby_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_ROM //sound rom, tested for the post check
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_SHARE("nvram") //backup ram
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)    /* I/O Ports */
	AM_RANGE(0x6000, 0x6003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)    /* I/O Ports */
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x7400, 0x74ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x7600, 0x7600) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x7601, 0x7601) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x7801, 0x780f) AM_WRITE(led_array_w)
	AM_RANGE(0x7a00, 0x7a00) AM_RAM //buffer for the key matrix
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("DSW")
ADDRESS_MAP_END

WRITE8_MEMBER(kingdrby_state::kingdrbb_lamps_w)
{
	// (same as the inputs but active high)
}

static ADDRESS_MAP_START( slave_1986_map, AS_PROGRAM, 8, kingdrby_state )
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_ROM //sound rom tested for the post check
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_SHARE("nvram") //backup ram
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)    /* I/O Ports */
//  AM_RANGE(0x6000, 0x6003) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write) /* I/O Ports */
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x7400, 0x74ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x7600, 0x7600) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x7601, 0x7601) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x7800, 0x7800) AM_READ_PORT("KEY0")
	AM_RANGE(0x7801, 0x7801) AM_READ_PORT("KEY1")
	AM_RANGE(0x7802, 0x7802) AM_READ_PORT("KEY2")
	AM_RANGE(0x7803, 0x7803) AM_READ_PORT("KEY3")
	AM_RANGE(0x7800, 0x7803) AM_WRITE(kingdrbb_lamps_w)
	AM_RANGE(0x7a00, 0x7a00) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("DSW")
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_io_map, AS_IO, 8, kingdrby_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_NOP //interrupt ack
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, kingdrby_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, kingdrby_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cowrace_sound_map, AS_PROGRAM, 8, kingdrby_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cowrace_sound_io, AS_IO, 8, kingdrby_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ym2203_device, write)
ADDRESS_MAP_END


WRITE8_MEMBER(kingdrby_state::outportb_w)
{
	//  printf("%02x B\n",data);
}

/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( kingdrby )
	/*this might be different.*/
	PORT_START("HPIO")  // ppi0 (5000)
	PORT_DIPNAME( 0x01, 0x01, "HPIO" )
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) //1p hopper i/o
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) //2p hopper i/o

	PORT_START("IN1")   // ppi0 (5001)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") //?
	PORT_DIPNAME( 0x02, 0x02, "IN1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )  PORT_NAME( "Analyzer" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("1P Coin")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("2P Coin")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY_1P")    // ppi1 (6001)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("1P 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("1P 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("1P 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("1P 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("1P 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("1P 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("1P 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("1P 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("1P 2-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("1P 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("1P 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("1P 3-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("1P 4-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("1P 4-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("1P 5-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEY_2P")    // ppi1 (6001)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("2P 1-2") PORT_CODE(KEYCODE_Q) PORT_COCKTAIL
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("2P 1-3") PORT_CODE(KEYCODE_W) PORT_COCKTAIL
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("2P 1-4") PORT_CODE(KEYCODE_E) PORT_COCKTAIL
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("2P 1-5") PORT_CODE(KEYCODE_R) PORT_COCKTAIL
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("2P 1-6") PORT_CODE(KEYCODE_T) PORT_COCKTAIL
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("2P 2-3") PORT_CODE(KEYCODE_A) PORT_COCKTAIL
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("2P 2-4") PORT_CODE(KEYCODE_S) PORT_COCKTAIL
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("2P 2-5") PORT_CODE(KEYCODE_D) PORT_COCKTAIL
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("2P 2-6") PORT_CODE(KEYCODE_F) PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("2P 3-4") PORT_CODE(KEYCODE_G) PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("2P 3-5") PORT_CODE(KEYCODE_H) PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("2P 3-6") PORT_CODE(KEYCODE_Z) PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON13 ) PORT_NAME("2P 4-5") PORT_CODE(KEYCODE_X) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON14 ) PORT_NAME("2P 4-6") PORT_CODE(KEYCODE_C) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON15 ) PORT_NAME("2P 5-6") PORT_CODE(KEYCODE_V) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MUX0")  // ppi1 (6002)
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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

	PORT_START("MUX1")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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
	PORT_DIPNAME( 0x20, 0x20, "Game Type?" ) //enables two new msgs "advance" and "exchange" in analyzer mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "POST Check" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Reset Backup RAM" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kingdrbb )
	/*this might be different.*/
	PORT_START("HPIO")  // ppi0 (5000)
	PORT_DIPNAME( 0x01, 0x01, "HPIO" )
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
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) //1p hopper i/o
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL ) //2p hopper i/o

	PORT_START("IN0")   // ppi0 (5001)
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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

	PORT_START("IN1")   // ppi0 (5001)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") //?
	PORT_DIPNAME( 0x02, 0x02, "IN1" )
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

	PORT_START("SYSTEM")    // ppi0 (5001)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x02, 0x02, "SYSTEM" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) //causes "hopper overpay" msg
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY0") // (7800)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1P 1-2") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1P 1-3") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("1P 1-4") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("1P 1-5") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1") // (7801)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("1P 1-6") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("1P 2-3") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("1P 2-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("1P 2-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("1P 2-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("1P 3-4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("1P 3-5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("1P 3-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("1P 4-5") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("1P 4-6") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("1P 5-6") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM" )
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
	PORT_DIPNAME( 0x20, 0x20, "Game Type?" ) //enables two new msgs "advance" and "exchange" in analyzer mode
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "POST Check" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Reset Backup RAM" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
*
* GFX decoding
*
*************************************/

static const gfx_layout layout8x8x2 =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(1,2),
		RGN_FRAC(0,2)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout16x16x2 =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(1,2),
		RGN_FRAC(0,2)
	},
	{ 0,1,2,3,4,5,6,7,16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8  },
	16*16
};

/* seems more suitable with 2bpp?*/
static const gfx_layout cowrace_layout16x16x2 =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{
		RGN_FRAC(1,2),
		RGN_FRAC(0,2),
	},
	{ 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7,0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8  },
	16*16
};

static GFXDECODE_START( kingdrby )
	GFXDECODE_ENTRY( "gfx1", 0x0000, layout16x16x2, 0x080, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, layout8x8x2,   0x000, 0x80 )
GFXDECODE_END

static GFXDECODE_START( cowrace )
	GFXDECODE_ENTRY( "gfx1", 0x000000, cowrace_layout16x16x2, 0x080, 0x10 )
	GFXDECODE_ENTRY( "gfx2", 0x000000, layout8x8x2, 0x000, 0x80 )
GFXDECODE_END

/*************************************
 *
 *  Sound HW Config
 *
 *************************************/

PALETTE_INIT_MEMBER(kingdrby_state,kingdrby)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x200; ++i)
	{
		bit0 = 0;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 0) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 7) & 0x01;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

PALETTE_INIT_MEMBER(kingdrby_state,kingdrbb)
{
	UINT8 *raw_prom = memregion("raw_prom")->base();
	UINT8 *prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for(i = 0; i < 0x200; i++)
	{
		/* this set has an extra address line shuffle applied on the prom */
		prom[i] = raw_prom[BITSWAP16(i, 15,14,13,12,11,10,9,8,7,6,5,0,1,2,3,4)+0x1000];
	}

	for(i = 0; i < 0x200; i++)
	{
		bit0 = 0;
		bit1 = (prom[i] >> 1) & 0x01;
		bit2 = (prom[i] >> 0) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (prom[i] >> 4) & 0x01;
		bit1 = (prom[i] >> 3) & 0x01;
		bit2 = (prom[i] >> 2) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (prom[i] >> 7) & 0x01;
		bit1 = (prom[i] >> 6) & 0x01;
		bit2 = (prom[i] >> 5) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

static MACHINE_CONFIG_START( kingdrby, kingdrby_state )
	MCFG_CPU_ADD("master", Z80, CLK_2)
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_IO_MAP(master_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingdrby_state,  irq0_line_hold)

	MCFG_CPU_ADD("slave", Z80, CLK_2)
	MCFG_CPU_PROGRAM_MAP(slave_map)
	MCFG_CPU_IO_MAP(slave_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kingdrby_state,  irq0_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, CLK_2)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(kingdrby_state, irq0_line_hold, 1000) /* guess, controls ay8910 tempo.*/

	MCFG_QUANTUM_PERFECT_CPU("master")

	MCFG_NVRAM_ADD_0FILL("nvram")

	// 5000-5003 PPI group modes 0/0 - A & B as input, C (all) as output.
	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(kingdrby_state, hopper_io_r))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(kingdrby_state, hopper_io_w))

	// 6000-6003 PPI group modes 0/0 - B & C (lower) as input, A & C (upper) as output.
	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kingdrby_state, sound_cmd_w))
	MCFG_I8255_IN_PORTB_CB(READ8(kingdrby_state, key_matrix_r))
	MCFG_I8255_IN_PORTC_CB(READ8(kingdrby_state, input_mux_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(kingdrby_state, outport2_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kingdrby)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(kingdrby_state,kingdrby)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)    /* controlled by CRTC */
	MCFG_SCREEN_UPDATE_DRIVER(kingdrby_state, screen_update_kingdrby)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CLK_1/32)  /* 53.333 Hz. guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, CLK_1/8)    /* guess */
	MCFG_AY8910_PORT_A_READ_CB(READ8(kingdrby_state, sound_cmd_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( kingdrbb, kingdrby )

	MCFG_CPU_MODIFY("slave")
	MCFG_CPU_PROGRAM_MAP(slave_1986_map)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(kingdrby_state,kingdrbb)

	MCFG_DEVICE_REMOVE("ppi8255_0")
	MCFG_DEVICE_REMOVE("ppi8255_1")

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	/* C as input, (all) as output */
	MCFG_I8255_OUT_PORTA_CB(WRITE8(kingdrby_state, sound_cmd_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN0"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(kingdrby_state, outportb_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN1"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	/* actually unused */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cowrace, kingdrbb )

	MCFG_CPU_MODIFY("soundcpu")
	MCFG_CPU_PROGRAM_MAP(cowrace_sound_map)
	MCFG_CPU_IO_MAP(cowrace_sound_io)

	MCFG_GFXDECODE_MODIFY("gfxdecode", cowrace)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(kingdrby_state,kingdrby)
	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_REPLACE("aysnd", YM2203, 3000000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(kingdrby_state, sound_cmd_r))
	MCFG_AY8910_PORT_B_READ_CB(DEVREAD8("oki", okim6295_device, read))   // read B
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("oki", okim6295_device, write))   // write B
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

ROM_START( kingdrby )
	ROM_REGION( 0x3000, "master", 0 )
	ROM_LOAD( "im4_d.d6", 0x0000, 0x1000, CRC(20f2d999) SHA1(91db46059f32b4791460df3330260f4e60f016a5) )
	ROM_LOAD( "im5_d.c6", 0x1000, 0x1000, CRC(c192cecc) SHA1(63436bf3d9c1e34f6549830c8164295b7758d666) )
	ROM_LOAD( "im6_d.b6", 0x2000, 0x1000, CRC(257f4e0d) SHA1(cd61f3cf70c536aa207ebfdd28be54ac586b5249) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "sg1_b.e1", 0x0000, 0x1000, CRC(92ef3c13) SHA1(1bf1e4106b37aadfc02822184510740e18a54d5c) )

	ROM_REGION( 0x4000, "slave", 0 )
	ROM_LOAD( "im1_yk.g1", 0x0000, 0x1000, CRC(1921605d) SHA1(0aa6f7195ea59d0080620ab02a737e5c319dd3e7) )
	ROM_LOAD( "im2_yk.f1", 0x1000, 0x1000, CRC(8504314e) SHA1(309645e17fb3149dce57ae6844cc58652a1eeb35) )
	ROM_LOAD( "im3_yk.e1", 0x2000, 0x1000, CRC(b0e473ec) SHA1(234598548b2a2a8f53d40bc07c3b1759074b7d93) )
	ROM_COPY( "soundcpu", 0x0000, 0x3000, 0x1000 )

	/* sprites gfxs */
	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "s1.d1", 0x0000, 0x1000, CRC(26974007) SHA1(5079daf9ad7d84f935c256458060db9497daef91) )
	ROM_LOAD( "s2.e1", 0x1000, 0x1000, CRC(bedebfa7) SHA1(5a2116ed4af6bc4b72199017515980e4a937236c) )
	ROM_LOAD( "s3.f1", 0x2000, 0x1000, CRC(0aa59571) SHA1(5005ffdd0030e4d4c1d8033fd3c78177c0fbd1b0) )
	ROM_LOAD( "s4.g1", 0x3000, 0x1000, CRC(ccd5fb0e) SHA1(3ee4377d15e7731586b7a3457dbae52edaed72d3) )
	ROM_LOAD( "s5.d2", 0x4000, 0x1000, CRC(32613df3) SHA1(21ce057c416e6f1d0a3e112d640b1cf52ba69206) )
	ROM_LOAD( "s6.e2", 0x5000, 0x1000, CRC(a151c422) SHA1(354efaee64c8cc457f96cba4722f6a0df66e14d3) )
	ROM_LOAD( "s7.f2", 0x6000, 0x1000, CRC(7cfcee55) SHA1(590ac02941e82371d56113d052eb4d4bcdbf83b0) )
	ROM_LOAD( "s8.g2", 0x7000, 0x1000, CRC(ad667c05) SHA1(d9bdf3a125eba2d40191b0659c2007ccbc6fd12b) )

	/* tile gfxs */
	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "s9_a.k8",  0x0000, 0x1000, CRC(ca82cd81) SHA1(fdf47df7705c8d0ae70b5a0e29b35819f3d0749a) )
	ROM_LOAD( "s10_a.l8", 0x1000, 0x1000, CRC(37b2736f) SHA1(15ef3f563aebd1f5506135c7c01e9a1db30a9ccc) )

	/* color proms */
	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "147.f8", 0x000, 0x200, CRC(9245c4af) SHA1(813d628ac55913542a4deabe6ac0a4f9db09cf19) )
ROM_END

// might be closer to the original than the cowrace bootleg even if it shares some roms with cowrace?
ROM_START( kingdrbb ) // has 'Made in Taiwan' on the PCB.
	ROM_REGION( 0x8000, "master", 0 )
	ROM_LOAD( "kingdrbb_u3.bin", 0x0000, 0x4000, CRC(90a14686) SHA1(ce2ee8c0cbb4212aa8e71d7145b1eefb4f282590) )
	ROM_CONTINUE(0x0000,0x4000)

	ROM_REGION( 0x4000, "slave", 0 ) // slave z80?
	ROM_LOAD( "kingdrbb_u30.bin", 0x0000, 0x4000, CRC(98717214) SHA1(70ca61c15f66642b85dd248b24638a95a8254bbb) )
	ROM_CONTINUE(0x0000,0x4000)

	ROM_REGION( 0x4000, "soundcpu", 0 ) // audio z80?
	ROM_LOAD( "kingdrbb_u161.bin", 0x0000, 0x4000, CRC(7d84b577) SHA1(d230b49df19bf68793a561f4c07413d22fa8ff28) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "kingdrbb_u94.bin", 0x0000, 0x4000, CRC(ba59e4b8) SHA1(425dc4055d5ea400eeef33e26fcc73fa726f414d) )
	ROM_LOAD( "kingdrbb_u95.bin", 0x4000, 0x4000, CRC(fa97deb6) SHA1(1630281f0cac3fe3bfaf924e1c6316107200eb4a) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "kingdrbb_u39.bin",  0x0000, 0x2000, CRC(4a34cef0) SHA1(b54f1f2ccd3dd773e47bfb044c5aec15c11426c2) )
	ROM_LOAD( "kingdrbb_u140.bin", 0x2000, 0x2000, CRC(7e24b674) SHA1(c774efeb8e4e833e73c29007d5294c93df1abef4) )

	ROM_REGION( 0x4000, "raw_prom", 0 )
	ROM_LOAD( "kingdrbb_u1.bin", 0x0000, 0x4000, CRC(97931952) SHA1(a0ef3be105f2ed7f744c73e92c583d25bb322e6a) ) // palette but in a normal rom?

	ROM_REGION( 0x200, "proms", ROMREGION_ERASE00 ) // address shuffled, decoded inside palette_init
//  ROM_COPY( "raw_prom", 0x1000, 0x000, 0x200 )
//  ROM_COPY( "raw_prom", 0x3000, 0x200, 0x200 ) //identical to 0x1000 bank

	ROM_REGION( 0x4000, "pals", 0 ) // all read protected
	ROM_LOAD( "palce16v.u101.bin", 0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u113.bin", 0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u160.bin", 0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u2.bin",   0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u29.bin",  0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u4.bin",   0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "palce16v.u75.bin",  0x0000, 0x117, CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	// jedutil just complains these are invalid..
	ROM_LOAD( "palce16v8q.u53.jed", 0x0000, 0x892, CRC(123d539a) SHA1(cccf0cbae3175b091a998eedf4aa44a55b679400) )
	ROM_LOAD( "palce16v8q.u77.jed", 0x0000, 0x892, CRC(b7956421) SHA1(57db38b571adf6cf49d7c221cd65a068a9a3383a) )
	ROM_LOAD( "palce16v8q.u87.jed", 0x0000, 0x892, CRC(b7956421) SHA1(57db38b571adf6cf49d7c221cd65a068a9a3383a) )
ROM_END

ROM_START( cowrace )
	ROM_REGION( 0x8000, "master", 0 )
	ROM_LOAD( "u3.bin", 0x0000, 0x8000, CRC(c05c3bd3) SHA1(b7199a069ab45edd25e021589b79105cdfa5511a) )

	ROM_REGION( 0x2000, "soundcpu", 0 )
	ROM_LOAD( "u164.bin", 0x0000, 0x2000, CRC(9affa1c8) SHA1(bfc07693e8f749cbf20ab8cda33975b66f567962) )

	ROM_REGION( 0x8000, "slave", ROMREGION_ERASEFF ) // slave z80?
	/* I've tried the kingdrbb slave CPU rom ... game works until the auto race in attract mode. We need to locate and dump this on the PCB. */
	ROM_LOAD( "slave.bin", 0x0000, 0x8000, NO_DUMP )
	ROM_FILL( 0x0000, 0x8000, 0xff ) // <- to remove once that the above is dumped

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u94.bin", 0x8000, 0x8000, CRC(945dc115) SHA1(bdd145234e6361c42ed20e8ca4cac64f07332748) )
	ROM_LOAD( "u95.bin", 0x0000, 0x8000, CRC(fc1fc006) SHA1(326a67c1ea0f487ecc8b7aef2d90124a01e6dee3) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "u139.bin", 0x2000, 0x2000, CRC(b746bb2f) SHA1(5f5f48752689079ed65fe7bb4a69512ada5db05d) )
	ROM_LOAD( "u140.bin", 0x0000, 0x2000, CRC(7e24b674) SHA1(c774efeb8e4e833e73c29007d5294c93df1abef4) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u4.bin", 0x00000, 0x20000, CRC(f92a3ab5) SHA1(fc164492793597eadb8a50154410936edb74fa23) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "u149.bin", 0x00000, 0x200, CRC(f41a5eca) SHA1(797f2d95d4e00f96e5a99604935810e1add59689) )
ROM_END

/*******************************************************************************************

   cpu bd

  H6 H5 H4 Z80A       8255
               2114 2114
 10pos
 rotary sw

 push button

 8pos dip sw
                      8255
   6845  Z80A
             H3 H2 H1 5101 5101

  video bd

             h16 9  8  7
             h17 13 12 11


       H15 H14       clr

*******************************************************************************************/

ROM_START( kingdrbb2 )
	ROM_REGION( 0x3000, "master", 0 )
	ROM_LOAD( "h4", 0x0000, 0x1000, CRC(20f2d999) SHA1(91db46059f32b4791460df3330260f4e60f016a5) ) // = im4_d.d6 kingdrby
	ROM_LOAD( "h5", 0x1000, 0x1000, CRC(c192cecc) SHA1(63436bf3d9c1e34f6549830c8164295b7758d666) ) // = im5_d.c6 kingdrby
	ROM_LOAD( "h6", 0x2000, 0x1000, CRC(257f4e0d) SHA1(cd61f3cf70c536aa207ebfdd28be54ac586b5249) ) // = im6_d.b6 kingdrby

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "sg1_b.e1", 0x0000, 0x1000, BAD_DUMP CRC(92ef3c13) SHA1(1bf1e4106b37aadfc02822184510740e18a54d5c) )   // not in this set, using kingdrby one...

	ROM_REGION( 0x4000, "slave", 0 )
	ROM_LOAD( "h1", 0x0000, 0x1000, CRC(444aa020) SHA1(0d40e9499892177b4d8123ad7b6909e8f6e0c8ab) )
	ROM_LOAD( "h2", 0x1000, 0x1000, CRC(4102d356) SHA1(d9eb2dcc7cbbe2449fe41ef5eef0470d4dcac0c8) )
	ROM_LOAD( "h3", 0x2000, 0x1000, CRC(b5a95093) SHA1(dd3e0742042fab113092664bde0e1215d009d861) )
	ROM_COPY( "soundcpu", 0x0000, 0x3000, 0x1000 )

	/* sprites gfxs */
	ROM_REGION( 0x8000, "gfx1", 0 ) // kingdrby sprites, smaller top roms
	ROM_LOAD( "7",   0x0000, 0x1000, CRC(26974007) SHA1(5079daf9ad7d84f935c256458060db9497daef91) ) // = s1.d1 kingdrby
	ROM_LOAD( "8",   0x1000, 0x1000, CRC(bedebfa7) SHA1(5a2116ed4af6bc4b72199017515980e4a937236c) ) // = s2.e1 kingdrby
	ROM_LOAD( "9",   0x2000, 0x1000, CRC(0aa59571) SHA1(5005ffdd0030e4d4c1d8033fd3c78177c0fbd1b0) ) // = s3.f1 kingdrby
	ROM_LOAD( "h16", 0x3000, 0x0800, CRC(252ee644) SHA1(cdc14fc0c04d6e92bb16c8025538d46f8110d121) ) // = s4.g1 [1/2] kingdrby
	ROM_FILL(        0x3800, 0x0800, 0xff )
	ROM_LOAD( "11",  0x4000, 0x1000, CRC(32613df3) SHA1(21ce057c416e6f1d0a3e112d640b1cf52ba69206) ) // = s5.d2 kingdrby
	ROM_LOAD( "12",  0x5000, 0x1000, CRC(a151c422) SHA1(354efaee64c8cc457f96cba4722f6a0df66e14d3) ) // = s6.e2 kingdrby
	ROM_LOAD( "13",  0x6000, 0x1000, CRC(7cfcee55) SHA1(590ac02941e82371d56113d052eb4d4bcdbf83b0) ) // = s7.f2 kingdrby
	ROM_LOAD( "h17", 0x7000, 0x0800, CRC(dd79b1fa) SHA1(1e0914d1047812cd3cbca1ad8baded7691cd5db4) ) // = s8.g2 [1/2] kingdrby
	ROM_FILL(        0x7800, 0x0800, 0xff )

	/* tile gfxs */
	ROM_REGION( 0x2000, "gfx2", 0 ) // kingdrby tiles, but Tazmi logo, Japanese text and upside-down letters removed
	ROM_LOAD( "h14", 0x0000, 0x1000, CRC(2d040e86) SHA1(5c6f0404c0740c56b9362aa4665fbd59cf8459d6) ) // = s9_a.k8  (95.6%) kingdrby
	ROM_LOAD( "h15", 0x1000, 0x1000, CRC(eec11348) SHA1(4933895aed31430f289342c36c57e6741c736a93) ) // = s10_a.l8 (78.6%) kingdrby

	/* color proms */
	ROM_REGION( 0x200, "proms", 0 ) // mostly 00 (black) -> red (80) changes
	ROM_LOAD( "clr", 0x000, 0x200, CRC(ffabacc9) SHA1(9769fb27fc5b6998e6600cb6050086385caa3f96) )
ROM_END


GAMEL( 1981, kingdrby,  0,        kingdrby, kingdrby, driver_device, 0, ROT0, "Tazmi",                        "King Derby (1981)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_kingdrby )
GAME ( 1986, kingdrbb,  kingdrby, kingdrbb, kingdrbb, driver_device, 0, ROT0, "bootleg (Casino Electronics)", "King Derby (Taiwan bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS )
GAMEL( 198?, kingdrbb2, kingdrby, kingdrby, kingdrby, driver_device, 0, ROT0, "bootleg",                      "King Derby (bootleg set 2)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND, layout_kingdrby )
GAME ( 2000, cowrace,   kingdrby, cowrace,  kingdrbb, driver_device, 0, ROT0, "bootleg (Gate In)",            "Cow Race (King Derby hack)",  MACHINE_NOT_WORKING | MACHINE_WRONG_COLORS )
