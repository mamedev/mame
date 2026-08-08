// license:BSD-3-Clause
// copyright-holders:
/*********************************************************************************

Skeleton driver for Covielsa "Multigame".
PC hardware:
- Gigabyte GA-8SIMLH Rev 2.0 motherboard.
   * Realtek RTL8100BL Ethernet controller.
   * Winbond W83697HF Super I/O.
   * Realtek ALC650 AC'97 audio.
   * SiS 962L southbridge.
   * SiS 651 northbridge.
- ATI Rage Mobiity-P AGP video.
- Intel Celeron 1.7 GHz (100x17).
- 128 MB RAM DDR 256.
- Barebone S.L. BAR974 industrial case.
- MAME 32 0.58 with a custom frontend.
- External PCB for inputs, audio and video to JAMMA:

       ________________________________
      |                               |
      |                               |
      |                               |
      |                               |
      |                               |
   ___|                               |
  |__                               __|
  |__                              |  |__ Audio (jack)
J |__                              |__|
A |__                                 |
M |__            ______             __|_
M |__           ULN2003A           |    |
A |__                     ______   |    | Video (VGA HD15)
  |__            ______   74HC86D  |____|
  |__            74HC125              |
  |__                 ___           __|_
  |__                |  |          |    |
  |__        MAX232->|__|          |    | Serial RS-232 (DB-9)
  |__                              |____|
     |    ____________________        |
     |   | Atmel AT89C52     |        |
     |   |___________________|        |
     |           Xtal                 |
     |         11.0592 MHz            |
     |________________________________|

*********************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class cmultigame_state : public driver_device
{
public:
	cmultigame_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void cmultigame(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void cmultigame_io(address_map &map) ATTR_COLD;
	void cmultigame_map(address_map &map) ATTR_COLD;
};


void cmultigame_state::cmultigame_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

void cmultigame_state::cmultigame_io(address_map &map)
{
}

static INPUT_PORTS_START(cmultigame)
INPUT_PORTS_END

void cmultigame_state::cmultigame(machine_config &config)
{
	PENTIUM4(config, m_maincpu, 100'000'000); // Intel Celeron 1.7 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cmultigame_state::cmultigame_map);
	m_maincpu->set_addrmap(AS_IO, &cmultigame_state::cmultigame_io);

	PCI_ROOT(config, "pci");
	// ...
}


/* Boots Windows 98 SE and loads "X-Info3 ArcadePC" to set up the arcade-compatible video frequency
   before starting the MAME frontend. */
ROM_START(cmultigame)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_SYSTEM_BIOS( 0, "f12", "F12" )
	ROMX_LOAD( "8simlh.f12.u14",   0x00000, 0x40000, CRC(98c68c69) SHA1(1721e709f11d651215f448ae8c0109a79e379aa6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "12b", "12b" )
	ROMX_LOAD( "8simlhsy.12b.u14", 0x00000, 0x40000, CRC(af8e56bb) SHA1(1a14211a544d3389c849d82fbd487ef42c62efb6), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "f11", "F11" )
	ROMX_LOAD( "8simlh.f11.u14",   0x00000, 0x40000, CRC(4c48f64e) SHA1(e53a467315d9177d768386d763cd39ac478826b1), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "f10", "F10" )
	ROMX_LOAD( "8simlh.f10.u14",   0x00000, 0x40000, CRC(020faaa6) SHA1(2840c6edce8ca9508f21e623ba14faf7385b1b0d), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "f9", "F9" )
	ROMX_LOAD( "8simlh.f9.u14",    0x00000, 0x40000, CRC(785fb849) SHA1(c828265724903b210ddd4641437c1b112d1f177b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "f8", "F8" )
	ROMX_LOAD( "8simlh.f8.u14",    0x00000, 0x40000, CRC(4cb59e7c) SHA1(1a8cbd39f03c31abcf28197485fa89440ba53124), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "f7", "F7" )
	ROMX_LOAD( "8simlh.f7.u14",    0x00000, 0x40000, CRC(13502db1) SHA1(664ba7be6fceb2e73b398988542d8fdbe6960312), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "f1", "F1" )
	ROMX_LOAD( "8simlh.f1.u14",    0x00000, 0x40000, CRC(77f74ddf) SHA1(f45178ce40bc3c4f620c066c05d3b387d663be23), ROM_BIOS(7) )
	ROM_DEFAULT_BIOS("f1") // The older one, but was the one found om the actual machine

	// ATI Rage Mobility-P BIOS
	ROM_REGION( 0x10000, "vga", 0 )
	ROM_LOAD( "mob-p.rom.u5", 0x00000, 0x10000, CRC(03e0b1b0) SHA1(51c4d4ee45d1bbf24ec8da2abb43e479853e52f8) )

	ROM_REGION( 0x2000, "io", 0 )   // I/O card for JAMMA interface
	ROM_LOAD( "covielsa_multigame_2.0_at89c52.u1", 0x0000, 0x2000, NO_DUMP ) // Protected

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "sp-mg-ver.b01__n_x-info_889", 0, SHA1(26811145255566b6bbb0a11450b0fe01a571c9c4) ) // Maxtor AD040H2 labeled "SP/MG/Ver.B01  Nº X-INFO 889"
ROM_END

} // anonymous namespace


GAME(2002, cmultigame, 0, cmultigame, cmultigame, cmultigame_state, empty_init, ROT0, "Covielsa", "Multigame (Covielsa)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
