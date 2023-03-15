// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Intellec 8 MCS

A development machine from Intel for the 8008 CPU, with Front Panel.
It has the usual array of switches, lights and buttons.

****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"

namespace {

class intlc8_state : public driver_device
{
public:
	intlc8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void intlc8(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void intlc8_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000, 0x7ff).rom();
	map(0x800, 0xfff).ram();  // no idea how much ram or where
}

void intlc8_state::io_map(address_map &map)
{
	map.unmap_value_high();
}

/* Input ports */
static INPUT_PORTS_START( intlc8 )
INPUT_PORTS_END

void intlc8_state::machine_reset()
{
}

void intlc8_state::machine_start()
{
}

void intlc8_state::intlc8(machine_config &config)
{
	/* basic machine hardware */
	I8008(config, m_maincpu, 800000);   // no idea of clock
	m_maincpu->set_addrmap(AS_PROGRAM, &intlc8_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &intlc8_state::io_map);
}

/* ROM definition */
ROM_START( intlc8 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )  // order of roms is a guess and imo some are missing?
	ROM_LOAD( "miss0.bin",    0x0000, 0x0100, NO_DUMP )
	ROM_LOAD( "miss1.bin",    0x0100, 0x0100, NO_DUMP )
	ROM_LOAD( "rom1.bin",     0x0200, 0x0100, CRC(0ae76bc7) SHA1(374a545cad8406ad862a5f2f1f03c6b6434fd3d8) )
	ROM_LOAD( "rom4.bin",     0x0300, 0x0100, CRC(4340fdfe) SHA1(d37f3bd65c2970736ac075c7e6d3d87d018c3ea4) )
	ROM_LOAD( "rom2.bin",     0x0400, 0x0100, CRC(dd1a71f4) SHA1(e33a2b64bea18c0aa58230167eb23bae464431be) )
	ROM_LOAD( "rom5.bin",     0x0500, 0x0100, CRC(d631224c) SHA1(812d37ac98daf1252bc8d087da679f3ad1f9d961) )
	ROM_LOAD( "rom3.bin",     0x0600, 0x0100, CRC(97c7ab95) SHA1(650316b820b84393bb73c4c56c11e177d658ce4a) )
	ROM_LOAD( "miss7.bin",    0x0700, 0x0100, NO_DUMP )
ROM_END

} // Anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY     FULLNAME         FLAGS
COMP( 1973, intlc8,   0,      0,      intlc8, intlc8,     intlc8_state,   empty_init, "Intel", "Intellec 8 MCS", MACHINE_IS_SKELETON )
