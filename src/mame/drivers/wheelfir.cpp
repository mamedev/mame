// license:BSD-3-Clause
// copyright-holders:David Haywood,Tomasz Slanina
/****************************************************************

Wheels & Fire

driver by
 David Haywood
 Tomasz Slanina


TODO:
- sound comms
- eeprom
- various gfx size/pos glitches (gaps, extra rows of pixels here and there)
- real gfx zoom, based on the zoom params, not lookup table
- apply double buffering (sometimes gfx is displayed at y+256 every other frame (extra bits are currently masked out))
- fix road/sky (extra bits in the scroll reg. are there two bitmap buffers? )
- bitmap layer clearing
- fix wrong coords of sprites rendered into the bitmap layer (intro car)
- is toggle_bit really a vblank  bit ? or somethign else (blitter status ?)
- implement(and find) layer enable/disable bits




----------------------------------
Produttore  TCH
N.revisione E133



CPU

2x MC68HC000FN16

2x TPC1020BFN-084C
1x BT478KPJ50-615-9132F
1x S9530AG-ADC0808-CCV
1x oscillator 32.000000MHz
1x ST93C46 EEPROM

ROMs

12x TMS27C040

1x GAL16V8QS
2x GAL22V10

Note

1x JAMMA connector
1x VGA connector - Used to link 2 PCBs together
2x trimmer (volume)


----

uses a blitter for the gfx, this is not fully understood...

level 5 interrupt = raster interrupt, used for road
level 3 interrupt = vblank interrupt
level 1 interrupt = blitter interrupt


Blitter data foramt ( offset in words, offset in bytes, offset inside ram data table )


        fedcba9876543210

0  0  0
        --------76543210    dest_x0 bits 0-7
        76543210--------    src_x0  bits 0-7

1  2  2
        --------76543210    dest_x1 bits 0-7



2  4  4
        --------76543210    dest_y0 bits 0-7
        76543210--------    src_y0  bits 0-7
3  6  6
        --------76543210    dest_y1 bits 0-7

4  8
5  a
6  c  8
        ??------????????    image flags (directly copied from image info table, page and ?)
        --3210----------    image page
        -------8--------    src_x0 bit 8
        ------8---------    src_y0 bit 8
7  e  a
        ????????--?-----    flags
        ---------------X    X direction (src?)
        --------------Y-    Y direction (src?)
        ---------8------    dest_x0 bit 8
        --------8-------    dest_y0 bit 8
        -----------L----    dest layer
        ------------??--    unknown bits, set usually when rendering target = bitmap layer


8 10  c
        -------5--------    x scale data1 bit 5
        ------5---------    y scale data1 bit 5
        -----5----------    x scale data2 bit 5
        ----5-----------    y scala data2 bit 5
        ---D------------    x scale >200%
        --D-------------    y scale >200%
        -x--------------    X direction (dest?)
        Y---------------    Y direction (dest?)
        ---------8------    scroll x bit 8
        --------8-------    scroll y bit 8
        ----------?-----    set for road ? buffer num (is there double buffering ? or two bitmap layers?)

9 12  e
        ---------------H    x scale < 50%
        --------------H-    y scale < 50%
        -------------8--    dest_x1 bit 8
        ------------8---    dest_y1 bit 8


a 14 10
        ---43210--------    x scale data1 bits 0-4
        -4--------------    y scale data1 bit 4
        --------76543210    scroll x of bitmap layer

b 16 12
        ---43210--------    x scale data2 bits 0-4
        10--------------    y scale data1 bits 0-1
        --------76543210    scroll y of bitmap layer
c 18 14
        ---43210--------    y scale data2 bits 0-4
        32--------------    y scale data1 bits 2-3
d 1a
e 1c
f 1e




X Scale Table:
76543210

---ABCDE
---FGHIJ
--------
---K-L-M
-------N

Y Scale Table:
76543210

-A------
DE------
BC-FGHIJ
--K-L-M-
------N-


BITS ABCDE = DATA1 (0-31)
BITS FGHIJ = DATA2 (0-31)
BIT K - ( scale > 200% ) ? 1 : 0
BIT L - ( DATA2 != 0) ? 1 : 0  (or DATA2 MSB)
BIT M - ( DATA1 != 0) ? 1 : 0  (or DATA1 MSB)
BIT N - ( scale < 50% ) ? 1 : 0


pcb linking
-----------

suspicious code:


(write/read (exchange data))

0A535C: 48E7 6080                  movem.l D1-D2/A0, -(A7)
0A5360: 41F9 0078 0000             lea     $780000.l, A0
0A5366: D040                       add.w   D0, D0
0A5368: 3181 0000                  move.w  D1, (A0,D0.w)  ; write data

0A536C: 3239 007E 0000             move.w  $7e0000.l, D1
0A5372: 0801 000D                  btst    #$d, D1
0A5376: 67F4                       beq     $a536c

0A5378: 3230 0000                  move.w  (A0,D0.w), D1 ; read data
0A537C: 0241 00FF                  andi.w  #$ff, D1

(unk., maybe sync or config ?)

0A53C2: 2F08                       move.l  A0, -(A7)
0A53C4: 3F01                       move.w  D1, -(A7)
0A53C6: 41F9 0078 0000             lea     $780000.l, A0
0A53CC: D040                       add.w   D0, D0
0A53CE: 3180 0000                  move.w  D0, (A0,D0.w) ; write offset
0A53D2: 3180 0000                  move.w  D0, (A0,D0.w) ; again

0A53D6: 3239 007E 0000             move.w  $7e0000.l, D1
0A53DC: 0801 000D                  btst    #$d, D1   ; flag test
0A53E0: 67F4                       beq     $a53d6

0A53E2: 3030 0000                  move.w  (A0,D0.w), D0
0A53E6: 0240 00FF                  andi.w  #$ff, D0
0A53EA: 321F                       move.w  (A7)+, D1
0A53EC: 205F                       movea.l (A7)+, A0
0A53EE: 4E75                       rts


*/


#include "emu.h"
#include "machine/eepromser.h"
#include "cpu/m68000/m68000.h"
#include "sound/dac.h"

static const int ZOOM_TABLE_SIZE=1<<14;
static const int NUM_SCANLINES=256-8;
static const int NUM_VBLANK_LINES=8;
static const int LAYER_BG=0;
static const int LAYER_FG=1;
static const int NUM_COLORS=256;

struct scroll_info
{
		INT32 x,y,unkbits;
};


class wheelfir_state : public driver_device
{
public:
	wheelfir_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	INT32 *m_zoom_table;
	UINT16 *m_blitter_data;

	UINT8 *m_palette_ptr;
	INT32 m_palpos;

	INT32 m_current_scanline;
	scroll_info *m_scanlines;

	INT32 m_soundlatch;

	INT32 m_direct_write_x0;
	INT32 m_direct_write_x1;
	INT32 m_direct_write_y0;
	INT32 m_direct_write_y1;
	INT32 m_direct_write_idx;

	INT32 m_toggle_bit;
	INT16 m_scanline_cnt;


	bitmap_ind16 *m_tmp_bitmap[2];

	INT32 get_scale(INT32 index)
	{
		while(index<ZOOM_TABLE_SIZE)
		{
			if(m_zoom_table[index]>=0)
			{
				return m_zoom_table[index];
			}
			++index;
		}
		return 0;
	}
	DECLARE_READ16_MEMBER(wheelfir_status_r);
	DECLARE_WRITE16_MEMBER(wheelfir_scanline_cnt_w);
	DECLARE_WRITE16_MEMBER(wheelfir_blit_w);
	DECLARE_WRITE16_MEMBER(pal_reset_pos_w);
	DECLARE_WRITE16_MEMBER(pal_data_w);
	DECLARE_WRITE16_MEMBER(wheelfir_7c0000_w);
	DECLARE_WRITE16_MEMBER(wheelfir_snd_w);
	DECLARE_READ16_MEMBER(wheelfir_snd_r);
	DECLARE_WRITE16_MEMBER(coin_cnt_w);
	DECLARE_DRIVER_INIT(wheelfir);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_wheelfir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_wheelfir(screen_device &screen, bool state);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer_callback);
};


READ16_MEMBER(wheelfir_state::wheelfir_status_r)
{
/*
    fedcba9876543210
    x---------------  vblank ?
    --x-------------  ? must be 1
    --------------x-  ? eeprom
    ---------------x  ? eeprom

*/

	return m_toggle_bit| (machine().rand()&0x2000);
}

WRITE16_MEMBER(wheelfir_state::wheelfir_scanline_cnt_w)
{
	COMBINE_DATA(&m_scanline_cnt);
}


WRITE16_MEMBER(wheelfir_state::wheelfir_blit_w)
{
	COMBINE_DATA(&m_blitter_data[offset]);

	if(!ACCESSING_BITS_8_15 && offset==0x6)  //LSB only!
	{
		int x,y;


		int direct_width=m_direct_write_x1-m_direct_write_x0+1;
		int direct_height=m_direct_write_y1-m_direct_write_y0+1;

		int sixdat = data&0xff;

		if(direct_width>0 && direct_height>0)
		{
			x= m_direct_write_idx % direct_width;
			y = (m_direct_write_idx / direct_width) %direct_height;

			x+=m_direct_write_x0;
			y+=m_direct_write_y0;

			if(x<512 && y <512)
			{
				m_tmp_bitmap[LAYER_BG]->pix16(y, x) = sixdat;
			}
		}

		++m_direct_write_idx;

		return;

	}

	int yscroll=-1;
	int xscroll=-1;

	if(offset==0x0a && ACCESSING_BITS_0_7)
	{
		xscroll = (m_blitter_data[0xa]&0x00ff) | (m_blitter_data[0x8]&0x0040) << 2;
	}

	if(offset==0x0b && ACCESSING_BITS_0_7)
	{
		yscroll = (m_blitter_data[0xb]&0x00ff) | (m_blitter_data[0x8]&0x0080) << 1;
	}

	if(offset==0x8 && ACCESSING_BITS_0_7)
	{
		xscroll = (m_blitter_data[0xa]&0x00ff) | (m_blitter_data[0x8]&0x0040) << 2;
		yscroll = (m_blitter_data[0xb]&0x00ff) | (m_blitter_data[0x8]&0x0080) << 1;
	}

	if(xscroll>=0)
	{
		int scl=m_current_scanline>=NUM_SCANLINES?0:m_current_scanline;
		m_scanlines[scl].x=xscroll;
		m_scanlines[scl].unkbits=m_blitter_data[0x8]&0xff;
	}

	if(yscroll>=0)
	{
		int scl=m_current_scanline>=NUM_SCANLINES?0:m_current_scanline;
		m_scanlines[scl].y=yscroll;
		m_scanlines[scl].unkbits=m_blitter_data[0x8]&0xff;
	}


	if(offset==0xf && data==0xffff)
	{
		m_maincpu->set_input_line(1, HOLD_LINE);

		{
			UINT8 *rom = memregion("gfx1")->base();

			int width = m_screen->width();
			int height = m_screen->height();

			int src_x0=(m_blitter_data[0]>>8)+((m_blitter_data[6]&0x100)?256:0);
			int src_y0=(m_blitter_data[2]>>8)+((m_blitter_data[6]&0x200)?256:0);

			int dst_x0=(m_blitter_data[0]&0xff)+((m_blitter_data[7]&0x40)?256:0);
			int dst_y0=(m_blitter_data[2]&0xff)+((m_blitter_data[7]&0x80)?256:0);

			int dst_x1=(m_blitter_data[1]&0xff)+((m_blitter_data[9]&4)?256:0);
			int dst_y1=(m_blitter_data[3]&0xff)+((m_blitter_data[9]&8)?256:0);

			int x_dst_step=(m_blitter_data[7]&0x1)?1:-1;
			int y_dst_step=(m_blitter_data[7]&0x2)?1:-1;

			int x_src_step=(m_blitter_data[8]&0x4000)?1:-1;
			int y_src_step=(m_blitter_data[8]&0x8000)?1:-1;

			int page=((m_blitter_data[6])>>10)*0x40000;


			if(page>=0x400000) /* src set to  unav. page before direct write to the framebuffer */
			{
					m_direct_write_x0=dst_x0;
					m_direct_write_x1=dst_x1;
					m_direct_write_y0=dst_y0;
					m_direct_write_y1=dst_y1;
					m_direct_write_idx=0;

			}

			if(x_dst_step<0)
			{
				if(dst_x0<=dst_x1)
				{
					return;
				}

			}
			else
			{
				if(dst_x0>=dst_x1)
				{
					return;
				}

			}

			if(y_dst_step<0)
			{
				if(dst_y0<=dst_y1)
				{
					return;
				}
			}
			else
			{
				if(dst_y0>=dst_y1)
				{
					return;
				}

			}


			//additional checks

			int d1, d2, hflag, dflag, index;

			d1=((m_blitter_data[0x0a]&0x1f00)>>8);

			d2=((m_blitter_data[0x0b]&0x1f00)>>8);


			d1|=((m_blitter_data[0x8]&0x100)>>3);
			d2|=((m_blitter_data[0x8]&0x400)>>5);
			hflag=(m_blitter_data[0x9]&0x1)?1:0;
			dflag=(m_blitter_data[0x8]&0x1000)?1:0;
			index=d1|(d2<<6)|(hflag<<12)|(dflag<<13);


			float scale_x=get_scale(index);

			d1=((m_blitter_data[0x0b]&0xc000)>>14) |
				((m_blitter_data[0x0c]&0xc000)>>12) |
				((m_blitter_data[0x0a]&0x4000)>>10);

			d2=((m_blitter_data[0x0c]&0x1f00)>>8);


			d1|=((m_blitter_data[0x8]&0x200)>>4);
			d2|=((m_blitter_data[0x8]&0x800)>>6);

			hflag=(m_blitter_data[0x9]&0x2)?1:0;
			dflag=(m_blitter_data[0x8]&0x2000)?1:0;
			index=d1|(d2<<6)|(hflag<<12)|(dflag<<13);


			float scale_y=get_scale(index);


			if(scale_x==0 || scale_y==0) return;


			float scale_x_step=100.f/scale_x;
			float scale_y_step=100.f/scale_y;



			int x,y;
			float idx_x,idx_y;

			int vpage=LAYER_FG;
			if(m_blitter_data[0x7]&0x10)
			{
				vpage=LAYER_BG;
/*
                printf("bg -> %d %d   %d %d  %d %d @ %x\n",dst_x0,dst_y0, dst_x1,dst_y1, dst_x1-dst_x0, dst_y1-dst_y0,space.device().safe_pc());

                for(int i=0;i<16;++i)
                {
                    printf("%x = %.4x\n",i,m_blitter_data[i]);
                }

                printf("\n");
*/
			}

			bool endx=false;
			bool endy=false;

			if(m_blitter_data[0x7]&0x0c)
			{
				//???
			}

			for( x=dst_x0, idx_x=0 ; !endx;x+=x_dst_step, idx_x+=scale_x_step )
			{
				endy=false;
				for( y=dst_y0, idx_y=0 ; !endy;y+=y_dst_step, idx_y+=scale_y_step)
				{
						endx=(x==dst_x1);
						endy=(y==dst_y1);


					int xx=src_x0+x_src_step*idx_x;
					int yy=src_y0+y_src_step*idx_y;

					int address=page+yy*512+xx;

					int pix = rom[address&(0x1000000-1)];

					int screen_x=x;
					int screen_y=y;


					if(page>=0x400000)
					{
						//hack for clear
						if(screen_x >0 && screen_y >0 && screen_x < width && screen_y <height)
						{
					//      m_tmp_bitmap[vpage]->pix16(screen_y , screen_x ) =0;
						}
					}
					else
					{
						screen_y&=0xff;

						if(pix && screen_x >0 && screen_y >0 && screen_x < width && screen_y <height)
						{
							m_tmp_bitmap[vpage]->pix16(screen_y , screen_x ) = pix;
						}
					}
				}
			}
		}
	}
}

void wheelfir_state::video_start()
{
	m_tmp_bitmap[0] = auto_bitmap_ind16_alloc(machine(), 512, 512);
	m_tmp_bitmap[1] = auto_bitmap_ind16_alloc(machine(), 512, 512);
}

UINT32 wheelfir_state::screen_update_wheelfir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for(int y=0;y<NUM_SCANLINES;++y)
	{
		UINT16 *source = &m_tmp_bitmap[LAYER_BG]->pix16(( (m_scanlines[y].y)&511));
		UINT16 *dest = &bitmap.pix16(y);

		for (int x=0;x<336;x++)
		{
			dest[x] = source[ (x+(m_scanlines[y].x)) &511];

		}
	}

	copybitmap_trans(bitmap, *m_tmp_bitmap[LAYER_FG], 0, 0, 0, 0, cliprect, 0);

/*
    {
        m_tmp_bitmap[LAYER_BG]->fill(0, screen.visible_area());

    }
*/

	return 0;
}

void wheelfir_state::screen_eof_wheelfir(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_tmp_bitmap[LAYER_FG]->fill(0, screen.visible_area());
	}
}


WRITE16_MEMBER(wheelfir_state::pal_reset_pos_w)
{
	m_palpos = 0;
}

WRITE16_MEMBER(wheelfir_state::pal_data_w)
{
	int color=m_palpos/3;
	m_palette_ptr[m_palpos] = data & 0xff;
	++m_palpos;

	m_palpos %=NUM_COLORS*3;

	{
		int r = m_palette_ptr[color*3];
		int g = m_palette_ptr[color*3+1];
		int b = m_palette_ptr[color*3+2];
		m_palette->set_pen_color(color, rgb_t(r,g,b));
	}

}

WRITE16_MEMBER(wheelfir_state::wheelfir_7c0000_w)
{
	/* seems to be scanline width/2 (used for scanline int timing ? or real width of scanline ?) */
}

WRITE16_MEMBER(wheelfir_state::wheelfir_snd_w)
{
	COMBINE_DATA(&m_soundlatch);
	m_subcpu->set_input_line(1, HOLD_LINE); /* guess, tested also with periodic interrupts and latch clear*/
	machine().scheduler().synchronize();
}

READ16_MEMBER(wheelfir_state::wheelfir_snd_r)
{
	return m_soundlatch;
}

WRITE16_MEMBER(wheelfir_state::coin_cnt_w)
{
	/* bits 0/1 coin counters */
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);
}


static ADDRESS_MAP_START( wheelfir_main, AS_PROGRAM, 16, wheelfir_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x700000, 0x70001f) AM_WRITE(wheelfir_blit_w)
	AM_RANGE(0x720000, 0x720001) AM_WRITE(pal_reset_pos_w)
	AM_RANGE(0x720002, 0x720003) AM_WRITE(pal_data_w)
	AM_RANGE(0x720004, 0x720005) AM_WRITENOP // always ffff?
	AM_RANGE(0x740000, 0x740001) AM_WRITE(wheelfir_snd_w)
	AM_RANGE(0x780000, 0x78000f) AM_READNOP /* net comms ? */
	AM_RANGE(0x760000, 0x760001) AM_WRITE(coin_cnt_w)
	AM_RANGE(0x7a0000, 0x7a0001) AM_WRITE(wheelfir_scanline_cnt_w)
	AM_RANGE(0x7c0000, 0x7c0001) AM_READWRITE(wheelfir_status_r, wheelfir_7c0000_w)
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ_PORT("P1")
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")

	ADDRESS_MAP_END


/* sub is sound cpu? the program roms contain lots of samples */
static ADDRESS_MAP_START( wheelfir_sub, AS_PROGRAM, 16, wheelfir_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM

	AM_RANGE(0x780000, 0x780001) AM_READ(wheelfir_snd_r)

	AM_RANGE(0x700000, 0x700001) AM_DEVWRITE8("dac1", dac_device, write_unsigned8, 0xff00) //guess for now
	AM_RANGE(0x740000, 0x740001) AM_DEVWRITE8("dac2", dac_device, write_unsigned8, 0xff00)
ADDRESS_MAP_END


static INPUT_PORTS_START( wheelfir )
	PORT_START("P1")    /* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x1000, 0x1000, "Test / Game?"  )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED ) /* net comm flag ? */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(wheelfir_state::scanline_timer_callback)
{
	machine().scheduler().synchronize();
	m_current_scanline=param;

	if(m_current_scanline<NUM_SCANLINES)
	{
		//visible scanline

		m_toggle_bit = 0x0000;

		--m_scanline_cnt;

		if(m_current_scanline>0)
		{
			//copy scanline offset
			m_scanlines[m_current_scanline].x=(m_scanlines[m_current_scanline-1].x);
			m_scanlines[m_current_scanline].y=(m_scanlines[m_current_scanline-1].y+1);
			m_scanlines[m_current_scanline].unkbits=m_scanlines[m_current_scanline-1].unkbits;
		}

		if(m_scanline_cnt==0) //<=0 ?
		{
			m_maincpu->set_input_line(5, HOLD_LINE); // raster IRQ, changes scroll values for road
		}

	}
	else
	{
		if(m_current_scanline==NUM_SCANLINES) /* vblank */
		{
			m_toggle_bit = 0x8000;
			m_maincpu->set_input_line(3, HOLD_LINE);
		}
	}
}


void wheelfir_state::machine_reset()
{
}

void wheelfir_state::machine_start()
{
	m_zoom_table = auto_alloc_array(machine(), INT32, ZOOM_TABLE_SIZE);
	m_blitter_data = auto_alloc_array(machine(), UINT16, 16);

	m_scanlines = reinterpret_cast<scroll_info*>(auto_alloc_array(machine(), UINT8, sizeof(scroll_info)*(NUM_SCANLINES+NUM_VBLANK_LINES)));
	m_palette_ptr = auto_alloc_array(machine(), UINT8, NUM_COLORS*3);


	for(int i=0;i<(ZOOM_TABLE_SIZE);++i)
	{
		m_zoom_table[i]=-1;
	}

	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	for(int j=0;j<400;++j)
	{
		int i=j<<3;
		int d1=ROM[0x200+i]&0x1f;
		int d0=(ROM[0x200+i]>>8)&0x1f;

		d0|=(ROM[0x200+1+i]&1)?0x20:0;
		d1|=(ROM[0x200+1+i]&4)?0x20:0;

		int hflag=(ROM[0x200+2+i]&0x100)?1:0;
		int dflag=(ROM[0x200+1+i]&0x10)?1:0;

		int index=d0|(d1<<6)|(hflag<<12)|(dflag<<13);
		m_zoom_table[index]=j;
	}
}


static MACHINE_CONFIG_START( wheelfir, wheelfir_state )

	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)
	MCFG_CPU_PROGRAM_MAP(wheelfir_main)

	MCFG_CPU_ADD("subcpu", M68000, 32000000/2)
	MCFG_CPU_PROGRAM_MAP(wheelfir_sub)
	//MCFG_CPU_PERIODIC_INT_DRIVER(wheelfir_state, irq1_line_hold, 256*60)

	MCFG_QUANTUM_TIME(attotime::from_hz(12000))


	MCFG_TIMER_DRIVER_ADD_SCANLINE("scan_timer", wheelfir_state, scanline_timer_callback, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(336, NUM_SCANLINES+NUM_VBLANK_LINES)
	MCFG_SCREEN_VISIBLE_AREA(0,335, 0, NUM_SCANLINES-1)
	MCFG_SCREEN_UPDATE_DRIVER(wheelfir_state, screen_update_wheelfir)
	MCFG_SCREEN_VBLANK_DRIVER(wheelfir_state, screen_eof_wheelfir)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", NUM_COLORS)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")



	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac1")
	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


ROM_START( wheelfir )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "tch1.u19", 0x00001, 0x80000, CRC(33bbbc67) SHA1(c2ecc0ab522ee442076ea7b9536aee6e1fad0540) )
	ROM_LOAD16_BYTE( "tch2.u21", 0x00000, 0x80000, CRC(ed6b9e8a) SHA1(214c5aaf55963a219db33dd5d530492e09ad5e07) )

	ROM_REGION( 0x100000, "subcpu", 0 ) /* 68000 Code + sound samples */
	ROM_LOAD16_BYTE( "tch3.u83",  0x00001, 0x80000, CRC(43c014a6) SHA1(6c01a08dda204f36e8768795dd5d405576a49140) )
	ROM_LOAD16_BYTE( "tch11.u65", 0x00000, 0x80000, CRC(fc894b2e) SHA1(ebe6d1adf889731fb6f53b4ce5f09c60e2aefb97) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // 512x512 gfx pages
	ROM_LOAD( "tch4.u52", 0x000000, 0x80000, CRC(fe4bc2c7) SHA1(33a2ef79cb13f9e7e7d513915c6e13c4e7fe0188) )
	ROM_LOAD( "tch5.u53", 0x080000, 0x80000, CRC(a38b9ca5) SHA1(083c9f700b9df1039fb553e918e205c6d32057ad) )
	ROM_LOAD( "tch6.u54", 0x100000, 0x80000, CRC(2733ae6b) SHA1(ebd91e123b670159f79be19a552d1ae0c8a0faff) )
	ROM_LOAD( "tch7.u55", 0x180000, 0x80000, CRC(6d98f27f) SHA1(d39f7f184abce645b9165b64e89e3b5354187eea) )
	ROM_LOAD( "tch8.u56", 0x200000, 0x80000, CRC(22b661fe) SHA1(b6edf8e1e8b479ee8813502157615f54627dc7c1) )
	ROM_LOAD( "tch9.u57", 0x280000, 0x80000, CRC(83c66de3) SHA1(50deaf3338d590340b928f891548c47ba8f3ca38) )
	ROM_LOAD( "tch10.u58",0x300000, 0x80000, CRC(2036ed80) SHA1(910381e2ccdbc2d06f873021d8af02795d22f595) )
	ROM_LOAD( "tch12.u59",0x380000, 0x80000, CRC(cce2e675) SHA1(f3d8916077b2e057169d0f254005cd959789a3b3) )
ROM_END

DRIVER_INIT_MEMBER(wheelfir_state,wheelfir)
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	RAM[0xdd3da/2] = 0x4e71; //hack
}

GAME( 199?, wheelfir,    0, wheelfir,    wheelfir, wheelfir_state,    wheelfir, ROT0,  "TCH", "Wheels & Fire", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
