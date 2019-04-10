// license:BSD-3-Clause
// copyright-holders:Robbbert, Dirk Best
/****************************************************************************

    Qume QVT-102 serial terminal

    Known hardware:
    - Motorola 6800 CPU
    - Hitachi HD46505 (Motorola 6845-compatible) CRTC
    - Hitachi HD46850 (Motorola 6850-compatible) ACIA
    - M58725P-15 (6116-compatible) (2k x 8bit RAM)
    - Zilog Z8430 CTC
    - 16.6698MHz Crystal
    - 2x TC5514-APL + 3V battery, functioning as NVRAM

    Keyboard: D8748D, 6.000MHz Crystal, Beeper

    Not sure what's going on here... the ACIA only has one serial channel,
    is it for the host, the printer, or the keyboard?

    If baudrate == 9600, then MASTER_CLOCK / 18 goes to CTC, it divides by
    6 to the ACIA, which divides by 16, to give roughly 9600.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "screen.h"
#include "emupal.h"
#include "bus/rs232/rs232.h"

#define MASTER_CLOCK         16.6698_MHz_XTAL

class qvt102_state : public driver_device
{
public:
	qvt102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "acia")
		, m_ctc(*this, "ctc")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void qvt102(machine_config &config);

private:
	void mem_map(address_map &map);
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<m6800_cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_device<z80ctc_device> m_ctc;
	required_device<h46505_device> m_crtc;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_nvram;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

void qvt102_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).ram().share("nvram");
	map(0x2800, 0x2803).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x3000, 0x3000).nopr();
	map(0x3800, 0x3800).nopw();    // many writes of 43 and 4B
	map(0x4000, 0x47ff).ram().share("videoram").mirror(0x3800);
	map(0x8000, 0x8000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x9800, 0x9801).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xa000, 0xa000).lr8("unk", [] (u8 data) { return 0xff; });   // needs to be negative
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

MC6845_UPDATE_ROW( qvt102_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();

	// line attribute (active for the rest of the line)
	uint8_t attr = 0;

	for (int x = 0; x < x_count; x++)
	{
		uint16_t mem = (ma + x) & 0x7ff;
		uint8_t chr = m_p_videoram[mem];
		uint16_t gfx = m_p_chargen[(chr << 4) | ra];

		// check for new attribute
		if (chr >= 0x90 && chr <= 0x9f)
			attr = chr;

		int half = BIT(chr, 7);

		// apply attributes
		if (BIT(attr, 2)) gfx = ~gfx; // reverse
		if (BIT(attr, 1) && (m_screen->frame_number() & 32)) gfx = 0; // blink (frequency?)
		if (BIT(attr, 0)) gfx = 0; // blank
		if (BIT(attr, 3) && ra == 11) gfx = 0x1ff; // underline

		// cursor active?
		if (x == cursor_x) gfx = ~gfx;

		// draw 9 pixels of the character
		for (int i = 0; i < 9; i++)
			bitmap.pix32(y, x*9 + (8-i)) = palette[BIT(gfx, i) ? 2 - half : 0];
	}
}

static const gfx_layout char_layout =
{
	8,12,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16
};

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
GFXDECODE_END

void qvt102_state::qvt102(machine_config &config)
{
	M6800(config, m_maincpu, MASTER_CLOCK / 18);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt102_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 2x TC5514-APL + 3V battery

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_raw(MASTER_CLOCK, 882, 9, 729, 315, 0, 300); // 80x24+1
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);
	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	H46505(config, m_crtc, MASTER_CLOCK / 9);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(qvt102_state::crtc_update_row), this);
	m_crtc->out_vsync_callback().set_inputline(m_maincpu, M6800_IRQ_LINE);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_acia->irq_handler().set_inputline("maincpu", M6800_IRQ_LINE);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "keyboard"));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	Z80CTC(config, m_ctc, MASTER_CLOCK / 9);
	m_ctc->set_clk<0>(MASTER_CLOCK / 18);
	m_ctc->set_clk<1>(MASTER_CLOCK / 18);
	m_ctc->zc_callback<0>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_ctc->zc_callback<0>().append(m_acia, FUNC(acia6850_device::write_rxc));
//  m_ctc->zc_callback<1>().set(m_acia, FUNC(acia6850_device::write_rxc));

	I8748(config, "kbdmcu", XTAL(6'000'000));
}

static INPUT_PORTS_START( qvt102 )
INPUT_PORTS_END

ROM_START( qvt102 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "1", "1" ) // no status bar by default
	ROMX_LOAD( "u8.bin",   0x0000, 0x2000, CRC(2e375abc) SHA1(12ad1e49c5773c36c3a8d65845c9a50f9dec141f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "2", "2" ) // status bar by default (+ more changes)
	ROMX_LOAD( "t205m.u8", 0x0000, 0x2000, CRC(59cc04f6) SHA1(ee2e3a3ea7b57a231483fcc74266f0f3f51204af), ROM_BIOS(1) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "c3205m.u32", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb) )

	ROM_REGION(0x0400, "kbdmcu", 0)
	ROM_LOAD( "k301.u302",  0x0000, 0x0400, CRC(67564b20) SHA1(5897ff920f8fae4aa498d3a4dfd45b58183c041d) )
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1983, qvt102, 0,      0,      qvt102,  qvt102,  qvt102_state, empty_init, "Qume",  "QVT-102", MACHINE_NO_SOUND_HW )
