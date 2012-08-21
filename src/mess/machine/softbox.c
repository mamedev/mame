/**********************************************************************

    SSE SoftBox emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "softbox.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG			"z80"
#define I8251_TAG		"i8251"
#define I8255_0_TAG		"ic17"
#define I8255_1_TAG		"ic16"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SOFTBOX = &device_creator<softbox_device>;


//-------------------------------------------------
//  ROM( softbox )
//-------------------------------------------------

ROM_START( softbox )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b) ) // Revision 27-Oct-81
	ROM_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3) ) // Revision 27-Oct-81
	ROM_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048) ) // Revision 09-June-1983
	ROM_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad) ) // Revision 09-June-1983
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
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x1000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_io, AS_IO, 8, softbox_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x, 0x) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	//AM_RANGE(0x, 0x) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	//AM_RANGE(0x, 0x) AM_DEVREADWRITE(I8255_0_TAG, i8255_device, read, write)
	//AM_RANGE(0x, 0x) AM_DEVREADWRITE(I8255_1_TAG, i8255_device, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  I8255A_INTERFACE( ppi0_intf )
//-------------------------------------------------

static const i8251_interface usart_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
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

static I8255A_INTERFACE( ppi0_intf )
{
	DEVCB_NULL,	// Port A read
	DEVCB_NULL,	// Port A write
	DEVCB_NULL,	// Port B read
	DEVCB_NULL,	// Port B write
	DEVCB_NULL,	// Port C read
	DEVCB_NULL	// Port C write
};


//-------------------------------------------------
//  I8255A_INTERFACE( ppi1_intf )
//-------------------------------------------------

static I8255A_INTERFACE( ppi1_intf )
{
	DEVCB_NULL,	// Port A read
	DEVCB_NULL,	// Port A write
	DEVCB_NULL,	// Port B read
	DEVCB_NULL,	// Port B write
	DEVCB_NULL,	// Port C read
	DEVCB_NULL	// Port C write
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( softbox )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( softbox )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000) // ???
	MCFG_CPU_PROGRAM_MAP(softbox_mem)
	MCFG_CPU_IO_MAP(softbox_io)

	MCFG_I8251_ADD(I8251_TAG, usart_intf)
	MCFG_I8255A_ADD(I8255_0_TAG, ppi0_intf)
	MCFG_I8255A_ADD(I8255_1_TAG, ppi1_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor softbox_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( softbox );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  softbox_device - constructor
//-------------------------------------------------

softbox_device::softbox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, SOFTBOX, "SoftBox", tag, owner, clock),
	  device_ieee488_interface(mconfig, *this),
	  m_maincpu(*this, Z80_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void softbox_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void softbox_device::device_reset()
{
}


//-------------------------------------------------
//  ieee488_atn - attention
//-------------------------------------------------

void softbox_device::ieee488_atn(int state)
{
}


//-------------------------------------------------
//  ieee488_ifc - interface clear
//-------------------------------------------------

void softbox_device::ieee488_ifc(int state)
{
}
