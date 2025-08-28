// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADI VT52

    VT52 compatible terminal

    Hardware:
    - SY6502
    - R6532-11
    - HD46505
    - SYU6551A
    - MC6850P
    - MB8128-15 (2k RAM)
    - 16.5888 MHz XTAL
    - 2x10 position DIP switch

    TODO:
    - VRAM bit 7 is unknown
    - Unknown DIP switch settings
    - Replace HLE serial keyboard
    - 6850 ACIA clock rate and source
    - AUX port

    Notes:
    - Displays "ERROR 8" - this means "keyboard error". The keyboard needs to
      send 0xfe on startup
    - Everything here is a guess, only a PCB picture is available
    - PCB marked "A. D. I. 304-029-01" and date code 841210

***************************************************************************/

#include "emu.h"

#include "adi_unk_kbd.h" // only here so that it doesn't bitrot

#include "bus/rs232/keyboard.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mos6530.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class adi_vt52_state : public driver_device
{
public:
	adi_vt52_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_riot(*this, "riot"),
		m_crtc(*this, "crtc"),
		m_palette(*this, "palette"),
		m_mos6551(*this, "mos6551"),
		m_rs232(*this, "rs232"),
		m_mc6850(*this, "mc6850"),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
	{ }

	void adi_vt52(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m6502_device> m_maincpu;
	required_device<mos6532_device> m_riot;
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<mos6551_device> m_mos6551;
	required_device<rs232_port_device> m_rs232;
	required_device<acia6850_device> m_mc6850;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void adi_vt52_state::mem_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0003).rw(m_mos6551, FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x0040, 0x0040).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x0041, 0x0041).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0080, 0x00ff).mirror(0x100).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0280, 0x029f).m(m_riot, FUNC(mos6532_device::io_map));
	map(0x2000, 0x27ff).mirror(0x800).ram().share("vram");
	map(0x4000, 0x4001).rw(m_mc6850, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x7000, 0x7fff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( adi_vt52 )
	PORT_START("dip1") // might be dip2
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DIP1:1") PORT_NAME("DIP1-1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DIP1:2") PORT_NAME("DIP1-2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DIP1:3") PORT_NAME("DIP1-3") // when set, vram bit 7 set for unwritten areas
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DIP1:4") PORT_NAME("DIP1-4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "DIP1:5") PORT_NAME("DIP1-5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "DIP1:6") PORT_NAME("DIP1-6")
	PORT_DIPNAME(0x40, 0x00, "Local Echo") PORT_DIPLOCATION("DIP1:7")
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPSETTING(0x40, DEF_STR( Off ))
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "DIP1:8") PORT_NAME("DIP1-8")

	PORT_START("dip2") // might be dip1
	PORT_DIPNAME(0x0f, 0x0e, "Baud Rate") PORT_DIPLOCATION("DIP2:1,2,3,4")
	PORT_DIPSETTING(0x00, "9600 (duplicate?)")
	PORT_DIPSETTING(0x01, "50")
	PORT_DIPSETTING(0x02, "75")
	PORT_DIPSETTING(0x03, "110")
	PORT_DIPSETTING(0x04, "134.5")
	PORT_DIPSETTING(0x05, "150")
	PORT_DIPSETTING(0x06, "300")
	PORT_DIPSETTING(0x07, "600")
	PORT_DIPSETTING(0x08, "1200")
	PORT_DIPSETTING(0x09, "1800")
	PORT_DIPSETTING(0x0a, "2400")
	PORT_DIPSETTING(0x0b, "3600")
	PORT_DIPSETTING(0x0c, "4800")
	PORT_DIPSETTING(0x0d, "7200")
	PORT_DIPSETTING(0x0e, "9600")
	PORT_DIPSETTING(0x0f, "19200")
	PORT_DIPUNKNOWN(0x10, IP_ACTIVE_LOW) PORT_NAME("DIP2-5") PORT_DIPLOCATION("DIP2:5") // 50/60 hz switch?
	PORT_DIPNAME(0x20, 0x00, "Stop Bits") PORT_DIPLOCATION("DIP2:6")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x20, "2")
	PORT_DIPNAME(0x40, 0x40, "Parity") PORT_DIPLOCATION("DIP2:7")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x40, "Even")
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "DIP2:8") PORT_NAME("DIP2-8")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

MC6845_UPDATE_ROW(adi_vt52_state::crtc_update_row)
{
	const pen_t *const pen = m_palette->pens();

	for (unsigned x = 0; x < x_count; x++)
	{
		uint8_t code = m_vram[(ma + x) & 0x7ff];
		uint8_t data = m_chargen[((code & 0x7f) << 4) | ra];

		if (x == cursor_x)
			data = 0xff;

		// draw 8 pixels of the character
		for (int i = 0; i < 8; i++)
			bitmap.pix(y, x * 8 + i) = pen[BIT(data, 7 - i)];
	}
}

static const gfx_layout char_layout =
{
	8, 12,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START( chars )
	GFXDECODE_ENTRY( "chargen", 0, char_layout, 0, 1 )
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void adi_vt52_state::machine_start()
{
}

void adi_vt52_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( rs232 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void adi_vt52_keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SERIAL_KEYBOARD);
}

static DEVICE_INPUT_DEFAULTS_START( keyboard )
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END

void adi_vt52_state::adi_vt52(machine_config &config)
{
	M6502(config, m_maincpu, 16.5888_MHz_XTAL / 16); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &adi_vt52_state::mem_map);

	input_merger_any_high_device &irq_merger(INPUT_MERGER_ANY_HIGH(config, "irq_merger"));
	irq_merger.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MOS6532(config, m_riot, 16.5888_MHz_XTAL / 16);
	m_riot->pa_rd_callback().set_ioport("dip1");
	m_riot->pb_rd_callback().set_ioport("dip2");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(16.5888_MHz_XTAL, 800, 0, 640, 307, 0, 288);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	HD6845S(config, m_crtc, 16.5888_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8); // might be 9
	m_crtc->set_update_row_callback(FUNC(adi_vt52_state::crtc_update_row));
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, "gfxdecode", m_palette, chars);

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	MOS6551(config, m_mos6551, 16.5888_MHz_XTAL / 16);
	m_mos6551->set_xtal(16.5888_MHz_XTAL / 9);
	m_mos6551->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_mos6551->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr)); // unknown if connected
	m_mos6551->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts)); // unknown if connected
	m_mos6551->irq_handler().set("irq_merger", FUNC(input_merger_any_high_device::in_w<0>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_mos6551, FUNC(mos6551_device::write_rxd));
	m_rs232->dsr_handler().set(m_mos6551, FUNC(mos6551_device::write_dsr)); // unknown if connected
	m_rs232->cts_handler().set(m_mos6551, FUNC(mos6551_device::write_cts)); // unknown if connected
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(rs232));

	ACIA6850(config, m_mc6850, 16.5888_MHz_XTAL / 16); // for keyboard
	m_mc6850->irq_handler().set("irq_merger", FUNC(input_merger_any_high_device::in_w<1>));

	// might be clocked externally by the keyboard
	clock_device &kbd_clock(CLOCK(config, "kbd_clock", 9600 * 64));
	kbd_clock.signal_handler().set(m_mc6850, FUNC(acia6850_device::write_rxc));

	rs232_port_device &kbd(RS232_PORT(config, "kbd", adi_vt52_keyboard_devices, "keyboard"));
	kbd.rxd_handler().set(m_mc6850, FUNC(acia6850_device::write_rxd));
	kbd.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( adi_vt52 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("vt52_rev0.u6", 0x0000, 0x1000, CRC(ddeb77ef) SHA1(46ee82b22ac1ed1693189305b49001d4daed8f46))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("vt52_cg.u16", 0x0000, 0x0800, CRC(af2be51d) SHA1(055a7c0de842a17a60924e17153241e823b2a7f1))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME  FLAGS
COMP( 1984, adi_vt52, 0,      0,      adi_vt52, adi_vt52, adi_vt52_state, empty_init, "ADI",   "VT52",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
