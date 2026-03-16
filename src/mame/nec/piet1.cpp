// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for NEC PI-ET1.

    - PCB Markings: NEC PWD-850 707V / 72408502 / W03-104PE
    - Cart Slot Markings: JAE JC20-C45PD-F1-A2-1 9X1
    - IC101: Hitachi 9M3T HC74N (Dual D-Type Flip-Flop)
    - IC106: NEC D70008A-8 (µPD780 CPU)
    - IC109: NEC D65013GFE63 (Gate Array)
    - IC110: NEC D4991A (4bit Calendar Clock)
    - IC111: NEC D43257AGU-15L (256Kbit Static RAM)
    - IC112: NEC D23C4000GF A69 (4Mbit Mask ROM)

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/upd4991a.h"

#include "emupal.h"
#include "screen.h"


namespace {

class piet1_state : public driver_device
{
public:
	piet1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_banks(*this, "bank%u", 0U)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_rtc(*this, "rtc")
		, m_screen(*this, "screen")
	{ }

	void piet1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank_array<4> m_banks;
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<upd4991a_device> m_rtc;
	required_device<screen_device> m_screen;

	void bank_switch(u8 bank, u8 entry);
	void piet1_io_map(address_map &map) ATTR_COLD;
	void piet1_map(address_map &map) ATTR_COLD;
	void piet1_palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
};

void piet1_state::bank_switch(u8 bank, u8 entry)
{
	if (entry < 0x20) {
		m_banks[bank]->set_entry(entry);
	}
}

void piet1_state::piet1_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x20, 0x20).lw8(
		NAME([this](u8 data) {
			logerror("%s: io_20_w %02x\n", machine().describe_context(), data);
			bank_switch(2, data);
		}));
	map(0x21, 0x21).lw8(
		NAME([this](u8 data) {
			logerror("%s: io_21_w %02x\n", machine().describe_context(), data);
			bank_switch(3, data);
		}));
	map(0x70, 0x70).lr8(
		NAME([this]() {
			logerror("%s: io_70_r\n", machine().describe_context());
			return 0x08; // Avoid jump to NMI ISR @ 0x0bb6
		}));
}

void piet1_state::piet1_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_banks[0]);
	map(0x4000, 0x7fff).bankrw(m_banks[1]);
	map(0x8000, 0xbfff).bankr(m_banks[2]);
	map(0xc000, 0xffff).bankr(m_banks[3]);
}

void piet1_state::machine_start()
{
	u8 *rom = memregion("mask_rom")->base();
	u8 *ram = m_ram->pointer();

	m_banks[0]->configure_entries(0, 32, rom, 0x4000);
	m_banks[1]->configure_entries(0,  2, ram, 0x4000);
	m_banks[2]->configure_entries(0, 32, rom, 0x4000);
	m_banks[3]->configure_entries(0, 32, rom, 0x4000);

	for (size_t i = 0; i < 4; i++) {
		m_banks[i]->set_entry(0);
	}
}

void piet1_state::piet1(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL); // FIXME: Guessed (D70008A-6 = 6MHz).
	m_maincpu->set_addrmap(AS_PROGRAM, &piet1_state::piet1_map);
	m_maincpu->set_addrmap(AS_IO, &piet1_state::piet1_io_map);

	RAM(config, RAM_TAG).set_default_size("32K").set_default_value(0x00);

	UPD4991A(config, m_rtc, 32'768); // Frequency marked on PCB.

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 64);
	m_screen->set_visarea(0, 160 - 1, 0, 64 - 1);
	m_screen->set_screen_update(FUNC(piet1_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(piet1_state::piet1_palette), 2);
}

ROM_START( piet1 )
	ROM_REGION16_LE(0x80000, "mask_rom", 0)
	ROM_LOAD("d23c4000gf-a69.ic112", 0x00000, 0x80000, CRC(e175fb54) SHA1(db5f72904b7f6a82a728a7ce968612673119def4))
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                  FLAGS
COMP( 1990, piet1, 0,      0,      piet1,   0,     piet1_state, empty_init, "NEC",   "Electronic Tool PI-ET1", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
