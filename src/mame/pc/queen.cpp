// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

"Queen"

Produttore  STG
N.revisione
CPU main PCB is a standard EPIA
ROMs    epia BIOS + solid state HD

main PCB VIA EPIA 5000EAG with:
- VIA Eden Processor (EBGA368 socket), clocked at 533MHz
  (VIA C3 or later, Pentium 3 equivalent + 3DNow as bare minimum)
- VIA VT8601A northbridge (PLE133 chipset)
- VIA VT8231 southbridge
- VIA EPIA Companion Chip VT1612A (2 channel AC'97 CODEC)
- VIA EPIA Companion Chip VT6103 (Networking)
- Max RAM 1 GB

- In shutms11 HDD will boot to a STBOX / STG splash screen at 800x600 res,
  eventually with blinking cursor then full screen blink (?).

TODO:
- dig the dump for serial/text debugging, refs for what video card is used here.
  (some EPIA mounts a Trident 8420B "CyberBlade i7")

**************************************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class queen_state : public driver_device
{
public:
	queen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void queen(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	void queen_io(address_map &map) ATTR_COLD;
	void queen_map(address_map &map) ATTR_COLD;
};


void queen_state::queen_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void queen_state::queen_io(address_map &map)
{
}

void queen_state::queen(machine_config &config)
{
	// TODO: EBGA368 VIA Eden processor really
	// FSB 66/100/133 MHz
	PENTIUM3(config, m_maincpu, 533000000/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &queen_state::queen_map);
	m_maincpu->set_addrmap(AS_IO, &queen_state::queen_io);

	PCI_ROOT(config, "pci");
	// ...
}


ROM_START( queen )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_LOAD( "bios-original.bin", 0x00000, 0x40000, CRC(feb542d4) SHA1(3cc5d8aeb0e3b7d9ed33248a4f3dc507d29debd9) )

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "pqiidediskonmodule", 0, SHA1(a56efcc711b1c5a2e63160b3088001a8c4fb56c2) )
ROM_END

} // anonymous namespace


GAME( 2002?, queen,  0,    queen, 0, queen_state, empty_init, ROT0, "STG", "Queen?", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
