// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Village 1797 FDC

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/ComputerVillage_FDC.html

**********************************************************************/


#include "emu.h"
#include "cv1797.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_CV1797, bbc_cv1797_device,  "bbc_cv1797", "Computer Village 1797 FDC")


//-------------------------------------------------
//  MACHINE_DRIVER( cv1797 )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( bbc_cv1797_device::floppy_formats )
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

ROM_START( cv1797 )
	ROM_REGION(0x4000, "dfs_rom", 0)
	ROM_DEFAULT_BIOS("lvldos")
	ROM_SYSTEM_BIOS(0, "lvldos", "LVL Dos 0.91")
	ROMX_LOAD("lvldos91.rom", 0x0000, 0x2000, CRC(69d7653a) SHA1(9cd39d011290b97d5ba05b6c745689a8553be5fc), ROM_BIOS(0))
	ROM_RELOAD(0x2000, 0x2000)
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_cv1797_device::device_add_mconfig(machine_config &config)
{
	FD1797(config, m_fdc, 8_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::intrq_w));
	m_fdc->drq_wr_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_fdc_slot_device::drq_w));
	m_fdc->hld_wr_callback().set(FUNC(bbc_cv1797_device::motor_w));

	FLOPPY_CONNECTOR(config, m_floppy0, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy1, bbc_floppies_525, "525qd", floppy_formats).enable_sound(true);
}

const tiny_rom_entry *bbc_cv1797_device::device_rom_region() const
{
	return ROM_NAME( cv1797 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_cv1797_device - constructor
//-------------------------------------------------

bbc_cv1797_device::bbc_cv1797_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_CV1797, tag, owner, clock)
	, device_bbc_fdc_interface(mconfig, *this)
	, m_fdc(*this, "fd1797")
	, m_floppy0(*this, "fd1797:0")
	, m_floppy1(*this, "fd1797:1")
	, m_drive_control(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_cv1797_device::device_start()
{
	save_item(NAME(m_drive_control));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_cv1797_device::read(offs_t offset)
{
	uint8_t data;

	if (offset & 0x04)
	{
		data = m_fdc->read(offset & 0x03);
	}
	else
	{
		data = m_drive_control;
	}
	return data;
}

void bbc_cv1797_device::write(offs_t offset, uint8_t data)
{
	if (offset & 0x04)
	{
		m_fdc->write(offset & 0x03, data);
	}
	else
	{
		floppy_image_device *floppy = nullptr;

		m_drive_control = data;
		logerror("fdc: Drive control %02x\n", data);
		// bit 0: drive select
		switch (BIT(data, 0))
		{
		case 0: floppy = m_floppy0->get_device(); break;
		case 1: floppy = m_floppy1->get_device(); break;
		}
		m_fdc->set_floppy(floppy);

		// bit 1: side select
		if (floppy)
			floppy->ss_w(BIT(data, 1));

		// bit 2: density
		m_fdc->dden_w(!BIT(data, 2));

		// bit 6: reset
		//if (BIT(data, 6)) m_fdc->soft_reset();
	}
}

WRITE_LINE_MEMBER(bbc_cv1797_device::motor_w)
{
	if (m_floppy0->get_device()) m_floppy0->get_device()->mon_w(!state);
	if (m_floppy1->get_device()) m_floppy1->get_device()->mon_w(!state);
}
