/**********************************************************************

    Scandia Metric FD2 floppy controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

/*

PCB Layout
----------

  |-------------------------------------------|
|-|                                           |
|-|    ROM0                          4MHz     |
|-|                                           |
|-|                Z80PIO                     |
|-|                                        CN1|
|-|                FD1771    2114             |
|-|                          2114             |
|-|                Z80       ROM1             |
|-|                                           |
  |-------------------------------------------|

Notes:
    Relevant IC's shown.

    ROM0    - AMI 8005SAJ 1Kx8 EPROM
    ROM1    - Motorola MCM2708C 1Kx8 EPROM
    Z80     - Zilog Z-80 CPU
    Z80PIO  - Zilog Z-80A PIO
    FD1771  - FD1771-B01
    2114    - National Semiconductor MM2114N 1Kx4 Static RAM
    CN1     - 2x17 pin PCB header

*/

#include "abc_fd2.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG		"2e"
#define Z80PIO_TAG	"2c"
#define FD1771_TAG	"2d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC_FD2 = &device_creator<abc_fd2_device>;


//-------------------------------------------------
//  ROM( abc_fd2 )
//-------------------------------------------------

ROM_START( abc_fd2 )
	ROM_REGION( 0x400, Z80_TAG, 0 )
	ROM_LOAD( "1.02.3f", 0x000, 0x400, NO_DUMP )

	ROM_REGION( 0x400, "abc80", 0 )
	ROM_LOAD( "ami 8005saj.1a", 0x000, 0x400, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc_fd2_device::device_rom_region() const
{
	return ROM_NAME( abc_fd2 );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_fd2_mem, AS_PROGRAM, 8, abc_fd2_device )
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_fd2_io, AS_IO, 8, abc_fd2_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  Z80PIO_INTERFACE( pio_intf )
//-------------------------------------------------

static Z80PIO_INTERFACE( pio_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80PIO_TAG },
	{ NULL }
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static const floppy_interface fd2_floppy_interface =
{
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    FLOPPY_STANDARD_5_25_SSSD,
    LEGACY_FLOPPY_OPTIONS_NAME(default),
    "floppy_5_25",
	NULL
};

static const wd17xx_interface fdc_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ FLOPPY_0, FLOPPY_1, NULL, NULL }
};


//-------------------------------------------------
//  MACHINE_DRIVER( abc_fd2 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc_fd2 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz/2) // ?
	MCFG_CPU_PROGRAM_MAP(abc_fd2_mem)
	MCFG_CPU_IO_MAP(abc_fd2_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_4MHz/2, pio_intf) // ?
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(fd2_floppy_interface)
	MCFG_FD1771_ADD(FD1771_TAG, fdc_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc_fd2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc_fd2 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_fd2_device - constructor
//-------------------------------------------------

abc_fd2_device::abc_fd2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, ABC_FD2, "ABC FD2", tag, owner, clock),
	  device_abcbus_card_interface(mconfig, *this),
	  m_maincpu(*this, Z80_TAG),
	  m_pio(*this, Z80PIO_TAG),
	  m_fdc(*this, FD1771_TAG),
	  m_image0(*this, FLOPPY_0),
	  m_image1(*this, FLOPPY_1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_fd2_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_fd2_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_fd2_device::abcbus_cs(UINT8 data)
{
}


//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

UINT8 abc_fd2_device::abcbus_xmemfl(offs_t offset)
{
	UINT8 data = 0xff;

	if (offset >= 0x6000 && offset < 0x6400) // TODO is this mirrored?
	{
		data = memregion("abc80")->base()[offset & 0x3ff];
	}

	return data;
}
