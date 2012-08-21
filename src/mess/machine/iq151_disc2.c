/***************************************************************************

    IQ151 Disc2 cartridge emulation

***************************************************************************/

#include "emu.h"
#include "iq151_disc2.h"


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static LEGACY_FLOPPY_OPTIONS_START( iq151_disc2 )
	LEGACY_FLOPPY_OPTION( iq151_disk, "iqd", "IQ-151 disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([1])
		TRACKS([77])
		SECTORS([26])
		SECTOR_LENGTH([128])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface iq151_disc2_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_8_SSSD,
	LEGACY_FLOPPY_OPTIONS_NAME(iq151_disc2),
	"floppy_8",
	NULL
};

static const upd765_interface iq151_disc2_fdc_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	NULL,
	UPD765_RDY_PIN_NOT_CONNECTED,
	{ NULL, FLOPPY_0, FLOPPY_1, NULL }
};

static MACHINE_CONFIG_FRAGMENT( iq151_disc2 )
	MCFG_UPD72065_ADD("fdc", iq151_disc2_fdc_intf)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(iq151_disc2_intf)
MACHINE_CONFIG_END

ROM_START( iq151_disc2 )
	ROM_REGION(0x0800, "disc2", 0)
	ROM_LOAD( "iq151_disc2_12_5_1987_v4_0.rom", 0x0000, 0x0800, CRC(b189b170) SHA1(3e2ca80934177e7a32d0905f5a0ad14072f9dabf))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IQ151_DISC2 = &device_creator<iq151_disc2_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iq151_disc2_device - constructor
//-------------------------------------------------

iq151_disc2_device::iq151_disc2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
      : device_t(mconfig, IQ151_DISC2, "IQ151 Disc2", tag, owner, clock),
		device_iq151cart_interface( mconfig, *this ),
		m_fdc(*this, "fdc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iq151_disc2_device::device_start()
{
	m_rom = (UINT8*)memregion("disc2")->base();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iq151_disc2_device::device_reset()
{
	m_rom_enabled = false;
}

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor iq151_disc2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( iq151_disc2 );
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *iq151_disc2_device::device_rom_region() const
{
	return ROM_NAME( iq151_disc2 );
}

//-------------------------------------------------
//  read
//-------------------------------------------------

void iq151_disc2_device::read(offs_t offset, UINT8 &data)
{
	// interal ROM is mapped at 0xe000-0xe7ff
	if (offset >= 0xe000 && offset < 0xe800 && m_rom_enabled)
		data = m_rom[offset & 0x7ff];
}


//-------------------------------------------------
//  IO read
//-------------------------------------------------

void iq151_disc2_device::io_read(offs_t offset, UINT8 &data)
{
	if (offset == 0xaa)
		data = upd765_status_r(m_fdc, 0);
	else if (offset == 0xab)
		data = upd765_data_r(m_fdc, 0);
}

//-------------------------------------------------
//  IO write
//-------------------------------------------------

void iq151_disc2_device::io_write(offs_t offset, UINT8 data)
{
	if (offset == 0xab)
		upd765_data_w(m_fdc, 0, data);
	else if (offset == 0xac)
		m_rom_enabled = (data == 0x01);
}

