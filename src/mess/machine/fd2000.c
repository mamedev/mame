/**********************************************************************

    CMD FD2000 disk drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "fd2000.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG		"m6502"
#define M6522_TAG		"m6522"
#define DP8473_TAG		"dp8473"
#define PC8477AV1_TAG	"pc8477av1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type FD2000 = &device_creator<fd2000_device>;


//-------------------------------------------------
//  ROM( fd2000 )
//-------------------------------------------------

ROM_START( fd2000 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "cmd_fd-2000_dos_v1.40_cs_33cc6f.bin", 0x0000, 0x8000, CRC(4e6ca15c) SHA1(0c61ba58269baf2b8aadf3bbc4648c7a5a6d2128) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *fd2000_device::device_rom_region() const
{
	return ROM_NAME( fd2000 );
}


//-------------------------------------------------
//  ADDRESS_MAP( fd2000_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fd2000_mem, AS_PROGRAM, 8, fd2000_device )
	AM_RANGE(0x0000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
	//AM_RANGE() AM_DEVREADWRITE(M6522_TAG, via6522_device, read, write)
	//AM_RANGE() AM_DEVREAD_LEGACY(DP8473_TAG, upd765_status_r)
	//AM_RANGE() AM_DEVREADWRITE_LEGACY(DP8473_TAG, upd765_data_r, upd765_data_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  via6522_interface via1_intf
//-------------------------------------------------

static const via6522_interface via_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
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

static SLOT_INTERFACE_START( fd2000_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_DRIVER( fd2000 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( fd2000 )
	MCFG_CPU_ADD(M6502_TAG, M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(fd2000_mem)

	MCFG_VIA6522_ADD(M6522_TAG, 2000000, via_intf)
	MCFG_UPD765A_ADD(DP8473_TAG, true, true)

	MCFG_FLOPPY_DRIVE_ADD(DP8473_TAG ":0", fd2000_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor fd2000_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fd2000 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fd2000_device - constructor
//-------------------------------------------------

fd2000_device::fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, FD2000, "FD-2000", tag, owner, clock),
	  device_cbm_iec_interface(mconfig, *this),
	  m_maincpu(*this, M6502_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fd2000_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void fd2000_device::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void fd2000_device::cbm_iec_srq(int state)
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void fd2000_device::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void fd2000_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void fd2000_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
