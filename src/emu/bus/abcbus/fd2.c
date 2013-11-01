// license:BSD-3-Clause
// copyright-holders:Curt Coder
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

#include "fd2.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG     "2e"
#define Z80PIO_TAG  "2c"
#define FD1771_TAG  "2d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC_FD2 = &device_creator<abc_fd2_device>;


//-------------------------------------------------
//  ROM( abc_fd2 )
//-------------------------------------------------

ROM_START( abc_fd2 )
	ROM_REGION( 0x400, Z80_TAG, 0 )
	ROM_LOAD( "1.02.3f", 0x000, 0x400, CRC(a19fbdc2) SHA1(d500377c34ac6c679c155f4a5208e1c3e00cd920) )

	ROM_REGION( 0x800, "abc80", 0 )
	ROM_LOAD( "ami 8005saj.1a", 0x000, 0x800, CRC(d865213f) SHA1(ae7399ede74520ccb2dd5be2e6bb13c33ee81bd0) )
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
	AM_RANGE(0x0800, 0x0bff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc_fd2_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_fd2_io, AS_IO, 8, abc_fd2_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xb0, 0xb3) AM_DEVREADWRITE(Z80PIO_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE(FD1771_TAG, fd1771_t, read, write)
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
//  SLOT_INTERFACE( abc_fd2_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( abc_fd2_floppies )
	SLOT_INTERFACE( "525sssd", FLOPPY_525_SSSD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  MACHINE_DRIVER( abc_fd2 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc_fd2 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(abc_fd2_mem)
	MCFG_CPU_IO_MAP(abc_fd2_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_Z80PIO_ADD(Z80PIO_TAG, XTAL_4MHz, pio_intf)
	MCFG_FD1771x_ADD(FD1771_TAG, XTAL_4MHz/2)

	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":0", abc_fd2_floppies, "525sssd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FD1771_TAG":1", abc_fd2_floppies, "525sssd", floppy_image_device::default_floppy_formats)
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
	: device_t(mconfig, ABC_FD2, "ABC FD2", tag, owner, clock, "abc_fd2", __FILE__),
		device_abcbus_card_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG),
		m_pio(*this, Z80PIO_TAG),
		m_fdc(*this, FD1771_TAG),
		m_floppy0(*this, FD1771_TAG":0"),
		m_floppy1(*this, FD1771_TAG":1"),
		m_rom(*this, "abc80")
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
		data = m_rom->base()[offset & 0x3ff];
	}

	return data;
}
