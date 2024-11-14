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
#include "pasopia.h"

#include "bus/pasopia/pac2.h"
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


namespace {

class pasopia7_state : public driver_device
{
public:
	pasopia7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_banks(*this, "bank%u", 0U)
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
		, m_pac2(*this, "pac2")
		, m_speaker(*this, "speaker")
		, m_font_rom(*this, "font")
	{ }

	void p7_base(machine_config &config);
	void p7_lcd(machine_config &config);
	void p7_raster(machine_config &config);

	void init_p7_lcd();
	void init_p7_raster();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void memory_ctrl_w(uint8_t data);
	void ram_bank_w(offs_t offset, uint8_t data);
	void pasopia7_6845_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t keyb_r();
	void mux_w(uint8_t data);
	uint8_t crtc_portb_r();
	void screen_mode_w(uint8_t data);
	void plane_reg_w(uint8_t data);
	void video_attr_w(uint8_t data);
	void video_misc_w(uint8_t data);
	void nmi_mask_w(uint8_t data);
	uint8_t unk_r();
	uint8_t nmi_reg_r();
	void nmi_reg_w(uint8_t data);
	uint8_t nmi_porta_r();
	uint8_t nmi_portb_r();
	void speaker_w(int state);
	TIMER_CALLBACK_MEMBER(pio_timer);
	void p7_lcd_palette(palette_device &palette) const;
	MC6845_UPDATE_ROW(update_row);

	void pasopia7_io(address_map &map) ATTR_COLD;
	void pasopia7_mem(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t[]> m_work_ram;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t m_vram_sel = 0;
	uint8_t m_mio_sel = 0;
	std::unique_ptr<uint8_t[]> m_p7_pal;
	uint8_t m_bank_reg = 0;
	uint8_t m_cursor_blink = 0;
	uint8_t m_plane_reg = 0;
	uint8_t m_attr_data = 0;
	uint8_t m_attr_wrap = 0;
	uint8_t m_attr_latch = 0;
	uint8_t m_pal_sel = 0;
	uint8_t m_x_width = 0;
	uint8_t m_gfx_mode = 0;
	uint8_t m_nmi_mask = 0;
	uint8_t m_nmi_enable_reg = 0;
	uint8_t m_nmi_trap = 0;
	uint8_t m_nmi_reset = 0;
	uint8_t m_screen_type = 0;
	void pasopia_nmi_trap();
	uint8_t m_mux_data = 0;
	u8 m_porta_2 = 0;
	bool m_spr_sw = false;
	emu_timer *m_pio_timer = nullptr;
	void fdc_irq(bool state);
	void draw_cg4_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count);
	void draw_tv_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count,int cursor_x);
	void draw_mixed_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count,int cursor_x);

	required_device<z80_device> m_maincpu;
	required_memory_bank_array<2> m_banks;
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
	required_device<pasopia_pac2_slot_device> m_pac2;
	required_device<speaker_sound_device> m_speaker;
	required_region_ptr<uint8_t> m_font_rom;
};

#define VDP_CLOCK 14.318181_MHz_XTAL / 16
#define LCD_CLOCK VDP_CLOCK/10

void pasopia7_state::machine_start()
{
	m_work_ram = std::make_unique<uint8_t[]>(0x10000);
	std::fill(&m_work_ram[0], &m_work_ram[0x10000], 0xff);

	m_vram = make_unique_clear<uint8_t[]>(0x10000);

	uint8_t *work_ram = m_work_ram.get();
	uint8_t *basic = memregion("basic")->base();
	uint8_t *bios = memregion("bios")->base();
	// 0000-3FFF
	m_banks[0]->configure_entry(0, bios);
	m_banks[0]->configure_entry(1, basic);
	m_banks[0]->configure_entry(2, work_ram);
	// 4000-7FFF
	m_banks[1]->configure_entry(0, bios);
	m_banks[1]->configure_entry(1, basic+0x4000);
	m_banks[1]->configure_entry(2, work_ram+0x4000);

	m_banks[0]->set_entry(0);
	m_banks[1]->set_entry(0);
}

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER( pasopia7_state::pio_timer )
{
	m_pio->port_b_write(keyb_r());
}

void pasopia7_state::video_start()
{
	m_p7_pal = std::make_unique<uint8_t[]>(0x10);
}

void pasopia7_state::draw_cg4_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count)
{
	for(int x=0;x<8*width;x+=8)
	{
		for(int xi=0;xi<8;xi++)
		{
			int pen_b = (m_vram[count+yi+0x0000]>>(7-xi)) & 1;
			int pen_r = (m_vram[count+yi+0x4000]>>(7-xi)) & 1;
			int pen_g = 0;//(m_vram[count+yi+0x8000]>>(7-xi)) & 1;

			int color =  pen_g<<2 | pen_r<<1 | pen_b<<0;

			bitmap.pix(y, x+xi) = m_palette->pen(color);
		}
		count+=8;
	}
}

void pasopia7_state::draw_tv_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count,int cursor_x)
{
	for(int x=0;x<width;x++)
	{
		int tile = m_vram[count+0x8000];
		int attr = m_vram[count+0xc000];
		int color = attr & 7;

		for(int xi=0;xi<8;xi++)
		{
			int pen = ((m_font_rom[tile*8+yi]>>(7-xi)) & 1) ? color : 0;

			bitmap.pix(y, x*8+xi) = m_palette->pen(pen);
		}

		// draw cursor
		if(cursor_x == x)
		{
			for(int xc=0;xc<8;xc++)
			{
				bitmap.pix(y, x*8+xc) = m_palette->pen(7);
			}
		}
		count+=8;
	}
}

void pasopia7_state::draw_mixed_line(bitmap_rgb32 &bitmap,int y,int yi,int width,int count,int cursor_x)
{
	for(int x=0;x<width;x++)
	{
		int tile = m_vram[count+0x8000];
		int attr = m_vram[count+0xc000+yi];

		if(attr & 0x80)
		{
			for(int xi=0;xi<8;xi++)
			{
				int pen_b = (m_vram[count+yi+0x0000]>>(7-xi)) & 1;
				int pen_r = (m_vram[count+yi+0x4000]>>(7-xi)) & 1;
				int pen_g = (m_vram[count+yi+0x8000]>>(7-xi)) & 1;

				int pen =  pen_g<<2 | pen_r<<1 | pen_b<<0;

				bitmap.pix(y, x*8+xi) = m_palette->pen(pen);
			}
		}
		else
		{
			int color = attr & 7;

			for(int xi=0;xi<8;xi++)
			{
				int pen = ((m_font_rom[tile*8+yi]>>(7-xi)) & 1) ? color : 0;

				bitmap.pix(y, x*8+xi) = m_palette->pen(pen);
			}
		}

		// draw cursor
		if(cursor_x == x)
		{
			for(int xc=0;xc<8;xc++)
			{
				bitmap.pix(y, x*8+xc) = m_palette->pen(7);
			}
		}

		count+=8;
	}
}

MC6845_UPDATE_ROW(pasopia7_state::update_row)
{
	if(m_gfx_mode)
		draw_mixed_line(bitmap,y,ra,x_count,ma*8,cursor_x);
	else
	{
		draw_cg4_line(bitmap,y,ra,x_count,ma*8);
		draw_tv_line(bitmap,y,ra,x_count,ma*8,cursor_x);
	}
}

uint8_t pasopia7_state::vram_r(offs_t offset)
{
	uint8_t res;

	if (m_vram_sel == 0)
	{
		return m_work_ram[offset+0x8000];
	}

	if (m_pal_sel && (m_plane_reg & 0x70) == 0x00)
		return m_p7_pal[offset & 0xf];

	res = 0xff;

	if ((m_plane_reg & 0x11) == 0x11)
		res &= m_vram[offset | 0x0000];
	if ((m_plane_reg & 0x22) == 0x22)
		res &= m_vram[offset | 0x4000];
	if ((m_plane_reg & 0x44) == 0x44)
	{
		res &= m_vram[offset | 0x8000];
		m_attr_latch = m_vram[offset | 0xc000] & 0x87;
	}

	return res;
}

void pasopia7_state::vram_w(offs_t offset, uint8_t data)
{
	if (m_vram_sel)
	{
		if (m_pal_sel && (m_plane_reg & 0x70) == 0x00)
		{
			m_p7_pal[offset & 0xf] = data & 0xf;
			return;
		}

		if (m_plane_reg & 0x10)
			m_vram[(offset & 0x3fff) | 0x0000] = (m_plane_reg & 1) ? data : 0xff;
		if (m_plane_reg & 0x20)
			m_vram[(offset & 0x3fff) | 0x4000] = (m_plane_reg & 2) ? data : 0xff;
		if (m_plane_reg & 0x40)
		{
			m_vram[(offset & 0x3fff) | 0x8000] = (m_plane_reg & 4) ? data : 0xff;
			m_attr_latch = m_attr_wrap ? m_attr_latch : m_attr_data;
			m_vram[(offset & 0x3fff) | 0xc000] = m_attr_latch;
		}
	}
	else
	{
		m_work_ram[offset+0x8000] = data;
	}
}

void pasopia7_state::memory_ctrl_w(uint8_t data)
{
	switch(data & 3)
	{
		case 0:
		case 3: //select Basic ROM
			m_banks[0]->set_entry(1);
			m_banks[1]->set_entry(1);
			break;
		case 1: //select Basic ROM + BIOS ROM
			m_banks[0]->set_entry(1);
			m_banks[1]->set_entry(0);
			break;
		case 2: //select Work RAM
			m_banks[0]->set_entry(2);
			m_banks[1]->set_entry(2);
			break;
	}

	m_bank_reg = data & 3;
	m_vram_sel = data & 4;
	m_mio_sel = data & 8;

	// bank4 is always RAM

//  printf("%02x\n",m_vram_sel);
}

/* writes always occurs to the RAM banks, even if the ROMs are selected. */
void pasopia7_state::ram_bank_w(offs_t offset, uint8_t data)
{
	m_work_ram[offset] = data;
}

void pasopia7_state::pasopia7_6845_w(offs_t offset, uint8_t data)
{
	if(offset == 0)
	{
		m_crtc->address_w(data);
	}
	else
	{
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

[[maybe_unused]] uint8_t pasopia7_state::fdc_r(offs_t offset)
{
	switch(offset)
	{
		case 4: return m_fdc->msr_r();
		case 5: return m_fdc->fifo_r();
		//case 6: bit 7 interrupt bit
	}

	return 0xff;
}

void pasopia7_state::fdc_w(offs_t offset, uint8_t data)
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


uint8_t pasopia7_state::io_r(offs_t offset)
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
		return m_pac2->read(io_port & 3);
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
//      return fdc_r(offset & 7);
//  else
	{
		logerror("(PC=%06x) Read i/o address %02x\n",m_maincpu->pc(),io_port);
	}

	return 0xff;
}

void pasopia7_state::io_w(offs_t offset, uint8_t data)
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
		pasopia7_6845_w(io_port-0x10, data);
	else
	if(io_port >= 0x18 && io_port <= 0x1b)
		m_pac2->write(io_port & 3, data);
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
		memory_ctrl_w(data);
	else
	if(io_port >= 0xe0 && io_port <= 0xe6)
		fdc_w(offset & 7, data);
	else
	{
		logerror("(PC=%06x) Write i/o address %02x = %02x\n",m_maincpu->pc(),offset,data);
	}
}

void pasopia7_state::pasopia7_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).w(FUNC(pasopia7_state::ram_bank_w));
	map(0x0000, 0x3fff).bankr("bank0");
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0xbfff).rw(FUNC(pasopia7_state::vram_r), FUNC(pasopia7_state::vram_w));
	map(0xc000, 0xffff).ram();
}

void pasopia7_state::pasopia7_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(pasopia7_state::io_r), FUNC(pasopia7_state::io_w));
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

static GFXDECODE_START( gfx_pasopia7 )
	GFXDECODE_ENTRY( "font",   0x00000, p7_chars_8x8,    0, 0x10 )
GFXDECODE_END

uint8_t pasopia7_state::keyb_r()
{
	u8 data = 0xff;
	for (u8 j=0; j<3; j++)
		if (BIT(m_mux_data, 4+j))
			for (u8 i=0; i<4; i++)
				if (BIT(m_mux_data, i))
					data &= m_keyboard[j*4+i]->read();

	return data;
}

void pasopia7_state::mux_w(uint8_t data)
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

uint8_t pasopia7_state::crtc_portb_r()
{
	// --x- ---- vsync bit
	// ---x ---- hardcoded bit, defines if the system screen is raster (1) or LCD (0)
	// ---- x--- disp bit
	uint8_t vdisp = (m_screen->vpos() < (m_screen_type ? 200 : 28)) ? 0x08 : 0x00; //TODO: check LCD vpos trigger
	uint8_t vsync = vdisp ? 0x00 : 0x20;

	return 0x40 | (m_attr_latch & 0x87) | vsync | vdisp | (m_screen_type << 4);
}

void pasopia7_state::screen_mode_w(uint8_t data)
{
	if(data & 0x5f)
		printf("GFX MODE %02x\n",data);

	m_x_width = data & 0x20;
	m_gfx_mode = data & 0x80;

//  printf("%02x\n",m_gfx_mode);
}

void pasopia7_state::plane_reg_w(uint8_t data)
{
	//if(data & 0x11)
	//printf("PLANE %02x\n",data);
	m_plane_reg = data;
}

void pasopia7_state::video_attr_w(uint8_t data)
{
	//printf("VIDEO ATTR %02x | TEXT_PAGE %02x\n",data & 0xf,data & 0x70);
	m_attr_data = (data & 0x7) | ((data & 0x8)<<4);
}

//#include "debugger.h"

void pasopia7_state::video_misc_w(uint8_t data)
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

void pasopia7_state::nmi_mask_w(uint8_t data)
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
uint8_t pasopia7_state::unk_r()
{
	return 0xff;//machine().rand();
}

uint8_t pasopia7_state::nmi_reg_r()
{
	//printf("C\n");
	return 0xfc | m_bank_reg;//machine().rand();
}

void pasopia7_state::nmi_reg_w(uint8_t data)
{
	/*
	    x--- ---- NMI mask
	    -x-- ---- NMI enable trap on PPI8255 2 r/w
	*/
	m_nmi_mask = data & 0x80;
	m_nmi_enable_reg = data & 0x40;
}

uint8_t pasopia7_state::nmi_porta_r()
{
	return 0xff;
}

uint8_t pasopia7_state::nmi_portb_r()
{
	u8 data = (m_cass->input() > +0.04) ? 0x20 : 0;
	return 0xd9 | data | m_nmi_trap | m_nmi_reset;
}

void pasopia7_state::speaker_w(int state)
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
	m_banks[0]->set_entry(0);
	m_banks[1]->set_entry(0);

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

[[maybe_unused]] void pasopia7_state::fdc_irq(bool state)
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
	FLOPPY_CONNECTOR(config, "fdc:0", pasopia7_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pasopia7_floppies, "525hd", floppy_image_device::default_mfm_floppy_formats);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	PASOPIA_PAC2(config, m_pac2, pac2_default_devices, nullptr);
}

void pasopia7_state::p7_raster(machine_config &config)
{
	p7_base(config);
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 32-1);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::BRG_3BIT);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia7);

	HD6845S(config, m_crtc, VDP_CLOCK); // HD46505S
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(pasopia7_state::update_row));
}


void pasopia7_state::p7_lcd(machine_config &config)
{
	p7_base(config);
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 200-1);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(pasopia7_state::p7_lcd_palette), 8);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia7);

	HD6845S(config, m_crtc, LCD_CLOCK); /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(pasopia7_state::update_row));
}

/* ROM definition */
ROM_START( pasopia7 )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "bios.rom", 0x0000, 0x4000, CRC(b8111407) SHA1(ac93ae62db4c67de815f45de98c79cfa1313857d))

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(8a58fab6) SHA1(5e1a91dfb293bca5cf145b0a0c63217f04003ed1))

	ROM_REGION( 0x1000, "font", ROMREGION_ERASEFF )
	ROM_LOAD( "2732.ic146", 0x0000, 0x1000, CRC(aabf66e8) SHA1(1ff5c2c35f07d7481c4c22a453192d9458590eb0))
ROM_END

/* using an identical ROMset from now, but the screen type is different */
ROM_START( pasopia7lcd )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "bios.rom", 0x0000, 0x4000, CRC(b8111407) SHA1(ac93ae62db4c67de815f45de98c79cfa1313857d))

	ROM_REGION( 0x8000, "basic", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x8000, CRC(8a58fab6) SHA1(5e1a91dfb293bca5cf145b0a0c63217f04003ed1))

	ROM_REGION( 0x1000, "font", ROMREGION_ERASEFF )
	ROM_LOAD( "2732.ic146", 0x0000, 0x1000, BAD_DUMP CRC(aabf66e8) SHA1(1ff5c2c35f07d7481c4c22a453192d9458590eb0))
ROM_END


void pasopia7_state::init_p7_raster()
{
	m_screen_type = 1;
	m_pio_timer = timer_alloc(FUNC(pasopia7_state::pio_timer), this);
	m_pio_timer->adjust(attotime::from_hz(5000), 0, attotime::from_hz(5000));
}

void pasopia7_state::init_p7_lcd()
{
	m_screen_type = 0;
	m_pio_timer = timer_alloc(FUNC(pasopia7_state::pio_timer), this);
	m_pio_timer->adjust(attotime::from_hz(5000), 0, attotime::from_hz(5000));
}

} // Anonymous namespace


/* Driver */

COMP( 1983, pasopia7,    0,        0, p7_raster, pasopia7, pasopia7_state, init_p7_raster, "Toshiba", "Pasopia 7 PA7007 (Raster)", MACHINE_NOT_WORKING )
COMP( 1983, pasopia7lcd, pasopia7, 0, p7_lcd,    pasopia7, pasopia7_state, init_p7_lcd,    "Toshiba", "Pasopia 7 PA7007 with PA7170 (LCD)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
