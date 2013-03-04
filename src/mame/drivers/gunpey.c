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
	DECLARE_WRITE8_MEMBER(gunpey_blitter_upper_w);
	DECLARE_WRITE8_MEMBER(gunpey_blitter_upper2_w);
	DECLARE_WRITE8_MEMBER(gunpey_output_w);
	DECLARE_WRITE16_MEMBER(gunpey_vram_bank_w);
	DECLARE_WRITE16_MEMBER(gunpey_vregs_addr_w);
	DECLARE_DRIVER_INIT(gunpey);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_gunpey(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(gunpey_scanline);
	TIMER_CALLBACK_MEMBER(blitter_end);
	void gunpey_irq_check(UINT8 irq_type);
	UINT8 draw_gfx(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int count,UINT8 scene_gradient);
	UINT16 m_vram_bank;
	UINT16 m_vreg_addr;

	UINT8* m_blit_rom;
	UINT8* m_vram;

	// work variables for the decompression
	int m_srcx;
	int m_srcxbase;
	int m_scrxcount;
	int m_srcy;
	int m_srcycount;
	UINT8 m_sourcewide;
	int m_ysize;
	int m_xsize;
	int m_dstx;
	int m_dsty;
	int m_dstxbase;
	int m_dstxcount;
	int m_dstycount;

	int m_latched_bits_left;
	UINT8 m_latched_byte;
	int m_zero_bit_count;

	void get_stream_next_byte(void);
	int get_steam_bit(void);
	UINT32 gunpey_state_get_stream_bits(int bits);

	int write_dest_byte(UINT8 usedata);
	//UINT16 main_m_vram[0x800][0x800];
};


void gunpey_state::video_start()
{
	m_blit_buffer = auto_alloc_array(machine(), UINT16, 512*512);
}

UINT8 gunpey_state::draw_gfx(running_machine &machine, bitmap_ind16 &bitmap,const rectangle &cliprect,int count,UINT8 scene_gradient)
{
	int x,y;
	int bpp_sel;
	int color;

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

//		UINT8 testhack = m_vram[((((ysource+0)&0x7ff)*0x800) + ((xsource+0)&0x7ff))];
//		UINT8 testhack2 = m_vram[((((ysource+0)&0x7ff)*0x800) + ((xsource+1)&0x7ff))];

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
					UINT8 data = m_vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
					UINT8 pix;
					UINT32 col_offs;
					UINT16 color_data;

					pix = (data & 0x0f);
					col_offs = ((pix + color*0x10) & 0xff) << 1;
					col_offs+= ((pix + color*0x10) >> 8)*0x800;
					color_data = (m_vram[col_offs])|(m_vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
					{
						if(scene_gradient & 0x40)
						{
							int r,g,b;

							r = (color_data & 0x7c00) >> 10;
							g = (color_data & 0x03e0) >> 5;
							b = (color_data & 0x001f) >> 0;
							r-= (scene_gradient & 0x1f);
							g-= (scene_gradient & 0x1f);
							b-= (scene_gradient & 0x1f);
							if(r < 0) r = 0;
							if(g < 0) g = 0;
							if(b < 0) b = 0;

							color_data = (color_data & 0x8000) | (r << 10) | (g << 5) | (b << 0);
						}

						if(cliprect.contains(x+(xi*2), y+yi))
							bitmap.pix16(y+yi, x+(xi*2)) = color_data & 0x7fff;
					}

					pix = (data & 0xf0)>>4;
					col_offs = ((pix + color*0x10) & 0xff) << 1;
					col_offs+= ((pix + color*0x10) >> 8)*0x800;
					color_data = (m_vram[col_offs])|(m_vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
					{
						if(scene_gradient & 0x40)
						{
							int r,g,b;

							r = (color_data & 0x7c00) >> 10;
							g = (color_data & 0x03e0) >> 5;
							b = (color_data & 0x001f) >> 0;
							r-= (scene_gradient & 0x1f);
							g-= (scene_gradient & 0x1f);
							b-= (scene_gradient & 0x1f);
							if(r < 0) r = 0;
							if(g < 0) g = 0;
							if(b < 0) b = 0;

							color_data = (color_data & 0x8000) | (r << 10) | (g << 5) | (b << 0);
						}

						if(cliprect.contains(x+1+(xi*2), y+yi))
							bitmap.pix16(y+yi, x+1+(xi*2)) = color_data & 0x7fff;
					}
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
					UINT8 data = m_vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
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
					UINT8 data = m_vram[((((ysource+yi)&0x7ff)*0x800) + ((xsource+xi)&0x7ff))];
					UINT8 pix;
					UINT32 col_offs;
					UINT16 color_data;

					pix = (data & 0xff);
					col_offs = ((pix + color*0x100) & 0xff) << 1;
					col_offs+= ((pix + color*0x100) >> 8)*0x800;
					color_data = (m_vram[col_offs])|(m_vram[col_offs+1]<<8);

					if(!(color_data & 0x8000))
					{
						if(scene_gradient & 0x40)
						{
							int r,g,b;

							r = (color_data & 0x7c00) >> 10;
							g = (color_data & 0x03e0) >> 5;
							b = (color_data & 0x001f) >> 0;
							r-= (scene_gradient & 0x1f);
							g-= (scene_gradient & 0x1f);
							b-= (scene_gradient & 0x1f);
							if(r < 0) r = 0;
							if(g < 0) g = 0;
							if(b < 0) b = 0;

							color_data = (color_data & 0x8000) | (r << 10) | (g << 5) | (b << 0);
						}

						if(cliprect.contains(x+xi, y+yi))
							bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
					}
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
	UINT16 vram_bank = m_vram_bank & 0x7fff;
	UINT16 vreg_addr = m_vreg_addr & 0x7fff;
	UINT8 end_mark;
	int count;
	int scene_index;

	bitmap.fill(machine().pens[0], cliprect); //black pen

	if((!(m_vreg_addr & 0x8000)) || (!(m_vram_bank & 0x8000)))
		return 0;

	for(scene_index = vreg_addr/2;scene_index<(vreg_addr+0x400)/2;scene_index+=0x10/2)
	{
		UINT16 start_offs;
		UINT16 end_offs;
		UINT8 scene_end_mark;
		UINT8 scene_enabled;
		UINT8 scene_gradient;

		start_offs = (vram_bank+(m_wram[scene_index+5] << 8))/2;
		end_offs = (vram_bank+(m_wram[scene_index+5] << 8)+0x1000)/2; //safety check
		scene_end_mark = m_wram[scene_index+0] & 0x80;
		scene_enabled = m_wram[scene_index+0] & 0x01;
		scene_gradient = m_wram[scene_index+1] & 0xff;

//		printf("%08x: %08x %08x %08x %08x | %08x %08x %08x %08x\n",scene_index,m_wram[scene_index+0],m_wram[scene_index+1],m_wram[scene_index+2],m_wram[scene_index+3],
//		                                            m_wram[scene_index+4],m_wram[scene_index+5],m_wram[scene_index+6],m_wram[scene_index+7]);

		if(scene_enabled)
		{
			for(count = start_offs;count<end_offs;count+=0x10/2)
			{
				end_mark = draw_gfx(screen.machine(), bitmap,cliprect,count,scene_gradient);

				if(end_mark == 0x80)
					break;
			}
		}

		if(scene_end_mark == 0x80)
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

void gunpey_state::get_stream_next_byte(void)
{
	// check if we need to move on to the next row of the source bitmap
	// to get the data requested
	if (m_scrxcount==m_sourcewide)
	{
		m_scrxcount = 0;
		m_srcx = m_srcxbase;
		m_srcy++; m_srcycount++;
	}

	m_latched_byte = m_blit_rom[(((m_srcy)&0x7ff)*0x800)+((m_srcx)&0x7ff)];
	m_latched_bits_left = 8;

	// increase counters
	m_srcx++; m_scrxcount++;
}

int gunpey_state::get_steam_bit(void)
{
	if (m_latched_bits_left==0)
	{
		get_stream_next_byte();
	}

	m_latched_bits_left--;

	int bit = (m_latched_byte >> (7-m_latched_bits_left))&1;
	
	if (bit==0) m_zero_bit_count++;
	else m_zero_bit_count=0;

	return bit;
}

UINT32 gunpey_state::gunpey_state_get_stream_bits(int bits)
{
	UINT32 output = 0;
	for (int i=0;i<bits;i++)
	{
		output = output<<1;
		output |= get_steam_bit();
	}

	return output;
}

int gunpey_state::write_dest_byte(UINT8 usedata)
{
	// write the byte we and to destination and increase our counters
	m_vram[(((m_dsty)&0x7ff)*0x800)+((m_dstx)&0x7ff)] = usedata;

	// increase destination counter and check if we've filled our destination rectangle
	m_dstx++; m_dstxcount++;
	if (m_dstxcount==m_xsize)
	{
		m_dstxcount = 0;
		m_dstx = m_dstxbase;
		m_dsty++; m_dstycount++;
		if (m_dstycount==m_ysize)
		{
			return -1;
		}
	}

	return 1;
}


//#define SHOW_COMPRESSED_DATA_DEBUG



WRITE8_MEMBER(gunpey_state::gunpey_blitter_w)
{
//	UINT16 *blit_buffer = m_blit_buffer;
	UINT16 *blit_ram = m_blit_ram;


	//	int x,y;

	//printf("gunpey_blitter_w offset %01x data %02x\n", offset,data);

	blit_ram[offset] = data;

	if(offset == 0 && data == 2) // blitter trigger, 0->1 transition
	{
		m_srcx = blit_ram[0x04]+(blit_ram[0x05]<<8);
		m_srcy = blit_ram[0x06]+(blit_ram[0x07]<<8);
		m_dstx = blit_ram[0x08]+(blit_ram[0x09]<<8);
		m_dsty = blit_ram[0x0a]+(blit_ram[0x0b]<<8);
		m_xsize = blit_ram[0x0c]+1;
		m_ysize = blit_ram[0x0e]+1;
		int rle = blit_ram[0x01];
//		int color,color_offs;

/*
	  printf("%02x %02x %02x %02x| (X SRC 4: %02x 5: %02x (val %04x))  (Y SRC 6: %02x 7: %02x (val %04x))  | (X DEST 8: %02x 9: %02x (val %04x))  (Y DEST a: %02x b: %02x (val %04x)) |  %02x %02x %02x %02x\n"
	   ,blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3]


	   ,blit_ram[4],blit_ram[5], m_srcx
	   ,blit_ram[6],blit_ram[7], m_srcy

	   ,blit_ram[8],blit_ram[9], m_dstx
	   ,blit_ram[0xa],blit_ram[0xb], m_dsty
       ,blit_ram[0xc],

		   blit_ram[0xd],blit_ram[0xe],blit_ram[0xf]);
*/
		//if (m_srcx & 0xf800) printf("(error m_srcx &0xf800)");
		//if (m_srcy & 0xf800) printf("(error m_srcy &0xf800)");

		// these are definitely needed for 4bpp..
		m_dstx<<=1;
		m_xsize<<=1;


		//int color = space.machine().rand()&0x1f;

/*
first few compressed transfers

02 08 00 8c|6c 06 73 06|80 03 00 00|27 00 ef 00
data: a8 68 ff 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b
02 08 00 8c|f4 05 16 05|00 00 00 01|3e 00 ef 00
data: a8 68 cd 9a 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b
02 08 00 8c|06 06 16 05|3f 00 00 01|10 00 ef 00
data: a8 68 cd 9a 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 e9 23 95 0a 19 a6 52 8d 0c 54 ce 6a b9 ac 2a 6e 53 0c 55 11 b0 55 a1 ca 2b 30 81 71 15 b7 1b 65 2a ae e1 02 c2 ca bb 7a 03 33 58 f9 d5 d6 0c 51 7e 09 08 2e bf 2c 28 bf 32 ac b8 69 d3 a6 95 85 29 bf 7a 13 c3 cb af e6 66 90 f2 ab ad b1 e5 57 4b f3 58 f9 ad cb 54 4d 5a 44 5c c5 e0 80 cc 64 15 3f d0 b0 e5 98 5a 50 45 da cb aa 52 c8 25 e5 21 8b 55 4a 94 09 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d
02 08 00 8c|ec 06 eb 04|00 00 f0 01|7e 00 77 00
data: 28 13 9a 36 6d 0a 2a b5 02 d5 f7 54 f4 34 b7 15 6d 45 85 aa cb a2 aa 82 ea a2 63 91 92 54 9b 3a aa 50 c5 55 a0 7a 95 4a aa a8 ae 21 84 ea aa 8c 04 aa 52 95 cb 01 6d a6 dc 52 54 2e 07 b4 95 6a 75 55 42 99 d1 b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 65 42 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 29 a8 d4 06 b4 9d eb 28 51 25 4e aa dc cb ea 10 89 f6 b9 dd 2a 5d 83 6b 4b 52 c9 b5 a1 8a b4 b5 c5 18 13 91 b6 0a 21 24 22 95 2b b1 b6 8d c5 58 22 22 da 52 a9 3a 62 d5
02 08 00 8c|66 07 eb 04|7f 00 f0 01|1a 00 77 00
data: 28 13 9a 36 6d 5a d1 0a ac 59 a9 b3 4d 24 c4 2a 55 8b 66 ac d2 34 a9 22 ad a8 44 48 d1 00 46 24 22 19 02 ea 29 47 80 de 23 4c 61 ca 0d 88 d2 24 08 11 d5 55 19 89 0a 75 08 52 94 09 4d 9b 96 59 cd 1a a2 1c d7 b4 29 31 e5 d4 b4 69 d3 a6 4d 9b 36 ad 68 03 16 ac 68 18 45 d3 f2 0c fa 5c b3 5f a4 14 a4 96 d2 54 89 6a 63 aa 26 65 52 d3 a6 15 6d d8 d7 9d ba 3f 3f 5d 4b 3d 4b 87 10 32 a2 0a 0d dc 7d 13 55 41 f8 61 f3 d2 47 9d a9 2a a4 89 fe d2 bf ce 7a ac da b0 ba 4c 19 74 c2 85 5a 52 b9 d1 22 68 18 6c 49 69 2a 38 25 22 6d 73 97 89 55 a1 4a 84 88 f2 94 11 6d d5 a2 5d 0f 55 40 15 c5 4e 4c d5 55 19 06 15 52 99 84 28 63 e8 c1 0a 60 9d 8e 32 73 a7 5e 5a 36 3e 1a c8 5d 72 ac ab 51 f7 9b 53 1a 5b fe dc 48 dc d2 20 6c a9 93 9d bd b6 07 60 f7 87 20 f3 32 3d f2 07 2d ec 0f 4b
02 08 00 8c|74 07 73 04|00 00 00 01|7b 00 ef 00
data: a8 68 2b 9a 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 6d 97 10 42 08 21 84 a8 1a 0c f1 04 49 88 88 20 9d 42 6c d1 40 a1 5a cc 1b 09 91 20 89 48 17 29 85 32 e4 0f 11 69 0f d9 44 90 23 9c 28 99 71 39 63 3b 86 a0 02 92 50 c2 1a 22 89 20 85 50 c8 b4 4a 90 10 22 8a 28 a1 0c 32 a5 22 02 29 43 ca 31 86 64 58 79 01 12 62 15 07 84 42 15 97 09 4d 9b 36 6d da b4 69 d3 a6 4d 9b 56 a4 64 25 10 a5 48 b9 54 2a 11 13 0d 90 88 48 63 8c 34 fd c4 96 44 08 51 cb 30 e7 10 6e f6 39 69 9d 91 7a 4c d1 a6 96 fa aa 97 1d 27 33 31 17 9b cb 25 44 7f 4b b7 0e 8b 46 c0 31 0b 9d 33 05 63 f6 36 d8 78 c9 40 2b 55 96 c6 6c 42
02 08 00 8c|e6 07 73 04|7c 00 00 01|23 00 ef 00
data: a8 68 2b 9a 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 69 d3 a6 4d 9b 36 6d da b4 ec 1e e5 dd 8a aa 0c b0 44 2b aa 2a 44 22 52 96 24 44 55 eb 28 00 69 52 21 51 47 c5 55 60 8e 72 ab 85 10 42 34 19 a2 14 05 10 91 35 42 c8 20 d1 86 90 30 a2 10 ed 1d e8 89 c4 14 c2 10 c3 08 33 a4 8e 84 aa 83 f4 0d 21 88 20 8d 84 a8 a8 80 10 b3 08 92 e8 17 62 15 22 44 1d fb 9c 74 0e 79 c7 2c 95 40 23 91 aa 90 b6 5c ef 9b 6f b1 db 94 12 a2 26 f5 60 64 35 81 12 9f 09 42 d8 61 09 91 21 e2 32 63 a4 30 e2 38 59 58 11 a5 c2 1a b2 64 45 cf b1 1b 8a 23 c9 9c dd 33 51 47 ef 4c 80 26 a0 dc 4b db c0 a4 0d d3 12 81 c4 ac ad 8a 34 a9 21 21 84 a9 16 05 22 f5 14 a5 30 62 e2 5e fb 6b b5 28 85

or as an LSB first 9-bit bitstream

02 08 00 8c|6c 06 73 06|80 03 00 00|27 00 ef 00
data: 000101010 001011011 111111110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110
02 08 00 8c|f4 05 16 05|00 00 00 01|3e 00 ef 00
data: 000101010 001011010 110011010 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110
02 08 00 8c|06 06 16 05|3f 00 00 01|10 00 ef 00
data: 000101010 001011010 110011010 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010111 110001001 010100101 010000100 110000110 010101001 010101100 010011000 000101010 011100110 101011010 011101001 101010101 010001110 110110010 100011000 010101010 100010000 000110110 101010100 001010101 001111010 100000011 001000000 110001110 101010001 110110111 011000101 001100101 010001110 101100001 110100000 001000011 010100111 101110101 011110110 000001100 110000011 010100111 111010101 101101011 001100001 000101001 111110100 100000001 000001110 100111111 010011010 000010100 111111010 100110000 110101000 111011001 011011001 011011001 011010100 110100001 100101001 111110101 011110110 010001100 001111010 011111101 010110011 101100110 000010010 100111111 010101101 101011000 110110100 111111010 101101001 011001111 000110101 001111110 110101110 100110010 101010110 010010110 100010001 000111010 101000110 000011100 000001001 100110010 011010101 000111111 000000101 100001101 101001110 001100101 011010000 010101010 001001011 011110100 110101010 101001010 000100111 010010010 100111100 001001101 000110101 010010100 100010100 110010000 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101 101100101
02 08 00 8c|ec 06 eb 04|00 00 f0 01|7e 00 77 00
data: 000101001 100100001 011001011 011001011 011001010 000010101 001010110 101000000 101010111 110111100 101010001 011110010 110011101 101101010 001011011 010100010 101000010 101010111 010011010 001010101 010101000 001010101 110100010 111000110 100010010 100100100 101010110 110010101 110001010 101000010 101010001 110101010 000001010 101111010 101001010 100100101 010100010 101011101 011000010 000100001 010101110 101010100 110001001 000000101 010101001 010101010 011101001 110000000 101101100 110010100 111011010 010100010 101001110 100111000 000010110 110101001 010101101 010111010 101010010 000101001 100110001 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 011001011 010011001 000010110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010110110 010100000 101010010 101101100 000001011 011011100 111010111 000101001 000101010 100100011 100100101 010100111 011110100 110101011 100001000 100100010 110111110 011101101 110110101 010010111 010110000 011101011 011010010 010010101 001001110 101101100 001010101 000100101 101101011 011010001 100011000 110010001 000100101 101101010 100001000 010000100 100010001 001010100 111010100 100011010 110110110 110001101 000110001 101001000 100010001 000101101 101001010 100101010 101110001 000110101
02 08 00 8c|66 07 eb 04|7f 00 f0 01|1a 00 77 00
data: 000101001 100100001 011001011 011001011 011001011 010100010 110101000 000110101 100110101 001010111 001101101 100100010 010000100 011010101 001010101 011010001 011001100 011010101 001011001 011001001 010101000 100101101 010001010 100100010 000100101 000101100 000000011 000100010 010001000 100100110 000100000 001010111 100101001 110001000 000001011 110111100 010000110 010100001 100101001 110110000 000100010 100101100 100100000 100001000 100010101 011101010 101001100 010010001 010100001 010111000 010000010 010100010 100110010 000101100 101101100 101101001 100110101 011001101 011000010 001010011 100011101 011001011 011001010 010001100 101001110 010101100 101101100 101101100 101101100 101101100 101101100 101101100 101101010 001011011 000000011 010000011 010100010 110000110 001010001 011001011 010011110 011000001 011111001 110101100 110111111 010001001 010010100 000100101 011010010 100101100 101010100 100010101 011011000 110010101 010110010 010100110 010010101 100101101 100101101 010001011 011000011 011111010 111011100 101011101 111111001 111110010 111010110 100101011 110011010 010111000 010000100 001001100 010001010 101000010 110000001 110111011 111011001 000101010 101000001 000011111 100001101 100111101 001011111 000101011 100110010 101010101 000010010 110010001 011111110 100101111 111101011 100110101 111000110 101010110 110000110 101011101 001100101 001100000 101110010 000111010 000101011 010010010 101001110 110001011 010001000 001011000 011000001 101101001 001010010 110010101 000001110 010100100 010001001 011011011 001110111 010011001 000110101 010100001 010101001 000100001 000100010 100111100 101001100 010001011 011010101 011010001 011011101 011110000 101010100 000001010 101000101 000110111 001000110 010101010 111010101 010011000 011000001 010100001 001010100 110010010 000100010 100110001 100001011 110000011 010100000 000011010 111001011 100010100 110011001 110111001 010111101 001011010 011011000 111110001 011000000 100111011 101001001 110001101 011101010 110001010 111011111 101100111 001010010 110001101 101001111 111001110 110001001 000111011 010010110 000010000 110110100 101011100 100110111 001101111 010110110 111100000 000001101 110111111 100001000 001001100 111101001 100101111 000100111 111100000 101101000 011011111 110000110
02 08 00 8c|74 07 73 04|00 00 00 01|7b 00 ef 00
data: 000101010 001011011 010100010 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110110111 010010000 100001000 010000100 001000010 000100001 000101010 101100000 110000100 011110010 000010010 010000100 010001000 100000100 101110010 100001000 110110100 010110000 001010000 101010110 100011001 111011000 100100001 000100100 000100100 100010001 001011101 000100101 001010000 101001100 001001111 111000010 001000100 101101111 000010011 011001000 100000100 111000100 001110010 001010010 011001100 011101001 110011000 110110111 000110000 100000101 010000000 100100100 001010010 000110101 100001000 100100100 010000010 010100001 000010100 001001100 101101010 100100000 100100001 000010001 000101000 100010100 100001010 011000001 001100101 001010100 010001000 000100101 001100001 001010011 100011000 110000100 100110000 110101001 111010000 000010010 000100011 010101000 111000000 010000101 000010101 010001110 100110010 000101100 101101100 101101100 101101100 101101100 101101100 101101100 101101100 101101100 101101100 101101010 001001010 010011010 100100000 010001010 010100010 010100111 010010101 001010100 100010001 100100010 110000000 010010001 000100010 010110001 100011000 100101100 101111110 010001101 101001001 000100001 000010001 010110100 110000110 011100111 000010000 111011001 101111100 111001001 011010111 001100010 010101111 000110010 100010110 110010101 101001010 111110101 010111101 001101110 001110010 011001100 100011001 110100011 011001110 100111010 010000100 010111111 101101001 011101101 011100001 101000101 100010000 000111000 110011010 000101110 011100110 010100000 110001100 110111101 101100000 110110001 111010010 011000000 101101010 010101010 011010010 110001100 110110010
02 08 00 8c|e6 07 73 04|7c 00 00 01|23 00 ef 00
data: 000101010 001011011 010100010 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 110010110 100110111 011110001 010011110 111011010 100010101 010100110 000000011 010010001 011010100 010101010 101010000 100010010 001000100 101001101 001001001 000010001 010101010 110101110 001010000 000000100 101100100 101010000 100100010 101110001 010100011 101010100 000011001 110001010 011101101 010110100 001000010 000100001 000101100 100110000 100010100 101000101 000000000 100010001 001101011 000100001 000010011 000001001 000101101 100001000 010010000 110001000 101000010 001011011 110111000 000101111 001000100 100011001 010000100 001100001 000110000 110001000 011001100 001001010 111000100 100001010 101011100 000100101 111101100 001000010 000010001 000001001 011000100 100001000 101010001 010100000 001000010 001100110 100010000 010010010 001011111 101000010 001101010 100001000 100001000 101011100 011011111 001110010 010111001 110000100 111101110 001100110 100101010 010000001 011000100 100010010 101010100 001001011 011010011 101011110 111110110 011111011 010001101 110110110 010100101 001000010 001010110 010010101 111000001 100010011 010101100 100000010 100100011 111001100 100000100 001000011 011100001 101001000 010001001 100001000 100011101 001100110 001100010 010100001 100010001 110001110 010011010 000110101 000100010 100101010 000110101 100001001 101001001 101010001 011110011 100011011 101100001 010001110 001001001 001100111 001101110 111100110 010001010 111000101 111011100 110010000 000010110 010000000 101001110 111101001 011011011 000000110 010010110 110000110 010110100 100010000 001001000 110011010 110110101 010100010 010110010 010101100 001001000 010000100 001100101 010110100 010100000 010001001 010111100 101000101 001010000 110001000 110010001 110111101 011011111 110101101 010110100 010100101

*/


		if(rle)
		{
			if(rle == 8)
			{

				// compressed stream format:
				//
				// byte 0 = source width   (data is often stored in fairly narrow columns)

#ifdef SHOW_COMPRESSED_DATA_DEBUG
				printf("%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x|%02x %02x %02x %02x\n"
					,blit_ram[0],blit_ram[1],blit_ram[2],blit_ram[3]
					,blit_ram[4],blit_ram[5],blit_ram[6],blit_ram[7]
					,blit_ram[8],blit_ram[9],blit_ram[0xa],blit_ram[0xb]
					,blit_ram[0xc],blit_ram[0xd],blit_ram[0xe],blit_ram[0xf]);
				int count = 0;
				printf("data: ");
				const int show_bytes = 0;
				int bitspacer = 0;
				int bitspace = 9;
				int linespacer = 0;
#endif

				
				m_dstxbase = m_dstx;
				m_dstxcount = 0;
				m_dstycount = 0;
				m_srcxbase = m_srcx;
				m_scrxcount = 0;
				m_srcycount = 0;

				m_sourcewide = m_blit_rom[(((m_srcy)&0x7ff)*0x800)+((m_srcx)&0x7ff)]+1;
				m_srcx++;m_scrxcount++; // we don't want to decode the width as part of the data stream..		
				m_latched_bits_left = 0;
				m_zero_bit_count = 0;

				int lastdata = 0xff; // hack so we can bail when we appear to have run out of compressed data
				bool out_of_data = false;

				for (;;)
				{
					
					int data = gunpey_state_get_stream_bits(8);
					
					// hack, really I imagine there is exactly enough compressed data to fill the dest bitmap area when decompressed, but to stop us
					// overrunning into reading other data we terminate on a 0000, which doesn't seem likely to be compressed data.
					if (data==0x00 && lastdata == 0x00)
						out_of_data = true;

					lastdata = data;


					UINT8 usedata = 0xff;
					if (!out_of_data)
					{
						#ifdef SHOW_COMPRESSED_DATA_DEBUG
						if (count<512)
						{				
							

							if (show_bytes)
							{
								printf("%02x ", data);
							}
							else
							{
								for (int z=0;z<8;z++)
								{
									printf("%d", (data>>(7-z))&1);
									bitspacer++;
									if (bitspacer == bitspace)
									{
										linespacer++;
										if ((linespacer%16) == 0) printf("\n");
										
										printf(" ");
										bitspacer = 0;
									}
								}
							}
							count++;
						}
						#endif

						usedata = data;
					}
					else
						usedata = 0x44;

					if ((write_dest_byte(usedata))==-1)
						break;

				}

#ifdef SHOW_COMPRESSED_DATA_DEBUG
				printf("\n");
#endif

			}
			else
				printf("unknown RLE mode %02x\n",rle);
		}
		else
		{
			m_dstxbase = m_dstx;
			m_dstxcount = 0;
			m_dstycount = 0;
			m_srcxbase = m_srcx;
			m_scrxcount = 0;
			m_srcycount = 0;

			for (;;)
			{
				UINT8 usedata = m_blit_rom[(((m_srcy)&0x7ff)*0x800)+((m_srcx)&0x7ff)];

				m_srcx++; m_scrxcount++;
				if (m_scrxcount==m_xsize)
				{
					m_scrxcount = 0;
					m_srcx = m_srcxbase;
					m_srcy++; m_srcycount++;
				}

				if ((write_dest_byte(usedata))==-1)
					break;
			}
		}

		machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(m_xsize*m_ysize), timer_expired_delegate(FUNC(gunpey_state::blitter_end),this));


/*

*/
	}
}

WRITE8_MEMBER(gunpey_state::gunpey_blitter_upper_w)
{
	//printf("gunpey_blitter_upper_w %02x %02x\n", offset, data);

}

WRITE8_MEMBER(gunpey_state::gunpey_blitter_upper2_w)
{
	//printf("gunpey_blitter_upper2_w %02x %02x\n", offset, data);

}


WRITE8_MEMBER(gunpey_state::gunpey_output_w)
{
	//bit 0 is coin counter
//	popmessage("%02x",data);

	m_oki->set_bank_base(((data & 0x70)>>4) * 0x40000);
}

WRITE16_MEMBER(gunpey_state::gunpey_vram_bank_w)
{
	COMBINE_DATA(&m_vram_bank);
}

WRITE16_MEMBER(gunpey_state::gunpey_vregs_addr_w)
{
	COMBINE_DATA(&m_vreg_addr);
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
	AM_RANGE(0x7fe0, 0x7fe5) AM_WRITE8(gunpey_blitter_upper_w, 0xffff )
	AM_RANGE(0x7ff0, 0x7ff5) AM_WRITE8(gunpey_blitter_upper2_w, 0xffff )

	//AM_RANGE(0x7FF0, 0x7FF1) AM_RAM
	AM_RANGE(0x7fec, 0x7fed) AM_WRITE(gunpey_vregs_addr_w)
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

	if(scanline == 240)
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

	m_blit_rom = memregion("blit_data")->base();
	m_vram = memregion("vram")->base();
	// ...
}

GAME( 2000, gunpey, 0, gunpey, gunpey, gunpey_state, gunpey,    ROT0, "Banpresto", "Gunpey (Japan)",GAME_NOT_WORKING)
