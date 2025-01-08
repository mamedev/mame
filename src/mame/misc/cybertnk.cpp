// license:LGPL-2.1+
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

Cyber Tank HW (c) 1987/1988 Coreland Technology

driver by Angelo Salese & David Haywood

Maybe it has some correlation with WEC Le Mans HW? (supposedly that was originally done by Coreland too)

TODO:
- improve sprite zooming
  Currently many sprites have an ugly 'bad' line at the top, the chances of this being caused by
  bad roms is very low because a single 8 pixel block of a sprite covers 4 roms, and it's only
  ever the top lines.  Do they have some special meaning, or does the zoom algorithm mean that
  those 'bad' lines would actually never be drawn?  Are the t1-t4 Roms related to the zoom?

- sprite shadows
  looks like sprites should have a shadow colour rather than the shadows being solid black, how
  is this marked? pen 0xe might be alpha blending related?

- verify on real hw that animation is 15fps


============================================================================================

(note: the following notes are from "PS")
- Communications
Master slave comms looks like shared RAM.
The slave tests these RAM banks all using the same routine at 0x000006E6
There are also routines for clearing each bank of RAM after POST,
including the shared RAM (0x).
The amount cleared in each bank is different than the amount tested.

IC20 (7D0*2 bytes starting @ 0x080000, code at 0x00000684) \ Tested as 0xFA0 (4000) bytes.
IC21 (7D0*2 bytes starting @ 0x080001, code at 0x00000690) / Cleared at 0x0000042a as 0x1000 bytes.
IC22 (1F4*2 bytes starting @ 0x0c0000, code at 0x0000069E) \ Tested as 0x3e8 (1000) bytes.
IC23 (1F4*2 bytes starting @ 0x0c0001, code at 0x000006AA) / Cleared at 0x00000440 as 0x4000 bytes.
IC24 (1F4*2 bytes starting @ 0x100001, code at 0x000006B8) > Shared RAM

The shared RAM is tested and cleared almost the same as the other RAM,
and is mapped into the master at E0000. Only every odd byte is used.

The first 0x10 bytes are used heavily as registers
during the POST, key ones are
    share_0001 hold - slave waits for this to be cleared.
    share_0009 master cmd to slave
    share_0011 slave status
    share_000b bit mask of failed ICs returned to master, defaults to all failed.

There are also block writes carried out by the slave every
second irq 3. The data transfer area appears to start at 0x100021.
Master reads at 0x00005E3C

It is tested as every odd byte from 0x100021 to 0x1003e8,
and cleared as every odd byte from 0x100021 up to 0x100fff)

- Unmapped reads/writes
CPU1 reads at 0x07fff8 and 0x07fffa are the slave reading validation values
to compare to slave program ROM checksums.
The test will never fail, the results of the comparison are ignored by the code,
so there may never have been an implementation.

CPU1 unmapped read at 0x20000 is a checksum overrun by a single loop iteration.
See loop at 0x000006D2, it's a "do while" loop that tests loop after testing ROM.

Unmapped read/write by CPU2 of 0xa005, 0xa006 This looks like loop overrun too,
or maybe caused by the initial base offset which is the same as the loop increments it.
Sub at CPU2:01B7, the block process starts at base 8020h and increments by 20h each time.
It overruns the top of RAM on the last iteration.

============================================================================================
Cyber Tank
Coreland Technology, Inc 1987

---------------------
BA87015

   SS2               -
   SS4               SS3
   -                 -
   -                 SS1

        Y8950             Y8950

    2064
    SS5

    Z80B
    3.5795MHz

---------------------
BA87035

        68000-10      20MHz    68000-10
        SUBH  SUBL             P2A  P1A
        2064  2064             2064 2064


 C04  C03  C02  C01
 C08  C07  C06  C05
 C12  C11  C10  C09
 C16  C15  C14  C13                        2016
                                           2016
                                        W31003
               SW1 SW2 SW3

---------------------
BA87034

  T2   T1                               43256   43256
              IC19  IC20                43256   43256
              IC29  IC30
                                        43256   43256
                                        43256   43256

                                        43256   43256
  T3                                    43256   43256
  T4
                                        43256   43256
                                        43256   43256



                                                  W31004
                                                  W31004
----------------------
BA87033

     22.8MHz    IC2                       2016     W31001
                           IC15           2016     W31004
                                          ROAD_CHL
                                          ROAD_CHH


                        S01
      2064              S02
      2064              S03                    T6          T5
    W31002       2064   S04      W31004

                        S05
                        S06
      2064              S07                    2064        2064
      2064       2064   S08      W31004        2064        2064
    W31002
                        S09
      2064              S10
      2064              S11
    W31002       2064   S12      W31004
                                                VID1CONN  VID2CONN
********************************************************************************************
M68k Master irq table:
lev 1 : 0x64 : 0000 0870 - vblank
lev 2 : 0x68 : 0000 0caa - input device clear?
lev 3 : 0x6c : 0000 0caa - input device clear?
lev 4 : 0x70 : 0000 0caa - input device clear?
lev 5 : 0x74 : 0000 0caa - input device clear?
lev 6 : 0x78 : 0000 0caa - input device clear?
lev 7 : 0x7c : ffff ffff - illegal

M68k Slave irq table:
lev 1 : 0x64 : 0000 07e0 - input device clear?
lev 2 : 0x68 : 0000 07e0 - input device clear?
lev 3 : 0x6c : 0000 0764 - vblank?
lev 4 : 0x70 : 0000 07e0 - input device clear?
lev 5 : 0x74 : 0000 07e0 - input device clear?
lev 6 : 0x78 : 0000 07e0 - input device clear?
lev 7 : 0x7c : 0000 07e0 - input device clear?

*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "layout/generic.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cybertnk_state : public driver_device
{
public:
	cybertnk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spr_ram(*this, "spr_ram"),
		m_vram(*this, "tilemap%u_vram", 0U),
		m_scroll(*this, "tilemap%u_scroll", 0U),
		m_roadram(*this, "roadram"),
		m_spr_gfx(*this, "spr_gfx"),
		m_traverse_io(*this, "TRAVERSE"),
		m_elevate_io(*this, "ELEVATE"),
		m_accel_io(*this, "ACCEL"),
		m_handle_io(*this, "HANDLE")
	{ }

	void init_cybertnk();

	void cybertnk(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spr_ram;
	required_shared_ptr_array<uint16_t, 3> m_vram;
	required_shared_ptr_array<uint16_t, 3> m_scroll;
	required_shared_ptr<uint16_t> m_roadram;

	required_memory_region m_spr_gfx;
	required_ioport m_traverse_io;
	required_ioport m_elevate_io;
	required_ioport m_accel_io;
	required_ioport m_handle_io;

	tilemap_t *m_tilemap[3];

	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t m_mux_data;
	void sound_cmd_w(offs_t offset, uint8_t data);
	void mux_w(offs_t offset, uint8_t data);
	uint8_t io_rdy_r();
	uint8_t mux_r();
	void irq_ack_w(offs_t offset, uint8_t data);
	void cnt_w(offs_t offset, uint8_t data);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	void draw_road(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift, int pri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift);
	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift);
	uint32_t screen_update_cybertnk_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cybertnk_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void master_mem(address_map &map) ATTR_COLD;
	void slave_mem(address_map &map) ATTR_COLD;
	void sound_mem(address_map &map) ATTR_COLD;
};

/* tile format

 1 word

 ---- ---- ---- ----
    t tttt tttt tttt  (0x1fff) tilenumber
 ppp                  (0xe000) LOWER 3 palette bits
    p pp              (0x1c00) UPPER 3 palette bits (overlap tilenumber!)

*/

template<int Layer>
TILE_GET_INFO_MEMBER(cybertnk_state::get_tile_info)
{
	int code = m_vram[Layer][tile_index];
	int pal = (code & 0xe000) >> 13;
	pal     |=(code & 0x1c00) >> 7;

	tileinfo.set(Layer,
			code & 0x1fff,
			pal,
			0);
}

void cybertnk_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cybertnk_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8,8,128,32);
	m_tilemap[0]->set_transparent_pen(0);

	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cybertnk_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8,8,128,32);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cybertnk_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 8,8,128,32);
	m_tilemap[2]->set_transparent_pen(0);
}



void cybertnk_state::draw_road(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift, int pri)
{
	gfx_element *gfx = m_gfxdecode->gfx(3);


	for (int i = 0; i < 0x1000/4; i+=4)
	{
		uint16_t param1 = m_roadram[i+2];
		uint16_t param2 = m_roadram[i+1];
		uint16_t param3 = m_roadram[i+0];

		int col = (param2 & 0x3f);

		// seems to be priority related, cases seen are 0xc0 and 0x00 (once the palette bits are masked out)
		if ((param2&0x80) == pri)
		{
			gfx->transpen(bitmap,cliprect,param1,col,0,0,-param3+screen_shift,i/4,0);
		}


	}
}

// check if these are similar / the same as weclemans
void cybertnk_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift)
{
	const uint32_t *sprrom = (uint32_t*)m_spr_gfx->base();
	const pen_t *paldata = m_palette->pens();

	int miny = cliprect.min_y;
	int maxy = cliprect.max_y;
	int minx = cliprect.min_x;
	int maxx = cliprect.max_x;
	uint16_t* dest;

	/*

	o = offset
	y = ypos  Y = ysize ^ = always 0xF?  ? = set for no obvious reason..
	x = xpos  X = xsize f = flipx

	Z = zoom   * = alt zoom? (ok for roadside, but 00 for player tank etc?)
	C = colour
	E = sprite enabled
	                        +word offset
	 CCCC CCCC #### Eooo   0x0  # bits are often set too?
	 oooo oooo oooo oooo   0x1
	 ---- ---- ---y yyyy   0x2
	 ---- ---- ---- ----   0x3 (always has a value here, gets set to FFFF on some cleared sprites?)
	 ???? ^^^^ YYYY YYYY   0x4
	 f--- --xx xxxx xxxx   0x5
	 ZZZZ ZZZZ ---- XXXX   0x6
	 ---- ---- **** ****   0x7

	*/


	for(int offs=0;offs<0x1000/2;offs+=8)
	{
		if ((m_spr_ram[offs+0x0] & 8) == 0)
			continue;

		int x = (m_spr_ram[offs+0x5] & 0x3ff);
		if (x&0x200) x-=0x400;

		int y = (m_spr_ram[offs+0x2] & 0x1ff);
		if (y&0x100) y-=0x200;


		uint32_t spr_offs = (((m_spr_ram[offs+0x0] & 7) << 16) | (m_spr_ram[offs+0x1]));
		int xsize = ((m_spr_ram[offs+0x6] & 0x000f)+1) << 3;
		int ysize = (m_spr_ram[offs+0x4] & 0x00ff)+1;
		int fx = (m_spr_ram[offs+0x5] & 0x8000) >> 15;
		int zoom = (m_spr_ram[offs+0x6] & 0xff00) >> 8;


		int col_bank = (m_spr_ram[offs+0x0] & 0xff00) >> 8;

		int xf = 0;
		int yf = 0;
		int xz = 0;
		int yz = 0;

		for(int yi = 0;yi < ysize;yi++)
		{
			xf = xz = 0;


			int yy = y+yz;

			if ((yy>=miny) && (yy<=maxy))
			{
				dest = &bitmap.pix(yy, 0);

				int start,end,inc;

				if (!fx)
				{
					start = 0;
					end = xsize;
					inc = 8;
				}
				else
				{
					start = xsize-8;
					end = -8;
					inc = -8;
				}

				for(int xi=start;xi != end;xi+=inc)
				{ // start x loop
					uint32_t color;

					color = sprrom[spr_offs+xi/8];

					uint16_t dot;
					int x_dec; //helpers

					int shift_pen = 0;

					x_dec = 0;

					// draw 8 pixels
					while(x_dec < 8)
					{
						if (!fx)
						{
							dot = (color >> shift_pen) & 0xf;
						}
						else
						{
							dot = (color >> (28-shift_pen)) & 0xf;
						}

						if (dot != 0)
						{
							int xx = (x+xz)+screen_shift;
							if ((xx>=minx) && (xx<=maxx))
							{
								dest[xx] = paldata[col_bank << 4 | dot];
							}
						}
						xf+=zoom;
						if(xf >= 0x100)
						{
							xz++;
							xf-=0x100;
						}
						else // next source pixel
						{
							shift_pen += 4;
							x_dec++;
							if(xf >= 0x80) { xz++; xf-=0x80; }
						}
					}
				} // end x loop
			}


			yf+=zoom;
			if(yf >= 0x100)
			{
				yi--;
				yz++;
				yf-=0x100;
			}
			else // next line
			{
				spr_offs += xsize/8;
				if(yf >= 0x80) { yz++; yf-=0x80; }
			}
		}
	}
}


uint32_t cybertnk_state::update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int screen_shift)
{
	m_tilemap[0]->set_scrolldx(screen_shift, screen_shift);
	m_tilemap[1]->set_scrolldx(screen_shift, screen_shift);
	m_tilemap[2]->set_scrolldx(screen_shift, screen_shift);

	m_tilemap[1]->set_scrolly(m_scroll[1][2]);
	m_tilemap[2]->set_scrolly(m_scroll[2][2]);

	m_tilemap[1]->set_scrollx(m_scroll[1][0]);
	m_tilemap[2]->set_scrollx(m_scroll[2][0]);



	bitmap.fill(m_palette->black_pen(), cliprect);


	draw_road(screen,bitmap,cliprect,screen_shift, 0x00);

	m_tilemap[2]->draw(screen, bitmap, cliprect, 0,0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0,0);

	draw_road(screen,bitmap,cliprect,screen_shift, 0x80);

	draw_sprites(screen,bitmap,cliprect,screen_shift);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0,0);


	return 0;
}

uint32_t cybertnk_state::screen_update_cybertnk_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, 0); }
uint32_t cybertnk_state::screen_update_cybertnk_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return update_screen(screen, bitmap, cliprect, -256); }

template<int Layer>
void cybertnk_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram[Layer][offset]);
	m_tilemap[Layer]->mark_tile_dirty(offset);
}


void cybertnk_state::sound_cmd_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		printf("sound_cmd_w offset 0 %02x\n", data);
	}
	else if (offset == 1)
	{
		m_soundlatch->write(data & 0xff);
	}
}


void cybertnk_state::mux_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
//      printf("mux_w offset 0 %02x\n", data);
	}
	else if (offset == 1)
	{
//      printf("mux_w offset 1 %02x\n", data);
		m_mux_data = data & 0x60;
		/* Other bits are unknown */
	}
}

uint8_t cybertnk_state::io_rdy_r()
{
	// bit 0: i/o controller busy?
	return 0;
}

uint8_t cybertnk_state::mux_r()
{
	switch (m_mux_data & 0x60)
	{
		case 0x00:
			return m_traverse_io->read();
			break;
		case 0x20:
			return m_elevate_io->read();
			break;
		case 0x40:
			return m_accel_io->read();
			break;
		case 0x60:
			return m_handle_io->read();
			break;
	}
	return 0;
}

/* Amusingly the data written here is pretty weird, it seems suited for an unused protection device (attract = coin count, in-game = return status of some inputs) */
void cybertnk_state::irq_ack_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		// unused?
		logerror("irq_ack_w offset 0 %02x\n", data);
	}
	else if (offset == 1)
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
	}
}

void cybertnk_state::cnt_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		// count counters / lamps?
		// writes 04 / 00 alternating during attract mode
		// writes 01 or 02 when coins are inserted depending on slot
	}
	else if (offset == 1)
	{
		// unused?
		logerror("cnt_w offset 1 %02x\n", data);
	}
}


void cybertnk_state::master_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x080000, 0x087fff).ram(); /*Work RAM*/
	map(0x0a0000, 0x0a0fff).ram().share("spr_ram"); // non-tile based sprite ram
	map(0x0c0000, 0x0c1fff).ram().w(FUNC(cybertnk_state::vram_w<0>)).share("tilemap0_vram");
	map(0x0c4000, 0x0c5fff).ram().w(FUNC(cybertnk_state::vram_w<1>)).share("tilemap1_vram");
	map(0x0c8000, 0x0c9fff).ram().w(FUNC(cybertnk_state::vram_w<2>)).share("tilemap2_vram");
	map(0x0e0000, 0x0e0fff).ram().share("sharedram");
	map(0x100000, 0x107fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); /* 2x palettes, one for each screen */

	map(0x110000, 0x110001).w(FUNC(cybertnk_state::sound_cmd_w));
	map(0x110002, 0x110003).portr("DSW1").nopw();// watchdog?
	map(0x110004, 0x110004).r(FUNC(cybertnk_state::io_rdy_r));
	map(0x110006, 0x110007).portr("IN0");
	map(0x110006, 0x110007).w(FUNC(cybertnk_state::mux_w));
	map(0x110008, 0x110009).portr("IN1").w(FUNC(cybertnk_state::cnt_w));
	map(0x11000a, 0x11000b).portr("DSW2");
	map(0x11000c, 0x11000d).w(FUNC(cybertnk_state::irq_ack_w));

	map(0x110040, 0x110045).ram().share("tilemap0_scroll");
	map(0x110048, 0x11004d).ram().share("tilemap1_scroll");
	map(0x110080, 0x110085).ram().share("tilemap2_scroll");

	map(0x1100d5, 0x1100d5).r(FUNC(cybertnk_state::mux_r));
}

void cybertnk_state::slave_mem(address_map &map)
{
	map(0x000000, 0x01ffff).rom();
	map(0x020000, 0x020001).nopr(); // POST debug?
	map(0x07fff8, 0x07fffd).nopr(); // POST debug?
	map(0x080000, 0x083fff).ram(); /*Work RAM*/
	map(0x0c0000, 0x0c0fff).ram().share("roadram");
	map(0x100000, 0x100fff).ram().share("sharedram");
	map(0x140000, 0x140003).noprw(); /*Watchdog? Written during loops and interrupts*/
}

void cybertnk_state::sound_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xa001).rw("ym1", FUNC(y8950_device::read), FUNC(y8950_device::write));
	map(0xa001, 0xa001).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xa005, 0xa006).noprw();
	map(0xc000, 0xc001).rw("ym2", FUNC(y8950_device::read), FUNC(y8950_device::write));
}

// Player 1 controls the Driving and the Cannons
// Player 2 controls the Machine Guns
static INPUT_PORTS_START( cybertnk )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Machine Gun 1 (Bomb)")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Cannon 1 (Bomb)")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Machine Gun 2 (Fire)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Cannon 2 (Fire)")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRAVERSE")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2) PORT_REVERSE PORT_NAME("P2 Machine Gun X")

	PORT_START("ELEVATE")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2) PORT_REVERSE PORT_NAME("P2 Machine Gun Y")

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1) PORT_NAME("P1 Accelerate")

	PORT_START("HANDLE")
	PORT_BIT( 0xff, 0x7f, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_PLAYER(1) PORT_REVERSE PORT_NAME("P1 Handle")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x0004, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(      0x000c, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
/*
+----------------+----------------------------+----------------------------+
|Difficulty Level|        Single Play         |          Pair Play         |
+----------------+----------------------------+----------------------------+
|                |  One Tank is increased at  |  One tank is increased at  |
|  Very Easy     |  every 500K.               |  500K and 1,500K, and      |
|                |                            |  every 1,000K thereafter.  |
+----------------+----------------------------+----------------------------+
|                |  One Tank is increased at  |  One tank is increased at  |
|    Easy        |  500K and 1,500K, and      |  500K and 2,000K, and      |
|                |  every 1,000K thereafter.  |  every 1,000K thereafter.  |
+----------------+----------------------------+----------------------------+
|                |  One Tank is increased at  |  One tank is increased at  |
|    Hard        |  500K and 2,000K, and      |  500K and 2,500K, and      |
|                |  every 1,000K thereafter.  |  every 1,000K thereafter.  |
+----------------+----------------------------+----------------------------+
|                |  One Tank is increased at  |  One tank is increased at  |
|  Very Hard     |  500K and 2,500K, and      |  500K and 3,000K, and      |
|                |  every 1,000K thereafter.  |  every 1,000K thereafter.  |
+----------------+----------------------------+----------------------------+
*/
	PORT_DIPNAME( 0x0010, 0x0000, "Coin B Value" )          PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0010, "Set by Dipswitches" )
	PORT_DIPSETTING(      0x0000, "Same Value as Coin A" )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:3") /* Manual states "Off Not Use" */
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0040, IP_ACTIVE_LOW, "SW2:2" )      /* Manual states "Off Not Use" */
	PORT_DIPNAME( 0x0080, 0x0080, "2 Credits to Start" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )           /* 2 credits to start single player, 3 credits to start Pair Play, 1 credit to continue (or add 2nd player) */

	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 1C_7C ) )

	PORT_START("DSW2")  /* Manual states "Not Use" */
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_BIT(     0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout tile_8x8x4 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4),RGN_FRAC(1,4),RGN_FRAC(2,4),RGN_FRAC(0,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const uint32_t xoffsets[] = { STEP1024(0,4) };
static const gfx_layout roadlayout =
{
	1024,1,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	EXTENDED_XOFFS,
	{ 0 },
	1024*4,
	xoffsets
};

static GFXDECODE_START( gfx_cybertnk )
	GFXDECODE_ENTRY( "tilemap0_gfx", 0, tile_8x8x4,     0x1400, 64 ) /*Pal offset???*/
	GFXDECODE_ENTRY( "tilemap1_gfx", 0, tile_8x8x4,     0x1800, 64 )
	GFXDECODE_ENTRY( "tilemap2_gfx", 0, tile_8x8x4,     0x1c00, 64 )
	GFXDECODE_ENTRY( "road_data", 0, roadlayout,        0x1000, 64 )
GFXDECODE_END

/* palette breakdown

 0x0000 - 0x0fff = sprites
 0x1000 - 0x13ff = road
 0x1400 - 0x17ff = tilemap0
 0x1800 - 0x1bff = tilemap1
 0x1c00 - 0x1fff = tilemap2

 0x2000 - 0x3fff = same but screen 2
*/


void cybertnk_state::cybertnk(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(20'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cybertnk_state::master_mem);
	m_maincpu->set_vblank_int("lscreen", FUNC(cybertnk_state::irq1_line_assert));

	m68000_device &slave(M68000(config, "slave", XTAL(20'000'000)/2));
	slave.set_addrmap(AS_PROGRAM, &cybertnk_state::slave_mem);
	slave.set_vblank_int("lscreen", FUNC(cybertnk_state::irq3_line_hold));

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &cybertnk_state::sound_mem);

	config.set_maximum_quantum(attotime::from_hz(60000)); //arbitrary value, needed to get the communication to work

	/* video hardware */
	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	lscreen.set_size(32*8, 32*8);
	lscreen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	lscreen.set_screen_update(FUNC(cybertnk_state::screen_update_cybertnk_left));
	lscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	rscreen.set_size(32*8, 32*8);
	rscreen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	rscreen.set_screen_update(FUNC(cybertnk_state::screen_update_cybertnk_right));
	rscreen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cybertnk);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x4000);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0, HOLD_LINE);

	// Split output per chip
	Y8950(config, "ym1", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "lspeaker", 1.0);

	Y8950(config, "ym2", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cybertnk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p1a.37",   0x00000, 0x20000, CRC(be1abd16) SHA1(6ad01516301b44899971000c36f7e21070c3d2da) )
	ROM_LOAD16_BYTE( "p2a.36",   0x00001, 0x20000, CRC(5290c89a) SHA1(5a11671505214c20770e2938dab1ee82a030b457) )

	ROM_REGION( 0x20000, "slave", 0 )
	ROM_LOAD16_BYTE( "subl",   0x00000, 0x10000, CRC(3814a2eb) SHA1(252800b21f5cfada34ef5208cda33088daab132b) )
	ROM_LOAD16_BYTE( "subh",   0x00001, 0x10000, CRC(1af7ad58) SHA1(450c65289729d74cd4d17e11be16469246e61b7d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ss5.37",    0x0000, 0x8000, CRC(c3ba160b) SHA1(cfbfcad443ff83cd4e707f045a650417aca03d85) )

	ROM_REGION( 0x40000, "ym1", ROMREGION_ERASEFF )
	ROM_LOAD( "ss1.10",    0x00000, 0x20000, CRC(27d1cf94) SHA1(26246f217192bcfa39692df6d388640d385e9ed9) )
	ROM_LOAD( "ss3.11",    0x20000, 0x20000, CRC(a327488e) SHA1(b55357101e392f50f0cf75cf496a3ff4b79b2633) )

	ROM_REGION( 0x80000, "ym2", ROMREGION_ERASEFF )
	ROM_LOAD( "ss2.31",    0x00000, 0x20000, CRC(27d1cf94) SHA1(26246f217192bcfa39692df6d388640d385e9ed9) )
	ROM_LOAD( "ss4.32",    0x20000, 0x20000, CRC(a327488e) SHA1(b55357101e392f50f0cf75cf496a3ff4b79b2633) )

	ROM_REGION( 0x40000, "tilemap0_gfx", 0 )
	ROM_LOAD( "s09", 0x00000, 0x10000, CRC(69e6470c) SHA1(8e7db6988366cae714fff72449623a7977af1db1) )
	ROM_LOAD( "s10", 0x10000, 0x10000, CRC(77230f44) SHA1(b79fc841fa784d23855e4085310cee435c11348f) )
	ROM_LOAD( "s11", 0x20000, 0x10000, CRC(bfda980d) SHA1(1f975fdd2cfdc345eeb03fbc26fc1be1b2d7737e) )
	ROM_LOAD( "s12", 0x30000, 0x10000, CRC(8a11fcfa) SHA1(a406ac9cf841dd9d829cb83bfe8feb5128a3e77e) )

	ROM_REGION( 0x40000, "tilemap2_gfx", 0 )
	ROM_LOAD( "s01", 0x00000, 0x10000, CRC(6513452c) SHA1(95ed2da8f90e16c50716011577606a7dc93ba65e) )
	ROM_LOAD( "s02", 0x10000, 0x10000, CRC(3a270e3b) SHA1(97c8282d4d782c9d2fcfb5e5dabbe1ca88978f5c) )
	ROM_LOAD( "s03", 0x20000, 0x10000, CRC(584eff66) SHA1(308ec058693ce3ce34b058a8dbeedf342134311c) )
	ROM_LOAD( "s04", 0x30000, 0x10000, CRC(51ba5402) SHA1(c4522c4562ce0514bef3257e323bcc255b635544) )

	ROM_REGION( 0x40000, "tilemap1_gfx", 0 )
	ROM_LOAD( "s05", 0x00000, 0x10000, CRC(bddb6008) SHA1(bacb822bac4893eee0648a19ce449e5559d32b5e) )
	ROM_LOAD( "s06", 0x10000, 0x10000, CRC(d65b0fa5) SHA1(ce398a52ad408778fd910c42a9618194b862becf) )
	ROM_LOAD( "s07", 0x20000, 0x10000, CRC(70220567) SHA1(44b48ded8581a6d78b27a3af833f62413ff31c76) )
	ROM_LOAD( "s08", 0x30000, 0x10000, CRC(988c4fcb) SHA1(68d32be70605ad5415f2b6aeabbd92e269f0c9af) )

	/* TODO: fix the rom loading accordingly*/
	ROM_REGION( 0x200000, "spr_gfx", 0 )
	ROM_LOAD32_BYTE( "c01.93" , 0x180001, 0x20000, CRC(b5ee3de2) SHA1(77b9a2818f36826891e510e8550f1025bacfa496) )
	ROM_LOAD32_BYTE( "c02.92" , 0x180000, 0x20000, CRC(1f857d79) SHA1(f410d50970c10814b80baab27cbe69965bf0ccc0) )
	ROM_LOAD32_BYTE( "c03.91" , 0x180003, 0x20000, CRC(d70a93e2) SHA1(e64bb10c58b27def4882f3006784be56de11b812) )
	ROM_LOAD32_BYTE( "c04.90" , 0x180002, 0x20000, CRC(04d6fdc2) SHA1(56f8091c1a010014e951f5f47084e1400006123e) )

	ROM_LOAD32_BYTE( "c05.102", 0x100001, 0x20000, CRC(3f537490) SHA1(12d6545d29dda9f88019040fa33c73a22a2a213b) )
	ROM_LOAD32_BYTE( "c06.101", 0x100000, 0x20000, CRC(ff69c6a4) SHA1(badd20d26ba771780aebf733e1fbd1d37aa66f9b) )
	ROM_LOAD32_BYTE( "c07.100", 0x100003, 0x20000, CRC(5e8eba75) SHA1(6d0c1916517802acf808c8edc8e0b6074bdc90be) )
	ROM_LOAD32_BYTE( "c08.98" , 0x100002, 0x20000, CRC(f0820ddd) SHA1(7fb6c7d66ff96148f14921bc8d0cc0c65ffce4c4) )

	ROM_LOAD32_BYTE( "c09.109", 0x080001, 0x20000, CRC(080f87c3) SHA1(aedebc22ff03d4cc710e71ca14e09c7808f59c72) )
	ROM_LOAD32_BYTE( "c10.108", 0x080000, 0x20000, CRC(777c6a62) SHA1(4684d1c5d88b37ecb20002b7aa4814bf566e7d4b) )
	ROM_LOAD32_BYTE( "c11.107", 0x080003, 0x20000, CRC(330ca5a1) SHA1(4409da231a5abcec8c7d2d66eefdfd2019a322db) )
	ROM_LOAD32_BYTE( "c12.106", 0x080002, 0x20000, CRC(c1ec8e61) SHA1(09f2f4ddc100e5675c9bd82c200718fb0b69655e) )

	ROM_LOAD32_BYTE( "c13.119", 0x000001, 0x20000, CRC(4e22a7e0) SHA1(69cc7dd528b8af0c28b448285768a3ed079099ba) )
	ROM_LOAD32_BYTE( "c14.118", 0x000000, 0x20000, CRC(bdbd6232) SHA1(94b0741d5eced558723dda32a89aa2b747cdcbbd) )
	ROM_LOAD32_BYTE( "c15.117", 0x000003, 0x20000, CRC(f163d768) SHA1(e54e31a6f956f7de52b59bcdd0cd4ac1662b5664) )
	ROM_LOAD32_BYTE( "c16.116", 0x000002, 0x20000, CRC(5e5017c4) SHA1(586cd729630f00cbaf10d1036edebed1672bc532) )

	ROM_REGION( 0x40000, "road_data", 0 )
	ROM_LOAD16_BYTE( "road_chl" , 0x000001, 0x20000, CRC(862b109c) SHA1(9f81918362218ddc0a6bf0a5317c5150e514b699) )
	ROM_LOAD16_BYTE( "road_chh" , 0x000000, 0x20000, CRC(9dedc988) SHA1(10bae1be0e35320872d4994f7e882cd1de988c90) )

	/* I think these are zoom tables etc.? */
	ROM_REGION( 0x30000, "user3", 0 )
	ROM_LOAD( "t1",   0x00000, 0x08000, CRC(24890512) SHA1(2a6c9d39ca0c1c8316e85d9f565f6b3922d596b2) ) // data repeated 4 times
	ROM_LOAD( "t2",   0x08000, 0x08000, CRC(5a10480d) SHA1(f17598442091dae14abe3505957d94793f3ed886) ) // data repeated 4 times
	ROM_LOAD( "t3",   0x10000, 0x08000, CRC(454af4dc) SHA1(e5b18a37715e50db2243432564f5a04fb39dea60) ) // data repeated 4 times
	ROM_LOAD( "t4",   0x18000, 0x08000, CRC(0e1ef6a9) SHA1(d230841bbee6d07bab05aa8d37ec2409fc6278bc) ) // data repeated 4 times

	ROM_REGION( 0x10000, "user4", 0 )
	/*The following two are identical*/
	ROM_LOAD( "t5",   0x00000, 0x08000, CRC(12eb51bc) SHA1(35708eb456207ebee498c70dd82340b364797c56) )
	ROM_LOAD( "t6",   0x08000, 0x08000, CRC(12eb51bc) SHA1(35708eb456207ebee498c70dd82340b364797c56) )

	ROM_REGION( 0x280, "proms", 0 )
	ROM_LOAD( "ic2",  0x0000, 0x0100, CRC(aad2a447) SHA1(a12923027e3093bd6d358af44d35d2e8e588dd1a) )//road proms related?
	ROM_LOAD( "ic15", 0x0100, 0x0100, CRC(5f8c2c00) SHA1(50162503ac0ee9395377d7e45a84672a9493fb7d) )
	ROM_LOAD( "ic19", 0x0200, 0x0020, CRC(bd15cd71) SHA1(e0946d12eebd5db8707d965be157914d70f7472b) )//T1-T6 proms related?
	ROM_LOAD( "ic20", 0x0220, 0x0020, CRC(2f237563) SHA1(b0081c1cc6e357a6f10ab1ff357bd4e989ec7fb3) )
	ROM_LOAD( "ic29", 0x0240, 0x0020, CRC(95b32c0f) SHA1(5a19f441ced983bacbf3bc1aaee94ca768166447) )
	ROM_LOAD( "ic30", 0x0260, 0x0020, CRC(2bb6033f) SHA1(eb994108734d7d04f8e293eca21bb3051a63cfe9) )
ROM_END

void cybertnk_state::init_cybertnk()
{
	uint32_t *spr = (uint32_t*)m_spr_gfx->base();
	for (int x = 0; x< 0x200000/4;x++)
	{
		// reorder the data to simplify sprite drawing
		// we draw 8 pixels at a time, each each nibble contains a pixel, however the original order of 32-bits (8 pixels)
		// is along the lines of 04 15 26 37 which is awkward to use
		spr[x] = bitswap<32>(spr[x],  27,26,25,24,   19,18,17,16,  11,10,9,8,  3,2,1,0, 31,30,29,28,   23,22,21,20,   15,14,13,12,   7,6,5,4 );
	}

}

} // anonymous namespace


GAME( 1988, cybertnk, 0, cybertnk, cybertnk, cybertnk_state, init_cybertnk, ROT0, "Coreland", "Cyber Tank (v1.4)", MACHINE_IMPERFECT_GRAPHICS )
