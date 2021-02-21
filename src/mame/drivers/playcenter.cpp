// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for "PlayCenter" PC-based touch games

    Hardware overview for "PlayCenter Champions Tournament":
    Motherboard: EPoX EP-MVP4F
    Unknown RAM size, AMD K6 processor (unknown speed)
    PCI Ethernet card (unknown chipset)

    The security scheme is quite simple:
    Each minigame is a VisualBasic executable that receives three numbers when
    invoked, the hard disk serial number (readed through the Windows enumerator
    file), the Ethernet MAC address, and a third one, and concatenates them
    moving some bits around.
    Then, calls the security board via RS-232 (it's a simple PCB with a PIC and
    a serial port), getting another number (the PIC always returns the same
    number, there are no states or logic).
    Finally, the game compares both numbers with a simple formula, refusing to
    work if there's a mismatch.
    Since the security involves hardware unique serial numbers, each PIC program
    is unique for each single machine (and these hardware parts are not
    replaceble by the user / operator).

*******************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "speaker.h"

namespace {

class playcenter_state : public driver_device
{
public:
	playcenter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void playcenter(machine_config &config);

private:
	void mem_map(address_map &map);
	void io_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};

void playcenter_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0xfffc0000, 0xffffffff).rom().region("mb_bios", 0);
}

void playcenter_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START(playcenter)
INPUT_PORTS_END

void playcenter_state::playcenter(machine_config &config)
{
	PENTIUM(config, m_maincpu, 166'000'000); // Actually an AMD K6, frequency unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &playcenter_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &playcenter_state::io_map);
}

ROM_START(plycntrchtr)
	ROM_REGION32_LE(0x40000, "mb_bios", 0) // Bios date: 03/13/2001
	ROM_LOAD("vp4f1313.bin", 0x00000, 0x40000, CRC(bd4b155f) SHA1(3eafe71e89bf84b72a42e933187676fe08db0492))

	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE("playcenter_epox_9.3_tournament", 0, SHA1(64a88d4ab10d82ba0bd175511242ba6771cfc5ce))
ROM_END

} // Anonymous namespace

GAME(2000, plycntrchtr, 0, playcenter, playcenter, playcenter_state, empty_init, ROT0, "Recreativos Presas / Undergaming", "PlayCenter Champions Tournament (v9.3)", MACHINE_IS_SKELETON)
