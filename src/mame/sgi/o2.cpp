// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI O2 workstation skeleton driver

    To Do: Everything

    Memory map:
    00000000 - 0fffffff      RAM (256mbyte mirror)
    14000000 - 15ffffff      CRIME
    1f000000 - 1f3fffff      MACE
    1fc00000 - 1fc7ffff      Boot ROM
    40000000 - 7fffffff      RAM

NOTE: The default Sgi O2 Keyboard (Model No. RT6856T, Part No. 121472-101-B,
      Sgi No. 062-0002-001) has a Zilog "RT101+228A" MCU, which is really a
      Zilog Z8615 (a Z8-based PC keyboard controller) with 4K ROM (undumped).
      It might have a custom ROM, since it had special marking.

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "machine/ds17x85.h"
#include "mace.h"
#include "crime.h"


namespace {

class o2_state : public driver_device
{
public:
	o2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mace(*this, "mace")
		, m_crime(*this, "crime")
	{
	}

	void o2(machine_config &config);

protected:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<r5000be_device> m_maincpu;
	required_device<mace_device> m_mace;
	required_device<crime_device> m_crime;
};

void o2_state::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("bank1");
	map(0x14000000, 0x15ffffff).m(m_crime, FUNC(crime_device::map));
	map(0x1f000000, 0x1f3fffff).m(m_mace, FUNC(mace_device::map));
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
	static const char* const bank_names[8] = { "bank1", "bank2", "bank3", "bank4", "bank5", "bank6", "bank7", "bank8" };
	for (uint32_t bank = 0; bank < 8; bank++)
	{
		const uint32_t base_addr = 0x40000000 + bank * 0x08000000;
		const uint32_t bank_size_32m = 0x02000000;
		for (uint32_t mirror = 0; mirror < 4; mirror++)
		{
			const uint32_t start_addr = base_addr + mirror * bank_size_32m;
			const uint32_t end_addr = base_addr + (mirror + 1) * bank_size_32m - 1;
			map(start_addr, end_addr).ram().share(bank_names[bank]);
		}
	}
}

static INPUT_PORTS_START( o2 )
INPUT_PORTS_END

void o2_state::o2(machine_config &config)
{
	R5000BE(config, m_maincpu, 60000000*3);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &o2_state::mem_map);
	m_maincpu->set_force_no_drc(true);

	SGI_MACE(config, m_mace, m_maincpu);
	m_mace->rtc_read_callback().set("rtc", FUNC(ds17x85_device::read_direct));
	m_mace->rtc_write_callback().set("rtc", FUNC(ds17x85_device::write_direct));

	SGI_CRIME(config, m_crime, m_maincpu);

	DS1687(config, "rtc", 32768);
}

ROM_START( o2 )
	ROM_REGION64_BE( 0x80000, "user1", 0 )

	ROM_SYSTEM_BIOS( 0, "ip32prom_4_3",  "IP32 PROM V4.3" )
	ROMX_LOAD( "ip32prom.rev4.3.bin",  0x000000, 0x080000, CRC(029f3d06) SHA1(cf5a31299131488d8aaf085054d2a6614bb16974), ROM_GROUPDWORD | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "ip32prom_4_18", "IP32 PROM V4.18" )
	ROMX_LOAD( "ip32prom.rev4.18.bin", 0x000000, 0x080000, CRC(02b3c53d) SHA1(f2cfa7246d67f88fe5490e40dac6c04b1deb4d28), ROM_GROUPDWORD | ROM_BIOS(1))
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS     INIT        COMPANY                  FULLNAME  FLAGS
COMP( 1996, o2,   0,      0,      o2,      o2,    o2_state, empty_init, "Silicon Graphics Inc.", "O2",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
