// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for TI Model 931 RS-232/fiber-optic video display terminal.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "bus/rs232/rs232.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class ti931_state : public driver_device
{
public:
	ti931_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pvtc(*this, "pvtc")
		, m_chargen(*this, "chargen")
	{
	}

	void ti931(machine_config &config);

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<scn2672_device> m_pvtc;
	required_region_ptr<u8> m_chargen;
};

SCN2672_DRAW_CHARACTER_MEMBER(ti931_state::draw_character)
{
	u16 dots = m_chargen[(charcode & 0x7f) << 4 | linecount];
	const bool half_shift = BIT(dots, 7);
	dots = (dots & 0x7f) << 1;

	// TODO: attributes
	for (int i = 0; i < 9; i++)
	{
		if (!half_shift)
			dots <<= 1;
		bitmap.pix(y, x++) = BIT(dots, 8) ? rgb_t::white() : rgb_t::black();
		if (half_shift)
			dots <<= 1;
		bitmap.pix(y, x++) = BIT(dots, 8) ? rgb_t::white() : rgb_t::black();
	}
}

void ti931_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).ram().share("charram");
	map(0x8800, 0x8fff).ram().share("attrram");
	map(0x9000, 0x97ff).ram().share("nvram");
	map(0x9800, 0x9fff).ram(); // also NVRAM?
}

void ti931_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x0b).rw("dart", FUNC(z80dart_device::cd_ba_r), FUNC(z80dart_device::cd_ba_w));
	map(0x10, 0x17).rw(m_pvtc, FUNC(scn2672_device::read), FUNC(scn2672_device::write));
}

void ti931_state::char_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("charram"); // HM6116LP-3
}

void ti931_state::attr_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("attrram"); // HM6116LP-3
}

static INPUT_PORTS_START(ti931)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "dart" },
	{ nullptr }
};

void ti931_state::ti931(machine_config &config)
{
	Z80(config, m_maincpu, 22.1184_MHz_XTAL / 6); // MK3880N-4
	m_maincpu->set_addrmap(AS_PROGRAM, &ti931_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ti931_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-3 or TC5517APL + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 22.1184_MHz_XTAL / 6)); // Z8340APS
	ctc.set_clk<0>(22.1184_MHz_XTAL / 12);
	ctc.set_clk<1>(22.1184_MHz_XTAL / 12);
	ctc.set_clk<2>(22.1184_MHz_XTAL / 12); // possibly different from the other two
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().append("dart", FUNC(z80dart_device::txca_w));
	// No CTC interrupts

	z80dart_device &dart(Z80DART(config, "dart", 22.1184_MHz_XTAL / 6)); // Z8370APS
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(22.1184_MHz_XTAL * 2, 954 * 2, 0, 720 * 2, 389, 0, 350);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 22.1184_MHz_XTAL / 9); // SCN2672B (with SCB2677B)
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9 * 2);
	m_pvtc->set_addrmap(0, &ti931_state::char_map);
	m_pvtc->set_addrmap(1, &ti931_state::attr_map);
	m_pvtc->set_display_callback(FUNC(ti931_state::draw_character));
	m_pvtc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

ROM_START(ti931)
	ROM_REGION(0x8000, "program", ROMREGION_ERASEFF)
	ROM_LOAD("2229208-0003_xu9.bin", 0x0000, 0x2000, CRC(a3c43008) SHA1(4bcce64487ed2b72cc8bc898c1c85291d9b766ab))
	ROM_LOAD("2229209-0003_xu7.bin", 0x2000, 0x2000, CRC(49481614) SHA1(b69b0b1f27f7e8910dbf501c00098b816e8920d7))
	ROM_LOAD("2229221-0003_xu5.bin", 0x4000, 0x2000, CRC(30f28d08) SHA1(81709b7d90899f5d9ea66026b2cf6add698e17c1))
	// XU3 socket is unpopulated

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("2229199-2_xu1.bin", 0x0000, 0x0800, CRC(f76bb2bb) SHA1(57ec0392a2b5798eef43f0408347402c2941cee1)) // 01/29/85 CTE
ROM_END

} // anonymous namespace


COMP(1983, ti931, 0, 0, ti931, ti931, ti931_state, empty_init, "Texas Instruments", "Model 931 Video Display Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)

