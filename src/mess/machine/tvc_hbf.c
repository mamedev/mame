/***************************************************************************

    tvc_hbf.c

***************************************************************************/

#include "emu.h"
#include "tvc_hbf.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static LEGACY_FLOPPY_OPTIONS_START(tvc_hbf)
	LEGACY_FLOPPY_OPTION(tvc_hbf, "img,dsk", "TVC DS disk image (720KB)", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
	LEGACY_FLOPPY_OPTION(tvc_hbf, "img,dsk", "TVC SS disk image (360KB)", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface tvc_hbf_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(tvc_hbf),
	"floppy_5_25",
	NULL
};

static MACHINE_CONFIG_FRAGMENT(tvc_hbf)
	MCFG_FD1793_ADD("fdc", default_wd17xx_interface_2_drives)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(tvc_hbf_floppy_interface)
MACHINE_CONFIG_END


ROM_START( tvc_hbf )
	ROM_REGION(0x4000, "hbf", 0)
    ROM_DEFAULT_BIOS("basic")
	ROM_SYSTEM_BIOS( 0, "basic", "BASIC" )
	ROMX_LOAD("hbf.rom",	    0x0000, 0x4000, CRC(ae34982b) SHA1(96c4154c04086c537ae1272fe051a256d2f5be3f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "upm", "UPM" )
	ROMX_LOAD("d_tvcupm.128",	0x0000, 0x4000, CRC(b3a567ad) SHA1(f92df6074b07f5f19e8c96ff1315da0cfeec9f74), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "vtdos11", "VT-DOS v1.1" )
	ROMX_LOAD("d_tvcdos.128",	0x0000, 0x4000, CRC(2acf8477) SHA1(07bf39b633a564f98dd4b2e93bd889501b341550), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "vtdos12", "VT-DOS v1.2" )
	ROMX_LOAD("d_dos12.128",	0x0000, 0x4000, CRC(f5c35597) SHA1(2fa44ad089a51f453b580e0b13e3be96a0f14649), ROM_BIOS(4))

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

tvc_hbf_device::tvc_hbf_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
      : device_t(mconfig, TVC_HBF, "HBF floppy interface", tag, owner, clock),
		device_tvcexp_interface( mconfig, *this ),
		m_fdc(*this, "fdc")
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
		logerror("'%s': unmapped write to %04x %02x\n", tag(), offset, data);
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

READ8_MEMBER(tvc_hbf_device::io_read)
{
	switch((offset>>2) & 0x03)
	{
		case 0x00:
			return wd17xx_r(m_fdc, offset & 3);
		case 0x01:
			return (wd17xx_drq_r(m_fdc)<<7) | wd17xx_intrq_r(m_fdc);
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
			wd17xx_w(m_fdc, offset & 3, data);
			break;
		case 0x01:
			// bit 0-3   drive select
			// bit 5     DDEN
			// bit 6     floppy motor
			// bit 7     side select
			if (BIT(data, 0))
				wd17xx_set_drive(m_fdc, 0);
			else if (BIT(data, 1))
				wd17xx_set_drive(m_fdc, 1);
			wd17xx_dden_w(m_fdc, BIT(data, 5));
			floppy_mon_w(subdevice(FLOPPY_0), !BIT(data, 6));
			floppy_mon_w(subdevice(FLOPPY_1), !BIT(data, 6));
			wd17xx_set_side(m_fdc, BIT(data, 7));
			break;
		case 0x02:
			m_rom_bank = (data>>4) & 0x03;
			break;
	}
}
