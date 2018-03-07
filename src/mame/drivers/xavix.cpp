// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/***************************************************************************

    Preliminary driver for XaviX TV PNP console and childs (Let's! Play TV Classic)

    CPU is an M6502 derivative with added opcodes for far-call handling

    Notes from http://www.videogameconsolelibrary.com/pg00-xavix.htm#page=reviews (thanks Guru!)
    (** this isn't entirely accurate, XaviX Tennis appears to be Super Xavix, see other notes in driver)

    XaviXPORT arrived on the scene with 3 game titles (XaviX Tennis, XaviX Bowling and XaviX Baseball) using their
    original XaviX Multiprocessor.  This proprietary chip is reported to contain an 8-bit high speed central processing
    unit (6502) at 21 MHz, picture processor, sound processor, DMA controller, 1K bytes high speed RAM, universal timer,
    AD/Converter and I/O device control.  Each cartridge comes with a wireless peripheral to be used with the game (Baseball Bat,
    Tennis Racquet, etc.) that requires "AA" batteries.  The XaviXPORT system retailed for $79.99 USD with the cartridges
    retailing for $49.99 USD.

    The following year at CES 2005, SSD COMPANY LIMITED introduced two new XaviXPORT titles (XaviX Golf and XaviX Bass Fishing) each
    containing the upgraded "Super XaviX".  This new chip is said to sport a 16-bit high central processing unit (65816) at 43 MHz.
    SSD COMPANY LIMITED is already working on their next chip called "XaviX II" that is said to be a 32-bit RISC processor
    with 3D capabilities.

    Notes:
	
	To access service mode in Monster Truck hold Horn and Nitro on startup

	There are multiple revisions of the CPU hardware, the SSD 2000 / SSD 2002 chips definitely add more opcodes
	(thanks to Sean Riddle for this table)

	name						PCB ID		ROM width	TSOP pads	ROM size		SEEPROM			die markings

	Play TV Ping Pong			8028		x8			48			1M				no				SSD 97 PA7270-107

	Play TV Bass Fishin'		71008		x8			40			1M				no				SSD 98 PA7351-107
	Play TV Boxing				72039		x8			48			2M				no				SSD 98 PA7351-107
	Play TV Card Night			71063		x8			40			1M				no				SSD 98 PA7351-107

	Play TV Baseball 2			72042		x8			48			2M				no				SSD 98 PL7351-181
	Play TV Monster Truck		74026		x8			48			4M				no				SSD 98 PL7351-181
	Radica/EA Madden Football	74021		x8			48			4M      		no				SSD 98 PL7351-181
	Play TV Snowboarder Blue	71023		x8			40			1M				no				SSD 98 PL7351-181
	Namco Nostalgia 2			CGSJ		x8			48			1M				24LC04			SSD 98 PL7351-181

	Star Wars Saga Lightsaber	SWSA		x8			48			8M				24C02			SSD 2000 NEC 85605-621
	Lord of the Rings			LORA		x8			48			8M				24C02			SSD 2000 NEC 85605-621
	MX Rebel Dirt				MTXA		x8			48			8M				24C04			SSD 2000 NEC 85605-621

	XaviXTennis					SGM6446		x16			48			8M				24C08			SSD 2002 NEC 85054-611

	Real Swing Golf				74037		x16			48			not dumped		
	Play TV Football 2			L7278		x16			48			not dumped		
	XaviXBowling				SGM644C		x16			48			not dumped		
	Basketball					75029		x16			48			not dumped		

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/xavix.h"
#include "machine/timer.h"
#include "screen.h"
#include "speaker.h"

// not confirmed for all games
#define MAIN_CLOCK XTAL(21'477'272)

class xavix_state : public driver_device
{
public:
	xavix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_spr_attr0(*this, "spr_attr0"),
		m_spr_attr1(*this, "spr_attr1"),
		m_spr_ypos(*this, "spr_ypos"),
		m_spr_xpos(*this, "spr_xpos"),
		m_spr_addr_lo(*this, "spr_addr_lo"),
		m_spr_addr_md(*this, "spr_addr_md"),
		m_spr_addr_hi(*this, "spr_addr_hi"),
		m_palram1(*this, "palram1"),
		m_palram2(*this, "palram2"),
		m_spr_attra(*this, "spr_attra"),
		m_palette(*this, "palette"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_region(*this, "REGION"),
		m_gfxdecode(*this, "gfxdecode"),
		m_alt_addressing(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xavix(machine_config &config);
	void xavixp(machine_config &config);

	DECLARE_WRITE8_MEMBER(xavix_7900_w);

	DECLARE_WRITE8_MEMBER(dma_trigger_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_md_w);
	DECLARE_WRITE8_MEMBER(rom_dmasrc_hi_w);
	DECLARE_WRITE8_MEMBER(rom_dmadst_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmadst_hi_w);
	DECLARE_WRITE8_MEMBER(rom_dmalen_lo_w);
	DECLARE_WRITE8_MEMBER(rom_dmalen_hi_w);
	DECLARE_READ8_MEMBER(dma_trigger_r);

	DECLARE_WRITE8_MEMBER(vid_dma_params_1_w);
	DECLARE_WRITE8_MEMBER(vid_dma_params_2_w);
	DECLARE_WRITE8_MEMBER(vid_dma_trigger_w);
	DECLARE_READ8_MEMBER(vid_dma_trigger_r);

	DECLARE_READ8_MEMBER(xavix_io_0_r);
	DECLARE_READ8_MEMBER(xavix_io_1_r);

	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(irq_vector0_lo_w);
	DECLARE_WRITE8_MEMBER(irq_vector0_hi_w);
	DECLARE_WRITE8_MEMBER(irq_vector1_lo_w);
	DECLARE_WRITE8_MEMBER(irq_vector1_hi_w);

	DECLARE_READ8_MEMBER(irq_source_r);
	DECLARE_WRITE8_MEMBER(irq_source_w);

	DECLARE_READ8_MEMBER(xavix_6ff0_r);
	DECLARE_WRITE8_MEMBER(xavix_6ff0_w);

	DECLARE_READ8_MEMBER(xavix_6ff8_r);
	DECLARE_WRITE8_MEMBER(xavix_6ff8_w);

	DECLARE_READ8_MEMBER(xavix_75f0_r);
	DECLARE_WRITE8_MEMBER(xavix_75f0_w);

	DECLARE_READ8_MEMBER(xavix_75f1_r);
	DECLARE_WRITE8_MEMBER(xavix_75f1_w);

	DECLARE_READ8_MEMBER(xavix_75f4_r);
	DECLARE_READ8_MEMBER(xavix_75f5_r);
	DECLARE_READ8_MEMBER(xavix_75f6_r);
	DECLARE_WRITE8_MEMBER(xavix_75f6_w);

	DECLARE_WRITE8_MEMBER(xavix_75f7_w);

	DECLARE_READ8_MEMBER(xavix_75f8_r);
	DECLARE_WRITE8_MEMBER(xavix_75f8_w);

	DECLARE_READ8_MEMBER(xavix_75f9_r);
	DECLARE_WRITE8_MEMBER(xavix_75f9_w);

	DECLARE_READ8_MEMBER(xavix_75fa_r);
	DECLARE_WRITE8_MEMBER(xavix_75fa_w);
	DECLARE_READ8_MEMBER(xavix_75fb_r);
	DECLARE_WRITE8_MEMBER(xavix_75fb_w);
	DECLARE_READ8_MEMBER(xavix_75fc_r);
	DECLARE_WRITE8_MEMBER(xavix_75fc_w);
	DECLARE_READ8_MEMBER(xavix_75fd_r);
	DECLARE_WRITE8_MEMBER(xavix_75fd_w);

	DECLARE_WRITE8_MEMBER(xavix_75fe_w);
	DECLARE_WRITE8_MEMBER(xavix_75ff_w);

	DECLARE_WRITE8_MEMBER(xavix_6fc0_w);
	DECLARE_WRITE8_MEMBER(tmap1_regs_w);
	DECLARE_WRITE8_MEMBER(xavix_6fd8_w);
	DECLARE_WRITE8_MEMBER(tmap2_regs_w);
	DECLARE_READ8_MEMBER(tmap2_regs_r);

	DECLARE_READ8_MEMBER(pal_ntsc_r);

	DECLARE_READ8_MEMBER(xavix_4000_r);

	DECLARE_READ8_MEMBER(mult_r);
	DECLARE_WRITE8_MEMBER(mult_param_w);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	DECLARE_DRIVER_INIT(xavix);
	DECLARE_DRIVER_INIT(taitons1);
	DECLARE_DRIVER_INIT(rad_box);

	void xavix_map(address_map &map);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	uint8_t m_rom_dmasrc_lo_data;
	uint8_t m_rom_dmasrc_md_data;
	uint8_t m_rom_dmasrc_hi_data;

	uint8_t m_rom_dmadst_lo_data;
	uint8_t m_rom_dmadst_hi_data;

	uint8_t m_rom_dmalen_lo_data;
	uint8_t m_rom_dmalen_hi_data;

	uint8_t m_irq_enable_data;
	uint8_t m_irq_vector0_lo_data;
	uint8_t m_irq_vector0_hi_data;
	uint8_t m_irq_vector1_lo_data;
	uint8_t m_irq_vector1_hi_data;

	uint8_t m_multparams[3];
	uint8_t m_multresults[2];

	uint8_t m_vid_dma_param1[2];
	uint8_t m_vid_dma_param2[2];

	uint8_t m_tmap1_regs[8];
	uint8_t m_tmap2_regs[8];

	uint8_t m_6ff0;
	uint8_t m_6ff8;

	uint8_t m_75f0;
	uint8_t m_75f1;

	uint8_t m_75f6;
	uint8_t m_75f8;
	uint8_t m_75fa;
	uint8_t m_75fb;
	uint8_t m_75fc;
	uint8_t m_75fd;

	uint8_t get_vectors(int which, int half);

	required_shared_ptr<uint8_t> m_mainram;

	required_shared_ptr<uint8_t> m_spr_attr0;
	required_shared_ptr<uint8_t> m_spr_attr1;
	required_shared_ptr<uint8_t> m_spr_ypos;
	required_shared_ptr<uint8_t> m_spr_xpos;

	required_shared_ptr<uint8_t> m_spr_addr_lo;
	required_shared_ptr<uint8_t> m_spr_addr_md;
	required_shared_ptr<uint8_t> m_spr_addr_hi;

	required_shared_ptr<uint8_t> m_palram1;
	required_shared_ptr<uint8_t> m_palram2;

	required_shared_ptr<uint8_t> m_spr_attra;

	required_device<palette_device> m_palette;

	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_region;

	required_device<gfxdecode_device> m_gfxdecode;

	void handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	double hue2rgb(double p, double q, double t);
	void draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int opaque);
	void draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int m_rgnlen;
	uint8_t* m_rgn;

	// variables used by rendering
	int m_tmp_dataaddress;
	int m_tmp_databit;
	void set_data_address(int address, int bit);
	uint8_t get_next_bit();
	uint8_t get_next_byte();

	int get_current_address_byte();

	int m_alt_addressing;
};

void xavix_state::set_data_address(int address, int bit)
{
	m_tmp_dataaddress = address;
	m_tmp_databit = bit;
}

uint8_t xavix_state::get_next_bit()
{
	uint8_t bit = m_rgn[m_tmp_dataaddress & (m_rgnlen - 1)];
	bit = bit >> m_tmp_databit;
	bit &= 1;

	m_tmp_databit++;

	if (m_tmp_databit == 8)
	{
		m_tmp_databit = 0;
		m_tmp_dataaddress++;
	}

	return bit;
}

uint8_t xavix_state::get_next_byte()
{
	uint8_t dat = 0;
	for (int i = 0; i < 8; i++)
	{
		dat |= (get_next_bit() << i);
	}
	return dat;
}

int xavix_state::get_current_address_byte()
{
	return m_tmp_dataaddress;
}


void xavix_state::video_start()
{
}


double xavix_state::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}


void xavix_state::handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// not verified
	int offs = 0;
	for (int index = 0; index < 256; index++)
	{
		uint16_t dat = m_palram1[offs];
		dat |= m_palram2[offs]<<8;
		offs++;

		int l_raw = (dat & 0x1f00) >> 8;
		int sl_raw =(dat & 0x00e0) >> 5;
		int h_raw = (dat & 0x001f) >> 0;

		//if (h_raw > 24)
		//  logerror("hraw >24 (%02x)\n", h_raw);

		//if (l_raw > 17)
		//  logerror("lraw >17 (%02x)\n", l_raw);

		//if (sl_raw > 7)
		//  logerror("sl_raw >5 (%02x)\n", sl_raw);

		double l = (double)l_raw / 17.0f;
		double s = (double)sl_raw / 7.0f;
		double h = (double)h_raw / 24.0f;

		double r, g, b;

		if (s == 0) {
			r = g = b = l; // greyscale
		}
		else {
			double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
			double p = 2 * l - q;
			r = hue2rgb(p, q, h + 1 / 3.0f);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1 / 3.0f);
		}

		int r_real = r * 255.0f;
		int g_real = g * 255.0f;
		int b_real = b * 255.0f;

		m_palette->set_pen_color(index, r_real, g_real, b_real);

	}
}

void xavix_state::draw_tilemap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which)
{
	int alt_tileaddressing = 0;
	int alt_tileaddressing2 = 0;

	int ydimension = 0;
	int xdimension = 0;
	int ytilesize = 0;
	int xtilesize = 0;
	int offset_multiplier = 0;
	int opaque = 0;

	uint8_t* tileregs;
	address_space& mainspace = m_maincpu->space(AS_PROGRAM);

	if (which == 0)
	{
		tileregs = m_tmap1_regs;
		opaque = 1;
	}
	else
	{
		tileregs = m_tmap2_regs;
		opaque = 0;
	}

	switch (tileregs[0x3] & 0x30)
	{
	case 0x00:
		ydimension = 32;
		xdimension = 32;
		ytilesize = 8;
		xtilesize = 8;
		break;

	case 0x10:
		ydimension = 32;
		xdimension = 16;
		ytilesize = 8;
		xtilesize = 16;
		break;

	case 0x20: // guess
		ydimension = 16;
		xdimension = 32;
		ytilesize = 16;
		xtilesize = 8;
		break;

	case 0x30:
		ydimension = 16;
		xdimension = 16;
		ytilesize = 16;
		xtilesize = 16;
		break;
	}

	offset_multiplier = (ytilesize * xtilesize)/8;

	if (tileregs[0x7] & 0x10)
		alt_tileaddressing = 1;
	else
		alt_tileaddressing = 0;

	if (tileregs[0x7] & 0x02)
		alt_tileaddressing2 = 1;

	static int hackx = 1;

	if (machine().input().code_pressed_once(KEYCODE_Q))
	{
		hackx--;
		printf("%02x\n", hackx);
	}

	if (machine().input().code_pressed_once(KEYCODE_W))
	{
		hackx++;
		printf("%02x\n", hackx);
	}

	if (tileregs[0x7] & 0x80)
	{
		//printf("draw tilemap %d, regs base0 %02x base1 %02x base2 %02x tilesize,bpp %02x scrollx %02x scrolly %02x pal %02x mode %02x\n", which, tileregs[0x0], tileregs[0x1], tileregs[0x2], tileregs[0x3], tileregs[0x4], tileregs[0x5], tileregs[0x6], tileregs[0x7]);

		// there's a tilemap register to specify base in main ram, although in the monster truck test mode it points to an unmapped region
		// and expected a fixed layout, we handle that in the memory map at the moment
		int count;

		count = 0;// ;
		for (int y = 0; y < ydimension; y++)
		{
			for (int x = 0; x < xdimension; x++)
			{
				int bpp, pal, scrolly, scrollx;
				int tile = 0;
				int extraattr = 0;

				// the register being 0 probably isn't the condition here
				if (tileregs[0x0] != 0x00) tile |= mainspace.read_byte((tileregs[0x0] << 8) + count);
				if (tileregs[0x1] != 0x00) tile |= mainspace.read_byte((tileregs[0x1] << 8) + count) << 8;

				// at the very least boxing leaves unwanted bits set in ram after changing between mode 0x8a and 0x82, expecting them to not be used
				if (tileregs[0x7] & 0x08)
					extraattr = mainspace.read_byte((tileregs[0x2] << 8) + count);

				count++;

				bpp = (tileregs[0x3] & 0x0e) >> 1;
				bpp++;
				pal = (tileregs[0x6] & 0xf0) >> 4;
				scrolly = tileregs[0x5];
				scrollx = tileregs[0x4];

				int basereg;
				int flipx = 0;
				int flipy = 0;
				int gfxbase;

				// tile 0 is always skipped, doesn't even point to valid data packets in alt mode
				// this is consistent with the top layer too
				// should we draw as solid in solid layer?
				if (tile == 0)
					continue;

				const int debug_packets = 0;
				int test = 0;

				if (!alt_tileaddressing)
				{
					if (!alt_tileaddressing2)
					{
						basereg = 0;
						gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);

						tile = tile * (offset_multiplier * bpp);
						tile += gfxbase;
					}
					else
					{
						tile = tile * 8;
						basereg = (tile & 0x70000) >> 16;
						tile &= 0xffff;
						gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);
						tile += gfxbase;

						if (tileregs[0x7] & 0x08)
						{
							// make use of the extraattr stuff?
							pal = (extraattr & 0xf0)>>4;
							// low bits are priority?
						}
					}
				}
				else
				{
					if (debug_packets) printf("for tile %04x (at %d %d): ", tile, (((x * 16) + scrollx) & 0xff), (((y * 16) + scrolly) & 0xff));


					basereg = (tile & 0xf000) >> 12;
					tile &= 0x0fff;
					gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);

					tile += gfxbase;
					set_data_address(tile, 0);

					// there seems to be a packet stored before the tile?!
					// the offset used for flipped sprites seems to specifically be changed so that it picks up an extra byte which presumably triggers the flipping
					uint8_t byte1 = 0;
					int done = 0;
					int skip = 0;

					do
					{
						byte1 = get_next_byte();

						if (debug_packets) printf(" %02x, ", byte1);

						if (skip == 1)
						{
							skip = 0;
							//test = 1;
						}
						else if ((byte1 & 0x0f) == 0x01)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x03)
						{
							// causes next byte to be skipped??
							skip = 1;
						}
						else if ((byte1 & 0x0f) == 0x05)
						{
							// the upper bits are often 0x00, 0x10, 0x20, 0x30, why?
							flipx = 1;
						}
						else if ((byte1 & 0x0f) == 0x06) // there must be other finish conditions too because sometimes this fails..
						{
							// tile data will follow after this, always?
							pal = (byte1 & 0xf0) >> 4;
							done = 1;
						}
						else if ((byte1 & 0x0f) == 0x07)
						{
							// causes next byte to be skipped??
							skip = 1;
						}
						else if ((byte1 & 0x0f) == 0x09)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0a)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0b)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0c)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0d)
						{
							// used
						}
						else if ((byte1 & 0x0f) == 0x0e)
						{
							// not seen
						}
						else if ((byte1 & 0x0f) == 0x0f)
						{
							// used
						}

					} while (done == 0);
					if (debug_packets) printf("\n");
					tile = get_current_address_byte();
				}

				if (test == 1) pal = machine().rand() & 0xf;


				draw_tile(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, ((y * ytilesize) - 16) - scrolly, ytilesize, xtilesize, flipx, flipy, pal, opaque);
				draw_tile(screen, bitmap, cliprect, tile, bpp, (x * xtilesize) + scrollx, (((y * ytilesize) - 16) - scrolly) + 256, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-y
				draw_tile(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, ((y * ytilesize) - 16) - scrolly, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-x
				draw_tile(screen, bitmap, cliprect, tile, bpp, ((x * xtilesize) + scrollx) - 256, (((y * ytilesize) - 16) - scrolly) + 256, ytilesize, xtilesize, flipx, flipy, pal, opaque); // wrap-y and x
			}
		}
	}
}

void xavix_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//logerror("frame\n");
	// priority doesn't seem to be based on list order, there are bad sprite-sprite priorities with either forward or reverse

	for (int i = 0xff; i >= 0; i--)
	{
		/* attribute 0 bits
		   pppp bbb-    p = palette, b = bpp

		   attribute 1 bits
		   ---- ss-f    s = size, f = flipx
		*/

		int ypos = m_spr_ypos[i];
		int xpos = m_spr_xpos[i];
		int tile = (m_spr_addr_hi[i] << 16) | (m_spr_addr_md[i] << 8) | m_spr_addr_lo[i];
		int attr0 = m_spr_attr0[i];
		int attr1 = m_spr_attr1[i];

		int pal = (attr0 & 0xf0) >> 4;
		int flipx = (attr1 & 0x01);

		int drawheight = 16;
		int drawwidth = 16;

		tile &= (m_rgnlen - 1);

		// taito nostalgia 1 also seems to use a different addressing, is it selectable or is the chip different?
		// taito nost attr1 is 84 / 80 / 88 / 8c for the various elements of the xavix logo.  monster truck uses ec / fc / dc / 4c / 5c / 6c (final 6 sprites ingame are 00 00 f0 f0 f0 f0, radar?)

		if ((attr1 & 0x0c) == 0x0c)
		{
			drawheight = 16;
			drawwidth = 16;
			if (m_alt_addressing == 1)
				tile = tile * 128;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x08)
		{
			drawheight = 16;
			drawwidth = 8;
			xpos += 4;
			if (m_alt_addressing == 1)
				tile = tile * 64;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x04)
		{
			drawheight = 8;
			drawwidth = 16;
			ypos -= 4;
			if (m_alt_addressing == 1)
				tile = tile * 64;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}
		else if ((attr1 & 0x0c) == 0x00)
		{
			drawheight = 8;
			drawwidth = 8;
			xpos += 4;
			ypos -= 4;
			if (m_alt_addressing == 1)
				tile = tile * 32;
			else if (m_alt_addressing == 2)
				tile = tile * 8;
		}

		if (m_alt_addressing != 0)
		{
			int basereg = (tile & 0xf0000) >> 16;
			tile &= 0xffff;
			int gfxbase = (m_spr_attra[(basereg * 2) + 1] << 16) | (m_spr_attra[(basereg * 2)] << 8);
			tile+= gfxbase;
		}

		ypos = 0x100 - ypos;

		xpos -= 136;
		ypos -= 152;

		xpos &= 0xff;
		ypos &= 0xff;

		if (ypos >= 192)
			ypos -= 256;

		int bpp = 1;

		bpp = (attr0 & 0x0e) >> 1;
		bpp += 1;

		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos, ypos, drawheight, drawwidth, flipx, 0, pal, 0);
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos - 256, ypos, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-x
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos, ypos - 256, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-y
		draw_tile(screen, bitmap, cliprect, tile, bpp, xpos - 256, ypos - 256, drawheight, drawwidth, flipx, 0, pal, 0); // wrap-x,y

		/*
		if ((m_spr_ypos[i] != 0x81) && (m_spr_ypos[i] != 0x80) && (m_spr_ypos[i] != 0x00))
		{
		    logerror("sprite with enable? %02x attr0 %02x attr1 %02x attr3 %02x attr5 %02x attr6 %02x attr7 %02x\n", m_spr_ypos[i], m_spr_attr0[i], m_spr_attr1[i], m_spr_xpos[i], m_spr_addr_lo[i], m_spr_addr_md[i], m_spr_addr_hi[i] );
		}
		*/
	}
}

void xavix_state::draw_tile(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tile, int bpp, int xpos, int ypos, int drawheight, int drawwidth, int flipx, int flipy, int pal, int opaque)
{
	// set the address here so we can increment in bits in the draw function
	set_data_address(tile, 0);

	for (int y = 0; y < drawheight; y++)
	{
		int row;
		if (flipy)
		{
			row = ypos + (drawheight-1) - y;
		}
		else
		{
			row = ypos + y;
		}

		for (int x = 0; x < drawwidth; x++)
		{

			int col;

			if (flipx)
			{
				col = xpos + (drawwidth-1) - x;
			}
			else
			{
				col = xpos + x;
			}

			uint8_t dat = 0;

			for (int i = 0; i < bpp; i++)
			{
				dat |= (get_next_bit() << i);
			}

			if ((row >= cliprect.min_y && row <= cliprect.max_y) && (col >= cliprect.min_x && col <= cliprect.max_x))
			{
				uint16_t* rowptr;

				rowptr = &bitmap.pix16(row);

				if (opaque)
				{
					rowptr[col] = (dat + (pal << 4)) & 0xff;
				}
				else
				{
					if (dat)
						rowptr[col] = (dat + (pal << 4)) & 0xff;
				}
			}
		}
	}
}

uint32_t xavix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_palette(screen, bitmap, cliprect);

	bitmap.fill(0, cliprect);

	draw_tilemap(screen,bitmap,cliprect,0);
	draw_sprites(screen,bitmap,cliprect);
	draw_tilemap(screen,bitmap,cliprect,1);

	return 0;
}


WRITE8_MEMBER(xavix_state::dma_trigger_w)
{
	if (data & 0x01) // namcons2 writes 0x81, most of the time things write 0x01
	{
		logerror("%s: dma_trigger_w (do DMA?) %02x\n", machine().describe_context(), data);

		uint32_t source = (m_rom_dmasrc_hi_data << 16) | (m_rom_dmasrc_md_data << 8) | m_rom_dmasrc_lo_data;
		uint16_t dest = (m_rom_dmadst_hi_data << 8) | m_rom_dmadst_lo_data;
		uint16_t len = (m_rom_dmalen_hi_data << 8) | m_rom_dmalen_lo_data;

		source &= m_rgnlen - 1;
		logerror("  (possible DMA op SRC %08x DST %04x LEN %04x)\n", source, dest, len);

		address_space& destspace = m_maincpu->space(AS_PROGRAM);

		for (int i = 0; i < len; i++)
		{
			uint8_t dat = m_rgn[(source + i) & (m_rgnlen - 1)];
			destspace.write_byte(dest + i, dat);
		}
	}
	else // the interrupt routine writes 0x80 to the trigger, maybe 'clear IRQ?'
	{
		logerror("%s: dma_trigger_w (unknown) %02x\n", machine().describe_context(), data);
	}
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_lo_w)
{
	logerror("%s: rom_dmasrc_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_md_w)
{
	logerror("%s: rom_dmasrc_md_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_md_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmasrc_hi_w)
{
	logerror("%s: rom_dmasrc_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmasrc_hi_data = data;
	// this would mean Taito Nostalgia relies on mirroring tho, as it has the high bits set... so could just be wrong
	logerror("  (DMA ROM source of %02x%02x%02x)\n", m_rom_dmasrc_hi_data, m_rom_dmasrc_md_data, m_rom_dmasrc_lo_data);
}

WRITE8_MEMBER(xavix_state::rom_dmadst_lo_w)
{
	logerror("%s: rom_dmadst_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmadst_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmadst_hi_w)
{
	logerror("%s: rom_dmadst_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmadst_hi_data = data;

	logerror("  (DMA dest of %02x%02x)\n", m_rom_dmadst_hi_data, m_rom_dmadst_lo_data);
}

WRITE8_MEMBER(xavix_state::rom_dmalen_lo_w)
{
	logerror("%s: rom_dmalen_lo_w %02x\n", machine().describe_context(), data);
	m_rom_dmalen_lo_data = data;
}

WRITE8_MEMBER(xavix_state::rom_dmalen_hi_w)
{
	logerror("%s: rom_dmalen_hi_w %02x\n", machine().describe_context(), data);
	m_rom_dmalen_hi_data = data;

	logerror("  (DMA len of %02x%02x)\n", m_rom_dmalen_hi_data, m_rom_dmalen_lo_data);
}

READ8_MEMBER(xavix_state::dma_trigger_r)
{
	logerror("%s: dma_trigger_r (operation status?)\n", machine().describe_context());
	return 0x00;
}



WRITE8_MEMBER(xavix_state::irq_enable_w)
{
	logerror("%s: irq_enable_w %02x\n", machine().describe_context(), data);
	m_irq_enable_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector0_lo_w)
{
	logerror("%s: irq_vector0_lo_w %02x\n", machine().describe_context(), data);
	m_irq_vector0_lo_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector0_hi_w)
{
	logerror("%s: irq_vector0_hi_w %02x\n", machine().describe_context(), data);
	m_irq_vector0_hi_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector1_lo_w)
{
	logerror("%s: irq_vector1_lo_w %02x\n", machine().describe_context(), data);
	m_irq_vector1_lo_data = data;
}

WRITE8_MEMBER(xavix_state::irq_vector1_hi_w)
{
	logerror("%s: irq_vector1_hi_w %02x\n", machine().describe_context(), data);
	m_irq_vector1_hi_data = data;
}


WRITE8_MEMBER(xavix_state::xavix_7900_w)
{
	logerror("%s: xavix_7900_w %02x (---FIRST WRITE ON STARTUP---)\n", machine().describe_context(), data);
}



TIMER_DEVICE_CALLBACK_MEMBER(xavix_state::scanline_cb)
{
/*
    int scanline = param;

    if (scanline == 200)
    {
        if (m_irq_enable_data != 0)
            m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
    }
*/
}

INTERRUPT_GEN_MEMBER(xavix_state::interrupt)
{
	//  if (m_irq_enable_data != 0)
	//      m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);

	// this logic is clearly VERY wrong

	if (m_irq_enable_data != 0)
	{
		if (m_6ff8)
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}



READ8_MEMBER(xavix_state::xavix_6ff0_r)
{
	//logerror("%s: xavix_6ff0_r\n", machine().describe_context());
	return m_6ff0;
}

WRITE8_MEMBER(xavix_state::xavix_6ff0_w)
{
	// expected to return data written
	m_6ff0 = data;
	//logerror("%s: xavix_6ff0_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_6ff8_r)
{
	//logerror("%s: xavix_6ff8_r\n", machine().describe_context());
	return m_6ff8;
}

WRITE8_MEMBER(xavix_state::xavix_6ff8_w)
{
	// I think this is something to do with IRQ ack / enable
	m_6ff8 = data;
	logerror("%s: xavix_6ff8_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75f0_r)
{
	logerror("%s: xavix_75f0_r\n", machine().describe_context());
	return m_75f0;
}

WRITE8_MEMBER(xavix_state::xavix_75f0_w)
{
	// expected to return data written
	m_75f0 = data;
	logerror("%s: xavix_75f0_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75f1_r)
{
	logerror("%s: xavix_75f1_r\n", machine().describe_context());
	return m_75f1;
}

WRITE8_MEMBER(xavix_state::xavix_75f1_w)
{
	// expected to return data written
	m_75f1 = data;
	logerror("%s: xavix_75f1_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75f6_r)
{
	logerror("%s: xavix_75f6_w\n", machine().describe_context());
	return m_75f6;
}

WRITE8_MEMBER(xavix_state::xavix_75f6_w)
{
	// expected to return data written
	m_75f6 = data;
	logerror("%s: xavix_75f6_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75fa_r)
{
	logerror("%s: xavix_75fa_w\n", machine().describe_context());
	return m_75fa;
}

WRITE8_MEMBER(xavix_state::xavix_75fa_w)
{
	// expected to return data written
	m_75fa = data;
	logerror("%s: xavix_75fa_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75fb_r)
{
	logerror("%s: xavix_75fb_w\n", machine().describe_context());
	return m_75fb;
}

WRITE8_MEMBER(xavix_state::xavix_75fb_w)
{
	// expected to return data written
	m_75fb = data;
	logerror("%s: xavix_75fb_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75fc_r)
{
	logerror("%s: xavix_75fc_w\n", machine().describe_context());
	return m_75fc;
}

WRITE8_MEMBER(xavix_state::xavix_75fc_w)
{
	// expected to return data written
	m_75fc = data;
	logerror("%s: xavix_75fc_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75fd_r)
{
	logerror("%s: xavix_75fd_w\n", machine().describe_context());
	return m_75fd;
}

WRITE8_MEMBER(xavix_state::xavix_75fd_w)
{
	// expected to return data written
	m_75fd = data;
	logerror("%s: xavix_75fd_w %02x\n", machine().describe_context(), data);
}





WRITE8_MEMBER(xavix_state::xavix_75f7_w)
{
	logerror("%s: xavix_75f7_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75f8_r)
{
	logerror("%s: xavix_75f8_r\n", machine().describe_context());
	return m_75f8;
}

WRITE8_MEMBER(xavix_state::xavix_75f8_w)
{
	// expected to return data written
	m_75f8 = data;
	logerror("%s: xavix_75f8_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_75f9_w)
{
	logerror("%s: xavix_75f9_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_75fe_w)
{
	logerror("%s: xavix_75fe_w %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_75ff_w)
{
	logerror("%s: xavix_75ff_w %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_75f9_r)
{
	logerror("%s: xavix_75f9_r\n", machine().describe_context());
	return 0x00;
}

READ8_MEMBER(xavix_state::xavix_io_0_r)
{
	return m_in0->read();
}

READ8_MEMBER(xavix_state::xavix_io_1_r)
{
	return m_in1->read();
}

READ8_MEMBER(xavix_state::xavix_75f4_r)
{
	// used with 75f0
	return 0xff;
}

READ8_MEMBER(xavix_state::xavix_75f5_r)
{
	// used with 75f1
	return 0xff;
}

READ8_MEMBER(xavix_state::mult_r)
{
	return m_multresults[offset];
}

WRITE8_MEMBER(xavix_state::mult_param_w)
{
	COMBINE_DATA(&m_multparams[offset]);
	// there are NOPs after one of the writes, so presumably the operation is write triggerd and not intstant
	// see test code at 0184a4 in monster truck

	if (offset == 2)
	{
		// assume 0 is upper bits, might be 'mode' instead, check
		uint16_t param1 = (m_multparams[0]<<8) | (m_multparams[1]);
		uint8_t param2 = (m_multparams[2]);

		uint16_t result =  param1*param2;

		m_multresults[1] = (result>>8)&0xff;
		m_multresults[0] = result&0xff;
	}
}

WRITE8_MEMBER(xavix_state::vid_dma_params_1_w)
{
	m_vid_dma_param1[offset] = data;
}

WRITE8_MEMBER(xavix_state::vid_dma_params_2_w)
{
	m_vid_dma_param2[offset] = data;
}

WRITE8_MEMBER(xavix_state::vid_dma_trigger_w)
{
	uint16_t len = data & 0x07;
	uint16_t src = (m_vid_dma_param1[1]<<8) | m_vid_dma_param1[0];
	uint16_t dst = (m_vid_dma_param2[0]<<8);
	dst += 0x6000;

	uint8_t unk = m_vid_dma_param2[1];

	logerror("%s: vid_dma_trigger_w with trg %02x size %04x src %04x dest %04x unk (%02x)\n", machine().describe_context(), data & 0xf8, len,src,dst,unk);

	if (unk)
	{
		fatalerror("m_vid_dma_param2[1] != 0x00 (is %02x)\n", m_vid_dma_param2[1]);
	}

	if (len == 0x00)
	{
		len = 0x08;
		logerror(" (length was 0x0, assuming 0x8)\n");
	}

	len = len << 8;

	address_space& dmaspace = m_maincpu->space(AS_PROGRAM);

	if (data & 0x40)
	{
		for (int i = 0; i < len; i++)
		{
			uint8_t dat = dmaspace.read_byte(src + i);
			dmaspace.write_byte(dst + i, dat);
		}
	}
}

READ8_MEMBER(xavix_state::vid_dma_trigger_r)
{
	// expects bit 0x40 to clear in most cases
	return 0x00;
}

READ8_MEMBER(xavix_state::pal_ntsc_r)
{
	// only seen 0x10 checked in code
	// in monster truck the tile base address gets set based on this, there are 2 copies of the test screen in rom, one for pal, one for ntsc, see 1854c
	// likewise card night has entirely different tilesets for each region title
	return m_region->read();
}

WRITE8_MEMBER(xavix_state::xavix_6fc0_w) // also related to tilemap 1?
{
	logerror("%s: xavix_6fc0_w data %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::tmap1_regs_w)
{
	/*
	   0x0 pointer to address where tile data is
	       it gets set to 0x40 in monster truck test mode, which is outside of ram but test mode requires a fixed 'column scan' layout
	       so that might be special

	   0x1 pointer to middle tile bits (if needed, depends on mode) (usually straight after the ram needed for above)

	   0x2 pointer to tile highest tile bits (if needed, depends on mode) (usually straight after the ram needed for above)

	   0x3 --tt bbb-     - = ?  tt = tile/tilemap size b = bpp    (0x36 xavix logo, 0x3c title screen, 0x36 course select)

	   0x4 and 0x5 are scroll

	   0x6 pppp ----     p = palette  - = ?   (0x02 xavix logo, 0x01 course select)

	   0x7 could be mode (16x16, 8x8 etc.)
	        0x00 is disabled?
	        0x80 means 16x16 tiles
	        0x81 might be 8x8 tiles
	        0x93 course / mode select bg / ingame (weird addressing?)


	 */

	/*
	    6aff base registers
	    -- ingame

	    ae 80
	    02 80
	    02 90
	    02 a0
	    02 b0
	    02 c0
	    02 d0
	    02 e0

	    02 00
	    04 80
	    04 90
	    04 a0
	    04 b0
	    04 c0
	    04 d0
	    04 e0

	    -- menu
	    af 80
	    27 80
	    27 90
	    27 a0
	    27 b0
	    27 c0
	    27 d0
	    27 e0

	    27 00
	    00 80
	    00 90
	    00 a0
	    00 b0
	    00 c0
	    00 d0
	    00 e0
	*/


	if ((offset != 0x4) && (offset != 0x5))
	{
		logerror("%s: tmap1_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap1_regs[offset]);
}

WRITE8_MEMBER(xavix_state::xavix_6fd8_w) // also related to tilemap 2?
{
	// possibly just a mirror of tmap2_regs_w, at least it writes 0x04 here which would be the correct
	// base address to otherwise write at tmap2_regs_w offset 0
//	tmap2_regs_w(space,offset,data);

	logerror("%s: xavix_6fd8_w data %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::tmap2_regs_w)
{
	// same as above but for 2nd tilemap
	if ((offset != 0x4) && (offset != 0x5))
	{
		//logerror("%s: tmap2_regs_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_tmap2_regs[offset]);
}

READ8_MEMBER(xavix_state::tmap2_regs_r)
{
	// does this return the same data or a status?

	logerror("%s: tmap2_regs_r offset %02x\n", offset, machine().describe_context());
	return m_tmap2_regs[offset];
}

READ8_MEMBER(xavix_state::xavix_4000_r)
{
	if (offset < 0x100)
	{
		return ((offset>>4) | (offset<<4));
	}
	else
	{
		return 0x00;
	}
}

READ8_MEMBER(xavix_state::irq_source_r)
{
	/* the 2nd IRQ routine (regular IRQ, not NMI?) reads here before deciding what to do

	 the following bits have been seen to be checked (active low?)

	  0x40 - Monster Truck - stuff with 6ffb 6fd6 and 6ff8
	  0x20 - most games (but not Monster Truck) - DMA related?
	  0x10 - card night + monster truck - 7c00 related? (increases 16-bit counter in ram stores 0xc1 at 7c00)
	  0x08 - several games - Input related (ADC? - used for analog control on Monster Truck) (uses 7a80 top bit to determine direction, and 7a81 0x08 as an output, presumably to clock)
	  0x04 - Monster Truck - loads/stores 7b81
	*/
	logerror("%s: irq_source_r\n", machine().describe_context());
	return 0xff;
}

WRITE8_MEMBER(xavix_state::irq_source_w)
{
	logerror("%s: irq_source_w %02x\n", machine().describe_context(), data);
	// cleared on startup in monster truck, no purpose?
}


// DATA reads from 0x8000-0xffff are banked by byte 0xff of 'ram' (this is handled in the CPU core)

ADDRESS_MAP_START(xavix_state::xavix_map)
	AM_RANGE(0x000000, 0x0001ff) AM_RAM
	AM_RANGE(0x000200, 0x003fff) AM_RAM AM_SHARE("mainram")

	// this might not be a real area, the tilemap base register gets set to 0x40 in monster truck service mode, and expects a fixed layout.
	// As that would point at this address maybe said layout is being read from here, or maybe it's just a magic tilemap register value that doesn't read address space at all.
	AM_RANGE(0x004000, 0x0041ff) AM_READ(xavix_4000_r)

	// 6xxx ranges are the video hardware
	// appears to be 256 sprites (shares will be renamed once their purpose is known)
	AM_RANGE(0x006000, 0x0060ff) AM_RAM AM_SHARE("spr_attr0")
	AM_RANGE(0x006100, 0x0061ff) AM_RAM AM_SHARE("spr_attr1")
	AM_RANGE(0x006200, 0x0062ff) AM_RAM AM_SHARE("spr_ypos") // cleared to 0x80 by both games, maybe enable registers?
	AM_RANGE(0x006300, 0x0063ff) AM_RAM AM_SHARE("spr_xpos")
	AM_RANGE(0x006400, 0x0064ff) AM_RAM // 6400 range gets populated in some cases, but it seems to be more like work ram, data doesn't matter and must be ignored?
	AM_RANGE(0x006500, 0x0065ff) AM_RAM AM_SHARE("spr_addr_lo")
	AM_RANGE(0x006600, 0x0066ff) AM_RAM AM_SHARE("spr_addr_md")
	AM_RANGE(0x006700, 0x0067ff) AM_RAM AM_SHARE("spr_addr_hi")
	AM_RANGE(0x006800, 0x0068ff) AM_RAM AM_SHARE("palram1") // written with 6900
	AM_RANGE(0x006900, 0x0069ff) AM_RAM AM_SHARE("palram2") // startup (taitons1)
	AM_RANGE(0x006a00, 0x006a1f) AM_RAM AM_SHARE("spr_attra") // test mode, pass flag 0x20


	AM_RANGE(0x006fc0, 0x006fc0) AM_WRITE(xavix_6fc0_w) // startup (maybe this is a mirror of tmap1_regs_w)

	AM_RANGE(0x006fc8, 0x006fcf) AM_WRITE(tmap1_regs_w) // video registers

	AM_RANGE(0x006fd0, 0x006fd7) AM_READWRITE(tmap2_regs_r, tmap2_regs_w)
	AM_RANGE(0x006fd8, 0x006fd8) AM_WRITE(xavix_6fd8_w) // startup (mirror of tmap2_regs_w?)

	AM_RANGE(0x006fe0, 0x006fe0) AM_READWRITE(vid_dma_trigger_r, vid_dma_trigger_w) // after writing to 6fe1/6fe2 and 6fe5/6fe6 rad_mtrk writes 0x43/0x44 here then polls on 0x40   (see function call at c273) write values are hardcoded, similar code at 18401
	AM_RANGE(0x006fe1, 0x006fe2) AM_WRITE(vid_dma_params_1_w)
	AM_RANGE(0x006fe5, 0x006fe6) AM_WRITE(vid_dma_params_2_w)

	// function in rad_mtrk at 0184b7 uses this
	AM_RANGE(0x006fe8, 0x006fe8) AM_RAM // r/w tested
	AM_RANGE(0x006fe9, 0x006fe9) AM_RAM // r/w tested
	//AM_RANGE(0x006fea, 0x006fea) AM_WRITENOP

	AM_RANGE(0x006ff0, 0x006ff0) AM_READWRITE(xavix_6ff0_r, xavix_6ff0_w) // r/w tested
	//AM_RANGE(0x006ff1, 0x006ff1) AM_WRITENOP // startup - cleared in interrupt 0
	//AM_RANGE(0x006ff2, 0x006ff2) AM_WRITENOP // set to 07 after clearing above things in interrupt 0

	AM_RANGE(0x006ff8, 0x006ff8) AM_READWRITE(xavix_6ff8_r, xavix_6ff8_w) // always seems to be a read/store or read/modify/store
	AM_RANGE(0x006ff9, 0x006ff9) AM_READ(pal_ntsc_r)

	// 7xxx ranges system controller?

	AM_RANGE(0x0075f0, 0x0075f0) AM_READWRITE(xavix_75f0_r, xavix_75f0_w) // r/w tested read/written 8 times in a row
	AM_RANGE(0x0075f1, 0x0075f1) AM_READWRITE(xavix_75f1_r, xavix_75f1_w) // r/w tested read/written 8 times in a row
	AM_RANGE(0x0075f3, 0x0075f3) AM_RAM
	AM_RANGE(0x0075f4, 0x0075f4) AM_READ(xavix_75f4_r) // related to 75f0 (read after writing there - rad_mtrk)
	AM_RANGE(0x0075f5, 0x0075f5) AM_READ(xavix_75f5_r) // related to 75f1 (read after writing there - rad_mtrk)

	// taitons1 after 75f7/75f8
	AM_RANGE(0x0075f6, 0x0075f6) AM_READWRITE(xavix_75f6_r, xavix_75f6_w) // r/w tested
	// taitons1 written as a pair
	AM_RANGE(0x0075f7, 0x0075f7) AM_WRITE(xavix_75f7_w)
	AM_RANGE(0x0075f8, 0x0075f8) AM_READWRITE(xavix_75f8_r, xavix_75f8_w) // r/w tested
	// taitons1 written after 75f6, then read
	AM_RANGE(0x0075f9, 0x0075f9) AM_READWRITE(xavix_75f9_r, xavix_75f9_w)
	// at another time
	AM_RANGE(0x0075fa, 0x0075fa) AM_READWRITE(xavix_75fa_r, xavix_75fa_w) // r/w tested
	AM_RANGE(0x0075fb, 0x0075fb) AM_READWRITE(xavix_75fb_r, xavix_75fb_w) // r/w tested
	AM_RANGE(0x0075fc, 0x0075fc) AM_READWRITE(xavix_75fc_r, xavix_75fc_w) // r/w tested
	AM_RANGE(0x0075fd, 0x0075fd) AM_READWRITE(xavix_75fd_r, xavix_75fd_w) // r/w tested
	AM_RANGE(0x0075fe, 0x0075fe) AM_WRITE(xavix_75fe_w)
	// taitons1 written other 75xx operations
	AM_RANGE(0x0075ff, 0x0075ff) AM_WRITE(xavix_75ff_w)

	//AM_RANGE(0x007810, 0x007810) AM_WRITENOP // startup

	//AM_RANGE(0x007900, 0x007900) AM_WRITE(xavix_7900_w) // startup
	//AM_RANGE(0x007902, 0x007902) AM_WRITENOP // startup

	// DMA trigger for below (written after the others) waits on status of bit 1 in a loop
	AM_RANGE(0x007980, 0x007980) AM_READWRITE(dma_trigger_r, dma_trigger_w)
	// DMA source
	AM_RANGE(0x007981, 0x007981) AM_WRITE(rom_dmasrc_lo_w)
	AM_RANGE(0x007982, 0x007982) AM_WRITE(rom_dmasrc_md_w)
	AM_RANGE(0x007983, 0x007983) AM_WRITE(rom_dmasrc_hi_w)
	// DMA dest
	AM_RANGE(0x007984, 0x007984) AM_WRITE(rom_dmadst_lo_w)
	AM_RANGE(0x007985, 0x007985) AM_WRITE(rom_dmadst_hi_w)
	// DMA length
	AM_RANGE(0x007986, 0x007986) AM_WRITE(rom_dmalen_lo_w)
	AM_RANGE(0x007987, 0x007987) AM_WRITE(rom_dmalen_hi_w)

	// GPIO stuff
	AM_RANGE(0x007a00, 0x007a00) AM_READ(xavix_io_0_r)
	AM_RANGE(0x007a01, 0x007a01) AM_READ(xavix_io_1_r) //AM_WRITENOP // startup (taitons1)
	//AM_RANGE(0x007a02, 0x007a02) AM_WRITENOP // startup, gets set to 20, 7a00 is then also written with 20
	//AM_RANGE(0x007a03, 0x007a03) AM_READNOP AM_WRITENOP // startup (gets set to 84 which is the same as the bits checked on 7a01, possible port direction register?)

	//AM_RANGE(0x007a80, 0x007a80) AM_WRITENOP

	//AM_RANGE(0x007b80, 0x007b80) AM_READNOP
	//AM_RANGE(0x007b81, 0x007b81) AM_WRITENOP // every frame?
	//AM_RANGE(0x007b82, 0x007b82) AM_WRITENOP

	//AM_RANGE(0x007c01, 0x007c01) AM_RAM // r/w tested
	//AM_RANGE(0x007c02, 0x007c02) AM_WRITENOP // once

	// this is a multiplication chip
	AM_RANGE(0x007ff2, 0x007ff4) AM_WRITE(mult_param_w)
	AM_RANGE(0x007ff5, 0x007ff6) AM_READ(mult_r)

	// maybe irq enable, written after below
	AM_RANGE(0x007ff9, 0x007ff9) AM_WRITE(irq_enable_w) // interrupt related, but probalby not a simple 'enable' otherwise interrupts happen before we're ready for them.
	// an IRQ vector (nmi?)
	AM_RANGE(0x007ffa, 0x007ffa) AM_WRITE(irq_vector0_lo_w)
	AM_RANGE(0x007ffb, 0x007ffb) AM_WRITE(irq_vector0_hi_w)

	AM_RANGE(0x007ffc, 0x007ffc) AM_READWRITE(irq_source_r, irq_source_w)

	// an IRQ vector (irq?)
	AM_RANGE(0x007ffe, 0x007ffe) AM_WRITE(irq_vector1_lo_w)
	AM_RANGE(0x007fff, 0x007fff) AM_WRITE(irq_vector1_hi_w)

//  rom is installed in init due to different rom sizes and mirroring required
//  AM_RANGE(0x008000, 0x7fffff) AM_ROM AM_REGION("bios", 0x008000) AM_MIRROR(0x800000) // rad_mtrk relies on rom mirroring
ADDRESS_MAP_END

static INPUT_PORTS_START( xavix )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "IN0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x00, "IN1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( xavixp )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

/* Test mode lists the following

  LED (on power button?)
  Throttle Low
  Throttle High
  Reverse
  NO2
  Steering Left (4 positions)
  Steering Right (4 positions)
  Horn

*/


static INPUT_PORTS_START( rad_mtrk )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Nitro")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Throttle High")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Throttle Low")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Reverse / Back")

	PORT_MODIFY("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Horn")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 ) // some kind of 'power off' (fades screen to black, jumps to an infinite loop) maybe low battery condition or just the power button?
INPUT_PORTS_END

static INPUT_PORTS_START( rad_mtrkp )
	PORT_INCLUDE(rad_mtrk)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_crdn )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // can press this to get to a game select screen
INPUT_PORTS_END

static INPUT_PORTS_START( rad_crdnp )
	PORT_INCLUDE(rad_crdn)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_box )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	// 6 types of punch and some navigation controls?
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left Jan")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Left Hook")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left Uppercut")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Left Jab")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Left Hook")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Left Uppercut")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Block")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // needs to be high to pass warning screen?
INPUT_PORTS_END

static INPUT_PORTS_START( rad_boxp )
	PORT_INCLUDE(rad_box)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( rad_snow )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Go") // is this a button, or 'up' ?

	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_snowp )
	PORT_INCLUDE(rad_snow)

	PORT_MODIFY("REGION") // PAL/NTSC flag
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END

static INPUT_PORTS_START( namcons2 )
	PORT_INCLUDE(xavix)

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
INPUT_PORTS_END

/* correct, 4bpp gfxs */
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static const gfx_layout char16layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ 1*4,0*4,3*4,2*4,5*4,4*4,7*4,6*4, 9*4,8*4,11*4,10*4,13*4,12*4,15*4,14*4   },
	{ STEP16(0,4*16) },
	16*16*4
};

static const gfx_layout charlayout8bpp =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static const gfx_layout char16layout8bpp =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16*16*8
};



static GFXDECODE_START( xavix )
	GFXDECODE_ENTRY( "bios", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "bios", 0, char16layout, 0, 16 )
	GFXDECODE_ENTRY( "bios", 0, charlayout8bpp, 0, 1 )
	GFXDECODE_ENTRY( "bios", 0, char16layout8bpp, 0, 1 )
GFXDECODE_END


void xavix_state::machine_start()
{
}

void xavix_state::machine_reset()
{
	m_rom_dmasrc_lo_data = 0;
	m_rom_dmasrc_md_data = 0;
	m_rom_dmasrc_hi_data = 0;

	m_rom_dmadst_lo_data = 0;
	m_rom_dmadst_hi_data = 0;

	m_rom_dmalen_lo_data = 0;
	m_rom_dmalen_hi_data = 0;

	m_irq_enable_data = 0;
	m_irq_vector0_lo_data = 0;
	m_irq_vector0_hi_data = 0;
	m_irq_vector1_lo_data = 0;
	m_irq_vector1_hi_data = 0;

	m_6ff0 = 0;
	m_6ff8 = 0;

	m_75f0 = 0;
	m_75f1 = 0;

	m_75f6 = 0;
	m_75f8 = 0;
	m_75fa = 0;
	m_75fb = 0;
	m_75fc = 0;
	m_75fd = 0;

	for (int i = 0; i < 3; i++)
		m_multparams[i] = 0;

	for (int i = 0; i < 2; i++)
		m_multresults[i] = 0;

	for (int i = 0; i < 2; i++)
	{
		m_vid_dma_param1[i] = 0;
		m_vid_dma_param2[i] = 0;
	}

	for (int i = 0; i < 8; i++)
	{
		m_tmap1_regs[i] = 0;
		m_tmap2_regs[i] = 0;
	}
}

typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

uint8_t xavix_state::get_vectors(int which, int half)
{
//  logerror("get_vectors %d %d\n", which, half);

	if (which == 0) // irq?
	{
		if (half == 0)
			return m_irq_vector0_hi_data;
		else
			return m_irq_vector0_lo_data;
	}
	else
	{
		if (half == 0)
			return m_irq_vector1_hi_data;
		else
			return m_irq_vector1_lo_data;
	}
}

MACHINE_CONFIG_START(xavix_state::xavix)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",XAVIX,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(xavix_map)
	MCFG_M6502_DISABLE_DIRECT()
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xavix_state,  interrupt)
	MCFG_XAVIX_VECTOR_CALLBACK(xavix_state, get_vectors)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", xavix_state, scanline_cb, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(xavix_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xavix)

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// sound is PCM
MACHINE_CONFIG_END


MACHINE_CONFIG_START(xavix_state::xavixp)
	xavix(config);

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER(xavix_state,xavix)
{
	m_rgnlen = memregion("bios")->bytes();
	m_rgn = memregion("bios")->base();

	// mirror the rom across the space
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_rom(0x8000, m_rgnlen, ((~(m_rgnlen-1))<<1)&0xffffff, m_rgn+0x8000);
}

DRIVER_INIT_MEMBER(xavix_state, taitons1)
{
	DRIVER_INIT_CALL(xavix);
	m_alt_addressing = 1;
}

DRIVER_INIT_MEMBER(xavix_state, rad_box)
{
	DRIVER_INIT_CALL(xavix);
	m_alt_addressing = 2;
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( taitons1 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia1.u3", 0x000000, 0x200000, CRC(25bd8c67) SHA1(a109cd2da6aa4596e3ca3abd1afce2d0001a473f) )
ROM_END

ROM_START( taitons2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia2.bin", 0x000000, 0x200000, CRC(d7dbd93d) SHA1(ad96f80d317e7fd64682a1fe406c5ee9dd5eabf9) )
ROM_END

ROM_START( namcons1 )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "namconostalgia1.bin", 0x000000, 0x100000, CRC(9bcccccd) SHA1(cf8fe6de76fbd23974f999299db6f558f79c8f22) )
ROM_END

ROM_START( namcons2 )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "nostalgia.bin", 0x000000, 0x100000, CRC(03f7f755) SHA1(bdf1b10ab0104ed580951b0c428c4e93e7373afe) )
ROM_END

ROM_START( rad_box )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("boxing.bin", 0x000000, 0x200000, CRC(5cd40714) SHA1(165260228c029a9502ca0598c84c24fd9bdeaebe) )
ROM_END

ROM_START( rad_boxp )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("boxing.bin", 0x000000, 0x200000, CRC(5cd40714) SHA1(165260228c029a9502ca0598c84c24fd9bdeaebe) )
ROM_END

ROM_START( rad_bass )
	ROM_REGION(0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bassfishin.bin", 0x000000, 0x100000, CRC(b54eb1c5) SHA1(084faa9349369f2b8846950765f9c8f758db3e9e) )
ROM_END

ROM_START( rad_bassp )
	ROM_REGION(0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("bassfishin.bin", 0x000000, 0x100000, CRC(b54eb1c5) SHA1(084faa9349369f2b8846950765f9c8f758db3e9e) )
ROM_END

ROM_START( rad_snow )
	ROM_REGION(0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snoblu.bin", 0x000000, 0x100000, CRC(593e40b3) SHA1(03483ac39eddd7746470fb60018e704382b0da59) )
ROM_END

ROM_START( rad_snowp )
	ROM_REGION(0x100000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("snoblu.bin", 0x000000, 0x100000, CRC(593e40b3) SHA1(03483ac39eddd7746470fb60018e704382b0da59) )
ROM_END


ROM_START( rad_ping )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pingpong.bin", 0x000000, 0x100000, CRC(629f7f47) SHA1(2bb19fd202f1e6c319d2f7d18adbfed8a7669235) )
ROM_END

ROM_START( rad_crdn )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "cardnight.bin", 0x000000, 0x100000, CRC(d19eba08) SHA1(cedb9fe785f2a559f518a1d8ecf80d500ddc63c7) )
ROM_END

ROM_START( rad_crdnp )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "cardnight.bin", 0x000000, 0x100000, CRC(d19eba08) SHA1(cedb9fe785f2a559f518a1d8ecf80d500ddc63c7) )
ROM_END

ROM_START( rad_bb2 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball2.bin", 0x000000, 0x200000, CRC(bdbf6202) SHA1(18d5cc2d77cbb734629a7a5b6e0f419d21beedbd) )
ROM_END

ROM_START( rad_mtrk )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "monstertruck.bin", 0x000000, 0x400000, CRC(dccda0a7) SHA1(7953cf29643672f8367639555b797c20bb533eab) )
ROM_END

ROM_START( rad_mtrkp ) // rom was dumped from NTSC unit, assuming to be the same
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "monstertruck.bin", 0x000000, 0x400000, CRC(dccda0a7) SHA1(7953cf29643672f8367639555b797c20bb533eab) )
ROM_END


ROM_START( rad_madf )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("madden.bin", 0x000000, 0x400000, CRC(e972fdcf) SHA1(52001316254880755da959c3441d232fd2c72c7a) )
ROM_END

ROM_START( rad_fb )
	ROM_REGION(0x400000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("rfootball.bin", 0x000000, 0x400000, CRC(025e0cb4) SHA1(60ce363de236d5119d078e346ad5d2ae50dbc7e1) )
ROM_END

ROM_START( eka_strt )
	ROM_REGION( 0x080000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekarastartcart.bin", 0x000000, 0x080000, CRC(8c12c0c2) SHA1(8cc1b098894af25a4bfccada884125b66f5fe8b2) )
ROM_END

ROM_START( has_wamg )
	ROM_REGION( 0x400000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "minigolf.bin", 0x000000, 0x400000, CRC(35cee2ad) SHA1(c7344e8ba336bc329638485ea571cd731ebf7649) )
ROM_END

/* Standalone TV Games */

CONS( 2006, taitons1,  0,          0,  xavix,  xavix,    xavix_state, taitons1, "Bandai / SSD Company LTD / Taito", "Let's! TV Play Classic - Taito Nostalgia 1", MACHINE_IS_SKELETON )

CONS( 2006, taitons2,  0,          0,  xavix,  namcons2, xavix_state, xavix,    "Bandai / SSD Company LTD / Taito", "Let's! TV Play Classic - Taito Nostalgia 2", MACHINE_IS_SKELETON )

CONS( 2006, namcons1,  0,          0,  xavix,  namcons2, xavix_state, taitons1, "Bandai / SSD Company LTD / Namco", "Let's! TV Play Classic - Namco Nostalgia 1", MACHINE_IS_SKELETON )

CONS( 2006, namcons2,  0,          0,  xavix,  namcons2, xavix_state, taitons1, "Bandai / SSD Company LTD / Namco", "Let's! TV Play Classic - Namco Nostalgia 2", MACHINE_IS_SKELETON )

CONS( 2000, rad_ping,  0,          0,  xavix,  xavix,    xavix_state, xavix,    "Radica / SSD Company LTD / Simmer Technology", "Play TV Ping Pong", MACHINE_IS_SKELETON ) // "Simmer Technology" is also known as "Hummer Technology Co., Ltd"

CONS( 2003, rad_mtrk,  0,          0,  xavix,  rad_mtrk, xavix_state, xavix,    "Radica / SSD Company LTD",                     "Play TV Monster Truck (NTSC)", MACHINE_IS_SKELETON )
CONS( 2003, rad_mtrkp, rad_mtrk,   0,  xavixp, rad_mtrkp,xavix_state, xavix,    "Radica / SSD Company LTD",                     "ConnecTV Monster Truck (PAL)", MACHINE_IS_SKELETON )

CONS( 200?, rad_box,   0,          0,  xavix,  rad_box,  xavix_state, rad_box,  "Radica / SSD Company LTD",                     "Play TV Boxing (NTSC)", MACHINE_IS_SKELETON)
CONS( 200?, rad_boxp,  rad_box,    0,  xavixp, rad_boxp, xavix_state, rad_box,  "Radica / SSD Company LTD",                     "ConnecTV Boxing (PAL)", MACHINE_IS_SKELETON)
 
CONS( 200?, rad_crdn,  0,          0,  xavix,  rad_crdn, xavix_state, rad_box,  "Radica / SSD Company LTD",                     "Play TV Card Night (NTSC)", MACHINE_IS_SKELETON)
CONS( 200?, rad_crdnp, rad_crdn,   0,  xavixp, rad_crdnp,xavix_state, rad_box,  "Radica / SSD Company LTD",                     "ConnecTV Card Night (PAL)", MACHINE_IS_SKELETON)

CONS( 2002, rad_bb2,   0,          0,  xavix,  xavix,    xavix_state, xavix,    "Radica / SSD Company LTD",                     "Play TV Baseball 2", MACHINE_IS_SKELETON ) // contains string "Radica RBB2 V1.0"

CONS( 2001, rad_bass,  0,          0,  xavix,  xavix,    xavix_state, rad_box,  "Radica / SSD Company LTD",                     "Play TV Bass Fishin' (NTSC)", MACHINE_IS_SKELETON)
CONS( 2001, rad_bassp, rad_bass,   0,  xavixp, xavixp,   xavix_state, rad_box,  "Radica / SSD Company LTD",                     "ConnecTV Bass Fishin' (PAL)", MACHINE_IS_SKELETON)

// there is another 'Snowboarder' with a white coloured board, it appears to be a newer game closer to 'SSX Snowboarder' but without the SSX license.
CONS( 2001, rad_snow,  0,          0,  xavix,  rad_snow, xavix_state, rad_box,  "Radica / SSD Company LTD",                     "Play TV Snowboarder (Blue) (NTSC)", MACHINE_IS_SKELETON)
CONS( 2001, rad_snowp, rad_snow,   0,  xavixp, rad_snowp,xavix_state, rad_box,  "Radica / SSD Company LTD",                     "ConnecTV Snowboarder (Blue) (PAL)", MACHINE_IS_SKELETON)

CONS( 2003, rad_madf,   0,          0,  xavix,  xavix,  xavix_state, taitons1,  "Radica / SSD Company LTD",                     "EA Sports Madden Football (NTSC)", MACHINE_IS_SKELETON) // no Play TV branding, USA only release?

CONS( 200?, rad_fb,     0,          0,  xavix,  xavix,  xavix_state, taitons1,  "Radica / SSD Company LTD",                     "Play TV Football (NTSC)", MACHINE_IS_SKELETON) // USA only release? doesn't change logo for PAL

CONS( 200?, has_wamg,  0,          0,  xavix,  xavix,    xavix_state, rad_box,  "Hasbro / Milton Bradley / SSD Company LTD",     "TV Wild Adventure Mini Golf", MACHINE_IS_SKELETON)

CONS (200?, eka_strt,  0,          0,  xavix,  xavix,    xavix_state, xavix, "Takara / SSD Company LTD",                     "e-kara Starter", MACHINE_IS_SKELETON)

/* The 'XaviXPORT' isn't a real console, more of a TV adapter, all the actual hardware (CPU including video hw, sound hw) is in the cartridges and controllers
   and can vary between games, see notes at top of driver.
*/

ROM_START( xavtenni )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xavixtennis.bin", 0x000000, 0x800000, CRC(23a1d918) SHA1(2241c59e8ea8328013e55952ebf9060ea0a4675b) )
ROM_END

/* Tiger games have extended opcodes too */


ROM_START( ttv_sw )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "jedi.bin", 0x000000, 0x800000, CRC(51cae5fd) SHA1(1ed8d556f31b4182259ca8c766d60c824d8d9744) )
ROM_END

ROM_START( ttv_lotr )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "lotr.bin", 0x000000, 0x800000, CRC(a034ecd5) SHA1(264a9d4327af0a075841ad6129db67d82cf741f1) )
ROM_END

ROM_START( ttv_mx )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "mxdirtrebel.bin", 0x000000, 0x800000, CRC(e64bf1a1) SHA1(137f97d7d857697a13e0c8984509994dc7bc5fc5) )
ROM_END


CONS( 2004, xavtenni,  0,   0,  xavix,  xavix, xavix_state, xavix, "SSD Company LTD",         "XaviX Tennis (XaviXPORT)", MACHINE_IS_SKELETON )

CONS( 2005, ttv_sw,    0,   0,  xavix,  xavix, xavix_state, xavix, "Tiger / SSD Company LTD", "Star Wars Saga Edition - Lightsaber Battle Game", MACHINE_IS_SKELETON )
CONS( 2005, ttv_lotr,  0,   0,  xavix,  xavix, xavix_state, xavix, "Tiger / SSD Company LTD", "Lord Of The Rings - Warrior of Middle-Earth", MACHINE_IS_SKELETON )
CONS( 2005, ttv_mx,    0,   0,  xavix,  xavix, xavix_state, xavix, "Tiger / SSD Company LTD", "MX Dirt Rebel", MACHINE_IS_SKELETON )
