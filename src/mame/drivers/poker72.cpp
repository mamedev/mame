// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
  Unknown game, dump was marked 'slot 72 - poker'

  GFX roms contain
  'Extrema Systems International Ltd'
  as well as a logo for the company.

  There are also 'Lucky Boy' graphics in various places, which might be the title.


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"


class poker72_state : public driver_device
{
public:
	poker72_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_pal(*this, "pal"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_pal;
	UINT8 m_tile_bank;
	DECLARE_WRITE8_MEMBER(poker72_paletteram_w);
	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_WRITE8_MEMBER(tile_bank_w);
	DECLARE_DRIVER_INIT(poker72);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(poker72);
	UINT32 screen_update_poker72(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void poker72_state::video_start()
{
}

UINT32 poker72_state::screen_update_poker72(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;

	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile = ((m_vram[count+1] & 0x0f) << 8 ) | (m_vram[count+0] & 0xff); //TODO: tile bank
			int fx = (m_vram[count+1] & 0x10);
			int fy = (m_vram[count+1] & 0x20);
			int color = (m_vram[count+1] & 0xc0) >> 6;

			tile|= m_tile_bank << 12;

			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,tile,color,fx,fy,x*8,y*8);

			count+=2;
		}
	}

	return 0;
}

WRITE8_MEMBER(poker72_state::poker72_paletteram_w)
{
	int r,g,b;
	m_pal[offset] = data;

	r = m_pal[(offset & 0x3ff)+0x000] & 0x3f;
	g = m_pal[(offset & 0x3ff)+0x400] & 0x3f;
	b = m_pal[(offset & 0x3ff)+0x800] & 0x3f;

	m_palette->set_pen_color( offset & 0x3ff, pal6bit(r), pal6bit(g), pal6bit(b));
}

WRITE8_MEMBER(poker72_state::output_w)
{
	UINT8 *ROM = memregion("maincpu")->base();

	printf("%02x\n",data);

/*  if((data & 0xc) == 0xc)
        membank("bank1")->set_base(&ROM[0x10000]);
    else*/
	if(data & 8)
		membank("bank1")->set_base(&ROM[0x08000]);
	else
		membank("bank1")->set_base(&ROM[0x00000]);
}

WRITE8_MEMBER(poker72_state::tile_bank_w)
{
	m_tile_bank = (data & 4) >> 2;
}

static ADDRESS_MAP_START( poker72_map, AS_PROGRAM, 8, poker72_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM //work ram
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xf000, 0xfbff) AM_RAM_WRITE(poker72_paletteram_w) AM_SHARE("pal")
	AM_RANGE(0xfc00, 0xfdff) AM_RAM //???
	AM_RANGE(0xfe08, 0xfe08) AM_READ_PORT("IN0")
	AM_RANGE(0xfe09, 0xfe09) AM_READ_PORT("IN1")
	AM_RANGE(0xfe0a, 0xfe0a) AM_READ_PORT("IN2")
	AM_RANGE(0xfe0c, 0xfe0c) AM_READ_PORT("IN3")
	AM_RANGE(0xfe0d, 0xfe0d) AM_READ_PORT("IN4")
	AM_RANGE(0xfe0e, 0xfe0e) AM_READ_PORT("IN5")

	AM_RANGE(0xfe17, 0xfe17) AM_READNOP //irq ack
	AM_RANGE(0xfe20, 0xfe20) AM_WRITE(output_w) //output, irq enable?
	AM_RANGE(0xfe22, 0xfe22) AM_WRITE(tile_bank_w)
	AM_RANGE(0xfe40, 0xfe40) AM_DEVREADWRITE("ay", ay8910_device, data_r, data_w)
	AM_RANGE(0xfe60, 0xfe60) AM_DEVWRITE("ay", ay8910_device, address_w)

	AM_RANGE(0xff00, 0xffff) AM_RAM //??
/*
bp 13a

fe06 w
fe08 w
fe0b = 9b (ppi?)
fe24 w

01f9 : call 6399 --> cls

*/
ADDRESS_MAP_END


static INPUT_PORTS_START( poker72 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("M. Bet")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Black")


	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x00, "IN3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x00, "IN4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x00, "IN5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "DSW0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )


INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4), RGN_FRAC(3,4)+4, RGN_FRAC(2,4), RGN_FRAC(2,4)+4 ,RGN_FRAC(1,4),RGN_FRAC(1,4)+4, RGN_FRAC(0,4),RGN_FRAC(0,4)+4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};



static GFXDECODE_START( poker72 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END

/* default 444 palette for debug purpose */
PALETTE_INIT_MEMBER(poker72_state, poker72)
{
	int x,r,g,b;

	for(x=0;x<0x100;x++)
	{
		r = (x & 0xf)*0x10;
		g = ((x & 0x3c)>>2)*0x10;
		b = ((x & 0xf0)>>4)*0x10;
		palette.set_pen_color(x,rgb_t(r,g,b));
	}
}

void poker72_state::machine_reset()
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->set_base(&ROM[0]);
}

static MACHINE_CONFIG_START( poker72, poker72_state )


	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,8000000)         /* ? MHz */
	MCFG_CPU_PROGRAM_MAP(poker72_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", poker72_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(poker72_state, screen_update_poker72)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", poker72)
	MCFG_PALETTE_ADD("palette", 0xe00)
	MCFG_PALETTE_INIT_OWNER(poker72_state, poker72)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay", AY8910, 8000000/8) /* ? Mhz */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



ROM_START( poker72 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27010.bin", 0x00000, 0x20000, CRC(62447341) SHA1(e442c1f834a5dd2ab6ab3bdd316dfa86f2ca6647) )

	ROM_REGION( 0x1000, "89c51", 0 )
	ROM_LOAD( "89c51.bin", 0x00000, 0x1000, CRC(3fdd2148) SHA1(ea39a52482967268c7387aec77cfab1ae5c427fa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "270135.bin", 0x00000, 0x20000, CRC(188c96ee) SHA1(7e883454cb080cdc82ce47ac92f51c8d45a55085) )
	ROM_LOAD( "270136.bin", 0x20000, 0x20000, CRC(f84c5068) SHA1(49178fe7b12f547a50879002236105a882767ebb) )
	ROM_LOAD( "270137.bin", 0x40000, 0x20000, CRC(310281d1) SHA1(c28f97bb3613c0b481ab6e16e215549c44b83c47) )
	ROM_LOAD( "270138.bin", 0x60000, 0x20000, CRC(d689313d) SHA1(8b9661b3af0e2ced7fe9fa487641e445ce7835b8) )
ROM_END

DRIVER_INIT_MEMBER(poker72_state,poker72)
{
	UINT8 *rom = memregion("maincpu")->base();

	rom[0x4a9] = 0x28;
}

GAME( 1995, poker72,  0,    poker72, poker72, poker72_state,  poker72, ROT0, "Extrema Systems International Ltd.", "Poker Monarch (v2.50)", MACHINE_NOT_WORKING ) // actually unknown, was marked 'slot 72 poker'  Manufacturers logo and 'Lucky Boy' gfx in rom..
