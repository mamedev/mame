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


#include "softbox.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "z80"
#define I8251_TAG       "ic15"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define COM8116_TAG     "ic14"
#define RS232_TAG       "rs232"
#define CORVUS_HDC_TAG  "corvus"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SOFTBOX = &device_creator<softbox_device>;


//-------------------------------------------------
//  ROM( softbox )
//-------------------------------------------------

ROM_START( softbox )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("19830609")
	ROM_SYSTEM_BIOS( 0, "19810908", "8/9/81" )
	ROMX_LOAD( "375.ic3", 0x000, 0x800, CRC(177580e7) SHA1(af6a97495de825b80cdc9fbf72329d5440826177), ROM_BIOS(1) )
	ROMX_LOAD( "376.ic4", 0x800, 0x800, CRC(edfee5be) SHA1(5662e9071cc622a1c071d89b00272fc6ba122b9a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "19811027", "27-Oct-81" )
	ROMX_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b), ROM_BIOS(2) )
	ROMX_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "19830609", "09-June-1983" )
	ROMX_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048), ROM_BIOS(3) )
	ROMX_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *softbox_device::device_rom_region() const
{
	return ROM_NAME( softbox );
}


//-------------------------------------------------
//  ADDRESS_MAP( softbox_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_mem, AS_PROGRAM, 8, softbox_device )
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_io, AS_IO, 8, softbox_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x09, 0x09) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(dbrg_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(I8255_0_TAG, i8255_device, read, write)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(I8255_1_TAG, i8255_device, read, write)
	AM_RANGE(0x18, 0x18) AM_DEVREADWRITE(CORVUS_HDC_TAG, corvus_hdc_t, read, write)
ADDRESS_MAP_END



//-------------------------------------------------
//  I8255A 0 Interface
//-------------------------------------------------

READ8_MEMBER( softbox_device::ppi0_pa_r )
{
	return m_bus->dio_r() ^ 0xff;
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

	UINT8 data = 0;

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

	UINT8 status = m_hdc->status_r(space, 0);
	UINT8 data = 0;

	data |= (status & CONTROLLER_BUSY) ? 0 : 0x10;
	data |= (status & CONTROLLER_DIRECTION) ? 0 : 0x20;

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

	output_set_led_value(LED_A, !BIT(data, 0));
	output_set_led_value(LED_B, !BIT(data, 1));
	output_set_led_value(LED_READY, !BIT(data, 2));
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
//  MACHINE_CONFIG_FRAGMENT( softbox )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( softbox )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(softbox_mem)
	MCFG_CPU_IO_MAP(softbox_io)

	// devices
	MCFG_DEVICE_ADD(I8251_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_dsr))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD(I8255_0_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(softbox_device, ppi0_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(softbox_device, ppi0_pb_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("SW1"))

	MCFG_DEVICE_ADD(I8255_1_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(softbox_device, ppi1_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(softbox_device, ppi1_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(softbox_device, ppi1_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(softbox_device, ppi1_pc_w))

	MCFG_DEVICE_ADD(COM8116_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_txc))

	MCFG_DEVICE_ADD(CORVUS_HDC_TAG, CORVUS_HDC, 0)
	MCFG_HARDDISK_ADD("harddisk1")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk2")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk3")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk4")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	//MCFG_IMI7000_BUS_ADD("imi5000h", NULL, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor softbox_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( softbox );
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

softbox_device::softbox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SOFTBOX, "PET SoftBox", tag, owner, clock, "pet_softbox", __FILE__),
		device_ieee488_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG),
		m_dbrg(*this, COM8116_TAG),
		m_hdc(*this, CORVUS_HDC_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void softbox_device::device_start()
{
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
