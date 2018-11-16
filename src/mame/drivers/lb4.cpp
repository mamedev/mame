// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************

    Liberty Electronics LB-4 serial terminal

    Known hardware:
    - Motorola 6800 CPU
    - Hitachi HD46505 (Motorola 6845-compatible) CRTC
    - Hitachi HD46850 (Motorola 6850-compatible) ACIA
    - Zilog Z8430 CTC
    - 16.6698MHz Crystal

    Not sure what's going on here... the ACIA only has one serial channel,
    is it for the host, the printer, or the keyboard? There's no real
    evidence of a keyboard in the photos, so perhaps it's a display-only
    unit with printer. If so, then Rx is from the host, and Tx is to the
    printer.
    If baudrate == 9600, then MASTER_CLOCK / 18 goes to CTC, it divides by
    6 to the ACIA, which divides by 16, to give roughly 9600.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/z80ctc.h"
#include "video/mc6845.h"
#include "screen.h"
#include "emupal.h"
#include "bus/rs232/rs232.h"

#define MASTER_CLOCK	16.6698_MHz_XTAL

class lb4_state : public driver_device
{
public:
	lb4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "acia")
		, m_ctc(*this, "ctc")
		, m_crtc(*this, "crtc")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void lb4(machine_config &config);

private:
	void mem_map(address_map &map);
	MC6845_UPDATE_ROW(crtc_update_row);

	required_device<m6800_cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia;
	required_device<z80ctc_device> m_ctc;
	required_device<h46505_device> m_crtc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

void lb4_state::mem_map(address_map &map)
{
	// There are what look like RAM reads/writes from 0000-03ff, but also accesses which look like attempts to talk to a device.
	map(0x0000, 0x03ff).ram();
	map(0x2800, 0x2803).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	//map(0x3800, 0x3800).w    // many writes of 43 and C3
	map(0x4000, 0x47ff).ram().share("videoram");
	map(0x8000, 0x8000).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x8001, 0x8001).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x9800, 0x9801).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

MC6845_UPDATE_ROW(lb4_state::crtc_update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 chr,gfx,inv;
	u16 mem,x;
	u32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];
		inv = ((x == cursor_x) ^ BIT(chr, 7)) ? 0xff : 0;
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
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

void lb4_state::lb4(machine_config &config)
{
	// All dividers unknown/guessed
	M6800(config, m_maincpu, MASTER_CLOCK / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &lb4_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, 2).set_init("palette", FUNC(palette_device::palette_init_monochrome));
	GFXDECODE(config, m_gfxdecode, m_palette, chars);

	H46505(config, m_crtc, MASTER_CLOCK / 10);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(lb4_state::crtc_update_row), this);

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	rs232.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));

	Z80CTC(config, m_ctc, MASTER_CLOCK / 4);
	m_ctc->set_clk<0>(MASTER_CLOCK / 18);
	m_ctc->zc_callback<0>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_ctc->zc_callback<0>().append(m_acia, FUNC(acia6850_device::write_rxc));
}


static INPUT_PORTS_START( lb4 )
INPUT_PORTS_END

ROM_START( lb4 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "u8.bin", 0x0000, 0x2000, CRC(2e375abc) SHA1(12ad1e49c5773c36c3a8d65845c9a50f9dec141f) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "u32.bin", 0x0000, 0x1000, CRC(f6d86e87) SHA1(c0885e4a35095a730d760bf91a1cf4e8edd6a2bb) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT    CLASS      INIT        COMPANY                FULLNAME  FLAGS
COMP( 197?, lb4,  0,      0,      lb4,     lb4,     lb4_state, empty_init, "Liberty Electronics", "LB-4",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
