// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  TeleVideo 990 terminal
  
  Preliminary driver by R. Belmont
  
  H/W:
  68000-P16 CPU (clock unknown, above 10 MHz it outruns the AT keyboard controller)
  16C452 dual 16450 (PC/AT standard) UART + PC-compatible Centronics
  AMI MEGA-KBD-H-Q PS/2 keyboard interface
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

  ASIC registers:
  0x01 = possibly width (80 written at startup)
  0x09 = possibly height (50 written at startup; 49 + the status bar)
  0x16 = cursor X position
  
  Status bar is out of the way of used VRAM at offset 0x36b0.  

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ins8250.h"
#include "machine/8042kbdc.h"
#include "bus/rs232/rs232.h"
#include "machine/nvram.h"

#define UART0_TAG		"ns16450_0"
#define UART1_TAG		"ns16450_1"
#define RS232A_TAG      "rs232a"
#define RS232B_TAG      "rs232b"

class tv990_state : public driver_device
{
public:
	tv990_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vram(*this, "vram"),
		m_fontram(*this, "fontram"),
		m_uart0(*this, UART0_TAG),
		m_uart1(*this, UART1_TAG),
		m_screen(*this, "screen"),
		m_kbdc(*this, "pc_kbdc"),
		m_palette(*this, "palette")
	{
	}

	required_device<m68000_device> m_maincpu;
	required_shared_ptr<UINT16> m_vram;
	required_shared_ptr<UINT16> m_fontram;
	required_device<ns16450_device> m_uart0, m_uart1;
	required_device<screen_device> m_screen;
	required_device<kbdc8042_device> m_kbdc;
	required_device<palette_device> m_palette;

	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	
	DECLARE_READ16_MEMBER(tvi1111_r);
	DECLARE_WRITE16_MEMBER(tvi1111_w);
	DECLARE_READ8_MEMBER(kbdc_r);
	DECLARE_WRITE8_MEMBER(kbdc_w);
	
	WRITE_LINE_MEMBER(uart0_irq);
	WRITE_LINE_MEMBER(uart1_irq);

	INTERRUPT_GEN_MEMBER(vblank);
	DECLARE_INPUT_CHANGED_MEMBER(color);
private:
	UINT16 tvi1111_regs[(0x100/2)+2];
	emu_timer *m_rowtimer;
	int m_rowh, m_width, m_height;
};

INTERRUPT_GEN_MEMBER(tv990_state::vblank)
{
	m_rowtimer->adjust(m_screen->time_until_pos(m_rowh));
	m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	tvi1111_regs[0x1d] |= 4;
}

void tv990_state::machine_start()
{
	m_rowtimer = timer_alloc();
}

void tv990_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_rowtimer->adjust(m_screen->time_until_pos(m_screen->vpos() + m_rowh));
	m_maincpu->set_input_line(M68K_IRQ_6, ASSERT_LINE);
	m_screen->update_now();
}

WRITE_LINE_MEMBER(tv990_state::uart0_irq)
{
	m_maincpu->set_input_line(M68K_IRQ_5, state);
}

WRITE_LINE_MEMBER(tv990_state::uart1_irq)
{
	m_maincpu->set_input_line(M68K_IRQ_4, state);
}

READ16_MEMBER(tv990_state::tvi1111_r)
{
	if (offset == (0x32/2))
	{
		tvi1111_regs[offset] |= 8;	// loop at 109ca wants this set
	}
	else if(offset == 0x1d)
	{
		m_maincpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
	}

	return tvi1111_regs[offset];
}

WRITE16_MEMBER(tv990_state::tvi1111_w)
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
		m_screen->set_visible_area(0, m_width * 16 - 1, 0, m_height * m_rowh - 1);
	}
}

UINT32 tv990_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT8 pixels, pixels2;
	UINT16 *vram = (UINT16 *)m_vram.target();
	UINT8 *fontram = (UINT8 *)m_fontram.target();
	UINT16 *curchar;
	UINT8 *fontptr;
	int miny = cliprect.min_y / m_rowh;
	int maxy = cliprect.max_y / m_rowh;

	bitmap.fill(0, cliprect);

	for (y = miny; y <= maxy; y++)
	{
		int i;
		for(i = 7; i >= 0; i--)
		{
			if(!BIT(tvi1111_regs[0x1f], i))
				continue;

			int starty = tvi1111_regs[i + 0x40] >> 8;
			int endy = tvi1111_regs[i + 0x40] & 0xff;
			if((y < starty) || (y >= endy))
				continue;

			curchar = &vram[tvi1111_regs[i + 0x50]];
			int minx = tvi1111_regs[i + 0x30] >> 8;
			int maxx = tvi1111_regs[i + 0x30] & 0xff;

			if(maxx > m_width)
				maxx = m_width;

			for (x = minx; x < maxx; x++)
			{
				UINT8 chr = curchar[x - minx] >> 8;
				UINT8 attr = curchar[x - minx] & 0xff;
				if((attr & 2) && (m_screen->frame_number() & 32)) // blink rate?
					continue;

				fontptr = (UINT8 *)&fontram[(chr + (attr & 0x40 ? 256 : 0)) * 64];

				UINT32 palette[2];

				int cursor_pos = tvi1111_regs[0x16] + 133;
				if(BIT(tvi1111_regs[0x1b], 0) && (x == (cursor_pos % 134)) && (y == (cursor_pos / 134)))
				{
					UINT8 attrchg;
					if(tvi1111_regs[0x15] & 0xff00) // what does this really mean? it looks like a mask but that doesn't work in 8line char mode
						attrchg = 8;
					else
						attrchg = 4;
					if(!BIT(tvi1111_regs[0x1b], 1))
						attr ^= attrchg;
					else if(m_screen->frame_number() & 32)
						attr ^= attrchg;
				}

				if (attr & 0x4)	// inverse video?
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
					scanline = &bitmap.pix32((y*m_rowh)+chary, (x*16));

					pixels = *fontptr++;
					pixels2 = *fontptr++;
					if((attr & 0x8) && (chary == m_rowh - 1))
					{
						pixels = 0xff;
						pixels2 = 0xff;
					}

					*scanline++ = palette[(pixels>>7)&1];
					*scanline++ = palette[(pixels2>>7)&1];
					*scanline++ = palette[(pixels>>6)&1];
					*scanline++ = palette[(pixels2>>6)&1];
					*scanline++ = palette[(pixels>>5)&1];
					*scanline++ = palette[(pixels2>>5)&1];
					*scanline++ = palette[(pixels>>4)&1];
					*scanline++ = palette[(pixels2>>4)&1];
					*scanline++ = palette[(pixels>>3)&1];
					*scanline++ = palette[(pixels2>>3)&1];
					*scanline++ = palette[(pixels>>2)&1];
					*scanline++ = palette[(pixels2>>2)&1];
					*scanline++ = palette[(pixels>>1)&1];
					*scanline++ = palette[(pixels2>>1)&1];
					*scanline++ = palette[(pixels&1)];
					*scanline++ = palette[(pixels2&1)];
				}
			}
		}
	}

	return 0;
}

READ8_MEMBER(tv990_state::kbdc_r)
{
	if(offset)
		return m_kbdc->data_r(space, 4);
	else
		return m_kbdc->data_r(space, 0);
}

WRITE8_MEMBER(tv990_state::kbdc_w)
{
	if(offset)
		m_kbdc->data_w(space, 4, data);
	else
		m_kbdc->data_w(space, 0, data);
}

static ADDRESS_MAP_START(tv990_mem, AS_PROGRAM, 16, tv990_state)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x060000, 0x063fff) AM_RAM	AM_SHARE("vram") // character/attribute RAM
	AM_RANGE(0X080000, 0X087fff) AM_RAM	AM_SHARE("fontram") // font RAM
	AM_RANGE(0x090000, 0x0900ff) AM_READWRITE(tvi1111_r, tvi1111_w)
	AM_RANGE(0x0a0000, 0x0a000f) AM_DEVREADWRITE8(UART0_TAG, ns16450_device, ins8250_r, ins8250_w, 0x00ff)
	AM_RANGE(0x0a0010, 0x0a001f) AM_DEVREADWRITE8(UART1_TAG, ns16450_device, ins8250_r, ins8250_w, 0x00ff)
	AM_RANGE(0x0b0000, 0x0b0003) AM_READWRITE8(kbdc_r, kbdc_w, 0x00ff)
	AM_RANGE(0x0c0000, 0x0c7fff) AM_RAM	AM_SHARE("nvram")// work RAM
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tv990 )
	PORT_INCLUDE( at_keyboard )
	PORT_START("Screen")
	PORT_CONFNAME( 0x30, 0x00, "Color")
	PORT_CONFSETTING(    0x00, "Green") PORT_CHANGED_MEMBER(DEVICE_SELF, tv990_state, color, nullptr)
	PORT_CONFSETTING(    0x10, "Amber") PORT_CHANGED_MEMBER(DEVICE_SELF, tv990_state, color, nullptr)
	PORT_CONFSETTING(    0x20, "White") PORT_CHANGED_MEMBER(DEVICE_SELF, tv990_state, color, nullptr)
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
			color = rgb_t::green;
			break;
		case 1:
			color = rgb_t::amber;
			break;
		case 2:
			color = rgb_t::white;
			break;
	}
	m_screen->static_set_color(m_screen, color);
}

void tv990_state::machine_reset()
{
	m_rowtimer->adjust(m_screen->time_until_pos(0));
	
	memset(tvi1111_regs, 0, sizeof(tvi1111_regs));
	m_rowh = 16;
	m_width = 80;
	m_height = 50;
}

static MACHINE_CONFIG_START( tv990, tv990_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14967500)	// verified (59.86992/4)
	MCFG_CPU_PROGRAM_MAP(tv990_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tv990_state, vblank)
	
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green)
	MCFG_SCREEN_UPDATE_DRIVER(tv990_state, screen_update)
	MCFG_SCREEN_SIZE(132*16, 50*16)
	MCFG_SCREEN_VISIBLE_AREA(0, (80*16)-1, 0, (50*16)-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")

	MCFG_DEVICE_ADD( UART0_TAG, NS16450, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232A_TAG, rs232_port_device, write_txd))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(tv990_state, uart0_irq))

	MCFG_DEVICE_ADD( UART1_TAG, NS16450, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232B_TAG, rs232_port_device, write_txd))
	MCFG_INS8250_OUT_INT_CB(WRITELINE(tv990_state, uart0_irq))

	MCFG_RS232_PORT_ADD(RS232A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(UART0_TAG, ns16450_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(UART0_TAG, ns16450_device, dcd_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(UART0_TAG, ns16450_device, cts_w))

	MCFG_RS232_PORT_ADD(RS232B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(UART1_TAG, ns16450_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(UART1_TAG, ns16450_device, dcd_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(UART1_TAG, ns16450_device, cts_w))
	
	MCFG_DEVICE_ADD("pc_kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_AT386)
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(INPUTLINE("maincpu", M68K_IRQ_2))

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tv990 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "180003-89_u3.bin", 0x000000, 0x010000, CRC(0465fc55) SHA1(b8874ce54bf2bf4f77664194d2f23c0e4e6ccbe9) ) 
	ROM_LOAD16_BYTE( "180003-90_u4.bin", 0x000001, 0x010000, CRC(fad7d77d) SHA1(f1114a4a07c8b4ffa0323a2e7ce03d82a386f7d3) ) 
ROM_END

/* Driver */
COMP( 1992, tv990, 0, 0, tv990, tv990, driver_device, 0, "TeleVideo", "TeleVideo 990", MACHINE_NO_SOUND)
