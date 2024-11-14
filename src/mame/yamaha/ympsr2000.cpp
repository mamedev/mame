// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/sh/sh4.h"
#include "video/sed1330.h"

#include "debugger.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class psr2000_state : public driver_device {
public:
	psr2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_lcdc(*this, "lcdc")
	{ }

	void psr2000(machine_config &config);

private:
	required_device<sh3be_device> m_maincpu;
	required_device<sed1330_device> m_lcdc;  // In reality a sed1335

	void map(address_map &map) ATTR_COLD;
	void lcdc_map(address_map &map) ATTR_COLD;

	void machine_start() override ATTR_COLD;
};

void psr2000_state::machine_start()
{
}

void psr2000_state::psr2000(machine_config &config)
{
	SH3BE(config, m_maincpu, 10_MHz_XTAL*4);
	m_maincpu->set_addrmap(AS_PROGRAM, &psr2000_state::map);

	auto &palette = PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(60);
	screen.set_screen_update(m_lcdc, FUNC(sed1330_device::screen_update));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_palette(palette);

	SED1330(config, m_lcdc, 16_MHz_XTAL / 2); // Clock provided by the fdc
	m_lcdc->set_screen("screen");
	m_lcdc->set_addrmap(0, &psr2000_state::lcdc_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void psr2000_state::map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().region("maincpu", 0).mirror(0xe0000000);
	map(0x08000000, 0x087fffff).rom().region("style", 0).mirror(0xe0000000);
	map(0x0c000000, 0x0c7fffff).ram().mirror(0xe0000000);
	map(0x10000000, 0x100fffff).rom().region("data", 0).mirror(0xe0000000);
	map(0x14000000, 0x1400000f); // 8bitcs -> lcdccs, fdccs
	map(0x14400000, 0x14400000).rw(m_lcdc, FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x14400002, 0x14400002).rw(m_lcdc, FUNC(sed1330_device::data_r), FUNC(sed1330_device::command_w));

	map(0x14c00000, 0x14ffffff).nopw();
	map(0x18000000, 0x1800000f); // 16bitcs -> vopcs, tgccs
}

void psr2000_state::lcdc_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).ram();
}

static INPUT_PORTS_START( psr2000 )
INPUT_PORTS_END

ROM_START( psr2000 )
	ROM_REGION64_BE( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "x031420.ic206", 0, 0x400000, CRC(42109711) SHA1(4b9ff95d55154fff50142134e2de0fd9b95c4e8e))
	ROM_LOAD32_WORD_SWAP( "x031520.ic205", 2, 0x400000, CRC(4b0943cb) SHA1(eb2895f3957b06b991fe3a3b3030fa9aa3cc85b6))

	ROM_REGION64_BE( 0x800000, "style", 0 )
	ROM_LOAD16_WORD_SWAP( "x031710.ic207", 0, 0x800000, CRC(669c31ba) SHA1(d0b13ca2414ad8b476391ea53a511783f9f8d030))

	ROM_REGION64_BE( 0x400000, "data", 0 ) // flash
	ROM_LOAD16_WORD_SWAP( "xv685a0.ic204", 0, 0x100000, CRC(c339a386) SHA1(6ae30913d2aff7ae2056f6a5f1a2264ad3f7798c))

	ROM_REGION32_BE( 0x1000000, "swp30", 0 )
	ROM_LOAD32_WORD_SWAP( "x004010.ic406", 0, 0x800000, CRC(126e5f07) SHA1(db93965e4226212fdbe34df2f0f5457173f01dda))
	ROM_LOAD32_WORD_SWAP( "x004110.ic407", 2, 0x800000, CRC(fae720fe) SHA1(df80a9a75308f0ab1ab33248b5e8cd6563e6aed5))
ROM_END

} // anonymous namespace

SYST( 2001, psr2000, 0, 0, psr2000, psr2000, psr2000_state, empty_init, "Yamaha", "PSR-2000", MACHINE_IS_SKELETON )
