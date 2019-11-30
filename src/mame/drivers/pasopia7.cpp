// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    Toshiba Pasopia 7 (c) 1983 Toshiba

    Preliminary driver by Angelo Salese.
    2019-10-14 added cassette & beeper [Robbbert]

    Machine unusable due to issues with sound.
    Cassette works.
    Beeper is only used for the keyclick, and it works.

    TODO:
    - floppy support (but floppy images are unobtainable at current time).
    - SOUND command uses the SN76489 chip, however no sound is produced, and the next SOUND command
      freezes the machine.
    - LCD version has gfx bugs, it must use a different ROM charset for instance (apparently a 8 x 4
      one, 40/80 x 8 tilemap).
    - Allow BIN files to be loaded (via a rampac presumably).
    - Allow CAS files to be loaded.

    Reading fdc has been commented out, until the code can be modified to
    work with new upd765 (was causing a hang at boot).

    Schematics: https://archive.org/details/Io19839/page/n331 (fdc system not included)

***************************************************************************************************/

#include "emu.h"
#include "includes/pasopia.h"

#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/upd765.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/sn76496.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class pasopia7_state : public driver_device
{
public:
	pasopia7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_ctc(*this, "ctc")
		, m_pio(*this, "pio")
		, m_crtc(*this, "crtc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0:525hd")
		, m_sn1(*this, "sn1")
		, m_sn2(*this, "sn2")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "KEY.%d", 0)
		, m_cass(*this, "cassette")
		, m_speaker(*this, "speaker")
	{ }

	void p7_base(machine_config &config);
	void p7_lcd(machine_config &config);
	void p7_raster(machine_config &config);

	void init_p7_lcd();
	void init_p7_raster();

private:
	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(pasopia7_memory_ctrl_w);
	DECLARE_WRITE8_MEMBER(pac2_w);
	DECLARE_READ8_MEMBER(pac2_r);
	DECLARE_WRITE8_MEMBER(ram_bank_w);
	DECLARE_WRITE8_MEMBER(pasopia7_6845_w);
	DECLARE_READ8_MEMBER(pasopia7_io_r);
	DECLARE_WRITE8_MEMBER(pasopia7_io_w);
	DECLARE_READ8_MEMBER(pasopia7_fdc_r);
	DECLARE_WRITE8_MEMBER(pasopia7_fdc_w);
	DECLARE_READ8_MEMBER(keyb_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_READ8_MEMBER(crtc_portb_r);
	DECLARE_WRITE8_MEMBER(screen_mode_w);
	DECLARE_WRITE8_MEMBER(plane_reg_w);
	DECLARE_WRITE8_MEMBER(video_attr_w);
	DECLARE_WRITE8_MEMBER(video_misc_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_READ8_MEMBER(nmi_reg_r);
	DECLARE_WRITE8_MEMBER(nmi_reg_w);
	DECLARE_READ8_MEMBER(nmi_porta_r);
	DECLARE_READ8_MEMBER(nmi_portb_r);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	TIMER_CALLBACK_MEMBER(pio_timer);
	DECLARE_VIDEO_START(pasopia7);
	void p7_lcd_palette(palette_device &palette) const;
	uint32_t screen_update_pasopia7(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pasopia7_io(address_map &map);
	void pasopia7_mem(address_map &map);

	uint8_t m_vram_sel;
	uint8_t m_mio_sel;
	std::unique_ptr<uint8_t[]> m_p7_pal;
	uint8_t m_bank_reg;
	uint16_t m_cursor_addr;
	uint8_t m_cursor_blink;
	uint8_t m_cursor_raster;
	uint8_t m_plane_reg;
	uint8_t m_attr_data;
	uint8_t m_attr_wrap;
	uint8_t m_attr_latch;
	uint8_t m_pal_sel;
	uint8_t m_x_width;
	uint8_t m_gfx_mode;
	uint8_t m_nmi_mask;
	uint8_t m_nmi_enable_reg;
	uint8_t m_nmi_trap;
	uint8_t m_nmi_reset;
	uint16_t m_pac2_index[2];
	uint32_t m_kanji_index;
	uint8_t m_pac2_bank_select;
	uint8_t m_screen_type;
	int m_addr_latch;
	void pasopia_nmi_trap();
	uint8_t m_mux_data;
	u8 m_porta_2;
	bool m_spr_sw;
	emu_timer *m_pio_timer;
	virtual void machine_reset() override;
	void fdc_irq(bool state);
	void draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width);
	void draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width);
	void draw_mixed_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width);

	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_image_device> m_floppy;
	required_device<sn76489a_device> m_sn1;
	required_device<sn76489a_device> m_sn2;
	required_device<palette_device> m_palette;
	required_ioport_array<12> m_keyboard;
	required_device<cassette_image_device> m_cass;
	required_device<speaker_sound_device> m_speaker;
};

#define VDP_CLOCK 14.318181_MHz_XTAL / 16
#define LCD_CLOCK VDP_CLOCK/10

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER( pasopia7_state::pio_timer )
{
	m_pio->port_b_write(keyb_r(generic_space(),0,0xff));
}

VIDEO_START_MEMBER(pasopia7_state,pasopia7)
{
	m_p7_pal = std::make_unique<uint8_t[]>(0x10);
}

void pasopia7_state::draw_cg4_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width)
{
	uint8_t *vram = memregion("vram")->base();
	int x,y,xi,yi;
	int count;

	for(yi=0;yi<8;yi++)
	{
		count = yi;
		for(y=0;y<200;y+=8)
		{
			for(x=0;x<8*width;x+=8)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen_b,pen_r,pen_g,color;

					pen_b = (vram[count+0x0000]>>(7-xi)) & 1;
					pen_r = (vram[count+0x4000]>>(7-xi)) & 1;
					pen_g = 0;//(p7_vram[count+0x8000]>>(7-xi)) & 1;

					color =  pen_g<<2 | pen_r<<1 | pen_b<<0;

					bitmap.pix16(y+yi, x+xi) = m_palette->pen(color);
				}
				count+=8;
			}
		}
	}
}

void pasopia7_state::draw_tv_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width)
{
	uint8_t *vram = memregion("vram")->base();
	uint8_t *gfx_data = memregion("font")->base();
	int x,y,xi,yi;
	int count;

	count = 0x0000;

	for(y=0;y<25;y++)
	{
		for(x=0;x<width;x++)
		{
			int tile = vram[count+0x8000];
			int attr = vram[count+0xc000];
			int color = attr & 7;

			for(yi=0;yi<8;yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen;
					pen = ((gfx_data[tile*8+yi]>>(7-xi)) & 1) ? color : 0;

					bitmap.pix16(y*8+yi, x*8+xi) = m_palette->pen(pen);
				}
			}

			// draw cursor
			if(m_cursor_addr*8 == count)
			{
				int xc,yc,cursor_on;

				cursor_on = 0;
				switch(m_cursor_raster & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(m_screen->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(m_screen->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for(yc=0;yc<(8-(m_cursor_raster & 7));yc++)
					{
						for(xc=0;xc<8;xc++)
						{
							bitmap.pix16(y*8-yc+7, x*8+xc) = m_palette->pen(7);
						}
					}
				}
			}
			count+=8;
		}
	}
}

void pasopia7_state::draw_mixed_screen(bitmap_ind16 &bitmap,const rectangle &cliprect,int width)
{
	uint8_t *vram = memregion("vram")->base();
	uint8_t *gfx_data = memregion("font")->base();
	int x,y,xi,yi;
	int count;

	count = 0x0000;

	for(y=0;y<25;y++)
	{
		for(x=0;x<width;x++)
		{
			int tile = vram[count+0x8000];

			for(yi=0;yi<8;yi++)
			{
				int attr = vram[count+0xc000+yi];

				if(attr & 0x80)
				{
					for(xi=0;xi<8;xi++)
					{
						int pen,pen_b,pen_r,pen_g;

						pen_b = (vram[count+yi+0x0000]>>(7-xi)) & 1;
						pen_r = (vram[count+yi+0x4000]>>(7-xi)) & 1;
						pen_g = (vram[count+yi+0x8000]>>(7-xi)) & 1;

						pen =  pen_g<<2 | pen_r<<1 | pen_b<<0;

						bitmap.pix16(y*8+yi, x*8+xi) = m_palette->pen(pen);
					}
				}
				else
				{
					int color = attr & 7;

					for(xi=0;xi<8;xi++)
					{
						int pen;
						pen = ((gfx_data[tile*8+yi]>>(7-xi)) & 1) ? color : 0;

						bitmap.pix16(y*8+yi, x*8+xi) = m_palette->pen(pen);
					}
				}
			}

			// draw cursor
			if(m_cursor_addr*8 == count)
			{
				int xc,yc,cursor_on;

				cursor_on = 0;
				switch(m_cursor_raster & 0x60)
				{
					case 0x00: cursor_on = 1; break; //always on
					case 0x20: cursor_on = 0; break; //always off
					case 0x40: if(m_screen->frame_number() & 0x10) { cursor_on = 1; } break; //fast blink
					case 0x60: if(m_screen->frame_number() & 0x20) { cursor_on = 1; } break; //slow blink
				}

				if(cursor_on)
				{
					for(yc=0;yc<(8-(m_cursor_raster & 7));yc++)
					{
						for(xc=0;xc<8;xc++)
						{
							bitmap.pix16(y*8-yc+7, x*8+xc) = m_palette->pen(7);
						}
					}
				}
			}

			count+=8;
		}
	}
}

uint32_t pasopia7_state::screen_update_pasopia7(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int width;

	bitmap.fill(m_palette->pen(0), cliprect);

	width = m_x_width ? 80 : 40;

	if(m_gfx_mode)
		draw_mixed_screen(bitmap,cliprect,width);
	else
	{
		draw_cg4_screen(bitmap,cliprect,width);
		draw_tv_screen(bitmap,cliprect,width);
	}

	return 0;
}

READ8_MEMBER( pasopia7_state::vram_r )
{
	uint8_t *vram = memregion("vram")->base();
	uint8_t res;

	if (m_vram_sel == 0)
	{
		uint8_t *work_ram = memregion("maincpu")->base();

		return work_ram[offset+0x8000];
	}

	if (m_pal_sel && (m_plane_reg & 0x70) == 0x00)
		return m_p7_pal[offset & 0xf];

	res = 0xff;

	if ((m_plane_reg & 0x11) == 0x11)
		res &= vram[offset | 0x0000];
	if ((m_plane_reg & 0x22) == 0x22)
		res &= vram[offset | 0x4000];
	if ((m_plane_reg & 0x44) == 0x44)
	{
		res &= vram[offset | 0x8000];
		m_attr_latch = vram[offset | 0xc000] & 0x87;
	}

	return res;
}

WRITE8_MEMBER( pasopia7_state::vram_w )
{
	uint8_t *vram = memregion("vram")->base();

	if (m_vram_sel)
	{
		if (m_pal_sel && (m_plane_reg & 0x70) == 0x00)
		{
			m_p7_pal[offset & 0xf] = data & 0xf;
			return;
		}

		if (m_plane_reg & 0x10)
			vram[(offset & 0x3fff) | 0x0000] = (m_plane_reg & 1) ? data : 0xff;
		if (m_plane_reg & 0x20)
			vram[(offset & 0x3fff) | 0x4000] = (m_plane_reg & 2) ? data : 0xff;
		if (m_plane_reg & 0x40)
		{
			vram[(offset & 0x3fff) | 0x8000] = (m_plane_reg & 4) ? data : 0xff;
			m_attr_latch = m_attr_wrap ? m_attr_latch : m_attr_data;
			vram[(offset & 0x3fff) | 0xc000] = m_attr_latch;
		}
	}
	else
	{
		uint8_t *work_ram = memregion("maincpu")->base();

		work_ram[offset+0x8000] = data;
	}
}

WRITE8_MEMBER( pasopia7_state::pasopia7_memory_ctrl_w )
{
	uint8_t *work_ram = memregion("maincpu")->base();
	uint8_t *basic = memregion("basic")->base();

	switch(data & 3)
	{
		case 0:
		case 3: //select Basic ROM
			membank("bank1")->set_base(basic    + 0x00000);
			membank("bank2")->set_base(basic    + 0x04000);
			break;
		case 1: //select Basic ROM + BIOS ROM
			membank("bank1")->set_base(basic    + 0x00000);
			membank("bank2")->set_base(work_ram + 0x10000);
			break;
		case 2: //select Work RAM
			membank("bank1")->set_base(work_ram + 0x00000);
			membank("bank2")->set_base(work_ram + 0x04000);
			break;
	}

	m_bank_reg = data & 3;
	m_vram_sel = data & 4;
	m_mio_sel = data & 8;

	// bank4 is always RAM

//  printf("%02x\n",m_vram_sel);
}

#if 0
READ8_MEMBER( pasopia7_state::fdc_r )
{
	return machine().rand();
}
#endif


WRITE8_MEMBER( pasopia7_state::pac2_w )
{
	/*
	select register:
	4 = ram1;
	3 = ram2;
	2 = kanji ROM;
	1 = joy;
	anything else is nop
	*/

	if(m_pac2_bank_select == 3 || m_pac2_bank_select == 4)
	{
		switch(offset)
		{
			case 0: m_pac2_index[(m_pac2_bank_select-3) & 1] = (m_pac2_index[(m_pac2_bank_select-3) & 1] & 0x7f00) | (data & 0xff); break;
			case 1: m_pac2_index[(m_pac2_bank_select-3) & 1] = (m_pac2_index[(m_pac2_bank_select-3) & 1] & 0xff) | ((data & 0x7f) << 8); break;
			case 2: // PAC2 RAM write
			{
				uint8_t *pac2_ram;

				pac2_ram = memregion(((m_pac2_bank_select-3) & 1) ? "rampac2" : "rampac1")->base();

				pac2_ram[m_pac2_index[(m_pac2_bank_select-3) & 1]] = data;
			}
		}
	}
	else if(m_pac2_bank_select == 2) // kanji ROM
	{
		switch(offset)
		{
			case 0: m_kanji_index = (m_kanji_index & 0x1ff00) | ((data & 0xff) << 0); break;
			case 1: m_kanji_index = (m_kanji_index & 0x100ff) | ((data & 0xff) << 8); break;
			case 2: m_kanji_index = (m_kanji_index & 0x0ffff) | ((data & 0x01) << 16); break;
		}
	}

	if(offset == 3)
	{
		if(data & 0x80)
		{
			// ...
		}
		else
			m_pac2_bank_select = data & 7;
	}
}

READ8_MEMBER( pasopia7_state::pac2_r )
{
	if(offset == 2)
	{
		if(m_pac2_bank_select == 3 || m_pac2_bank_select == 4)
		{
			uint8_t *pac2_ram;

			pac2_ram = memregion(((m_pac2_bank_select-3) & 1) ? "rampac2" : "rampac1")->base();

			return pac2_ram[m_pac2_index[(m_pac2_bank_select-3) & 1]];
		}
		else if(m_pac2_bank_select == 2)
		{
			uint8_t *kanji_rom = memregion("kanji")->base();

			return kanji_rom[m_kanji_index];
		}
		else
		{
			//printf("RAMPAC bank_select = %02x\n",m_pac2_bank_select);
		}
	}

	return 0xff;
}

/* writes always occurs to the RAM banks, even if the ROMs are selected. */
WRITE8_MEMBER( pasopia7_state::ram_bank_w )
{
	uint8_t *work_ram = memregion("maincpu")->base();

	work_ram[offset] = data;
}

WRITE8_MEMBER( pasopia7_state::pasopia7_6845_w )
{
	if(offset == 0)
	{
		m_addr_latch = data;
		m_crtc->address_w(data);
	}
	else
	{
		/* FIXME: this should be inside the MC6845 core! */
		if(m_addr_latch == 0x0a)
			m_cursor_raster = data;
		if(m_addr_latch == 0x0e)
			m_cursor_addr = ((data<<8) & 0x3f00) | (m_cursor_addr & 0xff);
		else if(m_addr_latch == 0x0f)
			m_cursor_addr = (m_cursor_addr & 0x3f00) | (data & 0xff);

		m_crtc->register_w(data);

		/* double pump the pixel clock if we are in 640 x 200 mode */
		if(m_screen_type == 1) // raster
			m_crtc->set_unscaled_clock( (m_x_width) ? VDP_CLOCK*2 : VDP_CLOCK);
		else // lcd
			m_crtc->set_unscaled_clock( (m_x_width) ? LCD_CLOCK*2 : LCD_CLOCK);
	}
}

void pasopia7_state::pasopia_nmi_trap()
{
	if(m_nmi_enable_reg)
	{
		m_nmi_trap |= 2;

		if(m_nmi_mask == 0)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

READ8_MEMBER( pasopia7_state::pasopia7_fdc_r )
{
	switch(offset)
	{
		case 4: return m_fdc->msr_r();
		case 5: return m_fdc->fifo_r();
		//case 6: bit 7 interrupt bit
	}

	return 0xff;
}

WRITE8_MEMBER( pasopia7_state::pasopia7_fdc_w )
{
	switch(offset)
	{
		case 0: m_fdc->tc_w(false); break;
		case 2: m_fdc->tc_w(true); break;
		case 5: m_fdc->fifo_w(data); break;
		case 6:
			if(data & 0x80)
				m_fdc->reset();
			/* TODO */
			m_floppy->mon_w(data & 0x40 ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


READ8_MEMBER( pasopia7_state::pasopia7_io_r )
{
	if(m_mio_sel)
	{
		address_space &ram_space = m_maincpu->space(AS_PROGRAM);

		m_mio_sel = 0;
			// hack: this is used for reading the keyboard data, we can fake it a little ... (modify fda4)
		return ram_space.read_byte(offset);
	}

	u8 io_port = offset & 0xff; //trim down to 8-bit bus

	if(io_port >= 0x08 && io_port <= 0x0b)
		return m_ppi0->read(io_port & 3);
	else
	if(io_port >= 0x0c && io_port <= 0x0f)
		return m_ppi1->read(io_port & 3);
	else
	if(io_port == 0x10)
		return m_crtc->status_r();
	else
	if(io_port == 0x11)
		return m_crtc->register_r();
	else
	if(io_port >= 0x18 && io_port <= 0x1b)
		return pac2_r(space, io_port & 3);
	else
	if(io_port >= 0x20 && io_port <= 0x23)
	{
		pasopia_nmi_trap();
		return m_ppi2->read(io_port & 3);
	}
	else
	if(io_port >= 0x28 && io_port <= 0x2b)
		return m_ctc->read(io_port & 3);
	else
	if(io_port >= 0x30 && io_port <= 0x33)
		return m_pio->read(io_port & 3);
//  else if(io_port == 0x3a)                    { SN1 }
//  else if(io_port == 0x3b)                    { SN2 }
//  else if(io_port == 0x3c)                    { bankswitch }
	else
//  if(io_port >= 0xe0 && io_port <= 0xe6)
//      return pasopia7_fdc_r(space, offset & 7);
//  else
	{
		logerror("(PC=%06x) Read i/o address %02x\n",m_maincpu->pc(),io_port);
	}

	return 0xff;
}

WRITE8_MEMBER( pasopia7_state::pasopia7_io_w )
{
	if(m_mio_sel)
	{
		address_space &ram_space = m_maincpu->space(AS_PROGRAM);
		m_mio_sel = 0;
		ram_space.write_byte(offset, data);
		return;
	}

	u8 io_port = offset & 0xff; //trim down to 8-bit bus

	if(io_port >= 0x08 && io_port <= 0x0b)
		m_ppi0->write(io_port & 3, data);
	else
	if(io_port >= 0x0c && io_port <= 0x0f)
		m_ppi1->write(io_port & 3, data);
	else
	if(io_port >= 0x10 && io_port <= 0x11)
		pasopia7_6845_w(space, io_port-0x10, data);
	else
	if(io_port >= 0x18 && io_port <= 0x1b)
		pac2_w(space, io_port & 3, data);
	else
	if(io_port >= 0x20 && io_port <= 0x23)
	{
		m_ppi2->write(io_port & 3, data);
		pasopia_nmi_trap();
	}
	else
	if(io_port >= 0x28 && io_port <= 0x2b)
		m_ctc->write(io_port & 3, data);
	else
	if(io_port >= 0x30 && io_port <= 0x33)
		m_pio->write(io_port & 3, data);
	else
	if(io_port == 0x3a)
		m_sn1->write(data);
	else
	if(io_port == 0x3b)
		m_sn2->write(data);
	else
	if(io_port == 0x3c)
		pasopia7_memory_ctrl_w(space,0, data);
	else
	if(io_port >= 0xe0 && io_port <= 0xe6)
		pasopia7_fdc_w(space, offset & 7, data);
	else
	{
		logerror("(PC=%06x) Write i/o address %02x = %02x\n",m_maincpu->pc(),offset,data);
	}
}

void pasopia7_state::pasopia7_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).w(FUNC(pasopia7_state::ram_bank_w));
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x7fff).bankr("bank2");
	map(0x8000, 0xbfff).rw(FUNC(pasopia7_state::vram_r), FUNC(pasopia7_state::vram_w));
	map(0xc000, 0xffff).bankrw("bank4");
}

void pasopia7_state::pasopia7_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(pasopia7_state::pasopia7_io_r), FUNC(pasopia7_state::pasopia7_io_w));
}

static INPUT_PORTS_START( pasopia7 )
	PASOPIA_KEYBOARD
	PORT_MODIFY("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_MODIFY("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("RIGHT")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Label") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INS/DEL") PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB/ESC") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_MODIFY("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME) PORT_NAME("HOME/CLS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Kanji") // guess? key has a Japanese label
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Copy")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Stop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("LEFT")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("UP/DOWN")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("SPACE") PORT_CHAR(' ')
INPUT_PORTS_END

static const gfx_layout p7_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout p7_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static GFXDECODE_START( gfx_pasopia7 )
	GFXDECODE_ENTRY( "font",   0x00000, p7_chars_8x8,    0, 0x10 )
	GFXDECODE_ENTRY( "kanji",  0x00000, p7_chars_16x16,  0, 0x10 )
GFXDECODE_END

READ8_MEMBER( pasopia7_state::keyb_r )
{
	u8 data = 0xff;
	for (u8 j=0; j<3; j++)
		if (BIT(m_mux_data, 4+j))
			for (u8 i=0; i<4; i++)
				if (BIT(m_mux_data, i))
					data &= m_keyboard[j*4+i]->read();

	return data;
}

WRITE8_MEMBER( pasopia7_state::mux_w )
{
	m_mux_data = data;
}

static const z80_daisy_config p7_daisy[] =
{
	{ "ctc" },
	{ "pio" },
//  { "fdc" }, /* TODO */
	{ nullptr }
};

READ8_MEMBER( pasopia7_state::crtc_portb_r )
{
	// --x- ---- vsync bit
	// ---x ---- hardcoded bit, defines if the system screen is raster (1) or LCD (0)
	// ---- x--- disp bit
	uint8_t vdisp = (m_screen->vpos() < (m_screen_type ? 200 : 28)) ? 0x08 : 0x00; //TODO: check LCD vpos trigger
	uint8_t vsync = vdisp ? 0x00 : 0x20;

	return 0x40 | (m_attr_latch & 0x87) | vsync | vdisp | (m_screen_type << 4);
}

WRITE8_MEMBER( pasopia7_state::screen_mode_w )
{
	if(data & 0x5f)
		printf("GFX MODE %02x\n",data);

	m_x_width = data & 0x20;
	m_gfx_mode = data & 0x80;

//  printf("%02x\n",m_gfx_mode);
}

WRITE8_MEMBER( pasopia7_state::plane_reg_w )
{
	//if(data & 0x11)
	//printf("PLANE %02x\n",data);
	m_plane_reg = data;
}

WRITE8_MEMBER( pasopia7_state::video_attr_w )
{
	//printf("VIDEO ATTR %02x | TEXT_PAGE %02x\n",data & 0xf,data & 0x70);
	m_attr_data = (data & 0x7) | ((data & 0x8)<<4);
}

//#include "debugger.h"

WRITE8_MEMBER( pasopia7_state::video_misc_w )
{
	/*
	    --x- ---- blinking
	    ---x ---- attribute wrap
	    ---- x--- pal disable
	    ---- xx-- palette selector (both bits enables this, odd hook-up)
	*/
	//if(data & 2)
	//{
	//  printf("VIDEO MISC %02x\n",data);
	//  machine().debug_break();
	//}
	m_cursor_blink = data & 0x20;
	m_attr_wrap = data & 0x10;
	//m_pal_sel = data & 0x02;
}

WRITE8_MEMBER( pasopia7_state::nmi_mask_w )
{
	/*
	--x- ---- tape motor
	---x ---- data rec out
	---- --x- sound off
	---- ---x reset NMI & trap
	*/
//  printf("SYSTEM MISC %02x\n",data);

	if(data & 1)
	{
		m_nmi_reset &= ~4;
		m_nmi_trap &= ~2;
		//m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE); //guess
	}
	m_cass->output(BIT(data, 4) ? -1.0 : +1.0);
	u8 changed = data ^ m_porta_2;
	m_porta_2 = data;
	if (BIT(changed, 5))
		m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
}

/* TODO: investigate on these. */
READ8_MEMBER( pasopia7_state::unk_r )
{
	return 0xff;//machine().rand();
}

READ8_MEMBER( pasopia7_state::nmi_reg_r )
{
	//printf("C\n");
	return 0xfc | m_bank_reg;//machine().rand();
}

WRITE8_MEMBER( pasopia7_state::nmi_reg_w )
{
	/*
	    x--- ---- NMI mask
	    -x-- ---- NMI enable trap on PPI8255 2 r/w
	*/
	m_nmi_mask = data & 0x80;
	m_nmi_enable_reg = data & 0x40;
}

READ8_MEMBER( pasopia7_state::nmi_porta_r )
{
	return 0xff;
}

READ8_MEMBER( pasopia7_state::nmi_portb_r )
{
	u8 data = (m_cass->input() > +0.04) ? 0x20 : 0;
	return 0xd9 | data | m_nmi_trap | m_nmi_reset;
}

WRITE_LINE_MEMBER( pasopia7_state::speaker_w )
{
	if (state)
	{
		m_spr_sw ^= 1;
		if (BIT(m_mux_data, 7))
			m_speaker->level_w(m_spr_sw);
	}
}

void pasopia7_state::machine_reset()
{
	uint8_t *bios = memregion("maincpu")->base();

	membank("bank1")->set_base(bios + 0x10000);
	membank("bank2")->set_base(bios + 0x10000);
//  membank("bank3")->set_base(bios + 0x10000);
//  membank("bank4")->set_base(bios + 0x10000);

	m_nmi_reset |= 4;
	m_porta_2 = 0xFF;
	m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

// TODO: palette values are mostly likely to be wrong in there
void pasopia7_state::p7_lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa0, 0xa8, 0xa0);

	for (int i = 1; i < 8; i++)
		palette.set_pen_color(i, 0x30, 0x38, 0x10);
}

void pasopia7_state::fdc_irq(bool state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

static void pasopia7_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

void pasopia7_state::p7_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 15.9744_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pasopia7_state::pasopia7_mem);
	m_maincpu->set_addrmap(AS_IO, &pasopia7_state::pasopia7_io);
	m_maincpu->set_daisy_config(p7_daisy);

	/* Audio */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489A(config, m_sn1, 15.9744_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76489A(config, m_sn2, 15.9744_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	Z80CTC(config, m_ctc, 15.9744_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(15.9744_MHz_XTAL / 4);
	m_ctc->set_clk<2>(15.9744_MHz_XTAL / 4);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<1>().set(FUNC(pasopia7_state::speaker_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio, 15.9744_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set(FUNC(pasopia7_state::mux_w));
	m_pio->in_pb_callback().set(FUNC(pasopia7_state::keyb_r));

	I8255(config, m_ppi0);
	m_ppi0->in_pa_callback().set(FUNC(pasopia7_state::unk_r));
	m_ppi0->out_pa_callback().set(FUNC(pasopia7_state::screen_mode_w));
	m_ppi0->in_pb_callback().set(FUNC(pasopia7_state::crtc_portb_r));

	I8255(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(pasopia7_state::plane_reg_w));
	m_ppi1->out_pb_callback().set(FUNC(pasopia7_state::video_attr_w));
	m_ppi1->out_pc_callback().set(FUNC(pasopia7_state::video_misc_w));

	I8255(config, m_ppi2);
	m_ppi2->in_pa_callback().set(FUNC(pasopia7_state::nmi_porta_r));
	m_ppi2->out_pa_callback().set(FUNC(pasopia7_state::nmi_mask_w));
	m_ppi2->in_pb_callback().set(FUNC(pasopia7_state::nmi_portb_r));
	m_ppi2->in_pc_callback().set(FUNC(pasopia7_state::nmi_reg_r));
	m_ppi2->out_pc_callback().set(FUNC(pasopia7_state::nmi_reg_w));

	UPD765A(config, m_fdc, 8'000'000, true, true);
	FLOPPY_CONNECTOR(config, "fdc:0", pasopia7_floppies, "525hd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pasopia7_floppies, "525hd", floppy_image_device::default_floppy_formats);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void pasopia7_state::p7_raster(machine_config &config)
{
	p7_base(config);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 32-1);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update(FUNC(pasopia7_state::screen_update_pasopia7));

	MCFG_VIDEO_START_OVERRIDE(pasopia7_state,pasopia7)

	PALETTE(config, m_palette, palette_device::BRG_3BIT);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia7);

	HD6845S(config, m_crtc, VDP_CLOCK); // HD46505S
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
}


void pasopia7_state::p7_lcd(machine_config &config)
{
	p7_base(config);
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 200-1);
	m_screen->set_screen_update(FUNC(pasopia7_state::screen_update_pasopia7));
	m_screen->set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(pasopia7_state,pasopia7)

	PALETTE(config, m_palette, FUNC(pasopia7_state::p7_lcd_palette), 8);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia7);

	HD6845S(config, m_crtc, LCD_CLOCK); /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
}

/* ROM definition */
ROM_START( pasopia7 )
	ROM_REGION( 0x14000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios.rom", 0x10000, 0x4000, CRC(b8111407) SHA1(ac93ae62db4c67de815f45de98c79cfa1313857d))

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(8a58fab6) SHA1(5e1a91dfb293bca5cf145b0a0c63217f04003ed1))

	ROM_REGION( 0x800, "font", ROMREGION_ERASEFF )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412))

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, CRC(6109e308) SHA1(5c21cf1f241ef1fa0b41009ea41e81771729785f))

	ROM_REGION( 0x8000, "rampac1", ROMREGION_ERASEFF )
//  ROM_LOAD( "rampac1.bin", 0x0000, 0x8000, CRC(0e4f09bd) SHA1(4088906d57e4f6085a75b249a6139a0e2eb531a1) )

	ROM_REGION( 0x8000, "rampac2", ROMREGION_ERASEFF )
//  ROM_LOAD( "rampac2.bin", 0x0000, 0x8000, CRC(0e4f09bd) SHA1(4088906d57e4f6085a75b249a6139a0e2eb531a1) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )
ROM_END

/* using an identical ROMset from now, but the screen type is different */
ROM_START( pasopia7lcd )
	ROM_REGION( 0x14000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios.rom", 0x10000, 0x4000, CRC(b8111407) SHA1(ac93ae62db4c67de815f45de98c79cfa1313857d))

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(8a58fab6) SHA1(5e1a91dfb293bca5cf145b0a0c63217f04003ed1))

	ROM_REGION( 0x800, "font", ROMREGION_ERASEFF )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412))

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji.rom", 0x0000, 0x20000, CRC(6109e308) SHA1(5c21cf1f241ef1fa0b41009ea41e81771729785f))

	ROM_REGION( 0x8000, "rampac1", ROMREGION_ERASEFF )
//  ROM_LOAD( "rampac1.bin", 0x0000, 0x8000, CRC(0e4f09bd) SHA1(4088906d57e4f6085a75b249a6139a0e2eb531a1) )

	ROM_REGION( 0x8000, "rampac2", ROMREGION_ERASEFF )
//  ROM_LOAD( "rampac2.bin", 0x0000, 0x8000, CRC(0e4f09bd) SHA1(4088906d57e4f6085a75b249a6139a0e2eb531a1) )

	ROM_REGION( 0x10000, "vram", ROMREGION_ERASE00 )
ROM_END


void pasopia7_state::init_p7_raster()
{
	m_screen_type = 1;
	m_pio_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pasopia7_state::pio_timer), this));
	m_pio_timer->adjust(attotime::from_hz(5000), 0, attotime::from_hz(5000));
}

void pasopia7_state::init_p7_lcd()
{
	m_screen_type = 0;
	m_pio_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pasopia7_state::pio_timer), this));
	m_pio_timer->adjust(attotime::from_hz(5000), 0, attotime::from_hz(5000));
}


/* Driver */

COMP( 1983, pasopia7,    0,        0, p7_raster, pasopia7, pasopia7_state, init_p7_raster, "Toshiba", "Pasopia 7 (Raster)", MACHINE_NOT_WORKING )
COMP( 1983, pasopia7lcd, pasopia7, 0, p7_lcd,    pasopia7, pasopia7_state, init_p7_lcd,    "Toshiba", "Pasopia 7 (LCD)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
