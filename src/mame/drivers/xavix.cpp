// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Skeleton driver for XaviX TV PNP console and childs (Let's! Play TV Classic)

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

	Important addresses

	0x18340 in Monster Truck is the self-test mode, shows me which registers need to retain their values etc.

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/xavix.h"
#include "machine/timer.h"
#include "screen.h"
#include "speaker.h"


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
		m_gfxdecode(*this, "gfxdecode"),
		m_alt_addressing(0),
		m_tilemap_enabled(1)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void xavix(machine_config &config);

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
	DECLARE_WRITE8_MEMBER(xavix_6fc8_w);
	DECLARE_WRITE8_MEMBER(xavix_6fd8_w);
	DECLARE_WRITE8_MEMBER(xavix_6fd7_w);
	DECLARE_READ8_MEMBER(xavix_6fd7_r);

	DECLARE_READ8_MEMBER(pal_ntsc_r);

	DECLARE_READ8_MEMBER(mult_r);
	DECLARE_WRITE8_MEMBER(mult_param_w);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	DECLARE_DRIVER_INIT(xavix);
	DECLARE_DRIVER_INIT(taitons1);

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

	uint8_t m_6fc8_regs[8];

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

	required_device<gfxdecode_device> m_gfxdecode;

	void handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	double hue2rgb(double p, double q, double t);

	int m_rgnlen;
	uint8_t* m_rgn;

	// variables used by rendering
	int m_tmp_dataaddress;
	int m_tmp_databit;
	void set_data_address(int address, int bit);
	uint8_t get_next_bit();

	int m_alt_addressing;
	int m_tilemap_enabled;

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
		//	logerror("hraw >24 (%02x)\n", h_raw);

		//if (l_raw > 17)
		//	logerror("lraw >17 (%02x)\n", l_raw);

		//if (sl_raw > 7)
		//	logerror("sl_raw >5 (%02x)\n", sl_raw);

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


uint32_t xavix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	handle_palette(screen, bitmap, cliprect);

	bitmap.fill(0, cliprect);

	if (m_tilemap_enabled)
	{
		// why are the first two logos in Monster Truck stored as tilemaps in main ram?
		// test mode isn't? is the code meant to process them instead? - there are no sprites in the sprite list at the time (and only one in test mode)
		gfx_element *gfx;
		int count;
		
		count = 0;
		gfx = m_gfxdecode->gfx(1);
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				int tile = m_mainram[count];
				tile |= (m_mainram[count+0x100]<<8);
				count++;

				int gfxbase = (m_spr_attra[1] << 8) | (m_spr_attra[0]);
				// upper bit is often set, maybe just because it relies on mirroring, maybe other purpose

				tile += (gfxbase << 1);

				gfx->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 16, y * 16);
			}
		}


		// there is a 2nd layer in monster truck too, there must be more to the gfxbase tho, because the lower layer can't be using the same gfxbase..
		count = 0x200;
		gfx = m_gfxdecode->gfx(0);
		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				int tile = m_mainram[count];
				tile |= (m_mainram[count+0x400]<<8);
				count++;

				int gfxbase = (m_spr_attra[1] << 8) | (m_spr_attra[0]);
				// upper bit is often set, maybe just because it relies on mirroring, maybe other purpose

				// even the transpen makes no sense here, it's 0 on the used elements, 15 on the unused ones.. are 00 tiles just ignored?
				if (tile)
				{
					tile += (gfxbase << 3);
					gfx->transpen(bitmap, cliprect, tile, 0, 0, 0, x * 8, (y * 8) - 16, 0);
				}
			}
		}




	}

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
			if (m_alt_addressing)
				tile = tile * 128;
		}
		else if ((attr1 & 0x0c) == 0x08)
		{
			drawheight = 16;
			drawwidth = 8;
			xpos += 4;
			if (m_alt_addressing)
				tile = tile * 64;
		}
		else if ((attr1 & 0x0c) == 0x04)
		{
			drawheight = 8;
			drawwidth = 16;
			ypos -= 4;
			if (m_alt_addressing)
				tile = tile * 64;
		}
		else if ((attr1 & 0x0c) == 0x00)
		{
			drawheight = 8;
			drawwidth = 8;
			xpos += 4;
			ypos -= 4;
			if (m_alt_addressing)
				tile = tile * 32;
		}

		if (m_alt_addressing)
			tile += 0xd8000;

		ypos = 0xff - ypos;

		xpos -= 136;
		ypos -= 152;

		xpos &= 0xff;
		ypos &= 0xff;

		if (ypos >=192)
			ypos -= 256;

		int bpp = 1;

		bpp = (attr0 & 0x0e) >> 1;
		bpp += 1;

		// set the address here so we can increment in bits in the draw function
		set_data_address(tile, 0);

		for (int y = 0; y < drawheight; y++)
		{
			int row = ypos + y;

			for (int x = 0; x < drawwidth; x++)
			{

				int col;

				if (flipx)
				{
					col = xpos - x;
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

				if ((row >= cliprect.min_x && row < cliprect.max_x) && (col >= cliprect.min_y && col < cliprect.max_y))
				{
					uint16_t* rowptr;

					rowptr = &bitmap.pix16(row);
					if (dat)
						rowptr[col] = (dat + (pal << 4)) & 0xff;
				}
			}

		}


		/*
		if ((m_spr_ypos[i] != 0x81) && (m_spr_ypos[i] != 0x80) && (m_spr_ypos[i] != 0x00))
		{
			logerror("sprite with enable? %02x attr0 %02x attr1 %02x attr3 %02x attr5 %02x attr6 %02x attr7 %02x\n", m_spr_ypos[i], m_spr_attr0[i], m_spr_attr1[i], m_spr_xpos[i], m_spr_addr_lo[i], m_spr_addr_md[i], m_spr_addr_hi[i] );
		}
		*/
	}

	return 0;
}


WRITE8_MEMBER(xavix_state::dma_trigger_w)
{
	logerror("%s: dma_trigger_w %02x\n", machine().describe_context(), data);

	uint32_t source = (m_rom_dmasrc_hi_data << 16) | (m_rom_dmasrc_md_data<<8) | m_rom_dmasrc_lo_data;
	uint16_t dest = (m_rom_dmadst_hi_data<<8) | m_rom_dmadst_lo_data;
	uint16_t len = (m_rom_dmalen_hi_data<<8) | m_rom_dmalen_lo_data;

	source &= m_rgnlen-1;
	logerror("  (possible DMA op SRC %08x DST %04x LEN %04x)\n", source, dest, len);

	address_space& destspace = m_maincpu->space(AS_PROGRAM);

	for (int i = 0; i < len; i++)
	{
		uint8_t dat = m_rgn[(source + i) & (m_rgnlen-1)];
		destspace.write_byte(dest + i, dat);
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
	//	if (m_irq_enable_data != 0)
	//		m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);

	if (m_irq_enable_data != 0)
	{
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
	//return 0x00;
	//return 0x01; // enter test mode
	return m_in0->read();
}

READ8_MEMBER(xavix_state::xavix_io_1_r)
{
	//logerror("%s: xavix_io_1_r (random status?)\n", machine().describe_context());
	/*	

	rad_mtrk checks for 0x40 at (interrupt code)

	008f02: lda $7a01 -- ??
	008f05: and #$40
	008f07: beq $8f17
	(code that leads to a far call screen fade function with a 'jump to self' at the end, no obvious way out)
	008f17 - normal flow?

	opposite condition to above
	018ac5: lda $7a01 -- ??
	018ac8: and #$40
	018aca: bne $18ac5

	there's a write with 0x80 before the 'jump to self'
	018aea: lda #$80
	018aec: sta $7a01
	018aef: jmp $18aef


	and 0x02 elsewhere

	near end of interrupt function will conditionally change the value stored at 0x33 to 00 or ff depending on this bit

	008f24: lda $7a01
	008f27: and #$02
	008f29: bne $8f3c
	(skips over code to store 0x00 at $33)

	008f31: lda $7a01
	008f34: and #$02
	008f36: beq $8f3c
	(skips over code to store 0xff at $33)

	there's a similar check fo 0x02 that results in a far call
	008099: lda $7a01
	00809c: and #$02
	00809e: beq $80ab

	writes 00 04 and 80

	*/

	//return 0x02;
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
	return 0x00; // NTSC
	//return 0x10; // PAL
}

WRITE8_MEMBER(xavix_state::xavix_6fc0_w)
{
	//logerror("%s: xavix_6fc0_w data %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_6fc8_w)
{
	// 0x4 and 0x5 appear to be bg tilemap scroll

	if ((offset != 0x4) && (offset != 0x5))
	{
		logerror("%s: xavix_6fc8_w offset %02x data %02x\n", machine().describe_context(), offset, data);
	}

	COMBINE_DATA(&m_6fc8_regs[offset]);
}

WRITE8_MEMBER(xavix_state::xavix_6fd8_w)
{
	logerror("%s: xavix_6fd8_w data %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(xavix_state::xavix_6fd7_w)
{
	logerror("%s: xavix_6fd7_w data %02x\n", machine().describe_context(), data);
}

READ8_MEMBER(xavix_state::xavix_6fd7_r)
{
	logerror("%s: xavix_6fd7_r\n", machine().describe_context());
	return machine().rand();
}


// DATA reads from 0x8000-0xffff are banked by byte 0xff of 'ram' (this is handled in the CPU core)

ADDRESS_MAP_START(xavix_state::xavix_map)
	AM_RANGE(0x000000, 0x0001ff) AM_RAM
	AM_RANGE(0x000200, 0x003fff) AM_RAM AM_SHARE("mainram")

	// 6xxx ranges are the video hardware
	// appears to be 256 sprites (shares will be renamed once their purpose is known)
	AM_RANGE(0x006000, 0x0060ff) AM_RAM AM_SHARE("spr_attr0")
	AM_RANGE(0x006100, 0x0061ff) AM_RAM AM_SHARE("spr_attr1")
	AM_RANGE(0x006200, 0x0062ff) AM_RAM AM_SHARE("spr_ypos") // cleared to 0x8f0 by both games, maybe enable registers?
	AM_RANGE(0x006300, 0x0063ff) AM_RAM AM_SHARE("spr_xpos")
	AM_RANGE(0x006400, 0x0064ff) AM_RAM // 6400 range unused by code, does it exist?
	AM_RANGE(0x006500, 0x0065ff) AM_RAM AM_SHARE("spr_addr_lo")
	AM_RANGE(0x006600, 0x0066ff) AM_RAM AM_SHARE("spr_addr_md")
	AM_RANGE(0x006700, 0x0067ff) AM_RAM AM_SHARE("spr_addr_hi")
	AM_RANGE(0x006800, 0x0068ff) AM_RAM AM_SHARE("palram1") // written with 6900
	AM_RANGE(0x006900, 0x0069ff) AM_RAM AM_SHARE("palram2") // startup (taitons1)
	AM_RANGE(0x006a00, 0x006a1f) AM_RAM AM_SHARE("spr_attra") // test mode, pass flag 0x20

	
	AM_RANGE(0x006fc0, 0x006fc0) AM_WRITE(xavix_6fc0_w) // startup

	AM_RANGE(0x006fc8, 0x006fcf) AM_WRITE(xavix_6fc8_w) // video registers

	AM_RANGE(0x006fd7, 0x006fd7) AM_READWRITE(xavix_6fd7_r, xavix_6fd7_w)
	AM_RANGE(0x006fd8, 0x006fd8) AM_WRITE(xavix_6fd8_w) // startup (taitons1)
	
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
	AM_RANGE(0x007ff9, 0x007ff9) AM_WRITE(irq_enable_w)
	// an IRQ vector (nmi?)
	AM_RANGE(0x007ffa, 0x007ffa) AM_WRITE(irq_vector0_lo_w)
	AM_RANGE(0x007ffb, 0x007ffb) AM_WRITE(irq_vector0_hi_w)
	// an IRQ vector (irq?)
	AM_RANGE(0x007ffe, 0x007ffe) AM_WRITE(irq_vector1_lo_w)
	AM_RANGE(0x007fff, 0x007fff) AM_WRITE(irq_vector1_hi_w)

//  rom is installed in init due to different rom sizes and mirroring required
//	AM_RANGE(0x008000, 0x7fffff) AM_ROM AM_REGION("bios", 0x008000) AM_MIRROR(0x800000) // rad_mtrk relies on rom mirroring
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
		m_6fc8_regs[i] = 0;
}

typedef device_delegate<uint8_t (int which, int half)> xavix_interrupt_vector_delegate;

uint8_t xavix_state::get_vectors(int which, int half)
{
//	logerror("get_vectors %d %d\n", which, half);

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
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xavix_state,  interrupt)
	MCFG_XAVIX_VECTOR_CALLBACK(xavix_state, get_vectors)

	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", xavix_state, scanline_cb, "screen", 0, 1)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(xavix_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xavix)

	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	// sound is PCM
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
	m_tilemap_enabled = 0;
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( taitons1 )
	ROM_REGION( 0x200000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "taitonostalgia1.u3", 0x000000, 0x200000, CRC(25bd8c67) SHA1(a109cd2da6aa4596e3ca3abd1afce2d0001a473f) )
ROM_END

ROM_START( rad_box )
	ROM_REGION(0x200000, "bios", ROMREGION_ERASE00)
	ROM_LOAD("boxing.bin", 0x000000, 0x200000, CRC(5cd40714) SHA1(165260228c029a9502ca0598c84c24fd9bdeaebe) )
ROM_END

ROM_START( rad_ping )
	ROM_REGION( 0x100000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "pingpong.bin", 0x000000, 0x100000, CRC(629f7f47) SHA1(2bb19fd202f1e6c319d2f7d18adbfed8a7669235) )
ROM_END

ROM_START( rad_crdn )
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

ROM_START( eka_strt )
	ROM_REGION( 0x080000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "ekarastartcart.bin", 0x000000, 0x080000, CRC(8c12c0c2) SHA1(8cc1b098894af25a4bfccada884125b66f5fe8b2) )
ROM_END

/* Standalone TV Games */

CONS( 2006, taitons1,  0,   0,  xavix,  xavix,    xavix_state, taitons1, "Bandai / SSD Company LTD / Taito", "Let's! TV Play Classic - Taito Nostalgia 1", MACHINE_IS_SKELETON )

CONS( 2000, rad_ping,  0,   0,  xavix,  xavix,    xavix_state, xavix, "Radica / SSD Company LTD / Simmer Technology", "Play TV Ping Pong", MACHINE_IS_SKELETON ) // "Simmer Technology" is also known as "Hummer Technology Co., Ltd"
CONS( 2003, rad_mtrk,  0,   0,  xavix,  rad_mtrk, xavix_state, xavix, "Radica / SSD Company LTD",                     "Play TV Monster Truck", MACHINE_IS_SKELETON )
CONS( 200?, rad_box,   0,   0,  xavix,  xavix,    xavix_state, xavix, "Radica / SSD Company LTD",                     "Play TV Boxing", MACHINE_IS_SKELETON)
CONS( 200?, rad_crdn,  0,   0,  xavix,  xavix,    xavix_state, xavix, "Radica / SSD Company LTD",                     "Play TV Card Night", MACHINE_IS_SKELETON)
CONS( 2002, rad_bb2,   0,   0,  xavix,  xavix,    xavix_state, xavix, "Radica / SSD Company LTD",                     "Play TV Baseball 2", MACHINE_IS_SKELETON ) // contains string "Radica RBB2 V1.0"

CONS (200?, eka_strt,  0,   0,  xavix,  xavix,    xavix_state, xavix, "Takara / SSD Company LTD",                     "e-kara Starter", MACHINE_IS_SKELETON)

/* The 'XaviXPORT' isn't a real console, more of a TV adapter, all the actual hardware (CPU including video hw, sound hw) is in the cartridges and controllers
   and can vary between games, see notes at top of driver.

   According to sources XaviX Tennis should be a standard XaviX CPU, but at the very least makes significantly more use of custom opcodes than the above titles
   which only appears to use the call far / return far for extended memory space.
   
   Furthermore it also seems to require some regular 6502 opcodes to be replaced with custom ones, yet the other games expect these to act normally.  This
   leads me to believe that XaviX Tennis is almost certainly a Super XaviX title.

   The CPU die on XaviX Tennis is internally marked as NEC 85054-611 and is very different to thw two below

   Radica Monster truck die is marked SSD PL7351 with SSD98 also printed on the die
   Radia Ping Pong      die is marked SSD PA7270 with SSD97 also printed on the die (otherwise looks identical to Monster Truck)
	
   Star Wars Saga Edition also seems to be making use of additional opcodes, meaning it could also be Super Xavix, or something in-between (unless they're
   caused by the bad dump, but it looks intentional)

*/

ROM_START( xavtenni )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	ROM_LOAD( "xavixtennis.bin", 0x000000, 0x800000, CRC(23a1d918) SHA1(2241c59e8ea8328013e55952ebf9060ea0a4675b) )
ROM_END


ROM_START( ttv_sw )
	ROM_REGION( 0x800000, "bios", ROMREGION_ERASE00 )
	// wasn't giving consistent reads
	ROM_LOAD( "jedibad.bin", 0x000000, 0x800000, BAD_DUMP CRC(a12862fe) SHA1(9b5a07bdf35f72f2e2d127de5e6849ce5fa2afa0) )
ROM_END


CONS( 2004, xavtenni,  0,   0,  xavix,  xavix, xavix_state, xavix, "SSD Company LTD",         "XaviX Tennis (XaviXPORT)", MACHINE_IS_SKELETON )
CONS( 2005, ttv_sw,    0,   0,  xavix,  xavix, xavix_state, xavix, "Tiger / SSD Company LTD", "Star Wars Saga Edition - Lightsaber Battle Game", MACHINE_IS_SKELETON )


