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
    - Barcode reader (top right of screen side)

    References:
    - http://videogamekraken.com/pi-et1-by-nec

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
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
		, m_bankdev(*this, "bankdev")
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_rtc(*this, "rtc")
		, m_keys(*this, "KEY%u", 0U)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void piet1(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_memory_bank_array<3> m_banks;
	required_device<address_map_bank_device> m_bankdev;
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<upd4991a_device> m_rtc;
	required_ioport_array<8> m_keys;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	std::unique_ptr<u8[]> m_gvram;

	void piet1_io_map(address_map &map) ATTR_COLD;
	void piet1_map(address_map &map) ATTR_COLD;
	void bankdev_map(address_map &map) ATTR_COLD;
	void palette_init(palette_device &palette) const ATTR_COLD {
		// TODO: refine
		palette.set_pen_color(0, rgb_t(160, 168, 160));
		palette.set_pen_color(1, rgb_t(126, 156, 190));
	}
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 m_gvram_index[3];
};

void piet1_state::video_start()
{
	const u32 gvram_size = 0x40 * 0x100 * 4;
	m_gvram = make_unique_clear<u8[]>(gvram_size);

	save_item(NAME(m_gvram_index));
	save_pointer(NAME(m_gvram), gvram_size);
}

u32 piet1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void piet1_state::piet1_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_banks[0]);
	map(0x4000, 0x7fff).bankrw(m_banks[1]);
	map(0x8000, 0xbfff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0xc000, 0xffff).bankr(m_banks[2]);
}

void piet1_state::piet1_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	// PC-88 style keyboard scan
	map(0x00, 0x07).lr8(NAME([this] (offs_t offset) { return m_keys[offset]->read(); }));

	map(0x20, 0x20).lw8(
		NAME([this](u8 data) {
			logerror("%s: io_20_w %02x\n", machine().describe_context(), data);
			m_bankdev->set_bank(data & 0xff);
		}));
	map(0x21, 0x21).lw8(
		NAME([this](u8 data) {
			logerror("%s: io_21_w %02x\n", machine().describe_context(), data);
			m_banks[2]->set_entry(data & 0x1f);
		}));
	// FIXME: guesswork, may not be right, also reads at $31/$33/$35 in places
	// Why it needs 3 pointers? Separate bitplanes?
	map(0x30, 0x30).select(6).lw8(
		NAME([this](offs_t offset, u8 data) {
			const u8 which = offset >> 1;
			if (which == 3) {
				return;
			}
			m_gvram_index[which] = data << 6;
		}));
	map(0x31, 0x31).select(6).lw8(
		NAME([this](offs_t offset, u8 data) {
			const u8 which = offset >> 1;
			if (which == 3) {
				return;
			}
			m_gvram[m_gvram_index[which] | (which * 0x4000)] = data;
			m_gvram_index[which] ++;
			m_gvram_index[which] &= 0x3fff;
		}));
	map(0x60, 0x60).noprw(); // verbose, standby control? Reads are discarded
	map(0x70, 0x70).lr8(
		NAME([this]() {
			logerror("%s: io_70_r\n", machine().describe_context());
			// TODO: irq source really? cfr. PC=A1A
			return 0x08; // Avoid jump to NMI ISR @ 0x0bb6
		}));
}

void piet1_state::bankdev_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("mask_rom", 0);
//  map(0x100000, 0x1*****) unknown, checks for "NEC/ET01", extension ROM? RAM disk?
//  map(0x200000, 0x2*****) IC card space? (header $18+<jump_vector> then "ComeOn")
}

// TODO: port 3 & 0x48 and port 5 & 4 are checked at startup, initialize sequence?
static INPUT_PORTS_START( piet1 )
	PORT_START("KEY0")
	PORT_DIPNAME( 0x01, 0x01, "KEY0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY1")
	PORT_DIPNAME( 0x01, 0x01, "KEY1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY2")
	PORT_DIPNAME( 0x01, 0x01, "KEY2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY3")
	PORT_DIPNAME( 0x01, 0x01, "KEY3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY4")
	// TODO: possibly lock toggle?
	// Puts the system in sleep mode, disables irq (or at least it was before bank were the wrong way)
	PORT_DIPNAME( 0x01, 0x01, "KEY4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY5")
	PORT_DIPNAME( 0x01, 0x01, "KEY5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY6")
	PORT_DIPNAME( 0x01, 0x01, "KEY6" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("KEY7")
	PORT_DIPNAME( 0x01, 0x01, "KEY7" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void piet1_state::machine_start()
{
	u8 *rom = memregion("mask_rom")->base();
	u8 *ram = m_ram->pointer();

	m_banks[0]->configure_entries(0, 32, rom, 0x4000);
	m_banks[1]->configure_entries(0,  2, ram, 0x4000);
//  m_banks[2]->configure_entries(0, 32, rom, 0x4000);
	m_banks[2]->configure_entries(0, 32, rom, 0x4000);

	for (size_t i = 0; i < 4; i++) {
		m_banks[i]->set_entry(0);
	}
}

void piet1_state::piet1(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL); // FIXME: Guessed (D70008A-6 = 6MHz).
	m_maincpu->set_addrmap(AS_PROGRAM, &piet1_state::piet1_map);
	m_maincpu->set_addrmap(AS_IO, &piet1_state::piet1_io_map);
	// TODO: should be a periodic int, not vblank
	m_maincpu->set_vblank_int("screen", FUNC(piet1_state::irq0_line_hold));

	RAM(config, RAM_TAG).set_default_size("32K").set_default_value(0x00);

	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&piet1_state::bankdev_map).set_options(ENDIANNESS_LITTLE, 8, 23, 0x4000);

	UPD4991A(config, m_rtc, 32'768); // Frequency marked on PCB.

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 64);
	m_screen->set_visarea(0, 160 - 1, 0, 64 - 1);
	m_screen->set_screen_update(FUNC(piet1_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, m_palette, FUNC(piet1_state::palette_init), 2);

	// TODO: sound, likely DAC1BIT
}

ROM_START( piet1 )
	ROM_REGION(0x80000, "mask_rom", 0)
	ROM_LOAD("d23c4000gf-a69.ic112", 0x00000, 0x80000, CRC(e175fb54) SHA1(db5f72904b7f6a82a728a7ce968612673119def4))
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                  FLAGS
COMP( 1990, piet1, 0,      0,      piet1,   piet1, piet1_state, empty_init, "NEC",   "Electronic Tool PI-ET1", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
