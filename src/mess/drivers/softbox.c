/*

	SSE SoftBox

	http://mikenaberezny.com/hardware/pet-cbm/sse-softbox-z80-computer/

*/

#include "includes/softbox.h"



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  dbrg_w - baud rate selection
//-------------------------------------------------

WRITE8_MEMBER( softbox_state::dbrg_w )
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( softbox_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_mem, AS_PROGRAM, 8, softbox_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_io, AS_IO, 8, softbox_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x09, 0x09) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(dbrg_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(I8255_0_TAG, i8255_device, read, write)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(I8255_1_TAG, i8255_device, read, write)
	//AM_RANGE(0x18, 0x18) AM_WRITE(corvus_data_r, corvus_data_w)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( softbox )
//-------------------------------------------------

static INPUT_PORTS_START( softbox )
	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  i8251_interface usart_intf
//-------------------------------------------------

static const i8251_interface usart_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, tx_r),
	DEVCB_DEVICE_LINE_MEMBER(TERMINAL_TAG, serial_terminal_device, rx_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi0_intf )
//-------------------------------------------------

READ8_MEMBER( softbox_state::pia0_pa_r )
{
	return m_ieee->dio_r() ^ 0xff;
}

WRITE8_MEMBER( softbox_state::pia0_pb_w )
{
	m_ieee->dio_w(data ^ 0xff);
}

static I8255A_INTERFACE( ppi0_intf )
{
	DEVCB_DRIVER_MEMBER(softbox_state, pia0_pa_r),
	DEVCB_NULL, // Port A write
	DEVCB_NULL, // Port B read
	DEVCB_DRIVER_MEMBER(softbox_state, pia0_pb_w),
	DEVCB_INPUT_PORT("SW1"), // Port C read
	DEVCB_NULL  // Port C write
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi1_intf )
//-------------------------------------------------

READ8_MEMBER( softbox_state::pia1_pa_r )
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

	data |= !m_ieee->atn_r();
	data |= !m_ieee->dav_r() << 1;
	data |= !m_ieee->ndac_r() << 2;
	data |= !m_ieee->nrfd_r() << 3;
	data |= !m_ieee->eoi_r() << 4;
	data |= !m_ieee->srq_r() << 5;
	data |= !m_ieee->ren_r() << 6;
	data |= !m_ieee->ifc_r() << 7;

	return data;
}

WRITE8_MEMBER( softbox_state::pia1_pb_w )
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

	m_ieee->atn_w(!BIT(data, 0));
	m_ieee->dav_w(!BIT(data, 1));
	m_ieee->ndac_w(!BIT(data, 2));
	m_ieee->nrfd_w(!BIT(data, 3));
	m_ieee->eoi_w(!BIT(data, 4));
	m_ieee->srq_w(!BIT(data, 5));
	m_ieee->ren_w(!BIT(data, 6));
	m_ieee->ifc_w(!BIT(data, 7));
}

READ8_MEMBER( softbox_state::pia1_pc_r )
{
	/*

	  bit     description

	  PC0     
	  PC1     
	  PC2     
	  PC3     
	  PC4     Corvus READY
	  PC5     Corvus ACTIVE
	  PC6     
	  PC7     

	*/

	return 0;
}

WRITE8_MEMBER( softbox_state::pia1_pc_w )
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

	output_set_led_value(LED_A, BIT(data, 0));
	output_set_led_value(LED_B, BIT(data, 1));
	output_set_led_value(LED_READY, BIT(data, 2));
}

static I8255A_INTERFACE( ppi1_intf )
{
	DEVCB_DRIVER_MEMBER(softbox_state, pia1_pa_r),
	DEVCB_NULL, // Port A write
	DEVCB_NULL, // Port B read
	DEVCB_DRIVER_MEMBER(softbox_state, pia1_pb_w),
	DEVCB_DRIVER_MEMBER(softbox_state, pia1_pc_r),
	DEVCB_DRIVER_MEMBER(softbox_state, pia1_pc_w)
};


//-------------------------------------------------
//  COM8116_INTERFACE( dbrg_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( softbox_state::fr_w )
{
	m_usart->receive_clock();
}

WRITE_LINE_MEMBER( softbox_state::ft_w )
{
	m_usart->transmit_clock();
}

static COM8116_INTERFACE( dbrg_intf )
{
	DEVCB_NULL, // fX/4
	DEVCB_DRIVER_LINE_MEMBER(softbox_state, fr_w),
	DEVCB_DRIVER_LINE_MEMBER(softbox_state, ft_w),
	COM8116_DIVISORS_16X_5_0688MHz, // receiver
	COM8116_DIVISORS_16X_5_0688MHz // transmitter
};


//-------------------------------------------------
//  serial_terminal_interface terminal_intf
//-------------------------------------------------

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "TERM_FRAME", 0x0f, 0x06 ) // 9600
	DEVICE_INPUT_DEFAULTS( "TERM_FRAME", 0x30, 0x10 ) // 7E1
DEVICE_INPUT_DEFAULTS_END

static const serial_terminal_interface terminal_intf =
{
	DEVCB_NULL
};



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( softbox )
//-------------------------------------------------

static MACHINE_CONFIG_START( softbox, softbox_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000) // ???
	MCFG_CPU_PROGRAM_MAP(softbox_mem)
	MCFG_CPU_IO_MAP(softbox_io)

	// devices
	MCFG_I8251_ADD(I8251_TAG, usart_intf)
	MCFG_I8255A_ADD(I8255_0_TAG, ppi0_intf)
	MCFG_I8255A_ADD(I8255_1_TAG, ppi1_intf)
	MCFG_COM8116_ADD(COM8116_TAG, XTAL_5_0688MHz, dbrg_intf)
	MCFG_CBM_IEEE488_ADD("c8050")
	MCFG_SERIAL_TERMINAL_ADD(TERMINAL_TAG, terminal_intf, 9600)
	MCFG_DEVICE_INPUT_DEFAULTS(terminal)

	// software lists
	//MCFG_SOFTWARE_LIST_ADD("flop_list", "softbox_flop")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( softbox )
//-------------------------------------------------

ROM_START( softbox )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("830609")
	ROM_SYSTEM_BIOS( 0, "811027", "27-Oct-81" )
	ROMX_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b), ROM_BIOS(1) )
	ROMX_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "830609", "09-June-1983" )
	ROMX_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048), ROM_BIOS(2) )
	ROMX_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad), ROM_BIOS(2) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS
COMP( 1981, softbox,    0,      0,      softbox,        softbox, driver_device, 0,      "Small Systems Engineering",  "SoftBox",  GAME_NOT_WORKING | GAME_NO_SOUND_HW )
