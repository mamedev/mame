// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    PEGASOS II

    Hardware:
    - PowerPC 750CXe (600 MHz) or MPC7447 (1 GHz)
    - Marvell Discovery II MV64361 northbridge
    - VIA 8231 southbridge
    - W83194 clock generator
    - DDR SDRAM memory, up to 2 GB
    - 1x AGP, 3x PCI slots
    - 2xLAN (Gigabit from northbridge, 100 MBit from southbridge)
    - VIA VT6306 (firewire)
    - AC'97 sound (Sigmatel STAC9766 Codec, v2.3)
    - Floppy
    - PS/2 keyboard/mouse
    - Joystick

    TODO:
    - Almost everything

    Notes:
    - Designed by bplan GmbH

***************************************************************************/

#include "emu.h"
#include "bus/dimm/dimm.h"
#include "bus/pci/pci_slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/powerpc/ppc.h"
#include "machine/input_merger.h"
#include "machine/mv6436x.h"
#include "machine/pci.h"
#include "machine/vt8231_acpi.h"
#include "machine/vt8231_isa.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pegasos2_state : public driver_device
{
public:
	pegasos2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_syscon(*this, "syscon"),
		m_smbus_data(*this, "smbus_data"),
		m_dimm(*this, "dimm%u", 0U)
	{
	}

	void pegasos2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;
	required_device<mv64361_device> m_syscon;
	required_device<input_merger_all_high_device> m_smbus_data;
	required_device_array<ddr_sdram_slot_device, 2> m_dimm;

	void mem_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pegasos2_state::mem_map(address_map &map)
{
	map(0xfff00000, 0xfff7ffff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( pegasos2 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pegasos2_state::machine_start()
{
}

void pegasos2_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( com1_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_115200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void pegasos2_state::pegasos2(machine_config &config)
{
	PPC750(config, m_maincpu, 600'000'000); // TODO: PowerPC 750CXe
	m_maincpu->set_bus_frequency(133'333'333);
	m_maincpu->set_addrmap(AS_PROGRAM, &pegasos2_state::mem_map);

	DDR_SDRAM_SLOT(config, m_dimm[0], "infineon_256mb_pc333").set_address(0x56);
	m_dimm[0]->sda_cb().set(m_smbus_data, FUNC(input_merger_device::in_w<1>));

	DDR_SDRAM_SLOT(config, m_dimm[1], nullptr).set_address(0x57);
	m_dimm[1]->sda_cb().set(m_smbus_data, FUNC(input_merger_device::in_w<2>));

	MV64361(config, m_syscon, m_maincpu, "pci0:00.0", "pci1:00.0");
	m_syscon->set_cs_tag(0, "^dimm1:rank0");
	m_syscon->set_cs_tag(1, "^dimm1:rank1");
	m_syscon->set_cs_tag(2, "^dimm0:rank0");
	m_syscon->set_cs_tag(3, "^dimm0:rank1");

	PCI_ROOT(config, "pci0");
	MV64361_PCI_HOST(config, "pci0:00.0", 66'666'666);

	PCI_SLOT(config, "pci0:1", agp_cards, 1, 0, 1, 2, 3, nullptr);

	PCI_ROOT(config, "pci1");
	MV64361_PCI_HOST(config, "pci1:00.0", 33'333'333);

	PCI_SLOT(config, "pci1:2", pci_cards, 7, 1, 2, 3, 0, nullptr);
	PCI_SLOT(config, "pci1:3", pci_cards, 6, 2, 3, 0, 1, nullptr);
	PCI_SLOT(config, "pci1:4", pci_cards, 5, 3, 0, 1, 2, nullptr);

	vt8231_isa_device &isa(VT8231_ISA(config, "pci1:0c.0"));
	isa.com1_txd_cb().set("com1", FUNC(rs232_port_device::write_txd));
	isa.com1_dtr_cb().set("com1", FUNC(rs232_port_device::write_dtr));
	isa.com1_rts_cb().set("com1", FUNC(rs232_port_device::write_rts));

	vt8231_acpi_device &acpi(VT8231_ACPI(config, "pci1:0c.4"));
	acpi.smbck1_cb().set(m_dimm[0], FUNC(ddr_sdram_slot_device::scl_w));
	acpi.smbck1_cb().append(m_dimm[1], FUNC(ddr_sdram_slot_device::scl_w));
	acpi.smbdt1_cb().set(m_smbus_data, FUNC(input_merger_device::in_w<0>));
	acpi.smbdt1_cb().append(m_dimm[0], FUNC(ddr_sdram_slot_device::sda_w));
	acpi.smbdt1_cb().append(m_dimm[1], FUNC(ddr_sdram_slot_device::sda_w));

	INPUT_MERGER_ALL_HIGH(config, m_smbus_data);
	m_smbus_data->output_handler().set(acpi, FUNC(vt8231_acpi_device::smbdt1_w));

	// IEEE 1394: PCI device 1, INTA# -> INTD

	rs232_port_device &com1(RS232_PORT(config, "com1", default_rs232_devices, "terminal"));
	com1.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(com1_defaults));
	com1.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(com1_defaults));
	com1.rxd_handler().set("pci1:0c.0", FUNC(vt8231_isa_device::com1_rxd_w));
	com1.dcd_handler().set("pci1:0c.0", FUNC(vt8231_isa_device::com1_dcd_w));
	com1.dsr_handler().set("pci1:0c.0", FUNC(vt8231_isa_device::com1_dsr_w));
	com1.ri_handler().set("pci1:0c.0", FUNC(vt8231_isa_device::com1_ri_w));
	com1.cts_handler().set("pci1:0c.0", FUNC(vt8231_isa_device::com1_cts_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pegasos2 )
	ROM_REGION(0x80000, "maincpu", ROMREGION_64BIT | ROMREGION_BE)
	ROM_DEFAULT_BIOS("v11")
	ROM_SYSTEM_BIOS( 0, "v11", "1.1 2004-04-05" ) // extracted from 'up050404'
	ROMX_LOAD("pegasos2_11_2004_04_05.rom", 0x00000, 0x80000, CRC(7e992266) SHA1(08dc28afb3d10fb223376a28eebfd07c9f8df9fa), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v12", "1.2 2004-10-08" ) // from 'update_1.2_08.10.2004.gz'
	ROMX_LOAD("pegasos2_12_2004-10-08.rom", 0x00000, 0x80000, CRC(125cd766) SHA1(f031142b7990df741dca06f89dcd84fc5f24a2a9), ROM_BIOS(1) )
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME      FLAGS
COMP( 2003, pegasos2, 0,      0,      pegasos2, pegasos2, pegasos2_state, empty_init, "Genesi", "PEGASOS II", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
