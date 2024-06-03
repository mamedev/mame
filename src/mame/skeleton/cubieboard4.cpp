// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************

Skeleton driver for "Cubieboard4" (CC A-80) and arcade games based on it.

Cubieboard4, also named CC-A80, is a open source mini PC or single board computer.
The main chipset Allwinner A80 is a 28nm Octa-Core A15/A7 big.LITTLE architecture application processor
with a CPU dominant frequency of 2GHz. It also has the GPU 64-core GPU graphics core PowerVR G6230 which
supports openGL ES, OpenGL, and OpenCL.
CC-A80 has the standard interfaces like desktop computer, such as HDMI & VGA, 100M/1000M RJ45, 4 USB2.0
host ports, 1 USB3.0 OTG port, audio output, microphone input, dual-band wifi and bluetooth 4.0, micro SD
card. It has 2GB DDR3 on-board memory and support Li-Po battery UPS powerinput.

Main Cubieboard4 components:
-Allwinner A80.
-Realtek RTL8211E integrated Ethernet transceiver.
-AMPAK Technology AP6330 WiFi + Bluetooth 4.0(HS) + FM Rx Module.
-SKhynix KLM4G1YEMD-B031 (embedded MultiMediaCard Ver. 5.0 compatible, soldered to the back side of the PCB).
-4 x H5TQ4G63AFR 4Gb DDR3 SDRAM.
-X-Powers AXP809 PMIC.
-X-Powers AC100 audio codec and RTC subsystem.

***********************************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"

namespace {

class cubiecca80_state : public driver_device
{
public:
	cubiecca80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cart_ext(*this, "emmcslot"),
		m_cart_int(*this, "sdcardslot"),
		m_maincpu(*this, "maincpu")

	{ }

	void cubiecca80(machine_config &config);

protected:
	optional_device<generic_slot_device> m_cart_ext;
	optional_device<generic_slot_device> m_cart_int;

private:
	required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( cubiecca80 )
INPUT_PORTS_END

void cubiecca80_state::cubiecca80(machine_config &config)
{
	// Basic machine hardware
	ARM9(config, m_maincpu, 200'000'000); // Actually an Allwinner A80 2 GHz

	// Video hardware
	//SCREEN(...)

	// Audio hardware
	//SPEAKER(...)

	GENERIC_CARTSLOT(config, m_cart_ext, generic_plain_slot, "sdcard"); // Removable MicroSD
	GENERIC_CARTSLOT(config, m_cart_int, generic_plain_slot, "emmc");   // Internal eMMC

	// Software list for adding other compatible software (Linux distros, etc.).
	SOFTWARE_LIST(config, "software_list").set_original("cubieboard4");
}

// Main machine, for booting SD card and eMMC images from the software list

ROM_START( cubieboard4 )
ROM_END

// Arcade games

/* Monkey Jump. Android-based arcade from the Spanish company Falgas.
   Has a separate I/O board, with a PIC18F46K22, connected by USB to the Cubieboard4 (using a RS-232 to USB adapter).
   More info: https://www.recreativas.org/monkey-jump-4029-falgas */
ROM_START( monkeyjmp )
	DISK_REGION( "nand" )
	DISK_IMAGE( "mmcblk1", 0, SHA1(5c005005b2ca17b916b2cccd48d291b19337d4cc) )

	ROM_REGION( 0x2000, "io", 0 )
	ROM_LOAD( "pic18f46k22.bin", 0x0000, 0x2000, NO_DUMP ) // 1024 bytes internal ROM
ROM_END

} // anonymous namespace

// Main machine
COMP( 20??, cubieboard4, 0, 0, cubiecca80, cubiecca80, cubiecca80_state, empty_init, "Cubietech Limited", "Cubieboard4 (CC A-20)", MACHINE_IS_SKELETON )

// Arcade games
GAME( 20??, monkeyjmp, 0, cubiecca80, cubiecca80, cubiecca80_state, empty_init, ROT90, "Falgas", "Monkey Jump", MACHINE_IS_SKELETON )
