// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

TAB Austria "Silverball" PC-based touch games

The Hardlock ASIC keys were recovered from the protected software. No dongle was dumped, so
the EEPROM contents are reconstructed; original serial numbers and options are unknown.

TODO:
- Silverball I/O controller and sound card.
- i440zx BIOSes crashes when writing Flash ROM sequences at PC=61C51 onward (by saving in CMOS).
- slvrball806: crashes in vmm32.vxd loading, doesn't check the dongle, -chs mismatch with the
  comment. Commented out for now;

===================================================================================================

Hardware overview for "Silverball (8.01)":
Motherboard: Soyo 5EAS / Soyo SY-7IZB+ / Acorp 694XT-694XT1 / others
32768K RAM (4 x 8MB)
Intel Pentium-S CPU 133 MHz (A80502133)

MicroTouch (external)
ESS AudioDrive ES1869FC ISA sound card
S3 Trio64V2/DX PCI VGA (86C775, 512KB RAM)
I/O Controller Silverball 3 (ISA card, Actel A40MX04-F PL84, DB50 external connector)

HDD Samsung Maxtor G1021U2 or other IDE HDD with similar capacity.

The manual points to a HardLock parallel port security dongle, but it was missing
from the dumped machine.

Acorp 694XT-694XT1 is based off VIA Apollo Pro133 (VT82C694 + VT82C686B)
Soyo SY-7IZB is based off Intel 82440ZX
SiS 5600/SiS 5595 is a Slot 1 Pentium II MB, the earlier version of what is in sis630.cpp

**************************************************************************************************/

#include "emu.h"
#include "bus/centronics/hardlock.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/pci-smbus.h"
#include "machine/i82443bx_host.h"
#include "machine/i82371eb_isa.h"
#include "machine/i82371eb_ide.h"
#include "machine/i82371eb_acpi.h"
#include "machine/i82371eb_usb.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pci/virge_pci.h"
#include "machine/w83977tf.h"

#include <array>

namespace {

#define PCI_IDE_ID "pci:07.1"

// These are reconstructed dongle EEPROM contents, not actual data.
constexpr std::array<u16, 64> SLVRBALL720_EEPROM =
{
	0xb406, 0xc71b, 0x9100, 0x8ad8, 0x940e, 0x4038, 0x09a8, 0x200f,
	0x967c, 0x3dd4, 0x342f, 0x16fe, 0x967c, 0x3dd4, 0x342f, 0x16fe,
	0x967c, 0x3dd4, 0x342f, 0x16fe, 0x967c, 0x3dd4, 0x342f, 0x16fe,
	0x967c, 0x3dd4, 0x342f, 0x16fe, 0x967c, 0x3dd4, 0x342f, 0x16fe,
	0x967c, 0x3dd4, 0x342f, 0x16fe, 0x967c, 0x3dd4, 0x342f, 0x16fe,
	0x967c, 0x3dd4, 0x342f, 0x16fe, 0x967c, 0x3dd4, 0x342f, 0x16fe,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};

constexpr std::array<u16, 64> SLVRBALL632_EEPROM =
{
	0x33b5, 0x69f9, 0xa9f8, 0x1965, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0xbd63, 0x26c3, 0xfe4f, 0xd2dd, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0xc7c8, 0x9643, 0x679f, 0x1120, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x970c, 0xfdee, 0x07c6, 0xb831, 0xd303, 0x1212, 0xde35, 0x01ef,
	0xf652, 0xa4dc, 0xdaa1, 0xdbb7, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x4f63, 0xe591, 0xf611, 0x8858,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
};

constexpr std::array<u16, 64> SLVRBALLBU409_EEPROM =
{
	0x33b5, 0x69f9, 0xa9f8, 0x1965, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0xbd63, 0x26c3, 0xfe4f, 0xd2dd, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0xe05b, 0x5886, 0x9419, 0x9f99, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x970c, 0xfdee, 0x07c6, 0xb831, 0x89ea, 0x7ee7, 0xd9e0, 0x43b9,
	0xb35f, 0xcccf, 0x620d, 0xb3ed, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x9dc4, 0xd8a0, 0xaf2b, 0x6c22,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
	0x31fe, 0x7360, 0x6f2b, 0xd1c5, 0x31fe, 0x7360, 0x6f2b, 0xd1c5,
};

struct dongle_data
{
	u16 module;
	u16 seed1;
	u16 seed2;
	u16 seed3;
	const std::array<u16, 64> &eeprom;
};

// Recovered ASIC keys: module address, then seeds 1-3.
constexpr dongle_data SLVRBALL720_DONGLE = { 0x6391, 0xdc31, 0x0cd7, 0x8325, SLVRBALL720_EEPROM };
constexpr dongle_data SLVRBALL632_DONGLE = { 0x6fd6, 0x1db8, 0x9b26, 0x0614, SLVRBALL632_EEPROM };
constexpr dongle_data SLVRBALLBU409_DONGLE = { 0x6fd6, 0x1db8, 0x9b26, 0x0614, SLVRBALLBU409_EEPROM };

class silverball_state : public driver_device
{
public:
	silverball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void silverball_i440zx(machine_config &config);
	void silverball632_i440zx(machine_config &config) ATTR_COLD;
	void silverball720_i440zx(machine_config &config) ATTR_COLD;
	void silverballbu_i440zx(machine_config &config) ATTR_COLD;

private:
	void silverball_map(address_map &map) ATTR_COLD;
	void silverball_io(address_map &map) ATTR_COLD;

	static void i440zx_superio_config(device_t *device) ATTR_COLD;
	static void hardlock_superio_config(device_t *device, const dongle_data &dongle) ATTR_COLD;

	required_device<pentium_device> m_maincpu;
};

void silverball_state::silverball_map(address_map &map)
{
	map.unmap_value_high();
}

void silverball_state::silverball_io(address_map &map)
{
	map.unmap_value_high();
}


static void isa_internal_devices(device_slot_interface &device)
{
	// TODO: additional Winbond W83782M for HW monitoring
	device.option_add("w83977tf", W83977TF);
}

void silverball_state::i440zx_superio_config(device_t *device)
{
	// TODO: unknown sub-type
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//  fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set(":pci:07.0", FUNC(i82371eb_isa_device::a20gate_w));
	fdc.irq1().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371eb_isa_device::pc_irq8n_w));
//  fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//  fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//  fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//  fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}

void silverball_state::hardlock_superio_config(device_t *device, const dongle_data &dongle)
{
	i440zx_superio_config(device);

	auto &port = *device->subdevice<centronics_device>("lpt:centronics");
	port.set_default_option("hardlock");
	port.set_fixed(true);
	port.set_option_machine_config("hardlock", [&dongle] (device_t *device)
	{
		auto &hardlock = *downcast<hardlock_device *>(device);
		hardlock.set_keys(dongle.module, dongle.seed1, dongle.seed2, dongle.seed3);
		hardlock.set_eeprom_data(dongle.eeprom);
	});
}

// SY-7IZB+
void silverball_state::silverball_i440zx(machine_config &config)
{
	PENTIUM2(config, m_maincpu, 133'000'000); // Pentium-S minimum, up to a Pentium II / Celeron Socket 370
	m_maincpu->set_addrmap(AS_PROGRAM, &silverball_state::silverball_map);
	m_maincpu->set_addrmap(AS_IO, &silverball_state::silverball_io);
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82443bx_host_device::smi_act_w));

	PCI_ROOT(config, "pci");
	// i440ZX (BX equivalent)
	// TODO: unknown RAM size
	I82443BX_HOST(config, "pci:00.0", "maincpu", 32*1024*1024);
	I82443BX_BRIDGE(config, "pci:01.0", 0 ); //"pci:01.0:00.0");
	//I82443BX_AGP   (config, "pci:01.0:00.0");

	i82371eb_isa_device &isa(I82371EB_ISA(config, "pci:07.0", m_maincpu, false));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	isa.a20m().set_inputline("maincpu", INPUT_LINE_A20);

	i82371eb_ide_device &ide(I82371EB_IDE(config, PCI_IDE_ID, m_maincpu));
	ide.irq_pri().set("pci:07.0", FUNC(i82371eb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371eb_isa_device::pc_mirq0_w));

	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_default_option("hdd");
	ide.subdevice<bus_master_ide_controller_device>("ide1")->slot(0).set_fixed(true);

	ide.subdevice<bus_master_ide_controller_device>("ide2")->slot(0).set_default_option(nullptr);

	I82371EB_USB (config, "pci:07.2");
	I82371EB_ACPI(config, "pci:07.3");
	ACPI_PIIX4   (config, "pci:07.3:acpi");
	SMBUS        (config, "pci:07.3:smbus", 0);

	// FIXME: determine ISA bus clock
	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", i440zx_superio_config);
	ISA16_SLOT(config, "isa1",   0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2",   0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

	PCI_SLOT(config, "pci:1", pci_cards, 14, 0, 1, 2, 3, "trio64dx");
}

void silverball_state::silverball632_i440zx(machine_config &config)
{
	silverball_i440zx(config);
	subdevice<isa16_slot_device>("board4")->set_option_machine_config("w83977tf",
			[] (device_t *device) { hardlock_superio_config(device, SLVRBALL632_DONGLE); });
}

void silverball_state::silverball720_i440zx(machine_config &config)
{
	silverball_i440zx(config);
	subdevice<isa16_slot_device>("board4")->set_option_machine_config("w83977tf",
			[] (device_t *device) { hardlock_superio_config(device, SLVRBALL720_DONGLE); });
}

void silverball_state::silverballbu_i440zx(machine_config &config)
{
	silverball_i440zx(config);
	subdevice<isa16_slot_device>("board4")->set_option_machine_config("w83977tf",
			[] (device_t *device) { hardlock_superio_config(device, SLVRBALLBU409_DONGLE); });
}


// Silverball machines used different motherboards. By now, we're adding all the known BIOSes here
// TODO: should be separated by MB type
#define SILVERBALL_BIOS \
	ROM_REGION32_LE(0x40000, "pci:07.0", 0) \
	/* Acorp 694XT/694XT1 (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(0, "bios47", "BIOS47 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios47.bin", 0x00000, 0x40000, CRC(248910ed) SHA1(2901c94fb003e5eaea7e91127ad1f7953826a248), ROM_BIOS(0)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(1, "bios46", "BIOS46 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios46.bin", 0x00000, 0x40000, CRC(90074f2b) SHA1(08fdabad6114ede03a15ce51dec775080585b650), ROM_BIOS(1)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(2, "bios45", "BIOS45 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios45.bin", 0x00000, 0x40000, CRC(790d5db0) SHA1(6bbfb35275eb0c7bb740ff9b1e299042d4c5952a), ROM_BIOS(2)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(3, "bios44", "BIOS44 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios44.bin", 0x00000, 0x40000, CRC(a4b2703e) SHA1(9baa5e3aaaf120054979039e42bf1bca2a50616e), ROM_BIOS(3)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	/* No BIOS43? */ \
	ROM_SYSTEM_BIOS(4, "bios42", "BIOS42 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios42.bin", 0x00000, 0x40000, CRC(820c057e) SHA1(b553961c1bbb6b4afa366eb0dba7ed29ca148f5e), ROM_BIOS(4)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(5, "bios41", "BIOS41 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios41.bin", 0x00000, 0x40000, CRC(0940aebd) SHA1(38c8645eed9e17ef8e2cd0856f929697ce13df74), ROM_BIOS(5)) /* 09/03/2001-694X-686A-6A6LJX3AC-00 */ \
	ROM_SYSTEM_BIOS(6, "bios40", "BIOS40 (Acorp 694XT/694XT1)") \
	ROMX_LOAD("bios40.bin", 0x00000, 0x40000, CRC(c5ed7d4f) SHA1(9fcffb385b5d68bf8cf87615ffee781f38f5fb74), ROM_BIOS(6)) /* 694XT TAB-Bios40 03/09/2001 */ \
	/* Unknown PCB with the BIOS string 'SiS-5600-ITE8661' (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(7, "bios39", "BIOS39 (unknown motherboard)") \
	ROMX_LOAD("bios39.bin", 0x00000, 0x40000, CRC(e7541a28) SHA1(dad1aff2cb142d65e7c8736064dce8004451ea4b), ROM_BIOS(7)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(8, "bios38", "BIOS38 (unknown motherboard)") \
	ROMX_LOAD("bios38.bin", 0x00000, 0x40000, CRC(cb49085b) SHA1(867b95ebef8fd53dad5b71cd4430ede08cf03540), ROM_BIOS(8)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(9, "bios37", "BIOS37 (unknown motherboard)") \
	ROMX_LOAD("bios37.bin", 0x00000, 0x40000, CRC(9f35e2d2) SHA1(83026fc9fc72f795bc3229af98099b409c9bdc81), ROM_BIOS(9)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(10, "bios36", "BIOS36 (unknown motherboard)") \
	ROMX_LOAD("bios36.bin", 0x00000, 0x40000, CRC(83ca05e9) SHA1(9d06677907c88481a167e52cc149995cfef0ff40), ROM_BIOS(10)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(11, "bios35", "BIOS35 (unknown motherboard)") \
	ROMX_LOAD("bios35.bin", 0x00000, 0x40000, CRC(d5a2f966) SHA1(4f4c9688501814bc00b4cde58ac590b6e961816e), ROM_BIOS(11)) /* SiS-5600-ITE8661 */ \
	ROM_SYSTEM_BIOS(12, "bios34", "BIOS34 (unknown motherboard)") \
	ROMX_LOAD("bios34.bin", 0x00000, 0x40000, CRC(400702fe) SHA1(63a9e04d2ea3d4ace3e5c9f11cc62d0131bc4af0), ROM_BIOS(12)) /* SiS-5600-ITE8661 */ \
	/* Soyo SY-7IZB+ (all of them from the Silverball software update files) */ \
	ROM_SYSTEM_BIOS(13, "bios33", "BIOS33 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios33.bin", 0x00000, 0x40000, CRC(96658274) SHA1(1a5fc83cf84a4c774abc055688fa0c32c3784086), ROM_BIOS(13)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(14, "bios32", "BIOS32 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios32.bin", 0x00000, 0x40000, CRC(63318d8a) SHA1(1194a30a65186f2d0a9e11742d51232598c16011), ROM_BIOS(14)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(15, "bios31", "BIOS31 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios31.bin", 0x00000, 0x40000, CRC(5121b267) SHA1(97e51ed4d53d19ed69db1faba5395b46a1edb6b0), ROM_BIOS(15)) /* Soyo Computer Inc. SY-7IZB+ */ \
	ROM_SYSTEM_BIOS(16, "bios30", "BIOS30 (Soyo SY-7IZB+)") \
	ROMX_LOAD("bios30.bin", 0x00000, 0x40000, CRC(c6e30fb1) SHA1(1d9a900a5383bf58b66a2e9f7fb34c61397dd327), ROM_BIOS(16)) /* Soyo Computer Inc. SY-7IZB+ */ \
	/* Soyo SY-5EAS */ \
	ROM_SYSTEM_BIOS(17, "bios29", "BIOS29 (Soyo SY-5EAS)") /* Dumped from the actual Silverball 8.06 machine */ \
	ROMX_LOAD("bios29.bin", 0x00000, 0x20000, CRC(ddbd94f4) SHA1(60ad74e56265a7936cf19e8480c657223d11f2d0), ROM_BIOS(17)) /* 06/18/1998-EQ82C6618A-ET-2A5LDS2FC-29 */ \
	ROM_RELOAD( 0x20000, 0x20000 ) \
	ROM_SYSTEM_BIOS(18, "test", "TEST (Soyo SY-5EAS)") /* BIOS update labeled as "TEST", from the Silverball software update files */ \
	ROMX_LOAD("test.bin",   0x00000, 0x20000, CRC(ddbd94f4) SHA1(60ad74e56265a7936cf19e8480c657223d11f2d0), ROM_BIOS(18)) /* 06/18/1998-EQ82C6618A-ET-2A5LDS2FC-29 */ \
	ROM_RELOAD( 0x20000, 0x20000 )


ROM_START(slvrball)
	SILVERBALL_BIOS
	ROM_DEFAULT_BIOS("bios33")
ROM_END

//ROM_START(slvrball806)
//  SILVERBALL_BIOS
//  ROM_DEFAULT_BIOS("bios29") // The one dumped from the actual machine
//  ROM_DEFAULT_BIOS("bios33")
//
//  DISK_REGION( PCI_IDE_ID":ide1:0:hdd" ) // 16383 cylinders, 16 heads, 63 sectors
//  DISK_IMAGE("silverball_8.06", 0, BAD_DUMP SHA1(4bd03240229a2f59d457e95e04837422c423111b)) // May contain operator data
//ROM_END

ROM_START(slvrball720)
	SILVERBALL_BIOS
//  ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from
	ROM_DEFAULT_BIOS("bios33")

	DISK_REGION( PCI_IDE_ID":ide1:0:hdd" )
	DISK_IMAGE("silverball_7.20", 0, BAD_DUMP SHA1(008d0146b579793f9ba2aa2e43ffa7ec1401f752)) // May contain operator data
ROM_END

ROM_START(slvrball632)
	SILVERBALL_BIOS
//  ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from
	ROM_DEFAULT_BIOS("bios33")

	DISK_REGION( PCI_IDE_ID":ide1:0:hdd" )
	DISK_IMAGE("silverball_6.32", 0, BAD_DUMP SHA1(0193fbc3b27e0b3ad6139830dfec04172eb3089a)) // May contain operator data
ROM_END

// SilverBall V4.09 BULOVA: Windows 3.1, HardLock parallel dongle and two VGA drivers loaded, S3 Trio and Chips and Technologies 6555x (mm55x16) Accelerator
ROM_START(slvrballbu409)
	SILVERBALL_BIOS
//  ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from
	ROM_DEFAULT_BIOS("bios33")

	DISK_REGION( PCI_IDE_ID":ide1:0:hdd" )
	DISK_IMAGE("silverball_bulova_4.09_1", 0, BAD_DUMP SHA1(da838ddccf285fb4e06d7f752949e745e6b4e2e7)) // May contain operator data
ROM_END

// SilverBall V4.09 BULOVA: Windows 3.1, HardLock parallel dongle and two VGA drivers loaded, S3 Trio and Chips and Technologies 6555x (mm55x16) Accelerator
// Probably the same as set 1, just with different operator data / configuration
ROM_START(slvrballbu409b)
	SILVERBALL_BIOS
//  ROM_DEFAULT_BIOS("bios29") // Not sure what PCB this HD was dumped from
	ROM_DEFAULT_BIOS("bios33")

	DISK_REGION( PCI_IDE_ID":ide1:0:hdd" )
	DISK_IMAGE("silverball_bulova_4.09_2", 0, BAD_DUMP SHA1(86bf947b39cabcd207f79b7d6132199819e1fed7)) // May contain operator data
ROM_END


} // Anonymous namespace

GAME(199?, slvrball, 0, silverball_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball BIOS", MACHINE_NOT_WORKING | MACHINE_IS_BIOS_ROOT )

//GAME(1997?, slvrball806,    slvrball, silverball_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (8.01)",               MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING)
GAME(1997?, slvrball720,    slvrball, silverball720_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (7.20)",               MACHINE_NOT_WORKING)
GAME(1997?, slvrball632,    slvrball, silverball632_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball (6.32)",               MACHINE_NOT_WORKING)

GAME(199?,  slvrballbu409,  slvrball,      silverballbu_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball Bulova (4.09, set 1)", MACHINE_NOT_WORKING)
GAME(199?,  slvrballbu409b, slvrballbu409, silverballbu_i440zx, 0, silverball_state, empty_init, ROT0, "TAB Austria", "Silverball Bulova (4.09, set 2)", MACHINE_NOT_WORKING) // Probably the same as set 1
