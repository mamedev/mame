// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Challenger 3-in-1

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Opus_Challenger3.html
    http://www.beebmaster.co.uk/8bit/Challenger.html

  OPUS CHALLENGER MEMORY MAP
               Read                        Write
  &FCF8   1770 Status register        1770 command register
  &FCF9             1770 track register
  &FCFA             1770 sector register
  &FCFB             1770 data register
  &FCFC                               1770 drive control

  Drive control register
  0   Select side: 0=side 0, 1=side 1
  1   Select drive 0
  2   Select drive 1
  3   ?Unused?
  4   ?Always Set
  5   Density select: 0=double, 1=single
  6   ?Unused?
  7   ?Unused?

  The RAM is accessible through JIM (page &FD). One page is visible in JIM at a time.
  The selected page is controlled by the two paging registers:

  &FCFE       Paging register MSB
  &FCFF       Paging register LSB

  256K model has 1024 pages &000 to &3FF
  512K model has 2048 pages &000 to &7FF

**********************************************************************/


#include "emu.h"
#include "opus3.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_OPUS3, bbc_opus3_device, "bbc_opus3", "Opus Challenger 3-in-1")
DEFINE_DEVICE_TYPE(BBC_OPUSA, bbc_opusa_device, "bbc_opusa", "Opus Challenger ADFS")


//-------------------------------------------------
//  MACHINE_DRIVER( opus3 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(bbc_opus3_device::floppy_formats)
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_ACORN_ADFS_OLD_FORMAT,
	FLOPPY_ACORN_DOS_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_OPUS_DDOS_FORMAT,
	FLOPPY_OPUS_DDCPM_FORMAT
FLOPPY_FORMATS_END0

void bbc_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


ROM_START( opus3 )
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("ch103")
	ROM_SYSTEM_BIOS(0, "ch100", "Challenger 1.00")
	ROMX_LOAD("chal100.rom", 0x0000, 0x4000, CRC(740a8335) SHA1(f3c75c21bcd7d4a4dfff922fd287230dcdb91d0e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ch101", "Challenger 1.01")
	ROMX_LOAD("chal101.rom", 0x0000, 0x4000, CRC(2f64503d) SHA1(37ee3f20bed50555720703b279f62aab0ed28922), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "ch103", "Challenger 1.03")
	ROMX_LOAD("chal103.rom", 0x0000, 0x4000, CRC(98367cf4) SHA1(eca3631aa420691f96b72bfdf2e9c2b613e1bf33), ROM_BIOS(2))
ROM_END

ROM_START( opusa )
	ROM_REGION(0x8000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("ch200")
	ROM_SYSTEM_BIOS(0, "ch200", "Challenger ADFS")
	ROMX_LOAD("challenger adfs_dfs.rom", 0x0000, 0x8000, CRC(e922c19a) SHA1(b9f5c749412528e4f8e9cda9f13e10f8405bb195), ROM_BIOS(0))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_opus3_device::device_add_mconfig(machine_config &config)
{
	/* fdc */
	WD1770(config, m_fdc, 16_MHz_XTAL / 2);
	m_fdc->drq_wr_callback().set(FUNC(bbc_opus3_device::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies, nullptr, floppy_formats).enable_sound(true);

	/* ram disk */
	RAM(config, m_ramdisk).set_default_size("512K").set_extra_options("256K").set_default_value(0);
}

const tiny_rom_entry *bbc_opus3_device::device_rom_region() const
{
	return ROM_NAME( opus3 );
}

const tiny_rom_entry *bbc_opusa_device::device_rom_region() const
{
	return ROM_NAME( opusa );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_opus3_device - constructor
//-------------------------------------------------

bbc_opus3_device::bbc_opus3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_1mhzbus_interface(mconfig, *this)
	, m_ramdisk(*this, "ramdisk")
	, m_fdc(*this, "wd1770")
	, m_floppy0(*this, "wd1770:0")
	, m_floppy1(*this, "wd1770:1")
	, m_ramdisk_page(0)
{
}

bbc_opus3_device::bbc_opus3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opus3_device(mconfig, BBC_OPUS3, tag, owner, clock)
{
}

bbc_opusa_device::bbc_opusa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_opus3_device(mconfig, BBC_OPUSA, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_opus3_device::device_start()
{
}

void bbc_opusa_device::device_start()
{
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_opus3_device::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
		data = m_fdc->read(offset & 0x03);
		break;
	}
	return data;
}

void bbc_opus3_device::fred_w(offs_t offset, uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
		m_fdc->write(offset & 0x03, data);
		break;
	case 0xfc:
		// bit 1, 2: drive select
		if (BIT(data, 1)) floppy = m_floppy0->get_device();
		if (BIT(data, 2)) floppy = m_floppy1->get_device();
		m_fdc->set_floppy(floppy);

		// bit 0: side select
		if (floppy)
			floppy->ss_w(BIT(data, 0));

		// bit 4: interrupt enabled
		m_fdc_ie = BIT(data, 4);

		// bit 5: density
		m_fdc->dden_w(BIT(data, 5));
		break;
	case 0xfe:
		m_ramdisk_page = (m_ramdisk_page & 0x00ff) | (data << 8);
		break;
	case 0xff:
		m_ramdisk_page = (m_ramdisk_page & 0xff00) | (data << 0);
		break;
	}
}

WRITE_LINE_MEMBER(bbc_opus3_device::fdc_drq_w)
{
	m_fdc_drq = state;

	m_slot->nmi_w((m_fdc_drq && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t bbc_opus3_device::jim_r(offs_t offset)
{
	if ((m_ramdisk_page << 8) < m_ramdisk->size())
		return m_ramdisk->read((m_ramdisk_page << 8) + offset);
	else
		return 0xff;
}

void bbc_opus3_device::jim_w(offs_t offset, uint8_t data)
{
	if ((m_ramdisk_page << 8) < m_ramdisk->size())
		m_ramdisk->write((m_ramdisk_page << 8) + offset, data);
}
