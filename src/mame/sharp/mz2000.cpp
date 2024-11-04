// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Sharp MZ-2000

    driver by Angelo Salese
    font conversion by Tomasz Slanina

    Basically a simpler version of Sharp MZ-2500

    TODO:
    - cassette interface, basically any program that's bigger than 8kb fails to load;
    - implement remaining video capabilities
    - add 80b compatibility support;
    - Vosque (color): keyboard doesn't work properly;

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/rp5c15.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"
#include "sound/beep.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/2d_dsk.h"
#include "formats/mz_cas.h"


namespace {

#define MASTER_CLOCK 17.73447_MHz_XTAL  / 5  /* TODO: was 4 MHz, but otherwise cassette won't work due of a bug with MZF support ... */


class mz2000_state : public driver_device
{
public:
	mz2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cass(*this, "cassette"),
		m_floppy(nullptr),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mb8877a(*this, "mb8877a"),
		m_floppy0(*this, "mb8877a:0"),
		m_floppy1(*this, "mb8877a:1"),
		m_floppy2(*this, "mb8877a:2"),
		m_floppy3(*this, "mb8877a:3"),
		m_pit8253(*this, "pit"),
		m_beeper(*this, "beeper"),
		m_region_tvram(*this, "tvram"),
		m_region_gvram(*this, "gvram"),
		m_region_chargen(*this, "chargen"),
		m_region_ipl(*this, "ipl"),
		m_region_wram(*this, "wram"),
		m_io_keys(*this, {"KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7", "KEY8", "KEY9", "KEYA", "KEYB", "KEYC", "KEYD", "UNUSED", "UNUSED"}),
		m_io_config(*this, "CONFIG"),
		m_palette(*this, "palette")
	{ }

	void mz2000(machine_config &config);
	void mz80b(machine_config &config);

private:
	static void floppy_formats(format_registration &fr);

	required_device<cassette_image_device> m_cass;

	floppy_image_device *m_floppy;

	uint8_t m_ipl_enable;
	uint8_t m_tvram_enable;
	uint8_t m_gvram_enable;
	uint8_t m_gvram_bank;

	uint8_t m_key_mux;

	uint8_t m_old_portc;
	uint8_t m_width80;
	uint8_t m_tvram_attr;
	uint8_t m_gvram_mask;

	uint8_t m_color_mode;
	uint8_t m_has_fdc;
	uint8_t m_hi_mode;

	uint8_t m_porta_latch;
	uint8_t m_tape_ctrl;
	uint8_t mz2000_ipl_r(offs_t offset);
	uint8_t mz2000_wram_r(offs_t offset);
	void mz2000_wram_w(offs_t offset, uint8_t data);
	uint8_t mz2000_tvram_r(offs_t offset);
	void mz2000_tvram_w(offs_t offset, uint8_t data);
	uint8_t mz2000_gvram_r(offs_t offset);
	void mz2000_gvram_w(offs_t offset, uint8_t data);
	uint8_t mz2000_mem_r(offs_t offset);
	void mz2000_mem_w(offs_t offset, uint8_t data);
	void mz2000_gvram_bank_w(uint8_t data);
	void floppy_select_w(uint8_t data);
	void floppy_side_w(uint8_t data);
	void timer_w(uint8_t data);
	void mz2000_tvram_attr_w(uint8_t data);
	void mz2000_gvram_mask_w(uint8_t data);
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_mz2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	uint8_t mz2000_porta_r();
	uint8_t mz2000_portb_r();
	uint8_t mz2000_portc_r();
	void mz2000_porta_w(uint8_t data);
	void mz2000_portb_w(uint8_t data);
	void mz2000_portc_w(uint8_t data);
	void mz2000_pio1_porta_w(uint8_t data);
	uint8_t mz2000_pio1_portb_r();
	uint8_t mz2000_pio1_porta_r();

	void mz2000_io(address_map &map) ATTR_COLD;
	void mz2000_map(address_map &map) ATTR_COLD;
	void mz80b_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mb8877_device> m_mb8877a;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<pit8253_device> m_pit8253;
	required_device<beep_device> m_beeper;
	required_memory_region m_region_tvram;
	required_memory_region m_region_gvram;
	required_memory_region m_region_chargen;
	required_memory_region m_region_ipl;
	required_memory_region m_region_wram;
	required_ioport_array<16> m_io_keys;
	required_ioport m_io_config;
	required_device<palette_device> m_palette;
};

void mz2000_state::video_start()
{
}

uint32_t mz2000_state::screen_update_mz2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *tvram = m_region_tvram->base();
	uint8_t *gvram = m_region_gvram->base();
	uint8_t *gfx_data = m_region_chargen->base();
	int x,y,xi,yi;
	uint8_t x_size;
	uint32_t count;

	count = 0;

	for(y=0;y<200;y++)
	{
		for(x=0;x<640;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				int pen;
				pen  = ((gvram[count+0x4000] >> (xi)) & 1) ? 1 : 0; //B
				pen |= ((gvram[count+0x8000] >> (xi)) & 1) ? 2 : 0; //R
				pen |= ((gvram[count+0xc000] >> (xi)) & 1) ? 4 : 0; //G
				pen &= m_gvram_mask;

				bitmap.pix(y*2+0, x+xi) = m_palette->pen(pen);
				bitmap.pix(y*2+1, x+xi) = m_palette->pen(pen);
			}
			count++;
		}
	}

	x_size = (m_width80+1)*40;

	for(y=0;y<25;y++)
	{
		for(x=0;x<x_size;x++)
		{
			uint8_t tile = tvram[y*x_size+x];
			uint8_t color = m_tvram_attr & 7;

			for(yi=0;yi<8*(m_hi_mode+1);yi++)
			{
				for(xi=0;xi<8;xi++)
				{
					int pen;
					int res_x,res_y;
					uint16_t tile_offset;

					res_x = x * 8 + xi;
					res_y = y * (8 *(m_hi_mode+1)) + yi;

					if(res_x > 640-1 || res_y > (200*(m_hi_mode+1))-1)
						continue;

					tile_offset = tile*(8*(m_hi_mode+1))+yi + (m_hi_mode * 0x800);

					pen = ((gfx_data[tile_offset] >> (7-xi)) & 1) ? color : -1;

					/* TODO: clean this up */
					if(pen != -1)
					{
						if(m_hi_mode)
						{
							if(m_width80 == 0)
							{
								bitmap.pix(res_y, res_x*2+0) = m_palette->pen(pen);
								bitmap.pix(res_y, res_x*2+1) = m_palette->pen(pen);
							}
							else
							{
								bitmap.pix(res_y, res_x) = m_palette->pen(pen);
							}
						}
						else
						{
							if(m_width80 == 0)
							{
								bitmap.pix(res_y*2+0, res_x*2+0) = m_palette->pen(pen);
								bitmap.pix(res_y*2+0, res_x*2+1) = m_palette->pen(pen);
								bitmap.pix(res_y*2+1, res_x*2+0) = m_palette->pen(pen);
								bitmap.pix(res_y*2+1, res_x*2+1) = m_palette->pen(pen);
							}
							else
							{
								bitmap.pix(res_y*2+0, res_x) = m_palette->pen(pen);
								bitmap.pix(res_y*2+1, res_x) = m_palette->pen(pen);
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

uint8_t mz2000_state::mz2000_ipl_r(offs_t offset)
{
	return m_region_ipl->base()[offset];
}

uint8_t mz2000_state::mz2000_wram_r(offs_t offset)
{
	return m_region_wram->base()[offset];
}

void mz2000_state::mz2000_wram_w(offs_t offset, uint8_t data)
{
	m_region_wram->base()[offset] = data;
}

uint8_t mz2000_state::mz2000_tvram_r(offs_t offset)
{
	return m_region_tvram->base()[offset];
}

void mz2000_state::mz2000_tvram_w(offs_t offset, uint8_t data)
{
	m_region_tvram->base()[offset] = data;
}

uint8_t mz2000_state::mz2000_gvram_r(offs_t offset)
{
	return m_region_gvram->base()[offset+m_gvram_bank*0x4000];
}

void mz2000_state::mz2000_gvram_w(offs_t offset, uint8_t data)
{
	m_region_gvram->base()[offset+m_gvram_bank*0x4000] = data;
}


uint8_t mz2000_state::mz2000_mem_r(offs_t offset)
{
	uint8_t page_mem;

	page_mem = (offset & 0xf000) >> 12;

	if(page_mem == 0 && m_ipl_enable)
		return mz2000_ipl_r(offset & 0xfff);

	if(((page_mem & 8) == 0) && m_ipl_enable == 0) // if ipl is enabled, 0x1000 - 0x7fff accesses to dummy region
		return mz2000_wram_r(offset);

	if(page_mem & 8)
	{
		if(page_mem == 0xd && m_tvram_enable)
			return mz2000_tvram_r(offset & 0xfff);
		else if(page_mem >= 0xc && m_gvram_enable)
			return mz2000_gvram_r(offset & 0x3fff);
		else
		{
			uint16_t wram_mask = (m_ipl_enable) ? 0x7fff : 0xffff;
			return mz2000_wram_r(offset & wram_mask);
		}
	}

	return 0xff;
}

void mz2000_state::mz2000_mem_w(offs_t offset, uint8_t data)
{
	uint8_t page_mem;

	page_mem = (offset & 0xf000) >> 12;

	if((page_mem & 8) == 0 && m_ipl_enable == 0)
		mz2000_wram_w(offset,data);

	if(page_mem & 8)
	{
		if(page_mem == 0xd && m_tvram_enable)
			mz2000_tvram_w(offset & 0xfff,data);
		else if(page_mem >= 0xc && m_gvram_enable)
			mz2000_gvram_w(offset & 0x3fff,data);
		else
		{
			uint16_t wram_mask = (m_ipl_enable) ? 0x7fff : 0xffff;

			mz2000_wram_w(offset & wram_mask,data);
		}
	}
}

void mz2000_state::mz2000_gvram_bank_w(uint8_t data)
{
	m_gvram_bank = data & 3;
}

uint8_t mz2000_state::fdc_r(offs_t offset)
{
	if(m_has_fdc)
		return m_mb8877a->read(offset) ^ 0xff;

	return 0xff;
}

void mz2000_state::fdc_w(offs_t offset, uint8_t data)
{
	if(m_has_fdc)
		m_mb8877a->write(offset, data ^ 0xff);
}

void mz2000_state::floppy_select_w(uint8_t data)
{
	switch (data & 0x03)
	{
	case 0: m_floppy = m_floppy0->get_device(); break;
	case 1: m_floppy = m_floppy1->get_device(); break;
	case 2: m_floppy = m_floppy2->get_device(); break;
	case 3: m_floppy = m_floppy3->get_device(); break;
	}

	m_mb8877a->set_floppy(m_floppy);

	// todo: bit 2 is connected to something too...

	if (m_floppy)
		m_floppy->mon_w(!BIT(data, 7));
}

void mz2000_state::floppy_side_w(uint8_t data)
{
	if (m_floppy)
		m_floppy->ss_w(BIT(data, 0));
}

void mz2000_state::timer_w(uint8_t data)
{
	m_pit8253->write_gate0(1);
	m_pit8253->write_gate1(1);
	m_pit8253->write_gate0(0);
	m_pit8253->write_gate1(0);
	m_pit8253->write_gate0(1);
	m_pit8253->write_gate1(1);
}

void mz2000_state::mz2000_tvram_attr_w(uint8_t data)
{
	m_tvram_attr = data;
}

void mz2000_state::mz2000_gvram_mask_w(uint8_t data)
{
	m_gvram_mask = data;
}

void mz2000_state::mz2000_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(mz2000_state::mz2000_mem_r), FUNC(mz2000_state::mz2000_mem_w));
}

void mz2000_state::mz80b_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xd8, 0xdb).rw(FUNC(mz2000_state::fdc_r), FUNC(mz2000_state::fdc_w));
	map(0xdc, 0xdc).w(FUNC(mz2000_state::floppy_select_w));
	map(0xdd, 0xdd).w(FUNC(mz2000_state::floppy_side_w));
	map(0xe0, 0xe3).rw("i8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe4, 0xe7).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xe8, 0xeb).rw("z80pio_1", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xf0, 0xf3).w(FUNC(mz2000_state::timer_w));
//  map(0xf4, 0xf4).w(FUNC(mz2000_state::vram_bank_w));
}

void mz2000_state::mz2000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	mz80b_io(map);
	map(0xf5, 0xf5).w(FUNC(mz2000_state::mz2000_tvram_attr_w));
	map(0xf6, 0xf6).w(FUNC(mz2000_state::mz2000_gvram_mask_w));
	map(0xf7, 0xf7).w(FUNC(mz2000_state::mz2000_gvram_bank_w));
}


/*
   The \ key is actually directly to the left of the BREAK key; the CLR/HOME and INST/DEL keys sit
   between the BREAK key and the CR key, and the ] key lies directly to the left of CR. The somewhat
   fudged key bindings for this corner of the keyboard approximate those used for other JIS keyboards.
   (The Japanese MZ-80B/MZ-2000 keyboard layout is almost but not quite JIS.)

   For the natural keyboard, GRPH and RVS/KANA are mapped to the left and right ALT keys; this follows
   their positions on the MZ-2500 keyboard. The unshifted INST/DEL functions as a backspace key and
   has been mapped accordingly.
*/

/* Input ports */
static INPUT_PORTS_START( mz80be ) // European keyboard
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_00_PAD) PORT_CHAR(UCHAR_MAMEKEY(00_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_LALT) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('\r') // also "ENT" on keypad
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"/  \u2190  \u2192") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')                 // ← → (arrows)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"A  \u2514")         PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')  // └ (box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"B  \u2663")         PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')  // ♣ (card suit)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"C  \u2665")         PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')  // ♥ (card suit)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"D  \u2502")         PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')  // │ (box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"E  \u2524")         PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')  // ┤ (box drawing)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"F  \u2500")         PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')  // ─ (box drawing)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"G  \u253c")         PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')  // ┼ (box drawing)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"H  \u2551")         PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')  // ║  (box drawing)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"I  \u255e")         PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')  // ╞  (box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"J  \u2550")         PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')  // ═  (box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"K  \u256c")         PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')  // ╬  (box drawing)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"L  \u256b")         PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')  // ╫  (box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"M  £")              PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"N  \u25cb")         PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')  // ○
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"O  \u2568")         PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')  // ╨  (box drawing)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"P  \u2565")         PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')  // ╥ (box drawing)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Q  \u250c")         PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')  // ┌ (box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"R  \u251c")         PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')  // ├ (box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"S  \u2518")         PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')  // ┘ (box drawing)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"T  \u2534")         PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')  // ┴ (box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"U  \u2521")         PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')  // ┡ (box drawing)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"V  \u2666")         PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')  // ♦ (card suit)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"W  \u2510")         PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')  // ┐ (box drawing)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"X  \u2660")         PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')  // ♠ (card suit)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Y  \u252c")         PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')  // ┬ (box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Z  \u256a")         PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')  // ╪ (box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                  PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                  PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"?  \u2191  \u2193") PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('?')                 // ↑ ↓ (arrows)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8".  >  π")           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8",  <  ¥")           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`') // actually between P and ~
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CLR  HOME") PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INST  DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR('\b') PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRPH") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RVS") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYC")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("KEYD")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED)

	PORT_START("UNUSED")
	PORT_BIT(0xff,IP_ACTIVE_LOW,IPT_UNUSED )

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Video Board" )
	PORT_CONFSETTING( 0x00, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
	PORT_CONFNAME( 0x02, 0x02, "Floppy Device" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x02, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x04, 0x04, "High Resolution" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x04, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mz80bj ) // Japanese keyboard (kana, no RVS)
	PORT_INCLUDE( mz80be )

	PORT_MODIFY("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"/  \u2190  \u30e1  \u2192") PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')                 // ← メ → (me, arrows)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"A  \u30c1  \u2514")         PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')  // チ └ (chi, box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"B  \u30b3  \u2663")         PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')  // コ ♣ (ko, card suit)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"C  \u30bd  \u2665")         PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')  // ソ ♥ (so, card suit)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"D  \u30b7  \u2502")         PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')  // シ │ (shi, box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"E  \u30a4  \u2524")         PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')  // イ ┤ (i, box drawing)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"F  \u30cf  \u2500")         PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')  // ハ ─ (ha, box drawing)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"G  \u30ad  \u253c")         PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')  // キ ┼ (ki, box drawing)

	PORT_MODIFY("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"H  \u30af  \u2551")         PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')  // ク ║ (ku, box drawing)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"I  \u30cb  \u255e")         PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')  // ニ ╞ (ni, box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"J  \u30de  \u2550")         PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')  // マ ═ (ma, box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"K  \u30ce  \u256c")         PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')  // ノ ╬ (no, box drawing)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"L  \u30ea  \u256b")         PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')  // リ ╫ (ri, box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"M  \u30e2  ¥")              PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')  // モ (mo)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"N  \u30df  \u25cb")         PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')  // ミ ○ (mi)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"O  \u30e9  \u2568")         PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')  // ラ ╨ (ra, box drawing)

	PORT_MODIFY("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"P  \u30bb  \u2565")         PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')  // セ ╥ (se, box drawing)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Q  \u30bf  \u250c")         PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')  // タ ┌ (ta, box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"R  \u30b9  \u251c")         PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')  // ス ├ (su, box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"S  \u30c8  \u2518")         PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')  // ト ┘ (to, box drawing)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"T  \u30ab  \u2534")         PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')  // カ ┴ (ka, box drawing)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"U  \u30ca  \u2561")         PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')  // ナ ╡ (na, box drawing)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"V  \u30d2  \u2666")         PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')  // ヒ ♦ (hi, card suit)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"W  \u30c6  \u2510")         PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')  // テ ┐ (te, box drawing)

	PORT_MODIFY("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"X  \u30b5  \u2660")         PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')  // サ ♠ (sa, card suit
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Y  \u30f3  \u252c")         PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')  // ン ┬ (n, box drawing)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Z  \u30c4  \u256a")         PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')  // ツ ╪ (tsu, box drawing)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"^  ~  \u30d8")              PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')  // ヘ (he)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\\  |  \u30f2")             PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('|')                 // ヲ (wo)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"?  \u2191  \u30ed  \u2193") PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR('?')                 // ↑ ロ ↓ (ro, arrows)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8".  >  \u30eb  \u3002")      PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')  // ル 。 (ru, full stop)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8",  <  \u30cd  π")           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')  // ネ (ne)

	PORT_MODIFY("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"0  _  \u30ef")              PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('_')  // ワ (wa)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"1  !  \u30cc")              PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')  // ヌ (nu)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"2  \"  \u30d5")             PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')  // フ (fu)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"3  #  \u30a2")              PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')  // ア (a)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"4  $  \u30a6")              PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')  // ウ (u)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"5  %  \u30a8")              PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')  // エ (e)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"6  &  \u30aa")              PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')  // オ (o)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"7  \'  \u30e4")             PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'') // ヤ (ya)

	PORT_MODIFY("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"8  (  \u30e6")              PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')  // ユ (yu
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"9  )  \u30e8")              PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')  // ヨ (yo
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8":  *  \u30b1")              PORT_CODE(KEYCODE_COLON)      PORT_CHAR(':') PORT_CHAR('*')  // ケ (ke
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8";  +  \u30ec")              PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(';') PORT_CHAR('+')  // レ (re
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"-  =  \u30db")              PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')  // ホ (ho
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"@  `  \u309b")              PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@') PORT_CHAR('`')  // ゛ (dakuten)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"[  {  \u309c  \u300c")      PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')  // ゜ 「 (handakuten, bracket)

	PORT_MODIFY("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"]  }  \u30e0  \u300d")      PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']') PORT_CHAR('}')  // ム 」 (mu, bracket)

	PORT_MODIFY("KEYB")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u30ab\u30ca")              PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(RALT)) // カナ (kana)
INPUT_PORTS_END


void mz2000_state::machine_reset()
{
	m_ipl_enable = 1;
	m_tvram_enable = 0;
	m_gvram_enable = 0;

	m_beeper->set_state(0);

	m_color_mode = m_io_config->read() & 1;
	m_has_fdc = (m_io_config->read() & 2) >> 1;
	m_hi_mode = (m_io_config->read() & 4) >> 2;
}


static const gfx_layout mz2000_charlayout_8 =
{
	8, 8,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout mz2000_charlayout_16 =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	8*16
};

static GFXDECODE_START( gfx_mz2000 )
	GFXDECODE_ENTRY( "chargen", 0x0000, mz2000_charlayout_8, 0, 1 )
	GFXDECODE_ENTRY( "chargen", 0x0800, mz2000_charlayout_16, 0, 1 )
GFXDECODE_END

uint8_t mz2000_state::mz2000_porta_r()
{
	printf("A R\n");
	return 0xff;
}

uint8_t mz2000_state::mz2000_portb_r()
{
	/*
	x--- ---- break key
	-x-- ---- read tape data
	--x- ---- no tape signal
	---x ---- no tape write signal
	---- x--- end of tape reached
	---- ---x "blank" control
	*/
	uint8_t res = 0x80;

	if(m_cass->get_image() != nullptr)
	{
		res |= (m_cass->input() > 0.0038) ? 0x40 : 0x00;
		res |= ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY) ? 0x00 : 0x20;
		res |= (m_cass->get_position() >= m_cass->get_length()) ? 0x08 : 0x00;
	}
	else
		res |= 0x20;

	res |= (m_screen->vblank()) ? 0x00 : 0x01;

	return res;
}

uint8_t mz2000_state::mz2000_portc_r()
{
	printf("C R\n");
	return 0xff;
}

void mz2000_state::mz2000_porta_w(uint8_t data)
{
	/*
	These are enabled thru a 0->1 transition
	x--- ---- tape "APSS"
	-x-- ---- tape "APLAY"
	--x- ---- tape "AREW"
	---x ---- reverse video
	---- x--- tape stop
	---- -x-- tape play
	---- --x- tape ff
	---- ---x tape rewind
	*/

	if((m_tape_ctrl & 0x80) == 0 && data & 0x80)
	{
		//printf("Tape APSS control\n");
	}

	if((m_tape_ctrl & 0x40) == 0 && data & 0x40)
	{
		//printf("Tape APLAY control\n");
	}

	if((m_tape_ctrl & 0x20) == 0 && data & 0x20)
	{
		//printf("Tape AREW control\n");
	}

	if((m_tape_ctrl & 0x10) == 0 && data & 0x10)
	{
		//printf("reverse video control\n");
	}

	if((m_tape_ctrl & 0x08) == 0 && data & 0x08) // stop
	{
		m_cass->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		m_cass->change_state(CASSETTE_STOPPED,CASSETTE_MASK_UISTATE);
	}

	if((m_tape_ctrl & 0x04) == 0 && data & 0x04) // play
	{
		m_cass->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
		m_cass->change_state(CASSETTE_PLAY,CASSETTE_MASK_UISTATE);
	}

	if((m_tape_ctrl & 0x02) == 0 && data & 0x02)
	{
		//printf("Tape FF control\n");
	}

	if((m_tape_ctrl & 0x01) == 0 && data & 0x01)
	{
		//printf("Tape Rewind control\n");
	}

	m_tape_ctrl = data;
}

void mz2000_state::mz2000_portb_w(uint8_t data)
{
	//printf("B W %02x\n",data);

	// ...
}

void mz2000_state::mz2000_portc_w(uint8_t data)
{
	/*
	    x--- ---- tape data write
	    -x-- ---- tape rec
	    --x- ---- tape ?
	    ---x ---- tape open
	    ---- x--- 0->1 transition = IPL reset
	    ---- -x-- beeper state
	    ---- --x- 0->1 transition = Work RAM reset
	*/
	//printf("C W %02x\n",data);

	if(((m_old_portc & 8) == 0) && data & 8)
		m_ipl_enable = 1;

	if(((m_old_portc & 2) == 0) && data & 2)
	{
		m_ipl_enable = 0;
		/* correct? */
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	}

	m_beeper->set_state(data & 0x04);

	m_old_portc = data;
}

void mz2000_state::mz2000_pio1_porta_w(uint8_t data)
{
	m_tvram_enable = ((data & 0xc0) == 0xc0);
	m_gvram_enable = ((data & 0xc0) == 0x80);
	m_width80 = ((data & 0x20) >> 5);
	m_key_mux = data & 0x1f;

	m_porta_latch = data;
}

uint8_t mz2000_state::mz2000_pio1_portb_r()
{
	if(((m_key_mux & 0x10) == 0x00) || ((m_key_mux & 0x0f) == 0x0f)) //status read
	{
		int res,i;

		res = 0xff;
		for(i=0;i<0xe;i++)
			res &= m_io_keys[i]->read();

		return res;
	}

	return m_io_keys[m_key_mux & 0xf]->read();
}

uint8_t mz2000_state::mz2000_pio1_porta_r()
{
	return m_porta_latch;
}


void mz2000_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_2D_FORMAT);
}

static void mz2000_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_525_DD);
}


void mz2000_state::mz2000(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mz2000_state::mz2000_map);
	m_maincpu->set_addrmap(AS_IO, &mz2000_state::mz2000_io);

	i8255_device &ppi(I8255(config, "i8255_0"));
	ppi.in_pa_callback().set(FUNC(mz2000_state::mz2000_porta_r));
	ppi.out_pa_callback().set(FUNC(mz2000_state::mz2000_porta_w));
	ppi.in_pb_callback().set(FUNC(mz2000_state::mz2000_portb_r));
	ppi.out_pb_callback().set(FUNC(mz2000_state::mz2000_portb_w));
	ppi.in_pc_callback().set(FUNC(mz2000_state::mz2000_portc_r));
	ppi.out_pc_callback().set(FUNC(mz2000_state::mz2000_portc_w));

	z80pio_device& pio(Z80PIO(config, "z80pio_1", MASTER_CLOCK));
	pio.in_pa_callback().set(FUNC(mz2000_state::mz2000_pio1_porta_r));
	pio.out_pa_callback().set(FUNC(mz2000_state::mz2000_pio1_porta_w));
	pio.in_pb_callback().set(FUNC(mz2000_state::mz2000_pio1_portb_r));

	/* TODO: clocks aren't known */
	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(31250);
	m_pit8253->set_clk<1>(31250); /* needed by "Art Magic" to boot */
	m_pit8253->set_clk<2>(31250);

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 4096).add_route(ALL_OUTPUTS,"mono",0.15);

	MB8877(config, m_mb8877a, 1_MHz_XTAL);

	FLOPPY_CONNECTOR(config, "mb8877a:0", mz2000_floppies, "dd", mz2000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "mb8877a:1", mz2000_floppies, "dd", mz2000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "mb8877a:2", mz2000_floppies, "dd", mz2000_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "mb8877a:3", mz2000_floppies, "dd", mz2000_state::floppy_formats);

	SOFTWARE_LIST(config, "flop_list").set_original("mz2000_flop");

	CASSETTE(config, m_cass);
	m_cass->set_formats(mz700_cassette_formats);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("mz_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("mz2000_cass");
	SOFTWARE_LIST(config, "cass_list2").set_original("mz2200_cass");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640-1, 0, 400-1);
	m_screen->set_screen_update(FUNC(mz2000_state::screen_update_mz2000));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mz2000);
	PALETTE(config, m_palette, palette_device::BRG_3BIT);
}

void mz2000_state::mz80b(machine_config &config)
{
	mz2000(config);
	m_maincpu->set_addrmap(AS_IO, &mz2000_state::mz80b_io);
}




ROM_START( mz80b )
	ROM_REGION( 0x1000, "ipl", 0 )
	ROM_LOAD( "ipl.rom",  0x0000, 0x0800, CRC(80beeec0) SHA1(d2b8167cc77ad023a807198993cb5e7a94c9e19e) )

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, "chargen", 0 )
	ROM_LOAD( "mzfont.rom", 0x0000, 0x0800, CRC(0631efc3) SHA1(99b206af5c9845995733d877e9e93e9681b982a8) )
ROM_END

ROM_START( mz2000 )
	ROM_REGION( 0x1000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "mz20ipl.bin",0x0000, 0x0800, CRC(d7ccf37f) SHA1(692814ffc2cf50fa8bf9e30c96ebe4a9ee536a86))

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, "chargen", 0 )
//  ROM_LOAD( "mzfont.rom", 0x0000, 0x0800, BAD_DUMP CRC(0631efc3) SHA1(99b206af5c9845995733d877e9e93e9681b982a8) ) //original has JP characters
	/* these are hand-crafted roms, converted from bmps floating around the net */
	ROM_LOAD( "font.bin",    0x0000, 0x0800, BAD_DUMP CRC(6ae6ce8e) SHA1(6adcdab9e4647429dd8deb73146264746b5eccda) )
	ROM_LOAD( "font400.bin", 0x0800, 0x1000, BAD_DUMP CRC(56c5d2bc) SHA1(fea655ff5eedacf8978fa3c185485db44376e24d) )
ROM_END

ROM_START( mz2200 )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "mz2200ipl.bin", 0x0000, 0x0800, CRC(476801e8) SHA1(6b1f0620945c5492475ea1694bd09a3fcf88549d) )

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "tvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x10000, "gvram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1800, "chargen", 0 )
//  ROM_LOAD( "mzfont.rom", 0x0000, 0x0800, BAD_DUMP CRC(0631efc3) SHA1(99b206af5c9845995733d877e9e93e9681b982a8) ) //original has JP characters
	/* these are hand-crafted roms, converted from bmps floating around the net */
	ROM_LOAD( "font.bin",    0x0000, 0x0800, BAD_DUMP CRC(6ae6ce8e) SHA1(6adcdab9e4647429dd8deb73146264746b5eccda) )
	ROM_LOAD( "font400.bin", 0x0800, 0x1000, BAD_DUMP CRC(56c5d2bc) SHA1(fea655ff5eedacf8978fa3c185485db44376e24d) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1981, mz80b,  0,      0,      mz80b,   mz80be, mz2000_state, empty_init, "Sharp", "MZ-80B",  MACHINE_NOT_WORKING )
COMP( 1982, mz2000, 0,      0,      mz2000,  mz80bj, mz2000_state, empty_init, "Sharp", "MZ-2000", MACHINE_NOT_WORKING )
COMP( 1982, mz2200, mz2000, 0,      mz2000,  mz80bj, mz2000_state, empty_init, "Sharp", "MZ-2200", MACHINE_NOT_WORKING )
