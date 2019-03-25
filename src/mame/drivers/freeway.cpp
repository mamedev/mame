// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    FreeWay (c) 1999 NVC Electronica Ltd.

    Skeleton driver.

*********************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/ins8250.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/timekpr.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"

class freeway_state : public driver_device
{
public:
	freeway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic")
		, m_charram(*this, "charram")
		, m_colorram(*this, "colorram")
		, m_lamps(*this, "lamp%u", 1U)
	{
	}

	void freeway(machine_config &config);

	DECLARE_WRITE_LINE_MEMBER(nmi_w);

protected:
	virtual void machine_start() override;

private:
	MC6845_UPDATE_ROW(update_row);

	void lamps_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;

	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_colorram;

	output_finder<3> m_lamps;
};

void freeway_state::machine_start()
{
	m_lamps.resolve();
}

MC6845_UPDATE_ROW(freeway_state::update_row)
{
}

void freeway_state::lamps_w(u8 data)
{
	for (int n = 0; n < 3; n++)
		m_lamps[n] = BIT(data, n);
}

WRITE_LINE_MEMBER(freeway_state::nmi_w)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

void freeway_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x08000, 0x09fff).rw("timekpr", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0xa0000, 0xa0fff).ram().share("charram");
	map(0xa4000, 0xa4fff).ram().share("colorram");
	map(0xf0000, 0xfffff).rom().region("program", 0);
}

void freeway_state::io_map(address_map &map)
{
	map(0x0020, 0x0021).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0030, 0x0033).w("pit", FUNC(pit8254_device::write));
	map(0x0040, 0x0047).rw("uart", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0x00a3, 0x00a3).w(FUNC(freeway_state::lamps_w));
	map(0x00d0, 0x00d0).portr("CONFIG");
	map(0x03d0, 0x03d0).w("crtc", FUNC(mc6845_device::address_w));
	map(0x03d1, 0x03d1).w("crtc", FUNC(mc6845_device::register_w));
}

static INPUT_PORTS_START(freeway)
	PORT_START("CONFIG")
	PORT_DIPNAME(0x01, 0x00, "Screen Format")
	PORT_DIPSETTING(0x00, "PAL")
	PORT_DIPSETTING(0x01, "NTSC?") // 268 lines total, 58.3 Hz refresh
	PORT_BIT(0x08, 0x08, IPT_UNKNOWN)
	PORT_BIT(0xf6, 0xf6, IPT_UNKNOWN) // probably unused

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MEMORY_RESET) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, freeway_state, nmi_w)
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8,
};

static GFXDECODE_START(gfx_freeway)
	GFXDECODE_ENTRY("gfx", 0, char_layout, 0x0, 1)
GFXDECODE_END

void freeway_state::freeway(machine_config &config)
{
	I8088(config, m_maincpu, 10_MHz_XTAL / 2); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &freeway_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &freeway_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	pit8254_device &pit(PIT8254(config, "pit"));
	pit.out_handler<0>().set(m_pic, FUNC(pic8259_device::ir0_w));

	M48T58(config, "timekpr");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10_MHz_XTAL / 2, 320, 0, 256, 312, 0, 256);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette").set_entries(0x10);
	GFXDECODE(config, "gfxdecode", "palette", gfx_freeway);

	mc6845_device &crtc(MC6845(config, "crtc", 10_MHz_XTAL / 16));
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");
	crtc.set_update_row_callback(FUNC(freeway_state::update_row), this);
	crtc.out_hsync_callback().set("pit", FUNC(pit8254_device::write_clk0)); // guess

	ns16450_device &uart(NS16450(config, "uart", 10_MHz_XTAL / 8)); // type unknown
	uart.out_int_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	uart.out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(ns16450_device::rx_w));
	rs232.cts_handler().set("uart", FUNC(ns16450_device::cts_w));
	rs232.dsr_handler().set("uart", FUNC(ns16450_device::dsr_w));
	rs232.dcd_handler().set("uart", FUNC(ns16450_device::dcd_w));
}

// 8088 CPU
// Intel 8254 Programmable Interval Timer
// Intel 8259
// 2x 8k SRAM
// 1x 32k SRAM
// 6845 video chip
// 5 roms
// Oscillator 10 MHz

ROM_START(freeway)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("vip88.bin", 0x00000, 0x10000, CRC(aeba6d5e) SHA1(bb84f7040bf1b6976cb2c50b1ffdc59ae88df223))

	ROM_REGION(0x18000, "gfx", 0)
	ROM_LOAD("sb_51.bin",  0x00000, 0x8000, CRC(d25bd328) SHA1(b8c692298f6dc5fd5ae2f9e7701e14b0436a95bb)) // xxx0xxxxxxxxxxx = 0xFF
	ROM_LOAD("sb_52.bin",  0x08000, 0x8000, CRC(f2b33acd) SHA1(e4786b4f00871d771aadacd9d6ec767691f4d939))
	ROM_LOAD("sb_53.bin",  0x10000, 0x8000, CRC(50407ae6) SHA1(2c6c4803905bed5f27c6783f99a24f8dee62c19b))

	ROM_REGION(0x8000, "cor", 0)
	ROM_LOAD("sb_cor.bin", 0x0000, 0x8000, CRC(5f86a160) SHA1(f21b7e0e6a407371c252d6fde6fcb32a2682824c)) // all 0xFF fill until 0x7C00; valid data is only 4 bits wide
ROM_END


GAME(1999, freeway, 0, freeway, freeway, freeway_state, empty_init, ROT0, "NVC Electronica", "FreeWay (V5.12)", MACHINE_IS_SKELETON)
