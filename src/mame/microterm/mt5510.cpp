// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for Microterm Model 5510 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/mc68681.h"
#include "screen.h"

namespace {

class mt5510_state : public driver_device
{
public:
	mt5510_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
	{
	}

	void mt5510(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bank_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
};

void mt5510_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x2000);
}

void mt5510_state::machine_reset()
{
	m_rombank->set_entry(0);
}

u32 mt5510_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void mt5510_state::bank_w(u8 data)
{
	m_rombank->set_entry(BIT(data, 2, 2));
}

void mt5510_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0).nopw();
	map(0x8000, 0x9fff).bankr("rombank");
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xdfff).ram().share("charram");
	map(0xe000, 0xffff).ram().share("attrram");
}

void mt5510_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x6f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0x71, 0x71).w(FUNC(mt5510_state::bank_w));
}

static INPUT_PORTS_START(mt5510)
INPUT_PORTS_END

void mt5510_state::mt5510(machine_config &config)
{
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mt5510_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mt5510_state::io_map);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL));
	duart.irq_cb().set_inputline(m_maincpu, 0);
	duart.outport_cb().set("eeprom1", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::di_write)).bit(5);
	duart.outport_cb().append("eeprom1", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	duart.outport_cb().append("eeprom1", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);
	duart.outport_cb().append("eeprom2", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(3);

	EEPROM_93C46_16BIT(config, "eeprom1").do_callback().set("duart", FUNC(scn2681_device::ip6_w));

	EEPROM_93C46_16BIT(config, "eeprom2").do_callback().set("duart", FUNC(scn2681_device::ip5_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(45.8304_MHz_XTAL / 2, 1120, 0, 960, 341, 0, 300); // wild guess at resolution
	screen.set_screen_update(FUNC(mt5510_state::screen_update));
}


/**************************************************************************************************************

Micro-Term 5510.
Chips: Z80, SCN2681, S8842C4/SCX6244UNT, 4x CXK5864BP-70L, 2x NMC9346N
Crystals: 6.000, 3.68640, 45.8304

***************************************************************************************************************/

ROM_START( mt5510 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "2500_m.p._r1.9.u11", 0x00000, 0x10000, CRC(71f19a53) SHA1(91df26d46a93359cd033d7137f1676bcfa58223b) )
ROM_END

} // anonymous namespace

COMP(1988, mt5510, 0, 0, mt5510, mt5510, mt5510_state, empty_init, "Microterm", "Microterm 5510", MACHINE_IS_SKELETON)
