// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Falco TS-series terminals.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "bus/rs232/rs232.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "video/mc6845.h"
#include "screen.h"

class falcots_state : public driver_device
{
public:
	falcots_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_vram(*this, "vram")
		, m_chargen(*this, "chargen")
	{
	}

	void ts2624(machine_config &config);

private:
	MC6845_UPDATE_ROW(update_row);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	optional_device<mc6845_device> m_crtc;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_chargen;
};

MC6845_UPDATE_ROW(falcots_state::update_row)
{
}

void falcots_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xbfff).ram(); // 4x HM6116P-3
	map(0xc000, 0xffff).ram().share("vram"); // 8x AM9016EPC (4116)
}

void falcots_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).nopr(); // keyboard input?
	map(0xe1, 0xe1).nopw(); // keyboard scan?
	map(0xe8, 0xe8).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xe9, 0xe9).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0xf0, 0xf3).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0xf8, 0xfb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START(ts2624)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

void falcots_state::ts2624(machine_config &config)
{
	Z80(config, m_maincpu, 14.7456_MHz_XTAL / 4); // Z8400AB1
	m_maincpu->set_addrmap(AS_PROGRAM, &falcots_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falcots_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 14.7456_MHz_XTAL / 4)); // Z8430AB1
	ctc.set_clk<0>(14.7456_MHz_XTAL / 16);
	ctc.set_clk<1>(14.7456_MHz_XTAL / 16);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 14.7456_MHz_XTAL / 4)); // Z8470AB1
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.7456_MHz_XTAL, 768, 0, 640, 320, 0, 286);
	//screen.set_raw(23.9616_MHz_XTAL, 1248, 0, 1056, 320, 0, 286);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 14.7456_MHz_XTAL / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->set_update_row_callback(FUNC(falcots_state::update_row), this);
}

ROM_START(ts2624)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(14fb80aa) SHA1(93bf0d39f3e4bf092b6cd850f95ee6cbd322ad13))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(d4c74a06) SHA1(291357a296c45fccdbe8e395ea170d847a3a6f03))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(90d0d04b) SHA1(099d6741091b3abbe4187c8278e2c7ebe151531c))
	ROM_LOAD("4.bin", 0x6000, 0x2000, CRC(b0c59ec8) SHA1(099f6d6a7594e177bc668fd19fa19c3f0f4ab38e))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("chr.bin", 0x0000, 0x2000, CRC(38569fe2) SHA1(c666c596bb6326e4f41ccfd91154bcfd75f5c0a3))

	ROM_REGION(0x60, "proms", 0)
	ROM_LOAD("msel64b.9c", 0x00, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.13d",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.12f",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
ROM_END

COMP(1982, ts2624, 0, 0, ts2624, ts2624, falcots_state, empty_init, "Falco Data Products", "TS-2624", MACHINE_IS_SKELETON)
