// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    http://www.sapi.cz/tns/hc08.php

    IC104 - address decoder
    /A5, /A3, /A3, /A2
     0    0    0    0     CS0 - not connected
     0    0    0    1     CS1 - SLOT 1/2
     0    0    1    0     CS2 - SLOT 1/2
     0    0    1    1     CS3 - not connected
     0    1    0    0     CS4 - 7474 clear (IC61)
     0    1    0    1     CS5 - 8255 (IC90) and 7474 clear (IC61)
     0    1    1    0     CS6 - 7474 preset (IC81)
     0    1    1    1     CS7 - mod on 7495 (IC62)
     1    0    0    0     CS8 - 7474 (IC77)
     1    0    0    1     CS9 - SLOT 1/2
     1    0    1    0     CS10 - SLOT 1/2
     1    0    1    1     CS11 - 8255 (IC89) and 74154 (IC111)
     1    1    0    0     CS12 - SIO (IC86)
     1    1    0    1     CS13 - PIO (IC51)
     1    1    1    0     CS14 - CTC (IC87)
     1    1    1    1     CS15 - CTC (IC88)

    8 x 4164   64K
    8 x 41256 256K

****************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"

namespace {

class tnshc08_state : public driver_device
{
public:
	tnshc08_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ppi(*this, "ic89")
	{ }

	void tnshc08(machine_config &config);

	u8 ppi_r() { return 0x20; }

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<i8255_device> m_ppi;
};

void tnshc08_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0xffff).ram();
}

void tnshc08_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x13).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input ports */
static INPUT_PORTS_START( tnshc08 )
INPUT_PORTS_END

void tnshc08_state::tnshc08(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12.288_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &tnshc08_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tnshc08_state::io_map);

	I8255(config, m_ppi, 0);
	m_ppi->in_pb_callback().set(FUNC(tnshc08_state::ppi_r));
}


/* ROM definition */
ROM_START( tnshc08 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fl1_29.ic53",  0x0000, 0x0800, CRC(e6581b55) SHA1(fb5ce8b30518f06b144a7c204a3b72fde9cab17c))
	ROM_LOAD( "fl2_29.ic54",  0x0800, 0x0800, CRC(c44d65e7) SHA1(395968b16398dbd89f3fc4b18b4f2f173496eeda))
	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "chargen.ic42", 0x0000, 0x1000, CRC(e607ff0f) SHA1(0c787593e26398d856c3d7c44250e462b138b424))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY            FULLNAME     FLAGS
COMP( 1988, tnshc08, 0,      0,      tnshc08, tnshc08, tnshc08_state, empty_init, u8"JZD Slušovice", "TNS HC-08", MACHINE_IS_SKELETON)
