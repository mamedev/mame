// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Robbbert, R. Belmont, Carl
/***************************************************************************

    Triumph-Adler (or Royal) Alphatronic PC

    Driver by Barry Rodewald, Robbbert, R. Belmont and Carl

    z80 + HD46505SP as a CRTC

    Other chips: 8251, 8257, 8259
    Crystals: 16MHz, 17.73447MHz
    Floppy format: 13cm, 2 sides, 40 tracks, 16 sectors, 256 bytes = 320kb.
    FDC (uPD765) is in a plug-in module, there is no ROM on the module.

    A configuration switch determines if the FDC is present.

    Has a socket for monochrome (to the standard amber monitor),
    and another for RGB. We emulate this with a configuration switch.

    ToDo:
    - Newer ROM set from Team Europe and try to work out the graphics expansion

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/td0_dsk.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/bankdev.h"
#include "machine/upd765.h"
#include "machine/i8257.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "sound/beep.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


class alphatro_state : public driver_device
{
public:
	alphatro_state(const machine_config &mconfig, device_type type, const char *tag, bool is_ntsc)
		: driver_device(mconfig, type, tag)
		, m_is_ntsc(is_ntsc)
		, m_ram(*this, RAM_TAG)
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_usart(*this, "usart")
		, m_cass(*this, "cassette")
		, m_beep(*this, "beeper")
		, m_palette(*this, "palette")
		, m_lowbank(*this, "lowbank")
		, m_cartbank(*this, "cartbank")
		, m_monbank(*this, "monbank")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_dmac(*this, "dmac")
		, m_config(*this, "CONFIG")
		, m_cart(*this, "cartslot")
	{ }

	void alphatro(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(alphatro_break);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	DECLARE_READ8_MEMBER (ram0000_r);
	DECLARE_WRITE8_MEMBER(ram0000_w);
	DECLARE_READ8_MEMBER (ram6000_r);
	DECLARE_WRITE8_MEMBER(ram6000_w);
	DECLARE_READ8_MEMBER (rama000_r);
	DECLARE_WRITE8_MEMBER(rama000_w);
	DECLARE_READ8_MEMBER (rame000_r);
	DECLARE_WRITE8_MEMBER(rame000_w);
	uint8_t port10_r();
	void port10_w(uint8_t data);
	void port20_w(uint8_t data);
	uint8_t port30_r();
	void port30_w(uint8_t data);
	uint8_t portf0_r();
	void portf0_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	void alphatro_palette(palette_device &palette) const;
	DECLARE_WRITE_LINE_MEMBER(kansas_r);
	DECLARE_WRITE_LINE_MEMBER(kansas_w);
	MC6845_UPDATE_ROW(crtc_update_row);

	image_init_result load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load) { return load_cart(image, m_cart); }
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void alphatro_io(address_map &map);
	void alphatro_map(address_map &map);
	void cartbank_map(address_map &map);
	void monbank_map(address_map &map);
	void rombank_map(address_map &map);
	void update_banking();

	const bool m_is_ntsc;
	uint8_t *m_ram_ptr;
	required_device<ram_device> m_ram;
	required_shared_ptr<u8> m_p_videoram;
	u8 m_flashcnt;
	u8 m_cass_data[4];
	u8 m_port_10, m_port_20, m_port_30, m_port_f0;
	bool m_cassbit;
	bool m_cassold, m_fdc_irq;
	required_region_ptr<u8> m_p_chargen;
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_usart;
	required_device<cassette_image_device> m_cass;
	required_device<beep_device> m_beep;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_lowbank, m_cartbank, m_monbank;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<i8257_device> m_dmac;
	required_ioport m_config;
	required_device<generic_slot_device> m_cart;
};

class alphatro_pal_state : public alphatro_state
{
public:
	alphatro_pal_state(const machine_config &mconfig, device_type type, const char *tag)
		: alphatro_state(mconfig, type, tag, false)
	{ }
};

class alphatro_ntsc_state : public alphatro_state
{
public:
	alphatro_ntsc_state(const machine_config &mconfig, device_type type, const char *tag)
		: alphatro_state(mconfig, type, tag, true)
	{ }
};

void alphatro_state::update_banking()
{
	if (m_port_10 & 0x80)   // RAM at 0000?
	{
		m_lowbank->set_bank(1);
	}
	else
	{
		m_lowbank->set_bank(0);
	}

	if (m_port_10 & 0x40)   // ROM cartridge at A000
	{
		m_cartbank->set_bank(0);
	}
	else
	{
		m_cartbank->set_bank(1);
	}

	if (m_port_20 & 0x08)    // VRAM at F000?
	{
		m_monbank->set_bank(0);
	}
	else
	{
		if (m_port_20 & 0x40)   // IPL or Monitor at F000?
		{
			m_monbank->set_bank(2);
		}
		else
		{
			m_monbank->set_bank(1);
		}
	}
}

READ8_MEMBER (alphatro_state::ram0000_r)
{
	if (offset < 0xf000)
	{
		return m_ram_ptr[offset];
	}

	return m_p_videoram[offset & 0xfff];
}

WRITE8_MEMBER(alphatro_state::ram0000_w)
{

	if (offset < 0xf000)
	{
		m_ram_ptr[offset] = data;
	}
	else
	{
		m_p_videoram[offset & 0xfff] = data;
	}
}

READ8_MEMBER (alphatro_state::ram6000_r) { return m_ram_ptr[offset+0x6000]; }
WRITE8_MEMBER(alphatro_state::ram6000_w) { m_ram_ptr[offset+0x6000] = data; }
READ8_MEMBER (alphatro_state::rama000_r) { return m_ram_ptr[offset+0xa000]; }
WRITE8_MEMBER(alphatro_state::rama000_w) { m_ram_ptr[offset+0xa000] = data; }
READ8_MEMBER (alphatro_state::rame000_r) { return m_ram_ptr[offset+0xe000]; }
WRITE8_MEMBER(alphatro_state::rame000_w) { m_ram_ptr[offset+0xe000] = data; }

uint8_t alphatro_state::port10_r()
{
// Bit 0 -> 1 = FDC is installed, 0 = not
// Bit 1 -> 1 = Graphic Board is installed, 0 = not
// Bits 2-4 = Country select: 0 = Intl, 1 = German, 2 = US
// Bit 5 -> 1 = BASIC LPRINT is RS-232, 0 = BASIC LPRINT is Centronics
// Bit 6 -> 1 = NTSC, 0 = PAL
// Bit 7 -> 1 = vblank or hblank, 0 = active display area

	u8 retval = m_is_ntsc ? 0x40 : 0x00;

	// we'll get "FDC present" and "graphics expansion present" from the config switches
	retval |= (m_config->read() & 3);

	if (!m_crtc->de_r())
	{
		retval |= 0x80;
	}

	return retval;
}

void alphatro_state::port10_w(uint8_t data)
{
// Bit 0 -> 0 = 40 cols; 1 = 80 cols
// Bit 1 -> 0 = display enable, 1 = display inhibit
// Bit 2 -> 0 = USART is connected to cassette, 1 = RS232 port
// Bit 3 -> 0 = cassette motor off, 1 = cassette motor on
// Bit 4 -> 0 = beeper off, 1 = beeper on
// Bit 5 -> always 0
// Bit 6 -> 1 = select ROM pack at A000, 0 = RAM at A000
// Bit 7 -> 0 = ROM enabled at 0, 1 = RAM enabled

	if (BIT(data ^ m_port_10, 0))
	{
		if (BIT(data, 0))
			m_crtc->set_unscaled_clock(16_MHz_XTAL / 8);
		else if (m_is_ntsc || system_bios() == 3) // kludge for bios 2, which expects a NTSC clock even for ~50 Hz video
			m_crtc->set_unscaled_clock(14.318181_MHz_XTAL / 16);
		else
			m_crtc->set_unscaled_clock(17.73447_MHz_XTAL / 16);
	}

	m_port_10 = data;

	m_beep->set_state(BIT(data, 4));

	m_cass->change_state( BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (BIT(data,2))
		m_cassbit = 1;

	update_banking();
}

void alphatro_state::port20_w(uint8_t data)
{
// Bit 0 -> 0 = CRTC reset release, 1 = CRTC reset enable
// Bit 1 -> 0 = Centronics reset release, 1 = Centronics reset enable
// Bit 2 -> 0 = no Centronics strobe, 1 = Centronics strobe active
// Bit 3 -> 0 = Monitor ROM at F000, 1 = VRAM at F000
// Bit 4 -> 0 = Graphic LED off, 1 = Graphic LED on
// Bit 5 -> 0 = Shift Lock LED off, 1 = Shift Lock LED on
// Bit 6 -> 0 = Lower 4K of Monitor at F000, 1 = Upper 4K of Monitor at F000
// Bit 7 -> N/A

	m_port_20 = data;

	update_banking();
}

uint8_t alphatro_state::port30_r()
{
// Bit 0 -> SIOC
// Bit 1 -> 1 = vsync, 0 = not
// Bit 2 -> 1 = Centronics ACK, 0 = not
// Bit 3 -> 1 = Centronics BUSY, 0 = not

	u8 retval = 0;

	if (m_crtc->vsync_r()) retval |= 0x02;

	return retval;
}

void alphatro_state::port30_w(uint8_t data)
{
	m_port_30 = data;
}

uint8_t alphatro_state::portf0_r()
{
	return m_fdc_irq << 6;
}

void alphatro_state::portf0_w(uint8_t data)
{
	if ((data & 0x1) && !(m_port_f0))
	{
		m_fdc->reset();

		for (auto &con : m_floppy)
		{
			floppy_image_device *floppy = con->get_device();
			if (floppy)
			{
				floppy->mon_w(0);
				m_fdc->set_rate(250000);
			}
		}
	}

	m_port_f0 = data;
}


MC6845_UPDATE_ROW( alphatro_state::crtc_update_row )
{
	const rgb_t *pens = m_palette->palette()->entry_list_raw();
	bool palette = BIT(m_config->read(), 5);
	if (y==0) m_flashcnt++;
	bool inv;
	u8 chr,gfx,attr,bg,fg;
	u16 mem,x;
	u32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		inv = (x == cursor_x);
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		attr = m_p_videoram[mem | 0x800];
		bg = (palette) ? 8 : attr & 7; // amber or RGB
		fg = (palette) ? 0 : (attr & 0x38) >> 3;

		if (BIT(attr, 7)) // reverse video
		{
			inv ^= 1;
			chr &= 0x7f;
		}

		if (BIT(attr, 6) & BIT(m_flashcnt, 4)) // flashing
		{
			inv ^= 1;
		}

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | ra];

		if (inv)
		{
			u8 t = bg;
			bg = fg;
			fg = t;
		}

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7) ? fg : bg];
		*p++ = pens[BIT(gfx, 6) ? fg : bg];
		*p++ = pens[BIT(gfx, 5) ? fg : bg];
		*p++ = pens[BIT(gfx, 4) ? fg : bg];
		*p++ = pens[BIT(gfx, 3) ? fg : bg];
		*p++ = pens[BIT(gfx, 2) ? fg : bg];
		*p++ = pens[BIT(gfx, 1) ? fg : bg];
		*p++ = pens[BIT(gfx, 0) ? fg : bg];
	}
}

INPUT_CHANGED_MEMBER( alphatro_state::alphatro_break )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

void alphatro_state::alphatro_map(address_map &map)
{
	map(0x0000, 0x5fff).m(m_lowbank, FUNC(address_map_bank_device::amap8));
	map(0x6000, 0x9fff).rw(FUNC(alphatro_state::ram6000_r), FUNC(alphatro_state::ram6000_w));
	map(0xa000, 0xdfff).m("cartbank", FUNC(address_map_bank_device::amap8));
	map(0xe000, 0xefff).rw(FUNC(alphatro_state::rame000_r), FUNC(alphatro_state::rame000_w));
	map(0xf000, 0xffff).m("monbank", FUNC(address_map_bank_device::amap8));

}

void alphatro_state::rombank_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("roms", 0x0000).w(FUNC(alphatro_state::ram0000_w));
	map(0x6000, 0xbfff).rw(FUNC(alphatro_state::ram0000_r), FUNC(alphatro_state::ram0000_w));
}

void alphatro_state::cartbank_map(address_map &map)
{
	map(0x0000, 0x3fff).r(m_cart, FUNC(generic_slot_device::read_rom)).w(FUNC(alphatro_state::rama000_w));
	map(0x4000, 0x7fff).rw(FUNC(alphatro_state::rama000_r), FUNC(alphatro_state::rama000_w));
}

void alphatro_state::monbank_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("videoram");
	map(0x1000, 0x1fff).rom().region("roms", 0x8000);
	map(0x2000, 0x2fff).rom().region("roms", 0x9000);
}

void alphatro_state::alphatro_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x10, 0x10).rw(FUNC(alphatro_state::port10_r), FUNC(alphatro_state::port10_w));
	map(0x20, 0x20).portr("X0").w(FUNC(alphatro_state::port20_w));
	map(0x21, 0x21).portr("X1");
	map(0x22, 0x22).portr("X2");
	map(0x23, 0x23).portr("X3");
	map(0x24, 0x24).portr("X4");
	map(0x25, 0x25).portr("X5");
	map(0x26, 0x26).portr("X6");
	map(0x27, 0x27).portr("X7");
	map(0x28, 0x28).portr("X8");
	map(0x29, 0x29).portr("X9");
	map(0x2a, 0x2a).portr("XA");
	map(0x2b, 0x2b).portr("XB");
	map(0x30, 0x30).rw(FUNC(alphatro_state::port30_r), FUNC(alphatro_state::port30_w));
	// USART for cassette reading and writing
	map(0x40, 0x41).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	// CRTC - HD46505 / HD6845SP
	map(0x50, 0x50).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x51, 0x51).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	// 8257 DMAC
	map(0x60, 0x68).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	// 8259 PIT
	//map(0x70, 0x72).r(FUNC(alphatro_state::)).w(FUNC(alphatro_state::));
	map(0xf0, 0xf0).r(FUNC(alphatro_state::portf0_r)).w(FUNC(alphatro_state::portf0_w));
	map(0xf8, 0xf8).rw(m_fdc, FUNC(upd765a_device::fifo_r), FUNC(upd765a_device::fifo_w));
	map(0xf9, 0xf9).r(m_fdc, FUNC(upd765a_device::msr_r));
}

static INPUT_PORTS_START( alphatro )
	PORT_START("X0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("X1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad - /") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_ASTERISK) // keypad equals
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad + x") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("X2")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("X3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("X4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("X5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("X6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xa3)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ ~") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@ `") PORT_CODE(KEYCODE_DEL) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')

	PORT_START("X8")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("X9")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("XA")
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS DEL") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Clear Home") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)

	PORT_START("XB")
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)

	PORT_START("other")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF,alphatro_state,alphatro_break,0)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x20, 0x00, "Monitor")
	PORT_CONFSETTING(    0x00, "RGB")
	PORT_CONFSETTING(    0x20, "Amber")

	PORT_CONFNAME(0x01, 0x00, "Floppy disk installed")
	PORT_CONFSETTING(0x00, "Not present")
	PORT_CONFSETTING(0x01, "Installed")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( gfx_alphatro )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 4 )
GFXDECODE_END

void alphatro_state::machine_start()
{
	m_port_10 = 0x01;

	save_item(NAME(m_port_10));
	save_item(NAME(m_port_20));
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void alphatro_state::machine_reset()
{
	m_ram_ptr = m_ram->pointer();
	port10_w(0);
	m_port_20 = 0;
	update_banking();

	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
	m_cassbit = 1;
	m_cassold = 1;
	m_fdc_irq = 0;
	m_usart->write_rxd(1);
	m_usart->write_cts(0);
	m_beep->set_state(0);
}

image_init_result alphatro_state::load_cart(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	if ((size != 0x4000) && (size != 0x2000))
	{
		image.seterror(IMAGE_ERROR_UNSUPPORTED, "Invalid size, must be 8 or 16 K" );
		return image_init_result::FAIL;
	}

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);

	if (size == 0x4000) // 16K ROMs come in at 0xA000
	{
		slot->common_load_rom(slot->get_rom_base(), size, "rom");
	}
	else    // load 8K ROMs at an offset of 8K so they end up at 0xC000
	{
		slot->common_load_rom(slot->get_rom_base()+0x2000, size, "rom");
	}

	return image_init_result::PASS;
}

void alphatro_state::alphatro_palette(palette_device &palette) const
{
	// RGB colours
	palette.set_pen_color(0, 0x00, 0x00, 0x00);
	palette.set_pen_color(1, 0x00, 0x00, 0xff);
	palette.set_pen_color(2, 0xff, 0x00, 0x00);
	palette.set_pen_color(3, 0xff, 0x00, 0xff);
	palette.set_pen_color(4, 0x00, 0xff, 0x00);
	palette.set_pen_color(5, 0x00, 0xff, 0xff);
	palette.set_pen_color(6, 0xff, 0xff, 0x00);
	palette.set_pen_color(7, 0xff, 0xff, 0xff);
	// Amber
	palette.set_pen_color(8, 0xf7, 0xaa, 0x00);
}


WRITE_LINE_MEMBER(alphatro_state::kansas_w)
{
	// incoming @19230Hz
	u8 twobit = m_cass_data[3] & 3;

	// relay off - exit, but wait for last bit to completely write
	if ((!BIT(m_port_10, 3)) && (twobit == 0))
	{
		m_cass->output(0);
		m_cass_data[3] = 0;     // reset waveforms
		return;
	}

	if (state)
	{
		if (twobit == 0)
			m_cassold = m_cassbit;

		if (m_cassold)
			m_cass->output(BIT(m_cass_data[3], 2) ? -1.0 : +1.0); // 2400Hz
		else
			m_cass->output(BIT(m_cass_data[3], 3) ? -1.0 : +1.0); // 1200Hz

		m_cass_data[3]++;
	}

	m_usart->write_txc(state);
}

WRITE_LINE_MEMBER(alphatro_state::kansas_r)
{
	if (!BIT(m_port_10, 3))
	{
		m_usart->write_rxd(1);
		m_cass_data[0] = m_cass_data[1] = 0;
		return;
	}

	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_usart->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
	m_usart->write_rxc(state);
}

WRITE_LINE_MEMBER(alphatro_state::fdc_irq_w)
{
	m_fdc_irq = state ? false : true;
}

WRITE_LINE_MEMBER(alphatro_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

FLOPPY_FORMATS_MEMBER( alphatro_state::floppy_formats )
	FLOPPY_DSK_FORMAT,
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static void alphatro_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void alphatro_state::alphatro(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &alphatro_state::alphatro_map);
	m_maincpu->set_addrmap(AS_IO, &alphatro_state::alphatro_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	if (m_is_ntsc)
		screen.set_raw(16_MHz_XTAL, 1016, 0, 640, 271, 0, 216);
	else
		screen.set_raw(16_MHz_XTAL, 1016, 0, 640, 314, 0, 240);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_alphatro);
	PALETTE(config, m_palette, FUNC(alphatro_state::alphatro_palette), 9); // 8 colours + amber

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, "beeper", 16_MHz_XTAL / 4 / 13 / 128).add_route(ALL_OUTPUTS, "mono", 1.00); // nominally 2.4 kHz

	/* Devices */
	UPD765A(config, m_fdc, 16_MHz_XTAL / 2, true, true); // clocked through SED-9420C
	m_fdc->intrq_wr_callback().set(FUNC(alphatro_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq2_w));
	FLOPPY_CONNECTOR(config, "fdc:0", alphatro_floppies, "525dd", alphatro_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", alphatro_floppies, "525dd", alphatro_state::floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_original("alphatro_flop");

	I8257(config, m_dmac, 16_MHz_XTAL / 4);
	m_dmac->out_hrq_cb().set(FUNC(alphatro_state::hrq_w));
	m_dmac->in_memr_cb().set(FUNC(alphatro_state::ram0000_r));
	m_dmac->out_memw_cb().set(FUNC(alphatro_state::ram0000_w));
	m_dmac->in_ior_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_cb<2>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dmac->out_tc_cb().set(m_fdc, FUNC(upd765a_device::tc_line_w));

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(alphatro_state::crtc_update_row));

	I8251(config, m_usart, 16_MHz_XTAL / 4);
	m_usart->txd_handler().set([this] (bool state) { m_cassbit = state; });

	clock_device &usart_clock(CLOCK(config, "usart_clock", 16_MHz_XTAL / 4 / 13 / 16));
	usart_clock.signal_handler().set(FUNC(alphatro_state::kansas_w));
	usart_clock.signal_handler().append(FUNC(alphatro_state::kansas_r));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("alphatro_cass");
	SOFTWARE_LIST(config, "cass_list").set_original("alphatro_cass");

	RAM(config, "ram").set_default_size("64K");

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "alphatro_cart", "bin").set_device_load(FUNC(alphatro_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("alphatro_cart");

	/* 0000 banking */
	ADDRESS_MAP_BANK(config, "lowbank").set_map(&alphatro_state::rombank_map).set_options(ENDIANNESS_BIG, 8, 32, 0x6000);

	/* A000 banking */
	ADDRESS_MAP_BANK(config, "cartbank").set_map(&alphatro_state::cartbank_map).set_options(ENDIANNESS_BIG, 8, 32, 0x4000);

	/* F000 banking */
	ADDRESS_MAP_BANK(config, "monbank").set_map(&alphatro_state::monbank_map).set_options(ENDIANNESS_BIG, 8, 32, 0x1000);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( alphatro )
	ROM_REGION( 0xa000, "roms", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "pcb-ig", "Alphatronic PC PCB-IG" ) // correctly displays German Umlauts
	ROMX_LOAD( "0_b4-6_ic1038.bin", 0x008000, 0x002000, CRC(e337db3b) SHA1(6010bade6a21975636383179903b58a4ca415e49), ROM_BIOS(0) )
	ROMX_LOAD( "1_b4-3_ic1058.bin", 0x000000, 0x002000, CRC(1509b15a) SHA1(225c36411de680eb8f4d6b58869460a58e60c0cf), ROM_BIOS(0) )
	ROMX_LOAD( "2_b4-3_ic1046.bin", 0x002000, 0x002000, CRC(998a865d) SHA1(294fe64e839ae6c4032d5db1f431c35e0d80d367), ROM_BIOS(0) )
	ROMX_LOAD( "3_b4-3_ic1037.bin", 0x004000, 0x002000, CRC(55cbafef) SHA1(e3376b92f80d5a698cdcb2afaa0f3ef4341dd624), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS( 1, "pcb-ii", "Alphatronic PC PCB-II")
	ROMX_LOAD( "613256.ic-1058", 0x0000, 0x6000, CRC(ceea4cb3) SHA1(b332dea0a2d3bb2978b8422eb0723960388bb467), ROM_BIOS(1) )
	ROMX_LOAD( "2764.ic-1038",   0x8000, 0x2000, CRC(e337db3b) SHA1(6010bade6a21975636383179903b58a4ca415e49), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 2, "bicom", "Alphatronic PC PCB-II with BICOM Graphics extension") // correctly displays German Umlauts
	ROMX_LOAD( "613256.ic-1058", 0x0000, 0x6000, CRC(ceea4cb3) SHA1(b332dea0a2d3bb2978b8422eb0723960388bb467), ROM_BIOS(2) )
	ROMX_LOAD( "tapcgv2_ic1038.bin",   0x8000, 0x2000, CRC(446b4235) SHA1(ef835ae46b3fdfe6a6f394971396a577528e7b5a), ROM_BIOS(2) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROMX_LOAD( "4_b4-0_ic1067.bin", 0x000000, 0x001000, CRC(00796934) SHA1(8e70f77cfe3eb2ec2051f660518da5c9d409119a), ROM_BIOS(0) )
	ROMX_LOAD( "2732.ic-1067",   0x0000, 0x1000, CRC(61f38814) SHA1(35ba31c58a10d5bd1bdb202717792ca021dbe1a8), ROM_BIOS(1) )
	ROMX_LOAD( "b40r_ic1067.bin",   0x0000, 0x1000, CRC(543e3ee8) SHA1(3e6c6f8c85d3a5d0735edfec52709c5670ff1646), ROM_BIOS(2) )
ROM_END

#define rom_alphatron rom_alphatro

COMP( 1983, alphatro,  0,        0, alphatro, alphatro, alphatro_pal_state,  empty_init, "Triumph-Adler", "Alphatronic PC (PAL)",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, alphatron, alphatro, 0, alphatro, alphatro, alphatro_ntsc_state, empty_init, "Triumph-Adler", "Alphatronic PC (NTSC)", MACHINE_SUPPORTS_SAVE )
