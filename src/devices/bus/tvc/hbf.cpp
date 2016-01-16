// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    hbf.c

***************************************************************************/

#include "emu.h"
#include "hbf.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

FLOPPY_FORMATS_MEMBER( tvc_hbf_device::floppy_formats )
	FLOPPY_TVC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( tvc_hbf_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(tvc_hbf)
	MCFG_FD1793_ADD("fdc", XTAL_16MHz / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", tvc_hbf_floppies, "525qd", tvc_hbf_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", tvc_hbf_floppies, "525qd", tvc_hbf_device::floppy_formats)
MACHINE_CONFIG_END

ROM_START( tvc_hbf )
	ROM_REGION(0x4000, "hbf", 0)
	ROM_DEFAULT_BIOS("basic")
	ROM_SYSTEM_BIOS( 0, "basic", "BASIC" )
	ROMX_LOAD("hbf.rom",        0x0000, 0x4000, CRC(ae34982b) SHA1(96c4154c04086c537ae1272fe051a256d2f5be3f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "upm", "UPM" )
	ROMX_LOAD("d_tvcupm.128",   0x0000, 0x4000, CRC(b3a567ad) SHA1(f92df6074b07f5f19e8c96ff1315da0cfeec9f74), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "vtdos11", "VT-DOS v1.1" )
	ROMX_LOAD("d_tvcdos.128",   0x0000, 0x4000, CRC(2acf8477) SHA1(07bf39b633a564f98dd4b2e93bd889501b341550), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "vtdos12", "VT-DOS v1.2" )
	ROMX_LOAD("d_dos12.128",    0x0000, 0x4000, CRC(f5c35597) SHA1(2fa44ad089a51f453b580e0b13e3be96a0f14649), ROM_BIOS(4))

	ROM_REGION(0x1000, "ram", ROMREGION_ERASE)
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type TVC_HBF = &device_creator<tvc_hbf_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tvc_hbf_device - constructor
//-------------------------------------------------

tvc_hbf_device::tvc_hbf_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, TVC_HBF, "HBF floppy interface", tag, owner, clock, "tvc_hbf", __FILE__),
		device_tvcexp_interface( mconfig, *this ),
		m_fdc(*this, "fdc"), m_rom(nullptr), m_ram(nullptr), m_rom_bank(0)
	{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tvc_hbf_device::device_start()
{
	m_rom = memregion("hbf")->base();
	m_ram = memregion("ram")->base();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tvc_hbf_device::device_reset()
{
	m_rom_bank = 0;
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor tvc_hbf_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tvc_hbf );
}

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const rom_entry *tvc_hbf_device::device_rom_region() const
{
	return ROM_NAME( tvc_hbf );
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
READ8_MEMBER(tvc_hbf_device::read)
{
	if (offset>=0x1000)
		return m_ram[offset& 0x0fff];
	else
		return m_rom[(m_rom_bank<<12) + (offset & 0x0fff)];
}

//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(tvc_hbf_device::write)
{
	if (offset>=0x1000)
		m_ram[offset & 0x0fff] = data;
	else
		logerror("'%s': unmapped write to %04x %02x\n", tag().c_str(), offset, data);
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

READ8_MEMBER(tvc_hbf_device::io_read)
{
	switch((offset>>2) & 0x03)
	{
		case 0x00:
			return m_fdc->read(space, offset & 3);
		case 0x01:
			return (m_fdc->drq_r()<<7) | (m_fdc->intrq_r() ? 0x01 : 0x00);
		default:
			return 0x00;
	}
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

WRITE8_MEMBER(tvc_hbf_device::io_write)
{
	switch((offset>>2) & 0x03)
	{
		case 0x00:
			m_fdc->write(space, offset & 3, data);
			break;
		case 0x01:
		{
			// bit 0-3   drive select
			// bit 5     DDEN
			// bit 6     floppy motor
			// bit 7     side select
			floppy_image_device *floppy = nullptr;

			if (BIT(data, 0)) floppy = subdevice<floppy_connector>("fdc:0")->get_device();
			if (BIT(data, 1)) floppy = subdevice<floppy_connector>("fdc:1")->get_device();
			m_fdc->set_floppy(floppy);
			m_fdc->dden_w(BIT(data, 5));
			if (floppy) floppy->mon_w(!BIT(data, 6));
			if (floppy) floppy->ss_w(BIT(data, 7));
			break;
		}
		case 0x02:
			m_rom_bank = (data>>4) & 0x03;
			break;
	}
}
