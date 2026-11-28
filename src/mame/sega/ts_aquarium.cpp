// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for Sega Toys Tenohira Suizokukan Aquarium (手のひら水族館 アクアリウム) virtual pet.

    Two models were released: "Angelfish" and "Dolphin".

    Button board has a (debug?) 5-pin pad (1 is GND, 3 and 4 are pulled-high, 5 is NC).

    Reference: https://web.archive.org/web/20081219185724/http://www.segatoys.co.jp/aquarium/index.html

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/st2204.h"

#include "emupal.h"
#include "screen.h"


namespace {

class ts_aquarium_state : public driver_device
{
public:
	ts_aquarium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void ts_aquarium(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD { }

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	void data_map(address_map &map) ATTR_COLD;
	void palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
};

void ts_aquarium_state::data_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("maincpu", 0);
}

void ts_aquarium_state::ts_aquarium(machine_config &config)
{
	// FIXME: Guessed clock and MCU:
	// - ROM offset 0x42DF has SMB instruction;
	// - ROM offset 0x4656 has WAI instruction surrounded by NOPs;
	// - Interrupt vector table has exactly 13 entries at same offsets as ST2204, but MCU has LCD controller with RGB support;
	ST2204(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_DATA, &ts_aquarium_state::data_map);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(96, 96);
	m_screen->set_visarea(0, 96 - 1, 0, 96 - 1);
	m_screen->set_screen_update(FUNC(ts_aquarium_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(ts_aquarium_state::palette), 256);
}

ROM_START( tsaqdlph )
	// Pin-compatible with MX23L6410
	ROM_REGION(0x800000, "maincpu", 0)
	ROM_LOAD("rom.bin", 0x000000, 0x800000, CRC(8e682b37) SHA1(3c14664b93ed52d2897fc454cca68d70f7c15eed))
ROM_END

} // anonymous namespace


// Build string: Copyright Gamze Limited 2007 2007-09-24 15:10
// Release date: 2008-03-20
COMP( 2008, tsaqdlph, 0, 0, ts_aquarium, 0, ts_aquarium_state, empty_init, "Sega Toys / Gamze Limited", "Tenohira Suizokukan Aquarium - Dolphin", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
