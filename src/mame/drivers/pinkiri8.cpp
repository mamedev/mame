// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

Janshi / Pinkiri 8 / Ron Jan
 - Wing / Eagle Mahjong board using HD647180X0P6 with internal ROM


 Todo:
 - background tilemap is not fully understood, we lack evidence to support
   it properly, all the games here just do a solid fill of one tile!

 - sprite heights?!

============================================================================
Janshi
(c)1992 Eagle

CPU: HD647180X0P6 (16K EPROM internal rom)
Sound: AY-3-8910, M6295
Others: Battery
OSC: 32MHz, 21MHz & 12.2880MHz

ROMs:
1.1A         [92b140a5]
2.1B         [6de7e086]
3.1D         [4e94d8f2]
4.1F         [a5f6e3ef]
5.1H         [ff2cc769]
6.1K         [8197034d]
11.1L        [a7692ddf]



--- Team Japump!!! ---
Dumped by Chackn
04/May/2007

***************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "sound/okim6295.h"


class pinkiri8_state : public driver_device
{
public:
	pinkiri8_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_janshi_back_vram(*this, "janshivdp:back_vram"),
		m_janshi_vram1(*this, "janshivdp:vram1"),
		m_janshi_unk1(*this, "janshivdp:unk1"),
		m_janshi_widthflags(*this, "janshivdp:widthflags"),
		m_janshi_unk2(*this, "janshivdp:unk2"),
		m_janshi_vram2(*this, "janshivdp:vram2"),
		m_janshi_paletteram(*this, "janshivdp:paletteram"),
		m_janshi_paletteram2(*this, "janshivdp:paletteram2"),
		m_janshi_crtc_regs(*this, "janshivdp:crtc_regs"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_janshi_back_vram;
	required_shared_ptr<UINT8> m_janshi_vram1;
	required_shared_ptr<UINT8> m_janshi_unk1;
	required_shared_ptr<UINT8> m_janshi_widthflags;
	required_shared_ptr<UINT8> m_janshi_unk2;
	required_shared_ptr<UINT8> m_janshi_vram2;
	required_shared_ptr<UINT8> m_janshi_paletteram;
	required_shared_ptr<UINT8> m_janshi_paletteram2;
	required_shared_ptr<UINT8> m_janshi_crtc_regs;
	UINT32 m_vram_addr;
	int m_prev_writes;
	UINT8 m_mux_data;
	UINT8 m_prot_read_index;
	UINT8 m_prot_char[5];
	UINT8 m_prot_index;
	DECLARE_WRITE8_MEMBER(output_regs_w);
	DECLARE_WRITE8_MEMBER(pinkiri8_vram_w);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(mux_p2_r);
	DECLARE_READ8_MEMBER(mux_p1_r);
	DECLARE_READ8_MEMBER(ronjan_prot_r);
	DECLARE_WRITE8_MEMBER(ronjan_prot_w);
	DECLARE_READ8_MEMBER(ronjan_prot_status_r);
	DECLARE_READ8_MEMBER(ronjan_patched_prot_r);
	DECLARE_DRIVER_INIT(ronjan);
	virtual void video_start() override;
	UINT32 screen_update_pinkiri8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};



/* VDP device to give us our own memory map */
class janshi_vdp_device : public device_t,
							public device_memory_interface
{
public:
	janshi_vdp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	DECLARE_ADDRESS_MAP(map, 8);
protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	address_space_config        m_space_config;
};


DEVICE_ADDRESS_MAP_START( map, 8, janshi_vdp_device )
	AM_RANGE(0xfc0000, 0xfc1fff) AM_RAM AM_SHARE("back_vram") // bg tilemap?
	AM_RANGE(0xfc2000, 0xfc2fff) AM_RAM AM_SHARE("vram1") // xpos, colour, tile number etc.

	AM_RANGE(0xfc3700, 0xfc377f) AM_RAM AM_SHARE("unk1") // ?? height related?
	AM_RANGE(0xfc3780, 0xfc37bf) AM_RAM AM_SHARE("widthflags")
	AM_RANGE(0xfc37c0, 0xfc37ff) AM_RAM AM_SHARE("unk2") // 2x increasing tables 00 10 20 30 etc.

	AM_RANGE(0xfc3800, 0xfc3fff) AM_RAM AM_SHARE("vram2") // y pos + unknown

	AM_RANGE(0xff0000, 0xff07ff) AM_RAM /*AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_lo_w)*/ AM_SHARE("paletteram")
	AM_RANGE(0xff2000, 0xff27ff) AM_RAM /*AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_byte_split_hi_w)*/ AM_SHARE("paletteram2")

	AM_RANGE(0xff6000, 0xff601f) AM_RAM AM_SHARE("crtc_regs")
ADDRESS_MAP_END

const device_type JANSHIVDP = &device_creator<janshi_vdp_device>;

janshi_vdp_device::janshi_vdp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, JANSHIVDP, "Janshi VDP", tag, owner, clock, "janshi_vdp", __FILE__),
		device_memory_interface(mconfig, *this),
		m_space_config("janshi_vdp", ENDIANNESS_LITTLE, 8,24, 0, address_map_delegate(FUNC(janshi_vdp_device::map), this))
{
}

void janshi_vdp_device::device_config_complete(){}
void janshi_vdp_device::device_validity_check(validity_checker &valid) const {}
void janshi_vdp_device::device_start() {}
void janshi_vdp_device::device_reset() {}

const address_space_config *janshi_vdp_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}

void pinkiri8_state::video_start() {}


void pinkiri8_state::draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	/* FIXME: color is a bit of a mystery */
	{
		int x, y, col, tile, count, attr;

		count = 0;

		for (y = 0; y < 64; y++)
		{
			for (x = 0; x < 32; x++)
			{
				tile = m_janshi_back_vram[count + 1] << 8 | m_janshi_back_vram[count + 0];
				attr = m_janshi_back_vram[count + 2] ^ 0xf0;
				col = (attr >> 4) | 0x10;

					gfx->transpen(bitmap,cliprect, tile, col, 0, 0, x * 16, y * 8, 0);

				count += 4;
			}
		}
	}
}

void pinkiri8_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int MACHINE_TYPE_hack = 0;
	int col_bank;
	gfx_element *gfx = m_gfxdecode->gfx(0);

	if (!strcmp(machine().system().name,"janshi")) MACHINE_TYPE_hack = 1;

	//popmessage("%02x",m_janshi_crtc_regs[0x0a]);
	col_bank = (m_janshi_crtc_regs[0x0a] & 0x40) >> 6;


	int x,y; //,unk2;
	int col;

	int spr_offs,i;

	int width, height;



	for(i=(0x1000/4)-4;i>=0;i--)
	{
	/*  "vram1" (video map 0xfc2000)

	    tttt tttt | 00tt tttt | cccc c000 | xxxx xxxx |

	    "vram2" (video map 0xfc3800)

	    yyyy yyyy | ???? ???? |


	    widths come from "widthflags" (0xfc3780)
	    "unk1" (0xfc3700) and "unk2" (0xfc37c0) are a mystery

	    */

		spr_offs = ((m_janshi_vram1[(i*4)+0] & 0xff) | (m_janshi_vram1[(i*4)+1]<<8)) & 0xffff;
		col = (m_janshi_vram1[(i*4)+2] & 0xf8) >> 3;
		x =   m_janshi_vram1[(i*4)+3] * 2;

//          unk2 = m_janshi_vram2[(i*2)+1];
		y = (m_janshi_vram2[(i*2)+0]);

		y = 0x100-y;

		col|= col_bank<<5;

	//  width = 0; height = 0;

		width = 2;
		height = 2;


		// this bit determines the sprite width, one bit is used in each word, each bit is used for a range of sprites
		int bit = m_janshi_widthflags[(i/0x20)*2 + 1];

		if (bit)
		{
			//col = machine().rand();
			width = 2;
		}
		else
		{
			width = 1;
			height = 2;
		}

		// hacks!
		if (MACHINE_TYPE_hack==1) // janshi
		{
			if (spr_offs<0x400)
			{
				height = 4;
			}
			else if (spr_offs<0x580)
			{
			//  height = 2;
			}
			else if (spr_offs<0x880)
			{
				height = 4;
			}
			else if (spr_offs<0x1000)
			{
			//  height = 2;
			}
			else if (spr_offs<0x1080)
			{
			//  height = 2;
			}
			else if (spr_offs<0x1700)
			{
				height = 4;
			}
			else if (spr_offs<0x1730)
			{
			//  height = 2;
			}
			else if (spr_offs<0x1930)
			{
				height = 4;
			}
			else if (spr_offs<0x19c0)
			{
				height = 1;
			}
			else
			{
				height = 4;
			}


		}






		if (height==1)
			y+=16;


		// hmm...
		if (height==2)
			y+=16;



		{
			int count = 0;


			for (int yy=0;yy<height;yy++)
			{
				for (int xx=0;xx<width;xx++)
				{
					gfx->transpen(bitmap,cliprect,spr_offs+count,col,0,0,(x+xx*16) -7 ,(y+yy*8)-33,0);
					count++;
				}
			}
		}
	}
}

UINT32 pinkiri8_state::screen_update_pinkiri8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* update palette */
	for (int pen = 0; pen < 0x800 ; pen++)
	{
		UINT16 val = (m_janshi_paletteram[pen]) | (m_janshi_paletteram2[pen]<<8);
		int r = (val & 0x001f) >> 0;
		int g = (val & 0x03e0) >> 5;
		int b = (val & 0x7c00) >> 10;
		m_palette->set_pen_color(pen, pal5bit(r), pal5bit(g), pal5bit(b));
	}



#if 0
	if ( machine().input().code_pressed_once(KEYCODE_W) )
	{
		int i;
		int count2;
		printf("-------------------------------\n");
		count2=0;
		for (i=0x00;i<0x40;i+=2)
		{
			printf("%02x, ", m_janshi_widthflags[i+1]);

			count2++;

			if (count2==0x10)
			{
				printf("\n");
				count2 = 0;
			}
		}
	}
#endif


	bitmap.fill(m_palette->black_pen(), cliprect);

	draw_background(bitmap, cliprect);

	draw_sprites(bitmap, cliprect);

	return 0;
}

static ADDRESS_MAP_START( pinkiri8_map, AS_PROGRAM, 8, pinkiri8_state )
	AM_RANGE(0x00000, 0x0bfff) AM_ROM
	AM_RANGE(0x0c000, 0x0dfff) AM_RAM
	AM_RANGE(0x0e000, 0x0ffff) AM_ROM
	AM_RANGE(0x10000, 0x1ffff) AM_ROM
ADDRESS_MAP_END

WRITE8_MEMBER(pinkiri8_state::output_regs_w)
{
	if(data & 0x40)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	//data & 0x80 is probably NMI mask
}


#define LOG_VRAM 0

WRITE8_MEMBER(pinkiri8_state::pinkiri8_vram_w)
{
	switch(offset)
	{
		case 0:
			m_vram_addr = (data << 0)  | (m_vram_addr&0xffff00);
			if (LOG_VRAM) printf("\n prev writes was %04x\n\naddress set to %04x -\n", m_prev_writes, m_vram_addr );
			m_prev_writes = 0;
			break;

		case 1:
			m_vram_addr = (data << 8)  | (m_vram_addr & 0xff00ff);
			if (LOG_VRAM)printf("\naddress set to %04x\n", m_vram_addr);
			break;

		case 2:
			m_vram_addr = (data << 16) | (m_vram_addr & 0x00ffff);
			if (LOG_VRAM)printf("\naddress set to %04x\n", m_vram_addr);
			break;

		case 3:
		{
			address_space &vdp_space = machine().device<janshi_vdp_device>("janshivdp")->space();

			if (LOG_VRAM) printf("%02x ", data);
			m_prev_writes++;
			m_vram_addr++;

			vdp_space.write_byte(m_vram_addr, data);
			break;
		}
	}
}


WRITE8_MEMBER(pinkiri8_state::mux_w)
{
	m_mux_data = data;
}

READ8_MEMBER(pinkiri8_state::mux_p2_r)
{
	switch(m_mux_data)
	{
		case 0x01: return ioport("PL2_01")->read();
		case 0x02: return ioport("PL2_02")->read();
		case 0x04: return ioport("PL2_03")->read();
		case 0x08: return ioport("PL2_04")->read();
		case 0x10: return ioport("PL2_05")->read();
	}

	return 0xff;
}

READ8_MEMBER(pinkiri8_state::mux_p1_r)
{
	switch(m_mux_data)
	{
		case 0x01: return ioport("PL1_01")->read();
		case 0x02: return ioport("PL1_02")->read();
		case 0x04: return ioport("PL1_03")->read();
		case 0x08: return ioport("PL1_04")->read();
		case 0x10: return ioport("PL1_05")->read();
	}

	return 0xff;
}

static ADDRESS_MAP_START( pinkiri8_io, AS_IO, 8, pinkiri8_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_RAM //Z180 internal I/O
	AM_RANGE(0x60, 0x60) AM_WRITE(output_regs_w)
	AM_RANGE(0x80, 0x83) AM_WRITE(pinkiri8_vram_w)

	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("oki", okim6295_device, read, write) //correct?
	AM_RANGE(0xb0, 0xb0) AM_WRITE(mux_w) //mux
	AM_RANGE(0xb0, 0xb0) AM_READ(mux_p2_r) // mux inputs
	AM_RANGE(0xb1, 0xb1) AM_READ(mux_p1_r) // mux inputs
	AM_RANGE(0xb2, 0xb2) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf8, 0xf8) AM_READ_PORT("DSW1")
	AM_RANGE(0xf9, 0xf9) AM_READ_PORT("DSW2")
	AM_RANGE(0xfa, 0xfa) AM_READ_PORT("DSW3")
	AM_RANGE(0xfb, 0xfb) AM_READ_PORT("DSW4")

	/* Wing custom sound chip, same as Lucky Girl Z180 */
	AM_RANGE(0xc3, 0xc3) AM_WRITENOP
	AM_RANGE(0xc7, 0xc7) AM_WRITENOP
	AM_RANGE(0xcb, 0xcb) AM_WRITENOP
	AM_RANGE(0xcf, 0xcf) AM_WRITENOP

	AM_RANGE(0xd3, 0xd3) AM_WRITENOP
	AM_RANGE(0xd7, 0xd7) AM_WRITENOP
	AM_RANGE(0xdb, 0xdb) AM_WRITENOP
	AM_RANGE(0xdf, 0xdf) AM_WRITENOP

	AM_RANGE(0xe3, 0xe3) AM_WRITENOP
	AM_RANGE(0xe7, 0xe7) AM_WRITENOP
	AM_RANGE(0xeb, 0xeb) AM_WRITENOP
	AM_RANGE(0xef, 0xef) AM_WRITENOP

	AM_RANGE(0xf3, 0xf3) AM_WRITENOP
	AM_RANGE(0xf7, 0xf7) AM_WRITENOP

ADDRESS_MAP_END

static INPUT_PORTS_START( base_inputs )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset SW")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Books SW")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) //ron jan needs this
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("PL1_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_START("PL1_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )

	PORT_START("PL2_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL2_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_START("PL2_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)

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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4" )
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
INPUT_PORTS_END

static INPUT_PORTS_START( janshi )
	PORT_INCLUDE( base_inputs )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/10 Credits")
	PORT_DIPNAME( 0x08, 0x08, "Round Up Bonus" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x30, 0x00, "Base Score" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "5000 / 8000" )
	PORT_DIPSETTING(    0x10, "4000 / 8000" )
	PORT_DIPSETTING(    0x20, "3000 / 8000" )
	PORT_DIPSETTING(    0x30, "2000 / 8000" )
	PORT_DIPNAME( 0xc0, 0x80, "Win Rate" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x01, "Play Time" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "12 Seconds" )
	PORT_DIPSETTING(    0x01, "10 Seconds" )
	PORT_DIPSETTING(    0x02, "8 Seconds" )
	PORT_DIPSETTING(    0x03, "6 Seconds" )
	PORT_DIPNAME( 0x04, 0x04, "Yakuman Bonus" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "BGM" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Nudity" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_MODIFY("DSW3")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( ronjan )
	PORT_INCLUDE( base_inputs )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Play Time Limit" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "16 Seconds" )
	PORT_DIPSETTING(    0x01, "13 Seconds" )
	PORT_DIPSETTING(    0x02, "10 Seconds" )
	PORT_DIPSETTING(    0x03, "7 Seconds" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Payment" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	//FIXME: Bet Min can't be higher than Bet Max, needs a conditional dip here (or maybe, unified dips)
	PORT_DIPNAME( 0x38, 0x38, "Bet Min" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Rate of Win" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPSETTING(    0x01, "95%" )
	PORT_DIPSETTING(    0x02, "92%" )
	PORT_DIPSETTING(    0x03, "89%" )
	PORT_DIPSETTING(    0x04, "86%" )
	PORT_DIPSETTING(    0x05, "83%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x07, "77%" )
	PORT_DIPSETTING(    0x08, "74%" )
	PORT_DIPSETTING(    0x09, "71%" )
	PORT_DIPSETTING(    0x0a, "68%" )
	PORT_DIPSETTING(    0x0b, "65%" )
	PORT_DIPSETTING(    0x0c, "62%" )
	PORT_DIPSETTING(    0x0d, "59%" )
	PORT_DIPSETTING(    0x0e, "56%" )
	PORT_DIPSETTING(    0x0f, "53%" )
	PORT_DIPNAME( 0x10, 0x10, "Limit Display" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Credit Limit" )  PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x60, "20000" )
	PORT_DIPSETTING(    0x80, "15000" )
	PORT_DIPSETTING(    0xa0, "10000" )
	PORT_DIPSETTING(    0xc0, "5000" )
	PORT_DIPSETTING(    0xe0, "No Limit" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Coinage" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x78, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:4,5,6,7")
	PORT_DIPSETTING(    0x78, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x68, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Odds Type" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x02, 0x02, "Special Bonus Odds" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x04, 0x00, "Kind Mark of Back" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Nudity" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, "BGM" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Voice" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Double Up Game" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Limit" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, "6" )
	PORT_DIPSETTING(    0x00, "No Limit" )
INPUT_PORTS_END

static INPUT_PORTS_START( pinkiri8 )
	PORT_INCLUDE( base_inputs )

	/* standard mahjong panel converted to a hanafuda one */
	PORT_MODIFY("PL1_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PL1_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )

	PORT_MODIFY("PL1_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL1_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL1_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_01")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("PL2_02")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)

	PORT_MODIFY("PL2_03")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_04")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_HANAFUDA_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("PL2_05")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Game Style" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Win / Bet" )
	PORT_DIPSETTING(    0x00, "Out / In" )
	PORT_DIPNAME( 0x02, 0x02, "Premium Hand" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Coin Payment" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	//FIXME: Bet Min can't be higher than Bet Max, needs a conditional dip here (or maybe, unified dips)
	PORT_DIPNAME( 0x38, 0x38, "Bet Min" ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "5" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, "Rate of Win" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_DIPSETTING(    0x01, "86%" )
	PORT_DIPSETTING(    0x02, "82%" )
	PORT_DIPSETTING(    0x03, "78%" )
	PORT_DIPSETTING(    0x04, "74%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x06, "66%" )
	PORT_DIPSETTING(    0x07, "62%" )
	PORT_DIPSETTING(    0x08, "58%" )
	PORT_DIPSETTING(    0x09, "54%" )
	PORT_DIPSETTING(    0x0a, "50%" )
	PORT_DIPSETTING(    0x0b, "46%" )
	PORT_DIPSETTING(    0x0c, "42%" )
	PORT_DIPSETTING(    0x0d, "38%" )
	PORT_DIPSETTING(    0x0e, "34%" )
	PORT_DIPSETTING(    0x0f, "30%" )
	PORT_DIPNAME( 0x10, 0x10, "Odds Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x20, 0x20, "BGM" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, "Oya (Owner)" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "CPU Only" )
	PORT_DIPSETTING(    0x00, "Winner" )
	PORT_DIPNAME( 0x80, 0x00, "Koi Time Limit" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "15 Seconds" )
	PORT_DIPSETTING(    0x00, "30 Seconds" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Coinage" ) PORT_DIPLOCATION("SW3:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin/500 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/200 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/100 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x05, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x06, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x78, 0x40, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW3:4,5,6,7")
	PORT_DIPSETTING(    0x78, "10 Coins/1 Credit" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x68, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x60, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x58, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x18, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x10, "1 Coin/25 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/100 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Pinkiri Bonus" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Same Month Bonus" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Play Time Limit" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, "7 Seconds" )
	PORT_DIPSETTING(    0x00, "12 Seconds" )
	PORT_DIPNAME( 0x18, 0x00, "Credit Clear" ) PORT_DIPLOCATION("SW4:4,5")
	PORT_DIPSETTING(    0x00, "300 Seconds" )
	PORT_DIPSETTING(    0x08, "180 Seconds" )
	PORT_DIPSETTING(    0x10, "120 Seconds" )
	PORT_DIPSETTING(    0x18, "60 Seconds" )
	PORT_DIPNAME( 0x20, 0x20, "Panel Type" ) PORT_DIPLOCATION("SW4:6") // no real difference?
	PORT_DIPSETTING(    0x20, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Hanafuda" )
	PORT_DIPNAME( 0x40, 0x40, "Flip Flop Button" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	16,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(4,5),RGN_FRAC(3,5),RGN_FRAC(2,5),RGN_FRAC(1,5),RGN_FRAC(0,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16
};

static GFXDECODE_START( pinkiri8 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 0x100 )
GFXDECODE_END

static MACHINE_CONFIG_START( pinkiri8, pinkiri8_state )
	MCFG_CPU_ADD("maincpu",Z180,16000000)
	MCFG_CPU_PROGRAM_MAP(pinkiri8_map)
	MCFG_CPU_IO_MAP(pinkiri8_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pinkiri8_state, nmi_line_assert)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 62*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pinkiri8_state, screen_update_pinkiri8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pinkiri8)
	MCFG_PALETTE_ADD("palette", 0x2000)


	MCFG_DEVICE_ADD("janshivdp", JANSHIVDP, 0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pinkiri8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pinkiri8-ver.1.02.l1",   0x0000, 0x20000, CRC(f2df5b12) SHA1(e374e184a6a1e932550516011ec09a5accec9b03) )
	ROM_LOAD( "bios.rom", 0x0000, 0x4000, CRC(399df1ee) SHA1(8251f3aa7da4c7899c8e739c10b61260f4471311) ) //overlapped internal ROM

	ROM_REGION( 0x20000*5, "gfx1", 0 )
	ROM_LOAD( "pinkiri8-chr-01.a1",  0x00000, 0x20000, CRC(8ec73662) SHA1(9098348e519ce753dd7f38f0d855181bfc65aa42) )
	ROM_LOAD( "pinkiri8-chr-02.bc1", 0x20000, 0x20000, CRC(8dc20a65) SHA1(4062510fe06e8844a732754b7915a3b67ba2a3c5) )
	ROM_LOAD( "pinkiri8-chr-03.d1",  0x40000, 0x20000, CRC(bd5f269a) SHA1(7dfd039227551f0f0ed4afaafc76ca64a39a9b83) )
	ROM_LOAD( "pinkiri8-chr-04.ef1", 0x60000, 0x20000, CRC(4d0e5005) SHA1(4b90119c359c4de576131fd0e28d2fe1482ce74f) )
	ROM_LOAD( "pinkiri8-chr-05.h1",  0x80000, 0x20000, CRC(036ca165) SHA1(c4a2d6e394bbabcae1413d8a2916a19c90687edf) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
ROM_END


ROM_START( janshi )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "11.1l",    0x00000, 0x20000, CRC(a7692ddf) SHA1(5e7f43d8337583977baf22a28bbcd9b2182c0cde) )
	ROM_LOAD( "[3] 9009 1992.1 new jansh.bin", 0x0000, 0x4000, CRC(63cd3f12) SHA1(aebac739bffaf043e6acffa978e935f73ee1385f) ) //overlapped internal ROM

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "1.1a", 0x000000, 0x40000, CRC(92b140a5) SHA1(f3b38563f74650604ed0faaf84460e0b04b386b7) )
	ROM_LOAD( "2.1b", 0x040000, 0x40000, CRC(6de7e086) SHA1(e87426264f0181c17383ffe0f7ec7ff5fce3d809) )
	ROM_LOAD( "3.1d", 0x080000, 0x40000, CRC(4e94d8f2) SHA1(a25f542943d74915fc82910baafb9ff9db1ffd70) )
	ROM_LOAD( "4.1f", 0x0c0000, 0x40000, CRC(a5f6e3ef) SHA1(f1f3d28b27eea682aa71855a311fb3abdf9af2cd) )
	ROM_LOAD( "5.1h", 0x100000, 0x40000, CRC(ff2cc769) SHA1(ba4cf2923cf3d4d815a9327595f8e1801c3c8a2b) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "6.1k", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )
ROM_END

ROM_START( ronjan )
	ROM_REGION( 0x24000, "maincpu", 0 )
	ROM_LOAD( "ver201.bin",    0x00000, 0x20000, CRC(caa98c79) SHA1(e18f52fc910e3a77142ad2a3167805cfd664f0f4) )
	ROM_LOAD( "9009 1996.08 ron jan.bin", 0x00000, 0x4000, CRC(4eb74322) SHA1(84f864c0da3fb69948f6eb7ffecf0e722a882efc) ) //overlapped internal ROM

	ROM_REGION( 0x140000, "gfx1", 0 )
	ROM_LOAD( "eagle.1", 0x000000, 0x40000, CRC(11cef2c4) SHA1(fcd46bfa123cd91053f8d49892778e02a275ffdd) )
	ROM_LOAD( "eagle.2", 0x040000, 0x40000, CRC(177c444c) SHA1(5af0f6040ba121c90b3480ce636885cce535d3ea) )
	ROM_LOAD( "eagle.3", 0x080000, 0x40000, CRC(5b15b99f) SHA1(b99e2fa4cde7c8661d1a81ce5045f5df4f1de9f2) )
	ROM_LOAD( "eagle.4", 0x0c0000, 0x40000, CRC(d6797340) SHA1(0394ba570f2008f5a16e7c0a4dc67b1182be8899) )
	ROM_LOAD( "eagle.5", 0x100000, 0x40000, CRC(1aa42eaf) SHA1(edae2d1b58429e09ecfcaa5bcf4a9bfd5fb7cbea) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "eagle.6", 0x00000, 0x40000, CRC(8197034d) SHA1(b501dc7a27b1faad1361c309afd726da14b8b5f5) )
ROM_END


READ8_MEMBER(pinkiri8_state::ronjan_prot_r)
{
	static const char wing_str[6] = { 'W', 'I', 'N', 'G', '8', '9' };

	m_prot_read_index++;

	if(m_prot_read_index & 1)
		return 0xff; //value is discarded

	return wing_str[(m_prot_read_index >> 1)-1];
}

WRITE8_MEMBER(pinkiri8_state::ronjan_prot_w)
{
	if(data == 0)
	{
		m_prot_index = 0;
	}
	else
	{
		if(m_prot_index == 5)
			return;

		m_prot_char[m_prot_index++] = data;

		if(m_prot_char[0] == 'E' && m_prot_char[1] == 'R' && m_prot_char[2] == 'R' && m_prot_char[3] == 'O' && m_prot_char[4] == 'R')
			m_prot_read_index = 0;
	}
}

READ8_MEMBER(pinkiri8_state::ronjan_prot_status_r)
{
	return 0; //bit 0 seems a protection status bit
}

READ8_MEMBER(pinkiri8_state::ronjan_patched_prot_r)
{
	return 0; //value is read then discarded
}

DRIVER_INIT_MEMBER(pinkiri8_state,ronjan)
{
	m_maincpu->space(AS_IO).install_readwrite_handler(0x90, 0x90, read8_delegate(FUNC(pinkiri8_state::ronjan_prot_r), this), write8_delegate(FUNC(pinkiri8_state::ronjan_prot_w), this));
	m_maincpu->space(AS_IO).install_read_handler(0x66, 0x66, read8_delegate(FUNC(pinkiri8_state::ronjan_prot_status_r), this));
	m_maincpu->space(AS_IO).install_read_handler(0x9f, 0x9f, read8_delegate(FUNC(pinkiri8_state::ronjan_patched_prot_r), this));
}

GAME( 1992,  janshi,    0,   pinkiri8, janshi, driver_device,    0,      ROT0, "Eagle",         "Janshi",          MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1994,  ronjan,    0,   pinkiri8, ronjan, pinkiri8_state,    ronjan, ROT0, "Wing Co., Ltd", "Ron Jan (Super)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING ) // 'SUPER' flashes in the middle of the screen
GAME( 1994,  pinkiri8,  0,   pinkiri8, pinkiri8, driver_device,  0,      ROT0, "Alta",          "Pinkiri 8",       MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
