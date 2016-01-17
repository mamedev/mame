// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD FD-2000/FD-4000 disk drive emulation

**********************************************************************/

/*

    TODO:

    - IEC
    - VIA
    - DP8473/PC8477A command extensions to upd765
    - D1M/D2M/D4M image format (http://ist.uwaterloo.ca/~schepers/formats/D2M-DNP.TXT)

*/

#include "fd2000.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define G65SC02PI2_TAG  "m6502"
#define R65C02P4_TAG    "m6502"
#define G65SC22P2_TAG   "m6522"
#define DP8473V_TAG     "dp8473"
#define PC8477AV1_TAG   "pc8477av1"
#define DS1216E_TAG     "ds1216e"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type FD2000 = &device_creator<fd2000_device>;
const device_type FD4000 = &device_creator<fd4000_device>;


//-------------------------------------------------
//  ROM( fd2000 )
//-------------------------------------------------

ROM_START( fd2000 )
	ROM_REGION( 0x8000, G65SC02PI2_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-2000 dos v1.34 fd-350026.bin", 0x0000, 0x8000, CRC(859a5edc) SHA1(487fa82a7977e5208d5088f3580f34e8c89560d1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-2000 dos v1.40 cs 33cc6f.bin", 0x0000, 0x8000, CRC(4e6ca15c) SHA1(0c61ba58269baf2b8aadf3bbc4648c7a5a6d2128), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( fd4000 )
//-------------------------------------------------

ROM_START( fd4000 )
	ROM_REGION( 0x8000, R65C02P4_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-4000 dos v1.34 fd-350022.bin", 0x0000, 0x8000, CRC(1f4820c1) SHA1(7a2966662e7840fd9377549727ccba62e4349c6f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-4000 dos v1.40 fd-350022.bin", 0x0000, 0x8000, CRC(b563ef10) SHA1(d936d76fd8b50ce4c65f885703653d7c1bd7d3c9), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *fd2000_device::device_rom_region() const
{
	switch (m_variant)
	{
	default:
		return ROM_NAME( fd2000 );

	case TYPE_FD4000:
		return ROM_NAME( fd4000 );
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( fd2000_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fd2000_mem, AS_PROGRAM, 8, fd2000_device )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0xbf0) AM_DEVREADWRITE(G65SC22P2_TAG, via6522_device, read, write)
	AM_RANGE(0x4e00, 0x4e07) AM_MIRROR(0x1f8) AM_DEVICE(DP8473V_TAG, dp8473_device, map)
	AM_RANGE(0x5000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(G65SC02PI2_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( fd4000_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( fd4000_mem, AS_PROGRAM, 8, fd4000_device )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0xbf0) AM_DEVREADWRITE(G65SC22P2_TAG, via6522_device, read, write)
	AM_RANGE(0x4e00, 0x4e07) AM_MIRROR(0x1f8) AM_DEVICE(PC8477AV1_TAG, pc8477a_device, map)
	AM_RANGE(0x5000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(R65C02P4_TAG, 0)
ADDRESS_MAP_END


READ8_MEMBER( fd2000_device::via_pa_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	return 0;
}

WRITE8_MEMBER( fd2000_device::via_pa_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       FAST DIR
	    6
	    7

	*/
}

READ8_MEMBER( fd2000_device::via_pb_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6
	    7       FDC INTRQ

	*/

	UINT8 data = 0;

	// FDC interrupt
	data |= m_fdc->get_irq() << 7;

	return data;
}

WRITE8_MEMBER( fd2000_device::via_pb_w )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5       LED
	    6       LED
	    7

	*/
}

static SLOT_INTERFACE_START( fd2000_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD ) // TEAC FD-235HF
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( fd4000_floppies )
	SLOT_INTERFACE( "35ed", FLOPPY_35_ED ) // TEAC FD-235J
SLOT_INTERFACE_END
/*
FLOPPY_FORMATS_MEMBER( fd2000_device::floppy_formats )
    FLOPPY_D81_FORMAT
    FLOPPY_D2M_FORMAT
FLOPPY_FORMATS_END
*/

//-------------------------------------------------
//  MACHINE_DRIVER( fd2000 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( fd2000 )
	MCFG_CPU_ADD(G65SC02PI2_TAG, M65C02, XTAL_24MHz/12)
	MCFG_CPU_PROGRAM_MAP(fd2000_mem)

	MCFG_DEVICE_ADD(G65SC22P2_TAG, VIA6522, XTAL_24MHz/12)
	MCFG_VIA6522_READPA_HANDLER(READ8(fd2000_device, via_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(fd2000_device, via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(fd2000_device, via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(fd2000_device, via_pb_w))

	MCFG_DP8473_ADD(DP8473V_TAG)

	MCFG_FLOPPY_DRIVE_ADD(DP8473V_TAG":0", fd2000_floppies, "35hd", floppy_image_device::default_floppy_formats)//fd2000_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_DRIVER( fd4000 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( fd4000 )
	MCFG_CPU_ADD(R65C02P4_TAG, M65C02, XTAL_24MHz/6)
	MCFG_CPU_PROGRAM_MAP(fd4000_mem)

	MCFG_DEVICE_ADD(G65SC22P2_TAG, VIA6522, XTAL_24MHz/12)
	MCFG_VIA6522_READPA_HANDLER(READ8(fd2000_device, via_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(fd2000_device, via_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(fd2000_device, via_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(fd2000_device, via_pb_w))

	MCFG_PC8477A_ADD(PC8477AV1_TAG)

	MCFG_FLOPPY_DRIVE_ADD(PC8477AV1_TAG":0", fd4000_floppies, "35ed", floppy_image_device::default_floppy_formats)//fd2000_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor fd2000_device::device_mconfig_additions() const
{
	switch (m_variant)
	{
	default:
		return MACHINE_CONFIG_NAME( fd2000 );

	case TYPE_FD4000:
		return MACHINE_CONFIG_NAME( fd4000 );
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fd2000_device - constructor
//-------------------------------------------------

fd2000_device::fd2000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FD2000, "FD-2000", tag, owner, clock, "fd2000", __FILE__),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, G65SC02PI2_TAG),
		m_fdc(*this, DP8473V_TAG),
		m_floppy0(*this, DP8473V_TAG":0"),
		m_variant(TYPE_FD2000)
{
}

fd2000_device::fd2000_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, R65C02P4_TAG),
		m_fdc(*this, PC8477AV1_TAG),
		m_floppy0(*this, PC8477AV1_TAG":0"),
		m_variant(variant)
{
}


//-------------------------------------------------
//  fd4000_device - constructor
//-------------------------------------------------

fd4000_device::fd4000_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: fd2000_device(mconfig, FD4000, "FD-4000", tag, owner, clock, TYPE_FD4000, "fd4000", __FILE__ ) { }


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
