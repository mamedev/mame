// license:BSD-3-Clause
// copyright-holders:
/* Bounty Hunter

Konami's Boxing Mania clone with punch & kick pads.
Ending can be seen at: https://www.youtube.com/watch?v=xa7GQC8Phfk

https://www.arcade-museum.com/game_detail.php?game_id=13327
- Pentium III 1 GHz
- nVidia GeForce 2 MX200
No other info about the MB used, most notable folders in the HDD dump:
- C:\bh:
\- *.gci files in data folder
  "Global Challanger Image Data Ver1.0.0 Copyright (C) 2002 by GC-Tech CO, LTD"
\- Several logs for *.pro files in data folder, generated with
   "3D Studio MAX PowerRender Exporter v1.05"
- C:\drv:
\- Korean setup files for Windows 98SE
\- VIA VT82C686A/VT82C686B/VT8231/VT8233/VT8233A/VT8233B/VT8235 audio driver files (ComboAudio_a1u300a)
\- VIA Hyperion 4in1 chipset driver (VIA_4IN1_V440V(a)P3)
\- VIA IDE miniport (IDE_MPD3014, VIA Bus Master (Ultra DMA) driver)
- C:\windows:
\- viagart.inf "VIA CPU to AGP controller"
\- viamach.inf "VIA Standard CPU to PCI Bridge" / "VIA Standard PCI to ISA Bridge" /
   "VIA Power Management Controller" / "VIA PCI to PCI Bridge Controller" /
   "VIA I/O APIC Interrupt Controller"
\- viaudio.inf / viaudoem.inf "VIA AC'97 Enhanced Audio Controller (WDM)"
\- nvaml.inf "nVidia Windows 95/98/ME Display Drivers"
\- monitor.pnf (custom DDC setup?)

Running this in pcipc will successfully boot a custom Korean Windows 98SE (no Microsoft splash boot screen), will try to install drivers, punts to a DOS sub-window requiring at least a 256 color mode.

*/


#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"


namespace {

class bntyhunt_state : public driver_device
{
public:
	bntyhunt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void bntyhunt(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	void bntyhunt_map(address_map &map) ATTR_COLD;
};

void bntyhunt_state::bntyhunt_map(address_map &map)
{
}

static INPUT_PORTS_START( bntyhunt )
INPUT_PORTS_END


void bntyhunt_state::bntyhunt(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 200'000'000); // unknown CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &bntyhunt_state::bntyhunt_map);
	m_maincpu->set_disable();

	PCI_ROOT(config, "pci", 0);
	// ...
}


ROM_START(bntyhunt)
	ROM_REGION32_LE(0x40000, "bios", 0)
	ROM_LOAD("bntyhunt.pcbios", 0x000000, 0x40000, NO_DUMP )

	DISK_REGION( "disks" )
	DISK_IMAGE( "bntyhunt", 0, SHA1(e50937d14d5c6adfb5e0012db5a7df090eebc2e1) )
ROM_END

} // anonymous namespace


GAME( 2002?, bntyhunt, 0, bntyhunt, bntyhunt, bntyhunt_state, empty_init, ROT0, "GC-Tech Co., LTD", "Bounty Hunter (GC-Tech Co., LTD)", MACHINE_IS_SKELETON )
