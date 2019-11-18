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

#include "emu.h"
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

DEFINE_DEVICE_TYPE(FD2000, fd2000_device, "fd2000", "FD-2000 Disk Drive")
DEFINE_DEVICE_TYPE(FD4000, fd4000_device, "fd4000", "FD-4000 Disk Drive")


//-------------------------------------------------
//  ROM( fd2000 )
//-------------------------------------------------

ROM_START( fd2000 )
	ROM_REGION( 0x8000, G65SC02PI2_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-2000 dos v1.34 fd-350026.bin", 0x0000, 0x8000, CRC(859a5edc) SHA1(487fa82a7977e5208d5088f3580f34e8c89560d1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-2000 dos v1.40 cs 33cc6f.bin", 0x0000, 0x8000, CRC(4e6ca15c) SHA1(0c61ba58269baf2b8aadf3bbc4648c7a5a6d2128), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( fd4000 )
//-------------------------------------------------

ROM_START( fd4000 )
	ROM_REGION( 0x8000, R65C02P4_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-4000 dos v1.34 fd-350022.bin", 0x0000, 0x8000, CRC(1f4820c1) SHA1(7a2966662e7840fd9377549727ccba62e4349c6f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-4000 dos v1.40 fd-350022.bin", 0x0000, 0x8000, CRC(b563ef10) SHA1(d936d76fd8b50ce4c65f885703653d7c1bd7d3c9), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *fd2000_device::device_rom_region() const
{
	return ROM_NAME( fd2000 );
}

const tiny_rom_entry *fd4000_device::device_rom_region() const
{
	return ROM_NAME( fd4000 );
}

//-------------------------------------------------
//  ADDRESS_MAP( fd2000_mem )
//-------------------------------------------------

void fd2000_device::fd2000_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x400f).mirror(0xbf0).m(G65SC22P2_TAG, FUNC(via6522_device::map));
	map(0x4e00, 0x4e07).mirror(0x1f8).m(DP8473V_TAG, FUNC(dp8473_device::map));
	map(0x5000, 0x7fff).ram();
	map(0x8000, 0xffff).rom().region(G65SC02PI2_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( fd4000_mem )
//-------------------------------------------------

void fd4000_device::fd4000_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x400f).mirror(0xbf0).m(G65SC22P2_TAG, FUNC(via6522_device::map));
	map(0x4e00, 0x4e07).mirror(0x1f8).m(PC8477AV1_TAG, FUNC(pc8477a_device::map));
	map(0x5000, 0x7fff).ram();
	map(0x8000, 0xffff).rom().region(R65C02P4_TAG, 0);
}


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

	uint8_t data = 0;

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

static void fd2000_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD); // TEAC FD-235HF
}

static void fd4000_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED); // TEAC FD-235J
}
/*
FLOPPY_FORMATS_MEMBER( fd2000_device::floppy_formats )
    FLOPPY_D81_FORMAT
    FLOPPY_D2M_FORMAT
FLOPPY_FORMATS_END
*/


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void fd2000_device::add_common_devices(machine_config &config)
{
	M65C02(config, m_maincpu, 24_MHz_XTAL / 12);

	via6522_device &via(VIA6522(config, G65SC22P2_TAG, 24_MHz_XTAL / 12));
	via.readpa_handler().set(FUNC(fd2000_device::via_pa_r));
	via.readpb_handler().set(FUNC(fd2000_device::via_pb_r));
	via.writepa_handler().set(FUNC(fd2000_device::via_pa_w));
	via.writepb_handler().set(FUNC(fd2000_device::via_pb_w));
}

void fd2000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fd2000_device::fd2000_mem);
	DP8473(config, m_fdc, 24_MHz_XTAL);
	FLOPPY_CONNECTOR(config, DP8473V_TAG":0", fd2000_floppies, "35hd", floppy_image_device::default_floppy_formats, true);//fd2000_device::floppy_formats);
}

void fd4000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fd4000_device::fd4000_mem);
	PC8477A(config, m_fdc, 24_MHz_XTAL);
	FLOPPY_CONNECTOR(config, PC8477AV1_TAG":0", fd4000_floppies, "35ed", floppy_image_device::default_floppy_formats, true);//fd2000_device::floppy_formats);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fd2000_device - constructor
//-------------------------------------------------

fd2000_device::fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fd2000_device(mconfig, FD2000, tag, owner, clock)
{
}

fd2000_device::fd2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
	, m_maincpu(*this, G65SC02PI2_TAG)
	, m_fdc(*this, DP8473V_TAG)
	, m_floppy0(*this, DP8473V_TAG":0")
{
}


//-------------------------------------------------
//  fd4000_device - constructor
//-------------------------------------------------

fd4000_device::fd4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fd2000_device(mconfig, FD4000, tag, owner, clock)
{
	m_maincpu.set_tag(*this, R65C02P4_TAG);
	m_fdc.set_tag(*this, PC8477AV1_TAG);
	m_floppy0.set_tag(*this, PC8477AV1_TAG":0");
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
