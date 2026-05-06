// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    otomedius.cpp: Konami Otomedius (and maybe related Konami PC-based stuff)

    Skeleton by R. Belmont

    Hardware for Otomedius:
        - Intel Socket 478 Celeron CPU, 2.5 GHz, S-Spec "SL6ZY"
          More info: http://www.cpu-world.com/sspec/SL/SL6ZY.html
        - Intel 82865G northbridge "Springdale-G"
        - Intel 82801EB southbridge / "ICH5" Super I/O
        - 512MB of system RAM
        - ATI-branded Radeon 9600XT AGP video card with 128 MB of VRAM
        - Konami protection dongle marked "GEGGG JA-B"
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class konami_pc_state : public driver_device
{
public:
	konami_pc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void konami_pc(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void konami_pc_map(address_map &map) ATTR_COLD;
};

void konami_pc_state::konami_pc_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000f0000, 0x000fffff).rom().region("maincpu", 0x70000);
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0);
}


void konami_pc_state::konami_pc(machine_config &config)
{
	// Socket 478
	PENTIUM3(config, m_maincpu, 100000000); // actually Celeron
	m_maincpu->set_addrmap(AS_PROGRAM, &konami_pc_state::konami_pc_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( otomedius )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sst49fl004b.u18", 0x000000, 0x080000, CRC(bb9f4e3e) SHA1(95b393a38a5eded3204debfe7a88cc7ea15adf9a) )

	ROM_REGION( 0x10000, "vbios", 0 )   // video card BIOS
	ROM_LOAD( "ati.9600xt.128.samsung.031113.rom", 0x000000, 0x00d000, CRC(020ec211) SHA1(3860c980106f00e5259ecd8d4cd2f9b3fca2428a) )

	DISK_REGION( "ide:0:hdd" ) // Seagate ST340015A 40GB PATA drive
	DISK_IMAGE( "otomedius", 0, SHA1(9283f8b7cd747be7b8e7321953adbf6cbe926f25) )
ROM_END

} // anonymous namespace


GAME( 2007, otomedius,  0,   konami_pc, 0, konami_pc_state, empty_init, ROT0, "Konami", "Otomedius (ver GGG:J:A:A:2008041801)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
