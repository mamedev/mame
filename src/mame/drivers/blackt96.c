/*

    Black Touch '96


Black Touch 96
D.G.R.M. of Korea, 1996

This game is a beat'em-up like Double Dragon

PCB Layout
----------

D.G.R.M. NO 1947
|---------------------------------------------|
| M6295    1     8MHz                         |
| M6295    2              2018  2018          |
|         16C57           2018  2018          |
|HA13001                  2018  2018          |
|                         2018  2018          |
|                       PAL     PAL           |
|    6116                 5      6            |
|J   6116                 7      8            |
|A                                            |
|M                                            |
|M                                            |
|A                                            |
|                               9  10         |
|    DSW1           24MHz               PAL   |
|    DSW2                                     |
|   PAL PAL           ACTEL     6116    11    |
|   62256    62256    A1020B            12    |
|   3        4        PL84C     6264    13    |
|                               6264    14    |
|18MHz 68000                    6264          |
|---------------------------------------------|
Notes:
      68000 clock 9.000MHz [18/2]
      M6295 clocks 1.000MHz [8/8] pin 7 high


2008-07
Added Dip Locations based on Service Mode


Is this based on some Seta / Taito HW design, the sprite-tilemaps look familiar

*/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"


class blackt96_state : public driver_device
{
public:
	blackt96_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16* m_tilemapram;
	UINT16* m_tilemapram2;
	DECLARE_WRITE16_MEMBER(blackt96_c0000_w);
	DECLARE_WRITE16_MEMBER(blackt96_80000_w);
	DECLARE_READ8_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_WRITE8_MEMBER(blackt96_soundio_port00_w);
	DECLARE_READ8_MEMBER(blackt96_soundio_port01_r);
	DECLARE_WRITE8_MEMBER(blackt96_soundio_port01_w);
	DECLARE_READ8_MEMBER(blackt96_soundio_port02_r);
	DECLARE_WRITE8_MEMBER(blackt96_soundio_port02_w);
};


static VIDEO_START( blackt96 )
{
}

static void draw_strip(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int stripnum, int xbase, int ybase, int bg)
{
	blackt96_state *state = machine.driver_data<blackt96_state>();
	const gfx_element *gfxspr = machine.gfx[1];
	const gfx_element *gfxbg = machine.gfx[0];

	int base = stripnum;
	int count = 0;
	int y;

	for (y=0;y<32;y++)
	{
		UINT16 tile = (state->m_tilemapram2[count*2 + (base/2)+1]&0x3fff);
		UINT16 flipx = (state->m_tilemapram2[count*2 + (base/2)+1]&0x4000);
		UINT16 colour = (state->m_tilemapram2[count*2 + (base/2)]&0x00ff);

		if (tile&0x2000)
		{
			if (bg) drawgfx_transpen(bitmap,cliprect,gfxbg,tile&0x1fff,colour>>4,flipx,0,xbase,ybase+y*16,0);
		}
		else
		{
			if (!bg) drawgfx_transpen(bitmap,cliprect,gfxspr,tile&0x1fff,colour,flipx,0,xbase,ybase+y*16,0);
		}

		count++;
	}

}

static void draw_main(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int bg)
{
	blackt96_state *state = machine.driver_data<blackt96_state>();

	int x;


	for (x=(0x1000/2)-2;x>=0;x-=2)
	{
		int xx;
		int yy;
		int s = 0;

		xx=  ((state->m_tilemapram2[x+0]&0x001f)<<4) | (state->m_tilemapram2[x+1]&0xf000)>>12;
		yy = ((state->m_tilemapram2[x+1]&0x1ff));

		if (xx&0x100) xx-=0x200;
		yy = 0x1ff-yy;
		if (yy&0x100) yy-=0x200;

		s = x*2;
		s &=0xfff;
		s += (x&7)*0x800;


		draw_strip(machine,bitmap,cliprect,s,xx,yy,bg);
	}

}

static SCREEN_UPDATE_IND16( blackt96 )
{
	blackt96_state *state = screen.machine().driver_data<blackt96_state>();
	int count;
	int x,y;
	const gfx_element *gfx = screen.machine().gfx[2];

	bitmap.fill(get_black_pen(screen.machine()), cliprect);

	draw_main(screen.machine(),bitmap,cliprect,1);
	draw_main(screen.machine(),bitmap,cliprect,0);

	/* Text Layer */
	count = 0;
	for (x=0;x<64;x++)
	{
		for (y=0;y<32;y++)
		{
			UINT16 tile = (state->m_tilemapram[count*2]&0x7ff)+0x800; // +0xc00 for korean text
			drawgfx_transpen(bitmap,cliprect,gfx,tile,0,0,0,x*8,-16+y*8,0);
			count++;
		}
	}

	return 0;
}


WRITE16_MEMBER(blackt96_state::blackt96_c0000_w)
{
	printf("blackt96_c0000_w %04x %04x\n",data,mem_mask);
}

WRITE16_MEMBER(blackt96_state::blackt96_80000_w)
{
	// TO sound MCU?
	printf("blackt96_80000_w %04x %04x\n",data,mem_mask);
}


static ADDRESS_MAP_START( blackt96_map, AS_PROGRAM, 16, blackt96_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("P1_P2") AM_WRITE(blackt96_80000_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("IN1") AM_WRITE(blackt96_c0000_w) // COIN INPUT
	AM_RANGE(0x0e0000, 0x0e0001) AM_READ_PORT("IN2")
	AM_RANGE(0x0e8000, 0x0e8001) AM_READ_PORT("IN3")
	AM_RANGE(0x0f0000, 0x0f0001) AM_READ_PORT("DSW1")
	AM_RANGE(0x0f0008, 0x0f0009) AM_READ_PORT("DSW2")

	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_BASE(m_tilemapram) // text tilemap
	AM_RANGE(0x200000, 0x207fff) AM_RAM AM_BASE(m_tilemapram2)// sprite list + sprite tilemaps
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_SHARE("paletteram")
	AM_RANGE(0xc00000, 0xc03fff) AM_RAM // main ram

ADDRESS_MAP_END



static INPUT_PORTS_START( blackt96 )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Test mode lists this as Service 1, but it appears to be Coin 1 (uses Coin 1 coinage etc.)
	PORT_DIPNAME( 0x0002, 0x0002, "xx")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) // Test mode lists this as Coin 1, but it doesn't work
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x0001, 0x0001, "3" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

    /* Dipswitch Port A */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0300, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(      0x0c00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPNAME( 0x2000, 0x2000, "Bonus Life Type" ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(      0x2000, "Every" )
	PORT_DIPSETTING(      0x0000, "Second Only" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:!2")	// ?
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

    /* Dipswitch Port B */
	PORT_START("DSW2")
	PORT_SERVICE( 0x0100, IP_ACTIVE_HIGH ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPNAME( 0x0200, 0x0200, "Continue" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c00, 0x0400, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(      0x0000, "20000 / 50000" )
	PORT_DIPSETTING(      0x0400, "60000 / 150000" )
	PORT_DIPSETTING(      0x0800, "40000 / 100000" )
	PORT_DIPSETTING(      0x0c00, "No Bonus" )
	PORT_DIPNAME( 0x3000, 0x0000, "Demo Sound / Video Freeze" ) PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(      0x0000, "Demo Sound On" )
	PORT_DIPSETTING(      0x1000, "Never Finish" )
	PORT_DIPSETTING(      0x2000, "Demo Sound Off" )
	PORT_DIPSETTING(      0x3000, "Stop Video" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")	// 'Level'
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0xc000, "4" )
INPUT_PORTS_END



static const gfx_layout blackt96_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{  1024+32, 1024+40, 1024+48, 1024+56, 1024+0, 1024+8, 1024+16, 1024+24,
		32,40,48,56,0,8,16,24 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	32*64
};

static const gfx_layout blackt962_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,24,8, 16 },
	{ 519, 515, 518, 514,  517,513,  516,512, 7,3,6,2,5,1,4,0 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	32*32
};


static const gfx_layout blackt963_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,4,8, 12 },
	{ 131,130,129,128,3,2,1,0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*16
};

static GFXDECODE_START( blackt96 )
	GFXDECODE_ENTRY( "gfx1", 0, blackt96_layout,    0x0, 0x10  )
	GFXDECODE_ENTRY( "gfx2", 0, blackt962_layout,   0x0, 0x80  )
	GFXDECODE_ENTRY( "gfx3", 0, blackt963_layout,   0x0, 0x80  )
GFXDECODE_END


READ8_MEMBER(blackt96_state::PIC16C5X_T0_clk_r)
{
	return 0;
}

WRITE8_MEMBER(blackt96_state::blackt96_soundio_port00_w)
{
}

READ8_MEMBER(blackt96_state::blackt96_soundio_port01_r)
{
	return machine().rand();
}

WRITE8_MEMBER(blackt96_state::blackt96_soundio_port01_w)
{
}

READ8_MEMBER(blackt96_state::blackt96_soundio_port02_r)
{
	return machine().rand();
}

WRITE8_MEMBER(blackt96_state::blackt96_soundio_port02_w)
{
}

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, blackt96_state )
	AM_RANGE(0x00, 0x00) AM_WRITE(blackt96_soundio_port00_w )
	AM_RANGE(0x01, 0x01) AM_READWRITE(blackt96_soundio_port01_r, blackt96_soundio_port01_w )
	AM_RANGE(0x02, 0x02) AM_READWRITE(blackt96_soundio_port02_r, blackt96_soundio_port02_w )
	AM_RANGE(PIC16C5x_T0, PIC16C5x_T0) AM_READ(PIC16C5X_T0_clk_r)
ADDRESS_MAP_END



static MACHINE_CONFIG_START( blackt96, blackt96_state )
	MCFG_CPU_ADD("maincpu", M68000, 18000000 /2)
	MCFG_CPU_PROGRAM_MAP(blackt96_map)
	MCFG_CPU_VBLANK_INT("screen", irq1_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 8000000)	/* ? */
	MCFG_CPU_IO_MAP(sound_io_map)

	MCFG_GFXDECODE(blackt96)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
//  MCFG_SCREEN_VISIBLE_AREA(0*8, 16*32-1, 0*8, 16*32-1)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 256-1, 0*8, 208-1)
	MCFG_SCREEN_UPDATE_STATIC(blackt96)

	MCFG_PALETTE_LENGTH(0x800)

	MCFG_VIDEO_START(blackt96)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 8000000/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)

	MCFG_OKIM6295_ADD("oki2", 8000000/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.47)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.47)
MACHINE_CONFIG_END


ROM_START( blackt96 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "3", 0x00001, 0x40000, CRC(fc2c1d79) SHA1(742478237819af16d3fd66039283202b3c07eedd) )
	ROM_LOAD16_BYTE( "4", 0x00000, 0x40000, CRC(caff5b4a) SHA1(9a388cbb07211fa66f27082a8a5b847168c86a4f) )

	ROM_REGION( 0x80000, "audiocpu", 0 ) /* PIC16c57 Code */
	ROM_LOAD( "pic16c57.bin", 0x00000, 0x2000, CRC(6053ba2f) SHA1(5dd28ddff17555de0e8574b78ff9e71204c503d3) )

	ROM_REGION( 0x080000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "1", 0x00000, 0x80000, CRC(6a934174) SHA1(087f5fa226dc68ee217f99c64d16cdf14372d44c) )

	ROM_REGION( 0x040000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "2", 0x00000, 0x40000, CRC(94009cd4) SHA1(aa36298e280c20bf86d70f3eb3fb33aca4df07e3) )

	ROM_REGION( 0x200000, "gfx1", 0 ) // bg tiles
	ROM_LOAD16_BYTE( "5", 0x100000, 0x40000, CRC(6e52c331) SHA1(31ef1d352d4ee5f7b3ef336b1f052c3a1468f22e) )
	ROM_LOAD16_BYTE( "6", 0x100001, 0x40000, CRC(69637a5a) SHA1(a5731478856d8bb91d34b747838b2b47772864ef) )
	ROM_LOAD16_BYTE( "7", 0x000000, 0x80000, CRC(6b04e8a8) SHA1(309ba1efd60600a30e1ae8f6e8b92939c23cd9c6) )
	ROM_LOAD16_BYTE( "8", 0x000001, 0x80000, CRC(16c656be) SHA1(06c40c16080a97b01a638776d28f36594ce4fb3b) )

	ROM_REGION( 0x100000, "gfx2", 0 ) // sprite tiles
	ROM_LOAD32_BYTE( "11", 0x00000, 0x40000, CRC(9eb773a3) SHA1(9c91ee938438a61f5fa650ced6249e34aa5321bd) )
	ROM_LOAD32_BYTE( "12", 0x00001, 0x40000, CRC(8894e608) SHA1(389974a0b208b7cbf7d5f83641ddc058ad5ebe87) )
	ROM_LOAD32_BYTE( "13", 0x00002, 0x40000, CRC(0acceb9d) SHA1(e8a85c7eab45d84613ac37a9b7ffbc45b44eb2e5) )
	ROM_LOAD32_BYTE( "14", 0x00003, 0x40000, CRC(b5e3de25) SHA1(33ac5602ab6bcadc8b0d1aa805a3bdce0b67c215) )

	ROM_REGION( 0x20000, "gfx3", 0 ) // txt tiles
	ROM_LOAD16_BYTE( "9",  0x00000, 0x10000, CRC(81a4cf4c) SHA1(94b2bbcbc8327d9babbc3b222bd88954c7e7b80e) )
	ROM_LOAD16_BYTE( "10", 0x00001, 0x10000, CRC(b78232a2) SHA1(36a4f01011faf64e46b73f0082ab04843ac8b0e2) )
ROM_END

GAME( 1996, blackt96,    0,        blackt96,    blackt96,    0, ROT0,  "D.G.R.M.", "Black Touch '96", GAME_NOT_WORKING | GAME_NO_SOUND )
