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


//-------------------------------------------------
//  MACHINE_DRIVER( opus3 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(bbc_opus3_device::floppy_formats)
	FLOPPY_ACORN_SSD_FORMAT,
	FLOPPY_ACORN_DSD_FORMAT,
	FLOPPY_FSD_FORMAT,
	FLOPPY_OPUS_DDOS_FORMAT,
	FLOPPY_OPUS_DDCPM_FORMAT
FLOPPY_FORMATS_END0

void bbc_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


ROM_START( opus3 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("ch103")
	ROM_SYSTEM_BIOS(0, "ch100", "Challenger 1.00")
	ROMX_LOAD("ch100.rom", 0x0000, 0x4000, CRC(740a8335) SHA1(f3c75c21bcd7d4a4dfff922fd287230dcdb91d0e), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "ch101", "Challenger 1.01")
	ROMX_LOAD("ch101.rom", 0x0000, 0x4000, CRC(2f64503d) SHA1(37ee3f20bed50555720703b279f62aab0ed28922), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "ch103", "Challenger 1.03")
	ROMX_LOAD("ch103.rom", 0x0000, 0x4000, CRC(98367cf4) SHA1(eca3631aa420691f96b72bfdf2e9c2b613e1bf33), ROM_BIOS(3))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(bbc_opus3_device::device_add_mconfig)
	/* fdc */
	MCFG_WD1770_ADD("wd1770", XTAL(16'000'000) / 2)
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, bbc_opus3_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("wd1770:0", bbc_floppies, "525qd", floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("wd1770:1", bbc_floppies, nullptr, floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	/* ram disk */
	MCFG_RAM_ADD("ramdisk")
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("256K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END

const tiny_rom_entry *bbc_opus3_device::device_rom_region() const
{
	return ROM_NAME( opus3 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_opus3_device - constructor
//-------------------------------------------------

bbc_opus3_device::bbc_opus3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_OPUS3, tag, owner, clock),
		device_bbc_1mhzbus_interface(mconfig, *this),
		m_dfs_rom(*this, "dfs_rom"),
		m_ramdisk(*this, "ramdisk"),
		m_fdc(*this, "wd1770"),
		m_floppy0(*this, "wd1770:0"),
		m_floppy1(*this, "wd1770:1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_opus3_device::device_start()
{
	address_space& space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<bbc_1mhzbus_slot_device *>(owner());

	space.install_readwrite_handler(0xfcf8, 0xfcfb, READ8_DEVICE_DELEGATE(m_fdc, wd1770_device, read), WRITE8_DEVICE_DELEGATE(m_fdc, wd1770_device, write));
	space.install_write_handler(0xfcfc, 0xfcfc, WRITE8_DELEGATE(bbc_opus3_device, wd1770l_write));
	space.install_write_handler(0xfcfe, 0xfcff, WRITE8_DELEGATE(bbc_opus3_device, page_w));
	space.install_readwrite_handler(0xfd00, 0xfdff, READ8_DELEGATE(bbc_opus3_device, ramdisk_r), WRITE8_DELEGATE(bbc_opus3_device, ramdisk_w));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_opus3_device::device_reset()
{
	machine().root_device().membank("bank4")->configure_entry(12, memregion("dfs_rom")->base());
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE8_MEMBER(bbc_opus3_device::wd1770l_write)
{
	floppy_image_device *floppy = nullptr;

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
}

WRITE_LINE_MEMBER(bbc_opus3_device::fdc_drq_w)
{
	m_fdc_drq = state;

	m_slot->nmi_w((m_fdc_drq && m_fdc_ie) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(bbc_opus3_device::page_w)
{
	switch (offset)
	{
	case 0x00: m_ramdisk_page = (m_ramdisk_page & 0x00ff) | (data << 8); break;
	case 0x01: m_ramdisk_page = (m_ramdisk_page & 0xff00) | (data << 0); break;
	}
}

READ8_MEMBER(bbc_opus3_device::ramdisk_r)
{
	if ((m_ramdisk_page << 8) < m_ramdisk->size())
		return m_ramdisk->read((m_ramdisk_page << 8) + offset);
	else
		return 0xff;
}

WRITE8_MEMBER(bbc_opus3_device::ramdisk_w)
{
	if ((m_ramdisk_page << 8) < m_ramdisk->size())
		m_ramdisk->write((m_ramdisk_page << 8) + offset, data);
}
