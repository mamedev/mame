// license:BSD-3-Clause
// copyright-holders:Sebastien Volpe, Tomasz Slanina, David Haywood
/*
    Super Real Mahjong P6 (JPN Ver.)
    (c)1996 Seta

WIP driver by Sebastien Volpe, Tomasz Slanina and David Haywood

Emulation Notes:
The graphics are compressed, using the same 8bpp RLE scheme as CPS3 uses
for the background on Sean's stage of Street Fighter III.

DMA Operations are not fully understood


according prg ROM (offset $0fff80):

    S12 SYSTEM
    SUPER REAL MAJAN P6
    SETA CO.,LTD
    19960410
    V1.00

TODO:
 - fix sound emulation
 - fix DMA operations
 - fix video emulation

Video reference : https://youtu.be/wNm3tu1iGvM

Are there other games on this 'System S12' hardware ???

---------------- dump infos ----------------

[Jun/15/2000]

Super Real Mahjong P6 (JPN Ver.)
(c)1996 Seta

SX011
E47-REV01B

CPU:    68000-16
Sound:  NiLe
OSC:    16.0000MHz
        42.9545MHz
        56.0000MHz

Chips:  ST-0026 NiLe (video, sound)
        ST-0017


SX011-01.22  chr, samples (?)
SX011-02.21
SX011-03.20
SX011-04.19
SX011-05.18
SX011-06.17
SX011-07.16
SX011-08.15

SX011-09.10  68000 data

SX011-10.4   68000 prg.
SX011-11.5


Dumped 06/15/2000

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"
#include "sound/setapcm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class srmp6_state : public driver_device
{
public:
	srmp6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_chrram(*this, "chrram"),
		m_dmaram(*this, "dmaram", 0x100, ENDIANNESS_BIG),
		m_video_regs(*this, "video_regs"),
		m_nile_region(*this, "nile"),
		m_nile_bank(*this, "nile_bank"),
		m_key_io(*this, "KEY%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprram(*this, "sprram")
	{ }

	void srmp6(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override { m_gfxdecode->gfx(0)->mark_all_dirty(); }

private:
	std::unique_ptr<u16[]> m_tileram;
	required_shared_ptr<u16> m_chrram;
	memory_share_creator<u16> m_dmaram;
	required_shared_ptr<u16> m_video_regs;
	required_region_ptr<u8> m_nile_region;

	required_memory_bank m_nile_bank;
	required_ioport_array<4> m_key_io;

	u16 m_brightness;
	u8 m_input_select;

	u16 m_lastb;
	u16 m_lastb2;
	int m_destl;

	void input_select_w(u16 data);
	u16 inputs_r();
	void video_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 video_regs_r(offs_t offset);
	void dma_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tileram_r(offs_t offset);
	void tileram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void paletteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 irq_ack_r();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void update_palette();
	u32 process(u8 b,u32 dst_offset);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram16_device> m_sprram;
	void srmp6_map(address_map &map) ATTR_COLD;
};

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

static const gfx_layout tiles8x8_layout =
{
	8,8,
	(0x100000*16)/0x40,
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*64
};

static inline u8 get_fade(int c, int f) // same as CPS3?
{
	// bit 7 unknown
	// bit 6 fade enable / disable
	// bit 5 fade mode
	// bit 4-0 fade value
	if (f & 0x40) // Fading enable / disable
	{
		f &= 0x3f;
		c = (f & 0x20) ? (c + (((0x1f - c) * (f & 0x1f)) / 0x1f)) : ((c * f) / 0x1f);
		c = std::clamp(c, 0, 0x1f);
	}
	return c;
}

void srmp6_state::update_palette()
{
	int brg = m_brightness & 0x7f;

	for (int i = 0; i < m_palette->entries(); i++)
	{
		u8 r = m_palette->basemem().read16(i) >>  0 & 0x1f;
		u8 g = m_palette->basemem().read16(i) >>  5 & 0x1f;
		u8 b = m_palette->basemem().read16(i) >> 10 & 0x1f;

		if (brg & 0x40)
		{
			r = get_fade(r, brg);
			g = get_fade(g, brg);
			b = get_fade(b, brg);
		}
		m_palette->set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

void srmp6_state::video_start()
{
	m_tileram = make_unique_clear<u16[]>(0x100000*16/2);

	// create the char set (gfx will then be updated dynamically from RAM)
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, tiles8x8_layout, (u8*)m_tileram.get(), 0, m_palette->entries() / 256, 0));
	m_gfxdecode->gfx(0)->set_granularity(256);

	m_brightness = 0;

	save_pointer(NAME(m_tileram), 0x100000*16/2);
	save_item(NAME(m_brightness));
	save_item(NAME(m_destl));
	save_item(NAME(m_lastb));
	save_item(NAME(m_lastb2));
}

#if 0
static int xixi = 0;
#endif

u32 srmp6_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int alpha;
	int x,y,tileno,height,width,xw,yw,sprite,xb,yb;
	u16 *sprite_list = m_sprram->buffer();
	u16 mainlist_offset = 0;

	union
	{
		s16 a;
		u16 b;
	} temp;

	bitmap.fill(0, cliprect);

#if 0
	//debug
	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		++xixi;
		logerror("%x\n",xixi);
	}

	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		--xixi;
		logerror("%x\n",xixi);
	}
#endif

	// Main spritelist is 0x0000 - 0x1fff in spriteram, sublists follow
	while (mainlist_offset<0x2000/2)
	{
		u16 *sprite_sublist = &sprite_list[sprite_list[mainlist_offset+1]<<3];
		u16 sublist_length=sprite_list[mainlist_offset+0] & 0x7fff; //+1 ?
		s16 global_x,global_y, flip_x, flip_y;
		u16 global_pal;

		// end of list marker
		if (sprite_list[mainlist_offset+0] == 0x8000)
			break;

		if (sprite_list[mainlist_offset+0] != 0)
		{
			temp.b=sprite_list[mainlist_offset+2];
			global_x=temp.a;
			temp.b=sprite_list[mainlist_offset+3];
			global_y=temp.a;

			global_pal = sprite_list[mainlist_offset+4] & 0x7;

			if ((sprite_list[mainlist_offset+5] & 0x700) == 0x700)
			{
				alpha = pal5bit(sprite_list[mainlist_offset+5] & 0x1f);
			}
			else
			{
				alpha = 255;
			}
			//  logerror("%x %x \n",sprite_list[mainlist_offset+1],sublist_length);

			while (sublist_length)
			{
				sprite=sprite_sublist[0] & 0x7fff;
				flip_x=sprite_sublist[1]>>8&1;
				flip_y=sprite_sublist[1]>>9&1;
				temp.b=sprite_sublist[2];
				x=temp.a;
				temp.b=sprite_sublist[3];
				y=temp.a;
				//x+=global_x;
				//y+=global_y;

				width=((sprite_sublist[1]) & 0x3);
				height=((sprite_sublist[1]>>2) & 0x3);

				height = 1 << height;
				width = 1 << width;

				y-=height*8;
				tileno = sprite;
				//tileno += (sprite_list[4] & 0xf)*0x4000; // this makes things worse in places (title screen for example)

				for (xw = 0; xw < width; xw++)
				{
					for (yw = 0; yw < height; yw++)
					{
						if (!flip_x)
							xb=x+xw*8+global_x;
						else
							xb=x+(width-xw-1)*8+global_x;

						if (!flip_y)
							yb=y+yw*8+global_y;
						else
							yb=y+(height-yw-1)*8+global_y;

						m_gfxdecode->gfx(0)->alpha(bitmap,cliprect,tileno,global_pal,flip_x,flip_y,xb,yb,0,alpha);
						tileno++;
					}
				}
				sprite_sublist+=8;
				--sublist_length;
			}
		}
		mainlist_offset+=8;
	}

	m_sprram->copy();

	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		FILE *p=fopen("tileram.bin","wb");
		fwrite(m_tileram.get(), 1, 0x100000*16, p);
		fclose(p);
	}


	return 0;
}

/***************************************************************************
    Main CPU memory handlers
***************************************************************************/

void srmp6_state::machine_start()
{
	m_nile_bank->configure_entries(0, 16, memregion("nile")->base(), 0x200000);
	save_item(NAME(m_input_select));
}

void srmp6_state::input_select_w(u16 data)
{
	m_input_select = data & 0x0f;
}

u16 srmp6_state::inputs_r()
{
	switch (m_input_select) // inputs
	{
		case 1<<0: return m_key_io[0]->read();
		case 1<<1: return m_key_io[1]->read();
		case 1<<2: return m_key_io[2]->read();
		case 1<<3: return m_key_io[3]->read();
	}

	return 0;
}


void srmp6_state::video_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x5e/2: // bank switch, used by ROM check
		{
			LOG(("%x\n",data));
			m_nile_bank->set_entry(data & 0x0f);
			break;
		}

		// set by IT4
		case 0x5c/2: // either 0x40 explicitly in many places, or according $2083b0 (IT4)
			//Fade in/out (0x40(dark)-0x60(normal)-0x7e?(bright) reset by 0x00?
			if (m_brightness != data)
			{
				m_brightness = data;
				update_palette();
			}
			break;

		// unknown registers - there are others

		// set by IT4 (jsr $b3c), according flip screen dsw
		case 0x48/2: //     0 /  0xb0 if flipscreen
		case 0x52/2: //     0 / 0x2ef if flipscreen
		case 0x54/2: // 0x152 / 0x15e if flipscreen

		// set by IT4 ($82e-$846)
		case 0x56/2: // written 8,9,8,9 successively

		default:
			logerror("video_regs_w (PC=%06X): %04x = %04x & %04x\n", m_maincpu->pcbase(), offset*2, data, mem_mask);
			break;
	}
	COMBINE_DATA(&m_video_regs[offset]);
}

u16 srmp6_state::video_regs_r(offs_t offset)
{
	logerror("video_regs_r (PC=%06X): %04x\n", m_maincpu->pcbase(), offset*2);
	return m_video_regs[offset];
}


// DMA RLE stuff - the same as CPS3
u32 srmp6_state::process(u8 b,u32 dst_offset)
{
	int l = 0;

	u8 *tram=(u8*)m_tileram.get();

	if (m_lastb == m_lastb2)  //rle
	{
		int rle=(b+1) & 0xff;

		for (int i = 0; i <rle; ++i)
		{
			tram[dst_offset + m_destl] = m_lastb;
			m_gfxdecode->gfx(0)->mark_dirty((dst_offset + m_destl)/0x40);

			dst_offset++;
			++l;
		}
		m_lastb2 = 0xffff;

		return l;
	}
	else
	{
		m_lastb2 = m_lastb;
		m_lastb = b;
		tram[dst_offset + m_destl] = b;
		m_gfxdecode->gfx(0)->mark_dirty((dst_offset + m_destl)/0x40);

		return 1;
	}
}


void srmp6_state::dma_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dmaram[offset]);
	if (offset==13 && m_dmaram[offset]==0x40)
	{
		u32 srctab=2*((((u32)m_dmaram[5])<<16)|m_dmaram[4]);
		u32 srcdata=2*((((u32)m_dmaram[11])<<16)|m_dmaram[10]);
		u32 len=4*(((((u32)m_dmaram[7]&3)<<16)|m_dmaram[6])+1); //??? WRONG!
		int tempidx = 0;

		// show params
		LOG(("DMA! %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x\n",
				m_dmaram[0x00/2],
				m_dmaram[0x02/2],
				m_dmaram[0x04/2],
				m_dmaram[0x06/2],
				m_dmaram[0x08/2],
				m_dmaram[0x0a/2],
				m_dmaram[0x0c/2],
				m_dmaram[0x0e/2],
				m_dmaram[0x10/2],
				m_dmaram[0x12/2],
				m_dmaram[0x14/2],
				m_dmaram[0x16/2],
				m_dmaram[0x18/2],
				m_dmaram[0x1a/2]));

		m_destl = m_dmaram[9]*0x40000;

		m_lastb = 0xfffe;
		m_lastb2 = 0xffff;

		while (1)
		{
			u8 ctrl=m_nile_region[srcdata];
			++srcdata;

			for (int i = 0; i < 8; ++i)
			{
				u8 p=m_nile_region[srcdata];

				if (ctrl & 0x80)
				{
					u8 real_byte;
					real_byte = m_nile_region[srctab+p*2];
					tempidx+=process(real_byte,tempidx);
					real_byte = m_nile_region[srctab+p*2+1];//px[DMA_XOR((current_table_address+p*2+1))];
					tempidx+=process(real_byte,tempidx);
				}
				else
				{
					tempidx+=process(p,tempidx);
				}

				ctrl<<=1;
				++srcdata;

				if (tempidx >= len)
				{
					LOG(("%x\n",srcdata));
					return;
				}
			}
		}
	}
}

// if tileram is actually bigger than the mapped area, how do we access the rest?
u16 srmp6_state::tileram_r(offs_t offset)
{
	return m_chrram[offset];
}

void srmp6_state::tileram_w(offs_t offset, u16 data, u16 mem_mask)
{
	//u16 tmp;
	COMBINE_DATA(&m_chrram[offset]);

	// are the DMA registers enabled some other way, or always mapped here, over RAM?
	if (offset >= 0xfff00/2 && offset <= 0xfff1a/2 )
	{
		offset &=0x1f;
		dma_w(offset ,data, mem_mask);
	}
}

void srmp6_state::paletteram_w(offs_t offset, u16 data, u16 mem_mask)
{
	int brg = m_brightness & 0x7f;

	m_palette->write16(offset, data, mem_mask);

	if (brg & 0x40)
	{
		u8 r = m_palette->basemem().read16(offset) >>  0 & 0x1f;
		u8 g = m_palette->basemem().read16(offset) >>  5 & 0x1f;
		u8 b = m_palette->basemem().read16(offset) >> 10 & 0x1f;

		r = get_fade(r, brg);
		g = get_fade(g, brg);
		b = get_fade(b, brg);

		m_palette->set_pen_color(offset, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}

u16 srmp6_state::irq_ack_r()
{
	m_maincpu->set_input_line(4, CLEAR_LINE);
	return 0; // value read doesn't matter
}

void srmp6_state::srmp6_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x23ffff).ram();                 // work RAM

	map(0x300000, 0x300005).w(FUNC(srmp6_state::input_select_w));     // inputs
	map(0x300000, 0x300001).portr("DSW");
	map(0x300002, 0x300005).r(FUNC(srmp6_state::inputs_r));     // inputs

	// OBJ RAM: checked [$400000-$47dfff]
	map(0x400000, 0x47ffff).ram().share("sprram");

	map(0x480000, 0x480fff).ram().w(FUNC(srmp6_state::paletteram_w)).share("palette");

	map(0x4c0000, 0x4c006f).rw(FUNC(srmp6_state::video_regs_r), FUNC(srmp6_state::video_regs_w)).share(m_video_regs);    // ? gfx regs ST-0026 NiLe
	map(0x4d0000, 0x4d0001).r(FUNC(srmp6_state::irq_ack_r));
	map(0x4e0000, 0x4e00ff).rw("nile", FUNC(nile_sound_device::snd_r), FUNC(nile_sound_device::snd_w));
	map(0x4e0100, 0x4e0101).rw("nile", FUNC(nile_sound_device::key_r), FUNC(nile_sound_device::key_w));
	//map(0x4e0110, 0x4e0111).noprw(); // ? accessed once ($268dc, written $b.w)

	// CHR RAM: checked [$500000-$5fffff]
	map(0x500000, 0x5fffff).rw(FUNC(srmp6_state::tileram_r), FUNC(srmp6_state::tileram_w)).share(m_chrram);
	//map(0x5fff00, 0x5fff1f).ram(); // ? see routine $5ca8, video_regs related ???
	//map(0x5fff00, 0x5fffff).w(FUNC(srmp6_state::dma_w)).share(m_dmaram);

	map(0x600000, 0x7fffff).bankr(m_nile_bank);    // banked ROM (used by ROM check)
	map(0x800000, 0x9fffff).rom().region("user1", 0);
	//map(0xf00004, 0xf00005).ram(); // ?
	//map(0xf00006, 0xf00007).ram(); // ?

}


/***************************************************************************
    Port definitions
***************************************************************************/

static INPUT_PORTS_START( srmp6 )

	PORT_START("KEY0")
	PORT_BIT( 0xfe01, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitly discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_SERVICE_NO_TOGGLE( 0x0100, IP_ACTIVE_LOW )

	PORT_START("KEY1")
	PORT_BIT( 0xfe41, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitly discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0xfe41, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitly discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0xfe61, IP_ACTIVE_LOW, IPT_UNUSED ) // explicitly discarded
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0180, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")   // 16-bit DSW1 (0x0000) +DSW2 (0x0700)
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )      PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) )       PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unused ) )       PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unused ) )       PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Re-Clothe" )             PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Nudity" )                PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPSETTING(      0x0100, "7" )
	PORT_DIPSETTING(      0x0200, "6" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0500, "2" )
	PORT_DIPSETTING(      0x0600, "1" )
	PORT_DIPSETTING(      0x0700, "4" )
	PORT_DIPNAME( 0x0800, 0x0000, "Kuitan" )                PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Continues ) )    PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x8000, IP_ACTIVE_LOW, "DSW2:8" )  PORT_DIPLOCATION("DSW2:8")
INPUT_PORTS_END

/***************************************************************************
    Machine driver
***************************************************************************/

void srmp6_state::srmp6(machine_config &config)
{
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &srmp6_state::srmp6_map);
	m_maincpu->set_vblank_int("screen", FUNC(srmp6_state::irq4_line_assert)); // irq3 is a timer irq, but it's never enabled

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 42*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(srmp6_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode_device::empty);

	BUFFERED_SPRITERAM16(config, m_sprram);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// matches video, needs to verified; playback rate: (42.9545Mhz / 7) / 160 or (42.9545Mhz / 5) / 224 or (42.9545Mhz / 4) / 280?
	nile_sound_device &nile(NILE_SOUND(config, "nile", XTAL(42'954'545) / 7));
	nile.add_route(0, "lspeaker", 1.0);
	nile.add_route(1, "rspeaker", 1.0);
}


/***************************************************************************
    ROM definition(s)
***************************************************************************/

ROM_START( srmp6 )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "sx011-10.4", 0x000001, 0x080000, CRC(8f4318a5) SHA1(44160968cca027b3d42805f2dd42662d11257ef6) )
	ROM_LOAD16_BYTE( "sx011-11.5", 0x000000, 0x080000, CRC(7503d9cf) SHA1(03ab35f13b6166cb362aceeda18e6eda8d3abf50) )

	ROM_REGION16_BE( 0x200000, "user1", 0 ) // 68000 Data
	ROM_LOAD16_WORD_SWAP( "sx011-09.10", 0x000000, 0x200000, CRC(58f74438) SHA1(a256e39ca0406e513ab4dbd812fb0b559b4f61f2) )

	// these are accessed directly by the 68k, DMA device etc.  NOT decoded
	ROM_REGION( 0x2000000, "nile", 0)   // Banked ROM
	ROM_LOAD16_WORD_SWAP( "sx011-08.15", 0x0000000, 0x0400000, CRC(01b3b1f0) SHA1(bbd60509c9ba78358edbcbb5953eafafd6e2eaf5) ) // CHR00
	ROM_LOAD16_WORD_SWAP( "sx011-07.16", 0x0400000, 0x0400000, CRC(26e57dac) SHA1(91272268977c5fbff7e8fbe1147bf108bd2ed321) ) // CHR01
	ROM_LOAD16_WORD_SWAP( "sx011-06.17", 0x0800000, 0x0400000, CRC(220ee32c) SHA1(77f39b54891c2381b967534b0f6d380962eadcae) ) // CHR02
	ROM_LOAD16_WORD_SWAP( "sx011-05.18", 0x0c00000, 0x0400000, CRC(87e5fea9) SHA1(abd751b5744d6ac7e697774ea9a7f7455bf3ac7c) ) // CHR03
	ROM_LOAD16_WORD_SWAP( "sx011-04.19", 0x1000000, 0x0400000, CRC(e90d331e) SHA1(d8afb1497cec8fe6de10d23d49427e11c4c57910) ) // CHR04
	ROM_LOAD16_WORD_SWAP( "sx011-03.20", 0x1400000, 0x0400000, CRC(f1f24b35) SHA1(70d6848f77940331e1be8591a33d62ac22a3aee9) ) // CHR05
	ROM_LOAD16_WORD_SWAP( "sx011-02.21", 0x1800000, 0x0400000, CRC(c56d7e50) SHA1(355c64b38e7b266f386b9c0b906c8581fc15374b) ) // CHR06
	ROM_LOAD16_WORD_SWAP( "sx011-01.22", 0x1c00000, 0x0400000, CRC(785409d1) SHA1(3e31254452a30d929161a1ea3a3daa69de058364) ) // CHR07
ROM_END

} // anonymous namespace


/***************************************************************************
    Game driver(s)
***************************************************************************/

GAME( 1995, srmp6, 0, srmp6, srmp6, srmp6_state, empty_init, ROT0, "Seta", "Super Real Mahjong P6 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
