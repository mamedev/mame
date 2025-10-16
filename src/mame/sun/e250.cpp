// license:BSD-3-Clause
// copyright-holders:

/***********************************************************************************************************
Skeleton driver for Sun Microsystems Enterprise 250 Server.

Hardware info about Enterprise 250 Server:
 -Dual 250, 300 or 400 MHz UltraSPARC II with onboard e-cache.
 -1-MB external cache per processor with 250-MHz CPU or 2-MB per processor with 300 or 400 MHz CPU.
 -16 DIMM slots, four banks of four slots. 128 MB to 2 GB total memory capacity.
 -Four full-size slots compliant with PCI specification version 2.1:
    One slot operating at 33- or 66-MHz, 32- or 64-bit data bus width, 3.3 volt
    Three slots operating at 33 MHz, 32- or 64-bit data bus width, 5 volt
 -One 40MB/sec, 68-pin, Ultra SCSI (SCSI-3), 2 channels (synchronous).
 -Two RS-232D/RS423 serial ports (DB25).

Main components found on its main PCB:
 -LSI LSA0009 LSI53C876 PCI to Dual Channel SCSI Multifunction Controller.
 -Sun Microsystems STP2223BGA (PN 100-4611-05) UPA to PCI Interface.
 -Motorola XPC823ZT66B2 RISC Microprocessor, 32-Bit, 66MHz.
 -SMSC LAN91C96 Non-PCI Single-Chip Ethernet Controller.
 -Synergy SY100E11LEJC.
 -Infineon SAB82532N 10 multiprotocol data communication controller with two symmetrical serial channels.
 -Sun Microsystems STP2210QFP (PN 100-3988-01) Uniprocessor System Controller (USC).
 -National Semiconductor ES0012BG DP83840AVCE-1.
 -National Semiconductor NS0052AJ PC87332VLJ SuperI/O III.
 -Sun Microsystems STP2003QFP (PN 100-4183-05) LAN Node Controller.
 -Sun Microsystems STP2202ABGA (ON 100-4671-02).
 -National Semiconductor ES00116AD DP83223V.
 -Analog Devices ADM5180JP.
 -Motorola MC12439.

***********************************************************************************************************/

#include "emu.h"

#include "cpu/powerpc/ppc.h"
#include "cpu/sparc/sparc.h"

#include "machine/pci.h"

#include "softlist_dev.h"

#include "sun_gem.h"
//#include "bus/pci/permedia2.h"


namespace {

class e250_state : public driver_device
{
public:
	e250_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
	{ }

	void e250(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
};


static INPUT_PORTS_START(e250)
INPUT_PORTS_END

void e250_state::e250(machine_config &config)
{
	SPARCV8(config, m_maincpu, 20'000'000); // Actually a UltraSPARC II 250, 300 or 400 MHz
	//SPARCV8(config, m_maincpu, 20'000'000); // Actually a UltraSPARC II 250, 300 or 400 MHz (optional 2nd CPU)

	// For management console
	MPC8240(config, m_subcpu, XTAL(66'000'000)); // Actually a Motorola XPC823ZT66B2

	SOFTWARE_LIST(config, "sun_sparc").set_original("sun_sparc");

	PCI_ROOT(config, "pci", 0);

	// PCI Cards
	SUN_GEM(config, "sun_fcnc_270_4373_07"); // Sun Microsystems 270-4373-07 Gigabit Fiber Channel Network Card
	//PERMEDIA2(config, "permedia2"); // Raptor GFX PCI Video Card
}


ROM_START(e250)
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD( "525-1718-15_a30250_e28f008sa.u2703", 0x000000, 0x100000, CRC(293ac969) SHA1(88fa0c2607bd68bbb209fc8242efeaba60956daa) )

	ROM_REGION(0x400000, "subcpu", 0)
	ROM_LOAD( "525-1724-08_a29898_e28f008sa.u4401", 0x000000, 0x100000, CRC(567fcc56) SHA1(c6a26b34f61559ec49119eed258666318d551378) ) // VxWorks for management console

	ROM_REGION(0x021000, "nvram", 0)
	ROM_LOAD( "525-1726-02_stm48t59y-70p10.u2706",  0x000000, 0x002000, CRC(adc6696f) SHA1(97e4d4709ba739e8adbe5298175d38e826c0321f) )

	ROM_REGION(0x008000, "seeprom", 0)
	ROM_LOAD( "24c02n.u4402",                       0x000000, 0x000100, CRC(e98dd85c) SHA1(014e89081945a6c16e1498a2f10604ce64048ae6) ) // Seems to contain passwords
ROM_END

} // anonymous namespace

COMP( 1998, e250, 0, 0, e250, e250, e250_state, empty_init, "Sun Microsystems", "Enterprise 250", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
