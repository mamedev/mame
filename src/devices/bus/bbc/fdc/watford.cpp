// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics DDB FDC

    DDB : 1770
    DDB2: 1772
    DDB3: 1770 (Acorn compatible)

**********************************************************************/


#include "emu.h"
#include "watford.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_WEDDB2, bbc_weddb2_device, "bbc_weddb2", "Watford Electronics DDB2 1772 FDC")
DEFINE_DEVICE_TYPE(BBC_WEDDB3, bbc_weddb3_device, "bbc_weddb3", "Watford Electronics DDB3 1770 FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( watford )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_watfordfdc_device::floppy_formats )
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT
FLOPPY_FORMATS_END0

static void bbc_floppies_525(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dd",   FLOPPY_525_DD);
	device.option_add("525qd",   FLOPPY_525_QD);
}

ROM_START( weddb2 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddfs153")
	ROM_SYSTEM_BIOS(0, "ddfs140", "Watford Electronics DDFS 1.40")
	ROMX_LOAD("ddfs140.rom", 0x0000, 0x4000, CRC(6c6eef94) SHA1(d86b26a6fe7b3532e7bd3c6f3e8e503edac3811c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ddfs150", "Watford Electronics DDFS 1.50")
	ROMX_LOAD("ddfs150.rom", 0x0000, 0x4000, CRC(d2fff497) SHA1(7693f14aeed26c34b2342c3487ed4b9ad233e279), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ddfs153", "Watford Electronics DDFS 1.53")
	ROMX_LOAD("ddfs153.rom", 0x0000, 0x4000, CRC(e1be4ee4) SHA1(6719dc958f2631e6dc8f045429797b289bfe649a), ROM_BIOS(3))
ROM_END

ROM_START( weddb3 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ddfs154t")
	ROM_SYSTEM_BIOS(0, "ddfs154t", "Watford Electronics DDFS 1.54T")
	ROMX_LOAD("ddfs154t.rom", 0x0000, 0x4000, CRC(6504c1ed) SHA1(618b7a1551f91e2b18608428b9e581f3d920d4b5), ROM_BIOS(1))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_weddb2_device::device_add_mconfig)
	MCFG_WD1772_ADD("wd1772", XTAL(16'000'000) / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_weddb2_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_weddb2_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("wd1772:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(bbc_weddb3_device::device_add_mconfig)
	MCFG_WD1770_ADD("wd1770", XTAL(16'000'000) / 2)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, bbc_weddb3_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_weddb3_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", bbc_floppies_525, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

const tiny_rom_entry *bbc_weddb2_device::device_rom_region() const
{
	return ROM_NAME( weddb2 );
}

const tiny_rom_entry *bbc_weddb3_device::device_rom_region() const
{
	return ROM_NAME( weddb3 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_watfordfdc_device - constructor
//-------------------------------------------------

bbc_watfordfdc_device::bbc_watfordfdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_bbc_fdc_interface(mconfig, *this)
{
}

bbc_weddb2_device::bbc_weddb2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_watfordfdc_device(mconfig, BBC_WEDDB2, tag, owner, clock),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "wd1772"),
	m_floppy0(*this, "wd1772:0"),
	m_floppy1(*this, "wd1772:1")
{
}

bbc_weddb3_device::bbc_weddb3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_watfordfdc_device(mconfig, BBC_WEDDB3, tag, owner, clock),
	m_dfs_rom(*this, "dfs_rom"),
	m_fdc(*this, "wd1770"),
	m_floppy0(*this, "wd1770:0"),
	m_floppy1(*this, "wd1770:1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_weddb2_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe83, READ8_DELEGATE(bbc_weddb2_device, wd177xl_read), WRITE8_DELEGATE(bbc_weddb2_device, wd177xl_write));
	space.install_readwrite_handler(0xfe84, 0xfe8f, READ8_DEVICE_DELEGATE(m_fdc, wd_fdc_device_base, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd_fdc_device_base, write));
}

void bbc_weddb3_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_fdc_slot_device *>(owner());

	space.install_readwrite_handler(0xfe80, 0xfe83, READ8_DELEGATE(bbc_weddb3_device, wd177xl_read), WRITE8_DELEGATE(bbc_weddb3_device, wd177xl_write));
	space.install_readwrite_handler(0xfe84, 0xfe8f, READ8_DEVICE_DELEGATE(m_fdc, wd_fdc_device_base, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd_fdc_device_base, write));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_weddb2_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());
}

void bbc_weddb3_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER(bbc_weddb2_device::wd177xl_read)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_weddb2_device::wd177xl_write)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 2: drive select
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();
	floppy = (BIT(data, 2) ? floppy1 : floppy0);
	m_fdc->set_floppy(floppy);

	// bit 0: density
	m_fdc->dden_w(BIT(data, 0));

	// bit 1: side select
	if (floppy)
		floppy->ss_w(BIT(data, 1));

	// bit 3: reset
	if (!BIT(data, 3)) m_fdc->soft_reset();
}

READ8_MEMBER(bbc_weddb3_device::wd177xl_read)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_weddb3_device::wd177xl_write)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_fdc->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_fdc->subdevice<floppy_connector>("1")->get_device();
	m_fdc->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_fdc->dden_w(BIT(data, 3));

	// bit 5: reset
	if (!BIT(data, 5)) m_fdc->soft_reset();
}

WRITE_LINE_MEMBER(bbc_watfordfdc_device::fdc_intrq_w)
{
	m_slot->intrq_w(state);
}

WRITE_LINE_MEMBER(bbc_watfordfdc_device::fdc_drq_w)
{
	m_slot->drq_w(state);
}
