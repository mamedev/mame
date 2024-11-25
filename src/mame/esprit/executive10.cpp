// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Esprit Systems Executive 10/102

    VT102 compatible terminal

    Hardware:
    - Intel P8088
    - Intel D8284A
    - Intel P8259A-8
    - 2x 2764 EPROM, 1x 2732 EPROM
    - SY2128-2 (2k RAM)
    - X2212D NOVRAM
    - MC68B50P
    - SCN2681A
    - 14.469 MHz XTAL (near CPU), 3.6864 MHz XTAL (near 2681)
    - CRT 9007
    - 2x CRT 9006-135
    - CRT9021A
    - 2764 EPROM
    - 4x SY2128-2 (8k RAM)
    - 21.800 MHz XTAL (near 9007), 16.960 MHz XTAL (near 9007)
    - 9 position DIP switch

    TODO:
    - Improve rendering (132 columns, smooth scrolling, non-line attributes(?))
    - NOVRAM store/recall
    - Printer
    - DIP switch

    Notes:
    - Other models in this line: 10/51 (IBM 5251), 10/78 (IBM 3278)
    - To go online: Enter Set-Up, press 5 for next screen, press 4 to switch
      from local to online (no feedback, would be shown on keyboard LEDs)

***************************************************************************/

#include "emu.h"

#include "executive10_102_kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/6850acia.h"
#include "machine/mc68681.h"
#include "machine/pic8259.h"
#include "machine/x2212.h"
#include "video/crt9007.h"

#include "emupal.h"
#include "multibyte.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class executive10_state : public driver_device
{
public:
	executive10_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_novram(*this, "novram"),
		m_pic(*this, "pic"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vpac(*this, "vpac"),
		m_duart(*this, "duart"),
		m_acia(*this, "acia"),
		m_cram(*this, "cram"),
		m_aram(*this, "aram"),
		m_chargen(*this, "chargen")
	{ }

	void executive10(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8088_cpu_device> m_maincpu;
	required_device<x2212_device> m_novram;
	required_device<pic8259_device> m_pic;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<crt9007_device> m_vpac;
	required_device<scn2681_device> m_duart;
	required_device<acia6850_device> m_acia;
	required_shared_ptr<uint8_t> m_cram;
	required_shared_ptr<uint8_t> m_aram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void executive10_state::mem_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2fff).ram().share("cram");
	map(0x4000, 0x4fff).ram().share("aram");
	map(0x6000, 0x60ff).rw(m_novram, FUNC(x2210_device::read), FUNC(x2210_device::write));
	map(0xb000, 0xffff).rom().region("maincpu", 0);
}

void executive10_state::io_map(address_map &map)
{
	map(0x000, 0x03f).rw(m_vpac, FUNC(crt9007_device::read), FUNC(crt9007_device::write));
	map(0x040, 0x04f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x080, 0x080).w(m_acia, FUNC(acia6850_device::control_w));
	map(0x081, 0x081).r(m_acia, FUNC(acia6850_device::status_r));
	map(0x082, 0x082).w(m_acia, FUNC(acia6850_device::data_w));
	map(0x083, 0x083).r(m_acia, FUNC(acia6850_device::data_r));
	map(0x0c0, 0x0c1).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	//map(0x180, 0x180).nopw(); // novram store/recall?
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( executive10 )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t executive10_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *const pen = m_palette->pens();

	for (int y = 0; y < 25; y++)
	{
		// f---------------  double height line
		// -e--------------  double width line
		// --dc------------  unknown
		// ----ba9876543210  line address

		uint16_t addr = get_u16le(&m_cram[0xfcc + (y * 2)]);
		bool dw = addr & 0xc000;

		uint8_t line_attr = 0x00;

		for (int x = 0; x < (dw ? 40 : 80); x++)
		{
			uint16_t code = m_cram[(addr & 0x0fff) + x];

			// move bit 7 to 8 (acs characters)
			code = bitswap<9>(code, 7, 8, 6, 5, 4, 3, 2, 1, 0);

			// select 80/132 column character set
			code |= 0x80; // fixed 80 columns mode for now

			// 7-------  active for line attributes
			// -6------  unknown
			// --5-----  blink
			// ---4----  reverse
			// ----3---  0 = blank?
			// -----2--  underline
			// ------1-  unknown
			// -------0  highlight

			uint8_t attr = m_aram[(addr & 0x0fff) + x];

			// new line attribute?
			if (BIT(attr, 7))
				line_attr = attr;

			for (int i = 0; i < 12; i++)
			{
				unsigned char_line = i;

				// adjust rendered line for double height mode
				if (BIT(addr, 15))
					char_line = (i / 2) + (BIT(addr, 14) * (12 / 2));

				uint16_t data = m_chargen[(code << 4) + char_line] << 1;

				// maybe? fixes line drawing characters
				if (BIT(data, 8) == 1)
				{
					if (BIT(data, 1) == 1)
						data |= 0x001;

					if (BIT(data, 7) == 0)
						data &= ~0x100;
				}

				// underline?
				if (BIT(line_attr, 2) && i == 10)
					data = 0x1ff;

				// reverse?
				if (BIT(line_attr, 4))
					data = ~data;

				// blink?
				if (BIT(line_attr, 5) && (m_screen->frame_number() & 0x20)) // wrong timing
					data = 0x000;

				// cursor?
				if (m_vpac->cursor_active(x, y))
					data = ~data; // might be solid instead

				// foreground/background colors
				rgb_t fg = BIT(line_attr, 0) ? pen[2] : pen[1];
				rgb_t bg = pen[0];

				// draw character line
				if (dw)
				{
					for (int p = 0; p < 9; p++)
					{
						bitmap.pix(y * 12 + i, x * 18 + p * 2 + 0) = BIT(data, 8 - p) ? fg : bg;
						bitmap.pix(y * 12 + i, x * 18 + p * 2 + 1) = BIT(data, 8 - p) ? fg : bg;
					}
				}
				else
				{
					for (int p = 0; p < 9; p++)
						bitmap.pix(y * 12 + i, x * 9 + p) = BIT(data, 8 - p) ? fg : bg;
				}
			}
		}
	}

	return 0;
}

static const gfx_layout char_layout =
{
	8, 12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8 * 16
};

static GFXDECODE_START( chars )
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void executive10_state::machine_start()
{
}

void executive10_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void executive10_state::executive10(machine_config &config)
{
	I8088(config, m_maincpu, 14.469_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &executive10_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &executive10_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	X2212(config, m_novram);

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(16.960_MHz_XTAL, 890, 0, 720, 320, 0, 300); // maybe
	m_screen->set_screen_update(FUNC(executive10_state::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	CRT9007(config, m_vpac, 16.960_MHz_XTAL / 9);
	m_vpac->set_screen("screen");
	m_vpac->set_character_width(9);
	m_vpac->int_callback().set(m_pic, FUNC(pic8259_device::ir2_w));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->a_tx_cb().set("serial", FUNC(rs232_port_device::write_txd));
	m_duart->outport_cb().set(m_acia, FUNC(acia6850_device::write_rxc)).bit(3);
	m_duart->outport_cb().append(m_acia, FUNC(acia6850_device::write_txc)).bit(3);
	m_duart->outport_cb().append(m_pic, FUNC(pic8259_device::ir0_w)).bit(4).invert();
	m_duart->outport_cb().append(m_pic, FUNC(pic8259_device::ir1_w)).bit(5).invert();

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	serial.cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));

	ACIA6850(config, m_acia);
	m_acia->irq_handler().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_acia->txd_handler().set("kbd", FUNC(executive10_102_kbd_device::rxd_w));

	executive10_102_kbd_device &kbd(EXECUTIVE10_102_KBD(config, "kbd"));
	kbd.txd_cb().set(m_acia, FUNC(acia6850_device::write_rxd));
	kbd.cts_cb().set(m_acia, FUNC(acia6850_device::write_cts));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( exe10102 )
	ROM_REGION(0x5000, "maincpu", 0)
	ROM_LOAD("113-03-0.u25", 0x0000, 0x1000, CRC(bf5498d4) SHA1(46a3e832a1ba9a08c1d4938d6c94b2194ef00081))
	ROM_LOAD("113-01-0.u26", 0x1000, 0x2000, CRC(5e6babb3) SHA1(4669107e9cfaba1a697db6e832bc36c8220ee591))
	ROM_LOAD("113-02-0.u27", 0x3000, 0x2000, CRC(05bcd49b) SHA1(fbeac116ded46215644df7db1810f4f9d41e49ca))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("111-01-0.u59", 0x0000, 0x2000, CRC(4e54c69f) SHA1(92640a57863c5eab9db6d07c31deebfa3d1dafa5))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE      INPUT        CLASS              INIT        COMPANY           FULLNAME            FLAGS
COMP( 1983, exe10102, 0,      0,      executive10, executive10, executive10_state, empty_init, "Esprit Systems", "Executive 10/102", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
