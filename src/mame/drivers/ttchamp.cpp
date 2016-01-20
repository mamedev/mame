// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Table Tennis Champions
   (c) 1995 Gamart

 ___________________________________________________
|        __     _________ __________   __________  |
|__      |_|   |UM62256D| |UM62256D|   |UM62256D|  |
  |    LN358N  |________| |________|   |________|  |
 _|            _________ _________     __________  |
|_  _1_______  |TI     | |TI     |     |UM62256D|  |
|_  |27C020 |  |52C1HXW| |52ALROW|     |________|  |
|_  |_______|  |       | |       |     __________  |
|__            |       | |       |     |UM62256D|  |
 _| __         |_______| |_______|     |________|  |
|_  |_| 74HC74                                     |
|_  OKI        74LS244   _3_______  _5_______      |
|_  M6295      74LS244   |27C040 |  |27C040 |      |
|_    57C55LK            |_______|  |_______|      |
|_             ________  _2_______  _4_______      |
|_    57C55LK  |GM76C28| |27C040 |  |27C040 |      |
|_             |_______| |_______|  |_______|      |
|_    74LS244  ________                            |
|_             |GM76C28| PAL16L8B PAL16L8B 74LS245 |
|_             |_______|                           |
|_                       74LS244  74LS373N 74LS373 |
|_    74LS244  PIC16C84                            |
|_                       74HC273N 74LS373N 74LS245 |
|_             74HC74B1              _____________ |
  |                            OSC   |NEC V30    | |
 _|   74LS244  74LS14N 74HC74  16MHz |D70116C-10 | |
|__________________________________________________|

The PCB is Spanish and manufactured by Gamart.


Table tennis Championships by Gamart 1995

This game come from Gamart,an obscure spanish software house.
Hardware info:
main cpu: V30
sound chip: oki6295
custom chip: tpc1020bfn x2
osc: 16 mhz
Rom files definition:
ttennis2/3 main program
ttennis1 adpcm data
ttennis4/5 graphics
*there is a pic16c84 that I cannot dump because my programmer doesn't support it.

Dumped by tirino73




- works in a very similar way to 'Spider' (twins.c)
  including the blitter (seems to be doubled up hardware tho, twice as many layers?)
- Convert this to a blitter device, and share it with twins.c
- A bunch of spurious RAM writes to ROM area (genuine bug? left-overs?)

Notes
I think the PIC is used to interface with battery backed RAM instead of an EEPROM,
we currently simulate this as the PIC is read protected.



*/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


class ttchamp_state : public driver_device
{
public:
	ttchamp_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	UINT16 m_paloff;
	UINT16 m_port10;
	UINT8 m_rombank;
	UINT16 m_videoram0[0x10000 / 2];
	UINT16 m_videoram2[0x10000 / 2];

	enum picmode
	{
		PIC_IDLE = 0,
		PIC_SET_READADDRESS = 1,
		PIC_SET_WRITEADDRESS = 2,
		PIC_SET_WRITELATCH = 3,
		PIC_SET_READLATCH = 4

	};

	picmode m_picmodex;

	int m_pic_readaddr;
	int m_pic_writeaddr;
	int m_pic_latched;
	int m_pic_writelatched;

	std::unique_ptr<UINT8[]> m_bakram;

	UINT16 m_mainram[0x10000 / 2];

	int m_spritesinit;
	int m_spriteswidth;
	int m_spritesaddr;
	UINT16* m_rom16;
	UINT8* m_rom8;

	DECLARE_WRITE16_MEMBER(paloff_w);
	DECLARE_WRITE16_MEMBER(paldat_w);

	DECLARE_WRITE16_MEMBER(port10_w);

	DECLARE_WRITE16_MEMBER(port20_w);
	DECLARE_WRITE16_MEMBER(port62_w);

	DECLARE_READ16_MEMBER(port1e_r);

	DECLARE_READ16_MEMBER(pic_r);
	DECLARE_WRITE16_MEMBER(pic_w);

	DECLARE_READ16_MEMBER(blit_start_r);

	DECLARE_READ16_MEMBER(mem_r);
	DECLARE_WRITE16_MEMBER(mem_w);

	virtual void machine_start() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);
};

ALLOW_SAVE_TYPE(ttchamp_state::picmode);


void ttchamp_state::machine_start()
{
	m_rom16 = (UINT16*)memregion("maincpu")->base();
	m_rom8 = memregion("maincpu")->base();

	m_picmodex = PIC_IDLE;

	m_bakram = std::make_unique<UINT8[]>(0x100);
	machine().device<nvram_device>("backram")->set_base(m_bakram.get(), 0x100);

	save_item(NAME(m_paloff));
	save_item(NAME(m_port10));
	save_item(NAME(m_rombank));
	save_item(NAME(m_videoram0));
	save_item(NAME(m_videoram2));
	save_item(NAME(m_picmodex));
	save_item(NAME(m_pic_readaddr));
	save_item(NAME(m_pic_writeaddr));
	save_item(NAME(m_pic_latched));
	save_item(NAME(m_pic_writelatched));
	save_item(NAME(m_mainram));
	save_item(NAME(m_spritesinit));
	save_item(NAME(m_spriteswidth));
	save_item(NAME(m_spritesaddr));

}

void ttchamp_state::video_start()
{
}

UINT32 ttchamp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	logerror("update\n");
	int y,x,count;

	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());
	UINT8 *videoramfg;
	UINT8* videorambg;

	count=0;
	videorambg = (UINT8*)m_videoram0;
	videoramfg = (UINT8*)m_videoram2;

	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			bitmap.pix16(y, x) = videorambg[BYTE_XOR_LE(count)]+0x300;
			count++;
		}
	}

	/*
	count=0;
	videoram = (UINT8*)m_videoram1;
	for (y=0;y<yyy;y++)
	{
	    for(x=0;x<xxx;x++)
	    {
	        UINT8 pix = videoram[BYTE_XOR_LE(count)];
	        if (pix) bitmap.pix16(y, x) = pix+0x200;
	        count++;
	    }
	}
	*/

	count=0;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			UINT8 pix = videoramfg[BYTE_XOR_LE(count)];
			if (pix)
			{
				// first pen values seem to be special
				// see char select and shadows ingame
				// pen 0 = transparent
				// pen 1 = blend 1
				// pen 2 = blend 2
				// pen 3 = ??

				if (pix == 0x01) // blend mode 1
				{
					UINT8 pix = videorambg[BYTE_XOR_LE(count)];
					bitmap.pix16(y, x) = pix + 0x200;
				}
				else if (pix == 0x02) // blend mode 2
				{
					UINT8 pix = videorambg[BYTE_XOR_LE(count)];
					bitmap.pix16(y, x) = pix + 0x100;
				}
				else
				{
					bitmap.pix16(y, x) = pix + 0x000;
				}
			}
			count++;
		}
	}

#if 0
	for (int i = 0; i < 0x8000; i++)
	{
		// how are layers cleared?
		// I think it actually does more blit operations with
		// different bits of m_port10 set to redraw the backgrounds using the video ram data as a source rather than ROM - notice the garbage you see behind 'sprites' right now
		// this method also removes the text layer, which we don't want
	//  m_videoram1[i] = 0x0000;
	//  m_videoram2[i] = 0x0000;
	}
#endif

	return 0;
}


WRITE16_MEMBER(ttchamp_state::paloff_w)
{
	COMBINE_DATA(&m_paloff);
}


WRITE16_MEMBER(ttchamp_state::paldat_w)
{
	// 0x8000 of offset is sometimes set
	m_palette->set_pen_color(m_paloff & 0x3ff,pal5bit(data>>0),pal5bit(data>>5),pal5bit(data>>10));
}

READ16_MEMBER(ttchamp_state::pic_r)
{
//  printf("%06x: read from PIC (%04x)\n", space.device().safe_pc(),mem_mask);
	if (m_picmodex == PIC_SET_READLATCH)
	{
//      printf("read data %02x from %02x\n", m_pic_latched, m_pic_readaddr);
		m_picmodex = PIC_IDLE;

		return m_pic_latched << 8;

	}
	return 0x00;

}

WRITE16_MEMBER(ttchamp_state::pic_w)
{
//  printf("%06x: write to PIC %04x (%04x) (%d)\n", space.device().safe_pc(),data,mem_mask, m_picmodex);
	if (m_picmodex == PIC_IDLE)
	{
		if (data == 0x11)
		{
			m_picmodex = PIC_SET_READADDRESS;
//          printf("state = SET_READADDRESS\n");
		}
		else if (data == 0x12)
		{
			m_picmodex = PIC_SET_WRITELATCH;
//          printf("latch write data.. \n" );
		}
		else if (data == 0x20)
		{
			m_picmodex = PIC_SET_WRITEADDRESS;
//          printf("state = PIC_SET_WRITEADDRESS\n");
		}
		else if (data == 0x21) // write latched data
		{
			m_picmodex = PIC_IDLE;
			m_bakram[m_pic_writeaddr] = m_pic_writelatched;
	//      printf("wrote %02x to %02x\n", m_pic_writelatched, m_pic_writeaddr);
		}
		else if (data == 0x22) // next data to latch
		{
			// why does it read twice as many addresses, forcing us to shift the
			// address by 1 to give correct results? maybe it can read 'previous' data' too?
			m_pic_latched = m_bakram[m_pic_readaddr>>1];

//          printf("latch read data %02x from %02x\n",m_pic_latched, m_pic_readaddr );
			m_picmodex = PIC_SET_READLATCH; // waiting to read...
		}
		else
		{
//          printf("unknown\n");
		}
	}
	else if (m_picmodex == PIC_SET_READADDRESS)
	{
		m_pic_readaddr = data;
		m_picmodex = PIC_IDLE;
	}
	else if (m_picmodex == PIC_SET_WRITEADDRESS)
	{
		m_pic_writeaddr = data;
		m_picmodex = PIC_IDLE;
	}
	else if (m_picmodex == PIC_SET_WRITELATCH)
	{
		m_pic_writelatched = data;
		m_picmodex = PIC_IDLE;
	}

}


READ16_MEMBER(ttchamp_state::mem_r)
{
	// bits 0xf0 are used too, so this is likely wrong.

	UINT16* vram;
	if ((m_port10&0xf) == 0x00)
		vram = m_videoram0;
	else if ((m_port10&0xf)  == 0x01)
		vram = m_videoram2;
	else if ((m_port10&0xf)  == 0x03)
		vram = m_videoram2;
	else
	{
		printf("unhandled video bank %02x\n", m_port10);
		vram = m_videoram2;
	}

	if (offset < 0x10000 / 2)
	{
		return m_mainram[offset&0x7fff];
	}
	else if (offset < 0x20000 / 2)
	{
		return vram[offset&0x7fff];
	}
	else
	{
		UINT16 *src = m_rom16 + (0x100000/2); // can the CPU ever see the lower bank?
		return src[offset];
	}
}

WRITE16_MEMBER(ttchamp_state::mem_w)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?

	// bits 0xf0 are used too, so this is likely wrong.

	UINT16* vram;
	if ((m_port10&0xf)  == 0x00)
		vram = m_videoram0;
	else if ((m_port10&0xf)  == 0x01)
		vram = m_videoram2;
	else if ((m_port10&0xf)  == 0x03)
		vram = m_videoram2;
	else
	{
		printf("unhandled video bank %02x\n", m_port10);
		vram = m_videoram2;
	}


	if (m_spritesinit == 1)
	{
	//  printf("%06x: spider_blitter_w %08x %04x %04x (init?) (base?)\n", space.device().safe_pc(), offset * 2, data, mem_mask);

		m_spritesinit = 2;
		m_spritesaddr = offset;

	}
	else if (m_spritesinit == 2)
	{
	//  printf("%06x: spider_blitter_w %08x %04x %04x (init2) (width?)\n", space.device().safe_pc(), offset * 2, data, mem_mask);
		m_spriteswidth = offset & 0xff;
		//printf("%08x\n",(offset*2) & 0xfff00);

		m_spritesinit = 3;
	}
	else
	{
		if (offset < 0x10000 / 2)
		{
			COMBINE_DATA(&m_mainram[offset&0x7fff]);
		}
		else if (offset < 0x20000 / 2)
		{
			COMBINE_DATA(&vram[offset&0x7fff]);
		}
		else if ((offset >= 0x30000 / 2) && (offset < 0x40000 / 2))
		{
			if(m_spritesinit != 3)
			{
				printf("blitter bus write but blitter unselected? %08x %04x\n",offset*2,data);
				return;
			}

			m_spritesinit = 0;

			// 0x30000-0x3ffff used, on Spider it's 0x20000-0x2ffff
			offset &= 0x7fff;

			UINT8 *src = m_rom8;

			if (m_rombank)
				src += 0x100000;

		//  printf("%06x: spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", space.device().safe_pc(), offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);
			offset &= 0x7fff;

			for (int i = 0; i < m_spriteswidth; i++)
			{
				if (m_port10 & 0x30) // this is set when moving objects are cleared, although not screen clears?
				{
					/* guess: assume that bit 4 is for layer 0 and bit 5 for layer 1
					   (according to 0x21 setted at the "Clubs League" color fade-out)
					*/
					if(m_port10 & 0x10)
						m_videoram0[offset] = 0x0000;
					if(m_port10 & 0x20)
						m_videoram2[offset] = 0x0000;

					offset++;
				}
				else
				{
					UINT8 data;

					data = (src[(m_spritesaddr * 2) + 1]);
					//data |= vram[offset] >> 8;

					/* bit 1 actually enables transparent pen */
					if (data || (m_port10 & 2) == 0)
						vram[offset] = (vram[offset] & 0x00ff) | data << 8;

					data = src[(m_spritesaddr * 2)];

					if (data || (m_port10 & 2) == 0)
						vram[offset] = (vram[offset] & 0xff00) | data;


					m_spritesaddr++;
					offset++;
				}

				offset &= 0x7fff;
			}
		}
		else
		{
			// sometimes happens, why? special meanings? wrong interpretation of something else?
			printf("%06x: spider_blitter_w unhandled RAM access %08x %04x %04x\n", space.device().safe_pc(), offset * 2, data, mem_mask);
		}
	}
}



static ADDRESS_MAP_START( ttchamp_map, AS_PROGRAM, 16, ttchamp_state )
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(mem_r, mem_w)
ADDRESS_MAP_END

/* Re-use same parameters as before (one-shot) */
READ16_MEMBER(ttchamp_state::port1e_r)
{
	m_spritesinit = 3;
	return 0xff;
}

READ16_MEMBER(ttchamp_state::blit_start_r)
{
	m_spritesinit = 1;
	return 0xff;
}

/* blitter mode select */
WRITE16_MEMBER(ttchamp_state::port10_w)
{
	/*
	 --xx ---- fill enable
	 ---- --x- opacity enable (Gamart logo)
	 ---- ---x layer select
	*/
	COMBINE_DATA(&m_port10);
}

/* selects upper bank for the blitter */
WRITE16_MEMBER(ttchamp_state::port20_w)
{
	//printf("%06x: port20_w %04x %04x\n", space.device().safe_pc(), data, mem_mask);
	m_rombank = 1;
}

/* selects lower bank for the blitter */
WRITE16_MEMBER(ttchamp_state::port62_w)
{
	//printf("%06x: port62_w %04x %04x\n", space.device().safe_pc(), data, mem_mask);
	m_rombank = 0;
}

static ADDRESS_MAP_START( ttchamp_io, AS_IO, 16, ttchamp_state )
	AM_RANGE(0x0000, 0x0001) AM_WRITENOP // startup only, nmi enable?

	AM_RANGE(0x0002, 0x0003) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0004, 0x0005) AM_READ_PORT("P1_P2")

	AM_RANGE(0x0006, 0x0007) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)

	AM_RANGE(0x0018, 0x0019) AM_READ(blit_start_r) // read before using bus write offset as blit parameters
	AM_RANGE(0x001e, 0x001f) AM_READ(port1e_r) // read before some blit operations (but not all)

	AM_RANGE(0x0008, 0x0009) AM_WRITE(paldat_w)
	AM_RANGE(0x000a, 0x000b) AM_WRITE(paloff_w) // bit 0x8000 sometimes gets set, why?

	AM_RANGE(0x0010, 0x0011) AM_WRITE(port10_w)

	AM_RANGE(0x0020, 0x0021) AM_WRITE(port20_w)

	AM_RANGE(0x0034, 0x0035) AM_READWRITE(pic_r, pic_w)

	AM_RANGE(0x0062, 0x0063) AM_WRITE(port62_w)

ADDRESS_MAP_END


static INPUT_PORTS_START(ttchamp)
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "0x000003" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(ttchamp_state::irq)/* right? */
{
	device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( ttchamp, ttchamp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(ttchamp_map)
	MCFG_CPU_IO_MAP(ttchamp_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", ttchamp_state,  irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024,1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DRIVER(ttchamp_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x400)

	MCFG_NVRAM_ADD_0FILL("backram")

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 8000000/8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

ROM_START( ttchamp )
	ROM_REGION16_LE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "2.bin", 0x000000, 0x080000,  CRC(6a6c6d75) SHA1(3742b82462176d77732a69e142db9e6f61f25dc5) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x080000,  CRC(6062c0b2) SHA1(c5f0ac58c847ce2588c805f40180f2586a6477b7) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) // read protected, only half the data is valid
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280,  BAD_DUMP CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END

ROM_START( ttchampa )
	ROM_REGION16_LE( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ttennis2.bin", 0x000000, 0x080000,  CRC(b060e72c) SHA1(376e71bb4b1687fec4b719cbc5a7b25b64d159ac) )
	ROM_LOAD16_BYTE( "ttennis3.bin", 0x000001, 0x080000,  CRC(33e085a8) SHA1(ea6af05690b4b0803c303a3c858df10e4d907fb1) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) // read protected, only half the data is valid
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280, BAD_DUMP CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END


// only the graphics differ between the two sets, code section is the same
GAME( 1995, ttchamp, 0,        ttchamp, ttchamp, driver_device, 0, ROT0,  "Gamart",                               "Table Tennis Champions", MACHINE_SUPPORTS_SAVE ) // this has various advertising boards, including 'Electronic Devices' and 'Deniam'
GAME( 1995, ttchampa,ttchamp,  ttchamp, ttchamp, driver_device, 0, ROT0,  "Gamart (Palencia Elektronik license)", "Table Tennis Champions (Palencia Elektronik license)", MACHINE_SUPPORTS_SAVE ) // this only has Palencia Elektronik advertising boards
