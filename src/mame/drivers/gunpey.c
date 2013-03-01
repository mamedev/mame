/********************************************************************************************

	Gunpey (c) 2000 Banpresto

ASM code study:
0x84718 main code
0x8472c call 0x81f34 reading of dip-switches
	0x5e62:
0x84731 call 0x81f5c move dip-switches settings to work RAM
0x84736 call 0x80ae4 writes to i/os 0x7fc0-0x7fff:
	0x7fc0: 0x00
	0x7fc1: 0x00
	0x7fc2: 0x40
	0x7fc3: 0x03
	0x7fc4: 0x72
	0x7fc5: 0x72
	0x7fc6: 0x90
	0x7fc7: 0x01
	0x7fc8: 0x55 irq mask?
	(note: skips irq ack)
	0x7fca: 0x00
	0x7fcb: 0x00
	0x7fcc: 0x00
	0x7fcd: 0x00
	0x7fce: 0x00
	0x7fcf: 0x07
	0x7fd0: 0x00
	0x7fd1: 0x00
	0x7fd2: 0x00
	0x7fd3: 0x8c
	0x7fd4-0x7fde: 0x00
	(skips 0x7fdf)
	0x7fe0: 0x40
	0x7fe1: 0x02
	0x7fe2: 0x00
	0x7fe3: 0x00
	0x7fe4: 0x60
	0x7fe5: 0x07
	0x7fe6: 0x88
	0x7fe7: 0x07
	0x7fe8: 0x9f
	0x7fe9: 0x08
	0x7fea: 0x77
	0x7feb: 0x08
	(here the code writes 16-bit words, read from 0x500e ...)
	0x7fec: 0x00
	0x7fed: 0x84
	(read from 0x5012)
	0x7fee: 0x00
	0x7fef: 0x88
	(returns to byte filling)
	0x7ff0: 0x84
	0x7ff1: 0x00
	0x7ff2: 0x00
	0x7ff3: 0x01
	0x7ff4: 0x00
	0x7ff5: 0x00
	0x7ff6: 0xc7
	0x7ff7: 0x56
	0x7ff8: 0x3f
	0x7ff9: 0x48
	0x7ffa: 0x9a
	0x7ffb: 0x22
	0x7ffc: 0x0e
	0x7ffd: 0x43
	0x7ffe: 0xf0
	0x7fff: 0x15
	Then, it fills the following work RAMs (with 0 except where noted):
	0x5e36, 0x5e38, 0x5e44, 0x5e46 (4), 0x5e48, 0x5e4a, 0x5e40, 0x5e42, 0x5b68 (0x9c8), 0x5b6a (0x9c8), 0x59c4 (1)
0x8473b: call 0x81415
	reads [0x500e] -> [0xf0bc]
	reads [0x5010] -> [0xf0be]
	(does it twice, shrug -.-")
	reads 0xf0bc / 0xf0be
	AW = 0xb3c9, CW = 0x10
	0x81447: call 0xb3aa0
		moves RAM from 0xb3c90-0xb3c9f to RAM 0x400-0x40f
	writes 0x800 to [0xf0b8] writes 0 to [0xf0ba]
	loops while writing to [0xf0b8] [0xf0ba] for 0x200 times, filling a table at [0x800][0x27f0] with a 1
	writes 0x800 to [0xf0b8] writes 0 to [0xf0ba] again
	reads the [0xf0b8][0xf0ba]
	0x81484: call 0xb3aa0
		moves RAM from 0xb3ca0-0xb3caf to RAM 0x800-0x80f
	writes 0x2800 to [0xf0b8] writes 0 to [0xf0ba]
	loops while writing to [0xf0b8] [0xf0ba] for 0x200 times, filling a table at [0x2800][0x47f0] with a 1
	0x81484: call 0xb3aa0
		moves RAM from 0xb3ca0-0xb3caf to RAM 0x2800-0x280f
0x84740: call 0x821cd
	fills 0x5c88-0x5c9a with this pattern (byte writes):
	[+0x00] 0x00
	[+0x02] 0x00
	[+0x08] 0x00
	[+0x04] 0x00
	[+0x0c] 0x00
	[+0x0e] 0x0c
	[+0x06] 0x00
	[+0x10] 0x00
	[+0x12] 0x0c
	does the same with 0x5c89-0x5c9b
0x84745: call 0x82026
	checks if 0x5e62 is 0 (bit 6 of 0x5c80, i/o 0x7f41)
	...
0x8474a: call 0xa7f53 sound init
...



=============================================================================================

Gunpey
Banpresto, 2000

The hardware looks very Raizing/8ing -ish, especially the ROM stickers and PCB silk-screening
which are identical to those on Brave Blade and Battle Bakraid ;-)

PCB Layout
----------

VG-10
|-------------------------------------|
|        M6295  ROM5                  |
|        YMZ280B-F      ROM4   ROM3   |
|  YAC516      16.93MHz               |
|                       61256  ROM1   |
|                       61256  ROM2   |
|J                                    |
|A             PAL       XILINX       |
|M                       XC95108      |
|M             57.2424MHz             |
|A                            V30     |
|                                     |
|              |-------|              |
|              |AXELL  |              |
|       DSW1   |AG-1   |              |
|       DSW2   |AX51101|  T2316162    |
|              |-------|  T2316162    |
|-------------------------------------|

Notes:
      V30 clock: 14.3106MHz (= 57.2424 / 4)
      YMZ280B clock: 16.93MHz
      OKI M6295 clock: 2.11625MHz (= 16.93 / 8), sample rate = clock / 165
      VSync: 60Hz
      HSync: 15.79kHz

      ROMs:
           GP_ROM1.021 \
           GP_ROM2.022 / 27C040, Main program

           GP_ROM3.025 \
           GP_ROM4.525 / SOP44 32M MASK, Graphics

           GP_ROM5.622   SOP44 32M MASK, OKI samples


AX51101 gfx chip:

Axell Corporation is a Japanese company that specialises in Sound and Graphics (Amusement Graphics) LSI.
The AG-1 is a sprite system controller meant for the amusement industry, such as pachi-slot machines,
it has reached end-of-life in about 2005.
These are the specifications of the Axell AG-1 AX51102A, it should be very similar to the AX51101.
(excuse the strange grammar, it is a JP->EN translation)

Drawing technique:               Sprite system
Buffer drawing method:           Double frame buffer
Configuration of the character:  Configured from 2 or more cells, a cell can be set in units of each dot a horizontal, vertical from 1 to 256 dots
Maximum character size:          4096 x 4096 dot
Display the number of sprite:    Up to 127 sheets (register 2KB)
Maximum drawing speed:           Dot sec / 2500-35000000 highest
Color depth:                     32,768 colors (5 bits for each RGB)
Color scheme:                    Cell character unit can be specified in 256 colors of palettes, and 16 colors
Scaling:                         256 times the resolution of 1/64 to 4 cell character unit
Semi-transparent processing:     Gradation in the unit cell or 32 character
Intensity modulation:            Gradation in the unit cell or 32 character
Other Features:                  Rotation, DMA, BitBLT, Built-in flip
Display resolution:              100 to 600 dots horizontal, 120 to 800 dots vertical
Virtual screen size:             Up to 4096 x 4096 dot
CGRAM space:                     4M-bit minimum, 32M-bit maximum
Operating frequency:             Up to 76MHz
Release:                         November 1999

********************************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"


class gunpey_state : public driver_device
{
public:
	gunpey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_wram(*this, "wram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_shared_ptr<UINT16> m_wram;

	UINT16 *m_blit_buffer;
	UINT16 m_blit_ram[0x10];
	UINT8 m_irq_cause, m_irq_mask;
	DECLARE_WRITE8_MEMBER(gunpey_status_w);
	DECLARE_READ8_MEMBER(gunpey_status_r);
	DECLARE_READ8_MEMBER(gunpey_inputs_r);
	DECLARE_WRITE8_MEMBER(gunpey_blitter_w);
	DECLARE_WRITE8_MEMBER(gunpey_output_w);
	DECLARE_WRITE16_MEMBER(gunpey_vram_bank_w);
	DECLARE_DRIVER_INIT(gunpey);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_gunpey(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(gunpey_scanline);
	TIMER_CALLBACK_MEMBER(blitter_end);
	void gunpey_irq_check(UINT8 irq_type);
	UINT8 draw_gfx(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int count);
	UINT16 m_vram_bank;


	//UINT16 main_vram[0x800][0x800];
};


void gunpey_state::video_start()
{
	m_blit_buffer = auto_alloc_array(machine(), UINT16, 512*512);
}

UINT8 gunpey_state::draw_gfx(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int count)
{
	int x,y;
	int bpp_sel;
	int color;
	UINT8 *vram = memregion("vram")->base();

	// there doesn't seem to be a specific bit to mark compressed sprites (we currently have a hack to look at the first byte of the data)
	// do they get decompressed at blit time instead? of are there other registers we need to look at

	// +0                    +1                    +2                    +3                    +4                    +5                    +6                    +7
	// cccc cccc e--b b--- | xxxx x--- u--t tttt | yyyy yy-- --XX XXXX | nnnn nnnn ---Y YYYY | mmmm mmmm -MMM -NNN | hhhh hhhh wwww wwww | ---- ---- oooo oooo | pppp pppp ---- ---- | 

	// c = color palette
	// e = END marker
	// b = bpp select
	// x = LSB x source
	// X = MSB x source
	// y = LSB y source
	// Y = MSB y source
	// n = LSB X DRAW position
	// N = MSB X DRAW position
	// m = LSB Y DRAW position
	// M = MSB Y DRAW position
	// h = height
	// w = width
	// u = unknown, set on text, maybe 'solid' ?
	// o = zoom width
	// p = zoom height
	// t = transparency / alpha related? (0x10 on player cursor, 0xf when swapping, other values at other times..)
	const int debug = 0;

	if(!(m_wram[count+0] & 1))
	{
		x = (m_wram[count+3] >> 8) | ((m_wram[count+4] & 0x03) << 8);
		y = (m_wram[count+4] >> 8) | ((m_wram[count+4] & 0x30) << 4);
		UINT8 zoomheight = (m_wram[count+5] >> 8);
		UINT8 zoomwidth = (m_wram[count+5] & 0xff);
		bpp_sel = (m_wram[count+0] & 0x18);
		color = (m_wram[count+0] >> 8);

		x-=0x160;
		y-=0x188;

		UINT8 sourcewidth  = (m_wram[count+6] & 0xff);
		UINT8 sourceheight = (m_wram[count+7] >> 8);

		//UINT32 col = 0xffffff;
	//		UINT32 val = (m_wram[count+1] << 16) | ((m_wram[count+2]));
		int xsource = ((m_wram[count+2] & 0x003f) << 5) | ((m_wram[count+1] & 0xf800) >> 11);
		int ysource = ((m_wram[count+3] & 0x001f) << 6) | ((m_wram[count+2] & 0xfc00) >> 10);
		//printf("%08x  %04x %04x\n", val, m_wram[count+1],m_wram[count+2] );
	//	UINT16 xsource = (m_wram[count+2] & 0x00ff << 4) | (m_wram[count+1] & 0xf000 >> 12);
		//xsource<<=1;

	//	xsource<<=1;
	//	ysource <<=2;

//		UINT8 testhack = vram[((((ysource+0)&0x7ff)*0x800) + ((xsource+0)&0x7ff))];
//		UINT8 testhack2 = vram[((((ysource+0)&0x7ff)*0x800) + ((xsource+1)&0x7ff))];

		//if (m_wram[count+1] & 0x0010) 
		//	color =  machine.rand()&0xf;



		
		UINT16 unused;
		if (debug) printf("sprite %04x %04x %04x %04x %04x %04x %04x %04x\n", m_wram[count+0], m_wram[count+1], m_wram[count+2], m_wram[count+3], m_wram[count+4], m_wram[count+5], m_wram[count+6], m_wram[count+7]);
		
		unused = m_wram[count+0]&~0xff98; if (unused) printf("unused bits set in word 0 - %04x\n", unused);
		unused = m_wram[count+1]&~0xf89f; if (unused) printf("unused bits set in word 1 - %04x\n", unused);
		unused = m_wram[count+2]&~0xfc3f; if (unused) printf("unused bits set in word 2 - %04x\n", unused);
		unused = m_wram[count+3]&~0xff1f; if (unused) printf("unused bits set in word 3 - %04x\n", unused);
		unused = m_wram[count+4]&~0xff77; if (unused) printf("unused bits set in word 4 - %04x\n", unused);
		unused = m_wram[count+5]&~0xffff; if (unused) printf("unused bits set in word 5 - %04x\n", unused);
		unused = m_wram[count+6]&~0x00ff; if (unused) printf("unused bits set in word 6 - %04x\n", unused);
		unused = m_wram[count+7]&~0xff00; if (unused) printf("unused bits set in word 7 - %04x\n", unused);
	
		if ((zoomwidth != sourcewidth) || (zoomheight!= zoomheight))
		{
			//printf("zoomed widths %02x %02x heights %02x %02x\n", sourcewidth, zoomwidth, sourceheight, zoomheight);
		}



//		if ((testhack2 & 0x0f) == 0x08)
//			return m_wram[count+0] & 0x80;

		//if (debug) printf("testhack1 %02x %02x\n", testhack, testhack2);

		if(bpp_sel == 0x00)  // 4bpp
		{
			for(int yi=0;yi<sourceheight;yi++)
			{
				for(int xi=0;xi<sourcewidth/2;xi++)
				{
					UINT8 data = vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
					UINT8 pix;
					UINT32 col_offs;
					UINT16 color_data;

					pix = (data & 0x0f);
					col_offs = ((pix + color*0x10) & 0xff) << 1;
					col_offs+= ((pix + color*0x10) >> 8)*0x800;
					color_data = (vram[col_offs])|(vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
						if(cliprect.contains(x+(xi*2), y+yi))
							bitmap.pix16(y+yi, x+(xi*2)) = color_data & 0x7fff;

					pix = (data & 0xf0)>>4;
					col_offs = ((pix + color*0x10) & 0xff) << 1;
					col_offs+= ((pix + color*0x10) >> 8)*0x800;
					color_data = (vram[col_offs])|(vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
						if(cliprect.contains(x+1+(xi*2), y+yi))
							bitmap.pix16(y+yi, x+1+(xi*2)) = color_data & 0x7fff;
				}
			}
		}
		else if(bpp_sel == 0x08) // 6bpp
		{
			printf("6bpp\n");
			#if 0
			for(int yi=0;yi<sourceheight;yi++)
			{
				for(int xi=0;xi<sourcewidth;xi++)
				{
					UINT8 data = vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
					UINT8 pix;
					UINT32 col_offs;
					UINT16 color_data;

					pix = (data & 0x3f);
					if(cliprect.contains(x+xi, y+yi))
						bitmap.pix16(y+yi, x+xi) = pix + color*64;
				}
			}
			#endif
		}
		else if(bpp_sel == 0x10) // 8bpp
		{
			for(int yi=0;yi<sourceheight;yi++)
			{
				for(int xi=0;xi<sourcewidth;xi++)
				{
					UINT8 data = vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
					UINT8 pix;
					UINT32 col_offs;
					UINT16 color_data;

					pix = (data & 0xff);
					col_offs = ((pix + color*0x100) & 0xff) << 1;
					col_offs+= ((pix + color*0x100) >> 8)*0x800;
					color_data = (vram[col_offs])|(vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
					if(cliprect.contains(x+xi, y+yi))
						bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
				}
			}
		}
		else if(bpp_sel == 0x18) // RGB32k
		{
			printf("32k\n");
			// ...
		}
	}

	return m_wram[count+0] & 0x80;
}

UINT32 gunpey_state::screen_update_gunpey(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//UINT16 *blit_buffer = m_blit_buffer;
	int count;
	UINT16 vram_bank = m_vram_bank & 0x7fff;
	UINT8 end_mark;

	bitmap.fill(machine().pens[0], cliprect); //black pen

	/* last 4 entries are special, skip them for now. */
	for(count = vram_bank/2;count<(vram_bank+0x1000)/2;count+=0x10/2)
	{
		end_mark = draw_gfx(screen.machine(), bitmap,cliprect,count);

		if(end_mark == 0x80)
			break;
	}

	for(count = (vram_bank+0x1000)/2;count<(vram_bank+0x2000)/2;count+=0x10/2)
	{
		end_mark = draw_gfx(screen.machine(), bitmap,cliprect,count);

		if(end_mark == 0x80)
			break;
	}

	return 0;
}

void gunpey_state::gunpey_irq_check(UINT8 irq_type)
{
	m_irq_cause |= irq_type;

	if(m_irq_cause & m_irq_mask)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x200/4);
	else
		m_maincpu->set_input_line_and_vector(0, CLEAR_LINE, 0x200/4);
}

WRITE8_MEMBER(gunpey_state::gunpey_status_w)
{
	if(offset == 1)
	{
		m_irq_cause &= ~data;
		gunpey_irq_check(0);
	}

	if(offset == 0)
	{
		m_irq_mask = data;
		gunpey_irq_check(0);
	}
}

READ8_MEMBER(gunpey_state::gunpey_status_r)
{
	if(offset == 1)
		return m_irq_cause;

	return m_irq_mask;
}

READ8_MEMBER(gunpey_state::gunpey_inputs_r)
{
	switch(offset+0x7f40)
	{
		case 0x7f40: return ioport("DSW1")->read();
		case 0x7f41: return ioport("DSW2")->read();
		case 0x7f42: return ioport("P1")->read();
		case 0x7f43: return ioport("P2")->read();
		case 0x7f44: return ioport("SYSTEM")->read();
	}

	return 0xff;
}

TIMER_CALLBACK_MEMBER(gunpey_state::blitter_end)
{
	gunpey_irq_check(4);
}



WRITE8_MEMBER(gunpey_state::gunpey_blitter_w)
{
//	UINT16 *blit_buffer = m_blit_buffer;
	UINT16 *blit_ram = m_blit_ram;
	UINT8 *blit_rom = memregion("blit_data")->base();
	UINT8 *vram = memregion("vram")->base();

	//	int x,y;

	//printf("gunpey_blitter_w offset %01x data %02x\n", offset,data);

	blit_ram[offset] = data;

	if(offset == 0 && data == 2) // blitter trigger, 0->1 transition
	{
		int srcx = blit_ram[0x04]+(blit_ram[0x05]<<8);
		int srcy = blit_ram[0x06]+(blit_ram[0x07]<<8);
		int dstx = blit_ram[0x08]+(blit_ram[0x09]<<8);
		int dsty = blit_ram[0x0a]+(blit_ram[0x0b]<<8);
		int xsize = blit_ram[0x0c]+1;
		int ysize = blit_ram[0x0e]+1;
//		int color,color_offs;

//		printf("%04x %04x %04x %04x\n",srcx,srcy,dstx,dsty);

/*
	  printf("%02x %02x %02x %02x| (X SRC 4: %02x 5: %02x (val %04x))  (Y SRC 6: %02x 7: %02x (val %04x))  | (X DEST 8: %02x 9: %02x (val %04x))  (Y DEST a: %02x b: %02x (val %04x)) |  %02x %02x %02x %02x\n"
	   ,blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3]


	   ,blit_ram[4],blit_ram[5], srcx
	   ,blit_ram[6],blit_ram[7], srcy

	   ,blit_ram[8],blit_ram[9], dstx
	   ,blit_ram[0xa],blit_ram[0xb], dsty
       ,blit_ram[0xc],

		   blit_ram[0xd],blit_ram[0xe],blit_ram[0xf]);
*/
		//if (srcx & 0xf800) printf("(error srcx &0xf800)");
		//if (srcy & 0xf800) printf("(error srcy &0xf800)");

		// these are definitely needed for 4bpp..
		dstx<<=1;
		xsize<<=1;

		for (int y=0;y<ysize;y++)
		{
			for (int x=0;x<xsize;x++)
			{
				vram[(((dsty+y)&0x7ff)*0x800)+((dstx+x)&0x7ff)] = blit_rom[(((srcy+y)&0x7ff)*0x800)+((srcx+x)&0x7ff)];
			}
		}

		machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(xsize*ysize), timer_expired_delegate(FUNC(gunpey_state::blitter_end),this));


/*
      printf("%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x\n"
      ,blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3]
      ,blit_ram[4],blit_ram[5],blit_ram[6],blit_ram[7]
      ,blit_ram[8],blit_ram[9],blit_ram[0xa],blit_ram[0xb]
      ,blit_ram[0xc],blit_ram[0xd],blit_ram[0xe],blit_ram[0xf]);
*/
	}
}

WRITE8_MEMBER(gunpey_state::gunpey_output_w)
{
	//bit 0 is coin counter
//	popmessage("%02x",data);

	downcast<okim6295_device *>(m_oki)->set_bank_base(((data & 0x70)>>4) * 0x40000);
}

WRITE16_MEMBER(gunpey_state::gunpey_vram_bank_w)
{
	COMBINE_DATA(&m_vram_bank);
}

/***************************************************************************************/

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 16, gunpey_state )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM AM_SHARE("wram")
//  AM_RANGE(0x50000, 0x500ff) AM_RAM
//  AM_RANGE(0x50100, 0x502ff) AM_NOP
	AM_RANGE(0x80000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 16, gunpey_state )
	AM_RANGE(0x7f40, 0x7f45) AM_READ8(gunpey_inputs_r,0xffff)

	AM_RANGE(0x7f48, 0x7f49) AM_WRITE8(gunpey_output_w,0x00ff)
	AM_RANGE(0x7f80, 0x7f81) AM_DEVREADWRITE8_LEGACY("ymz", ymz280b_r, ymz280b_w, 0xffff)

	AM_RANGE(0x7f88, 0x7f89) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)

	AM_RANGE(0x7fc8, 0x7fc9) AM_READWRITE8(gunpey_status_r,  gunpey_status_w, 0xffff )
	AM_RANGE(0x7fd0, 0x7fdf) AM_WRITE8(gunpey_blitter_w, 0xffff )
	//AM_RANGE(0x7FF0, 0x7FF1) AM_RAM
	AM_RANGE(0x7fee, 0x7fef) AM_WRITE(gunpey_vram_bank_w)

ADDRESS_MAP_END


/***************************************************************************************/


static void sound_irq_gen(device_t *device, int state)
{
	logerror("sound irq\n");
}

static const ymz280b_interface ymz280b_intf =
{
	sound_irq_gen
};


/***************************************************************************************/

static INPUT_PORTS_START( gunpey )
	PORT_START("DSW1")  // IN0 - 7f40
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty (vs. mode)" ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x00, "Matches (vs. mode)?" )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")  // IN1 - 7f41
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")    // IN2 - 7f42
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")    // IN3 - 7f43
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")    // IN4 - 7f44
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )   // TEST!!
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************************/

void gunpey_state::palette_init()
{
	int i;

	for (i = 0; i < 0x8000; i++)
		palette_set_color(machine(), i, MAKE_RGB( pal5bit((i >> 10)&0x1f), pal5bit(((i >> 5))&0x1f), pal5bit((i >> 0)&0x1f)));
}


/*:
0x01
0x04 blitter ready
0x10 vblank too? (otherwise you'll get various hangs/inputs stop to work)
0x40 almost certainly vblank (reads inputs)
0x80
*/
TIMER_DEVICE_CALLBACK_MEMBER(gunpey_state::gunpey_scanline)
{
	int scanline = param;

	if(scanline == 224)
	{
		//printf("frame\n");
		gunpey_irq_check(0x50);
	}
}





// this isn't a real decode as such, but the graphic data is all stored in pages 2048 bytes wide at varying BPP levelsl, some (BG data) compressed with what is likely a lossy scheme
// palette data is in here too, the blocks at the bottom right of all this?
static GFXLAYOUT_RAW( gunpey, 2048, 1, 2048*8, 2048*8 )

static GFXDECODE_START( gunpey )
	GFXDECODE_ENTRY( "blit_data", 0, gunpey,     0x0000, 0x1 )
	GFXDECODE_ENTRY( "vram", 0, gunpey,     0x0000, 0x1 )
GFXDECODE_END



/***************************************************************************************/
static MACHINE_CONFIG_START( gunpey, gunpey_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 57242400 / 4)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", gunpey_state, gunpey_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(57242400/8, 442, 0, 320, 264, 0, 240) /* just to get ~60 Hz */
	MCFG_SCREEN_UPDATE_DRIVER(gunpey_state, screen_update_gunpey)

	MCFG_PALETTE_LENGTH(0x10000)
	MCFG_GFXDECODE(gunpey)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")

	MCFG_OKIM6295_ADD("oki", XTAL_16_9344MHz / 8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.25)

	MCFG_SOUND_ADD("ymz", YMZ280B, XTAL_16_9344MHz)
	MCFG_SOUND_CONFIG(ymz280b_intf)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.25)
MACHINE_CONFIG_END


/***************************************************************************************/

ROM_START( gunpey )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "gp_rom1.021",  0x00000, 0x80000, CRC(07a589a7) SHA1(06c4140ffd5f74b3d3ddfc424f43fcd08d903490) )
	ROM_LOAD16_BYTE( "gp_rom2.022",  0x00001, 0x80000, CRC(f66bc4cf) SHA1(54931d878d228c535b9e2bf22a0a3e41756f0fe5) )

	ROM_REGION( 0x400000, "blit_data", 0 )
	ROM_LOAD( "gp_rom3.025",  0x00000, 0x400000,  CRC(f2d1f9f0) SHA1(0d20301fd33892074508b9d127456eae80cc3a1c) )
	ROM_REGION( 0x400000, "vram", ROMREGION_ERASEFF )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "gp_rom4.525",  0x000000, 0x400000, CRC(78dd1521) SHA1(91d2046c60e3db348f29f776def02e3ef889f2c1) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "gp_rom5.622",  0x000000, 0x400000,  CRC(f79903e0) SHA1(4fd50b4138e64a48ec1504eb8cd172a229e0e965)) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END



DRIVER_INIT_MEMBER(gunpey_state,gunpey)
{
//	UINT8 *rom = memregion("maincpu")->base();

	/* patch SLOOOOW cycle checks ... */
//	rom[0x848b5] = 0x7e;
//  rom[0x848b6] = 0x03;
//	rom[0x89657] = 0x75;
//	rom[0x8e628] = 0x75;

}

GAME( 2000, gunpey, 0, gunpey, gunpey, gunpey_state, gunpey,    ROT0, "Banpresto", "Gunpey",GAME_NOT_WORKING)
