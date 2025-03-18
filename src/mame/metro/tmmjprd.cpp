// license:BSD-3-Clause
// copyright-holders:David Haywood
/* tmmjprd.cpp

 - split from rabbit.cpp (it uses the same GFX chip, but is otherwise a different PCB, and until the methods
   of configuring the graphic chip are understood it's easier to work on them here)

 - in 16x16 tile mode, the offset into tileram doesn't necessarily align to 16x16 tiles! This makes using the
   tilemap system excessively difficult, as it expects predecoded tiles which simply isn't possible here.
   This is used for the girls in the intro at least, they specify 16x16 tiles on non 16x16 boundaries.
   (basically the physical tile rom addressing doesn't change between modes even if the data type does)
   (handling this in the tilemap system is very messy, might just be best with custom renderer)

 - Rom Test not hooked up (can read the gfx roms via a BANK
    - should hook this up as a test to help determine if the tmpdoki roms should really be different

 - EEPROM might be wrong, pressing what might be coin3 causes the game to hang
   (see input ports)

 - Video has a 'blitter' but it isn't used by these games, it is used by Rabbit

 - sprites from one screen are overlapping on the other, probably there's a way to limit them to a single screen

 - priority is wrong.

*/


#include "emu.h"
#include "cpu/m68000/m68020.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "sound/i5000.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "layout/generic.h"


namespace {

#define EMULATE_BLITTER 0 // FIXME: code is incomplete

class tmmjprd_state : public driver_device
{
public:
	tmmjprd_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tilemap_regs(*this, "tilemap_regs.%u", 0),
		m_spriteregs(*this, "spriteregs"),
#if EMULATE_BLITTER
		m_blitterregs(*this, "blitterregs"),
#endif
		m_spriteram(*this, "spriteram"),
		m_gfxroms(*this, "gfx2"),
		m_pl1(*this, "PL1.%u", 1),
		m_pl2(*this, "PL2.%u", 1),
		m_system(*this, "SYSTEM")
	{ }

	void tmmjprd(machine_config &config);
	void tmpdoki(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint32_t, 4> m_tilemap_regs;
	required_shared_ptr<uint32_t> m_spriteregs;
#if EMULATE_BLITTER
	required_shared_ptr<uint32_t> m_blitterregs;
#endif
	required_shared_ptr<uint32_t> m_spriteram;

	required_region_ptr<uint8_t> m_gfxroms;

	required_ioport_array<5> m_pl1;
	required_ioport_array<5> m_pl2;
	required_ioport m_system;

	std::unique_ptr<uint32_t[]> m_tilemap_ram[4];
	uint8_t m_mux_data;
	uint8_t m_system_in;
	double m_old_brt[2];
#if EMULATE_BLITTER
	tilemap_t *m_tilemap[4];
	emu_timer *m_blit_done_timer;
#endif

	template<uint8_t Tilemap> void tilemap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<uint8_t Tilemap> uint32_t tilemap_r(offs_t offset);
	uint32_t randomtmmjprds();
	uint32_t mux_r();
	template<uint8_t Number> void brt_w(uint32_t data);
	void eeprom_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

#if EMULATE_BLITTER
	void blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void do_blit();
#endif

	uint32_t screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

#if EMULATE_BLITTER
	TIMER_CALLBACK_MEMBER(blit_done);
#endif

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int screen);
	void draw_tile(bitmap_ind16 &bitmap, const rectangle &cliprect, int x,int y,int sizex,int sizey, uint32_t tiledata, uint8_t* rom);
	void draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t*tileram, uint32_t*tileregs, uint8_t*rom );

	void main_map(address_map &map) ATTR_COLD;
};

template<uint8_t Tilemap>
void tmmjprd_state::tilemap_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tilemap_ram[Tilemap][offset]);
}

void tmmjprd_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int screen)
{
	int xpos,ypos;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	//  int todraw = (m_spriteregs[5]&0x0fff0000)>>16; // how many sprites to draw (start/end reg..) what is the other half?

//  uint32_t *source = (m_spriteram+ (todraw*2))-2;
//  uint32_t *finish = m_spriteram;

	uint32_t *source = m_spriteram+(0xc000/4)-2;
	uint32_t *finish = m_spriteram;
	int xoffs = (screen & 1)*320;

	for(;source>finish;source-=2)
	{
		if(screen != (source[0]&0x2000)>>13) continue; //screen flag
		if(!(source[0]&0x80000000)) continue; //disable flag

		/* Note: sprite-sprite priorities works as back-to-top but abs x/y calcs works as top-to-back */
		if(source[0]&0x40000000)
		{
			xpos = (source[0]&0x00000fff);
			ypos = (source[0]&0x0fff0000)>>16;
			if(xpos&0x800)xpos-=0x1000;
			if(ypos&0x800)ypos-=0x1000;
		}
		else
		{
			uint32_t *calc_abs;
			uint8_t calc_abs_helper;
			int abs_x = 0,abs_y = 0;

			calc_abs = source-2;

			/* TODO: wrong sprite positioning for the right screen */
			calc_abs_helper = 0;
			while(calc_abs > finish || calc_abs_helper == 0) //sanity check
			{
				if(calc_abs[0] & 0x40000000)
				{
					abs_x = (calc_abs[0]&0x00000fff);
					abs_y = (calc_abs[0]&0x0fff0000)>>16;

					if(abs_x & 0x800) abs_x-=0x1000;
					if(abs_y & 0x800) abs_y-=0x1000;

					calc_abs_helper = 1;
				}
				calc_abs-=2;
			}

			xpos = (source[0]&0x00000fff);
			ypos = (source[0]&0x0fff0000)>>16;

			if(xpos&0x800)xpos-=0x1000;
			if(ypos&0x800)ypos-=0x1000;
			xpos+=abs_x;
			ypos+=abs_y;
			xpos&=0x7ff;
			ypos&=0x7ff;
		}

		int xflip =  (source[0]&0x00008000)>>15;
		int yflip =  (source[0]&0x00004000)>>14;
		int tileno = (source[1]&0x0003ffff);
		int colr =   (source[1]&0x0ff00000)>>20;

		// the tiles in this are 8bpp, it can probably do the funky sub-tile addressing for them too tho..
		tileno >>=1;

		// 255 for 8bpp
		gfx->transpen(bitmap,cliprect,tileno,colr,!xflip,yflip,(xpos-xoffs)-8,(ypos)-8,255);
	}
}

void tmmjprd_state::draw_tile(bitmap_ind16 &bitmap, const rectangle &cliprect, int x,int y,int sizex,int sizey, uint32_t tiledata, uint8_t* rom)
{
	/* note, it's tile address _NOT_ tile number, 'sub-tile' access is possible, hence using the custom rendering */
	int tileaddr = (tiledata&0x000fffff)>>0;
	int colour   = (tiledata&0x0ff00000)>>20;
	int depth    = (tiledata&0x10000000)>>28;
	//int flipxy   = (tiledata&0x60000000)>>29;
	//                       0x80000000   (blank tile like metro.cpp?)
	int count;

	// entirely off right
	if (x > cliprect.max_x)
		return;

	// entirely off left
	if ((x+sizex) < cliprect.min_x)
		return;

	// entirely off bottom
	if (y > cliprect.max_y)
		return;

	// entirely off bottom
	if ((y+sizey) < cliprect.min_y)
		return;

	count = 0;
	for (int drawy=y;drawy<y+sizey;drawy++)
	{
		for (int drawx=x;drawx<x+sizex;drawx++)
		{
			if (!depth)
			{
				if (cliprect.contains(drawx, drawy))
				{
					uint16_t dat = (rom[(tileaddr*32)+count] & 0xf0)>>4;
					if (dat!=15)
					{
						//dat += (colour<<8);
						bitmap.pix(drawy, drawx) = dat;
					}
				}
				drawx++;
				if (cliprect.contains(drawx, drawy))
				{
					uint16_t dat = (rom[(tileaddr*32)+count] & 0x0f);
					if (dat!=15)
					{
						//dat += (colour<<8);
						bitmap.pix(drawy, drawx) = dat;
					}
				}

				count++;
			}
			else
			{
				if (cliprect.contains(drawx, drawy))
				{
					uint16_t dat = (rom[(tileaddr*32)+count] & 0xff);
					if (dat!=255)
					{
						dat += (colour<<8) & 0xf00;
						bitmap.pix(drawy, drawx) = dat;
					}
				}
				count++;
			}
		}
	}
}

void tmmjprd_state::draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t*tileram, uint32_t*tileregs, uint8_t*rom )
{
	int tilemap_sizex = 64;
	int tilemap_sizey = 64;

	int tile_sizex;
	int tile_sizey;

	int scrolly, scrollx;

	if (tileregs[0]&0x00400000)
	{
		tile_sizex = 16;
		tile_sizey = 16;
	}
	else
	{
		tile_sizex = 8;
		tile_sizey = 8;
	}

	scrolly = (tileregs[2] & 0xfff00000) >> 20;
	scrollx = (tileregs[2] & 0x0000fff0) >> 4;

	int count = 0;
	for (int y=0;y<tilemap_sizey;y++)
	{
		for (int x=0;x<tilemap_sizex;x++)
		{
			uint32_t tiledata = tileram[count];
			// todo: handle wraparound
			draw_tile(bitmap,cliprect,(x*tile_sizex)-scrollx,(y*tile_sizey)-scrolly,tile_sizex,tile_sizey, tiledata, rom);
			count++;
		}
	}

}

uint32_t tmmjprd_state::screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	draw_tilemap(bitmap, cliprect, m_tilemap_ram[3].get(), m_tilemap_regs[3], m_gfxroms );
	draw_sprites(bitmap,cliprect, 1);
	draw_tilemap(bitmap, cliprect, m_tilemap_ram[2].get(), m_tilemap_regs[2], m_gfxroms );

	/*
	popmessage("%08x %08x %08x %08x %08x %08x",
	m_tilemap_regs[2][0],
	m_tilemap_regs[2][1],
	m_tilemap_regs[2][2],
	m_tilemap_regs[2][3],
	m_tilemap_regs[2][4],
	m_tilemap_regs[2][5]);
	*/

/*
    popmessage("%08x %08x %08x %08x %08x %08x %08x",
    m_spriteregs[0],
    m_spriteregs[1],
    m_spriteregs[2],
    m_spriteregs[3],
    m_spriteregs[4],
    m_spriteregs[5],
    m_spriteregs[6]);
*/

	return 0;
}

uint32_t tmmjprd_state::screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	draw_tilemap(bitmap, cliprect, m_tilemap_ram[1].get(), m_tilemap_regs[1], m_gfxroms );
	draw_sprites(bitmap,cliprect, 0);
	draw_tilemap(bitmap, cliprect, m_tilemap_ram[0].get(), m_tilemap_regs[0], m_gfxroms );

	return 0;
}

void tmmjprd_state::video_start()
{
	/* the tilemaps are bigger than the regions the cpu can see, need to allocate the ram here */
	/* or maybe not for this game/hw .... */
	m_tilemap_ram[0] = make_unique_clear<uint32_t[]>(0x8000);
	m_tilemap_ram[1] = make_unique_clear<uint32_t[]>(0x8000);
	m_tilemap_ram[2] = make_unique_clear<uint32_t[]>(0x8000);
	m_tilemap_ram[3] = make_unique_clear<uint32_t[]>(0x8000);


	save_pointer(NAME(m_tilemap_ram[0]), 0x8000);
	save_pointer(NAME(m_tilemap_ram[1]), 0x8000);
	save_pointer(NAME(m_tilemap_ram[2]), 0x8000);
	save_pointer(NAME(m_tilemap_ram[3]), 0x8000);

	save_item(NAME(m_old_brt));
}

template<uint8_t Tilemap>
uint32_t tmmjprd_state::tilemap_r(offs_t offset)
{
	return m_tilemap_ram[Tilemap][offset];
}

uint32_t tmmjprd_state::randomtmmjprds()
{
	return 0x0000;//machine().rand();
}


#define BLITCMDLOG 0
#define BLITLOG 0

#if EMULATE_BLITTER
TIMER_CALLBACK_MEMBER(tmmjprd_state::blit_done)
{
	m_maincpu->set_input_line(3, HOLD_LINE);
}

void tmmjprd_state::do_blit()
{
	uint8_t *blt_data = memregion("gfx1")->base();
	int blt_source = (m_blitterregs[0]&0x000fffff)>>0;
	int blt_column = (m_blitterregs[1]&0x00ff0000)>>16;
	int blt_line   = (m_blitterregs[1]&0x000000ff);
	int blt_tilemp = (m_blitterregs[2]&0x0000e000)>>13;
	int blt_oddflg = (m_blitterregs[2]&0x00000001)>>0;
	int mask,shift;


	if(BLITCMDLOG) osd_printf_debug("BLIT command %08x %08x %08x\n", m_blitterregs[0], m_blitterregs[1], m_blitterregs[2]);

	if (blt_oddflg&1)
	{
		mask = 0xffff0000;
		shift= 0;
	}
	else
	{
		mask = 0x0000ffff;
		shift= 16;
	}

	blt_oddflg>>=1; /* blt_oddflg is now in dword offsets*/
	blt_oddflg+=0x80*blt_line;

	blt_source<<=1; /* blitsource is in word offsets */

	while(1)
	{
		int blt_commnd = blt_data[blt_source+1];
		int blt_amount = blt_data[blt_source+0];
		int blt_value;
		int loopcount;
		int writeoffs;
		blt_source+=2;

		switch (blt_commnd)
		{
			case 0x00: /* copy nn bytes */
				if (!blt_amount)
				{
					if(BLITLOG) osd_printf_debug("end of blit list\n");
					m_blit_done_timer->adjust(attotime::from_usec(500));
					return;
				}

				if(BLITLOG) osd_printf_debug("blit copy %02x bytes\n", blt_amount);
				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
					blt_source+=2;
					writeoffs=blt_oddflg+blt_column;
					m_tilemap_ram[blt_tilemp][writeoffs]=(m_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					m_tilemap[blt_tilemp]->mark_tile_dirty(writeoffs);

					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x02: /* fill nn bytes */
				if(BLITLOG) osd_printf_debug("blit fill %02x bytes\n", blt_amount);
				blt_value = ((blt_data[blt_source+1]<<8)|(blt_data[blt_source+0]));
				blt_source+=2;

				for (loopcount=0;loopcount<blt_amount;loopcount++)
				{
					writeoffs=blt_oddflg+blt_column;
					m_tilemap_ram[blt_tilemp][writeoffs]=(m_tilemap_ram[blt_tilemp][writeoffs]&mask)|(blt_value<<shift);
					m_tilemap[blt_tilemp]->mark_tile_dirty(writeoffs);
					blt_column++;
					blt_column&=0x7f;
				}

				break;

			case 0x03: /* next line */
				if(BLITLOG) osd_printf_debug("blit: move to next line\n");
				blt_column = (m_blitterregs[1]&0x00ff0000)>>16; /* --CC---- */
				blt_oddflg+=128;
				break;

			default: /* unknown / illegal */
				if(BLITLOG) osd_printf_debug("unknown blit command %02x\n",blt_commnd);
				break;
		}
	}

}



void tmmjprd_state::blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_blitterregs[offset]);

	if (offset == 0x0c/4)
	{
		do_blit();
	}
}
#endif

void tmmjprd_state::machine_start()
{
#if EMULATE_BLITTER
	m_blit_done_timer = timer_alloc(FUNC(tmmjprd_state::blit_done), this);
#endif

	save_item(NAME(m_mux_data));
	save_item(NAME(m_system_in));
}

void tmmjprd_state::eeprom_write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// don't disturb the EEPROM if we're not actually writing to it
	// (in particular, data & 0x100 here with mask = ffff00ff looks to be the watchdog)
	if (mem_mask == 0x000000ff)
		m_mux_data = (~data & 0xff);

	if (mem_mask == 0xff000000)
	{
		// latch the bit
		m_eeprom->di_write((data & 0x01000000) >> 24);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x04000000) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x02000000) ? ASSERT_LINE : CLEAR_LINE );
	}
}

uint32_t tmmjprd_state::mux_r()
{
	m_system_in = m_system->read();

	switch(m_mux_data)
	{
		case 0x01: return (m_system_in & 0xff) | m_pl1[0]->read()<<8 | m_pl2[0]->read()<<16 | 0xff000000;
		case 0x02: return (m_system_in & 0xff) | m_pl1[1]->read()<<8 | m_pl2[1]->read()<<16 | 0xff000000;
		case 0x04: return (m_system_in & 0xff) | m_pl1[2]->read()<<8 | m_pl2[2]->read()<<16 | 0xff000000;
		case 0x08: return (m_system_in & 0xff) | m_pl1[3]->read()<<8 | m_pl2[3]->read()<<16 | 0xff000000;
		case 0x10: return (m_system_in & 0xff) | m_pl1[4]->read()<<8 | m_pl2[4]->read()<<16 | 0xff000000;
	}

	return (m_system_in & 0xff) | 0xffffff00;
}

static INPUT_PORTS_START( tmmjprd )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Left Screen Coin A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Left Screen Coin B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // might actually be coin 3 button.. but hangs the game? (eeprom issue?)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Right Screen Coin A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Right Screen Coin B") // might actually be service 1
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))   // CHECK!

	PORT_START("PL1.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //bet button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1.5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //bet button
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2.5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END




/* notice that data & 0x4 is always cleared on brt_1 and set on brt_2.        *
 * My wild guess is that bits 0,1 and 2 controls what palette entries to dim. */
template<uint8_t Number>
void tmmjprd_state::brt_w(uint32_t data)
{
	data>>=24;
	double brt = ((data & 0x78)>>3) / 15.0;
	int bank = data & 0x4 ? 0x800 : 0; //guess

	if(data & 0x80 && m_old_brt[Number] != brt)
	{
		m_old_brt[Number] = brt;
		for (int i = bank; i < 0x800+bank; i++)
			m_palette->set_pen_contrast(i, brt);
	}
}


void tmmjprd_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200010, 0x200013).r(FUNC(tmmjprd_state::randomtmmjprds)); // gfx chip status?
	/* check these are used .. */
//  map(0x200010, 0x200013).writeonly().share("viewregs0");
	map(0x200100, 0x200117).writeonly().share(m_tilemap_regs[0]); // tilemap regs1
	map(0x200120, 0x200137).writeonly().share(m_tilemap_regs[1]); // tilemap regs2
	map(0x200140, 0x200157).writeonly().share(m_tilemap_regs[2]); // tilemap regs3
	map(0x200160, 0x200177).writeonly().share(m_tilemap_regs[3]); // tilemap regs4
	map(0x200200, 0x20021b).writeonly().share(m_spriteregs); // sprregs?
//  map(0x200300, 0x200303).w(FUNC(tmmjprd_state::rombank_w)); // used during rom testing, rombank/area select + something else?
	map(0x20040c, 0x20040f).w(FUNC(tmmjprd_state::brt_w<0>));
	map(0x200410, 0x200413).w(FUNC(tmmjprd_state::brt_w<1>));
//  map(0x200500, 0x200503).writeonly().share("viewregs7");
#if EMULATE_BLITTER
	map(0x200700, 0x20070f).w(FUNC(tmmjprd_state::blitter_w)).share(m_blitterregs);
#endif
//  map(0x200800, 0x20080f).writeonly().share("viewregs9"); // never changes?
	map(0x200900, 0x2009ff).rw("i5000snd", FUNC(i5000snd_device::read), FUNC(i5000snd_device::write));
	/* hmm */
//  map(0x279700, 0x279713).writeonly().share("viewregs10");
	/* tilemaps */
	map(0x280000, 0x283fff).rw(FUNC(tmmjprd_state::tilemap_r<0>), FUNC(tmmjprd_state::tilemap_w<0>));
	map(0x284000, 0x287fff).rw(FUNC(tmmjprd_state::tilemap_r<1>), FUNC(tmmjprd_state::tilemap_w<1>));
	map(0x288000, 0x28bfff).rw(FUNC(tmmjprd_state::tilemap_r<2>), FUNC(tmmjprd_state::tilemap_w<2>));
	map(0x28c000, 0x28ffff).rw(FUNC(tmmjprd_state::tilemap_r<3>), FUNC(tmmjprd_state::tilemap_w<3>));
	/* ?? is palette ram shared with sprites in this case or just a different map */
	map(0x290000, 0x29bfff).ram().share(m_spriteram);
	map(0x29c000, 0x29ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");

	map(0x400000, 0x400003).r(FUNC(tmmjprd_state::mux_r)).w(FUNC(tmmjprd_state::eeprom_write));
	map(0xf00000, 0xffffff).ram();
}



static const gfx_layout sprite_16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{
	40, 32, RGN_FRAC(1,2)+40,RGN_FRAC(1,2)+32,
	56, 48,RGN_FRAC(1,2)+56, RGN_FRAC(1,2)+48,
	8, 0,RGN_FRAC(1,2)+8   , RGN_FRAC(1,2)+0,
	24, 16,RGN_FRAC(1,2)+24, RGN_FRAC(1,2)+16,
	},


	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,8*64,9*64,10*64,11*64,12*64,13*64,14*64,15*64 },
	16*64
};


// gfx decoding is ugly.. 16*16 tiles can start at varying different offsets..
static GFXDECODE_START( gfx_tmmjprd )
	/* this seems to be sprites */
//  GFXDECODE_ENTRY( "gfx1", 0, sprite_8x8x4_layout,   0x0, 0x1000  )
//  GFXDECODE_ENTRY( "gfx1", 0, sprite_16x16x4_layout, 0x0, 0x1000  )
//  GFXDECODE_ENTRY( "gfx1", 0, sprite_8x8x8_layout,   0x0, 0x1000  )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_16x16x8_layout, 0x0, 0x10  )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(tmmjprd_state::scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line(5, HOLD_LINE);

	if(scanline == 736) // blitter irq?
		m_maincpu->set_input_line(3, HOLD_LINE);

}

void tmmjprd_state::tmpdoki(machine_config &config)
{
	M68EC020(config, m_maincpu, 24000000); /* 24 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &tmmjprd_state::main_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(tmmjprd_state::scanline), "lscreen", 0, 1);

	EEPROM_93C46_16BIT(config, m_eeprom, eeprom_serial_streaming::ENABLE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tmmjprd);
	PALETTE(config, m_palette).set_format(palette_device::xGRB_888, 0x1000);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(64*16, 64*16);
	lscreen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	//lscreen.set_visarea(0*8, 64*16-1, 0*8, 64*16-1);
	lscreen.set_screen_update(FUNC(tmmjprd_state::screen_update_left));
	lscreen.set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	i5000snd_device &i5000snd(I5000_SND(config, "i5000snd", XTAL(40'000'000)));
	i5000snd.add_route(0, "rspeaker", 1.0);
	i5000snd.add_route(1, "lspeaker", 1.0);
}

void tmmjprd_state::tmmjprd(machine_config &config)
{
	tmpdoki(config);

	config.set_default_layout(layout_dualhsxs);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(64*16, 64*16);
	rscreen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	//rscreen.set_visarea(0*8, 64*16-1, 0*8, 64*16-1);
	rscreen.set_screen_update(FUNC(tmmjprd_state::screen_update_right));
	rscreen.set_palette(m_palette);
}

ROM_START( tmmjprd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "p00.bin", 0x000000, 0x080000, CRC(a1efd960) SHA1(7f41ab58de32777bccbfe28e6e5a1f2dca35bb90) )
	ROM_LOAD32_BYTE( "p01.bin", 0x000001, 0x080000, CRC(9c325374) SHA1(1ddf1c292fc1bcf4dcefb5d4aa3abdeb1489c020) )
	ROM_LOAD32_BYTE( "p02.bin", 0x000002, 0x080000, CRC(729a5f12) SHA1(615704d36afdceb4b1ff2e5dc34856e614181e16) )
	ROM_LOAD32_BYTE( "p03.bin", 0x000003, 0x080000, CRC(595615ab) SHA1(aca746d74aa6e7e856eb5c9b740d884778743b27) )

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprite Roms */
	ROM_LOAD32_WORD( "00.bin", 0x1000002, 0x400000, CRC(303e91a1) SHA1(c29a22061ab8af8b72e0e6bdb36915a0cb5b2a5c) )
	ROM_LOAD32_WORD( "01.bin", 0x0000002, 0x400000, CRC(3371b775) SHA1(131dd850bd01dac52fa82c41948d900c4833db3c) )
	ROM_LOAD32_WORD( "02.bin", 0x1000000, 0x400000, CRC(4c1e13b9) SHA1(d244eb74f755350604824670db58ab2a56a856cb) )
	ROM_LOAD32_WORD( "03.bin", 0x0000000, 0x400000, CRC(9cf86152) SHA1(e27e0d9befb12ad5c2acf547afe80d1c7921a4d1) )
	ROM_LOAD32_WORD( "10.bin", 0x1800002, 0x400000, CRC(5ab6af41) SHA1(e29cee23c84e17dd8dabd2ec71e622c25418646e) )
	ROM_LOAD32_WORD( "11.bin", 0x0800002, 0x400000, CRC(1d1fd633) SHA1(655be5b72bb70a90d23e49512ca84d9978d87b0b) )
	ROM_LOAD32_WORD( "12.bin", 0x1800000, 0x400000, CRC(5b8bb9d6) SHA1(ee93774077d8a2ddcf70869a9c2f4961219a85b4) )
	ROM_LOAD32_WORD( "13.bin", 0x0800000, 0x400000, CRC(d950df0a) SHA1(3b109341ab4ad87005113fb481b5d1ed9a82f50f) )

	ROM_REGION( 0x2000000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD32_WORD( "40.bin", 0x0000000, 0x400000, CRC(8bedc606) SHA1(7159c8b86e8d7d5ae202c239638483ccdc7dfc25) )
	ROM_LOAD32_WORD( "41.bin", 0x0000002, 0x400000, CRC(e19713dd) SHA1(a8f1b716913f2e391abf277e5bf0e9986cc75898) )
	ROM_LOAD32_WORD( "50.bin", 0x0800000, 0x400000, CRC(85ca9ce9) SHA1(c5a7270507522e11e9485196be325508846fda90) )
	ROM_LOAD32_WORD( "51.bin", 0x0800002, 0x400000, CRC(6ba1d2ec) SHA1(bbe7309b33f213c8cb9ab7adb3221ea79f89e8b0) )
	ROM_LOAD32_WORD( "60.bin", 0x1000000, 0x400000, CRC(7cb132e0) SHA1(f9c366befec46c7f6e307111a62eede029202b16) )
	ROM_LOAD32_WORD( "61.bin", 0x1000002, 0x400000, CRC(caa7e854) SHA1(592867e001abd0781f83a5124bf9aa62ad1aa7f3) )
	ROM_LOAD32_WORD( "70.bin", 0x1800000, 0x400000, CRC(9b737ae4) SHA1(0b62a90d42ace81ee32db073a57731a55a32f989) )
	ROM_LOAD32_WORD( "71.bin", 0x1800002, 0x400000, CRC(189f694e) SHA1(ad0799d4aadade51be38d824910d299257a758a3) )

	ROM_REGION( 0x400000, "i5000snd", 0 ) /* Sound Roms */
	ROM_LOAD( "21.bin", 0x0000000, 0x400000, CRC(bb5fa8da) SHA1(620e609b3e2524d06d58844625f186fd4682205f))
ROM_END

// single screen?
ROM_START( tmpdoki )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD32_BYTE( "u70_p0.bin", 0x000000, 0x080000, CRC(c0ee1942) SHA1(0cebc3e326d84e200c2399208d810c0ac767dbb4) )
	ROM_LOAD32_BYTE( "u72_p1.bin", 0x000001, 0x080000, CRC(3c1bc6f6) SHA1(7b3719d4bb52e45db793564b0ccee067fd7af4e4) )
	ROM_LOAD32_BYTE( "u71_p2.bin", 0x000002, 0x080000, CRC(f2091cce) SHA1(88c6822eb1546e914c2644264367e71fb2a82be3) )
	ROM_LOAD32_BYTE( "u73_p3.bin", 0x000003, 0x080000, CRC(cca8ef13) SHA1(d5b077f3d8d38262e69d058a7d61e4563332abce) )

	ROM_REGION( 0x2000000, "gfx1", 0 ) /* Sprite Roms */
	ROM_LOAD32_WORD( "00.bin", 0x1000002, 0x400000, CRC(303e91a1) SHA1(c29a22061ab8af8b72e0e6bdb36915a0cb5b2a5c) )
	ROM_LOAD32_WORD( "01.bin", 0x0000002, 0x400000, CRC(3371b775) SHA1(131dd850bd01dac52fa82c41948d900c4833db3c) )
	ROM_LOAD32_WORD( "02.bin", 0x1000000, 0x400000, CRC(4c1e13b9) SHA1(d244eb74f755350604824670db58ab2a56a856cb) )
	ROM_LOAD32_WORD( "03.bin", 0x0000000, 0x400000, CRC(9cf86152) SHA1(e27e0d9befb12ad5c2acf547afe80d1c7921a4d1) )
	ROM_LOAD32_WORD( "10.bin", 0x1800002, 0x400000, CRC(5ab6af41) SHA1(e29cee23c84e17dd8dabd2ec71e622c25418646e) )
	ROM_LOAD32_WORD( "11.bin", 0x0800002, 0x400000, CRC(1d1fd633) SHA1(655be5b72bb70a90d23e49512ca84d9978d87b0b) )
	ROM_LOAD32_WORD( "12.bin", 0x1800000, 0x400000, CRC(5b8bb9d6) SHA1(ee93774077d8a2ddcf70869a9c2f4961219a85b4) )
	ROM_LOAD32_WORD( "13.bin", 0x0800000, 0x400000, CRC(d950df0a) SHA1(3b109341ab4ad87005113fb481b5d1ed9a82f50f) )

	ROM_REGION( 0x2000000, "gfx2", 0 ) /* BG Roms */
	ROM_LOAD32_WORD( "40.bin", 0x0000000, 0x400000, CRC(8bedc606) SHA1(7159c8b86e8d7d5ae202c239638483ccdc7dfc25) )
	ROM_LOAD32_WORD( "41.bin", 0x0000002, 0x400000, CRC(e19713dd) SHA1(a8f1b716913f2e391abf277e5bf0e9986cc75898) )
	ROM_LOAD32_WORD( "50.bin", 0x0800000, 0x400000, CRC(85ca9ce9) SHA1(c5a7270507522e11e9485196be325508846fda90) )
	ROM_LOAD32_WORD( "51.bin", 0x0800002, 0x400000, CRC(6ba1d2ec) SHA1(bbe7309b33f213c8cb9ab7adb3221ea79f89e8b0) )

	/* I think these should be different, the game attempts to draw tiles from here for the title logo, but
	   the tiles are empty.  Once the ROM check is hooked up this will be easier to confirm */
	ROM_LOAD32_WORD( "60.bin", 0x1000000, 0x400000, BAD_DUMP CRC(7cb132e0) SHA1(f9c366befec46c7f6e307111a62eede029202b16) )
	ROM_LOAD32_WORD( "61.bin", 0x1000002, 0x400000, BAD_DUMP CRC(caa7e854) SHA1(592867e001abd0781f83a5124bf9aa62ad1aa7f3) )
	ROM_LOAD32_WORD( "70.bin", 0x1800000, 0x400000, BAD_DUMP CRC(9b737ae4) SHA1(0b62a90d42ace81ee32db073a57731a55a32f989) )
	ROM_LOAD32_WORD( "71.bin", 0x1800002, 0x400000, BAD_DUMP CRC(189f694e) SHA1(ad0799d4aadade51be38d824910d299257a758a3) )

	ROM_REGION( 0x400000, "i5000snd", 0 ) /* Sound Roms */
	ROM_LOAD( "21.bin", 0x0000000, 0x400000, CRC(bb5fa8da) SHA1(620e609b3e2524d06d58844625f186fd4682205f))
ROM_END

} // anonymous namespace


GAME( 1997, tmmjprd,       0, tmmjprd, tmmjprd, tmmjprd_state, empty_init, ROT0, "Media / Sonnet", "Tokimeki Mahjong Paradise - Dear My Love",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tmpdoki, tmmjprd, tmpdoki, tmmjprd, tmmjprd_state, empty_init, ROT0, "Media / Sonnet", "Tokimeki Mahjong Paradise - Doki Doki Hen", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // missing gfx due to wrong roms?
