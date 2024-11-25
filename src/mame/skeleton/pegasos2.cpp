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
    - AC97 sound (Sigmatel STAC 9766 Codec)
    - Floppy
    - PS/2 keyboard/mouse
    - Joystick

    TODO:
    - Everything

    Notes:
    - Designed by bplan GmbH

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/powerpc/ppc.h"
#include "machine/mv6436x.h"
#include "machine/pci.h"
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
		m_syscon(*this, "syscon")
	{
	}

	void pegasos2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;
	required_device<mv64361_device> m_syscon;

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
	PPC604(config, m_maincpu, 100000000); // wrong cpu/clock
	m_maincpu->set_addrmap(AS_PROGRAM, &pegasos2_state::mem_map);

	MV64361(config, m_syscon, 0, m_maincpu, "pci0:00.0", "pci1:00.0");

	PCI_ROOT(config, "pci0", 0);
	MV64361_PCI_HOST(config, "pci0:00.0", 0);

	PCI_ROOT(config, "pci1", 0);
	MV64361_PCI_HOST(config, "pci1:00.0", 0);
	vt8231_isa_device &isa(VT8231_ISA(config, "pci1:0c.0", 0));
	isa.com1_txd_cb().set("com1", FUNC(rs232_port_device::write_txd));
	isa.com1_dtr_cb().set("com1", FUNC(rs232_port_device::write_dtr));
	isa.com1_rts_cb().set("com1", FUNC(rs232_port_device::write_rts));

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
	// extracted from 'up050404'
	ROM_LOAD("pegasos2.rom", 0x00000, 0x80000, CRC(7e992266) SHA1(08dc28afb3d10fb223376a28eebfd07c9f8df9fa))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME      FLAGS
COMP( 2003, pegasos2, 0,      0,      pegasos2, pegasos2, pegasos2_state, empty_init, "Genesi", "PEGASOS II", MACHINE_IS_SKELETON )
