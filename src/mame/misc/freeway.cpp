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


namespace {

class freeway_state : public driver_device
{
public:
	freeway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic(*this, "pic")
		, m_charram(*this, "charram")
		, m_colorram(*this, "colorram")
		, m_gfx_rom(*this, "gfx")
		, m_color_rom(*this, "color_rom")
		, m_lamps(*this, "lamp%u", 1U)
	{
	}

	void freeway(machine_config &config);

	void nmi_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	void lamps_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;

	required_shared_ptr<u8> m_charram;
	required_shared_ptr<u8> m_colorram;

	required_region_ptr<u8> m_gfx_rom;
	required_region_ptr<u8> m_color_rom;

	output_finder<3> m_lamps;

	void palette_init(palette_device &palette) const;
};

void freeway_state::machine_start()
{
	m_lamps.resolve();
}

// TODO: just the minimum to show errors on screen
MC6845_UPDATE_ROW(freeway_state::update_row)
{
	uint16_t x = 0;

	for (uint8_t cx = 0; cx < x_count; cx++)
	{
		int const attr = m_colorram[ma & 0xfff];
		int addr = ((m_charram[ma & 0xfff] | ((attr & 0x10) << 5)) << 3) | (ra & 0x07);

		uint8_t const *const data = m_gfx_rom;

		for (int i = 7; i >= 0; i--)
		{
			int col = 0;
			col |= (BIT(data[0x0000 | addr], i) << 2);
			col |= (BIT(data[0x8000 | addr], i) << 1);
			col |= (BIT(data[0x10000 | addr], i) << 0);

			const u32 pen = (BIT(col, 2) ? 0xff : 0) | (BIT(col, 1) ? 0xff00 : 0) | (BIT(col, 0) ? 0xff0000 : 0);
			bitmap.pix(y, x) = pen;

			x++;
		}
		ma++;
	}
}

void freeway_state::lamps_w(u8 data)
{
	for (int n = 0; n < 3; n++)
		m_lamps[n] = BIT(data, n);
}

void freeway_state::nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}

void freeway_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0x08000, 0x09fff).rw("timekpr", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0xa0000, 0xa0fff).ram().share(m_charram);
	map(0xa4000, 0xa4fff).ram().share(m_colorram);
	map(0xf0000, 0xfffff).rom().region("program", 0);
}

void freeway_state::io_map(address_map &map)
{
	map(0x0020, 0x0021).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0030, 0x0033).w("pit", FUNC(pit8254_device::write));
	map(0x0040, 0x0047).rw("uart", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
	map(0x00a3, 0x00a3).w(FUNC(freeway_state::lamps_w));
	map(0x00d0, 0x00d0).portr("CONFIG");
	map(0x00d1, 0x00d1).portr("INPUTS");
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

	PORT_START("INPUTS")
	PORT_DIPNAME( 0x01, 0x01, "IN0.1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // brings up the 'auditoria' screen in freeway
	PORT_DIPNAME( 0x04, 0x04, "IN0.3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0.4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN0.5" ) // increases the 'num dias' value in freeway
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN0.6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN0.7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN0.8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_MEMORY_RESET) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, freeway_state, nmi_w)
INPUT_PORTS_END

static GFXDECODE_START(gfx_freeway)
	GFXDECODE_ENTRY("gfx", 0, gfx_8x8x3_planar, 0x0, 0x20)
GFXDECODE_END

// TODO: diverges between freeway and freewaya
// makes more sense with latter, different?
void freeway_state::palette_init(palette_device &palette) const
{
	uint8_t const *const data = m_color_rom;

	u8 value = 0;

	for (int i = 0; i < palette.entries(); i++)
	{
		value = data[i + 0x7c00];
		int const r = (value == 0xff) ? 0 : 0x0e * BIT(value, 0) + 0x1f * BIT(value, 1) + 0x43 * BIT(value, 2) + 0x8f * BIT(value, 3);

		value = data[i + 0x7d00];
		int const g = (value == 0xff) ? 0 : 0x0e * BIT(value, 0) + 0x1f * BIT(value, 1) + 0x43 * BIT(value, 2) + 0x8f * BIT(value, 3);

		value = data[i + 0x7f00];
		int const b = (value == 0xff) ? 0 : 0x0e * BIT(value, 0) + 0x1f * BIT(value, 1) + 0x43 * BIT(value, 2) + 0x8f * BIT(value, 3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


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

	PALETTE(config, "palette", FUNC(freeway_state::palette_init), 0x100);
	GFXDECODE(config, "gfxdecode", "palette", gfx_freeway);

	mc6845_device &crtc(MC6845(config, "crtc", 10_MHz_XTAL / 16));
	crtc.set_char_width(8);
	crtc.set_show_border_area(false);
	crtc.set_screen("screen");
	crtc.set_update_row_callback(FUNC(freeway_state::update_row));
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

	ROM_REGION(0x8000, "color_rom", 0)
	ROM_LOAD("sb_cor.bin", 0x0000, 0x8000, CRC(5f86a160) SHA1(f21b7e0e6a407371c252d6fde6fcb32a2682824c)) // all 0xFF fill until 0x7C00; valid data is only 4 bits wide
ROM_END

ROM_START(freewaya)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("memory_431b_eprom-512", 0x00000, 0x10000, CRC(64a23693) SHA1(ad8001c3652fb813d0f15875c7f9478fe2dd873c))

	ROM_REGION(0x18000, "gfx", 0)
	ROM_LOAD("rgb 1fw 256 vr 512b",  0x00000, 0x8000, CRC(b0bed9a0) SHA1(0275f1aa008963eb54f672565d3d5c1dd8c96213))
	ROM_LOAD("rgb 2fw 256 vr 512b",  0x08000, 0x8000, CRC(ed1bb4d8) SHA1(78f9eced819c1fa269e685bb176671158ffcec26))
	ROM_LOAD("rgb 3fw 256 vr 512b",  0x10000, 0x8000, CRC(c685c530) SHA1(deec9ce1df500f14b9ebd007d482473f97b3ecf3))

	ROM_REGION(0x8000, "color_rom", 0)
	ROM_LOAD("color_431b_eprom-256", 0x0000, 0x8000, CRC(e49fc782) SHA1(2c50cf644b7c6449880ed3d6e778ba116e123ae2)) // FIXED BITS (0000xxxx)

	ROM_REGION(0x21ee, "nvram", 0)
	ROM_LOAD("ram_m48t08_431", 0x0000, 0x21ee, CRC(e8fe8d9c) SHA1(bf93ead6ae0f03a646b4952cb999cf0dbc58d223)) // weird size, check when emulation is more mature
ROM_END

} // anonymous namespace


GAME(1999, freeway,        0, freeway, freeway, freeway_state, empty_init, ROT0, "NVC Electronica", "FreeWay (V5.12)", MACHINE_IS_SKELETON)
GAME(1997, freewaya, freeway, freeway, freeway, freeway_state, empty_init, ROT0, "NVC Electronica", "FreeWay (V4.31)", MACHINE_IS_SKELETON)
