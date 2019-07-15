// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
/**********************************************************************

    SSE SoftBox emulation

**********************************************************************/

/*
    This is an emulation of the SoftBox as a PET/CBM peripheral, where
    the PET is used as a terminal over IEEE-488.  For the standalone
    mode where an RS-232 terminal is used, and also information on
    how to set up the Corvus drive, see: src/mame/drivers/softbox.c.
*/


#include "emu.h"
#include "softbox.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "z80"
#define I8251_TAG       "ic15"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SOFTBOX, softbox_device, "pet_softbox", "SSE SoftBox")


//-------------------------------------------------
//  ROM( softbox )
//-------------------------------------------------

ROM_START( softbox )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("19830609")
	ROM_SYSTEM_BIOS( 0, "19810908", "8/9/81" )
	ROMX_LOAD( "375.ic3", 0x000, 0x800, CRC(177580e7) SHA1(af6a97495de825b80cdc9fbf72329d5440826177), ROM_BIOS(0) )
	ROMX_LOAD( "376.ic4", 0x800, 0x800, CRC(edfee5be) SHA1(5662e9071cc622a1c071d89b00272fc6ba122b9a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "19811027", "27-Oct-81" )
	ROMX_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b), ROM_BIOS(1) )
	ROMX_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "19830609", "09-June-1983" )
	ROMX_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048), ROM_BIOS(2) )
	ROMX_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *softbox_device::device_rom_region() const
{
	return ROM_NAME( softbox );
}


//-------------------------------------------------
//  ADDRESS_MAP( softbox_mem )
//-------------------------------------------------

void softbox_device::softbox_mem(address_map &map)
{
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xffff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

void softbox_device::softbox_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x08).rw(I8251_TAG, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x09, 0x09).rw(I8251_TAG, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x0c, 0x0c).w(FUNC(softbox_device::dbrg_w));
	map(0x10, 0x13).rw(I8255_0_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x14, 0x17).rw(I8255_1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x18, 0x18).rw(m_hdc, FUNC(corvus_hdc_device::read), FUNC(corvus_hdc_device::write));
}



//-------------------------------------------------
//  I8255A 0 Interface
//-------------------------------------------------

READ8_MEMBER( softbox_device::ppi0_pa_r )
{
	return m_bus->read_dio() ^ 0xff;
}

WRITE8_MEMBER( softbox_device::ppi0_pb_w )
{
	m_bus->dio_w(this, data ^ 0xff);
}

//-------------------------------------------------
//  I8255A 1 Interface
//-------------------------------------------------

READ8_MEMBER( softbox_device::ppi1_pa_r )
{
	/*

	  bit     description

	  PA0     ATN
	  PA1     DAV
	  PA2     NDAC
	  PA3     NRFD
	  PA4     EOI
	  PA5     SRQ
	  PA6     REN
	  PA7     IFC

	*/

	uint8_t data = 0;

	data |= !m_bus->atn_r();
	data |= !m_bus->dav_r() << 1;
	data |= !m_bus->ndac_r() << 2;
	data |= !m_bus->nrfd_r() << 3;
	data |= !m_bus->eoi_r() << 4;
	data |= !m_bus->srq_r() << 5;
	data |= !m_bus->ren_r() << 6;
	data |= !m_bus->ifc_r() << 7;

	return data;
}

WRITE8_MEMBER( softbox_device::ppi1_pb_w )
{
	/*

	  bit     description

	  PB0     ATN
	  PB1     DAV
	  PB2     NDAC
	  PB3     NRFD
	  PB4     EOI
	  PB5     SRQ
	  PB6     REN
	  PB7     IFC

	*/

	m_bus->atn_w(this, !BIT(data, 0));
	m_bus->dav_w(this, !BIT(data, 1));
	m_bus->ndac_w(this, !BIT(data, 2));
	m_bus->nrfd_w(this, !BIT(data, 3));
	m_bus->eoi_w(this, !BIT(data, 4));
	m_bus->srq_w(this, !BIT(data, 5));
	m_bus->ren_w(this, !BIT(data, 6));
	m_bus->ifc_w(this, !BIT(data, 7));
}

READ8_MEMBER( softbox_device::ppi1_pc_r )
{
	/*

	  bit     description

	  PC0
	  PC1
	  PC2
	  PC3
	  PC4     Corvus READY
	  PC5     Corvus DIRC
	  PC6
	  PC7

	*/

	uint8_t status = m_hdc->status_r(space, 0);
	uint8_t data = 0;

	data |= (status & corvus_hdc_device::CONTROLLER_BUSY) ? 0 : 0x10;
	data |= (status & corvus_hdc_device::CONTROLLER_DIRECTION) ? 0 : 0x20;

	return data;
}

WRITE8_MEMBER( softbox_device::ppi1_pc_w )
{
	/*

	  bit     description

	  PC0     LED "A"
	  PC1     LED "B"
	  PC2     LED "READY"
	  PC3
	  PC4
	  PC5
	  PC6
	  PC7

	*/

	m_leds[LED_A] = BIT(~data, 0);
	m_leds[LED_B] = BIT(~data, 1);
	m_leds[LED_READY] = BIT(~data, 2);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void softbox_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &softbox_device::softbox_mem);
	m_maincpu->set_addrmap(AS_IO, &softbox_device::softbox_io);

	// devices
	i8251_device &i8251(I8251(config, I8251_TAG, 0));
	i8251.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	i8251.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	i8251.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	i8255_device &ppi0(I8255A(config, I8255_0_TAG));
	ppi0.in_pa_callback().set(FUNC(softbox_device::ppi0_pa_r));
	ppi0.out_pb_callback().set(FUNC(softbox_device::ppi0_pb_w));
	ppi0.in_pc_callback().set_ioport("SW1");

	i8255_device &ppi1(I8255A(config, I8255_1_TAG));
	ppi1.in_pa_callback().set(FUNC(softbox_device::ppi1_pa_r));
	ppi1.out_pb_callback().set(FUNC(softbox_device::ppi1_pb_w));
	ppi1.in_pc_callback().set(FUNC(softbox_device::ppi1_pc_r));
	ppi1.out_pc_callback().set(FUNC(softbox_device::ppi1_pc_w));

	COM8116(config, m_dbrg, 5.0688_MHz_XTAL);
	m_dbrg->fr_handler().set(I8251_TAG, FUNC(i8251_device::write_rxc));
	m_dbrg->ft_handler().set(I8251_TAG, FUNC(i8251_device::write_txc));

	CORVUS_HDC(config, m_hdc, 0);
	HARDDISK(config, "harddisk1", "corvus_hdd");
	HARDDISK(config, "harddisk2", "corvus_hdd");
	HARDDISK(config, "harddisk3", "corvus_hdd");
	HARDDISK(config, "harddisk4", "corvus_hdd");
	//imi7000_bus_device::add_config(config, "imi5000h", nullptr, nullptr, nullptr);
}


//-------------------------------------------------
//  INPUT_PORTS( softbox )
//-------------------------------------------------

INPUT_PORTS_START( softbox )
	/* An 8-position DIP switch may be installed at SW1.  Some
	   SoftBox units have it and some do not.  The switches are
	   not used by the SoftBox BIOS. */
	PORT_START("SW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor softbox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( softbox );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  softbox_device - constructor
//-------------------------------------------------

softbox_device::softbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SOFTBOX, tag, owner, clock)
	, device_ieee488_interface(mconfig, *this)
	, m_maincpu(*this, Z80_TAG)
	, m_dbrg(*this, "ic14")
	, m_hdc(*this, "corvus")
	, m_leds(*this, "led%u", 0U)
	, m_ifc(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void softbox_device::device_start()
{
	m_leds.resolve();
}


//-------------------------------------------------
//  device_reset_after_children - device-specific
//    reset that must happen after child devices
//    have performed their resets
//-------------------------------------------------

void softbox_device::device_reset_after_children()
{
	/* The Z80 starts at address 0x0000 but the SoftBox has RAM there and
	   needs to start from the BIOS at 0xf000.  The PCB has logic and a
	   74S287 PROM that temporarily changes the memory map so that the
	   IC3 EPROM at 0xf000 is mapped to 0x0000 for the first instruction
	   fetch only.  The instruction normally at 0xf000 is an absolute jump
	   into the BIOS.  On reset, the Z80 will fetch it from 0x0000 and set
	   its PC, then the normal map will be restored before the next
	   instruction fetch.  Here we just set the PC to 0xf000 after the Z80
	   resets, which has the same effect. */

	m_maincpu->set_state_int(Z80_PC, 0xf000);
}


//-------------------------------------------------
//  ieee488_ifc - interface clear (reset)
//-------------------------------------------------

void softbox_device::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}


//-------------------------------------------------
//  dbrg_w - baud rate selection
//-------------------------------------------------

WRITE8_MEMBER( softbox_device::dbrg_w )
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}
