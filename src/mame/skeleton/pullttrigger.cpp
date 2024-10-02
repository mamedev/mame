// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************

"Pull The Trigger", developed by the Korean company "Digital Silkroad Co. Ltd." and distributed by Covielsa.

Standard PC with:
-Elitegroup L4S5MG/651+ V5.0 motherboard.
-512MB RAM, DDR PC2100 (one SIMM).
-Wibu Systems WibuKey USB security dongle.
-GeForce 4 MX440 8X 64MB DDR TV-OUT video card.
-Exact CPU model and speed unknown.
-Samsung SP0411N/OMD 40GB HDD.

And an external PCB for inputs (guns, etc.) with:
-ATF1502AS
-AT89C51
-AT89C2051
-11.0592 MHz xtal near the AT89C2051
-11.0592 MHz xtal near the AT89C51
-4.9152 MHz xtal near the ATF1502AS.

Also, the machine allows players to keep their scores on military dogtag like cards with a SEEPROM, named
"DSR-PT1 MEMORY" (they were probably for sale).

***********************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class pullttrig_state : public driver_device
{
public:
	pullttrig_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void pullttrig(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void pullttrig_io(address_map &map) ATTR_COLD;
	void pullttrig_map(address_map &map) ATTR_COLD;
};


void pullttrig_state::pullttrig_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void pullttrig_state::pullttrig_io(address_map &map)
{
}

static INPUT_PORTS_START(pullttrig)
INPUT_PORTS_END

void pullttrig_state::pullttrig(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Exact CPU and frequency unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &pullttrig_state::pullttrig_map);
	m_maincpu->set_addrmap(AS_IO, &pullttrig_state::pullttrig_io);

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(pullttrig)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("51402.bin",    0x00000, 0x40000, CRC(33bed7c0) SHA1(169374b6dac5bbba335e113a97ac34dc830c2599))

	ROM_REGION(0x50000, "io", 0)
	ROM_LOAD("at89c51.bin",  0x00000, 0x01000, NO_DUMP) // AT89C51, protected
	ROM_LOAD("at89c2051.u3", 0x10000, 0x04000, NO_DUMP) // 2 Kbytes internal ROM


	DISK_REGION("ide:0:hdd")
	DISK_IMAGE("pullthetrigger", 0, SHA1(438d57499e435e9e4e4a5980e9b1ca0df4f4ccca))
ROM_END

} // anonymous namespace


GAME(2003, pullttrig, 0, pullttrig, pullttrig, pullttrig_state, empty_init, ROT0, "Digital Silkroad", "Pull The Trigger", MACHINE_IS_SKELETON)
