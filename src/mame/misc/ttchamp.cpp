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
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class ttchamp_state : public driver_device
{
public:
	ttchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")  { }

	void ttchamp(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;

	uint16_t m_paloff = 0;
	uint16_t m_port10 = 0;
	uint8_t m_rombank = 0;
	uint16_t m_videoram0[0x10000 / 2]{};
	uint16_t m_videoram2[0x10000 / 2]{};

	enum class picmode : u8
	{
		IDLE = 0,
		SET_READADDRESS = 1,
		SET_WRITEADDRESS = 2,
		SET_WRITELATCH = 3,
		SET_READLATCH = 4
	};

	picmode m_picmodex{};

	int m_pic_readaddr = 0;
	int m_pic_writeaddr = 0;
	int m_pic_latched = 0;
	int m_pic_writelatched = 0;

	std::unique_ptr<uint8_t[]> m_bakram;

	uint16_t m_mainram[0x10000 / 2];

	int m_spritesinit = 0;
	int m_spriteswidth = 0;
	int m_spritesaddr = 0;
	uint16_t* m_rom16 = nullptr;
	uint8_t* m_rom8 = nullptr;

	void paloff_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void paldat_w(uint16_t data);

	void port10_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void port20_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void port62_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t port1e_r();

	uint16_t pic_r(offs_t offset, uint16_t mem_mask = ~0);
	void pic_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t blit_start_r();

	uint16_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(irq);
	void ttchamp_io(address_map &map) ATTR_COLD;
	void ttchamp_map(address_map &map) ATTR_COLD;
};

ALLOW_SAVE_TYPE(ttchamp_state::picmode);


void ttchamp_state::machine_start()
{
	m_rom16 = (uint16_t*)memregion("maincpu")->base();
	m_rom8 = memregion("maincpu")->base();

	m_picmodex = picmode::IDLE;

	m_bakram = std::make_unique<uint8_t[]>(0x100);
	subdevice<nvram_device>("backram")->set_base(m_bakram.get(), 0x100);

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

uint32_t ttchamp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	logerror("update\n");
	int count;

	static const int xxx=320,yyy=204;

	bitmap.fill(m_palette->black_pen());
	uint8_t const *const videoramfg = (uint8_t*)m_videoram2;
	uint8_t const *const videorambg = (uint8_t*)m_videoram0;

	count=0;

	for (int y=0;y<yyy;y++)
	{
		for(int x=0;x<xxx;x++)
		{
			bitmap.pix(y, x) = videorambg[BYTE_XOR_LE(count)]+0x300;
			count++;
		}
	}

#if 0
	count=0;
	videoram = (uint8_t*)m_videoram1;
	for (int y=0;y<yyy;y++)
	{
		for (int x=0;x<xxx;x++)
		{
			uint8_t pix = videoram[BYTE_XOR_LE(count)];
			if (pix) bitmap.pix(y, x) = pix+0x200;
			count++;
		}
	}
#endif

	count=0;
	for (int y=0;y<yyy;y++)
	{
		for(int x=0;x<xxx;x++)
		{
			uint8_t pix = videoramfg[BYTE_XOR_LE(count)];
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
					uint8_t pix = videorambg[BYTE_XOR_LE(count)];
					bitmap.pix(y, x) = pix + 0x200;
				}
				else if (pix == 0x02) // blend mode 2
				{
					uint8_t pix = videorambg[BYTE_XOR_LE(count)];
					bitmap.pix(y, x) = pix + 0x100;
				}
				else
				{
					bitmap.pix(y, x) = pix + 0x000;
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


void ttchamp_state::paloff_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_paloff);
}


void ttchamp_state::paldat_w(uint16_t data)
{
	// 0x8000 of offset is sometimes set
	m_palette->set_pen_color(m_paloff & 0x3ff,pal5bit(data>>0),pal5bit(data>>5),pal5bit(data>>10));
}

uint16_t ttchamp_state::pic_r(offs_t offset, uint16_t mem_mask)
{
//  printf("%06x: read from PIC (%04x)\n", m_maincpu->pc(),mem_mask);
	if (m_picmodex == picmode::SET_READLATCH)
	{
//      printf("read data %02x from %02x\n", m_pic_latched, m_pic_readaddr);
		m_picmodex = picmode::IDLE;

		return m_pic_latched << 8;

	}
	return 0x00;

}

void ttchamp_state::pic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  printf("%06x: write to PIC %04x (%04x) (%d)\n", m_maincpu->pc(),data,mem_mask, m_picmodex);
	if (m_picmodex == picmode::IDLE)
	{
		if (data == 0x11)
		{
			m_picmodex = picmode::SET_READADDRESS;
//          printf("state = SET_READADDRESS\n");
		}
		else if (data == 0x12)
		{
			m_picmodex = picmode::SET_WRITELATCH;
//          printf("latch write data.. \n" );
		}
		else if (data == 0x20)
		{
			m_picmodex = picmode::SET_WRITEADDRESS;
//          printf("state = picmode::SET_WRITEADDRESS\n");
		}
		else if (data == 0x21) // write latched data
		{
			m_picmodex = picmode::IDLE;
			m_bakram[m_pic_writeaddr] = m_pic_writelatched;
	//      printf("wrote %02x to %02x\n", m_pic_writelatched, m_pic_writeaddr);
		}
		else if (data == 0x22) // next data to latch
		{
			// why does it read twice as many addresses, forcing us to shift the
			// address by 1 to give correct results? maybe it can read 'previous' data' too?
			m_pic_latched = m_bakram[m_pic_readaddr>>1];

//          printf("latch read data %02x from %02x\n",m_pic_latched, m_pic_readaddr );
			m_picmodex = picmode::SET_READLATCH; // waiting to read...
		}
		else
		{
//          printf("unknown\n");
		}
	}
	else if (m_picmodex == picmode::SET_READADDRESS)
	{
		m_pic_readaddr = data;
		m_picmodex = picmode::IDLE;
	}
	else if (m_picmodex == picmode::SET_WRITEADDRESS)
	{
		m_pic_writeaddr = data;
		m_picmodex = picmode::IDLE;
	}
	else if (m_picmodex == picmode::SET_WRITELATCH)
	{
		m_pic_writelatched = data;
		m_picmodex = picmode::IDLE;
	}

}


uint16_t ttchamp_state::mem_r(offs_t offset)
{
	// bits 0xf0 are used too, so this is likely wrong.

	uint16_t* vram;
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
		uint16_t *src = m_rom16 + (0x100000/2); // can the CPU ever see the lower bank?
		return src[offset];
	}
}

void ttchamp_state::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// this is very strange, we use the offset (address bits) not data bits to set values..
	// I get the impression this might actually overlay the entire address range, including RAM and regular VRAM?

	// bits 0xf0 are used too, so this is likely wrong.

	uint16_t* vram;
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
	//  printf("%06x: spider_blitter_w %08x %04x %04x (init?) (base?)\n", m_maincpu->pc(), offset * 2, data, mem_mask);

		m_spritesinit = 2;
		m_spritesaddr = offset;

	}
	else if (m_spritesinit == 2)
	{
	//  printf("%06x: spider_blitter_w %08x %04x %04x (init2) (width?)\n", m_maincpu->pc(), offset * 2, data, mem_mask);
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

			uint8_t *src = m_rom8;

			if (m_rombank)
				src += 0x100000;

		//  printf("%06x: spider_blitter_w %08x %04x %04x (previous data width %d address %08x)\n", m_maincpu->pc(), offset * 2, data, mem_mask, m_spriteswidth, m_spritesaddr);
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
					uint8_t data;

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
			printf("%06x: spider_blitter_w unhandled RAM access %08x %04x %04x\n", m_maincpu->pc(), offset * 2, data, mem_mask);
		}
	}
}



void ttchamp_state::ttchamp_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(ttchamp_state::mem_r), FUNC(ttchamp_state::mem_w));
}

/* Re-use same parameters as before (one-shot) */
uint16_t ttchamp_state::port1e_r()
{
	m_spritesinit = 3;
	return 0xff;
}

uint16_t ttchamp_state::blit_start_r()
{
	m_spritesinit = 1;
	return 0xff;
}

/* blitter mode select */
void ttchamp_state::port10_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	 --xx ---- fill enable
	 ---- --x- opacity enable (Gamart logo)
	 ---- ---x layer select
	*/
	COMBINE_DATA(&m_port10);
}

/* selects upper bank for the blitter */
void ttchamp_state::port20_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("%06x: port20_w %04x %04x\n", m_maincpu->pc(), data, mem_mask);
	m_rombank = 1;
}

/* selects lower bank for the blitter */
void ttchamp_state::port62_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	//printf("%06x: port62_w %04x %04x\n", m_maincpu->pc(), data, mem_mask);
	m_rombank = 0;
}

void ttchamp_state::ttchamp_io(address_map &map)
{
	map(0x0000, 0x0001).nopw(); // startup only, nmi enable?

	map(0x0002, 0x0003).portr("SYSTEM");
	map(0x0004, 0x0005).portr("P1_P2");

	map(0x0006, 0x0006).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x0018, 0x0019).r(FUNC(ttchamp_state::blit_start_r)); // read before using bus write offset as blit parameters
	map(0x001e, 0x001f).r(FUNC(ttchamp_state::port1e_r)); // read before some blit operations (but not all)

	map(0x0008, 0x0009).w(FUNC(ttchamp_state::paldat_w));
	map(0x000a, 0x000b).w(FUNC(ttchamp_state::paloff_w)); // bit 0x8000 sometimes gets set, why?

	map(0x0010, 0x0011).w(FUNC(ttchamp_state::port10_w));

	map(0x0020, 0x0021).w(FUNC(ttchamp_state::port20_w));

	map(0x0034, 0x0035).rw(FUNC(ttchamp_state::pic_r), FUNC(ttchamp_state::pic_w));

	map(0x0062, 0x0063).w(FUNC(ttchamp_state::port62_w));

}


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
	device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void ttchamp_state::ttchamp(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ttchamp_state::ttchamp_map);
	m_maincpu->set_addrmap(AS_IO, &ttchamp_state::ttchamp_io);
	m_maincpu->set_vblank_int("screen", FUNC(ttchamp_state::irq));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024,1024);
	screen.set_visarea(0, 320-1, 0, 200-1);
	screen.set_screen_update(FUNC(ttchamp_state::screen_update));
	screen.set_palette(m_palette);
	PALETTE(config, m_palette).set_entries(0x400);

	NVRAM(config, "backram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 8000000/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

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
GAME( 1995, ttchamp, 0,        ttchamp, ttchamp, ttchamp_state, empty_init, ROT0,  "Gamart",                               "Table Tennis Champions",                               MACHINE_SUPPORTS_SAVE ) // this has various advertising boards, including 'Electronic Devices' and 'Deniam'
GAME( 1995, ttchampa,ttchamp,  ttchamp, ttchamp, ttchamp_state, empty_init, ROT0,  "Gamart (Palencia Elektronik license)", "Table Tennis Champions (Palencia Elektronik license)", MACHINE_SUPPORTS_SAVE ) // this only has Palencia Elektronik advertising boards
