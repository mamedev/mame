// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Mannesmann Kienzle System 9002 Terminal

2017-08-17 Skeleton driver.

Chips used:
- Siemens SAB8085A-P
- NEC D8251AFC * 2
- NEC D4016C-3 * 4 + 2
- ST M2764A-4F1 * 4
- HD6845P

Chargen is missing.

Both uarts are programmed as synchronous. A conversion box is needed
to talk with RS-232.


****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


namespace {

class sys9002_state : public driver_device
{
public:
	sys9002_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vram(*this, "videoram")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
	{ }

	void sys9002(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	u8 port11_r();

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_vram;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
};


void sys9002_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom(); // 4 * 4K ROM
	map(0x8000, 0x9fff).ram(); // 4 * 2k RAM
	map(0xa000, 0xa7ff).ram().share("videoram"); // 2k RAM
	map(0xc000, 0xc7ff).ram(); // attributes??
	map(0xe000, 0xefff).ram(); // ??
}

void sys9002_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x04).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x05, 0x05).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x08, 0x09).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x11).r(FUNC(sys9002_state::port11_r)); //nopr();  // continuous read
	map(0x1c, 0x1d).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

u8 sys9002_state::port11_r()
{
	static u8 data = 0;
	data += 0x08;
	return data;
}

/* Input ports */
static INPUT_PORTS_START( sys9002 )
INPUT_PORTS_END

MC6845_UPDATE_ROW( sys9002_state::crtc_update_row )
{
	rgb_t const *const pens = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix(y);

	for (uint16_t x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_vram[mem] & 0x7f;

		/* get pattern of pixels for that character scanline */
		uint8_t gfx = m_p_chargen[(chr<<4) | ra] ^ ((x == cursor_x) ? 0xff : 0);

		/* Display a scanline of a character (8 pixels) */
		*p++ = pens[BIT(gfx, 7)];
		*p++ = pens[BIT(gfx, 6)];
		*p++ = pens[BIT(gfx, 5)];
		*p++ = pens[BIT(gfx, 4)];
		*p++ = pens[BIT(gfx, 3)];
		*p++ = pens[BIT(gfx, 2)];
		*p++ = pens[BIT(gfx, 1)];
		*p++ = pens[BIT(gfx, 0)];
	}
}


void sys9002_state::sys9002(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(2'000'000)); // XTAL not visible on images
	m_maincpu->set_addrmap(AS_PROGRAM, &sys9002_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sys9002_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not correct
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	//GFXDECODE(config, "gfxdecode", m_palette, gfx_sys9002);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	/* Devices */
	mc6845_device &crtc(MC6845(config, "crtc", XTAL(2'000'000))); // clk unknown
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(sys9002_state::crtc_update_row));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 9600));
	uart_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_txc));
	uart_clock.signal_handler().append("uart2", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0)); // sync
	uart1.txd_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232a", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232a", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232a.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	i8251_device &uart2(I8251(config, "uart2", 0)); // sync
	uart2.txd_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	uart2.dtr_handler().set("rs232b", FUNC(rs232_port_device::write_dtr));
	uart2.rts_handler().set("rs232b", FUNC(rs232_port_device::write_rts));
	uart2.rxrdy_handler().set_inputline("maincpu", I8085_RST55_LINE);

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set("uart2", FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set("uart2", FUNC(i8251_device::write_dsr));
	rs232b.cts_handler().set("uart2", FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( sys9002 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "55-040.bin", 0x0000, 0x2000, CRC(781eaca9) SHA1(1bdae2bcc43deaef2eb1d6ec302fbadbb779fd48))
	ROM_LOAD( "55-041.bin", 0x2000, 0x2000, CRC(0f89fe81) SHA1(2dc8de7dabaf11a150cfd34460c5b47612cf5e61))
	ROM_LOAD( "55-042.bin", 0x4000, 0x2000, CRC(e6fbc837) SHA1(fc11f6a6927709552bedf06b9eb0dc66e9a81264))
	ROM_LOAD( "55-048.bin", 0x6000, 0x2000, CRC(879ef945) SHA1(a54fc01ac26a3cd05f6d1e1139d6d99198556575))

	ROM_REGION( 0x0800, "chargen", 0 )  // chargen not dumped, using one from mbee for now
	ROM_LOAD("charrom.bin",  0x0000,  0x0800, BAD_DUMP CRC(b149737b) SHA1(a3cd4f5d0d3c71137cd1f0f650db83333a2e3597) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME                FLAGS
COMP( 198?, sys9002, 0,      0,      sys9002, sys9002, sys9002_state, empty_init, "Mannesmann Kienzle", "System 9002 Terminal", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
