// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina, David Haywood, Luca Elia, Morten Shearman Kirkegaard
/********************************************************************************************

    Gunpey (c) 2000 Banpresto

    TODO:
    - Framebuffer is actually in the same DRAM as the blitter data gets copied to, not a
      separate RAM.  It (and the double buffered copy) appear near the top right of the
      framebuffer, so there are likely registers to control this that need finding.
    - Other sprite modes supported by the video chip (not used here?)
    - Check zooming precision against hardware
    - Cleanup (still a lot of debug code in here)

=============================================================================================
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
#include "machine/timer.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class gunpey_state : public driver_device
{
public:
	gunpey_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_wram(*this, "wram")
		, m_palette(*this, "palette")
		, m_blit_rom(*this, "blit_data")
	{ }

	// TODO: make these non-static and private
	static void set_o(struct state_s *s, unsigned char v);
	static unsigned char get_o(struct state_s *s, int x, int y);
	static void hn_bytes_new_colour(struct state_s *s, unsigned char v, int n);
	static void hn_bytes_prev_colour(struct state_s *s, unsigned char v, int n);
	static void hn_copy_directly(struct state_s *s, unsigned char v, int n);
	static void hn_copy_plus_one(struct state_s *s, unsigned char v, int n);
	static void hn_copy_minus_one(struct state_s *s, unsigned char v, int n);

	void gunpey(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);

	void init_gunpey();
private:

	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(inputs_r);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_WRITE8_MEMBER(blitter_upper_w);
	DECLARE_WRITE8_MEMBER(blitter_upper2_w);
	DECLARE_WRITE8_MEMBER(output_w);
	DECLARE_WRITE16_MEMBER(vram_bank_w);
	DECLARE_WRITE16_MEMBER(vregs_addr_w);
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	TIMER_CALLBACK_MEMBER(blitter_end);

	void irq_check(uint8_t irq_type);
	uint8_t draw_gfx(bitmap_ind16 &bitmap, const rectangle &cliprect, int count, uint8_t scene_gradient);
	uint16_t m_vram_bank;
	uint16_t m_vreg_addr;

	emu_timer *m_blitter_end_timer;

	// work variables for the decompression
	int m_srcx;
	int m_srcxbase;
	int m_scrxcount;
	int m_srcy;
	int m_srcycount;
	int m_ysize;
	int m_xsize;
	int m_dstx;
	int m_dsty;
	int m_dstxbase;
	int m_dstxcount;
	int m_dstycount;
	uint16_t m_blit_ram[0x10];

	uint8_t get_vrom_byte(int x, int y);
	int write_dest_byte(uint8_t usedata);
	int decompress_sprite(unsigned char *buf, int ix, int iy, int ow, int oh, int dx, int dy);
	int next_node(struct huffman_node_s **res, struct state_s *s);
	int get_next_bit(struct state_s *s);

	uint8_t m_irq_cause, m_irq_mask;
	std::unique_ptr<uint16_t[]> m_blit_buffer;
	std::unique_ptr<uint8_t[]> m_vram;

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_shared_ptr<uint16_t> m_wram;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_blit_rom;
};


void gunpey_state::video_start()
{
	m_blit_buffer = std::make_unique<uint16_t[]>(512*512);
	m_vram = std::make_unique<uint8_t[]>(0x400000);
	std::fill_n(&m_vram[0], 0x400000, 0xff);
	m_blitter_end_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gunpey_state::blitter_end), this));
	save_pointer(NAME(m_vram), 0x400000);
}

uint8_t gunpey_state::draw_gfx(bitmap_ind16 &bitmap,const rectangle &cliprect,int count,uint8_t scene_gradient)
{
	int x,y;
	int bpp_sel;
	int color;
	const int ZOOM_SHIFT = 15;
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
		uint32_t zoomheight = (m_wram[count+5] >> 8);
		uint32_t zoomwidth = (m_wram[count+5] & 0xff);
		bpp_sel = (m_wram[count+0] & 0x18);
		color = (m_wram[count+0] >> 8);

		x-=0x160;
		y-=0x188;

		uint32_t sourcewidth  = (m_wram[count+6] & 0xff)<<ZOOM_SHIFT;
		uint32_t sourceheight = (m_wram[count+7] >> 8)<<ZOOM_SHIFT;
		int xsource = ((m_wram[count+2] & 0x003f) << 5) | ((m_wram[count+1] & 0xf800) >> 11);
		int ysource = ((m_wram[count+3] & 0x001f) << 6) | ((m_wram[count+2] & 0xfc00) >> 10);

		int alpha =  m_wram[count+1] & 0x1f;

		uint32_t widthstep = 1<<ZOOM_SHIFT;
		uint32_t heightstep = 1<<ZOOM_SHIFT;

		if (zoomwidth) widthstep = sourcewidth / zoomwidth;
		if (zoomheight) heightstep = sourceheight / zoomheight;

		uint16_t unused;
		if (debug) logerror("sprite %04x %04x %04x %04x %04x %04x %04x %04x\n", m_wram[count+0], m_wram[count+1], m_wram[count+2], m_wram[count+3], m_wram[count+4], m_wram[count+5], m_wram[count+6], m_wram[count+7]);

		unused = m_wram[count+0]&~0xff98; if (unused) logerror("unused bits set in word 0 - %04x\n", unused);
		unused = m_wram[count+1]&~0xf89f; if (unused) logerror("unused bits set in word 1 - %04x\n", unused);
		unused = m_wram[count+2]&~0xfc3f; if (unused) logerror("unused bits set in word 2 - %04x\n", unused);
		unused = m_wram[count+3]&~0xff1f; if (unused) logerror("unused bits set in word 3 - %04x\n", unused);
		unused = m_wram[count+4]&~0xff77; if (unused) logerror("unused bits set in word 4 - %04x\n", unused);
		unused = m_wram[count+5]&~0xffff; if (unused) logerror("unused bits set in word 5 - %04x\n", unused);
		unused = m_wram[count+6]&~0x00ff; if (unused) logerror("unused bits set in word 6 - %04x\n", unused);
		unused = m_wram[count+7]&~0xff00; if (unused) logerror("unused bits set in word 7 - %04x\n", unused);

		if (((zoomwidth<<ZOOM_SHIFT) != sourcewidth) || ((zoomheight<<ZOOM_SHIFT) != sourceheight))
		{
		//  logerror("sw %08x zw %08x sh %08x zh %08x heightstep %08x widthstep %08x \n", sourcewidth, zoomwidth<<ZOOM_SHIFT, sourceheight, zoomheight<<ZOOM_SHIFT, heightstep, widthstep );
		}

		if(bpp_sel == 0x00)  // 4bpp
		{
			int ysourceoff = 0;
			for(int yi=0;yi<zoomheight;yi++)
			{
				int yi2 = ysourceoff>>ZOOM_SHIFT;
				int xsourceoff = 0;

				for(int xi=0;xi<zoomwidth;xi++)
				{
					int xi2 = xsourceoff>>ZOOM_SHIFT;
					uint8_t data = m_vram[((((ysource + yi2) & 0x7ff) * 0x800) + ((xsource + (xi2/2)) & 0x7ff))];
					uint8_t pix;
					uint32_t col_offs;
					uint16_t color_data;

					if (xi2 & 1)
					{
						pix = (data & 0xf0)>>4;
					}
					else
					{
						pix = (data & 0x0f);
					}

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

						if(cliprect.contains(x+xi, y+yi))
						{
							if (alpha==0x00) // a value of 0x00 is solid
							{
								bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
							}
							else
							{
								uint16_t basecolor = bitmap.pix16(y+yi, x+xi);
								int base_r = ((basecolor >> 10)&0x1f)*alpha;
								int base_g = ((basecolor >> 5)&0x1f)*alpha;
								int base_b = ((basecolor >> 0)&0x1f)*alpha;
								int r = ((color_data & 0x7c00) >> 10)*(0x1f-alpha);
								int g = ((color_data & 0x03e0) >> 5)*(0x1f-alpha);
								int b = ((color_data & 0x001f) >> 0)*(0x1f-alpha);
								r = (base_r+r)/0x1f;
								g = (base_g+g)/0x1f;
								b = (base_b+b)/0x1f;
								color_data = (color_data & 0x8000) | (r << 10) | (g << 5) | (b << 0);
								bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
							}
						}
					}
					xsourceoff += widthstep;
				}
				ysourceoff += heightstep;
			}
		}
		else if(bpp_sel == 0x08) // 6bpp
		{
			// not used by Gunpey?
			logerror("6bpp\n");
		}
		else if(bpp_sel == 0x10) // 8bpp
		{
			int ysourceoff = 0;
			for(int yi=0;yi<zoomheight;yi++)
			{
				int yi2 = ysourceoff>>ZOOM_SHIFT;
				int xsourceoff = 0;

				for(int xi=0;xi<zoomwidth;xi++)
				{
					int xi2 = xsourceoff>>ZOOM_SHIFT;

					uint8_t data = m_vram[((((ysource+yi2)&0x7ff)*0x800) + ((xsource+xi2)&0x7ff))];
					uint8_t pix;
					uint32_t col_offs;
					uint16_t color_data;

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

						if(cliprect.contains(x+xi,y+yi))
						{
							if (alpha==0x00) // a value of 0x00 is solid
							{
								bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
							}
							else
							{
								uint16_t basecolor = bitmap.pix16(y+yi, x+xi);
								int base_r = ((basecolor >> 10)&0x1f)*alpha;
								int base_g = ((basecolor >> 5)&0x1f)*alpha;
								int base_b = ((basecolor >> 0)&0x1f)*alpha;
								int r = ((color_data & 0x7c00) >> 10)*(0x1f-alpha);
								int g = ((color_data & 0x03e0) >> 5)*(0x1f-alpha);
								int b = ((color_data & 0x001f) >> 0)*(0x1f-alpha);
								r = (base_r+r)/0x1f;
								g = (base_g+g)/0x1f;
								b = (base_b+b)/0x1f;
								color_data = (color_data & 0x8000) | (r << 10) | (g << 5) | (b << 0);
								bitmap.pix16(y+yi, x+xi) = color_data & 0x7fff;
							}
						}
					}
					xsourceoff += widthstep;
				}
				ysourceoff += heightstep;
			}
		}
		else if(bpp_sel == 0x18) // RGB32k
		{
			// not used by Gunpey?
			logerror("32k\n");
			// ...
		}
	}

	return m_wram[count+0] & 0x80;
}

uint32_t gunpey_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint16_t *blit_buffer = m_blit_buffer;
	uint16_t vram_bank = m_vram_bank & 0x7fff;
	uint16_t vreg_addr = m_vreg_addr & 0x7fff;
	uint8_t end_mark;
	int count;
	int scene_index;

	bitmap.fill(m_palette->pen(0), cliprect); //black pen

	if((!(m_vreg_addr & 0x8000)) || (!(m_vram_bank & 0x8000)))
		return 0;

	for(scene_index = vreg_addr/2;scene_index<(vreg_addr+0x400)/2;scene_index+=0x10/2)
	{
		uint16_t start_offs;
		uint16_t end_offs;
		uint8_t scene_end_mark;
		uint8_t scene_enabled;
		uint8_t scene_gradient;

		start_offs = (vram_bank+(m_wram[scene_index+5] << 8))/2;
		end_offs = (vram_bank+(m_wram[scene_index+5] << 8)+0x1000)/2; //safety check
		scene_end_mark = m_wram[scene_index+0] & 0x80;
		scene_enabled = m_wram[scene_index+0] & 0x01;
		scene_gradient = m_wram[scene_index+1] & 0xff;

//      logerror("%08x: %08x %08x %08x %08x | %08x %08x %08x %08x\n",scene_index,m_wram[scene_index+0],m_wram[scene_index+1],m_wram[scene_index+2],m_wram[scene_index+3],
//                                                  m_wram[scene_index+4],m_wram[scene_index+5],m_wram[scene_index+6],m_wram[scene_index+7]);

		if(scene_enabled)
		{
			for(count = start_offs;count<end_offs;count+=0x10/2)
			{
				end_mark = draw_gfx(bitmap,cliprect,count,scene_gradient);

				if(end_mark == 0x80)
					break;
			}
		}

		if(scene_end_mark == 0x80)
			break;
	}

	return 0;
}

void gunpey_state::irq_check(uint8_t irq_type)
{
	m_irq_cause |= irq_type;

	if(m_irq_cause & m_irq_mask)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x200/4); // V30
	else
		m_maincpu->set_input_line_and_vector(0, CLEAR_LINE, 0x200/4); // V30
}

WRITE8_MEMBER(gunpey_state::status_w)
{
	if(offset == 1)
	{
		m_irq_cause &= ~data;
		irq_check(0);
	}

	if(offset == 0)
	{
		m_irq_mask = data;
		irq_check(0);
	}
}

READ8_MEMBER(gunpey_state::status_r)
{
	if(offset == 1)
		return m_irq_cause;

	return m_irq_mask;
}

READ8_MEMBER(gunpey_state::inputs_r)
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
	irq_check(4);
}


int gunpey_state::write_dest_byte(uint8_t usedata)
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


inline uint8_t gunpey_state::get_vrom_byte(int x, int y)
{
	return m_blit_rom[((x)+2048 * (y)) & m_blit_rom.mask()];
}

struct state_s
{
	unsigned char *buf;
	unsigned char bits;
	int num_bits;
	unsigned char colour[16];
	int basex;
	int ix, iy, iw;
	int dx, dy;
	int ow, oh;
	int ox, oy;
};

inline int gunpey_state::get_next_bit(struct state_s *s)
{
	if (s->num_bits == 0)
	{
		if (s->ix >= s->basex + s->iw)
		{
			s->ix = s->basex;
			s->iy++;
		}
		s->bits = get_vrom_byte(s->ix, s->iy);
		s->ix++;
		s->num_bits = 8;
	}

	int res = 1 & s->bits;

	s->bits >>= 1;
	s->num_bits--;

	return res;
}


void gunpey_state::set_o(struct state_s *s, unsigned char v)
{
	assert(s->ox >= 0);
	assert(s->ox < s->ow);
	assert(s->ox < 256);
	assert(s->oy >= 0);
	assert(s->oy < s->oh);
	assert(s->oy < 256);

	unsigned char a = v;
	for (int i = 0; i < sizeof(s->colour) / sizeof(*s->colour); i++)
	{
		unsigned char b = s->colour[i];
		s->colour[i] = a;
		a = b;
		if (a == v)
			break;
	}

	s->buf[((s->dx + s->ox++) & 0x7ff) + (((s->dy + s->oy) & 0x7ff) * 0x800)] = v;
}


unsigned char gunpey_state::get_o(struct state_s *s, int x, int y)
{
	assert(x >= 0);
	assert(x < s->ow);
	assert(x < 256);
	assert(y >= 0);
	assert(y < s->oh);
	assert(y < 256);

	return s->buf[((s->dx + x) & 0x7ff) + (((s->dy + y) & 0x7ff) * 0x800)];
}


void gunpey_state::hn_bytes_new_colour(struct state_s *s, unsigned char v, int n)
{
	for (int i = 0; i < n; i++)
	{
		set_o(s, v);
	}
}


void gunpey_state::hn_bytes_prev_colour(struct state_s *s, unsigned char v, int n)
{
	int c = s->colour[v];

	for (int i = 0; i < n; i++)
	{
		set_o(s, c);
	}
}


void gunpey_state::hn_copy_directly(struct state_s *s, unsigned char v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s->ox / 12) & 1)
		{
			set_o(s, get_o(s, s->ox, s->oy + 1));
		}
		else
		{
			set_o(s, get_o(s, s->ox, s->oy - 1));
		}
	}
}



void gunpey_state::hn_copy_plus_one(struct state_s *s, unsigned char v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s->ox / 12) & 1)
		{
			set_o(s, get_o(s, s->ox + 1, s->oy + 1));
		}
		else
		{
			set_o(s, get_o(s, s->ox + 1, s->oy - 1));
		}
	}
}


void gunpey_state::hn_copy_minus_one(struct state_s *s, unsigned char v, int n)
{
	for (int i = 0; i < n; i++)
	{
		if ((s->ox / 12) & 1)
		{
			set_o(s, get_o(s, s->ox - 1, s->oy + 1));
		}
		else
		{
			set_o(s, get_o(s, s->ox - 1, s->oy - 1));
		}
	}
}


static struct huffman_node_s
{
	const char *bits;
	void (*func)(struct state_s *, unsigned char, int);
	int arg0_bits;
	int arg1_val;
} hn[] = {
	{ "11",                 gunpey_state::hn_bytes_new_colour,   8,  1 },
	{ "10111",              gunpey_state::hn_bytes_new_colour,   8,  2 },
	{ "10110011",           gunpey_state::hn_bytes_new_colour,   8,  3 },
	{ "1010011000",         gunpey_state::hn_bytes_new_colour,   8,  4 },
	{ "1010101010000",      gunpey_state::hn_bytes_new_colour,   8,  5 },
	{ "101010001011110",    gunpey_state::hn_bytes_new_colour,   8,  6 },
	{ "101010010101110",    gunpey_state::hn_bytes_new_colour,   8,  7 },
	{ "101010010101111",    gunpey_state::hn_bytes_new_colour,   8,  8 },
	{ "101010010101100",    gunpey_state::hn_bytes_new_colour,   8,  9 },
	{ "10101000101100",     gunpey_state::hn_bytes_new_colour,   8, 10 },
	{ "101010010101101",    gunpey_state::hn_bytes_new_colour,   8, 11 },
	{ "10101000101101",     gunpey_state::hn_bytes_new_colour,   8, 12 },
	{ "0",                  gunpey_state::hn_bytes_prev_colour,  4,  1 },
	{ "100",                gunpey_state::hn_bytes_prev_colour,  4,  2 },
	{ "101011",             gunpey_state::hn_bytes_prev_colour,  4,  3 },
	{ "1010010",            gunpey_state::hn_bytes_prev_colour,  4,  4 },
	{ "101010100",          gunpey_state::hn_bytes_prev_colour,  4,  5 },
	{ "1010100100",         gunpey_state::hn_bytes_prev_colour,  4,  6 },
	{ "10101010110",        gunpey_state::hn_bytes_prev_colour,  4,  7 },
	{ "10100111000",        gunpey_state::hn_bytes_prev_colour,  4,  8 },
	{ "101010101001",       gunpey_state::hn_bytes_prev_colour,  4,  9 },
	{ "101001111000",       gunpey_state::hn_bytes_prev_colour,  4, 10 },
	{ "101010010100",       gunpey_state::hn_bytes_prev_colour,  4, 11 },
	{ "1010011001",         gunpey_state::hn_bytes_prev_colour,  4, 12 },
	{ "101101",             gunpey_state::hn_copy_directly,      0,  2 },
	{ "10101011",           gunpey_state::hn_copy_directly,      0,  3 },
	{ "101010011",          gunpey_state::hn_copy_directly,      0,  4 },
	{ "101001101",          gunpey_state::hn_copy_directly,      0,  5 },
	{ "1010011111",         gunpey_state::hn_copy_directly,      0,  6 },
	{ "1010100011",         gunpey_state::hn_copy_directly,      0,  7 },
	{ "10101000100",        gunpey_state::hn_copy_directly,      0,  8 },
	{ "101010101111",       gunpey_state::hn_copy_directly,      0,  9 },
	{ "101001110010",       gunpey_state::hn_copy_directly,      0, 10 },
	{ "1010011100111",      gunpey_state::hn_copy_directly,      0, 11 },
	{ "101100101",          gunpey_state::hn_copy_directly,      0, 12 },
	{ "1011000",            gunpey_state::hn_copy_plus_one,      0,  2 },
	{ "101010000",          gunpey_state::hn_copy_plus_one,      0,  3 },
	{ "10101001011",        gunpey_state::hn_copy_plus_one,      0,  4 },
	{ "101010001010",       gunpey_state::hn_copy_plus_one,      0,  5 },
	{ "1010101011101",      gunpey_state::hn_copy_plus_one,      0,  6 },
	{ "1010101011100",      gunpey_state::hn_copy_plus_one,      0,  7 },
	{ "1010011110010",      gunpey_state::hn_copy_plus_one,      0,  8 },
	{ "10101000101110",     gunpey_state::hn_copy_plus_one,      0,  9 },
	{ "101010001011111",    gunpey_state::hn_copy_plus_one,      0, 10 },
	{ "1010011110011",      gunpey_state::hn_copy_plus_one,      0, 11 },
	{ "101000",             gunpey_state::hn_copy_minus_one,     0,  2 },
	{ "101100100",          gunpey_state::hn_copy_minus_one,     0,  3 },
	{ "1010011101",         gunpey_state::hn_copy_minus_one,     0,  4 },
	{ "10101010101",        gunpey_state::hn_copy_minus_one,     0,  5 },
	{ "101001111011",       gunpey_state::hn_copy_minus_one,     0,  6 },
	{ "101001111010",       gunpey_state::hn_copy_minus_one,     0,  7 },
	{ "1010101010001",      gunpey_state::hn_copy_minus_one,     0,  8 },
	{ "1010011100110",      gunpey_state::hn_copy_minus_one,     0,  9 },
	{ "10101001010101",     gunpey_state::hn_copy_minus_one,     0, 10 },
	{ "10101001010100",     gunpey_state::hn_copy_minus_one,     0, 11 },
	{ NULL },
};


int gunpey_state::next_node(struct huffman_node_s **res, struct state_s *s)
{
	char bits[128];

	memset(bits, 0, sizeof(bits));

	for (int i = 0; i < sizeof(bits) - 1; i++)
	{
		bits[i] = '0' + get_next_bit(s);

		for (int j = 0;; j++)
		{
			if (!hn[j].bits) return -1;

			if (strncmp(bits, hn[j].bits, i + 1) == 0)
			{
				if (!hn[j].bits[i + 1])
				{
					*res = &hn[j];
					return 0;
				}
				break;
			}
		}
	}

	return -1;
}

int gunpey_state::decompress_sprite(unsigned char *buf, int ix, int iy, int ow, int oh, int dx, int dy)
{
	struct huffman_node_s *n;
	struct state_s s;
	unsigned char v;
	int eol;

	s.buf = buf;
	s.num_bits = 0;

	memset(s.colour, 0, sizeof(s.colour));

	s.basex = ix;

	s.ix = ix;
	s.iy = iy;
	s.iw = get_vrom_byte(s.ix++, s.iy) + 1;
	s.dx = dx;
	s.dy = dy;

	s.ow = ow;
	s.oh = oh;
	s.ox = 0;
	s.oy = 0;

	int temp;
	temp = get_next_bit(&s);
	temp |= get_next_bit(&s) << 1;
	temp |= get_next_bit(&s) << 2;
	assert(temp == 0);

	while (s.ox < s.ow)
	{
		if (next_node(&n, &s))
			return -1;

		v = 0;
		for (int i = 0; i < n->arg0_bits; i++)
		{
			v |= get_next_bit(&s) << i;
		}

		n->func(&s, v, n->arg1_val);

		if ((s.ox % 12) == 0)
		{
			s.ox -= 12;
			if ((s.ox / 12) & 1)
			{
				s.oy -= 1;
			}
			else
			{
				s.oy += 1;
			}
			eol = 1;

		}
		else if (s.ox == s.ow)
		{
			s.ox -= s.ow % 12;
			if ((s.ox / 12) & 1)
			{
				s.oy -= 1;
			}
			else
			{
				s.oy += 1;
			}
			eol = 1;

		}
		else
		{
			eol = 0;
		}

		if (eol)
		{
			if ((s.oy == s.oh) || (s.oy == -1))
			{
				s.ox += 12;
				if ((s.ox / 12) & 1)
				{
					s.oy -= 1;
				}
				else
				{
					s.oy += 1;
				}
			}
		}
	}

	return 0;
}

WRITE8_MEMBER(gunpey_state::blitter_w)
{
	uint16_t *blit_ram = m_blit_ram;

	//logerror("gunpey_blitter_w offset %01x data %02x\n", offset,data);

	blit_ram[offset] = data;

	if(offset == 0 && data == 2) // blitter trigger, 0->1 transition
	{
		m_srcx = blit_ram[0x04]|(blit_ram[0x05]<<8);
		m_srcy = blit_ram[0x06]|(blit_ram[0x07]<<8);
		m_dstx = blit_ram[0x08]|(blit_ram[0x09]<<8);
		m_dsty = blit_ram[0x0a]|(blit_ram[0x0b]<<8);
		m_xsize = blit_ram[0x0c]+1;
		m_ysize = blit_ram[0x0e]+1;
		int compression = blit_ram[0x01];

		m_dstx<<=1;
		m_xsize<<=1;

		if(compression)
		{
			if(compression == 8)
			{
				if (decompress_sprite(m_vram.get(), m_srcx, m_srcy, m_xsize, m_ysize, m_dstx, m_dsty))
				{
					logerror("[-] Failed to decompress sprite at %04x %04x\n", m_srcx, m_srcy);
				}
			}
			else
				logerror("unknown compression mode %02x\n",compression);
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
				uint8_t usedata = m_blit_rom[(((m_srcy)&0x7ff)*0x800)+((m_srcx)&0x7ff)];
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

		m_blitter_end_timer->adjust(m_maincpu->cycles_to_attotime(m_xsize*m_ysize));
	}
}

WRITE8_MEMBER(gunpey_state::blitter_upper_w)
{
	//logerror("gunpey_blitter_upper_w %02x %02x\n", offset, data);

}

WRITE8_MEMBER(gunpey_state::blitter_upper2_w)
{
	//logerror("gunpey_blitter_upper2_w %02x %02x\n", offset, data);

}


WRITE8_MEMBER(gunpey_state::output_w)
{
	//bit 0 is coin counter
//  popmessage("%02x",data);

	m_oki->set_rom_bank((data & 0x70) >> 4);
}

WRITE16_MEMBER(gunpey_state::vram_bank_w)
{
	COMBINE_DATA(&m_vram_bank);
}

WRITE16_MEMBER(gunpey_state::vregs_addr_w)
{
	COMBINE_DATA(&m_vreg_addr);
}

/***************************************************************************************/

void gunpey_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).ram().share("wram");
//  map(0x50000, 0x500ff).ram();
//  map(0x50100, 0x502ff).noprw();
	map(0x80000, 0xfffff).rom();
}

void gunpey_state::io_map(address_map &map)
{
	map(0x7f40, 0x7f45).r(FUNC(gunpey_state::inputs_r));

	map(0x7f48, 0x7f48).w(FUNC(gunpey_state::output_w));
	map(0x7f80, 0x7f81).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0x7f88, 0x7f88).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));

	map(0x7fc8, 0x7fc9).rw(FUNC(gunpey_state::status_r), FUNC(gunpey_state::status_w));
	map(0x7fd0, 0x7fdf).w(FUNC(gunpey_state::blitter_w));
	map(0x7fe0, 0x7fe5).w(FUNC(gunpey_state::blitter_upper_w));
	map(0x7ff0, 0x7ff5).w(FUNC(gunpey_state::blitter_upper2_w));

	//map(0x7ff0, 0x7ff1).ram();
	map(0x7fec, 0x7fed).w(FUNC(gunpey_state::vregs_addr_w));
	map(0x7fee, 0x7fef).w(FUNC(gunpey_state::vram_bank_w));

}


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

/*:
0x01
0x04 blitter ready
0x10 vblank too? (otherwise you'll get various hangs/inputs stop to work)
0x40 almost certainly vblank (reads inputs)
0x80
*/
TIMER_DEVICE_CALLBACK_MEMBER(gunpey_state::scanline)
{
	int scanline = param;

	if(scanline == 240)
	{
		//logerror("frame\n");
		irq_check(0x50);
	}
}

/***************************************************************************************/
void gunpey_state::gunpey(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, 57242400 / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &gunpey_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &gunpey_state::io_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(gunpey_state::scanline), "screen", 0, 1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(57242400/8, 442, 0, 320, 264, 0, 240); /* just to get ~60 Hz */
	screen.set_screen_update(FUNC(gunpey_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::RGB_555);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	OKIM6295(config, m_oki, XTAL(16'934'400) / 8, okim6295_device::PIN7_LOW);
	m_oki->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_oki->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'934'400)));
	ymz.add_route(0, "lspeaker", 0.25);
	ymz.add_route(1, "rspeaker", 0.25);
}

/***************************************************************************************/

ROM_START( gunpey )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* V30 code */
	ROM_LOAD16_BYTE( "gp_rom1.021",  0x00000, 0x80000, CRC(07a589a7) SHA1(06c4140ffd5f74b3d3ddfc424f43fcd08d903490) )
	ROM_LOAD16_BYTE( "gp_rom2.022",  0x00001, 0x80000, CRC(f66bc4cf) SHA1(54931d878d228c535b9e2bf22a0a3e41756f0fe5) )

	ROM_REGION( 0x400000, "blit_data", 0 )
	ROM_LOAD( "gp_rom3.025",  0x00000, 0x400000,  CRC(f2d1f9f0) SHA1(0d20301fd33892074508b9d127456eae80cc3a1c) )

	ROM_REGION( 0x400000, "ymz", 0 )
	ROM_LOAD( "gp_rom4.525",  0x000000, 0x400000, CRC(78dd1521) SHA1(91d2046c60e3db348f29f776def02e3ef889f2c1) ) // 11xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "oki", 0 )
	ROM_LOAD( "gp_rom5.622",  0x000000, 0x400000,  CRC(f79903e0) SHA1(4fd50b4138e64a48ec1504eb8cd172a229e0e965)) // 1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

void gunpey_state::init_gunpey()
{
}

GAME( 2000, gunpey, 0, gunpey, gunpey, gunpey_state, init_gunpey, ROT0, "Bandai / Banpresto", "Gunpey (Japan)", 0 )
