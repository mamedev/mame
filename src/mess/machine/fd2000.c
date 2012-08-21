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


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( fd2000 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( fd2000 )
	LEGACY_FLOPPY_OPTION( fd2000, "d81", "Commodore 1581 Disk Image", d81_dsk_identify, d81_dsk_construct, NULL, NULL )
	//LEGACY_FLOPPY_OPTION( fd2000, "d2m", "CMD FD-2000 Disk Image", d2m_dsk_identify, d2m_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  floppy_interface fd2000_floppy_interface
//-------------------------------------------------

static const floppy_interface fd2000_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(fd2000),
	"floppy_3_5",
	NULL
};


//-------------------------------------------------
//  upd765_interface fdc_intf
//-------------------------------------------------

static const struct upd765_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	NULL,
	UPD765_RDY_PIN_CONNECTED,
	{ FLOPPY_0, NULL, NULL, NULL }
};


//-------------------------------------------------
//  MACHINE_DRIVER( fd2000 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( fd2000 )
	MCFG_CPU_ADD(M6502_TAG, M65C02, 2000000)
	MCFG_CPU_PROGRAM_MAP(fd2000_mem)

	MCFG_VIA6522_ADD(M6522_TAG, 2000000, via_intf)
	MCFG_UPD765A_ADD(DP8473_TAG, fdc_intf)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, fd2000_floppy_interface)
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
