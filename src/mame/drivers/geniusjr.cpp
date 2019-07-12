// license:BSD-3-Clause
// copyright-holders:AJR
/*

VTech Genius Junior series

CPU is 68HC05 derived?

*/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"
#include "softlist_dev.h"


class geniusjr_state : public driver_device
{
public:
	geniusjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
	{
	}

	void gj4000(machine_config &config);
	void gj5000(machine_config &config);
	void gjrstar(machine_config &config);
	void gjmovie(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	void gj4000_map(address_map &map);
	void gj5000_map(address_map &map);
	void gjrstar_map(address_map &map);

	required_device<m68hc05_device> m_maincpu;
	required_memory_bank m_rombank;

	u16 m_bank_size;
};

void geniusjr_state::gj4000_map(address_map &map)
{
	map(0x8000, 0xffff).bankr("rombank");
}

void geniusjr_state::gj5000_map(address_map &map)
{
	map(0x4000, 0x7fff).bankr("rombank");
}

void geniusjr_state::gjrstar_map(address_map &map)
{
	map(0x2000, 0x3fff).bankr("rombank");
}


INPUT_PORTS_START( geniusjr )
INPUT_PORTS_END


void geniusjr_state::machine_start()
{
	memory_region *extrom = memregion("extrom");

	m_rombank->configure_entries(0, extrom->bytes() / m_bank_size, extrom->base(), m_bank_size);
	m_rombank->set_entry(0);
}

void geniusjr_state::gj4000(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gj4000_map);

	m_bank_size = 0x8000;
}

void geniusjr_state::gj5000(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock (type also uncertain)
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gj5000_map);

	m_bank_size = 0x4000;
}

void geniusjr_state::gjrstar(machine_config &config)
{
	M68HC05L9(config, m_maincpu, 8'000'000); // unknown clock (type also uncertain)
	m_maincpu->set_addrmap(AS_PROGRAM, &geniusjr_state::gjrstar_map);

	m_bank_size = 0x2000;
}

void geniusjr_state::gjmovie(machine_config &config)
{
	gjrstar(config);

	SOFTWARE_LIST(config, "cart_list").set_original("gjmovie");
}


ROM_START( gj4000 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-05886-000-000.u4", 0x000000, 0x40000, CRC(5f6db95b) SHA1(fe683154e33a82ea38696096616d11e850e0c7a3))
ROM_END

ROM_START( gj5000 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x80000, "extrom", 0 )
	ROM_LOAD( "27-6019-01.u2", 0x000000, 0x80000, CRC(946e5b7d) SHA1(80963d6ad80d49e54c8996bfc77ac135c4935be5))
ROM_END

ROM_START( gjmovie )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "lh532hlk.bin", 0x000000, 0x40000, CRC(2e64c296) SHA1(604034f902e20851cb9af60964031a508ceef83e))
ROM_END

ROM_START( gjrstar )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f))
ROM_END

ROM_START( gjrstar2 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "27-5740-00.u1", 0x000000, 0x40000, CRC(ff3dc3bb) SHA1(bc16dfc1e12b0008456c700c431c8df6263b671f))     // identical to 'Genius Junior Redstar'
ROM_END

ROM_START( gjrstar3 )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "hc05_internal.bin", 0x0000, 0x1fff, NO_DUMP )

	ROM_REGION( 0x40000, "extrom", 0 )
	ROM_LOAD( "54-06056-000-000.u3", 0x000000, 0x040000, CRC(72522179) SHA1(ede9491713ad018012cf925a519bcafe126f1ad3))
ROM_END


//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME                             FLAGS
COMP( 1996, gj4000,   0,       0,      gj4000,   geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior 4000 (Germany)",      MACHINE_IS_SKELETON )
COMP( 1993, gjmovie,  0,       0,      gjmovie,  geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior Movie (Germany)",     MACHINE_IS_SKELETON )
COMP( 1996, gjrstar,  0,       0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior Redstar (Germany)",   MACHINE_IS_SKELETON )
COMP( 1996, gjrstar2, gjrstar, 0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior Redstar 2 (Germany)", MACHINE_IS_SKELETON )
COMP( 1998, gjrstar3, 0,       0,      gjrstar,  geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior Redstar 3 (Germany)", MACHINE_IS_SKELETON )
COMP( 1998, gj5000,   0,       0,      gj5000,   geniusjr, geniusjr_state, empty_init, "VTech",  "Genius Junior 5000 (Germany)",      MACHINE_IS_SKELETON )
