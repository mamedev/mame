// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Agro's ~~Good~~ Fantastic Video Game
(where Good is strikethrough)

Redemption machine, single button.

DOS 6.21 on El Torito CD

- PCChips/Hsin Tech M590 motherboard;
- SiS5591 + SiS5595;
- SiS 3D Pro on-board (SiS6326 equivalent);
- SoundPro HT1869 on-board, CMI8330A/C3D equivalent;
- ITE IT8770F Super I/O, seems a minor undocumented upgrade from IT8661F (cfr. BIOS at $ffe90);
- Unspecified RAM size, 384MB is the max;
- 3x PCI + 2x ISA card slots;
- Winbond W83194R-17 PLL;

Has no config.sys, autoexec.bat just has a BOOT.EXE inside, which should eventually call GAME.EXE
over the same boot partition.

TODO:
- stub;
- On shutms11 pre-breakage it will start MS-DOS then quickly access $1000'0000, is the protection
  device mapping on PCI space?
- Most likely uses SiS6326 TV Out for the actual game, as boot.exe tries to initialize that part;

**************************************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class agro_state : public driver_device
{
public:
	agro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void agro(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void agro_map(address_map &map) ATTR_COLD;
};


void agro_state::agro_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0xe0000);
	map(0xfff00000, 0xffffffff).rom().region("bios", 0);
}

void agro_state::agro(machine_config &config)
{
	// Socket 7 / PGA321
	// dump was using a Cyrix MII-333
	PENTIUM_MMX(config, m_maincpu, 66'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &agro_state::agro_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

ROM_START( agro )
	// AMIBIOS based
	ROM_REGION32_LE( 0x100000, "bios", 0 )
	// BADADDR    -x-xxxxxxxxxxxxxxxxx, oversized?
	ROM_LOAD( "du98504.u38", 0, 0x100000, CRC(f0d44dbe) SHA1(e66e1c4839ef6595e4ced6ac0f3651291abb0b28) )
//	ROM_LOAD( "5901202s.rom", 0xe0000, 0x20000, CRC(2ccdef7a) SHA1(ce2a62540e253679c31dbebd0a9bee7ff0afdeec) )

	DISK_REGION( "pci:07.0:ide1:0:cdrom" )
	DISK_IMAGE( "agro", 0, SHA1(edf27fd243944c0ff1468b532874f15346f74442) )
ROM_END

} // Anonymous namespace


// version 1 is a prototype JAMMA, unknown specifics
// distributed by Associated Leisure
GAME(1997, agro, 0, agro, 0, agro_state, empty_init, ROT0, "Kyle Hodgetts / The Game Room", "Agro's Fantastic Video Game (version 2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
