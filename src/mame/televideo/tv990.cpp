// license:BSD-3-Clause
// copyright-holders:R. Belmont, Carl
/***************************************************************************

  TeleVideo 990/995 terminal

  Driver by Carl and R. Belmont
  Thanks to Al Kossow.

  H/W:
  68000-P16 CPU (clock unknown, above 10 MHz it outruns the AT keyboard controller)
  16C452 dual 16450 (PC/AT standard) UART + PC-compatible Centronics (integrated into
         ASIC on 995)
  AMI MEGA-KBD-H-Q PS/2 keyboard interface on 990, PS/2 8042 on 995
  Televideo ASIC marked "134446-00 TVI1111-0 427"
  3x AS7C256 (32K x 8 SRAM)

  IRQs:
  2 = PS/2 keyboard
  3 = Centronics
  4 = UART 1
  5 = UART 0
  6 = VBL (9003b is status, write 3 to 9003b to reset

  Video modes include 80 or 132 wide by 24, 25, 42, 43, 48, or 49 lines high plus an
                      optional status bar
  Modes include TeleVideo 990, 950, and 955, Wyse WY-60, WY-150/120/50+/50, ANSI,
                      DEC VT320/220, VT100/52, SCO Console, and PC TERM.

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/8042kbdc.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "machine/pc_lpt.h"
#include "machine/pckeybrd.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"
#define LPT_TAG         "lpt"

class tv990_state : public driver_device
{
public:
	tv990_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vram(*this, "vram"),
		m_fontram(*this, "fontram"),
		m_uart(*this, "ns16450_%u", 0U),
		m_screen(*this, "screen"),
		m_kbdc(*this, "pc_kbdc"),
		m_palette(*this, "palette"),
		m_beep(*this, "beep")
	{
	}

	void tv990(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(color);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	void tv990_mem(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(trigger_row_irq);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint16_t tvi1111_r(offs_t offset);
	void tvi1111_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t kbdc_r(offs_t offset);
	void kbdc_w(offs_t offset, uint8_t data);

	void uart0_irq(int state);
	void uart1_irq(int state);
	void lpt_irq(int state);
	void vblank_irq(int state);

	required_device<m68000_device> m_maincpu;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_fontram;
	required_device_array<ns16450_device, 2> m_uart;
	required_device<screen_device> m_screen;
	required_device<kbdc8042_device> m_kbdc;
	required_device<palette_device> m_palette;
	required_device<beep_device> m_beep;

	uint16_t tvi1111_regs[(0x100/2)+2];
	emu_timer *m_rowtimer = nullptr;
	int m_rowh = 0;
	int m_width = 0;
	int m_height = 0;
};

void tv990_state::vblank_irq(int state)
{
	if (state)
	{
		m_rowtimer->adjust(m_screen->time_until_pos(m_rowh));
		m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
		tvi1111_regs[0x1d] |= 4;
	}
}

void tv990_state::machine_start()
{
	m_rowtimer = timer_alloc(FUNC(tv990_state::trigger_row_irq), this);

	save_item(NAME(tvi1111_regs));
	save_item(NAME(m_rowh));
	save_item(NAME(m_width));
	save_item(NAME(m_height));
}

TIMER_CALLBACK_MEMBER(tv990_state::trigger_row_irq)
{
	m_rowtimer->adjust(m_screen->time_until_pos(m_screen->vpos() + m_rowh));
	m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	m_screen->update_now();
}

void tv990_state::uart0_irq(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_5, state);
}

void tv990_state::uart1_irq(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_4, state);
}

void tv990_state::lpt_irq(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

uint16_t tv990_state::tvi1111_r(offs_t offset)
{
	if (offset == (0x32/2))
	{
		tvi1111_regs[offset] |= 8;  // loop at 109ca wants this set
	}
	else if(offset == 0x1d)
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}

	return tvi1111_regs[offset];
}

void tv990_state::tvi1111_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
#if 0
	//if ((offset != 0x50) && (offset != 0x68) && (offset != 0x1d) && (offset != 0x1e) && (offset != 0x17) && (offset != 0x1c))
	{
		if (mem_mask == 0x00ff)
		{
			printf("%x (%d) to ASIC @ %x (mask %04x)\n", data & 0xff, data & 0xff, offset, mem_mask);
		}
		else if (mem_mask == 0xff00)
		{
			printf("%x (%d) to ASIC @ %x (mask %04x)\n", data & 0xff, data & 0xff, offset, mem_mask);
		}
		else
		{
			printf("%x (%d) to ASIC @ %x (mask %04x)\n", data, data, offset, mem_mask);
		}
	}
#endif
	COMBINE_DATA(&tvi1111_regs[offset]);
	if((offset == 0x1c) || (offset == 0x10) || (offset == 0x9) || (offset == 0xa))
	{
		m_width = BIT(tvi1111_regs[0x1c], 11) ? 132 : 80;
		m_rowh = (tvi1111_regs[0x10] & 0xff) + 1;
		if(!m_rowh)
			m_rowh = 16;
		m_height = (tvi1111_regs[0xa] - tvi1111_regs[0x9]) / m_rowh;
		// m_height can be 0 or -1 while machine is starting, leading to a crash on a debug build, so we sanitise it.
		if(m_height < 8 || m_height > 99)
			m_height = 0x1a;
		m_screen->set_visible_area(0, m_width * 16 - 1, 0, m_height * m_rowh - 1);
	}
	if(offset == 0x17)
		m_beep->set_state(tvi1111_regs[0x17] & 4 ? ASSERT_LINE : CLEAR_LINE);
}

uint32_t tv990_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t const *const vram = (uint16_t *)m_vram.target();
	uint8_t const *const fontram = (uint8_t *)m_fontram.target();
	int const miny = cliprect.min_y / m_rowh;
	int const maxy = cliprect.max_y / m_rowh;

	bitmap.fill(0, cliprect);

	for (int y = miny; y <= maxy; y++)
	{
		for(int i = 7; i >= 0; i--)
		{
			if(!BIT(tvi1111_regs[0x1f], i))
				continue;

			int const starty = tvi1111_regs[i + 0x40] >> 8;
			int const endy = tvi1111_regs[i + 0x40] & 0xff;
			if((y < starty) || (y >= endy))
				continue;

			uint16_t const row_offset = tvi1111_regs[i + 0x50];
			uint16_t const *curchar = &vram[row_offset];
			int minx = tvi1111_regs[i + 0x30] >> 8;
			int maxx = tvi1111_regs[i + 0x30] & 0xff;

			if(maxx > m_width)
				maxx = m_width;

			uint16_t const cursor_x = tvi1111_regs[0x16] - row_offset;

			for (int x = minx; x < maxx; x++)
			{
				uint8_t chr = curchar[x - minx] >> 8;
				uint8_t attr = curchar[x - minx] & 0xff;
				if((attr & 2) && (m_screen->frame_number() & 32)) // blink rate?
					continue;

				uint8_t const *fontptr = &fontram[(chr + (attr & 0x40 ? 256 : 0)) * 64];

				if (BIT(tvi1111_regs[0x1b], 0) && x == cursor_x)
				{
					uint8_t attrchg;
					if(tvi1111_regs[0x15] & 0xff00) // what does this really mean? it looks like a mask but that doesn't work in 8line char mode
						attrchg = 8;
					else
						attrchg = 4;
					if(!BIT(tvi1111_regs[0x1b], 1))
						attr ^= attrchg;
					else if(m_screen->frame_number() & 32)
						attr ^= attrchg;
				}

				uint32_t palette[2];
				if (attr & 0x4) // inverse video?
				{
					palette[1] = m_palette->pen(0);
					palette[0] = (attr & 0x10) ? m_palette->pen(1) : m_palette->pen(2);
				}
				else
				{
					palette[0] = m_palette->pen(0);
					palette[1] = (attr & 0x10) ? m_palette->pen(1) : m_palette->pen(2);
				}

				for (int chary = 0; chary < m_rowh; chary++)
				{
					uint32_t *scanline = &bitmap.pix((y*m_rowh)+chary, (x*16));

					uint8_t pixels = *fontptr++;
					uint8_t pixels2 = *fontptr++;
					if((attr & 0x8) && (chary == m_rowh - 1))
					{
						pixels = 0xff;
						pixels2 = 0xff;
					}

					*scanline++ = palette[BIT(pixels, 7)];
					*scanline++ = palette[BIT(pixels2, 7)];
					*scanline++ = palette[BIT(pixels, 6)];
					*scanline++ = palette[BIT(pixels2, 6)];
					*scanline++ = palette[BIT(pixels, 5)];
					*scanline++ = palette[BIT(pixels2, 5)];
					*scanline++ = palette[BIT(pixels, 4)];
					*scanline++ = palette[BIT(pixels2, 4)];
					*scanline++ = palette[BIT(pixels, 3)];
					*scanline++ = palette[BIT(pixels2, 3)];
					*scanline++ = palette[BIT(pixels, 2)];
					*scanline++ = palette[BIT(pixels2, 2)];
					*scanline++ = palette[BIT(pixels, 1)];
					*scanline++ = palette[BIT(pixels2, 1)];
					*scanline++ = palette[BIT(pixels, 0)];
					*scanline++ = palette[BIT(pixels2, 0)];
				}
			}
		}
	}

	return 0;
}

uint8_t tv990_state::kbdc_r(offs_t offset)
{
	if(offset)
		return m_kbdc->data_r(4);
	else
		return m_kbdc->data_r(0);
}

void tv990_state::kbdc_w(offs_t offset, uint8_t data)
{
	if(offset)
		m_kbdc->data_w(4, data);
	else
		m_kbdc->data_w(0, data);
}

void tv990_state::tv990_mem(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x060000, 0x06ffff).ram().share("vram"); // character/attribute RAM
	map(0x080000, 0x087fff).ram().share("fontram"); // font RAM
	map(0x090000, 0x0900ff).rw(FUNC(tv990_state::tvi1111_r), FUNC(tv990_state::tvi1111_w));
	map(0x0a0000, 0x0a000f).rw(m_uart[0], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff);
	map(0x0a0010, 0x0a001f).rw(m_uart[1], FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w)).umask16(0x00ff);
	map(0x0a0028, 0x0a002d).rw(LPT_TAG, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write)).umask16(0x00ff);
	map(0x0b0000, 0x0b0003).rw(FUNC(tv990_state::kbdc_r), FUNC(tv990_state::kbdc_w)).umask16(0x00ff);
	map(0x0c0000, 0x0c7fff).ram().share("nvram");// work RAM
}

/* Input ports */
static INPUT_PORTS_START( tv990 )
	PORT_START("Screen")
	PORT_CONFNAME( 0x30, 0x00, "Color") PORT_CHANGED_MEMBER(DEVICE_SELF, tv990_state, color, 0)
	PORT_CONFSETTING(    0x00, "Green")
	PORT_CONFSETTING(    0x10, "Amber")
	PORT_CONFSETTING(    0x20, "White")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(tv990_state::color)
{
	rgb_t color;
	if(newval == oldval)
		return;

	switch(newval)
	{
		case 0:
		default:
			color = rgb_t::green();
			break;
		case 1:
			color = rgb_t::amber();
			break;
		case 2:
			color = rgb_t::white();
			break;
	}
	m_screen->set_color(color);
}

void tv990_state::machine_reset()
{
	m_rowtimer->adjust(m_screen->time_until_pos(0));

	memset(tvi1111_regs, 0, sizeof(tvi1111_regs));
	m_rowh = 16;
	m_width = 80;
	m_height = 50;
}

void tv990_state::device_post_load()
{
	m_screen->set_visible_area(0, m_width * 16 - 1, 0, m_height * m_rowh - 1);
}

void tv990_state::tv990(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14967500);   // verified (59.86992/4)
	m_maincpu->set_addrmap(AS_PROGRAM, &tv990_state::tv990_mem);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(tv990_state::screen_update));
	m_screen->set_size(132*16, 50*16);
	m_screen->set_visarea(0, (80*16)-1, 0, (50*16)-1);
	m_screen->set_refresh_hz(60);
	m_screen->screen_vblank().set(FUNC(tv990_state::vblank_irq));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	NS16450(config, m_uart[0], 3.6864_MHz_XTAL);
	m_uart[0]->out_dtr_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart[0]->out_rts_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_rts));
	m_uart[0]->out_tx_callback().set(RS232A_TAG, FUNC(rs232_port_device::write_txd));
	m_uart[0]->out_int_callback().set(FUNC(tv990_state::uart0_irq));

	NS16450(config, m_uart[1], 3.6864_MHz_XTAL);
	m_uart[1]->out_dtr_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_dtr));
	m_uart[1]->out_rts_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_rts));
	m_uart[1]->out_tx_callback().set(RS232B_TAG, FUNC(rs232_port_device::write_txd));
	m_uart[1]->out_int_callback().set(FUNC(tv990_state::uart1_irq));

	pc_lpt_device &lpt(PC_LPT(config, LPT_TAG));
	lpt.irq_handler().set(FUNC(tv990_state::lpt_irq));

	rs232_port_device &rs232a(RS232_PORT(config, RS232A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_uart[0], FUNC(ns16450_device::rx_w));
	rs232a.dcd_handler().set(m_uart[0], FUNC(ns16450_device::dcd_w));
	rs232a.cts_handler().set(m_uart[0], FUNC(ns16450_device::cts_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_uart[1], FUNC(ns16450_device::rx_w));
	rs232b.dcd_handler().set(m_uart[1], FUNC(ns16450_device::dcd_w));
	rs232b.cts_handler().set(m_uart[1], FUNC(ns16450_device::cts_w));

	KBDC8042(config, m_kbdc);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->input_buffer_full_callback().set_inputline("maincpu", M68K_IRQ_2);
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "mono").front_center();
	BEEP(config, "beep", 1000).add_route(ALL_OUTPUTS, "mono", 1.0); //whats the freq?
}

/* ROM definition */
ROM_START( tv990 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "180003-89_u3.bin", 0x000000, 0x010000, CRC(0465fc55) SHA1(b8874ce54bf2bf4f77664194d2f23c0e4e6ccbe9) )
	ROM_LOAD16_BYTE( "180003-90_u4.bin", 0x000001, 0x010000, CRC(fad7d77d) SHA1(f1114a4a07c8b4ffa0323a2e7ce03d82a386f7d3) )
ROM_END

ROM_START( tv995 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "995-65_u3.bin", 0x000000, 0x020000, CRC(2d71b6fe) SHA1(a2a3406c19308eb9232db319ea8f151949b2ac74) )
	ROM_LOAD16_BYTE( "995-65_u4.bin", 0x000001, 0x020000, CRC(dc002af2) SHA1(9608e7a729c5ac0fc58f673eaf441d2f4f591ec6) )
ROM_END

} // anonymous namespace


/* Driver */
COMP( 1992, tv990, 0, 0, tv990, tv990, tv990_state, empty_init, "TeleVideo", "TeleVideo 990",    MACHINE_SUPPORTS_SAVE )
COMP( 1994, tv995, 0, 0, tv990, tv990, tv990_state, empty_init, "TeleVideo", "TeleVideo 995-65", MACHINE_SUPPORTS_SAVE )
