// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************************************

    Cycle Maabou (c) 1984 Taito Corporation / Seta
    Sky Destroyer (c) 1985 Taito Corporation

    appears to be in the exact middle between the gsword / josvolly HW and the ppking / gladiator HW

    driver by Angelo Salese

    TODO:
    - inputs in Cycle Maabou;
    - sound (controlled by three i8741);
    - add flipscreen;
    - color prom resistor network is guessed, cyclemb yellows are more reddish on pcb video and photos;

    BTANB verified on pcb: cyclemb standing cones are reddish-yellow/black instead of red/white

=====================================================================================================

Cycle Mahbou
(c)1984 Taito/Seta

----------------------------------------
Top
P1-002A
----------------------------------------
P0_20.3D     [53e3a36e]
P0_21.3E     [a7dab6d8]


----------------------------------------
Bottom
P0-001A
CPU  :Z80A x2
OSC  :18.000MHz
Other:AP-001,AP-004,AP-005,AP-006,P7,P8,P9
----------------------------------------
P0_1.1A      [a1588264]
P0_2.1B      [04141837]
P0_3.1C      [a9dd4b22]
P0_4.1E      [456a30df]
P0_5.1F      [a3b9c297]
P0_6.1H      [ec76a0a6]
P0_7.1K      [6507d23f]
P0_10.1N     [a98415db]
P0_11.1R     [626556fe]
P0_12.1S     [1e08902c]
P0_13.1T     [086639c1]
P0_14.1U     [3f5fe2b6]

P0_15.10C    [9cc52c32]
P0_16.10D    [8d03227e]

AP-002.7B    [Not Dump] 8741
AP-003.7C    [Not Dump] /

P1.2E        [6297104c] 82S123
P2.4E        [70a09cc5] /

P0_3.11T     [be89c1f7] 82S129
P0_4.11U     [4886d832] /



--- Team Japump!!! ---
Dumped by Chack'n
27/Nov/2009
28/Nov/2009

****************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "machine/tait8741.h"


class cyclemb_state : public driver_device
{
public:
	cyclemb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_obj1_ram(*this, "obj1_ram"),
		m_obj2_ram(*this, "obj2_ram"),
		m_obj3_ram(*this, "obj3_ram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_cram;
	required_shared_ptr<UINT8> m_obj1_ram;
	required_shared_ptr<UINT8> m_obj2_ram;
	required_shared_ptr<UINT8> m_obj3_ram;

	struct
	{
		UINT8 rxd;
		UINT8 txd;
		UINT8 rst;
		UINT8 state;
		UINT8 packet_type;
	} m_mcu[2];

	UINT16 m_dsw_pc_hack;

	DECLARE_WRITE8_MEMBER(cyclemb_bankswitch_w);
//  DECLARE_READ8_MEMBER(mcu_status_r);
//  DECLARE_WRITE8_MEMBER(sound_cmd_w);
	DECLARE_WRITE8_MEMBER(cyclemb_flip_w);
	DECLARE_READ8_MEMBER(skydest_i8741_0_r);
	DECLARE_WRITE8_MEMBER(skydest_i8741_0_w);

	DECLARE_DRIVER_INIT(skydest);
	DECLARE_DRIVER_INIT(cyclemb);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(cyclemb);

	UINT32 screen_update_cyclemb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_skydest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cyclemb_draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cyclemb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void skydest_draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void skydest_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void skydest_i8741_reset();
};



PALETTE_INIT_MEMBER(cyclemb_state, cyclemb)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x100]) | (color_prom[i+0x000]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void cyclemb_state::cyclemb_draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int x,y,count;
	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int attr = m_cram[count];
			int tile = (m_vram[count]) | ((attr & 3)<<8);
			int color = ((attr & 0xf8) >> 3) ^ 0x1f;
			int odd_line = y & 1 ? 0x40 : 0x00;
	//      int sx_offs = flip_screen() ? 512 : 0
			int scrollx = ((m_vram[(y/2)+odd_line]) + (m_cram[(y/2)+odd_line]<<8) + 48) & 0x1ff;

			if(!(attr & 4))
				color += 0x20;//screen.machine().rand();

			if(flip_screen())
			{
				gfx->opaque(bitmap,cliprect,tile,color,1,1,512-(x*8)-scrollx,256-(y*8));
				/* wrap-around */
				gfx->opaque(bitmap,cliprect,tile,color,1,1,512-(x*8)-scrollx+512,256-(y*8));
			}
			else
			{
				gfx->opaque(bitmap,cliprect,tile,color,0,0,(x*8)-scrollx,(y*8));
				/* wrap-around */
				gfx->opaque(bitmap,cliprect,tile,color,0,0,(x*8)-scrollx+512,(y*8));
			}

			count++;
		}
	}
}


	/*
	bank 1
	xxxx xxxx [0] sprite offset
	---x xxxx [1] color offset
	bank 2
	xxxx xxxx [0] y offs
	xxxx xxxx [1] x offs
	bank 3
	---- ---x [1] sprite enable flag?
	*/
void cyclemb_state::cyclemb_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 col,fx,fy,region;
	UINT16 spr_offs,i;
	INT16 x,y;

	/*
	0x3b-0x3c-0x3d tire (0x13 0x00 / 0x17 0x00 )
	0x3b- shirt (0x16 0x00)
	0x20 tire stick (0x16 0x00)
	0x2e go sign (0x11 0x00)
	0x18 trampoline (0x13 0x00)
	0x27 cone (0x13 0x00)
	*/

	for(i=0;i<0x40;i+=2)
	{
		y = 0xf1 - m_obj2_ram[i];
		x = m_obj2_ram[i+1] - 56;
		spr_offs = (m_obj1_ram[i+0]);
		spr_offs += ((m_obj3_ram[i+0] & 3) << 8);
		col = (m_obj1_ram[i+1] & 0x3f);
		region = ((m_obj3_ram[i] & 0x10) >> 4) + 1;
		if(region == 2)
		{
			spr_offs >>= 2;
			//spr_offs += ((m_obj3_ram[i+0] & 3) << 5);
			y-=16;
		}

		if(m_obj3_ram[i+1] & 1)
			x+=256;
		//if(m_obj3_ram[i+1] & 2)
//              x-=256;
		fx = (m_obj3_ram[i+0] & 4) >> 2;
		fy = (m_obj3_ram[i+0] & 8) >> 3;

		if(flip_screen())
		{
			fx = !fx;
			fy = !fy;
		}
		m_gfxdecode->gfx(region)->transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,0);
	}
}


void cyclemb_state::skydest_draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int x,y;


	for (y=0;y<32;y++)
	{
		for (x=2;x<62;x++)
		{
			/* upper bits of the first address of cram seems to be related to colour cycling */

			int attr = m_cram[x+y*64];
			int tile = (m_vram[x+y*64]) | ((attr & 3)<<8);
			int color = ((attr & 0xfc) >> 2);
			int scrollx = m_vram[0*64+0];
			scrollx |= (m_cram[0*64+0] & 0x01)<<8;

			int cycle = (m_cram[0*64+0] & 0xf0)>>4;

			color ^= 0x3f;
			// hardcoded to thie palette bit?
			if (attr & 0x40) color ^= cycle;

			scrollx -= 0xc0;

			int scrolly;
			if (x<32)
				scrolly = m_vram[(x)*64+0];
			else
				scrolly = m_vram[(x-32)*64+1];


			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*8+scrollx,((y*8)-scrolly)&0xff);
			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*8+scrollx-480,((y*8)-scrolly)&0xff);
			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*8+scrollx+480,((y*8)-scrolly)&0xff);


		}
	}
}

/*
    bank 1
    xxxx xxxx [0] sprite offset
    ---x xxxx [1] color offset
    bank 2
    xxxx xxxx [0] y offs
    xxxx xxxx [1] x offs
    bank 3
    ---- ---x [1] sprite enable flag?
*/

void cyclemb_state::skydest_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 col,fx,fy,region;
	UINT16 spr_offs,i;
	INT16 x,y;

//  popmessage("%d %d",m_obj2_ram[0x0d], 0xf1 - m_obj2_ram[0x0c+1] + 68);

	for(i=0;i<0x80;i+=2)
	{
		y = m_obj2_ram[i] - 1;
		x = m_obj2_ram[i+1];

		if(m_obj3_ram[i+1] & 1)
			x |= 0x100;

			x = 0x138 - x;

		spr_offs = (m_obj1_ram[i+0]);
		spr_offs += ((m_obj3_ram[i+0] & 3) << 8);
		col = (m_obj1_ram[i+1] & 0x3f);
		region = ((m_obj3_ram[i] & 0x10) >> 4) + 1;
		if(region == 2)
		{
			spr_offs >>= 2;
		//  spr_offs += ((m_obj3_ram[i+0] & 3) << 5);
			x-=16;
		}


		fx = (m_obj3_ram[i+0] & 4) >> 2;
		fy = (m_obj3_ram[i+0] & 8) >> 3;

		if(flip_screen())
		{
			fx = !fx;
			fy = !fy;
		}
		m_gfxdecode->gfx(region)->transpen(bitmap,cliprect,spr_offs,col,fx,fy,x,y,0);
	}
}

UINT32 cyclemb_state::screen_update_cyclemb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	cyclemb_draw_tilemap(screen,bitmap,cliprect);
	cyclemb_draw_sprites(screen,bitmap,cliprect);
	return 0;
}

UINT32 cyclemb_state::screen_update_skydest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	skydest_draw_tilemap(screen,bitmap,cliprect);
	skydest_draw_sprites(screen,bitmap,cliprect);
	return 0;
}

WRITE8_MEMBER(cyclemb_state::cyclemb_bankswitch_w)
{
	membank("bank1")->set_entry(data & 3);
}

#if 0
WRITE8_MEMBER(cyclemb_state::sound_cmd_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}
#endif

#if 0
READ8_MEMBER(cyclemb_state::mcu_status_r)
{
	return 1;
}


WRITE8_MEMBER(cyclemb_state::sound_cmd_w)//actually ciom
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}
#endif

WRITE8_MEMBER(cyclemb_state::cyclemb_flip_w)
{
	flip_screen_set(data & 1);

	// a bunch of other things are setted here
}

void cyclemb_state::skydest_i8741_reset()
{
	m_mcu[0].rxd = 0;
	m_mcu[0].txd = 0;
	m_mcu[0].rst = 0;
	m_mcu[0].state = 0;
	m_mcu[0].packet_type = 0;
}

READ8_MEMBER( cyclemb_state::skydest_i8741_0_r )
{
	if(offset == 1) //status port
	{
		//printf("STATUS PC=%04x\n",m_maincpu->pc());

		return 1;
	}
	else
	{
		UINT8 i,pt;

		//printf("%04x\n",m_maincpu->pc());

		/* TODO: internal state of this */
		if(m_maincpu->pc() == m_dsw_pc_hack)
			m_mcu[0].rxd = (ioport("DSW1")->read() & 0x1f) << 2;
		else if(m_mcu[0].rst)
		{
			//printf("READ PC=%04x\n",m_maincpu->pc());
			{
				switch(m_mcu[0].state)
				{
					case 1:
					{
						m_mcu[0].packet_type^=0x20;
						if(m_mcu[0].packet_type & 0x20)
							m_mcu[0].rxd = ((ioport("DSW3")->read()) & 0x9f) | (m_mcu[0].packet_type);
						else
							m_mcu[0].rxd = ((ioport("SYSTEM")->read()) & 0x9f) | (m_mcu[0].packet_type);
						break;
					}
					case 2:
					{
						m_mcu[0].packet_type^=0x20;
						if(m_mcu[0].packet_type & 0x20)
						{
							m_mcu[0].rxd = ((ioport("P1_1")->read()) & 0x9f) | (m_mcu[0].packet_type);
						}
						else
						{
							m_mcu[0].rxd = ((ioport("P1_0")->read()) & 0x1f) | (m_mcu[0].packet_type);
							m_mcu[0].rxd |= ((ioport("SYSTEM")->read()) & 0x80);
						}
						break;
					}
					case 3:
					{
						m_mcu[0].packet_type^=0x20;
						if(m_mcu[0].packet_type & 0x20)
							m_mcu[0].rxd = ((ioport("P2_1")->read()) & 0x9f) | (m_mcu[0].packet_type);
						else
						{
							m_mcu[0].rxd = ((ioport("P2_0")->read()) & 0x1f) | (m_mcu[0].packet_type);
							m_mcu[0].rxd |= ((ioport("SYSTEM")->read()) & 0x80);
						}
						break;
					}
					default:
						//printf("%02x\n",m_mcu[0].txd);
						m_mcu[0].rxd = 0x00;
						break;
				}
			}


			for(i=0,pt=0;i<8;i++)
			{
				if(m_mcu[0].rxd & (1 << i))
					pt++;
			}

			if((pt % 2) == 1)
				m_mcu[0].rxd|=0x40;
		}

		return m_mcu[0].rxd;
	}
}

WRITE8_MEMBER( cyclemb_state::skydest_i8741_0_w )
{
	if(offset == 1) //command port
	{
		//printf("%02x CMD PC=%04x\n",data,m_maincpu->pc());
		switch(data)
		{
			case 0:
				m_mcu[0].rxd = 0x40;
				m_mcu[0].rst = 0;
				m_mcu[0].state = 0;
				break;
			case 1:
				/*
				status codes:
				0x06 sub NG IOX2
				0x05 sub NG IOX1
				0x04 sub NG CIOS
				0x03 sub NG OPN
				0x02 sub NG ROM
				0x01 sub NG RAM
				0x00 ok
				*/
				m_mcu[0].rxd = 0x40;
				m_mcu[0].rst = 0;
				break;
			case 2:
				m_mcu[0].rxd = (ioport("DSW2")->read() & 0x1f) << 2;
				m_mcu[0].rst = 0;
				break;
			case 3:
				//m_mcu[0].rxd = (ioport("DSW1")->read() & 0x1f) << 2;
				m_mcu[0].rst = 1;
				m_mcu[0].txd = 0;
				break;
		}
	}
	else
	{
		//printf("%02x DATA PC=%04x\n",data,m_maincpu->pc());

		m_mcu[0].txd = data;

		if(m_mcu[0].txd == 0x41)
			m_mcu[0].state = 1;
		if(m_mcu[0].txd == 0x42)
			m_mcu[0].state = 2;
		if(m_mcu[0].txd == 0x44)
			m_mcu[0].state = 3;
	}
}


static ADDRESS_MAP_START( cyclemb_map, AS_PROGRAM, 8, cyclemb_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_ROMBANK("bank1")
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x9800, 0x9fff) AM_RAM AM_SHARE("cram")
	AM_RANGE(0xa000, 0xa7ff) AM_RAM AM_SHARE("obj1_ram") //ORAM1 (only a000-a3ff tested)
	AM_RANGE(0xa800, 0xafff) AM_RAM AM_SHARE("obj2_ram") //ORAM2 (only a800-abff tested)
	AM_RANGE(0xb000, 0xb7ff) AM_RAM AM_SHARE("obj3_ram") //ORAM3 (only b000-b3ff tested)
	AM_RANGE(0xb800, 0xbfff) AM_RAM //WRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclemb_io, AS_IO, 8, cyclemb_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(cyclemb_bankswitch_w)
	//AM_RANGE(0xc020, 0xc020) AM_WRITENOP // ?
	AM_RANGE(0xc09e, 0xc09f) AM_READWRITE(skydest_i8741_0_r, skydest_i8741_0_w)
	AM_RANGE(0xc0bf, 0xc0bf) AM_WRITE(cyclemb_flip_w) //flip screen
ADDRESS_MAP_END


static ADDRESS_MAP_START( skydest_io, AS_IO, 8, cyclemb_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(cyclemb_bankswitch_w)
	//AM_RANGE(0xc020, 0xc020) AM_WRITENOP // ?
	AM_RANGE(0xc080, 0xc081) AM_READWRITE(skydest_i8741_0_r, skydest_i8741_0_w)
	//AM_RANGE(0xc0a0, 0xc0a0) AM_WRITENOP // ?
	AM_RANGE(0xc0bf, 0xc0bf) AM_WRITE(cyclemb_flip_w) //flip screen
ADDRESS_MAP_END


static ADDRESS_MAP_START( cyclemb_sound_map, AS_PROGRAM, 8, cyclemb_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x63ff) AM_RAM

ADDRESS_MAP_END

static ADDRESS_MAP_START( cyclemb_sound_io, AS_IO, 8, cyclemb_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x40, 0x40) AM_READ(soundlatch_byte_r) AM_WRITE(soundlatch2_byte_w)
ADDRESS_MAP_END


void cyclemb_state::machine_start()
{
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_mcu[i].rxd), i);
		save_item(NAME(m_mcu[i].txd), i);
		save_item(NAME(m_mcu[i].rst), i);
		save_item(NAME(m_mcu[i].state), i);
		save_item(NAME(m_mcu[i].packet_type), i);
	}
}

void cyclemb_state::machine_reset()
{
	skydest_i8741_reset();
}


static INPUT_PORTS_START( cyclemb )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("P1_0")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_1")
	PORT_BIT( 0x9f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "PAD_P1")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PAD_P1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(16) PORT_PLAYER(1)

	PORT_START("P2_0")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_1")
	PORT_BIT( 0x9f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, driver_device,custom_port_read, "PAD_P2")
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PAD_P2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(8) PORT_PLAYER(2)

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
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2" )
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
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x02, 0x00, "Disallow Game Over (Cheat)" )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x0c, 0x00, "Stage Start" )
	PORT_DIPSETTING(    0x00, "Tutorial" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "Bonus Game" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( skydest )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x04, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(2)

	PORT_START("P1_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x04, 0x00, "P1_0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1_1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_DIPNAME( 0x04, 0x00, "P1_1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P2_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x04, 0x00, "P1_0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P2_1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x04, 0x00, "P1_1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility (Cheat)" )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0,1,2,3,64,65,66,67 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	16*8
};

static const gfx_layout spritelayout_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static const gfx_layout spritelayout_32x32 =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3,
			64*8+0, 64*8+1, 64*8+2, 64*8+3, 72*8+0, 72*8+1, 72*8+2, 72*8+3,
			80*8+0, 80*8+1, 80*8+2, 80*8+3, 88*8+0, 88*8+1, 88*8+2, 88*8+3},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8,
			128*8, 129*8, 130*8, 131*8, 132*8, 133*8, 134*8, 135*8,
			160*8, 161*8, 162*8, 163*8, 164*8, 165*8, 166*8, 167*8 },
	64*8*4    /* every sprite takes (64*8=16x6)*4) bytes */
};

static GFXDECODE_START( cyclemb )
	GFXDECODE_ENTRY( "tilemap_data", 0, charlayout,     0, 0x40 )
	GFXDECODE_ENTRY( "sprite_data", 0, spritelayout_16x16,    0x00, 0x40 )
	GFXDECODE_ENTRY( "sprite_data", 0, spritelayout_32x32,    0x00, 0x40 )
GFXDECODE_END

static MACHINE_CONFIG_START( cyclemb, cyclemb_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18MHz/3) // Z8400BPS
	MCFG_CPU_PROGRAM_MAP(cyclemb_map)
	MCFG_CPU_IO_MAP(cyclemb_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cyclemb_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_18MHz/6)
	MCFG_CPU_PROGRAM_MAP(cyclemb_sound_map)
	MCFG_CPU_IO_MAP(cyclemb_sound_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cyclemb_state, screen_update_cyclemb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cyclemb)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(cyclemb_state, cyclemb)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_18MHz/12)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( skydest, cyclemb )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(skydest_io)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 34*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cyclemb_state, screen_update_skydest)

//  MCFG_PALETTE_INIT_OWNER(cyclemb_state,skydest)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cyclemb )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "p0_1.1a",   0x00000, 0x2000, CRC(a1588264) SHA1(ff17df61207e39443a8ea62be1fce102c163d8e1) )
	ROM_LOAD( "p0_2.1b",   0x02000, 0x2000, CRC(04141837) SHA1(18d2f17fd5334b306ca13a1c26780f4a868a4ac8) )
	ROM_LOAD( "p0_3.1c",   0x04000, 0x2000, CRC(a9dd4b22) SHA1(8d3535ecd43aa0eccf3856b7cbad8702d17dd576) )
	ROM_LOAD( "p0_4.1e",   0x06000, 0x2000, CRC(456a30df) SHA1(75594178e6299ef5a81c134138ac1f1231a36caa) )
	ROM_LOAD( "p0_5.1f",   0x10000, 0x2000, CRC(a3b9c297) SHA1(edbab8639cb73e1376306ef70ef4ae451a75e4a9) )
	ROM_LOAD( "p0_6.1h",   0x12000, 0x2000, CRC(ec76a0a6) SHA1(9d1d3c050c76df42da53896f38ae53c5f79b0c5c) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "p0_15.10c",   0x0000, 0x2000, CRC(9cc52c32) SHA1(05d4e7c8ce8fdfc995013c0ed693b4d4778acc25) )
	ROM_LOAD( "p0_16.10d",   0x2000, 0x2000, CRC(8d03227e) SHA1(7e90437cbe5e854025e799348bb2cbca98368bd9) )

	ROM_REGION( 0x4000, "tilemap_data", 0 )
	ROM_LOAD( "p0_21.3e",   0x0000, 0x2000, CRC(a7dab6d8) SHA1(c5802e76abd394a2ce1526815bfbfc12e5e57587) )
	ROM_LOAD( "p0_20.3d",   0x2000, 0x2000, CRC(53e3a36e) SHA1(d95c1dfe216bb8b1f3e14c72a480eb2befa9d1dd) )

	ROM_REGION( 0x10000, "sprite_data", ROMREGION_ERASEFF )
	ROM_LOAD( "p0_7.1k",    0x0000, 0x2000, CRC(6507d23f) SHA1(1640b25a6efa0976f13ed7838f31ef53c37c8d2d) )
	ROM_LOAD( "p0_10.1n",   0x6000, 0x2000, CRC(a98415db) SHA1(218a1d3ad27c30263daf87be87b4d5e06d5ac604) )
	ROM_LOAD( "p0_11.1r",   0x8000, 0x2000, CRC(626556fe) SHA1(ebd08a407fe466af14813bdeeb852d6816da932e) )
	ROM_LOAD( "p0_12.1s",   0xa000, 0x2000, CRC(1e08902c) SHA1(3d5f620580dc1fc43cd5f99b2a1e62a6d749f8b9) )
	ROM_LOAD( "p0_13.1t",   0xc000, 0x2000, CRC(086639c1) SHA1(3afbe76bb466d4c5916ef85d4cfc42e0c3f69883) )
	ROM_LOAD( "p0_14.1u",   0xe000, 0x2000, CRC(3f5fe2b6) SHA1(a7d1d0bc449f557ba827936b0fdbcccf7b1ee629) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "p0_3.11t",   0x0000, 0x100, CRC(be89c1f7) SHA1(7fb2d9fccf6c74130c3e0db4ea4269aeb45359e3) )
	ROM_LOAD( "p0_4.11u",   0x0100, 0x100, CRC(4886d832) SHA1(49e77923b7e2a0d5e9d990706dac258ecfd7720e) )

	ROM_REGION( 0x40, "timing_proms", 0 ) //???
	ROM_LOAD( "p1.2e",      0x000, 0x020, CRC(6297104c) SHA1(f2a40811505625a7a7ef4a7e4168c556c263449b) )
	ROM_LOAD( "p2.4e",      0x020, 0x020, CRC(70a09cc5) SHA1(82c0f3122d2c1e8be74b857737380c2e978adeef) )
ROM_END

ROM_START( skydest )
	ROM_REGION( 0x18000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pd1-1.1a",     0x000000, 0x002000, CRC(78951c75) SHA1(f6d36a1b9b35a346a1e7389956e90332ada07454) )
	ROM_LOAD( "pd0-2.1b",     0x002000, 0x002000, CRC(da2d48cd) SHA1(5f4871f66bca8515505e4ef887cadf41a4e88f4d) )
	ROM_LOAD( "pd0-3.1c",     0x004000, 0x002000, CRC(28ef8eda) SHA1(c2ca346b1170e8ca7239bee4040225c50923e527) )
	ROM_LOAD( "pd1-4.1e",     0x006000, 0x002000, CRC(b8ec9938) SHA1(79f9be7ba74af9542488247c83a4aa731ebb7917) )

	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "pd0-15.10c",   0x000000, 0x002000, CRC(f8b3d3f7) SHA1(447907f982362f5995d7eb646628cf0e07ba8f64) )
	ROM_LOAD( "pd0-16.10d",   0x002000, 0x002000, CRC(19ce8106) SHA1(31186d3b1c0d124da82310930a002a481941ebb1) )

	ROM_REGION( 0x4000, "tilemap_data", ROMREGION_INVERT )
	ROM_LOAD( "pd0-20.1h",    0x000000, 0x004000, CRC(8b2137f2) SHA1(1f83e081cab116c69a8349fd33ba1916b1c91826) ) // on daughterboard

	ROM_REGION( 0x10000, "sprite_data", ROMREGION_ERASEFF )
	ROM_LOAD( "pd0-7.1k",     0x000000, 0x002000, CRC(83137d42) SHA1(7e35f28577d6bfeee184a0ac3095b478999d6477) ) //ok
	ROM_LOAD( "pd1-8.1l",     0x002000, 0x002000, CRC(b810858b) SHA1(385e625fc989a1dfa18559a62c99363b62c66a67) ) //ok
	ROM_LOAD( "pd0-9.1m",     0x004000, 0x002000, CRC(6f558bee) SHA1(0539feaa848d6cfb9f90a46a851f73fb74e82676) ) //ok
	ROM_LOAD( "pd1-10.1n",    0x006000, 0x002000, CRC(5840b5b5) SHA1(1b5b188023c4d3198402c946b8c5a51d7f512a07) )
	ROM_LOAD( "pd0-11.1r",    0x008000, 0x002000, CRC(29e5fce4) SHA1(59748e3a192a45dce7920e8d5a7a11d5145915b0) ) //ok
	ROM_LOAD( "pd0-12.1s",    0x00a000, 0x002000, CRC(06234942) SHA1(1cc40a8c8e24ab6db1dc7dc88979be23b7a9cab6) )
	ROM_LOAD( "pd1-13.1t",    0x00c000, 0x002000, CRC(3cca5b95) SHA1(74baec7c128254c394dd3162df7abacf5ed5a99b) ) //ok
	ROM_LOAD( "pd0-14.1u",    0x00e000, 0x002000, CRC(7ef05b01) SHA1(f36ad1c0dac201729def78dc18feacda8fcf1a3f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "green.11t",    0x000, 0x100, CRC(f803beb7) SHA1(9c979a296de04728d43c94e9e06f8d8600dc9cfb) )
	ROM_LOAD( "red.11u",      0x100, 0x100, CRC(24b7b6f3) SHA1(c2f6477baa5be038c41f5f2ecd16522a6b8d84db) )

	ROM_REGION( 0x40, "timing_proms", 0 ) //???
	ROM_LOAD( "p1.2e",        0x000, 0x020, NO_DUMP )
	ROM_LOAD( "p0.4e",        0x020, 0x020, NO_DUMP )

	ROM_REGION( 0x100, "unk_prom", 0 ) //???
	ROM_LOAD( "blue.4j",      0x000, 0x100, CRC(34579681) SHA1(10e5e137837bdd71959f0c4bf52e0f333630a22f) ) // on daughterboard, _not_ a color prom
ROM_END

DRIVER_INIT_MEMBER(cyclemb_state,cyclemb)
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x1000);
	m_dsw_pc_hack = 0x760;
}

DRIVER_INIT_MEMBER(cyclemb_state,skydest)
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x1000);
	m_dsw_pc_hack = 0x554;
}

GAME( 1984, cyclemb,  0,   cyclemb,  cyclemb, cyclemb_state,  cyclemb, ROT0, "Taito Corporation", "Cycle Maabou (Japan)", MACHINE_NO_COCKTAIL | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, skydest,  0,   skydest,  skydest, cyclemb_state,  skydest, ROT0, "Taito Corporation", "Sky Destroyer (Japan)", MACHINE_NO_COCKTAIL | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
