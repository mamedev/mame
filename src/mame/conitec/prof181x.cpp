// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"

#include "cpu/z180/z180.h"

#define HD64180_TAG "u1"

namespace {

class prof181x_state : public driver_device
{
public:
	prof181x_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, HD64180_TAG)
	{
	}

	void prof181x(machine_config &config);

private:
	required_device<z180_device> m_maincpu;

	virtual void machine_start() override ATTR_COLD;

	void prof181x_mem(address_map &map) ATTR_COLD;
	void prof181x_io(address_map &map) ATTR_COLD;
};

void prof181x_state::prof181x_mem(address_map &map)
{
}

void prof181x_state::prof181x_io(address_map &map)
{
}

static INPUT_PORTS_START( prof181x )
INPUT_PORTS_END

void prof181x_state::machine_start()
{
}

void prof181x_state::prof181x(machine_config &config)
{
	HD64180RP(config, m_maincpu, XTAL(12'288'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &prof181x_state::prof181x_mem);
	m_maincpu->set_addrmap(AS_IO, &prof181x_state::prof181x_io);
}

ROM_START( prof181x )
	ROM_REGION( 0x20000, HD64180_TAG, 0 )
	ROM_LOAD( "prof181x.u13", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x157, "plds", 0 )
	ROM_LOAD( "pal20v8.u14", 0x000, 0x157, CRC(46da52b0) SHA1(c11362223c0d5c57c6ef970e66d674b89d8e7784) )
	ROM_LOAD( "pal20v8.u15", 0x000, 0x157, CRC(19fef936) SHA1(579ad23ee3c0b1c64c584383f9c8085c6ce3d094) )
	ROM_LOAD( "pal20v8.u19", 0x000, 0x157, CRC(69348c3b) SHA1(6eb8432660eb9b639a95b1973a54dab8b99f10ef) )
	ROM_LOAD( "pal20v8.u21", 0x000, 0x157, CRC(6df4e281) SHA1(602fa4637cd9356acc31b2adfb3a084fd5a0bfcb) )
ROM_END

} // anonymous namespace

/*    YEAR  NAME      PARENT COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY                 FULLNAME     FLAGS */
COMP( 1992, prof181x, 0, 	 0,      prof181x, prof181x, prof181x_state, empty_init, "Conitec Datensysteme", "PROF-181X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
