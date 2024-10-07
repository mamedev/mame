// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Sega ST-V dev-box unit "837-12764 486 BD FOR ST-V"

Presumably for stvbios "hold F2 at boot -> System Configuration" cart dev mode.

ALi/ACER FINALi 486 PCI motherboard
M1489 A1 (northbridge) + M1487 B1 (southbridge)
M5113 (super i/o, sorta compatible with FDC37C665 as per PSC-586VGA single board)
DP8432V-33
CAK0003UA
2x Altera EPM7032LC44-10 near northbridge
Two empty sockets, u0621 (BIOS extension?) and u0629 (user space?)

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class stvdev_state : public driver_device
{
public:
	stvdev_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void stvdev(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void stvdev_io(address_map &map) ATTR_COLD;
	void stvdev_map(address_map &map) ATTR_COLD;
};


void stvdev_state::stvdev_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void stvdev_state::stvdev_io(address_map &map)
{
}

static INPUT_PORTS_START(stvdev)
INPUT_PORTS_END

void stvdev_state::stvdev(machine_config &config)
{
	// TODO: two other clocks, one at 36 MHz near CAK chip and another at 14.318 MHz near 486 socket
	I486(config, m_maincpu, 33'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &stvdev_state::stvdev_map);
	m_maincpu->set_addrmap(AS_IO, &stvdev_state::stvdev_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START( stvdev )
	ROM_REGION32_LE( 0x20000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "epr-19159.u0628", 0, 0x20000, CRC(70735b87) SHA1(825954978a6b09d307ec33ef46128a45a76ff63b) )
ROM_END

} // anonymous namespace


// NOTE: stvbios mentions HP and SUN archs being supported, "PC" suffix comes from there
COMP(1998, stvdev, 0, 0, stvdev, stvdev, stvdev_state, empty_init, "Sega / ALi", "ST-V 486 dev box PC", MACHINE_IS_SKELETON )
