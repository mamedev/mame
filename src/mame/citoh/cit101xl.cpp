// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for CIE Terminals (C. Itoh) CIT-50+ and CIT-101XL video terminals.

***********************************************************************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/z180/z180.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "screen.h"


namespace {

class cit101xl_state : public driver_device
{
public:
	cit101xl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_avdc(*this, "avdc")
	{
	}

	void cit101xl(machine_config &config);

private:
	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	required_device<z180_device> m_maincpu;
	required_device<scn2674_device> m_avdc;
};


SCN2674_DRAW_CHARACTER_MEMBER(cit101xl_state::draw_character)
{
}


void cit101xl_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("program", 0);
	map(0x1c000, 0x1ffff).rom().region("program", 0x8000);
	map(0x2e000, 0x2ffff).ram().share("nvram");
}

void cit101xl_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); // HD64180 internal registers
	map(0x40, 0x47).rw(m_avdc, FUNC(scn2674_device::read), FUNC(scn2674_device::write));
	map(0x48, 0x48).nopr(); // watchdog?
	map(0x60, 0x60).w(m_avdc, FUNC(scn2674_device::buffer_w));
	map(0x68, 0x68).r(m_avdc, FUNC(scn2674_device::buffer_r));
	map(0x70, 0x70).w(m_avdc, FUNC(scn2674_device::attr_buffer_w));
	map(0x78, 0x78).r(m_avdc, FUNC(scn2674_device::attr_buffer_r));
}

void cit101xl_state::char_map(address_map &map)
{
	map(0x0000, 0x3fff).ram(); // TMM2063P-12 x2
}

void cit101xl_state::attr_map(address_map &map)
{
	map(0x0000, 0x3fff).ram(); // TMM2063P-12 x2
}


static INPUT_PORTS_START(cit101xl)
INPUT_PORTS_END


void cit101xl_state::cit101xl(machine_config &config)
{
	HD64180RP(config, m_maincpu, 12.288_MHz_XTAL); // HD64B180R0P
	m_maincpu->set_addrmap(AS_PROGRAM, &cit101xl_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cit101xl_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5564APL-15 + battery?

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24.27_MHz_XTAL, 980, 0, 820, 413, 0, 364);
	screen.set_screen_update(m_avdc, FUNC(scn2674_device::screen_update));

	SCN2674(config, m_avdc, 24.27_MHz_XTAL / 10); // SCN2674BC4N40
	//m_avdc->intr_callback().set_inputline(m_maincpu, ???);
	m_avdc->set_character_width(10); // 10x13 character cell
	m_avdc->set_display_callback(FUNC(cit101xl_state::draw_character));
	m_avdc->set_addrmap(0, &cit101xl_state::char_map);
	m_avdc->set_addrmap(1, &cit101xl_state::attr_map);
	m_avdc->set_screen("screen");
}

// XTAL OSCs: 12.288 MHz (CPU), 24.270 MHz, 36.000 MHz
// Gate arrays: HG61H09R84F (QFP80), L7A0084 4155P 00416A (LCC68)
ROM_START(cit101xl)
	ROM_REGION(0xc000, "program", 0)
	ROM_LOAD("cit50p-101xl_v.1.1_0800.u2", 0x0000, 0x8000, CRC(2dbbd7f6) SHA1(f87b32e803bda5a8dd0e39e2e339357cfa4082ad)) // HN27256G-25 with handwritten label
	ROM_LOAD("cit50p-101xl_v.1.1_8456.u4", 0x8000, 0x4000, CRC(4df0b677) SHA1(1916c65935c47cb0e11a8c7f293b608e648c542b)) // M5L27128K with handwritten label

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("tmm2464ap_1104_cit50p_v1.2.u3", 0x0000, 0x2000, CRC(e07723f7) SHA1(466d69382cc75ac0abcda08e1a227da73fc77980)) // Toshiba OTP ROM with silkscreened label
ROM_END

} // anonymous namespace


COMP(1987, cit101xl, 0, 0, cit101xl, cit101xl, cit101xl_state, empty_init, "CIE Terminals", "CIT-101XL Video Display Terminal", MACHINE_IS_SKELETON)
