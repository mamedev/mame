// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/****************************************************************

Wheels & Fire
Power Ball

driver by
 Tomasz Slanina


TODO:
- analogue accelerator is really slow
- various gfx size/pos glitches (gaps, extra rows of pixels here and there)
- real gfx zoom, based on the zoom params, not lookup table
- apply double buffering (sometimes gfx is displayed at y+256 every other frame (extra bits are currently masked out))
- fix road/sky (extra bits in the scroll reg. are there two bitmap buffers? )
- bitmap layer clearing
- fix wrong coords of sprites rendered into the bitmap layer (intro car)
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


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

static const int ZOOM_TABLE_SIZE=1<<14;
static const int NUM_SCANLINES=256-8;
static const int NUM_VBLANK_LINES=8;
static const int LAYER_BG=0;
static const int LAYER_FG=1;
static const int NUM_COLORS=256;

struct scroll_info
{
		int32_t x = 0, y = 0, unkbits = 0;
};


class wheelfir_state : public driver_device
{
public:
	wheelfir_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_adc_eoc(0),
		m_is_pwball(false),
		m_disable_raster_irq(false)
	{ }

	void wheelfir(machine_config &config);
	void kongball(machine_config &config);

	int adc_eoc_r();

	void init_pwball();
	void init_kongball();

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<int32_t[]> m_zoom_table;
	std::unique_ptr<uint16_t[]> m_blitter_data;

	std::unique_ptr<scroll_info[]> m_scanlines;

	int32_t m_direct_write_x0 = 0;
	int32_t m_direct_write_x1 = 0;
	int32_t m_direct_write_y0 = 0;
	int32_t m_direct_write_y1 = 0;
	int32_t m_direct_write_idx = 0;

	int16_t m_scanline_cnt = 0;


	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[2]{};

	int32_t get_scale(int32_t index)
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
	void wheelfir_scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wheelfir_blit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void wheelfir_7c0000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t wheelfir_7c0000_r(offs_t offset, uint16_t mem_mask = ~0);
	void coin_cnt_w(uint16_t data);
	void adc_eoc_w(int state);
	uint32_t screen_update_wheelfir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_wheelfir(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer_callback);
	void ramdac_map(address_map &map);
	void wheelfir_main(address_map &map);
	void wheelfir_sub(address_map &map);
	int m_adc_eoc;

	bool m_is_pwball;
	bool m_disable_raster_irq;
};


void wheelfir_state::wheelfir_scanline_cnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scanline_cnt);
}


void wheelfir_state::wheelfir_blit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_blitter_data[offset]);

	if(!ACCESSING_BITS_8_15 && offset==0x6)  //LSB only!
	{
		int direct_width=m_direct_write_x1-m_direct_write_x0+1;
		int direct_height=m_direct_write_y1-m_direct_write_y0+1;

		int sixdat = data&0xff;

		if(direct_width>0 && direct_height>0)
		{
			int x = m_direct_write_idx % direct_width;
			int y = (m_direct_write_idx / direct_width) % direct_height;

			x+=m_direct_write_x0;
			y+=m_direct_write_y0;

			if(x<512 && y <512)
			{
				m_tmp_bitmap[LAYER_BG]->pix(y, x) = sixdat;
			}
		}

		++m_direct_write_idx;

		return;
	}

	if(offset==0xf && data==0xffff)
	{
		m_maincpu->set_input_line(1, HOLD_LINE);

		uint8_t const *const rom = memregion("gfx1")->base();

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


		if (!m_is_pwball)
		{
			if (page >= 0x400000) /* src set to  unav. page before direct write to the framebuffer */
			{
				m_direct_write_x0 = dst_x0;
				m_direct_write_x1 = dst_x1;
				m_direct_write_y0 = dst_y0;
				m_direct_write_y1 = dst_y1;
				m_direct_write_idx = 0;
			}
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

		const float scale_x=get_scale(index);


		d1=((m_blitter_data[0x0b]&0xc000)>>14) |
			((m_blitter_data[0x0c]&0xc000)>>12) |
			((m_blitter_data[0x0a]&0x4000)>>10);

		d2=((m_blitter_data[0x0c]&0x1f00)>>8);


		d1|=((m_blitter_data[0x8]&0x200)>>4);
		d2|=((m_blitter_data[0x8]&0x800)>>6);

		hflag=(m_blitter_data[0x9]&0x2)?1:0;
		dflag=(m_blitter_data[0x8]&0x2000)?1:0;
		index=d1|(d2<<6)|(hflag<<12)|(dflag<<13);

		const float scale_y=get_scale(index);


		if(scale_x==0 || scale_y==0) return;


		float scale_x_step = 100.f / scale_x;
		float scale_y_step = 100.f / scale_y;

		if (m_is_pwball)
		{
			scale_x_step = 1.0f;
			scale_y_step = 1.0f;
		}

		int vpage=LAYER_FG;
		if(m_blitter_data[0x7]&0x10)
		{
			vpage=LAYER_BG;
/*
            printf("%s bg -> %d %d   %d %d  %d %d @ %x\n",machine().describe_context().c_str(), dst_x0,dst_y0, dst_x1,dst_y1, dst_x1-dst_x0, dst_y1-dst_y0);

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

		float idx_x = 0;
		for(int x=dst_x0; !endx; x+=x_dst_step, idx_x+=scale_x_step)
		{
			endy=false;
			float idx_y = 0;
			for(int y=dst_y0; !endy; y+=y_dst_step, idx_y+=scale_y_step)
			{
				endx=(x==(dst_x1+1));
				endy=(y==(dst_y1+1));


				int xx=src_x0+x_src_step*idx_x;
				int yy=src_y0+y_src_step*idx_y;

				int address=page+yy*512+xx;

				int pix = rom[address&(0x1000000-1)];

				int screen_x=x;
				int screen_y=y;


				if ((page>=0x400000) && (!m_is_pwball))
				{
					//hack for clear
					if(screen_x >0 && screen_y >0 && screen_x < width && screen_y <height)
					{
				//      m_tmp_bitmap[vpage]->pix(screen_y , screen_x ) =0;
					}
				}

				{
					if (vpage == LAYER_FG) screen_y&=0xff;

					if(pix && screen_x >0 && screen_y >0 && screen_x < width && screen_y <height)
					{
						m_tmp_bitmap[vpage]->pix(screen_y, screen_x) = pix;
					}
				}
			}
		}
	}
}

void wheelfir_state::video_start()
{
	m_tmp_bitmap[0] = std::make_unique<bitmap_ind16>(512, 512);
	m_tmp_bitmap[1] = std::make_unique<bitmap_ind16>(512, 512);
}

uint32_t wheelfir_state::screen_update_wheelfir(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for(int y=cliprect.min_y; y < cliprect.max_y; y++)
	{
		uint16_t const *const source = &m_tmp_bitmap[LAYER_BG]->pix(( (m_scanlines[y].y)&511));
		uint16_t *const dest = &bitmap.pix(y);

		for (int x = cliprect.min_x; x < cliprect.max_x; x++)
		{
			dest[x] = source[(x + m_scanlines[y].x) & 511];
		}
	}

	copybitmap_trans(bitmap, *m_tmp_bitmap[LAYER_FG], 0, 0, 0, 0, cliprect, 0);

/*
    m_tmp_bitmap[LAYER_BG]->fill(0, screen.visible_area());
*/

	return 0;
}

void wheelfir_state::screen_vblank_wheelfir(int state)
{
	// rising edge
	if (state)
	{
		m_tmp_bitmap[LAYER_FG]->fill(0, m_screen->visible_area());
	}
}


void wheelfir_state::wheelfir_7c0000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		//{uint16_t x = data & 0xf800; static int y = -1; if (x != y) { y = x; printf("%s wheelfir_7c0000_w %d%d%d%d%d\n", machine().describe_context().c_str(), BIT(data, 15), BIT(data, 14), BIT(data, 13), BIT(data, 12), BIT(data, 11)); }}
		//{uint16_t x = data & 0x0700; static int y = -1; if (x != y) { y = x; printf("%s eeprom write %d%d%d\n", machine().describe_context().c_str(), BIT(data, 10), BIT(data, 9), BIT(data, 8)); }}
		m_eeprom->di_write(BIT(data, 9));
		m_eeprom->clk_write(BIT(data, 8));
		m_eeprom->cs_write(BIT(data, 10));
	}

	if (ACCESSING_BITS_0_7)
	{
		/// alternates between ac & ff
		//printf("7c0001 %02x\n", data & 0xff);
		/* seems to be scanline width/2 (used for scanline int timing ? or real width of scanline ?) */
	}
}

uint16_t wheelfir_state::wheelfir_7c0000_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_8_15)
	{
		data |= (machine().rand() & 0x2000); // ?
		data |= m_eeprom->do_read() << 15;
		//printf("%s eeprom read %04x %04x\n", machine().describe_context().c_str(), data, mem_mask);
	}

	if (ACCESSING_BITS_0_7)
	{
		//printf("unknown read %04x %04x\n", data, mem_mask);
	}

	return data;
}

void wheelfir_state::coin_cnt_w(uint16_t data)
{
	/* bits 0/1 coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

void wheelfir_state::wheelfir_main(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x700000, 0x70001f).w(FUNC(wheelfir_state::wheelfir_blit_w));
	map(0x720001, 0x720001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x720003, 0x720003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x720005, 0x720005).w("ramdac", FUNC(ramdac_device::mask_w)); // word write?
	map(0x740000, 0x740001).w("soundlatch", FUNC(generic_latch_16_device::write));
	map(0x760000, 0x760001).w(FUNC(wheelfir_state::coin_cnt_w));
	map(0x780000, 0x78000f).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask16(0x00ff);
	map(0x7a0000, 0x7a0001).w(FUNC(wheelfir_state::wheelfir_scanline_cnt_w));
	map(0x7c0000, 0x7c0001).rw(FUNC(wheelfir_state::wheelfir_7c0000_r), FUNC(wheelfir_state::wheelfir_7c0000_w));
	map(0x7e0000, 0x7e0001).portr("P1");
	map(0x7e0002, 0x7e0003).portr("P2");
}


void wheelfir_state::wheelfir_sub(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x20ffff).ram();

	map(0x780000, 0x780001).r("soundlatch", FUNC(generic_latch_16_device::read));

	map(0x700000, 0x700001).w("ldac", FUNC(dac_word_interface::data_w));
	map(0x740000, 0x740001).w("rdac", FUNC(dac_word_interface::data_w));
}


static INPUT_PORTS_START( pwball )
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
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME( "Test" )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_READ_LINE_DEVICE_MEMBER(DEVICE_SELF, wheelfir_state, adc_eoc_r)
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

	PORT_START("STEERING")
	PORT_START("ACCELERATOR")
	PORT_START("BRAKE")
INPUT_PORTS_END


static INPUT_PORTS_START( wheelfir )
	PORT_INCLUDE(pwball)

	PORT_MODIFY("STEERING")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel") PORT_REVERSE

	PORT_MODIFY("ACCELERATOR")
	PORT_BIT(0xff, 0x00, IPT_PEDAL)  PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Accelerator Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE

	PORT_MODIFY("BRAKE")
	PORT_BIT(0xff, 0x00, IPT_PEDAL2) PORT_INVERT PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00, 0xff) PORT_REVERSE
INPUT_PORTS_END

int wheelfir_state::adc_eoc_r()
{
	return m_adc_eoc;
}

void wheelfir_state::adc_eoc_w(int state)
{
	m_adc_eoc = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(wheelfir_state::scanline_timer_callback)
{
	if(param<NUM_SCANLINES)
	{
		//copy scanline offset
		int xscroll = (m_blitter_data[0xa] & 0x00ff) | (m_blitter_data[0x8] & 0x0040) << 2;
		int yscroll = (m_blitter_data[0xb] & 0x00ff) | (m_blitter_data[0x8] & 0x0080) << 1;

		m_scanlines[param].x = xscroll;
		m_scanlines[param].y = yscroll;
		m_scanlines[param].unkbits = m_blitter_data[0x8] & 0xff;

		m_blitter_data[0xb]++;

		//visible scanline
		m_scanline_cnt--;

		if ((m_scanline_cnt==0) && (!m_disable_raster_irq))
		{
			m_maincpu->set_input_line(5, HOLD_LINE); // raster IRQ, changes scroll values for road
		}
		//m_screen->update_partial(param);

		if ((param == 0) && m_is_pwball)
		{
			m_maincpu->set_input_line(1, HOLD_LINE);
		}
	}
	else
	{
		if(param==NUM_SCANLINES) /* vblank */
		{
			m_maincpu->set_input_line(3, HOLD_LINE);
		}
	}

	m_subcpu->set_input_line(1, HOLD_LINE);
}


void wheelfir_state::machine_start()
{
	m_zoom_table = std::make_unique<int32_t[]>(ZOOM_TABLE_SIZE);
	m_blitter_data = std::make_unique<uint16_t[]>(16);

	m_scanlines = std::make_unique<scroll_info[]>(NUM_SCANLINES+NUM_VBLANK_LINES);


	for(int i=0;i<(ZOOM_TABLE_SIZE);++i)
	{
		m_zoom_table[i]=-1;
	}

	uint16_t *ROM = (uint16_t *)memregion("maincpu")->base();

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

void wheelfir_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb888_w));
}


static const uint32_t texlayout_xoffset[512] = { STEP512(0,8) };
static const uint32_t texlayout_yoffset[512] = { STEP512(0,4096) };

static const gfx_layout wheelfir512x5125x8_texlayout =
{
	512, 512,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	512*512*8,
	texlayout_xoffset,
	texlayout_yoffset
};

static GFXDECODE_START( gfx_wheelfir )
	GFXDECODE_ENTRY( "gfx1", 0, wheelfir512x5125x8_texlayout, 0x0, 1 )
GFXDECODE_END

void wheelfir_state::wheelfir(machine_config &config)
{
	M68000(config, m_maincpu, 32000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &wheelfir_state::wheelfir_main);

	M68000(config, m_subcpu, 32000000/2);
	m_subcpu->set_addrmap(AS_PROGRAM, &wheelfir_state::wheelfir_sub);

	//config.set_maximum_quantum(attotime::from_hz(12000));

	adc0808_device &adc(ADC0808(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set(FUNC(wheelfir_state::adc_eoc_w));
	adc.in_callback<0>().set_ioport("STEERING");
	adc.in_callback<1>().set_ioport("ACCELERATOR");
	adc.in_callback<2>().set_ioport("BRAKE");

	TIMER(config, "scan_timer").configure_scanline(FUNC(wheelfir_state::scanline_timer_callback), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(336, NUM_SCANLINES+NUM_VBLANK_LINES);
	m_screen->set_visarea(0,335, 0, NUM_SCANLINES-1);
	m_screen->set_screen_update(FUNC(wheelfir_state::screen_update_wheelfir));
	m_screen->screen_vblank().set(FUNC(wheelfir_state::screen_vblank_wheelfir));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_wheelfir);

	PALETTE(config, m_palette).set_entries(NUM_COLORS);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &wheelfir_state::ramdac_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	GENERIC_LATCH_16(config, "soundlatch");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DAC_10BIT_R2R(config, "ldac", 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0); // unknown DAC
	DAC_10BIT_R2R(config, "rdac", 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0); // unknown DAC
}


void wheelfir_state::kongball(machine_config& config)
{
	wheelfir(config);
	m_subcpu->set_disable(); // sound ROMS were not present, so don't run sound CPU
}


ROM_START( wheelfir )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "tch1.u19", 0x00001, 0x80000, CRC(33bbbc67) SHA1(c2ecc0ab522ee442076ea7b9536aee6e1fad0540) )
	ROM_LOAD16_BYTE( "tch2.u21", 0x00000, 0x80000, CRC(ed6b9e8a) SHA1(214c5aaf55963a219db33dd5d530492e09ad5e07) )

	ROM_REGION( 0x100000, "subcpu", 0 ) /* 68000 Code + sound samples */
	ROM_LOAD16_BYTE( "tch3.u83",  0x00001, 0x80000, CRC(43c014a6) SHA1(6c01a08dda204f36e8768795dd5d405576a49140) )
	ROM_LOAD16_BYTE( "tch11.u65", 0x00000, 0x80000, CRC(fc894b2e) SHA1(ebe6d1adf889731fb6f53b4ce5f09c60e2aefb97) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "tch4.u52", 0x000000, 0x80000, CRC(fe4bc2c7) SHA1(33a2ef79cb13f9e7e7d513915c6e13c4e7fe0188) )
	ROM_LOAD( "tch5.u53", 0x080000, 0x80000, CRC(a38b9ca5) SHA1(083c9f700b9df1039fb553e918e205c6d32057ad) )
	ROM_LOAD( "tch6.u54", 0x100000, 0x80000, CRC(2733ae6b) SHA1(ebd91e123b670159f79be19a552d1ae0c8a0faff) )
	ROM_LOAD( "tch7.u55", 0x180000, 0x80000, CRC(6d98f27f) SHA1(d39f7f184abce645b9165b64e89e3b5354187eea) )
	ROM_LOAD( "tch8.u56", 0x200000, 0x80000, CRC(22b661fe) SHA1(b6edf8e1e8b479ee8813502157615f54627dc7c1) )
	ROM_LOAD( "tch9.u57", 0x280000, 0x80000, CRC(83c66de3) SHA1(50deaf3338d590340b928f891548c47ba8f3ca38) )
	ROM_LOAD( "tch10.u58",0x300000, 0x80000, CRC(2036ed80) SHA1(910381e2ccdbc2d06f873021d8af02795d22f595) )
	ROM_LOAD( "tch12.u59",0x380000, 0x80000, CRC(cce2e675) SHA1(f3d8916077b2e057169d0f254005cd959789a3b3) )

	ROM_REGION16_BE(0x80, "eeprom", 0)
	ROM_LOAD16_WORD_SWAP( "eeprom", 0x000000, 0x000080, CRC(961e4bc9) SHA1(8944504bf56a272e9aa08185e73c6b4212d52383) )
ROM_END


ROM_START( pwball )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "pball.u19", 0x00001, 0x80000, CRC(52f433ce) SHA1(f25c43188c320636d69fec2de2605303ef79a2f8) )
	ROM_LOAD16_BYTE( "pball.u21", 0x00000, 0x80000, CRC(c0250bc8) SHA1(c7ad2dd0e7fde337a638b09fd325c0cfe1b0b966) )

	ROM_REGION( 0x100000, "subcpu", 0 ) /* 68000 Code + sound samples */
	ROM_LOAD16_BYTE( "pball.u63", 0x00001, 0x80000, CRC(ab8bba31) SHA1(cc89d1c8b5998b712161dda4283856acd8d91723) )
	ROM_LOAD16_BYTE( "pball.u65", 0x00000, 0x80000, CRC(f2583796) SHA1(f044a575ae3dafaac8ad05aee6d4b672166af969) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "pball.u52", 0x000000, 0x80000, CRC(ad8567d3) SHA1(348a85388a5a98b3baa9484c947af6f878c4b70b) )
	ROM_LOAD( "pball.u53", 0x080000, 0x80000, CRC(9b954f59) SHA1(2cd9afea1677bb11e6d914c14e013da84e623356) )
	ROM_LOAD( "pball.u54", 0x100000, 0x80000, CRC(6979682d) SHA1(517abe80f4a44810427dd6da9187141ffd4a881c) )
	ROM_LOAD( "pball.u55", 0x180000, 0x80000, CRC(33593547) SHA1(9d13af4644e7fab01005609897c61acc50db8282) )
	ROM_LOAD( "pball.u56", 0x200000, 0x80000, CRC(bb64c0ed) SHA1(fa39121978412c02e7eb7adcf4d3b0b012904fee) )
	ROM_LOAD( "pball.u57", 0x280000, 0x80000, CRC(eb187063) SHA1(accd174cf5f582d6ed7ab3373bbe9e1bb1d1dbb8) )
	ROM_LOAD( "pball.u58", 0x300000, 0x80000, CRC(2cf2057d) SHA1(da075980892b43cac89d11c76d30b91c85c09af6) )
	ROM_LOAD( "pball.u59", 0x380000, 0x80000, CRC(e1c656f4) SHA1(9bc3713d4b5185b417f4d027ae97dace1feec065) )
	ROM_LOAD( "pball.u200",0x400000, 0x80000, CRC(26e68e24) SHA1(d88994a521ad14ac91e2c3cdc646ea7fb1ca8e72) )
	ROM_LOAD( "pball.u201",0x480000, 0x80000, CRC(bc8575fc) SHA1(0b1e237ee321f046f9b268e3515d07be206b4d76) )
	ROM_LOAD( "pball.u202",0x500000, 0x80000, CRC(97e2cf4f) SHA1(82ce199b0918762b0593b56bc8c19947993cfb59) )
	ROM_LOAD( "pball.u203",0x580000, 0x80000, CRC(89605b33) SHA1(6fb4ebacfd686266142317b3f864705d90ce97ca) )
	ROM_LOAD( "pball.u204",0x600000, 0x80000, CRC(efddd379) SHA1(903a931a7012f16df516b3cbec8b689a4fc0ba9c) )
	// u205 is empty (missing or unused?)
ROM_END

ROM_START( kongball )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "kong.u19", 0x00001, 0x80000, CRC(0283df0a) SHA1(559b547127728c78ba66191278bba4b4cce37eba) )
	ROM_LOAD16_BYTE( "kong.u21", 0x00000, 0x80000, CRC(ca6ad0da) SHA1(d5e07d7827587263b8d3028ab006dc0be454a7c7) )

	ROM_REGION( 0x100000, "subcpu", 0 ) /* 68000 Code + sound samples */
	// these were not present on the PCB (empty sockets) was sound never programmed, or were they removed at some point?
	ROM_LOAD16_BYTE( "kong.u63", 0x00001, 0x80000, NO_DUMP )
	ROM_LOAD16_BYTE( "kong.u65", 0x00000, 0x80000, NO_DUMP )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "kong.u52", 0x000000, 0x80000, CRC(52487271) SHA1(a15a4011b1b23f5b2af0d7b6d05b3df3ae1daebb) )
	ROM_LOAD( "kong.u53", 0x080000, 0x80000, CRC(aaaa3840) SHA1(0dcd2d70687d3c646198c40743b8b0a64cafced0) )
	ROM_LOAD( "kong.u54", 0x100000, 0x80000, CRC(cb7ef2c2) SHA1(293b694e201221a5b27b22a1b729db5d7e52bf99) )
	ROM_LOAD( "kong.u55", 0x180000, 0x80000, CRC(c9596c6f) SHA1(dd989d3e0c0be696331a8c42e03439fec8685bc8) )
	ROM_LOAD( "kong.u56", 0x200000, 0x80000, CRC(cbb69adc) SHA1(244d36e6913feb7edc606fdaa7a80f2a8a480efa) )
	ROM_LOAD( "kong.u57", 0x280000, 0x80000, CRC(f33af567) SHA1(8cbf9cc78b95edf33fb250bd232f032b4d7f4881) )
	ROM_LOAD( "kong.u58", 0x300000, 0x80000, CRC(17818ee8) SHA1(7e857f7c5c146cfcdaa9b1077c63bbee1d143438) )
	ROM_LOAD( "kong.u59", 0x380000, 0x80000, CRC(6d35e911) SHA1(d37bc2d364ced0d828598af9b2fe7abf6ffb4f82) )
	ROM_LOAD( "kong.u200",0x400000, 0x80000, CRC(ec552e97) SHA1(7a9a06ff77ef7fc51782caa4d4514864bf8099e7) )
ROM_END


ROM_START( radendur )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "endu.u19", 0x00001, 0x80000, CRC(d24ca61c) SHA1(64d2648959579d35fbf4c8479c0ff75781d28146) )
	ROM_LOAD16_BYTE( "endu.u21", 0x00000, 0x80000, CRC(45244675) SHA1(69be2eca3548644f97b9f6956bb3ff979740b1e4) )

	ROM_REGION( 0x100000, "subcpu", 0 ) /* 68000 Code + sound samples */
	ROM_LOAD16_BYTE( "endu.u63", 0x00001, 0x80000, CRC(43c35368) SHA1(62167a434bfe00825f2f97ecadd0af3e2d2b63f2) )
	ROM_LOAD16_BYTE( "endu.u65", 0x00000, 0x80000, CRC(25259894) SHA1(b260324760dba1595ab8bd53e1bbc20c04b640dd) )

	ROM_REGION( 0x1000000, "gfx1", ROMREGION_ERASE00  ) // each ROM contains 2 512x512 8bpp gfx pages
	ROM_LOAD( "endu.u52", 0x000000, 0x80000, CRC(2b686f81) SHA1(9d9aa0f6e1a3dc7e96f647ddda3214f1f4c6bf1d) )
	ROM_LOAD( "endu.u53", 0x080000, 0x80000, CRC(1a224c69) SHA1(ed1a8fdd97ea0caa649d3f1238fa13b1fd19376c) )
	ROM_LOAD( "endu.u54", 0x100000, 0x80000, CRC(217a9725) SHA1(14240e457967c5b23c54d4395fc1ab7394a59fc9) )
	ROM_LOAD( "endu.u55", 0x180000, 0x80000, CRC(bfce1b17) SHA1(415ab6815836dafb1a0b86464e84f3aaa5d1075d) )
	ROM_LOAD( "endu.u56", 0x200000, 0x80000, CRC(4b9460a8) SHA1(d2b9484f133d5d9abbe6a163c57d079944b3221f) )
	ROM_LOAD( "endu.u57", 0x280000, 0x80000, CRC(f17d5752) SHA1(354a76e9e130f10fc103ab83ea15bf1f47fca391) ) // mostly blank ROM (contains a single image)
	ROM_LOAD( "endu.u58", 0x300000, 0x80000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) ) // blank ROM (intentional)
	ROM_LOAD( "endu.u59", 0x380000, 0x80000, CRC(037648df) SHA1(19b0cbdb60ac4b14f06dd71ddf4e3b2d2dbdb19d) ) // FIXED BITS (111xxxxx) but correct, not all 8bpp used
	ROM_LOAD( "endu.u200",0x400000, 0x80000, CRC(e1417f94) SHA1(dba02ce1f9bbe18e78c67e770fe84184e227799b) )
	ROM_LOAD( "endu.u201",0x480000, 0x80000, CRC(25d9db1c) SHA1(c61491ae2dc97ea5c9f0f0d58f32edf0a3a94d95) )
	ROM_LOAD( "endu.u202",0x500000, 0x80000, CRC(15f4fbd4) SHA1(2ad28af1bd0924fc3f1f6e026ca834c7df774b64) )
	ROM_LOAD( "endu.u203",0x580000, 0x80000, CRC(e6361a27) SHA1(826be3b326e68c3b7c0f5c3d84993a971b32f84e) )
	ROM_LOAD( "endu.u204",0x600000, 0x80000, CRC(0db97872) SHA1(56e38a9034082c3ceb78c0f3aba6125ce6fabd49) )
	ROM_LOAD( "endu.u205",0x680000, 0x80000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) ) // blank ROM (intentional)
ROM_END

} // anonymous namespace

void wheelfir_state::init_pwball()
{
	// temp hack as pwball doesn't like zooming and other things (maybe different FPGA programming?)
	m_is_pwball = true;
}

void wheelfir_state::init_kongball()
{
	init_pwball();
	m_disable_raster_irq = true; // the raster interrupt points outside of code
}


GAME( 199?, wheelfir,    0, wheelfir,    wheelfir, wheelfir_state, empty_init,    ROT0,  "TCH", "Wheels & Fire", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 199?, pwball,      0, wheelfir,    pwball,   wheelfir_state, init_pwball,   ROT0,  "TCH", "Power Ball (prototype)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // mostly complete

// these might not be fully functional games
GAME( 199?, kongball,    0, kongball,    pwball,   wheelfir_state, init_kongball, ROT0,  "TCH", "Kong Ball (early prototype)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // can't get ingame?
GAME( 199?, radendur,    0, wheelfir,    pwball,   wheelfir_state, init_kongball, ROT0,  "TCH / Sator Videogames", "Radical Enduro (early prototype)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // can get ingame if you overclock CPUs significantly (IRQ problems?)
