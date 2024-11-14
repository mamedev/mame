// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    CompuData Tulip System I

    Hardware:
    - I8086 CPU
    - I8087 FPU (optional)
    - I8089 I/O Processor (optional)
    - 8 kb ROM, 4 + 8 kb graphics ROM
    - 128 to 896 kb RAM
    - 2x TC5517AP VRAM
    - HD46505SP-2 CRT (+ 7220?)
    - MC68B50P ACIA (for the keyboard?)
    - 8256A-P MUART
    - WD2793PL-02 FDC

    TODO:
    - No MUART emulation which handles interrupts, timers, serial and
      parallel interfaces.

    Notes:
    - If you hit the keyboard a few times it tries to load the OS first
      from floppy and then from the hard drive.

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "emupal.h"
#include "screen.h"


namespace {

class tulip1_state : public driver_device
{
public:
	tulip1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_acia(*this, "acia"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fdc(*this, "fdc"),
		m_vram(*this, "vram"),
		m_chargen(*this, "chargen")
		{}

	void tulip1(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

private:
	required_device<i8086_cpu_device> m_cpu;
	required_device<hd6845s_device> m_crtc;
	required_device<acia6850_device> m_acia;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<wd2793_device> m_fdc;
	required_shared_ptr<uint16_t> m_vram;
	required_region_ptr<uint8_t> m_chargen;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);
};

void tulip1_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0xdffff).noprw();
	map(0xe0000, 0xe0fff).ram().share("vram");
	map(0xfe000, 0xfffff).rom().region("bios", 0);
}

void tulip1_state::io_map(address_map &map)
{
	map(0xfc00, 0xfc07).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write)).umask16(0x00ff);
//  map(0xfca0, 0xfcaf) // ?
//  map(0xfcb0) // ?
//  map(0xfc80) // ?
	map(0xfc91, 0xfc91).lr8(NAME([]() -> u8 { return 0x04; })); // ?
//  map(0xfd00, 0xfd1f) // muart
	map(0xfd98, 0xfd98).rw(m_acia, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0xfd9c, 0xfd9c).rw(m_acia, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));
	map(0xfe00, 0xfe00).w(m_crtc, FUNC(hd6845s_device::address_w));
	map(0xfe02, 0xfe02).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
//  map(0xfe82) // ?
//  map(0xff00, 0xff1f) // ?
//  map(0xff30) // ?
}

static INPUT_PORTS_START( tulip1 )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8, 11,
	256*3,
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END

MC6845_UPDATE_ROW( tulip1_state::crtc_update_row )
{
	pen_t const *const pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		uint16_t code = m_vram[(ma + i) & 0xfff] & 0xff;
		uint8_t data = m_chargen[(code << 4) | ra];

		// draw 8 pixels of the character
		for (int x = 0; x < 8; x++)
			bitmap.pix(y, x + i*8) = pen[BIT(data, 7 - x)];
	}
}

static void tulip1_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void tulip1_state::machine_start()
{
}

void tulip1_state::tulip1(machine_config &config)
{
	// main cpu
	I8086(config, m_cpu, 8000000);
	m_cpu->set_addrmap(AS_PROGRAM, &tulip1_state::mem_map);
	m_cpu->set_addrmap(AS_IO, &tulip1_state::io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16000000, 912, 0, 640, 312, 0, 275); // unknown clock
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	HD6845S(config, m_crtc, 2000000); // unknown clock
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(tulip1_state::crtc_update_row));

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("kbd", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "kbd", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	uart_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	// floppy disk controller
	WD2793(config, m_fdc, 1000000); // unknown clock

	// floppy drives
	FLOPPY_CONNECTOR(config, "fdc:0", tulip1_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", tulip1_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
}

ROM_START( tulip1 )
	ROM_REGION16_LE(0x2000, "bios", 0)
	ROM_LOAD16_BYTE("tulip1_15d.bin", 0x0000, 0x1000, CRC(78793e46) SHA1(9850d82062ef617497a3eb49ab07848650ee9b8c))
	ROM_LOAD16_BYTE("tulip1_16d.bin", 0x0001, 0x1000, CRC(8c9e4475) SHA1(e7446334e5284512a8103d1b5e4612a2922b1bdc))

	ROM_REGION(0x3000, "chargen", ROMREGION_INVERT)
	ROM_LOAD("tulip1_h106.bin", 0x0000, 0x1000, CRC(fdde779d) SHA1(01df551853f117ad91b47e389edf10e0f0d0d4c2))
	ROM_LOAD("tulip1_h305.bin", 0x1000, 0x2000, CRC(f17f03f1) SHA1(060cca61fadd82fe9917430340bb880478ceeec6))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY    FULLNAME            FLAGS
COMP( 1983, tulip1, 0,      0,      tulip1,  tulip1, tulip1_state, empty_init, "CompuData", "Tulip System I", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
