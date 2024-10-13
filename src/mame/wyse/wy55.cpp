// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-55 and related terminals.

    The WY-55's custom video gate array is numbered 211019-05. The WY-185 is believed to run on similar hardware, though with
    85 Hz and 60 Hz vertical refresh rates rather than 80 Hz and 70 Hz.

    WY-65's "PELVIS" ASIC (QFP160) supports refresh rates up to 94 Hz.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "machine/scn_pci.h"
#include "screen.h"

namespace {

class wy55_state : public driver_device
{
public:
	wy55_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_epci(*this, "epci")
		, m_screen(*this, "screen")
		, m_progbank(*this, "progbank")
	{
	}

	void wy55(machine_config &config);
	void wy65(machine_config &config);
	void wy185es(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void driver_start() override;

private:
	void wy65_progbank_w(u8 data) { m_progbank->set_entry(data & 0x03); }
	u8 epci_r(offs_t offset) { return m_epci->read(offset >> 10); }
	void epci_w(offs_t offset, u8 data) { m_epci->write(offset >> 10, data); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void wy65_ext_map(address_map &map) ATTR_COLD;
	void wy185es_ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	optional_device<scn_pci_device> m_epci;
	required_device<screen_device> m_screen;
	required_memory_bank m_progbank;
};


void wy55_state::machine_start()
{
	memory_region *rgn = memregion("program");
	m_progbank->configure_entries(0, rgn->bytes() / 0x10000, rgn->base(), 0x10000);
	m_progbank->set_entry(0);
}

u32 wy55_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wy55_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).bankr("progbank");
}

void wy55_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram0");
	map(0x8000, 0x9fff).ram().share("nvram1");
	map(0xa000, 0xbfff).ram().share("fontram");
	//map(0xf028, 0xf037).rw("uart", FUNC(pc16552_device::read), FUNC(pc16552_device::write));
}

void wy55_state::wy185es_ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram0");
	map(0x8000, 0x9fff).ram().share("nvram1");
	map(0xa000, 0xbfff).ram().share("fontram");
	map(0xe000, 0xefff).r(FUNC(wy55_state::epci_r));
	map(0xf000, 0xffff).w(FUNC(wy55_state::epci_w));
}

void wy55_state::wy65_ext_map(address_map &map)
{
	map(0x0000, 0xdfff).ram();
	map(0xee02, 0xee02).w(FUNC(wy55_state::wy65_progbank_w));
}


static INPUT_PORTS_START(wy55)
INPUT_PORTS_END

void wy55_state::wy55(machine_config &config)
{
	I80C32(config, m_maincpu, 14.7456_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wy55_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &wy55_state::ext_map);
	m_maincpu->port_out_cb<1>().set_membank("progbank").bit(2);

	NVRAM(config, "nvram0", nvram_device::DEFAULT_ALL_0); // 8K SRAM + battery
	NVRAM(config, "nvram1", nvram_device::DEFAULT_ALL_0); // 8K SRAM + battery

	//PC16552D(config, "uart", 14.7456_MHz_XTAL / 2); // 16C452 (divider not verified)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(49.4235_MHz_XTAL, 1575, 0, 1188, 448, 0, 416);
	//m_screen->set_raw(49.4235_MHz_XTAL * 2 / 3, 1050, 0, 800, 392, 0, 338);
	m_screen->set_screen_update(FUNC(wy55_state::screen_update));
}

void wy55_state::wy185es(machine_config &config)
{
	wy55(config);
	m_maincpu->set_clock(11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_IO, &wy55_state::wy185es_ext_map);
	m_maincpu->port_out_cb<1>().set_nop();

	SCN2661B(config, m_epci, 49.4235_MHz_XTAL / 10); // SCN2661BC1N28
	m_epci->rxrdy_handler().set_inputline(m_maincpu, MCS51_INT1_LINE);
}

void wy55_state::wy65(machine_config &config)
{
	DS80C320(config, m_maincpu, 58.9824_MHz_XTAL / 4); // divider uncertain
	m_maincpu->set_addrmap(AS_PROGRAM, &wy55_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &wy55_state::wy65_ext_map);

	// TODO: NVRAM? (4x W24257S-70LL on board)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(58.9824_MHz_XTAL, 1575, 0, 1188, 448, 0, 416); // dimensions probably wrong
	m_screen->set_screen_update(FUNC(wy55_state::screen_update));
}


ROM_START(wy55)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("251352-12.bin", 0x00000, 0x20000, CRC(efe41862) SHA1(52ee76d636b166fa10a37356aef81011a9b079cc)) // v2.1
ROM_END

ROM_START(wy65)
	ROM_REGION(0x40000, "program", 0)
	ROM_LOAD("251455-03.bin", 0x00000, 0x40000, CRC(2afbf73b) SHA1(5a29b78ef377a6e2f2f91ff42a7e4d86eb511a5f)) // v2.1
ROM_END

ROM_START(wy185es)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("251201-03.bin", 0x00000, 0x10000, CRC(5b8cace5) SHA1(484bba8244a99edb80d7f7a5437c2be52c980fc1)) // v2.0
ROM_END

void wy55_state::driver_start()
{
	memory_region *rgn = memregion("program");
	uint8_t *rom = rgn->base();

	for (offs_t base = 0x00000; base < rgn->bytes(); base += 0x4000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x4000]);

		for (offs_t offset = 0; offset < 0x4000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<14>(offset, 3, 8, 2, 0, 7, 4, 9, 10, 11, 12, 13, 5, 1, 6)], 3, 4, 5, 2, 6, 1, 7, 0);
	}
}

} // anonymous namespace

COMP(1991, wy185es, 0, 0, wy185es, wy55, wy55_state, empty_init, "Wyse Technology", "WY-185ES (v2.0)", MACHINE_IS_SKELETON)
COMP(1993, wy55,    0, 0, wy55,    wy55, wy55_state, empty_init, "Wyse Technology", "WY-55 (v2.1)", MACHINE_IS_SKELETON)
COMP(1996, wy65,    0, 0, wy65,    wy55, wy55_state, empty_init, "Wyse Technology", "WY-65 (v2.1)", MACHINE_IS_SKELETON)
