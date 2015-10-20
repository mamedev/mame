// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/***************************************************************************
Vega by Olympia
---------------

preliminary WIP driver by Tomasz Slanina

hardware seems like an evolution of Olympia's Monza GP

I8035 (MCU)
AY 3-8910 (Sound)
PPI 8255 (I/O)
INS 8154 (I/O + RAM)
DP8350 (CRT controller)

TODO:
- proper scanline (with 2x scaling when needed) based renderer
- color mixer
- controls
- object positions (part of the scanline renderer)

| R | V | B || A | M | U | L ||   R   |   G   |   B   |
-------------------------------------------------------
| - | - | - || - | - | - | 0 || 5.000 | 5.000 | 5.000 |
| - | 0 | 0 || 0 | 0 | 0 | 1 || 0.810 | 0.585 | 0.810 |
| - | 1 | 0 || 0 | 0 | 0 | 1 || 0.816 | 1.440 | 0.816 |
| - | 0 | 1 || 0 | 0 | 0 | 1 || 0.838 | 0.606 | 4.630 |
| - | 1 | 1 || 0 | 0 | 0 | 1 || 0.845 | 1.500 | 4.670 |
| 0 | 0 | 0 || 1 | 0 | 0 | 1 || 0.811 | 0.811 | 0.811 |
| 1 | 0 | 0 || 1 | 0 | 0 | 1 || 4.640 | 0.840 | 0.840 |
| 0 | 1 | 0 || 1 | 0 | 0 | 1 || 0.840 | 4.640 | 0.840 |
| 1 | 1 | 0 || 1 | 0 | 0 | 1 || 4.810 | 4.810 | 0.871 |
| 0 | 0 | 1 || 1 | 0 | 0 | 1 || 0.840 | 0.840 | 4.640 |
| 1 | 0 | 1 || 1 | 0 | 0 | 1 || 4.810 | 0.871 | 4.810 |
| 0 | 1 | 1 || 1 | 0 | 0 | 1 || 0.871 | 4.810 | 4.810 |
| 1 | 1 | 1 || 1 | 0 | 0 | 1 || 5.000 | 5.000 | 5.000 |
| - | 0 | 0 || 0 | 1 | 0 | 1 || 0.583 | 0.489 | 0.807 |
| - | 1 | 0 || 0 | 1 | 0 | 1 || 0.586 | 0.976 | 0.811 |
| - | 0 | 1 || 0 | 1 | 0 | 1 || 0.604 | 0.507 | 4.620 |
| - | 1 | 1 || 0 | 1 | 0 | 1 || 0.607 | 1.010 | 4.640 |
| 0 | 0 | 0 || 1 | 1 | 0 | 1 || 0.584 | 0.639 | 0.808 |
| 1 | 0 | 0 || 1 | 1 | 0 | 1 || 1.440 | 0.644 | 0.815 |
| 0 | 1 | 0 || 1 | 1 | 0 | 1 || 0.590 | 1.830 | 0.817 |
| 1 | 1 | 0 || 1 | 1 | 0 | 1 || 1.460 | 1.840 | 0.824 |
| 0 | 0 | 1 || 1 | 1 | 0 | 1 || 0.605 | 0.661 | 4.620 |
| 1 | 0 | 1 || 1 | 1 | 0 | 1 || 1.490 | 0.667 | 4.660 |
| 0 | 1 | 1 || 1 | 1 | 0 | 1 || 0.612 | 1.900 | 4.680 |
| 1 | 1 | 1 || 1 | 1 | 0 | 1 || 1.510 | 1.910 | 4.720 |
| - | 0 | 0 || 0 | 0 | 1 | 1 || 0.636 | 0.456 | 0.582 |
| - | 1 | 0 || 0 | 0 | 1 | 1 || 0.639 | 0.852 | 0.584 |
| - | 0 | 1 || 0 | 0 | 1 | 1 || 0.642 | 0.459 | 1.440 |
| - | 1 | 1 || 0 | 0 | 1 | 1 || 0.644 | 0.858 | 1.440 |
| 0 | 0 | 0 || 1 | 0 | 1 | 1 || 0.637 | 0.583 | 0.583 |
| 1 | 0 | 0 || 1 | 0 | 1 | 1 || 1.830 | 0.589 | 0.589 |
| 0 | 1 | 0 || 1 | 0 | 1 | 1 || 0.642 | 1.440 | 0.587 |
| 1 | 1 | 0 || 1 | 0 | 1 | 1 || 1.840 | 1.450 | 0.594 |
| 0 | 0 | 1 || 1 | 0 | 1 | 1 || 0.642 | 0.587 | 1.440 |
| 1 | 0 | 1 || 1 | 0 | 1 | 1 || 1.840 | 0.594 | 1.450 |
| 0 | 1 | 1 || 1 | 0 | 1 | 1 || 0.648 | 1.450 | 1.450 |
| 1 | 1 | 1 || 1 | 0 | 1 | 1 || 1.860 | 1.470 | 1.470 |
| - | 0 | 0 || 0 | 1 | 1 | 1 || 0.487 | 0.395 | 0.581 |
| - | 1 | 0 || 0 | 1 | 1 | 1 || 0.489 | 0.663 | 0.582 |
| - | 0 | 1 || 0 | 1 | 1 | 1 || 0.491 | 0.398 | 1.430 |
| - | 1 | 1 || 0 | 1 | 1 | 1 || 0.492 | 0.669 | 1.440 |
| 0 | 0 | 0 || 1 | 1 | 1 | 1 || 0.488 | 0.488 | 0.581 |
| 1 | 0 | 0 || 1 | 1 | 1 | 1 || 0.973 | 0.490 | 0.584 |
| 0 | 1 | 0 || 1 | 1 | 1 | 1 || 0.490 | 0.973 | 0.584 |
| 1 | 1 | 0 || 1 | 1 | 1 | 1 || 0.977 | 0.977 | 0.587 |
| 0 | 0 | 1 || 1 | 1 | 1 | 1 || 0.492 | 0.492 | 1.440 |
| 1 | 0 | 1 || 1 | 1 | 1 | 1 || 0.981 | 0.494 | 1.440 |
| 0 | 1 | 1 || 1 | 1 | 1 | 1 || 0.494 | 0.981 | 1.440 |
| 1 | 1 | 1 || 1 | 1 | 1 | 1 || 0.985 | 0.985 | 1.450 |

***************************************************************************/


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8255.h"
#include "machine/ins8154.h"
#include "sound/ay8910.h"


struct vega_obj
{
	int m_x, m_y, m_enable, m_type;
};

enum
{
	OBJ_0,
	OBJ_1,
	OBJ_2,
	OBJ_PLAYER,

	NUM_OBJ
};

class vega_state : public driver_device
{
public:

	vega_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_i8255(*this, "ppi8255"),
	m_ins8154(*this, "ins8154"),
	m_ay8910(*this, "ay8910"),
	m_gfxdecode(*this, "gfxdecode"),
	m_palette(*this, "palette") {}

	required_device<cpu_device>     m_maincpu;
	required_device<i8255_device>   m_i8255;
	required_device<ins8154_device> m_ins8154;
	required_device<ay8910_device>  m_ay8910;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_p2_data;
	int m_ext_offset_w;
	int m_ext_offset_r;

	int m_tmp;
	int m_t1;

	UINT8 m_ins8154_ram[0x80];
	UINT8 m_txt_ram[0x400];

	vega_obj    m_obj[NUM_OBJ];

	int m_frame_counter;

	int m_tilemap_offset_x, m_tilemap_offset_y, m_tilemap_flags, m_tilemap_top;

	DECLARE_READ8_MEMBER( extern_r );
	DECLARE_WRITE8_MEMBER(extern_w);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_WRITE8_MEMBER(rombank_w);

	DECLARE_READ8_MEMBER(txtram_r);
	DECLARE_READ8_MEMBER(randomizer);
	DECLARE_WRITE8_MEMBER(txtram_w);
	DECLARE_WRITE8_MEMBER(ppi_pb_w);
	DECLARE_WRITE8_MEMBER(ppi_pc_w);

	DECLARE_READ8_MEMBER(ins8154_pa_r);
	DECLARE_WRITE8_MEMBER(ins8154_pa_w);
	DECLARE_READ8_MEMBER(ins8154_pb_r);
	DECLARE_WRITE8_MEMBER(ins8154_pb_w);

	DECLARE_READ8_MEMBER(ay8910_pa_r);
	DECLARE_WRITE8_MEMBER(ay8910_pa_w);
	DECLARE_READ8_MEMBER(ay8910_pb_r);
	DECLARE_WRITE8_MEMBER(ay8910_pb_w);

	DECLARE_DRIVER_INIT(vega);


	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(vega);
	void draw_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	UINT32 screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

WRITE8_MEMBER(vega_state::extern_w)
{
	m_ext_offset_w=offset;

	switch((m_p2_data>>2)&7) /* 7442 = lines 2,3,4 - select device */
	{
		case 0:  /* 00-03 */
		{
			/* PPI 8255 /CS */
			m_i8255->write(space, (m_p2_data>>6)&3, data);
		}
		break;

		case 1: /* 04-07 */
		{
			/* AY 3-8910 */
			m_ay8910->address_w(space, 0, offset);
		}
		break;

		case 2: /* 08-0b */
		{
			/* INS 8154  /CS0 */

			if(m_p2_data&0x40) /* P26 connected to M/IO pin */
			{
					m_ins8154_ram[offset&0x7f]=data;
			}
			else
			{
				//register w ?
				m_ins8154->ins8154_w(space,offset&0x7f,data);
			}
		}
		break;

		case 3: /* 0c-0f */
		{
			if(offset&4)
			{
				int num=0; //?

				switch(offset&3)
				{
					case 0:
						m_obj[num].m_x=(m_obj[num].m_x&0x80)|(data>>1); //?
						m_obj[num].m_enable=data&1;
					break;

					case 1:
						m_obj[num].m_x=(m_obj[num].m_x&0x7f)|((data&1)<<7);
					break;

					case 2: m_obj[num].m_y=data; break;
					case 3: m_obj[num].m_type=data&0x0f; break;
				}
			}
			else
			{
					// 0 - y ?
				switch(offset&3)
				{
					case 0:
						m_obj[OBJ_PLAYER].m_y=data;
					break;

					case 1:
						m_tilemap_offset_y=data;
					break;

					case 2:
						m_tilemap_offset_x=((m_tilemap_offset_x)&(~0xff))|data;
					break;

					case 3:
						m_tilemap_top=data&0x0f;
						m_tilemap_flags=data>>4;
					break;
				}

			}

		}
		break;

		case 4: /* 10-13 */
		{
			int num=(offset&4)?1:2;

			switch(offset&3)
			{
				case 0:
					m_obj[num].m_x=(m_obj[num].m_x&0x80)|(data>>1);
					m_obj[num].m_enable=data&1;
				break;

				case 1:
					m_obj[num].m_x=(m_obj[num].m_x&0x7f)|((data&1)<<7);
				break;

				case 2: m_obj[num].m_y=data; break;
				case 3: m_obj[num].m_type=data&0x0f; break;

			}

		}
		break;

	//  case 5: /* 14-17 */
	//  {
	//
	//  }
	//  break;
	//
	//  case 6: /* 18-1b */
	//  {
	//
	//  }
	//  break;
	//
	//  case 7: /* 1c-1f */
	//  {
	//
	//  }
	//  break;

	default: logerror("unknown w %x %x %x\n",m_p2_data,offset, data);

	}
}
READ8_MEMBER(vega_state::extern_r)
{
	m_ext_offset_r=offset;

	switch((m_p2_data>>2)&7)
	{
		case 0: /* PPI 8255 /CS */
		{
			return m_i8255->read( space, m_p2_data>>6); /* A6,A7 -> A0,A1 */
		}

		case 1: /* 04-07 */
		{
			/* AY 3-8910 */
			m_ay8910->data_w(space, 0, offset);
			return 0xff;//mame_rand(space.machine);
		}

		case 2: /* 08-0b */
		{
			/* INS 8154  /CS0 */

			if(m_p2_data&0x40) /* P26 connected to M/IO pin */
			{
				return m_ins8154_ram[offset&0x7f];
			}
			else
			{
				//register r ?
				return m_ins8154->ins8154_r(space,offset&0x7f);
			}
		}
#if 0
		case 3: /* 0c-0f */
		{
		}
		break;

		case 4: /* 10-13 */
		{
		}
		break;

		case 5: /* 14-17 */
		{
		}
		break;

		case 6: /* 18-1b */
		{
		}
		break;

		case 7: /* 1c-1f */
		{
		}
		break;
#endif
		default: logerror("unknown r %x %x\n",m_p2_data,offset);
	}

	return 0;
}

READ8_MEMBER(vega_state::p2_r)
{
	return m_p2_data;
}

WRITE8_MEMBER(vega_state::p2_w)
{
	m_p2_data=data;
}

READ8_MEMBER(vega_state::t1_r)
{
	return machine().rand();
}

WRITE8_MEMBER(vega_state::rombank_w)
{
	/* bit 7 used to select ROM bank (other bits = DSW ) */
	membank("bank1")->set_entry(data >>7);
}

static ADDRESS_MAP_START( vega_map, AS_PROGRAM, 8, vega_state )
	AM_RANGE(0x000, 0x7ff) AM_ROMBANK("bank1")
	AM_RANGE(0x800, 0xfff) AM_ROM
ADDRESS_MAP_END

/*
MCS48_PORT_P0   = 0x100,
    MCS48_PORT_P1   = 0x101,
    MCS48_PORT_P2   = 0x102,
    MCS48_PORT_T0   = 0x110,
    MCS48_PORT_T1   = 0x111,
    MCS48_PORT_BUS  = 0x120,
    MCS48_PORT_PROG = 0x121      0/1
    */
static ADDRESS_MAP_START( vega_io_map, AS_IO, 8, vega_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(extern_r, extern_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ_PORT("DSW") AM_WRITE(rombank_w) //101
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(p2_r, p2_w)//102
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r) //111
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_WRITENOP /* prog - inputs CLK */
ADDRESS_MAP_END


static INPUT_PORTS_START( vega )
	PORT_START("DSW") /* connected directly to CPU  P1 (except for bit 7, used to bankswitch ROM) */
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )

	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_3C ) )

	PORT_DIPNAME( 0x20, 0x20, "Speed" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_DIPNAME( 0x40, 0x40, "Ext Play at" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "3.000" )
	PORT_DIPSETTING(    0x00, "4.000" )

	PORT_DIPNAME( 0x80, 0x80, "Bomb Speed" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )


	//PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) /* output */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )


	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,IPT_COIN2 )

	PORT_DIPNAME( 0x08, 0x00, "1-3" )  //unused ?
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x00, "1-4" ) //unused ?
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x20, 0x00, "1-5" ) //unused ?
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x00, "1-6" ) //some video status ?
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x00, "1-7" ) //some video status ?
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END



PALETTE_INIT_MEMBER(vega_state, vega)
{
	int i;
	for(i=0;i<8;++i)
	{
		palette.set_pen_color( 2*i, rgb_t(0x00, 0x00, 0x00) );
		palette.set_pen_color( 2*i+1, rgb_t( (i&1)?0xff:0x00, (i&2)?0xff:0x00, (i&4)?0xff:0x00) );
	}
}


void vega_state::draw_tilemap(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	{
	UINT8 *map_lookup = memregion("tilemaps")->base();

	int offset_y=m_tilemap_offset_y;
	int offset_x=m_tilemap_offset_x;

	//logerror("%d  %d\n",offset_x, offset_y);

	for(int xx=0;xx<128;++xx)
	{
		for(int yy=0;yy<8;yy++)
		{
			int x0=xx*32;
			int y0=yy*32;


			int id=map_lookup[((yy+xx*8)+  ((m_tilemap_flags&2)?0x400:0) + (m_tilemap_top<<6 )  )  &0x7ff];

			int flip=BIT(id,5);


			int num=(BITSWAP8( ((id>>2) &7),   7,6,5,4,3,0,1,2 ));

			int bank=id&3;

			if(bank!=3)
			{
				num+=bank*8;

			num*=8*4;


			for(int x=0;x<8;++x)
				for(int y=0;y<4;++y)
				{
					//for(int x=0;x<4;++x)
					{
						m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, num, 0, 1,flip?1:0,  x*4+x0-offset_x, (flip?(3-y):y)*8+y0-offset_y, 0);
						++num;
					}
				}
			}
		}
	}


	}




}

UINT32 vega_state::screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	++m_frame_counter;

	bitmap.fill(0, cliprect);

	draw_tilemap(screen, bitmap, cliprect);


	{
		int x,y;
		int idx=0;
		UINT8 *color_lookup = memregion("proms")->base() + 0x200;

		for(y=0;y<25;++y)
			for(x=0;x<40;++x)
			{
				int character=m_txt_ram[idx];
				//int color=BITSWAP8(color_lookup[character],7,6,5,4,0,1,2,3)>>1;
				int color=color_lookup[character]&0xf;
				/*
				 bit 0 - unknown
				 bit 1 - blue
				 bit 2 - green
				 bit 3 - red
				 */

					color=BITSWAP8(color,7,6,5,4,0,1,2,3)&0x7;

				color^=0xf;

			//  if(color==0) color=0xf;

					m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, character, color, 0, 0, x*7, y*10,0);

				++idx;
			}

	}

	{
		for(int i=OBJ_0;i<OBJ_PLAYER;++i)
		{
			int x0=255-m_obj[i].m_x;
			int y0=255-m_obj[i].m_y;
			int num=m_obj[i].m_type&7;
			int flip=m_obj[i].m_type&8;

			num*=4*8;
			for(int x=0;x<8;++x)
			for(int y=0;y<4;++y)
			{
				//for(int x=0;x<4;++x)
				{
						m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, num, 0, 1, flip?1:0, x*4+x0, (flip?(3-y):y)*8+y0, 0);
					++num;
				}
			}
		}
	}

/*

    64 strips

    x - -  x x x



*/


	{
		if(BIT(m_obj[OBJ_PLAYER].m_type,5))
		{
			int x0=m_obj[OBJ_PLAYER].m_x;
			int y0=255-m_obj[OBJ_PLAYER].m_y-32;

			UINT8 *sprite_lookup = memregion("proms")->base();


			for(int x=0;x<16;++x)
			{
				int prom_data=sprite_lookup[ ((m_obj[OBJ_PLAYER].m_type&0xf)<<2)|((x>>2)&3)|(((m_frame_counter>>1)&3)<<6) ];

				int xor_line=( ! (( ! ((BIT(prom_data,1))&(BIT(prom_data,2))&(BIT(prom_data,3))&(BIT(x,2)) ) ) &
								( (BIT(prom_data,2)) | (BIT(prom_data,3)) | ( BIT(m_obj[OBJ_PLAYER].m_type,4)) ) ));

				int strip_num=((prom_data)&0x7)|(   ((x&3)^(xor_line?0x3:0))  <<3)|((BIT(prom_data,3))<<5);

				strip_num<<=2;

				for(int y=0;y<4;++y)
				{
						m_gfxdecode->gfx(3)->transpen(bitmap,cliprect, strip_num, 0, !xor_line, 0, x*4+x0, y*8+y0, 0);
					++strip_num;
				}
			}
		}
	}

	return 0;
}

static const gfx_layout text_charlayout =
{
	7,10,        /* 8 x 8 characters */
	RGN_FRAC(1,1),        /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },      /* one bitplane */
	/* x offsets */
	{ 1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8, 8*8, 4*8, 12*8, 2*8, 10*8, 6*8, 14*8, 1*8, 9*8 },
	8*16
};

static const gfx_layout tile_layout2 =
{
	4,8,
	RGN_FRAC(1,1),
	2, /* 2 bit per pixel */
	{ 0,4 },
	{ 3,2,1,0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static const gfx_layout tile_layoutzoom =
{
	4,16,
	RGN_FRAC(1,1),
	2, /* 2 bit per pixel */
	{ 0,4 },
	{ 3,2,1,0 },
	{ 0*8, 0*8, 1*8, 1*8, 2*8, 2*8, 3*8, 3*8 ,
		4*8, 4*8, 5*8, 5*8, 6*8, 6*8, 7*8, 7*8 },
	8*8
};

static const gfx_layout tile_layout3 =
{
	4,8,
	RGN_FRAC(1,2),
	4, /* 4 bits per pixel */
	{ 0,4,0+RGN_FRAC(1,2),4+RGN_FRAC(1,2) },
	{ 3,2,1,0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


static GFXDECODE_START( test_decode )
	GFXDECODE_ENTRY( "gfx1", 0,  text_charlayout, 0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0,  tile_layout2, 16, 1 )
	GFXDECODE_ENTRY( "gfx3", 0,  tile_layout3, 16, 1 )
	GFXDECODE_ENTRY( "gfx4", 0,  tile_layout3, 16, 1 )
	GFXDECODE_ENTRY( "gfx2", 0,  tile_layoutzoom, 16, 1 )
GFXDECODE_END

void vega_state::machine_reset()
{
	membank("bank1")->set_entry(1);
}

WRITE8_MEMBER(vega_state::txtram_w)
{
	m_txt_ram[m_ext_offset_w+((m_p2_data&3)<<8)]=data;
}

READ8_MEMBER(vega_state::txtram_r)
{
	return m_txt_ram[m_ext_offset_r+((m_p2_data&3)<<8)];
}


WRITE8_MEMBER( vega_state::ay8910_pa_w )
{
	m_tmp=0;
	logerror("AY PA W %x\n",data);
}

WRITE8_MEMBER( vega_state::ay8910_pb_w )
{
	m_t1=(data&4)?1:0;
	logerror("AY PB W %x\n",data);
}

READ8_MEMBER( vega_state::ay8910_pa_r )
{
	//m_tmp=0;
	logerror("AY PA R\n");
	return 0;
}

READ8_MEMBER( vega_state::ay8910_pb_r )
{
	m_tmp=0;
	logerror("AY PB R\n");
	return 0;
}

WRITE8_MEMBER(vega_state::ppi_pb_w )
{
	m_tmp=0;
}

WRITE8_MEMBER(vega_state::ppi_pc_w )
{
	m_tmp=0;
	logerror("ppi pc w %x\n",data);
}


WRITE8_MEMBER( vega_state::ins8154_pa_w )
{
	m_obj[OBJ_PLAYER].m_x=data;
}

WRITE8_MEMBER( vega_state::ins8154_pb_w )
{
	m_obj[OBJ_PLAYER].m_type=data;
	m_obj[OBJ_PLAYER].m_enable=data&0x20;
	//logerror("INS PB W %x\n",data);
}

READ8_MEMBER( vega_state::ins8154_pa_r )
{
	m_tmp=0;
	logerror("INS PA R\n");
	return 0;
}

READ8_MEMBER( vega_state::ins8154_pb_r )
{
	logerror("INS PB R\n");
	return 0;
}


READ8_MEMBER(vega_state::randomizer )
{
	return (ioport("IN1")->read()&7)|(machine().rand()&(~7));
}

void vega_state::machine_start()
{
}


static MACHINE_CONFIG_START( vega, vega_state )


	MCFG_CPU_ADD("maincpu", I8035, 4000000)
	MCFG_CPU_PROGRAM_MAP(vega_map)
	MCFG_CPU_IO_MAP(vega_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", vega_state, irq0_line_hold)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(vega_state, txtram_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(vega_state, txtram_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN0"))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(vega_state, ppi_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(vega_state, randomizer))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(vega_state, ppi_pc_w))

	MCFG_DEVICE_ADD( "ins8154", INS8154, 0 )
	MCFG_INS8154_IN_A_CB(READ8(vega_state, ins8154_pa_r))
	MCFG_INS8154_OUT_A_CB(WRITE8(vega_state, ins8154_pa_w))
	MCFG_INS8154_IN_B_CB(READ8(vega_state, ins8154_pb_r))
	MCFG_INS8154_OUT_B_CB(WRITE8(vega_state, ins8154_pb_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 280, 0*8, 239)
	MCFG_SCREEN_UPDATE_DRIVER(vega_state, screen_update_vega)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(vega_state, vega)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", test_decode)

	/* sound hardware */

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay8910", AY8910, 1500000 )
	MCFG_AY8910_PORT_A_READ_CB(READ8(vega_state, ay8910_pa_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(vega_state, ay8910_pb_r))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(vega_state, ay8910_pa_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(vega_state, ay8910_pb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END

ROM_START( vega )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom9.bin",         0x0800, 0x0800, CRC(191c73cd) SHA1(17b1c3790f82b276e55d25ea8a38a3c9cf20bf12) )
	ROM_LOAD( "rom10a.bin",       0x1000, 0x1000, CRC(fca9a570) SHA1(598772db11b32518ed6bf5155a19f4f1761a4831) )

	ROM_REGION( 0x01000, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "rom8.bin",         0x0000, 0x0800, CRC(ccb8598c) SHA1(8c4a702f0653bb189db7d8ac4c2a06aacecc0de0) )
	ROM_LOAD( "rom7.bin",         0x0800, 0x0800, CRC(1de564cd) SHA1(7408cd29f1afc111aa695ecb00160d8f7fba7532) )

	ROM_REGION( 0x01800, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "rom2.bin",         0x0000, 0x0800, CRC(718da952) SHA1(1a0023be1ee3a48ed3ddb8daddbb49ca3f442d46) )
	ROM_LOAD( "rom3.bin",         0x0800, 0x0800, CRC(37944311) SHA1(8b20be3d3ca5cb27bef78a73ee7e977fdf76c7f1) )
	ROM_LOAD( "rom4.bin",         0x1000, 0x0800, CRC(09453d7a) SHA1(75fe96ae25467f82c0725834c6c04a197f50cce7) )

	ROM_REGION( 0x01000, "gfx3", ROMREGION_INVERT  )
	ROM_LOAD( "rom5.bin",         0x0000, 0x0800, CRC(be3df449) SHA1(acba1e07bdf9c0e971f47f2433d2760472c4326a) )
	ROM_LOAD( "rom6.bin",         0x0800, 0x0800, CRC(dc46527c) SHA1(d10a54d8d3ce9ffd8a53bede3d089625aff445a2) )

	ROM_REGION( 0x01000, "gfx4", ROMREGION_INVERT  )
	ROM_LOAD( "rom11.bin",        0x0000, 0x0800, CRC(d1896f77) SHA1(5b80bf7aa81508edfae4fa583b4b0077575a300c) )
	ROM_LOAD( "rom12.bin",        0x0800, 0x0800, CRC(f5f1df2f) SHA1(5851b468702e5e4f085b64afbe7d8b797bb109b5) )

	ROM_REGION( 0x10000, "proms", 0 )
	ROM_LOAD( "r8.bin",  0x0000, 0x0100, CRC(40c9caad) SHA1(ddd427ff4df4cb2d217690efefdd5e53e3add118) ) // FIXED BITS (0000xxxx)
	ROM_LOAD( "r9.bin",  0x0100, 0x0100, CRC(db0bcea5) SHA1(692bea2d9e28985fe7270a940e9f48ac64bdeaa8) ) // FIXED BITS (0000xxxx)
	ROM_LOAD( "r10.bin", 0x0200, 0x0100, CRC(ca5a3627) SHA1(8c632fa9174e336c588074f92f3519b0cf224852) ) // FIXED BITS (0000xxxx) - txt layer lookup table
	ROM_LOAD( "r11.bin", 0x0300, 0x0100, CRC(d8aab14a) SHA1(798feaa929dd7b71266220b568826997acd2a93e) ) // FIXED BITS (000011xx) - RNG? not used

	ROM_REGION( 0x800, "tilemaps", 0 )
	ROM_LOAD( "rom1.bin",         0x0000, 0x0800, CRC(a0c0e0af) SHA1(7ccbfe3c23cda4c3a639c89ff4b2f554e2876c98) ) // FIXED BITS (00xxxxxx) (tile attribs?)

ROM_END


DRIVER_INIT_MEMBER(vega_state, vega)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0x1000], 0x800);
}

GAME( 1982, vega,   0, vega, vega, vega_state, vega, ROT270, "Olympia", "Vega", MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS )
